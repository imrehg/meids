/**
 * @file me8100_device.c
 *
 * @brief The ME-8100 device class implementation.
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
# include "mehardware_access.h"
# include "me_interrupt_types.h"
# include "me_spin_lock.h"

# include "mesubdevice.h"
# include "me8100_di.h"
# include "me8100_do.h"
# include "me8254.h"

# include "me8100_device.h"

# define ME_MODULE_NAME		ME8100_NAME
# define ME_MODULE_VERSION	ME8100_VERSION


static void me8100_get_device_info(me8100_device_t* device, uint16_t device_id);

//Interrupt handler.
static irqreturn_t me8100_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

# if defined(ME_PCI)
me_device_t* me8100_pci_constr(
# elif defined(ME_USB)
me_device_t* me8100_usb_constr(
# elif defined(ME_COMEDI)
me_device_t* me8100_comedi_constr(
# endif
				me_general_dev_t* device, me_device_t* instance)
{
	me8100_device_t* me8100_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int i;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me8100_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me8100_device = kzalloc(sizeof(me8100_device_t), GFP_KERNEL);
		if (!me8100_device)
		{
			PERROR_CRITICAL("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me8100_device->dio_ctrl_reg_lock);
		ME_INIT_LOCK(&me8100_device->ctr_ctrl_reg_lock);
		ME_INIT_LOCK(&me8100_device->clk_src_reg_lock);

		// Set constans.
		me8100_get_device_info(me8100_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me8100_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		// Initialize irq context
		me8100_device->base.irq_context.no_subdev = me8100_versions[version_idx].di_subdevices;
		if(me8100_device->base.irq_context.no_subdev > 0)
		{
			me8100_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me8100_device->base.irq_context.no_subdev), GFP_KERNEL);
			if (!me8100_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me8100_device->base.irq_context.int_status = kmalloc(sizeof(me8100_interrupt_status_t), GFP_KERNEL);
			if (!me8100_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me8100_interrupt_status_t *)me8100_device->base.irq_context.int_status)->intcsr = me8100_device->base.bus.PCI_Base[1] + PLX9052_INTCSR;
			me8100_device->base.irq_context.me_device_irq_handle = me8100_isr;
		}

		/// Create subdevice instances.
		// DI
		for (i = 0; i < me8100_versions[version_idx].di_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8100_di_constr(
							me8100_device->base.bus.PCI_Base[2],
							i,
							&me8100_device->dio_ctrl_reg_lock,
							&me8100_device->dio_ctrl_reg_copy);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}

			me8100_device->base.irq_context.subdevice[i] = subdevice;
			me_slist_add(&me8100_device->base.slist, (void *)&me8100_device->base.bus.local_dev, subdevice);
		}

		// DO
		for (i = 0; i < me8100_versions[version_idx].do_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8100_do_constr(
							me8100_device->base.bus.PCI_Base[2],
							i,
							&me8100_device->dio_ctrl_reg_lock,
							&me8100_device->dio_ctrl_reg_copy);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me8100_device->base.slist, (void *)&me8100_device->base.bus.local_dev, subdevice);
		}

		// CTR (8254)
		for (i = 0; i < me8100_versions[version_idx].ctr_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me8254_constr(
							device->device,
							me8100_device->base.bus.PCI_Base[2],
							0,
							i,
							&me8100_device->ctr_ctrl_reg_lock,
							&me8100_device->clk_src_reg_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for CTR%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me8100_device->base.slist, (void *)&me8100_device->base.bus.local_dev, subdevice);
		}
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me8100_device = (me8100_device_t *)instance;
		}
		else
			return NULL;
	}

	// Init interrupts
	if (me_device_init_irq(&me8100_device->base))
	{
		goto ERROR;
	}

	// Enable interrupts on PLX
	me_writel(&me8100_device->base.bus.local_dev,
				PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_LOCAL_INT1_POL |
				PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_LOCAL_INT2_POL |
				PLX9052_INTCSR_PCI_INT_EN,
				me8100_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);

	return (me_device_t *) me8100_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me8100_device)
	{
		me_device_disconnect((me_device_t *)me8100_device);
		if (me8100_device->base.me_device_destructor)
		{
			me8100_device->base.me_device_destructor((me_device_t *)me8100_device);
		}
		kfree(me8100_device);
		me8100_device = NULL;
	}
	return NULL;
}

static void me8100_get_device_info(me8100_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME8100_A:
			device->base.info.device_name = 		ME8100_NAME_DEVICE_ME8100A;
			device->base.info.device_description = 	ME8100_DESCRIPTION_DEVICE_ME8100A;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME8100_B:
			device->base.info.device_name = 		ME8100_NAME_DEVICE_ME8100B;
			device->base.info.device_description = 	ME8100_DESCRIPTION_DEVICE_ME8100B;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me8100_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me8100_interrupt_status_t* interrupt_status;
	me8100_device_t* me8100_dev;
	me_subdevice_t* instance;

	irqreturn_t ret = IRQ_NONE;
	uint32_t irq_status_val = 0x00;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	me8100_dev = container_of((void *)irq_context, me8100_device_t, base.irq_context);
	interrupt_status = (me8100_interrupt_status_t *)irq_context->int_status;

	if (!irq_context->unhandled_irq)
	{
		PDEBUG("executed.\n");
	}

	me_readl(&me8100_dev->base.bus.local_dev, &irq_status_val, interrupt_status->intcsr);
	if ((irq_status_val & (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_PCI_INT_EN)) == (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_PCI_INT_EN))
	{//Interrupt line 1
		PINFO("IRQ line A.\n");
		instance = irq_context->subdevice[0];
		if(!instance->me_subdevice_irq_handle(instance, irq_status_val))
			ret = IRQ_HANDLED;
	}

	if (irq_context->no_subdev > 1)
	{
		if ((irq_status_val & (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_PCI_INT_EN)) == (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_PCI_INT_EN))
		{//Interrupt line 2
			PINFO("IRQ line B.\n");
			instance = irq_context->subdevice[1];
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
static int __init me8100_init(void)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_SUCCESS;
}

static void __exit me8100_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me8100_init);
module_exit(me8100_exit);

// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-81xx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-81xx Device");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));

// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me8100_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me8100_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me8100_comedi_constr);
# endif
