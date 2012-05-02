/**
 * @file me0700_device.c
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

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_common.h"
# include "me_internal.h"
# include "me_plx9052_reg.h"
# include "mehardware_access.h"

# include "mesubdevice.h"
# include "mefirmware.h"
# include "me0700_reg.h"
# include "me4600_dio.h"
# include "me8254.h"
# include "me0700_ai.h"
# include "me4600_ao.h"

# include "me0700_device.h"

# define ME_MODULE_NAME		ME0700_NAME
# define ME_MODULE_VERSION	ME0700_VERSION
# define RESERVED_DIO_PORTS	2

# define ME0700_FIRMWARE "me4600.bin"

static void me0700_get_device_info(me0700_device_t* device, uint16_t device_id);
static irqreturn_t me0700_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

/**
 * @brief Global variable.
 * This is working queue for runing a separate task that will be responsible for work status (start, stop, timeouts).
 */
static 	struct workqueue_struct* me0700_workqueue;

# if defined(ME_PCI)
me_device_t* me0700_pci_constr(
# elif defined(ME_USB)
me_device_t* me0700_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me0700_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	me0700_device_t* me0700_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int err;
	int i;
	uint32_t version;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me0700_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me0700_device = kzalloc(sizeof(me0700_device_t), GFP_KERNEL);
		if (!me0700_device)
		{
			PERROR("Cannot get memory for ME-0700 device instance.\n");
			return NULL;
		}

		// Initialize spin locks.
		ME_INIT_LOCK(&me0700_device->preload_reg_lock);
		ME_INIT_LOCK(&me0700_device->dio_lock);
		ME_INIT_LOCK(&me0700_device->ctr_ctrl_reg_lock);
		ME_INIT_LOCK(&me0700_device->ctr_clk_src_reg_lock);

		// Set constans.
		me0700_get_device_info(me0700_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me0700_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		me0700_device->preload_flags = 0;

		// Initialize irq context
		me0700_device->base.irq_context.no_subdev = me0700_versions[version_idx].ai_subdevices
												  + me0700_versions[version_idx].ao_fifo;

		if(me0700_device->base.irq_context.no_subdev > 0)
		{

			me0700_device->base.irq_context.subdevice = kzalloc((me0700_device->base.irq_context.no_subdev) * sizeof(me_subdevice_t*), GFP_KERNEL);

			if (!me0700_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me0700_device->base.irq_context.int_status = kzalloc(sizeof(me0700_interrupt_status_t), GFP_KERNEL);
			if (!me0700_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me0700_interrupt_status_t *)me0700_device->base.irq_context.int_status)->intcsr = me0700_device->base.bus.PCI_Base[2] + ME0700_IRQ_STATUS_REG;

			me0700_device->base.irq_context.me_device_irq_handle = me0700_isr;
		}

		/// Download the xilinx firmware.
#if defined(ME_USB)
		err = me_fimware_download_NET2282_DMA(&me0700_device->base.bus.local_dev,
										me0700_device->base.bus.PCI_Base[1],
										me0700_device->base.bus.PCI_Base[2],
										(long int)me0700_device->base.bus.PCI_Base[4],
										ME0700_FIRMWARE);
#else
		err = me_xilinx_download(&me0700_device->base.bus.local_dev,
									me0700_device->base.bus.PCI_Base[1],
									me0700_device->base.bus.PCI_Base[2],
									ME0700_FIRMWARE);
#endif

		if (err)
		{
			PERROR("Can't download firmware.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// DIO
		for (i = 0; i < me0700_versions[version_idx].dio_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_dio_constr(
							me0700_device->base.bus.PCI_Base[2],
							RESERVED_DIO_PORTS + i,
							&me0700_device->dio_lock,
							ME_TYPE_DIO);


			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me0700_device->base.slist, (void *)&me0700_device->base.bus.local_dev, subdevice);
		}


		// AI
		for (i = 0; i < me0700_versions[version_idx].ai_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me0700_ai_constr(
							me0700_device->base.bus.PCI_Base[2],
							me0700_device->base.bus.PCI_Base[4],
							me0700_device->base.bus.PCI_Base[1],
							i,
							me0700_versions[version_idx].ai_channels,
							me0700_versions[version_idx].ai_ranges,
							me0700_versions[version_idx].ai_features,
							/*(me0700_interrupt_status_t *)me0700_device->base.irq_context.int_status,*/
							me0700_workqueue,
							&me0700_device->base.bus.local_dev,
							&me0700_device->dio_lock,
							me0700_versions[version_idx].ai_current_channels);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AI%d subdevice.\n", i);
				goto ERROR;
			}

			me0700_device->base.irq_context.subdevice[i] = subdevice;

			me_slist_add(&me0700_device->base.slist, (void *)&me0700_device->base.bus.local_dev, subdevice);

		}


		// AO
		for (i = 0; i < me0700_versions[version_idx].ao_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me4600_ao_constr(
							me0700_device->base.bus.PCI_Base[2],
							me0700_device->base.bus.PCI_Base[4],
							i,
							&me0700_device->preload_reg_lock,
							&me0700_device->preload_flags,
							me0700_versions[version_idx].ao_fifo,
							me0700_workqueue);


			if (!subdevice)
			{
				PERROR("Cannot get memory for AO%d subdevice.\n", i);
				goto ERROR;
			}

			if (me0700_versions[version_idx].ao_fifo > i)
			{

				PDEBUG("Register subdevice %d for IRQ\n", ((me4600_ao_subdevice_t *)subdevice)->base.idx);
				me0700_device->base.irq_context.subdevice[me0700_versions[version_idx].ai_subdevices + i] = subdevice;
			}

			me_slist_add(&me0700_device->base.slist, (void *)&me0700_device->base.bus.local_dev, subdevice);
		}


		// CTR (8254)
		for (i = 0; i < me0700_versions[version_idx].ctr_subdevices; i++)
		{

			subdevice = (me_subdevice_t *) me8254_constr(
							device->device,
							me0700_device->base.bus.PCI_Base[3],
							0,
							i,
							&me0700_device->ctr_ctrl_reg_lock,
							&me0700_device->ctr_clk_src_reg_lock);


			if (!subdevice)
			{
				PERROR("Cannot get memory for CTR%d subdevice.\n", i);
				goto ERROR;
			}

			me_slist_add(&me0700_device->base.slist, (void *)&me0700_device->base.bus.local_dev, subdevice);
		}

		// Init interrupts
		if (me_device_init_irq(&me0700_device->base))
		{
			goto ERROR;
		}
	}
	else
	{
		me0700_device = (me0700_device_t *)instance;
		if (!me_device_reinit(instance, device))
		{
			/// Download the xilinx firmware.
#if defined(ME_USB)
			err = me_fimware_download_NET2282_DMA(&me0700_device->base.bus.local_dev,
											me0700_device->base.bus.PCI_Base[1],
											me0700_device->base.bus.PCI_Base[2],
											(long int)me0700_device->base.bus.PCI_Base[4],
											ME0700_FIRMWARE);
#else
			err = me_xilinx_download(&me0700_device->base.bus.local_dev,
											me0700_device->base.bus.PCI_Base[1],
											me0700_device->base.bus.PCI_Base[2],
											ME0700_FIRMWARE);
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
		if (me_device_init_irq(&me0700_device->base))
		{
			goto ERROR_REINIT;
		}
	}

	/// Enable interrupts on PLX
	if (me0700_device->base.irq_context.no_subdev > 0)
	{
		me_writel(&me0700_device->base.bus.local_dev, ME_PLX9052_PCI_ACTIVATE_INT1, me0700_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
	}

	me_readl(&me0700_device->base.bus.local_dev, &version, me0700_device->base.bus.PCI_Base[2] + 0xC4);
	PLOG("me0700/me4600 firmware version: %04x %01x %02x %01x\n", (version >> 16) & 0xFFFF, (version>>12) & 0xF, (version>>4) & 0xFF, version & 0xF);

	return (me_device_t *) me0700_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me0700_device)
	{
		me_device_disconnect((me_device_t *)me0700_device);
		if (me0700_device->base.me_device_destructor)
		{
			me0700_device->base.me_device_destructor((me_device_t *)me0700_device);
		}
		kfree(me0700_device);
		me0700_device = NULL;
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
	return (me_device_t *) me0700_device;
}

static void me0700_get_device_info(me0700_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME0752:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0752;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0752;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0754:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0754;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0754;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0762:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0762;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0762;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0764:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0764;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0764;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0772:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0772;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0772;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0774:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0774;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0774;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0782:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0782;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0782;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0784:
			device->base.info.device_name = 		ME0700_NAME_DEVICE_ME0784;
			device->base.info.device_description =	ME0700_DESCRIPTION_DEVICE_ME0784;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me0700_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me0700_interrupt_status_t* interrupt_status;
	me0700_device_t* me0700_dev;
	me_subdevice_t* instance;
	uint32_t irq_status_val;
	int version_idx;
	int i, gi=0;

	irqreturn_t ret = IRQ_NONE;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me0700_dev = container_of((void *)irq_context, me0700_device_t, base.irq_context);
	interrupt_status = (me0700_interrupt_status_t *)irq_context->int_status;
	version_idx = me0700_versions_get_device_index(me0700_dev->base.bus.local_dev.device);

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	me_readl(&me0700_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	for (i=0; i < me0700_versions[version_idx].ai_subdevices; i++, gi++)
	{
		if (irq_status_val & (ME0700_IRQ_STATUS_BIT_AI_HF | ME0700_IRQ_STATUS_BIT_SC | ME0700_IRQ_STATUS_BIT_LE | ME0700_IRQ_STATUS_BIT_AI_OF | ME4600_IRQ_STATUS_BIT_EX))
		{
			PINFO("Interrupt for AI_subdevice %d\n", i);
			instance = irq_context->subdevice[gi];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			{
				ret = IRQ_HANDLED;
			}
		}
	}

	for (i=0; i<me0700_versions[version_idx].ao_fifo; i++, gi++)
	{// More deep checking is done in subdevices.
		if (irq_status_val & (ME0700_IRQ_STATUS_BIT_AO_HF << i))
		{
			PINFO("Interrupt for AO_subdevice %d\n", i);
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
static int __init me0700_init(void)
{
	PDEBUG("executed.\n");

	me0700_workqueue = create_singlethread_workqueue("me0700");
	return 0;
	return 0;
}

static void __exit me0700_exit(void)
{
	PDEBUG("executed.\n");

	if (me0700_workqueue)
	{
		flush_workqueue(me0700_workqueue);
		destroy_workqueue(me0700_workqueue);
		me0700_workqueue = NULL;
	}
}

module_init(me0700_init);
module_exit(me0700_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-07xx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-07xx Devices");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));
MODULE_FIRMWARE(ME0700_FIRMWARE);


// Export the constructors.
# if defined(ME_PCI)
EXPORT_SYMBOL(me0700_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me0700_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me0700_comedi_constr);
# endif
