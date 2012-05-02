/**
 * @file me1600_device.c
 *
 * @brief The ME-1600 device class implementation.
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
#include "mehardware_access.h"
#include "me_spin_lock.h"

#include "medevice.h"
#include "mesubdevice.h"
#include "me1600_ao.h"

#include "me1600_device.h"

# define ME_MODULE_NAME		ME1600_NAME
# define ME_MODULE_VERSION	ME1600_VERSION

static void me1600_set_registers(me1600_device_t* subdevice, void* reg_base);
static void me1600_destructor(struct me_device* device);
static void me1600_get_device_info(me1600_device_t* device, uint16_t device_id);

/**
 * @brief Global variable.
 * This is working queue for runing a separate atask that will be responsible for work status (start, stop, timeouts).
 */
static 	struct workqueue_struct* me1600_workqueue;

#if defined(ME_PCI)
me_device_t* me1600_pci_constr(
#elif defined(ME_USB)
me_device_t* me1600_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1600_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me1600_device_t* me1600_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int i;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the number of analog output subdevices.
		version_idx = me1600_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me1600_device = kzalloc(sizeof(me1600_device_t), GFP_KERNEL);
		if (!me1600_device)
		{
			PERROR_CRITICAL("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me1600_device->config_regs_lock);
		ME_INIT_LOCK(&me1600_device->ao_shadows_lock);

		// Set constans.
		me1600_get_device_info(me1600_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me1600_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		// Create shadow instance.
		me1600_device->ao_regs_shadows.count = me1600_versions[version_idx].ao_chips;
		me1600_device->ao_regs_shadows.registers = kmalloc(me1600_versions[version_idx].ao_chips * sizeof(void *), GFP_KERNEL);
		if (!me1600_device->ao_regs_shadows.registers)
		{
			PERROR_CRITICAL("Cannot get memory for registers' addresses.\n");
			goto ERROR;
		}
		me1600_set_registers(me1600_device, me1600_device->base.bus.PCI_Base[2]);
		me1600_device->ao_regs_shadows.shadow = kmalloc(me1600_versions[version_idx].ao_chips * sizeof(uint16_t), GFP_KERNEL);
		if (!me1600_device->ao_regs_shadows.shadow)
		{
			PERROR_CRITICAL("Cannot get memory for shadow registers.\n");
			goto ERROR;
		}
		me1600_device->ao_regs_shadows.mirror = kmalloc(me1600_versions[version_idx].ao_chips * sizeof(uint16_t), GFP_KERNEL);
		if (!me1600_device->ao_regs_shadows.mirror)
		{
			PERROR_CRITICAL("Cannot get memory for shadow registers' mirrors.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// Analog outputs
		for (i = 0; i < me1600_versions[version_idx].ao_chips; i++)
		{
			subdevice = (me_subdevice_t *) me1600_ao_constr(
							me1600_device->base.bus.PCI_Base[2],
							i,
							&me1600_device->config_regs_lock,
							&me1600_device->ao_shadows_lock,
							&me1600_device->ao_regs_shadows,
							((me1600_versions[version_idx].curr > i) ? 1:0),
							me1600_workqueue);

			if (!subdevice)
			{
				kfree(me1600_device);
				PERROR("Cannot get memory for subdevice.\n");
				return NULL;
			}
			me_slist_add(&me1600_device->base.slist, (void *)&me1600_device->base.bus.local_dev, subdevice);
		}

		// Overwrite base class methods. - Needed for cleaning shadows.
		me1600_device->base.me_device_destructor = me1600_destructor;
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me1600_device = (me1600_device_t *)instance;
		}
		else
			return NULL;
	}

	return (me_device_t *) me1600_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me1600_device)
	{
		me_device_disconnect((me_device_t *)me1600_device);
		if (me1600_device->base.me_device_destructor)
		{
			me1600_device->base.me_device_destructor((me_device_t *)me1600_device);
		}

		if(me1600_device->ao_regs_shadows.registers)
		{
			kfree(me1600_device->ao_regs_shadows.registers);
			me1600_device->ao_regs_shadows.registers = NULL;
		}
		if(me1600_device->ao_regs_shadows.shadow)
		{
			kfree(me1600_device->ao_regs_shadows.shadow);
			me1600_device->ao_regs_shadows.shadow = NULL;
		}
		if(me1600_device->ao_regs_shadows.mirror)
		{
			kfree(me1600_device->ao_regs_shadows.mirror);
			me1600_device->ao_regs_shadows.mirror = NULL;
		}

		kfree(me1600_device);
		me1600_device = NULL;
	}
	return NULL;
}

static void me1600_destructor(struct me_device* device)
{
	me1600_device_t* me1600_device = (me1600_device_t *)device;
	PDEBUG("executed.\n");

	// Destroy shadow instance.
	if(me1600_device->ao_regs_shadows.registers)
	{
		kfree(me1600_device->ao_regs_shadows.registers);
		me1600_device->ao_regs_shadows.registers = NULL;
	}
	if(me1600_device->ao_regs_shadows.shadow)
	{
		kfree(me1600_device->ao_regs_shadows.shadow);
		me1600_device->ao_regs_shadows.shadow = NULL;
	}
	if(me1600_device->ao_regs_shadows.mirror)
	{
		kfree(me1600_device->ao_regs_shadows.mirror);
		me1600_device->ao_regs_shadows.mirror = NULL;
	}

// 	me_device_deinit((me_device_t *) me1600_device);
}

static void me1600_set_registers(me1600_device_t* subdevice, void* reg_base)
{// Create shadow structure.
	if (subdevice->ao_regs_shadows.count >= 1)
	{
		subdevice->ao_regs_shadows.registers[0] = reg_base + ME1600_CHANNEL_0_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 2)
	{
		subdevice->ao_regs_shadows.registers[1] = reg_base + ME1600_CHANNEL_1_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 3)
	{
		subdevice->ao_regs_shadows.registers[2] = reg_base + ME1600_CHANNEL_2_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 4)
	{
		subdevice->ao_regs_shadows.registers[3] = reg_base + ME1600_CHANNEL_3_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 5)
	{
		subdevice->ao_regs_shadows.registers[4] = reg_base + ME1600_CHANNEL_4_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 6)
	{
		subdevice->ao_regs_shadows.registers[5] = reg_base + ME1600_CHANNEL_5_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 7)
	{
		subdevice->ao_regs_shadows.registers[6] = reg_base + ME1600_CHANNEL_6_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 8)
	{
		subdevice->ao_regs_shadows.registers[7] = reg_base + ME1600_CHANNEL_7_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 9)
	{
		subdevice->ao_regs_shadows.registers[8] = reg_base + ME1600_CHANNEL_8_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 10)
	{
		subdevice->ao_regs_shadows.registers[9] = reg_base + ME1600_CHANNEL_9_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 11)
	{
		subdevice->ao_regs_shadows.registers[10] = reg_base + ME1600_CHANNEL_10_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 12)
	{
		subdevice->ao_regs_shadows.registers[11] = reg_base + ME1600_CHANNEL_11_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 13)
	{
		subdevice->ao_regs_shadows.registers[12] = reg_base + ME1600_CHANNEL_12_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 14)
	{
		subdevice->ao_regs_shadows.registers[13] = reg_base + ME1600_CHANNEL_13_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 15)
	{
		subdevice->ao_regs_shadows.registers[14] = reg_base + ME1600_CHANNEL_14_REG;
	}
	if (subdevice->ao_regs_shadows.count >= 16)
	{
		subdevice->ao_regs_shadows.registers[15] = reg_base + ME1600_CHANNEL_15_REG;
	}
	if (subdevice->ao_regs_shadows.count > 16)
	{
		PERROR("More than 16 outputs! (%d)\n", subdevice->ao_regs_shadows.count);
	}
}

static void me1600_get_device_info(me1600_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME1600_4U:
			device->base.info.device_name =			ME1600_NAME_DEVICE_ME16004U;
			device->base.info.device_description =	ME1600_DESCRIPTION_DEVICE_ME16004U;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME1600_8U:
			device->base.info.device_name =			ME1600_NAME_DEVICE_ME16008U;
			device->base.info.device_description =	ME1600_DESCRIPTION_DEVICE_ME16008U;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME1600_12U:
			device->base.info.device_name =			ME1600_NAME_DEVICE_ME160012U;
			device->base.info.device_description =	ME1600_DESCRIPTION_DEVICE_ME160012U;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME1600_16U:
			device->base.info.device_name =			ME1600_NAME_DEVICE_ME160016U;
			device->base.info.device_description =	ME1600_DESCRIPTION_DEVICE_ME160016U;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME1600_16U_8I:
			device->base.info.device_name =			ME1600_NAME_DEVICE_ME160016U8I;
			device->base.info.device_description =	ME1600_DESCRIPTION_DEVICE_ME160016U8I;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

// Init and exit of module.
static int __init me1600_init(void)
{
	PDEBUG("executed\n.");

	me1600_workqueue = create_singlethread_workqueue("me1600");
	return 0;
}

static void __exit me1600_exit(void)
{
	PDEBUG("executed\n.");

	flush_workqueue(me1600_workqueue);
	destroy_workqueue(me1600_workqueue);
}

module_init(me1600_init);
module_exit(me1600_exit);

// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-16xx Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-16xx devices.");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));

// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me1600_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me1600_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me1600_comedi_constr);
#endif
