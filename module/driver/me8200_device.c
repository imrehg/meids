/**
 * @file me8200_device.c
 *
 * @brief The ME-8200 device class implementation.
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

# include <linux/module.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_common.h"
# include "me_internal.h"
# include "me_plx9052_reg.h"
# include "me_interrupt_types.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "mesubdevice.h"
# include "me8200_reg.h"
# include "me8200_di.h"
# include "me8200_do.h"
# include "me8200_dio.h"

# include "me8200_device.h"

# define ME_MODULE_NAME		ME8200_NAME
# define ME_MODULE_VERSION	ME8200_VERSION

static void me8200_get_device_info(me8200_device_t* device, uint16_t device_id);

//Interrupt handler.
static irqreturn_t me8200_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

#if defined(ME_PCI)
me_device_t* me8200_pci_constr(
#elif defined(ME_USB)
me_device_t* me8200_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me8200_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me8200_device_t* me8200_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int i;
	int interrupt_idx = 0;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me8200_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me8200_device = kzalloc(sizeof(me8200_device_t), GFP_KERNEL);
		if (!me8200_device)
		{
			PERROR("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me8200_device->irq_ctrl_lock);
		ME_INIT_LOCK(&me8200_device->irq_mode_lock);
		ME_INIT_LOCK(&me8200_device->dio_ctrl_lock);

		// Set constans.
		me8200_get_device_info(me8200_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me8200_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		// Initialize irq context
		me8200_device->base.irq_context.no_subdev = me8200_versions[version_idx].di_subdevices +  me8200_versions[version_idx].do_subdevices;
		if (me8200_device->base.irq_context.no_subdev > 0)
		{
			me8200_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me8200_device->base.irq_context.no_subdev), GFP_KERNEL);
			if (!me8200_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me8200_device->base.irq_context.int_status = kmalloc(sizeof(me8200_interrupt_status_t), GFP_KERNEL);
			if (!me8200_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me8200_interrupt_status_t *)me8200_device->base.irq_context.int_status)->intcsr = me8200_device->base.bus.PCI_Base[2] + ME8200_DO_IRQ_STATUS_REG;
			me8200_device->base.irq_context.me_device_irq_handle = me8200_isr;
		}

		/// Create subdevice instances.
		// DI
		for (i = 0; i < me8200_versions[version_idx].di_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8200_di_constr(
							device,
							me8200_device->base.bus.PCI_Base[2],
							i,
							&me8200_device->irq_ctrl_lock,
							&me8200_device->irq_mode_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}
			me8200_device->base.irq_context.subdevice[interrupt_idx] = subdevice;
			interrupt_idx++;
			me_slist_add(&me8200_device->base.slist, (void *)&me8200_device->base.bus.local_dev, subdevice);
		}

		// DO
		for (i = 0; i < me8200_versions[version_idx].do_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8200_do_constr(
							device,
							me8200_device->base.bus.PCI_Base[2],
							i,
							&me8200_device->irq_mode_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DO%d subdevice.\n", i);
				goto ERROR;
			}
			me8200_device->base.irq_context.subdevice[interrupt_idx] = subdevice;
			interrupt_idx++;
			me_slist_add(&me8200_device->base.slist, (void *)&me8200_device->base.bus.local_dev, subdevice);
		}

		// DIO
		for (i = 0; i < me8200_versions[version_idx].dio_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8200_dio_constr(
							me8200_device->base.bus.PCI_Base[2],
							i,
							&me8200_device->dio_ctrl_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me8200_device->base.slist, (void *)&me8200_device->base.bus.local_dev, subdevice);
		}
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me8200_device = (me8200_device_t *)instance;
		}
		else
			return NULL;
	}

	// Init interrupts
	if (me_device_init_irq(&me8200_device->base))
	{
		goto ERROR;
	}

	// Enable interrupts on PLX
	me_writel(&me8200_device->base.bus.local_dev,
				PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_LOCAL_INT1_POL |
				PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_LOCAL_INT2_POL |
				PLX9052_INTCSR_PCI_INT_EN,
				me8200_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);

	return (me_device_t *) me8200_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me8200_device)
	{
		me_device_disconnect((me_device_t *)me8200_device);
		if (me8200_device->base.me_device_destructor)
		{
			me8200_device->base.me_device_destructor((me_device_t *)me8200_device);
		}
		kfree(me8200_device);
		me8200_device = NULL;
	}
	return NULL;
}

static void me8200_get_device_info(me8200_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME8200_A:
			device->base.info.device_name = 		ME8200_NAME_DEVICE_ME8200A;
			device->base.info.device_description =	ME8200_DESCRIPTION_DEVICE_ME8200A;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME8200_B:
			device->base.info.device_name = 		ME8200_NAME_DEVICE_ME8200B;
			device->base.info.device_description =	ME8200_DESCRIPTION_DEVICE_ME8200B;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me8200_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me8200_interrupt_status_t* interrupt_status;
	me8200_device_t* me8200_dev;
	me_subdevice_t* instance;

	unsigned int version_idx;
	int input_index, output_index;

	irqreturn_t ret = IRQ_NONE;
	uint8_t irq_status_val = 0x00;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me8200_dev = container_of((void *)irq_context, me8200_device_t, base.irq_context);
	interrupt_status = (me8200_interrupt_status_t *)irq_context->int_status;

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	version_idx = me8200_versions_get_device_index(me8200_dev->base.bus.local_dev.device);

	//Checking DI interrupts
	for (input_index = 0; input_index < me8200_versions[version_idx].di_subdevices; input_index++)
	{
		instance = irq_context->subdevice[input_index];
		if(!instance->me_subdevice_irq_handle(instance, 0))
		{
			PINFO("DI IRQ port %d\n", input_index);
			ret = IRQ_HANDLED;
		}
	}

	//Checking DO interrupts
	me_readb(&me8200_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	for (output_index = 0; output_index < me8200_versions[version_idx].do_subdevices; output_index++)
	{
		if (irq_status_val & (ME8200_DO_IRQ_STATUS_BIT_ACTIVE << (output_index * ME8200_DO_IRQ_STATUS_SHIFT)))
		{
			PINFO("DO IRQ port %d\n", output_index);
			instance = irq_context->subdevice[output_index+input_index];
			if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
				ret = IRQ_HANDLED;
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
static int __init me8200_init(void)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_SUCCESS;
}

static void __exit me8200_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me8200_init);
module_exit(me8200_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-82xx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-82xx Devices");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));


// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me8200_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me8200_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me8200_comedi_constr);
#endif
