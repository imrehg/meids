/**
 * @file me4600_device.c
 *
 * @brief ME-4600 device class implementation.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
 * Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <asm/uaccess.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_common.h"
# include "me_internal.h"
# include "me_plx9052_reg.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "mesubdevice.h"
# include "mefirmware.h"
# include "me4600_reg.h"
# include "me4600_dio.h"
# include "me8254.h"
# include "me4600_ai.h"
# include "me4600_ao.h"
# include "me4600_ext_irq.h"

# include "me4600_device.h"

# define ME_MODULE_NAME		ME4600_NAME
# define ME_MODULE_VERSION	ME4600_VERSION

static me_device_t* me4600_constr(me_general_dev_t* device, me_device_t* instance, const char* firmwarename);
static void me4600_get_device_info(me4600_device_t* device, uint16_t device_id);
static int me4600_config_load(me_device_t *me_device, struct file* filep, void* config, unsigned int size);

static irqreturn_t me4600_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

/**
 * @brief Global variable.
 * This is working queue for runing a separate task that will be responsible for work status (start, stop, timeouts).
 */
static 	struct workqueue_struct* me4600_workqueue;
# if defined(ME_PCI)
me_device_t* me4600_pci_constr(
# elif defined(ME_USB)
me_device_t* me4600_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me4600_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me4600_constr(device, instance, (device->device == PCI_DEVICE_ID_MEILHAUS_ME4610) ? ME4610_FIRMWARE : ME4600_FIRMWARE);
}

# if defined(ME_PCI)
me_device_t* me4800_pci_constr(
# elif defined(ME_USB)
me_device_t* me4800_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me4800_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me4600_constr(device, instance, ME4800_FIRMWARE);
}

static me_device_t* me4600_constr(me_general_dev_t* device, me_device_t* instance, const char* firmwarename)
{

	me4600_device_t* me4600_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int err;
	int i;
	uint32_t version;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me4600_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me4600_device = kzalloc(sizeof(me4600_device_t), GFP_KERNEL);
		if (!me4600_device)
		{
			PERROR("Cannot get memory for ME-4600 device instance.\n");
			return NULL;
		}

		// Initialize spin locks.
		ME_INIT_LOCK(&me4600_device->preload_reg_lock);
		ME_INIT_LOCK(&me4600_device->dio_lock);
		ME_INIT_LOCK(&me4600_device->ctr_ctrl_reg_lock);
		ME_INIT_LOCK(&me4600_device->ctr_clk_src_reg_lock);

		// Set constans.
		me4600_get_device_info(me4600_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me4600_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		me4600_device->preload_flags = 0;

		// Initialize irq context
		me4600_device->base.irq_context.no_subdev = me4600_versions[version_idx].ai_subdevices
												  + me4600_versions[version_idx].ao_fifo
												  + me4600_versions[version_idx].ext_irq_subdevices;

		if(me4600_device->base.irq_context.no_subdev > 0)
		{

			me4600_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me4600_device->base.irq_context.no_subdev), GFP_KERNEL);

			if (!me4600_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me4600_device->base.irq_context.int_status = kzalloc(sizeof(me4600_interrupt_status_t), GFP_KERNEL);
			if (!me4600_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me4600_interrupt_status_t *)me4600_device->base.irq_context.int_status)->intcsr = me4600_device->base.bus.PCI_Base[2] + ME4600_IRQ_STATUS_REG;

			me4600_device->base.irq_context.me_device_irq_handle = me4600_isr;
		}

		/// Download the xilinx firmware.
#if defined(ME_USB)
		err = me_fimware_download_NET2282_DMA(&me4600_device->base.bus.local_dev,
										me4600_device->base.bus.PCI_Base[1],
										me4600_device->base.bus.PCI_Base[2],
										(long int)me4600_device->base.bus.PCI_Base[4],
										firmwarename);
#else
		err = me_xilinx_download(&me4600_device->base.bus.local_dev,
									me4600_device->base.bus.PCI_Base[1],
									me4600_device->base.bus.PCI_Base[2],
									firmwarename);
#endif

		if (err)
		{
			PERROR("Can't download firmware.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// DI
		for (i = 0; i < me4600_versions[version_idx].di_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
					me4600_device->base.bus.PCI_Base[2],
					me4600_versions[version_idx].do_subdevices + i,
					&me4600_device->dio_lock,
					ME_TYPE_DI);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);
		}


		// DO
		for (i = 0; i < me4600_versions[version_idx].do_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
							me4600_device->base.bus.PCI_Base[2],
							i,
							&me4600_device->dio_lock,
							ME_TYPE_DO);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);
		}


		// DIO
		for (i = 0; i < me4600_versions[version_idx].dio_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
							me4600_device->base.bus.PCI_Base[2],
							me4600_versions[version_idx].do_subdevices + me4600_versions[version_idx].di_subdevices + i,
							&me4600_device->dio_lock,
							ME_TYPE_DIO);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);
		}


		// AI
		for (i = 0; i < me4600_versions[version_idx].ai_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ai_constr(
							me4600_device->base.bus.PCI_Base[2],
							me4600_device->base.bus.PCI_Base[4],
							me4600_device->base.bus.PCI_Base[1],
							i,
							me4600_versions[version_idx].ai_channels,
							me4600_versions[version_idx].ai_ranges,
							me4600_versions[version_idx].ai_features,
							/* (me4600_interrupt_status_t *)me4600_device->base.irq_context.int_status, */
							me4600_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AI%d subdevice.\n", i);
				goto ERROR;
			}

			me4600_device->base.irq_context.subdevice[i] = subdevice;

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);

		}


		// AO
		for (i = 0; i < me4600_versions[version_idx].ao_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ao_constr(
							me4600_device->base.bus.PCI_Base[2],
							me4600_device->base.bus.PCI_Base[4],
							i,
							&me4600_device->preload_reg_lock,
							&me4600_device->preload_flags,
							me4600_versions[version_idx].ao_fifo,
							me4600_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AO%d subdevice.\n", i);
				goto ERROR;
			}

			if (me4600_versions[version_idx].ao_fifo > i)
			{

				PDEBUG("Register subdevice %d for IRQ\n", ((me4600_ao_subdevice_t *)subdevice)->base.idx);
				me4600_device->base.irq_context.subdevice[me4600_versions[version_idx].ai_subdevices + i] = subdevice;
			}

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);
		}


		// CTR (8254)
		for (i = 0; i < me4600_versions[version_idx].ctr_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me8254_constr(
							device->device,
							me4600_device->base.bus.PCI_Base[3],
							0,
							i,
							&me4600_device->ctr_ctrl_reg_lock,
							&me4600_device->ctr_clk_src_reg_lock);


			if (!subdevice)
			{
				PERROR("Cannot get memory for CTR%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);
		}

		// EXT_IRQ
		for (i = 0; i < me4600_versions[version_idx].ext_irq_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ext_irq_constr(
							me4600_device->base.bus.PCI_Base[2],
							i);


			if (!subdevice)
			{
				PERROR("Cannot get memory for EXT_IRQ%d subdevice.\n", i);
				goto ERROR;
			}

			me4600_device->base.irq_context.subdevice[me4600_versions[version_idx].ai_subdevices + me4600_versions[version_idx].ao_fifo + i] = subdevice;

			me_slist_add(&me4600_device->base.slist, (void *)&me4600_device->base.bus.local_dev, subdevice);

		}

		// Init interrupts
		if (me_device_init_irq(&me4600_device->base))
		{
			goto ERROR;
		}
		// Overwrite base class methods.
		me4600_device->base.me_device_config_load = me4600_config_load;
	}
	else
	{
		me4600_device = (me4600_device_t *)instance;
		if (!me_device_reinit(instance, device))
		{
			/// Download the xilinx firmware.
#if defined(ME_USB)
			err = me_fimware_download_NET2282_DMA(&me4600_device->base.bus.local_dev,
											me4600_device->base.bus.PCI_Base[1],
											me4600_device->base.bus.PCI_Base[2],
											(long int)me4600_device->base.bus.PCI_Base[4],
											firmwarename);
#else
			err = me_xilinx_download(&me4600_device->base.bus.local_dev,
											me4600_device->base.bus.PCI_Base[1],
											me4600_device->base.bus.PCI_Base[2],
											firmwarename);
#endif
			if (err)
			{
				PERROR("Can't download firmware.\n");
				goto ERROR_REINIT;
			}
		}
		else
		{
			goto ERROR_REINIT;
		}
		// Init interrupts
		if (me_device_init_irq(&me4600_device->base))
		{
			goto ERROR_REINIT;
		}
	}

	/// Enable interrupts on PLX
	if (me4600_device->base.irq_context.no_subdev > 0)
	{
		me_writel(&me4600_device->base.bus.local_dev, ME_PLX9052_PCI_ACTIVATE_INT1, me4600_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
	}

	me_readl(&me4600_device->base.bus.local_dev, &version, me4600_device->base.bus.PCI_Base[2] + 0xC4);
	PLOG("me4600 firmware version: %04x %01x %02x %01x\n", (version >> 16) & 0xFFFF, (version>>12) & 0xF, (version>>4) & 0xFF, version & 0xF);

	return (me_device_t *) me4600_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me4600_device)
	{
		me_device_disconnect((me_device_t *)me4600_device);
		if (me4600_device->base.me_device_destructor)
		{
			me4600_device->base.me_device_destructor((me_device_t *)me4600_device);
		}
		kfree(me4600_device);
		me4600_device = NULL;
	}

ERROR_REINIT:
#if defined(ME_USB)
	if (1)
	{
		uint32_t tmp;
		NET2282_NET2282_reg_read(device, &tmp, NET2282_USBCTL);
		NET2282_NET2282_reg_write(device, tmp | NET2282_TIMED_DISCONNECT, NET2282_USBCTL);
		PERROR("Timed disconnect\n");
	}
#endif
	return (me_device_t *) me4600_device;
}

static void me4600_get_device_info(me4600_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME4610:
			device->base.info.device_name = 		ME4000_NAME_DEVICE_ME4610;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4610;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4650:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4650;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4650;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4650I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4650I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4650I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4650S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4650S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4650S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4650IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4650IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4650IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4660:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4660;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4660;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4660I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4660I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4660I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4660S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4660S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4660S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4660IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4660IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4660IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4670:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4670;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4670;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4670I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4670I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4670I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4670S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4670S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4670S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4670IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4670IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4670IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4680:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4680;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4680;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4680I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4680I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4680I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4680S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4680S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4680S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4680IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4680IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4680IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4850:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4850;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4850;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4850I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4850I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4850I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4850S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4850S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4850S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4850IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4850IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4850IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4860:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4860;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4860;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4860I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4860I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4860I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4860S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4860S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4860S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4860IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4860IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4860IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4870:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4870;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4870;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4870I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4870I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4870I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4870S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4870S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4870S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4870IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4870IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4870IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4880:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4880;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4880;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4880I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4880I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4880I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4880S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4880S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4880S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4880IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4880IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4880IS;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me4600_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me4600_interrupt_status_t* interrupt_status;
	me4600_device_t* me4600_dev;
	me_subdevice_t* instance;
	uint32_t irq_status_val;
	int version_idx;
	int i, gi=0;

	irqreturn_t ret = IRQ_NONE;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me4600_dev = container_of((void *)irq_context, me4600_device_t, base.irq_context);
	interrupt_status = (me4600_interrupt_status_t *)irq_context->int_status;
	version_idx = me4600_versions_get_device_index(me4600_dev->base.bus.local_dev.device);

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	me_readl(&me4600_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	for (i=0; i < me4600_versions[version_idx].ai_subdevices; i++, gi++)
	{
		if (irq_status_val & (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC | ME4600_IRQ_STATUS_BIT_LE | ME4600_IRQ_STATUS_BIT_AI_OF))
		{
			PINFO("Interrupt for AI_subdevice %d\n", i);
			instance = irq_context->subdevice[gi];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			{
				ret = IRQ_HANDLED;
			}
		}
	}

	for (i=0; i<me4600_versions[version_idx].ao_fifo; i++, gi++)
	{// More deep checking is done in subdevices.
		if (irq_status_val & (ME4600_IRQ_STATUS_BIT_AO_HF << i))
		{
			PINFO("Interrupt for AO_subdevice %d\n", i);
			instance = irq_context->subdevice[gi];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			{
				ret = IRQ_HANDLED;
			}
		}
	}

	for (i=0; i<me4600_versions[version_idx].ext_irq_subdevices; i++, gi++)
	{
		if (irq_status_val & ME4600_IRQ_STATUS_BIT_EX)
		{
			PINFO("Interrupt for EXT_IRQ_subdevice %d\n", i);
			instance = irq_context->subdevice[gi];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			{
				ret = IRQ_HANDLED;
			}
		}
	}

	if (ret == IRQ_HANDLED)
	{
		irq_context->unhandled_irq = 0;
	}
	else
	{
#ifdef MEDEBUG_DEBUG
		if (!irq_context->unhandled_irq)
		{
			PDEBUG("IRQ triggered by ghost! irq_status_val=0x%08x\n", irq_status_val);
		}
#endif

		irq_context->unhandled_irq++;
		if (irq_context->unhandled_irq > ME_MAX_UNHANDLED_IRQ)
		{	// Kernel blockes IRQ when too many calls is not handled.
			irq_context->unhandled_irq = 1;
			ret = IRQ_HANDLED;
		}

	}
	return ret;
}

static int me4600_config_load(me_device_t *me_device, struct file* filep, void* config, unsigned int size)
{
	me4600_device_t* me4600_device;
	me4600_config_load_t me4600_config;
	me_subdevice_t* subdevice;
	int err = ME_ERRNO_SUCCESS;
	int no_subdev;
	int i;

	PDEBUG("executed.\n");

	me4600_device = (me4600_device_t *) me_device;

	if (size != sizeof(me4600_config_load_t))
	{
		PERROR("Invalid configuration size. Must be %ld [sizeof(me4600_config_load_t)].\n", (long int)sizeof(me4600_config_load_t));
		return ME_ERRNO_INTERNAL;
	}

	if (!config)
	{
		return ME_ERRNO_INVALID_POINTER;
	}

	err = copy_from_user(&me4600_config, config, size);
	if (err)
	{
		PERROR("Can't copy config entry structure to kernel space.\n");
		return -EFAULT;
	}

	no_subdev = me_slist_get_number_subdevices(&me4600_device->base.slist);

	// Enter device.
	err = me_dlock_enter(&me_device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	for (i=0; i<no_subdev; ++i)
	{
		// Add more subdevices
		subdevice = me_slist_get_subdevice(&me_device->slist, i);
		if (!subdevice)
		{
			PERROR("Cannot access subdevice %d.\n", i);
			err = ME_ERRNO_INTERNAL;
			break;
		}
		if (subdevice->me_subdevice_config_load)
		{
			subdevice->me_subdevice_config_load(subdevice, filep, &config);
		}
	}

	// Exit device.
	me_dlock_exit(&me_device->dlock, filep);

	return err;
}


// Init and exit of module.
static int __init me4600_init(void)
{
	PDEBUG("executed.\n");

	me4600_workqueue = create_singlethread_workqueue("me4600");
	return 0;
}

static void __exit me4600_exit(void)
{
	PDEBUG("executed.\n");

	if (me4600_workqueue)
	{
		flush_workqueue(me4600_workqueue);
		destroy_workqueue(me4600_workqueue);
		me4600_workqueue = NULL;
	}
}

module_init(me4600_init);
module_exit(me4600_exit);


// Administrative stuff for modinfo.
MODULE_AUTHOR("Guenter Gebhardt & Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_DESCRIPTION("Device Driver Module for ME-46xx and ME-48xx Devices");
MODULE_SUPPORTED_DEVICE("Meilhaus ME-46xx and ME-48xx Devices");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));
MODULE_FIRMWARE(ME4610_FIRMWARE);
MODULE_FIRMWARE(ME4800_FIRMWARE);
MODULE_FIRMWARE(ME4600_FIRMWARE);


// Export the constructors.
# if defined(ME_PCI)
EXPORT_SYMBOL(me4600_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me4600_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me4600_comedi_constr);
# endif
# if defined(ME_PCI)
EXPORT_SYMBOL(me4800_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me4800_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me4800_comedi_constr);
# endif
