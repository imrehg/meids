/**
 * @file me1000_device.c
 *
 * @brief ME-1000 device class implementation.
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

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "me_debug.h"
#include "me_error.h"
#include "me_common.h"
#include "me_internal.h"
#include "mehardware_access.h"
# include "me_spin_lock.h"

#include "medevice.h"
#include "mesubdevice.h"

#include "me_data_types.h"
#include "me1000_dio.h"

#include "me1000_device.h"

# define ME_MODULE_NAME		ME1000_NAME
# define ME_MODULE_VERSION	ME1000_VERSION

static void me1000_get_device_info(me1000_device_t* device, uint16_t device_id);
static int me1000_config_load(me_device_t *me_device, struct file* filep, void* config, unsigned int size);

static int me1000_config_load(me_device_t *me_device, struct file* filep, void* config, unsigned int size)
{
	me1000_device_t* me1000_device;
	me1000_config_load_t me1000_config;
	me_subdevice_t* subdevice;
	int err = ME_ERRNO_SUCCESS;
	int no_subdev;
	int i;

	PDEBUG("executed.\n");

	me1000_device = (me1000_device_t *) me_device;


	if (!config)
	{
		return ME_ERRNO_INVALID_POINTER;
	}

	if (size != sizeof(me1000_config_load_t))
	{
		PERROR("Invalid configuration size. Must be %ld [sizeof(me1000_config_load_t)].\n", (long int)sizeof(me1000_config_load_t));
		return ME_ERRNO_INTERNAL;
	}
	else
	{
		err = copy_from_user(&me1000_config, config, size);
		if (err)
		{
			PERROR("Can't copy config entry structure to kernel space.\n");
			return -EFAULT;
		}

		no_subdev = me_slist_get_number_subdevices(&me1000_device->base.slist);

		if (no_subdev < me1000_config.subdevice_no)
		{
			for (i=no_subdev; i < me1000_config.subdevice_no; i++)
			{
				// Add more subdevices
				subdevice = (me_subdevice_t *) me1000_dio_constr(me1000_device->base.bus.PCI_Base[2], i, &me1000_device->ctrl_lock);
				if (!subdevice)
				{
					PERROR("Cannot create dio subdevice.\n");
					err = ME_ERRNO_INTERNAL;
				}
				me_slist_add(&me1000_device->base.slist, (void *)&me1000_device->base.bus.local_dev, subdevice);
			}
		}
		else if (no_subdev > me1000_config.subdevice_no)
		{
			for (i=no_subdev; i < me1000_config.subdevice_no; i++)
			{
				// Remove extra subdevices
				subdevice = me_slist_del_subdevice_tail(&me1000_device->base.slist);
				if (subdevice)
					subdevice->me_subdevice_destructor(subdevice);
			}
		}
		else
		{
			PDEBUG("No need to change config.\n");
		}
	}

	return err;
}


#if defined(ME_PCI)
me_device_t* me1000_pci_constr(
#elif defined(ME_USB)
me_device_t* me1000_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1000_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)
{
	me1000_device_t *me1000_device;
	me_subdevice_t* subdevice;
	int err;
	int i;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Allocate structure for device instance.
		me1000_device = kzalloc(sizeof(me1000_device_t), GFP_KERNEL);
		if (!me1000_device)
		{
			PERROR("Cannot get memory for ME-1000 device instance.\n");
			return NULL;
		}

		// Initialize spin lock .
		ME_INIT_LOCK(&me1000_device->ctrl_lock);

		// Set constans.
		me1000_get_device_info(me1000_device, device ->device);

		// Initialize base class structure.
		err = me_device_init(&me1000_device->base, device );
		if (err)
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// DIO
		for (i = 0; i < ME1000_DEFAULT_SUBDEVICE_COUNT; i++)
		{
			subdevice = (me_subdevice_t *) me1000_dio_constr(
							me1000_device->base.bus.PCI_Base[2],
							i,
							&me1000_device->ctrl_lock);

			if (!subdevice)
			{
				PERROR("Cannot get memory for DIO%d subdevice.\n", i);
				goto ERROR;
			}
			me_slist_add(&me1000_device->base.slist, (void *)&me1000_device->base.bus.local_dev, subdevice);
		}

		// Overwrite base class methods.
		me1000_device->base.me_device_config_load = me1000_config_load;
	}
	else
	{
		if (!me_device_reinit(instance, device))
		{
			me1000_device = (me1000_device_t *)instance;
		}
		else
			return NULL;
	}


	return (me_device_t *) me1000_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(me1000_device)
	{
		me_device_disconnect((me_device_t *)me1000_device);
		if (me1000_device->base.me_device_destructor)
		{
			me1000_device->base.me_device_destructor((me_device_t *)me1000_device);
		}
		kfree(me1000_device);
		me1000_device = NULL;
	}
	return NULL;
}

static void me1000_get_device_info(me1000_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =				ME_MODULE_VERSION;
	device->base.info.driver_name =					ME_MODULE_NAME;

	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME1000:
		case PCI_DEVICE_ID_MEILHAUS_ME1000_A:
		case PCI_DEVICE_ID_MEILHAUS_ME1000_B:
			device->base.info.device_name =			ME1000_NAME_DEVICE_ME1000;
			device->base.info.device_description =	ME1000_DESCRIPTION_DEVICE_ME1000;
			break;

		default:
			device->base.info.device_name =			EMPTY_NAME_DEVICE;
			device->base.info.device_description =	EMPTY_DESCRIPTION_DEVICE;
	}
}

// Init and exit of module.
static int __init me1000_init(void)
{
	PDEBUG("executed.\n");
	return 0;
}

static void __exit me1000_exit(void)
{
	PDEBUG("executed.\n");
}

module_init(me1000_init);
module_exit(me1000_exit);

// Administrative stuff for modinfo.
MODULE_SUPPORTED_DEVICE("Meilhaus ME-1000 Digital I/O Devices");
MODULE_DESCRIPTION("Device Driver Module for Meilhaus ME-1000 device.");
MODULE_AUTHOR("Guenter Gebhardt");
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_MODULE_VERSION));


// Export the constructor.
# if defined(ME_PCI)
EXPORT_SYMBOL(me1000_pci_constr);
# elif defined(ME_USB)
EXPORT_SYMBOL(me1000_usb_constr);
# elif defined(ME_COMEDI)
EXPORT_SYMBOL(me1000_comedi_constr);
# endif
