/**
 * @file me4700_device.c
 *
 * @brief ME-4700 device class implementation.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# include "me4700_fi.h"
# include "me4700_fo.h"

# include "me4700_device.h"

# define ME_MODULE_NAME  ME4700_NAME
# define ME_MODULE_VERSION	ME4700_VERSION

static me_device_t* me4700_constr(me_general_dev_t* device, me_device_t* instance, const char* firmwarename);
static void me4700_destructor(struct me_device* device);
static void me4700_get_device_info(me4700_device_t* device, uint16_t device_id);
static irqreturn_t me4700_isr(int irq, void* context

#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

/**
 * @brief Global variable.
 * This is working queue for runing a separate task that will be responsible for work status (start, stop, timeouts).
 */
static 	struct workqueue_struct* me4700_workqueue;

# if defined(ME_PCI)
me_device_t* me4500_pci_constr(
# elif defined(ME_USB)
me_device_t* me4500_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me4500_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me4700_constr(device, instance, ME4500_FIRMWARE);
}

# if defined(ME_PCI)
me_device_t* me4700_pci_constr(
# elif defined(ME_USB)
me_device_t* me4700_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me4700_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me4700_constr(device, instance, ME4700_FIRMWARE);
}

static me_device_t* me4700_constr(me_general_dev_t* device, me_device_t* instance, const char* firmwarename)
{	me4700_device_t* me4700_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int err;
	int i;
	uint32_t version;
	int irq_idx;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me4700_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me4700_device = kzalloc(sizeof(me4700_device_t), GFP_KERNEL);
		if (!me4700_device)
		{
			PERROR("Cannot get memory for ME-4600 device instance.\n");
			return NULL;
		}

		// Initialize spin locks.
		ME_INIT_LOCK(&me4700_device->preload_reg_lock);
		ME_INIT_LOCK(&me4700_device->dio_fio_lock);
		ME_INIT_LOCK(&me4700_device->ctr_ctrl_reg_lock);
		ME_INIT_LOCK(&me4700_device->ctr_clk_src_reg_lock);

		// Set constans.
		me4700_get_device_info(me4700_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me4700_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		me4700_device->preload_flags = 0;

		// Create and initializate FO shared context instance.
		me4700_device->fo_shared_context.regs = kmalloc(me4700_versions[version_idx].fo_subdevices * sizeof(me4700_fo_regs_t), GFP_KERNEL);
		if (!me4700_device->fo_shared_context.regs)
		{
			PERROR_CRITICAL("Cannot get memory for registers.\n");
			goto ERROR;
		}
		me4700_device->fo_shared_context.shadow = kmalloc(me4700_versions[version_idx].fo_subdevices * sizeof(me4700_fo_params_t), GFP_KERNEL);
		if (!me4700_device->fo_shared_context.shadow)
		{
			PERROR_CRITICAL("Cannot get memory for shadows.\n");
			goto ERROR;
		}
		me4700_device->fo_shared_context.mirror = kmalloc(me4700_versions[version_idx].fo_subdevices * sizeof(me4700_fo_params_t), GFP_KERNEL);
		if (!me4700_device->fo_shared_context.mirror)
		{
			PERROR_CRITICAL("Cannot get memory for mirrors.\n");
			goto ERROR;
		}
		me4700_device->fo_shared_context.count = me4700_versions[version_idx].fo_subdevices;
		ME_INIT_LOCK(&me4700_device->fo_shared_context.fo_context_lock);

		// Overwrite base class methods. - Needed for cleaning shadows.
		me4700_device->base.me_device_destructor = me4700_destructor;

		// Initialize irq context
		me4700_device->base.irq_context.no_subdev = me4700_versions[version_idx].ai_subdevices
												  + me4700_versions[version_idx].ao_fifo
												  + me4700_versions[version_idx].fi_subdevices
												  + me4700_versions[version_idx].ext_irq_subdevices;
		irq_idx = 0;

		if(me4700_device->base.irq_context.no_subdev > 0)
		{

			me4700_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me4700_device->base.irq_context.no_subdev), GFP_KERNEL);

			if (!me4700_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me4700_device->base.irq_context.int_status = kzalloc(sizeof(me4700_interrupt_status_t), GFP_KERNEL);
			if (!me4700_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me4700_interrupt_status_t *)me4700_device->base.irq_context.int_status)->intcsr = me4700_device->base.bus.PCI_Base[2] + ME4600_IRQ_STATUS_REG;

			me4700_device->base.irq_context.me_device_irq_handle = me4700_isr;
		}

		/// Download the xilinx firmware.
#if defined(ME_USB)
		err = me_fimware_download_NET2282_DMA(&me4700_device->base.bus.local_dev,
										me4700_device->base.bus.PCI_Base[1],
										me4700_device->base.bus.PCI_Base[2],
										(long int)me4700_device->base.bus.PCI_Base[4],
										firmwarename);
#else
		err = me_xilinx_download(&me4700_device->base.bus.local_dev,
									me4700_device->base.bus.PCI_Base[1],
									me4700_device->base.bus.PCI_Base[2],
									firmwarename);
#endif

		if (err)
		{
			PERROR("Can't download firmware.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// DI
		for (i = 0; i < me4700_versions[version_idx].di_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
					me4700_device->base.bus.PCI_Base[2],
					me4700_versions[version_idx].do_subdevices + i,
					&me4700_device->dio_fio_lock,
					ME_TYPE_DI);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}


		// DO
		for (i = 0; i < me4700_versions[version_idx].do_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
							me4700_device->base.bus.PCI_Base[2],
							i,
							&me4700_device->dio_fio_lock,
							ME_TYPE_DO);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}


		// DIO
		for (i = 0; i < me4700_versions[version_idx].dio_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
							me4700_device->base.bus.PCI_Base[2],
							me4700_versions[version_idx].do_subdevices + me4700_versions[version_idx].di_subdevices + i,
							&me4700_device->dio_fio_lock,
							ME_TYPE_DIO);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}


		// AI
		for (i = 0; i < me4700_versions[version_idx].ai_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ai_constr(
							me4700_device->base.bus.PCI_Base[2],
							me4700_device->base.bus.PCI_Base[4],
							me4700_device->base.bus.PCI_Base[1],
							i,
							me4700_versions[version_idx].ai_channels,
							me4700_versions[version_idx].ai_ranges,
							me4700_versions[version_idx].ai_features,
							/* (me4700_interrupt_status_t *)me4700_device->base.irq_context.int_status, */
							me4700_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AI%d subdevice.\n", i);
				goto ERROR;
			}

			me4700_device->base.irq_context.subdevice[irq_idx++] = subdevice;

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);

		}


		// AO
		for (i = 0; i < me4700_versions[version_idx].ao_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ao_constr(
							me4700_device->base.bus.PCI_Base[2],
							me4700_device->base.bus.PCI_Base[4],
							i,
							&me4700_device->preload_reg_lock,
							&me4700_device->preload_flags,
							me4700_versions[version_idx].ao_fifo,
							me4700_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AO%d subdevice.\n", i);
				goto ERROR;
			}

			if (me4700_versions[version_idx].ao_fifo > i)
			{

				PDEBUG("Register subdevice %d for IRQ\n", ((me4600_ao_subdevice_t *)subdevice)->base.idx);
				me4700_device->base.irq_context.subdevice[irq_idx++] = subdevice;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}


		// CTR (8254)
		for (i = 0; i < me4700_versions[version_idx].ctr_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me8254_constr(
							device->device,
							me4700_device->base.bus.PCI_Base[3],
							0,
							i,
							&me4700_device->ctr_ctrl_reg_lock,
							&me4700_device->ctr_clk_src_reg_lock);


			if (!subdevice)
			{
				PERROR("Cannot get memory for CTR%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}

		// FI
		for (i = 0; i < me4700_versions[version_idx].fi_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4700_fi_constr(
					me4700_device->base.bus.PCI_Base[2],
					i,
					&me4700_device->dio_fio_lock);


			if (!subdevice)
			{
				PERROR("Cannot get memory for FI%d subdevice.\n", i);
				goto ERROR;
			}

			me4700_device->base.irq_context.subdevice[irq_idx++] = subdevice;

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}


		// FO
		for (i = 0; i < me4700_versions[version_idx].fo_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4700_fo_constr(
							me4700_device->base.bus.PCI_Base[2],
							i,
							&me4700_device->dio_fio_lock,
							&me4700_device->fo_shared_context,
							me4700_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for FO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);
		}

		// EXT_IRQ
		for (i = 0; i < me4700_versions[version_idx].ext_irq_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ext_irq_constr(
							me4700_device->base.bus.PCI_Base[2],
							i);


			if (!subdevice)
			{
				PERROR("Cannot get memory for EXT_IRQ%d subdevice.\n", i);
				goto ERROR;
			}

			me4700_device->base.irq_context.subdevice[irq_idx++] = subdevice;

			me_slist_add(&me4700_device->base.slist, (void *)&me4700_device->base.bus.local_dev, subdevice);

		}

		// Init interrupts
		if (me_device_init_irq(&me4700_device->base))
		{
			goto ERROR;
		}
	}
	else
	{
		me4700_device = (me4700_device_t *)instance;
		if (!me_device_reinit(instance, device))
		{
			/// Download the xilinx firmware.
#if defined(ME_USB)
			err = me_fimware_download_NET2282_DMA(&me4700_device->base.bus.local_dev,
											me4700_device->base.bus.PCI_Base[1],
											me4700_device->base.bus.PCI_Base[2],
											(long int)me4700_device->base.bus.PCI_Base[4],
											firmwarename);
#else
			err = me_xilinx_download(&me4700_device->base.bus.local_dev,
											me4700_device->base.bus.PCI_Base[1],
											me4700_device->base.bus.PCI_Base[2],
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
		if (me_device_init_irq(&me4700_device->base))
		{
			goto ERROR_REINIT;
		}
	}

	/// Enable interrupts on PLX
	if (me4700_device->base.irq_context.no_subdev > 0)
	{
		me_writel(&me4700_device->base.bus.local_dev, ME_PLX9052_PCI_ACTIVATE_INT1, me4700_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
	}

	me_readl(&me4700_device->base.bus.local_dev, &version, me4700_device->base.bus.PCI_Base[2] + 0xC4);
	PLOG("me4700 firmware version: %04x %01x %02x %01x\n", (version >> 16) & 0xFFFF, (version>>12) & 0xF, (version>>4) & 0xFF, version & 0xF);

	return (me_device_t *) me4700_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me4700_device)
	{
		me_device_disconnect((me_device_t *)me4700_device);
		if (me4700_device->base.me_device_destructor)
		{
			me4700_device->base.me_device_destructor((me_device_t *)me4700_device);
		}
		kfree(me4700_device);
		me4700_device = NULL;
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
	return (me_device_t *) me4700_device;
}

static void me4700_destructor(struct me_device* device)
{
	me4700_device_t* me4700_device = (me4700_device_t *)device;
	PDEBUG("executed.\n");

	// Destroy shadow instance.
	if(me4700_device->fo_shared_context.regs)
	{
		kfree(me4700_device->fo_shared_context.regs);
		me4700_device->fo_shared_context.regs = NULL;
	}
	if(me4700_device->fo_shared_context.shadow)
	{
		kfree(me4700_device->fo_shared_context.shadow);
		me4700_device->fo_shared_context.shadow = NULL;
	}
	if(me4700_device->fo_shared_context.mirror)
	{
		kfree(me4700_device->fo_shared_context.mirror);
		me4700_device->fo_shared_context.mirror = NULL;
	}
}

static void me4700_get_device_info(me4700_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME4550:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4550;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4550;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4550I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4550I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4550I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4550S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4550S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4550S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4550IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4550IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4550IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4560:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4560;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4560;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4560I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4560I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4560I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4560S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4560S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4560S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4560IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4560IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4560IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4570:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4570;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4570;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4570I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4570I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4570I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4570S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4570S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4570S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4570IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4570IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4570IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4750:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4750;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4750;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4750I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4750I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4750I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4750S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4750S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4750S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4750IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4750IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4750IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4760:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4760;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4760;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4760I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4760I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4760I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4760S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4760S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4760S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4760IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4760IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4760IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4770:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4770;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4770;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4770I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4770I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4770I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4770S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4770S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4770S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4770IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4770IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4770IS;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4780:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4780;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4780;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4780I:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4780I;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4780I;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4780S:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4780S;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4780S;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4780IS:
			device->base.info.device_name =			ME4000_NAME_DEVICE_ME4780IS;
			device->base.info.device_description =	ME4000_DESCRIPTION_DEVICE_ME4780IS;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me4700_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me4600_interrupt_status_t* interrupt_status;
	me4700_device_t* me4700_dev;
	me_subdevice_t* instance;
	uint32_t irq_status_val;
	int version_idx;
	int i, gi=0;

	irqreturn_t ret = IRQ_NONE;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me4700_dev = container_of((void *)irq_context, me4700_device_t, base.irq_context);
	interrupt_status = (me4600_interrupt_status_t *)irq_context->int_status;
	version_idx = me4700_versions_get_device_index(me4700_dev->base.bus.local_dev.device);

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	me_readl(&me4700_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	for (i=0; i < me4700_versions[version_idx].ai_subdevices; i++, gi++)
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

	for (i=0; i<me4700_versions[version_idx].ao_fifo; i++, gi++)
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

	for (i=0; i<me4700_versions[version_idx].fi_subdevices; i++, gi++)
	{
		if (irq_status_val & (0x01 << (ME4700_FI_IRQ_CTRL_BIT_BASE + i)))
		{
			PINFO("Interrupt for FI_subdevice %d\n", i);
			instance = irq_context->subdevice[gi];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			{
				ret = IRQ_HANDLED;
			}
		}
	}

	for (i=0; i<me4700_versions[version_idx].ext_irq_subdevices; i++, gi++)
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

// Init and exit of module.
static int __init me4700_init(void)
{
	PDEBUG("executed.\n");

	me4700_workqueue = create_singlethread_workqueue("me4700");
	return 0;
}

static void __exit me4700_exit(void)
{
	PDEBUG("executed.\n");

	if (me4700_workqueue)
	{
		flush_workqueue(me4700_workqueue);
		destroy_workqueue(me4700_workqueue);
		me4700_workqueue = NULL;
	}
}

module_init(me4700_init);
module_exit(me4700_exit);


// Administrative stuff for modinfo.
MODULE_DESCRIPTION("Device Driver Module for ME-47xx and ME-45xx devices.");
MODULE_SUPPORTED_DEVICE("Meilhaus ME-47xx and ME-45xx Devices");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));
MODULE_FIRMWARE(ME4500_FIRMWARE);
MODULE_FIRMWARE(ME4700_FIRMWARE);


// Export the constructors.
# if defined(ME_PCI)
EXPORT_SYMBOL(me4700_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me4700_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me4700_comedi_constr);
# endif

# if defined(ME_PCI)
EXPORT_SYMBOL(me4500_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me4500_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me4500_comedi_constr);
# endif
