/**
 * @file me6000_device.c
 *
 * @brief The ME-6000 device class implementation.
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
# include "me_common.h"
# include "me_internal.h"
# include "me_plx9052_reg.h"
# include "mehardware_access.h"
# include "me_interrupt_types.h"

# include "mesubdevice.h"
# include "mefirmware.h"
# include "me6000_reg.h"
# include "me6000_dio.h"
# include "me6000_ao.h"

# include "me6000_device.h"

# define ME_MODULE_NAME		ME6000_NAME
# define ME_MODULE_VERSION	ME6000_VERSION

# define ME6000_FIRMWARE "me6000.bin"

static void me6000_get_device_info(me6000_device_t* device, uint16_t device_id);
static irqreturn_t me6000_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);
static me_device_t* me6x00_constr(me_general_dev_t* device, me_device_t* instance, int U_PLUS);

/**
 * @brief Global variable.
 * This is working queue for runing a separate atask that will be responsible for work status (start, stop, timeouts).
 */
static 	struct workqueue_struct* me6000_workqueue;

#if defined(ME_PCI)
me_device_t* me6000_pci_constr(
#elif defined(ME_USB)
me_device_t* me6000_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6000_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me6x00_constr(device, instance, 0);
}

#if defined(ME_PCI)
me_device_t* me6100_pci_constr(
#elif defined(ME_USB)
me_device_t* me6100_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6100_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me6x00_constr(device, instance, 0);
}

#if defined(ME_PCI)
me_device_t* me6200_pci_constr(
#elif defined(ME_USB)
me_device_t* me6200_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6200_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me6x00_constr(device, instance, (device->device == PCI_DEVICE_ID_MEILHAUS_ME6259)?1:0);
}

#if defined(ME_PCI)
me_device_t* me6300_pci_constr(
#elif defined(ME_USB)
me_device_t* me6300_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6300_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	return me6x00_constr(device, instance, (device->device == PCI_DEVICE_ID_MEILHAUS_ME6359)?1:0);
}

static me_device_t* me6x00_constr(me_general_dev_t* device, me_device_t* instance, int U_PLUS)
{
	me6000_device_t* me6000_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int err = 0;
	int i;
	int fifo;
	int high_range = 0;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me6000_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me6000_device = kzalloc(sizeof(me6000_device_t), GFP_KERNEL);
		if (!me6000_device)
		{
			PERROR("Cannot get memory for %s instance.\n", ME_MODULE_NAME);
			return NULL;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me6000_device->preload_reg_lock);
		ME_INIT_LOCK(&me6000_device->dio_ctrl_reg_lock);

		// Set constans.
		me6000_get_device_info(me6000_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me6000_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		me6000_device->preload_flags = 0;

		// Initialize irq context
		me6000_device->base.irq_context.no_subdev = me6000_versions[version_idx].ao_fifo;
		if(me6000_device->base.irq_context.no_subdev > 0)
		{
			me6000_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me6000_device->base.irq_context.no_subdev), GFP_KERNEL);
			if (!me6000_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me6000_device->base.irq_context.int_status = kmalloc(sizeof(me6000_interrupt_status_t), GFP_KERNEL);
			if (!me6000_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}

			((me6000_interrupt_status_t *)me6000_device->base.irq_context.int_status)->intcsr = me6000_device->base.bus.PCI_Base[2] + ME6000_AO_IRQ_STATUS_REG;

			me6000_device->base.irq_context.me_device_irq_handle = me6000_isr;
		}
		/// Download the xilinx firmware.
#if defined(ME_USB)
		err = me_fimware_download_NET2282_DMA(&me6000_device->base.bus.local_dev,
										me6000_device->base.bus.PCI_Base[1],
										me6000_device->base.bus.PCI_Base[2],
										(long int)me6000_device->base.bus.PCI_Base[4],
										ME6000_FIRMWARE);
#else
		err = me_xilinx_download(&me6000_device->base.bus.local_dev,
									me6000_device->base.bus.PCI_Base[1],
									me6000_device->base.bus.PCI_Base[2],
									ME6000_FIRMWARE);
#endif
		if (err)
		{
			PERROR("Can't download firmware.\n");
			goto ERROR;
		}
		/// Create subdevice instances.
		//DIO
		for (i = 0; i < me6000_versions[version_idx].dio_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me6000_dio_constr(
							me6000_device->base.bus.PCI_Base[3],
							i,
							&me6000_device->dio_ctrl_reg_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me6000_device->base.slist, (void *)&me6000_device->base.bus.local_dev, subdevice);
		}

		// AO
		for (i = 0; i < me6000_versions[version_idx].ao_subdevices; i++)
		{
			high_range = ((i==8) && (U_PLUS)) ? 1 : 0;

			fifo = (i < me6000_versions[version_idx].ao_fifo) ? ME6000_AO_HAS_FIFO : 0x0;
			fifo |= (i < 4) ? ME6000_AO_EXTRA_HARDWARE : 0x0;	//First 4 channels have different hardware than others.

			subdevice = (me_subdevice_t *) me6000_ao_constr(
							me6000_device->base.bus.PCI_Base[2],
							me6000_device->base.bus.PCI_Base[4],
							i,
							&me6000_device->preload_reg_lock,
							&me6000_device->preload_flags,
							&me6000_device->triggering_flags,
							fifo,
							high_range,
							me6000_workqueue);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}

			if (fifo & ME6000_AO_HAS_FIFO)
			{
				PDEBUG("Register subdevice %d for IRQ\n", ((me6000_ao_subdevice_t *)subdevice)->base.idx);
				me6000_device->base.irq_context.subdevice[i] = subdevice;
			}
			me_slist_add(&me6000_device->base.slist, (void *)&me6000_device->base.bus.local_dev, subdevice);
		}

		// Init interrupts
		err = me_device_init_irq(&me6000_device->base);
		if (err)
		{
			goto ERROR;
		}

	}
	else
	{

		me6000_device = (me6000_device_t *)instance;
		if (!me_device_reinit(instance, device))
		{
			/// Download the xilinx firmware.
#if defined(ME_USB)
		err = me_fimware_download_NET2282_DMA(&me6000_device->base.bus.local_dev,
										me6000_device->base.bus.PCI_Base[1],
										me6000_device->base.bus.PCI_Base[2],
										(long int)me6000_device->base.bus.PCI_Base[4],
										ME6000_FIRMWARE);
#else
		err = me_xilinx_download(&me6000_device->base.bus.local_dev,
									me6000_device->base.bus.PCI_Base[1],
									me6000_device->base.bus.PCI_Base[2],
									ME6000_FIRMWARE);
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
		err = me_device_init_irq(&me6000_device->base);
		if (err)
		{
			goto ERROR_REINIT;
		}
	}

	/// Enable interrupts on PLX
	if (me6000_device->base.irq_context.no_subdev > 0)
	{
		PDEBUG("Enable interrupts on PLX\n");
		me_writel(&me6000_device->base.bus.local_dev, ME_PLX9052_PCI_ACTIVATE_INT1, me6000_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
	}

	return (me_device_t *) me6000_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me6000_device)
	{
		me_device_disconnect((me_device_t *)me6000_device);
		if(me6000_device->base.me_device_destructor)
		{
			me6000_device->base.me_device_destructor((me_device_t *)me6000_device);
		}
		kfree(me6000_device);
		me6000_device = NULL;
	}

ERROR_REINIT:
#if defined(ME_USB)
	if (1)
	{
		uint32_t tmp;
		NET2282_NET2282_reg_read(device, &tmp, NET2282_USBCTL);
		NET2282_NET2282_reg_write(device, tmp | NET2282_TIMED_DISCONNECT, NET2282_USBCTL);
		PERROR("Timed disconnect.\n");
	}
#endif
	return (me_device_t *) me6000_device;
}

static void me6000_get_device_info(me6000_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id & ~ME6000_MEMORY_MAP_MARKER)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME6004:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME60004;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME60004;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6008:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME60008;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME60008;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME600F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME600016;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME600016;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6014:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I4;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I4;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6018:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I8;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I8;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME601F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I16;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I16;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6034:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000ISLE4;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000ISLE4;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6038:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000ISLE8;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000ISLE8;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME603F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000ISLE16;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000ISLE16;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6104:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME61004;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME61004;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6108:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME61008;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME61008;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME610F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME610016;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME610016;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6114:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I4;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I4;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6118:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I8;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I8;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME611F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I16;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I16;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6134:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE4;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE4;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6138:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE8;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE8;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME613F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE16;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE16;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6044:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME60004DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME60004DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6048:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME60008DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME60008DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME604F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME600016DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME600016DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6054:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I4DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I4DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6058:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I8DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I8DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME605F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000I16DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000I16DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6078:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000ISLE8DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000ISLE8DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME607F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6000ISLE16DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6000ISLE16DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6144:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME61004DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME61004DIO;
			break;

			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
		case PCI_DEVICE_ID_MEILHAUS_ME6148:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME61008DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME61008DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME614F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME610016DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME610016DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6154:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I4DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I4DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6158:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I8DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I8DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME615F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100I16DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100I16DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6174:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE4DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE4DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6178:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE8DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE8DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME617F:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6100ISLE16DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6100ISLE16DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6259:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6200I9DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6200I9DIO;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME6359:
			device->base.info.device_name =			ME6000_NAME_DEVICE_ME6300I9DIO;
			device->base.info.device_description =	ME6000_DESCRIPTION_DEVICE_ME6300I9DIO;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me6000_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me6000_interrupt_status_t* interrupt_status;
	me6000_device_t* me6000_dev;
	me_subdevice_t* instance;
	int i;

	irqreturn_t ret = IRQ_NONE;
	uint32_t irq_status_val = 0;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me6000_dev = container_of((void *)irq_context, me6000_device_t, base.irq_context);
	interrupt_status = (me6000_interrupt_status_t *)irq_context->int_status;

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	if (irq_status_val & 0x0F)
	{
		PDEBUG("irq_status_val=0x%08x\n", irq_status_val);
		PDEBUG("irq_context->no_subdev=%d\n", irq_context->no_subdev);
	}

	me_readl(&me6000_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	for (i=0; i < irq_context->no_subdev; i++)
	{
		if (irq_status_val & (ME6000_IRQ_STATUS_BIT_AO_HF << i))
		{//Interrupt line X
			PINFO("Interrupt for AO_subdevice %d\n", i);
			instance = irq_context->subdevice[i];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
				ret = IRQ_HANDLED;
		}
	}

	if (!(irq_status_val & 0x0F))
	{
		PDEBUG("IRQ triggered by ghost! irq_status_val=0x%08x\n", irq_status_val);
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
static int __init me6000_init(void)
{
	PDEBUG("executed.\n");

	me6000_workqueue = create_singlethread_workqueue("me6000");
	return 0;
}

static void __exit me6000_exit(void)
{
	PDEBUG("executed.\n");

	flush_workqueue(me6000_workqueue);
	destroy_workqueue(me6000_workqueue);
}

module_init(me6000_init);
module_exit(me6000_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-6xxx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-6xxx devices");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));
MODULE_FIRMWARE(ME6000_FIRMWARE);


// Export the constructors.
# if defined(ME_PCI)
EXPORT_SYMBOL(me6000_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me6000_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me6000_comedi_constr);
# endif

# if defined(ME_PCI)
EXPORT_SYMBOL(me6100_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me6100_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me6100_comedi_constr);
# endif

# if defined(ME_PCI)
EXPORT_SYMBOL(me6200_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me6200_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me6200_comedi_constr);
# endif

# if defined(ME_PCI)
EXPORT_SYMBOL(me6300_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me6300_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me6300_comedi_constr);
# endif
