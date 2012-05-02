/**
 * @file me1400_device.c
 *
 * @brief ME-1400 device instance.
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

#ifndef MODULE
# define MODULE
#endif

#include "me_debug.h"
#include "me_error.h"
#include "me_common.h"
#include "me_internal.h"
#include "me_plx9052_reg.h"
#include "mehardware_access.h"
# include "me_spin_lock.h"

#include "mesubdevice.h"
#include "me1400AB_ext_irq.h"
#include "me1400CD_ext_irq.h"
#include "me8254.h"
#include "me8254_reg.h"
#include "me8255.h"

#include "me1400_device.h"

# define ME_MODULE_NAME		ME1400_NAME
# define ME_MODULE_VERSION	ME1400_VERSION

static void me1400_get_device_info(me1400_device_t* device, uint16_t device_id);
static irqreturn_t me1400_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							);

#if defined(ME_PCI)
me_device_t* me1400_pci_constr(
#elif defined(ME_USB)
me_device_t* me1400_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1400_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me1400_device_t *me1400_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;

	uint16_t me1400_id;
	int index, i;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Allocate structure for device instance.
		me1400_device = kzalloc(sizeof(me1400_device_t), GFP_KERNEL);
		if (!me1400_device)
		{
			PERROR_CRITICAL("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me1400_device->clk_src_reg_lock);
		for (i = 0; i < ME1400_MAX_8255; i++)
		{
			ME_INIT_LOCK(&me1400_device->dio_ctrl_reg_lock[i]);
		}
		for (i = 0; i < ME1400_MAX_8254; i++)
		{
			ME_INIT_LOCK(&me1400_device->ctr_ctrl_reg_lock[i]);
		}

		// Initialize base class structure.
		if (me_device_init(&me1400_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		me1400_id = device->device;

		/// Check for ME1400 extension device. If detected we have to set to fake a 'ME-1400 D' device id.
		if (me1400_id == PCI_DEVICE_ID_MEILHAUS_ME140C)
		{
			uint8_t tmp;
			me_readb(&me1400_device->base.bus.local_dev, &tmp, me1400_device->base.bus.PCI_Base[2] + ME1400D_CLK_SRC_2_REG);
			me_writeb(&me1400_device->base.bus.local_dev, tmp | 0xF0, me1400_device->base.bus.PCI_Base[2] + ME1400D_CLK_SRC_2_REG);
			me_readb(&me1400_device->base.bus.local_dev, &tmp, me1400_device->base.bus.PCI_Base[2] + ME1400D_CLK_SRC_2_REG);

			if ((tmp & 0xF0) == 0xF0)
			{
				PINFO("ME1400 D detected.\n");
				me1400_id = PCI_DEVICE_ID_MEILHAUS_ME140D;
			}
		}

		switch (me1400_id)
		{
			case PCI_DEVICE_ID_MEILHAUS_ME1400:
			case PCI_DEVICE_ID_MEILHAUS_ME140A:
			case PCI_DEVICE_ID_MEILHAUS_ME140B:
			case PCI_DEVICE_ID_MEILHAUS_ME14E0:
			case PCI_DEVICE_ID_MEILHAUS_ME14EA:
			case PCI_DEVICE_ID_MEILHAUS_ME14EB:
			case PCI_DEVICE_ID_MEILHAUS_ME140C:
			case PCI_DEVICE_ID_MEILHAUS_ME140D:
				break;

			default:
				PERROR("Board not supported. ID:0x%04x\n", me1400_id);
				goto ERROR;
		}

		// Get the index from the device version information table.
		version_idx = me1400_versions_get_device_index(me1400_id);

		// Set constans.
		me1400_get_device_info(me1400_device, me1400_id);

		// Initialize irq context
		me1400_device->base.irq_context.no_subdev = me1400_versions[version_idx].ext_irq_subdevices;
		if (me1400_device->base.irq_context.no_subdev > 0)
		{
			me1400_device->base.irq_context.subdevice = kzalloc(sizeof(me_subdevice_t*) * (me1400_device->base.irq_context.no_subdev), GFP_KERNEL);
			if (!me1400_device->base.irq_context.subdevice)
			{
					PERROR_CRITICAL("Cannot get memory for subdevices' handlers in interrupt context.\n");
					goto ERROR;
			}

			me1400_device->base.irq_context.int_status = kmalloc(sizeof(me1400_interrupt_status_t), GFP_KERNEL);
			if (!me1400_device->base.irq_context.int_status)
			{
					PERROR_CRITICAL("Cannot get memory for IRQ status registry in interrupt context.\n");
					goto ERROR;
			}
			((me1400_interrupt_status_t *)me1400_device->base.irq_context.int_status)->intcsr = me1400_device->base.bus.PCI_Base[1] + PLX9052_INTCSR;
			me1400_device->base.irq_context.me_device_irq_handle = me1400_isr;
		}

		/// Create subdevice instances.
		// DIO (8255)
		for (index = 0; index < me1400_versions[version_idx].dio_chips; index++)
		{
			for (i = 0; i < me1400_versions[version_idx].dio_subdevices; i++)
			{
				subdevice = (me_subdevice_t *) me8255_constr(
								me1400_id,
								me1400_device->base.bus.PCI_Base[2],
								index,
								i,
								&me1400_device->dio_output_value[index],
								&me1400_device->dio_current_mode[index],
								&me1400_device->dio_ctrl_reg_lock[index]);

				if (!subdevice)
				{
					PERROR("Cannot get memory for TTL%d subdevice.\n", i);
					goto ERROR;
				}
				me_slist_add(&me1400_device->base.slist, (void *)&me1400_device->base.bus.local_dev, subdevice);
			}
		}

		// CTR (8254)
		for (index = 0; index < me1400_versions[version_idx].ctr_chips; index++)
		{
			for (i = 0; i < me1400_versions[version_idx].ctr_subdevices; i++)
			{
				subdevice = (me_subdevice_t *) me8254_constr(
								me1400_id,
								me1400_device->base.bus.PCI_Base[2],
								index,
								i,
								&me1400_device->ctr_ctrl_reg_lock[index],
								&me1400_device->clk_src_reg_lock);

				if (!subdevice)
				{
					PERROR("Cannot get memory for CTR%d subdevice.\n", i);
					goto ERROR;
				}
				me_slist_add(&me1400_device->base.slist, (void *)&me1400_device->base.bus.local_dev, subdevice);
			}
		}

		// Create ext_irq subdevices
		for (i = 0; i < me1400_versions[version_idx].ext_irq_subdevices; i++)
		{
			switch (me1400_id)
			{
				case PCI_DEVICE_ID_MEILHAUS_ME140C:
				case PCI_DEVICE_ID_MEILHAUS_ME140D:
					subdevice = (me_subdevice_t *) me1400CD_ext_irq_constr(me1400_device->base.bus.PCI_Base[2], i, &me1400_device->clk_src_reg_lock);
					break;

				default:
					subdevice = (me_subdevice_t *) me1400AB_ext_irq_constr(me1400_device->base.bus.PCI_Base[2], i);
			}

			if (!subdevice)
			{
					PERROR("Cannot get memory for EXT_IRQ%d subdevice.\n", i);
					goto ERROR;
			}
			me1400_device->base.irq_context.subdevice[i] = subdevice;
			me_slist_add(&me1400_device->base.slist, (void *)&me1400_device->base.bus.local_dev, subdevice);
		}
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me1400_device = (me1400_device_t *)instance;
		}
		else
			return NULL;

		me1400_id = me1400_device->base.bus.local_dev.device;
	}

	// Init interrupts
	if (me_device_init_irq(&me1400_device->base))
	{
		goto ERROR;
	}

	if (me1400_device->base.irq_context.no_subdev > 0)
	{
		// Enable interrupts on PLX
			switch (me1400_id)
			{
				case PCI_DEVICE_ID_MEILHAUS_ME140D:
					me_writel(&me1400_device->base.bus.local_dev,ME_PLX9052_PCI_ACTIVATE, me1400_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
					break;

				default:
					me_writel(&me1400_device->base.bus.local_dev,ME_PLX9052_PCI_ACTIVATE_INT1, me1400_device->base.bus.PCI_Base[1] + PLX9052_INTCSR);
			}
	}

	return (me_device_t *) me1400_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me1400_device)
	{
		me_device_disconnect((me_device_t *)me1400_device);
		if (me1400_device->base.me_device_destructor)
		{
			me1400_device->base.me_device_destructor((me_device_t *)me1400_device);
		}
		kfree(me1400_device);
		me1400_device = NULL;
	}

	return NULL;
}

static void me1400_get_device_info(me1400_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME1400:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140A:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400A;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400A;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140B:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400B;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400B;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME14E0:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400E;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400E;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME14EA:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400EA;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400EA;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME14EB:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400EB;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400EB;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140C:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400C;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400C;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140D:
			device->base.info.device_name =			ME1400_NAME_DEVICE_ME1400D;
			device->base.info.device_description =	ME1400_DESCRIPTION_DEVICE_ME1400D;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

//Interrupt handler.
static irqreturn_t me1400_isr(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
							, struct pt_regs* regs
#  endif
 							)
{
	me1400_interrupt_status_t* interrupt_status;
	me1400_device_t* me1400_dev;
	me_subdevice_t* instance;

	irqreturn_t ret = IRQ_NONE;
	uint32_t status = 0;

	me_irq_context_t* irq_context;

	irq_context = (me_irq_context_t *) context;

	if (irq_context->no_subdev <= 0)
		return ret;

	me1400_dev = container_of((void *)irq_context, me1400_device_t, base.irq_context);
	interrupt_status = (me1400_interrupt_status_t *)irq_context->int_status;

	PDEBUG("executed.\n");

	me_readl(&me1400_dev->base.bus.local_dev, &status, interrupt_status->intcsr);
	if ((status & (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN)) == (PLX9052_INTCSR_LOCAL_INT1_STATE | PLX9052_INTCSR_LOCAL_INT1_EN))
	{//Interrupt line (first line available)
		PINFO("IRQ line A.\n");
		instance = irq_context->subdevice[0];
		if(!instance->me_subdevice_irq_handle(instance, status))
			ret = IRQ_HANDLED;
	}
	if ((status & (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN)) == (PLX9052_INTCSR_LOCAL_INT2_STATE | PLX9052_INTCSR_LOCAL_INT2_EN))
	{//Interrupt line (second line available)
		PINFO("IRQ line B.\n");
		instance = irq_context->subdevice[1];
		if(!instance->me_subdevice_irq_handle(instance, status))
			ret = IRQ_HANDLED;
	}

	return ret;
}

// Init and exit of module.
static int __init me1400_init(void)
{
	PDEBUG("executed.\n");
	return 0;
}

static void __exit me1400_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me1400_init);
module_exit(me1400_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-14xx Devices");
MODULE_DESCRIPTION("Device Driver Module for Meilhaus ME-14xx devices.");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));

// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me1400_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me1400_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me1400_comedi_constr);
#endif
