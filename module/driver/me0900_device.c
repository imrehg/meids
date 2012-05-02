/**
 * @file me0900_device.c
 *
 * @brief The ME-0900 (ME-9x) device class implementation.
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

#include "medevice.h"
#include "mesubdevice.h"
#include "me0900_device.h"
#include "me0900_reg.h"
#include "me0900_do.h"
#include "me0900_di.h"

# define ME_MODULE_NAME		ME0900_NAME
# define ME_MODULE_VERSION	ME0900_VERSION

static void me0900_get_device_info(me0900_device_t* device, uint16_t device_id);

#if defined(ME_PCI)
me_device_t* me0900_pci_constr(
#elif defined(ME_USB)
me_device_t* me0900_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me0900_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me0900_device_t* me0900_device;
	me_subdevice_t* subdevice;
	unsigned int version_idx;
	int i, port_shift;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Get the index from the device version information table.
		version_idx = me0900_versions_get_device_index(device->device);

		// Allocate structure for device instance.
		me0900_device = kzalloc(sizeof(me0900_device_t), GFP_KERNEL);
		if (!me0900_device)
		{
			PERROR_CRITICAL("Cannot get memory for device instance.\n");
			goto ERROR;
		}

		// Set constans.
		me0900_get_device_info(me0900_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&me0900_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		/// Initialize 8255 chip to desired mode.
		/// @todo Move this to subdevice reset.
		if (device->device == PCI_DEVICE_ID_MEILHAUS_ME0940)
		{
			me_writeb(&me0900_device->base.bus.local_dev, 0x9B, me0900_device->base.bus.PCI_Base[2] + ME0900_CTRL_REG);
		}
		else if (device->device == PCI_DEVICE_ID_MEILHAUS_ME0950)
		{
			me_writeb(&me0900_device->base.bus.local_dev, 0x89, me0900_device->base.bus.PCI_Base[2] + ME0900_CTRL_REG);
			me_writeb(&me0900_device->base.bus.local_dev, 0x00, me0900_device->base.bus.PCI_Base[2] + ME0900_WRITE_ENABLE_REG);
		}
		else if (device->device == PCI_DEVICE_ID_MEILHAUS_ME0960)
		{
			me_writeb(&me0900_device->base.bus.local_dev, 0x8B, me0900_device->base.bus.PCI_Base[2] + ME0900_CTRL_REG);
			me_writeb(&me0900_device->base.bus.local_dev, 0x00, me0900_device->base.bus.PCI_Base[2] + ME0900_WRITE_ENABLE_REG);
		}

		port_shift = (device->device == PCI_DEVICE_ID_MEILHAUS_ME0960) ? 1 : 0;

		/// Create subdevice instances.
		// DI
		for (i = 0; i < me0900_versions[version_idx].di_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0900_di_constr(me0900_device->base.bus.PCI_Base[2], i, port_shift);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DI%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me0900_device->base.slist, (void *)&me0900_device->base.bus.local_dev, subdevice);
		}

		// DO
		for (i = 0; i < me0900_versions[version_idx].do_subdevices; i++)
		{
			subdevice = (me_subdevice_t *) me0900_do_constr(me0900_device->base.bus.PCI_Base[2], i);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me0900_device->base.slist, (void *)&me0900_device->base.bus.local_dev, subdevice);
		}
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me0900_device = (me0900_device_t *)instance;
		}
		else
			return NULL;
	}

	return (me_device_t *) me0900_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me0900_device)
	{
		me_device_disconnect((me_device_t *)me0900_device);
		if (me0900_device->base.me_device_destructor)
		{
			me0900_device->base.me_device_destructor((me_device_t *)me0900_device);
		}
		kfree(me0900_device);
		me0900_device = NULL;
	}
	return NULL;
}

static void me0900_get_device_info(me0900_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME0940:
			device->base.info.device_name =			ME0900_NAME_DEVICE_ME0940;
			device->base.info.device_description =	ME0900_DESCRIPTION_DEVICE_ME0940;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0950:
			device->base.info.device_name =			ME0900_NAME_DEVICE_ME0950;
			device->base.info.device_description =	ME0900_DESCRIPTION_DEVICE_ME0950;
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME0960:
			device->base.info.device_name =			ME0900_NAME_DEVICE_ME0960;
			device->base.info.device_description =	ME0900_DESCRIPTION_DEVICE_ME0960;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

// Init and exit of module.
static int __init me0900_init(void)
{
	PDEBUG("executed.\n");

	return ME_ERRNO_SUCCESS;
}

static void __exit me0900_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me0900_init);
module_exit(me0900_exit);


// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-9x Devices");
MODULE_DESCRIPTION("Device Driver Module for ME-9x devices.");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));

// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me0900_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me0900_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me0900_comedi_constr);
#endif
