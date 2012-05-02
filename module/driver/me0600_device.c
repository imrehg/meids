/**
 * @file me0600_device.c
 *
 * @brief ME-630 device class implementation.
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

#include "me_debug.h"
#include "me_error.h"
#include "me_common.h"
#include "me_internal.h"
#include "me_plx9052_reg.h"
#include "mehardware_access.h"

#include "mesubdevice.h"
#include "me0600_relay.h"
#include "me0600_di.h"
#include "me0600_dio.h"
#include "me0600_ext_irq.h"

#include "me0600_device.h"

# define ME_MODULE_NAME			ME0600_NAME
# define ME_MODULE_VERSION		ME0600_VERSION

static void me0600_get_device_info(me0600_device_t* device, uint16_t device_id);

//Interrupt handler.
static irqreturn_t me0600_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

#if defined(ME_PCI)
me_device_t* me0600_pci_constr(
#elif defined(ME_USB)
me_device_t* me0600_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me0600_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me0600_device_t* me0600_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int i;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me0600_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me0600_device = kzalloc(sizeof(me0600_device_t), GFP_KERNEL);
		if (!me0600_device)
		{
			PERROR_CRITICAL("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me0600_device->dio_ctrl_reg_lock);
		ME_INIT_LOCK(&me0600_device->intcsr_lock);

		// Set constans.
		me0600_get_device_info(me0600_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me0600_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		// Initialize irq context
		me0600_device->base.irq_context.no_subdev = me0600_versions[version_idx].ext_irq_subdevices;
		if(me0600_device->base.irq_context.no_subdev > 0)
		{
			me0600_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me0600_device->base.irq_context.no_subdev), GFP_KERNEL);
			if (!me0600_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me0600_device->base.irq_context.int_status = kmalloc(sizeof(me0600_interrupt_status_t), GFP_KERNEL);
			if (!me0600_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me0600_interrupt_status_t *)me0600_device->base.irq_context.int_status)->intcsr = me0600_device->base.bus.PCI_Base[1] + PLX9052_INTCSR;
			me0600_device->base.irq_context.me_device_irq_handle = me0600_isr;
		}

		/// Create subdevice instances.
		// digital inputs
		for (i = 0; i < me0600_versions[version_idx].di_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0600_di_constr(me0600_device->base.bus.PCI_Base[2], i);
			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me0600_device->base.slist, (void *)&me0600_device->base.bus.local_dev, subdevice);
		}

		// RELAY outputs
		for (i = 0; i < me0600_versions[version_idx].relay_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0600_relay_constr(me0600_device->base.bus.PCI_Base[2], i);
			if (!subdevice)
			{
				PERROR("Cannot get memory for RELAY%d subdevice.\n", i);
				goto ERROR;

			}
			me_slist_add(&me0600_device->base.slist, (void *)&me0600_device->base.bus.local_dev, subdevice);
		}

		//TTL bidirectional port
		for (i = 0; i < me0600_versions[version_idx].dio_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0600_dio_constr(me0600_device->base.bus.PCI_Base[2], i, &me0600_device->dio_ctrl_reg_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me0600_device->base.slist, (void *)&me0600_device->base.bus.local_dev, subdevice);
		}

		// EXT_IRQ
		for (i = 0; i < me0600_versions[version_idx].ext_irq_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0600_ext_irq_constr(
							me0600_device->base.bus.PCI_Base[1],
							me0600_device->base.bus.PCI_Base[2],
							i,
							&me0600_device->intcsr_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for EXT_IRQ%d subdevice.\n", i);
				goto ERROR;
			}
			me0600_device->base.irq_context.subdevice[i] = subdevice;
			me_slist_add(&me0600_device->base.slist, (void *)&me0600_device->base.bus.local_dev, subdevice);
		}
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me0600_device = (me0600_device_t *)instance;
		}
		else
			return NULL;
	}

	// Init interrupts
	if (me_device_init_irq(&me0600_device->base))
	{
		goto ERROR;
	}

	return (me_device_t *) me0600_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s.\n", ME_MODULE_NAME);
	if(me0600_device)
	{
		me_device_disconnect((me_device_t *)me0600_device);
		if (me0600_device->base.me_device_destructor)
		{
			me0600_device->base.me_device_destructor((me_device_t *)me0600_device);
		}
		kfree(me0600_device);
		me0600_device = NULL;
	}
	return NULL;
}

static void me0600_get_device_info(me0600_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME0630:
			device->base.info.device_name =			ME0600_NAME_DEVICE_ME0630;
			device->base.info.device_description =	ME0600_DESCRIPTION_DEVICE_ME0630;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me0600_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me0600_interrupt_status_t* interrupt_status;
	me0600_device_t* me0600_dev;
	me_subdevice_t* instance;

	irqreturn_t ret = IRQ_NONE;
	uint32_t status = 0;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me0600_dev = container_of((void *)irq_context, me0600_device_t, base.irq_context);
	interrupt_status = (me0600_interrupt_status_t *)irq_context->int_status;

	PDEBUG("executed.\n");

	me_readl(&me0600_dev->base.bus.local_dev, &status, interrupt_status->intcsr);
	if ((status & (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_PCI_INT_EN)) == (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_PCI_INT_EN))
	{//Interrupt line 1
		PINFO("IRQ line A.\n");
		instance = irq_context->subdevice[0];
		if(!instance->me_subdevice_irq_handle(instance, status))
			ret = IRQ_HANDLED;
	}

	if ((status & (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_PCI_INT_EN)) == (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_PCI_INT_EN))
	{//Interrupt line 2
		PINFO("IRQ line B.\n");
		instance = irq_context->subdevice[1];
			if(!instance->me_subdevice_irq_handle(instance, status))
				ret = IRQ_HANDLED;
	}

	if (ret == IRQ_NONE)
	{
		PDEBUG("IRQ triggered by ghost! irq_status_val=0x%08x\n", status);
	}

	return ret;
}

// Init and exit of module.
static int __init me0600_init(void)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_SUCCESS;
}

static void __exit me0600_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me0600_init);
module_exit(me0600_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-6xx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-6xx Device");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));

// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me0600_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me0600_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me0600_comedi_constr);
#endif
