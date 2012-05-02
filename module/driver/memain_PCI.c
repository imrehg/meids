/**
 * @file memain_pci.c
 *
 * @brief Loader module for Meilhaus Driver System (PCI bus).
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

#ifndef ME_PCI
# error PCI driver not defined!
#endif

#ifdef ME_USB
# error USB driver defined!
#endif

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

#include <linux/module.h>
#include <linux/pci.h>

#include <linux/cdev.h>

# include "me_common.h"
# include "me_internal.h"
# include "me_defines.h"
# include "me_error.h"
# include "me_debug.h"
# include "me_ioctl.h"

#include "medevice.h"

#include "memain_common.h"
#include "memain_pci.h"

static int me_probe_pci(struct pci_dev* raw_dev, const struct pci_device_id* id);
static void me_remove_pci(struct pci_dev *raw_dev);
static int me_pci_board_check(struct pci_local_dev* dev, struct pci_dev* raw_dev);

///Globals
struct file* me_filep = NULL;
int me_count = 0;
me_lock_t me_lock;
DECLARE_RWSEM(me_rwsem);

// Board instances are kept in a global list.
LIST_HEAD(me_device_list);

/// Char device structure.
static struct cdev *cdevp;

/// File operations provided by the module
static struct file_operations me_file_operations =
{
	.owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	.ioctl = me_ioctl,
#else
	.unlocked_ioctl = me_ioctl,
#endif
	.open = me_open,
	.release = me_release,
};

struct pci_driver me_pci_driver =
{
	.name = ME_NAME_DRIVER,
	.id_table = me_pci_table,
	.probe = me_probe_pci,
	.remove = me_remove_pci
};

/// HOTPLUG (udev) support
static struct class* memain_class = NULL;
static struct device* memain_dev = NULL;

/// Module parameters
static unsigned int major = 0;
#ifdef module_param
module_param(major, int, S_IRUGO);
#else
MODULE_PARM(major, "i");
#endif

static int me_probe_pci(struct pci_dev* raw_dev, const struct pci_device_id* id)
{
	int err = ME_ERRNO_SUCCESS;
	struct pci_local_dev* dev;

	me_constr_t constructor = NULL;
	me_device_t* n_device = NULL;
	me_device_t* o_device = NULL;

	char constructor_name[20]="me0000_pci_constr\0";
	char module_name[16]="me0000PCI\0";

 	PDEBUG("executed.\n");

 	/// Allocate structures.
 	dev = kzalloc(sizeof(struct pci_local_dev), GFP_KERNEL);
	if (!dev)
	{
		PERROR("Can't get memory for device's instance.");
		err = -ENOMEM;
		goto ERROR;
	}

	/// Check this board
	if (me_pci_board_check(dev, raw_dev))
	{
		PERROR("NOT SUPPORTED! This is not Meilhaus board.\n");
		err = -ENODEV;
		goto ERROR;
	}

	constructor_name[2] += (char)((dev->device >> 12) & 0x000F);
	constructor_name[3] += (char)((dev->device >> 8) & 0x000F);
 	PDEBUG("constructor_name: %s\n", constructor_name);
	module_name[2] += (char)((dev->device >> 12) & 0x000F);
	module_name[3] += (char)((dev->device >> 8) & 0x000F);
	if (module_name[2] == '6')
	{// Exceptions: me61xx, me62xx, me63xx are handled by one driver.
		module_name[3] = '0';
	}
	if (module_name[2] == '4')
	{
		if (module_name[3] == '8')
		{// Exceptions: me46xx and me48xx are handled by one driver.
			module_name[3] = '6';
		}
		else if (module_name[3] == '5')
		{// Exceptions: me45xx and me47xx are handled by one driver.
			module_name[3] = '7';
		}
	}
 	PDEBUG("module_name: %s\n", module_name);

/**
	Choice:
	a) New device connected.  Add to device list.
	b) Old device reconected. Refresh device structure.
*/
	o_device = find_device_on_list(dev, ME_PLUGGED_ANY);
	if(o_device)
	{
		PDEBUG("Device already registred.\n");
		// Old device.
		if (o_device->bus.plugged == ME_PLUGGED_IN)
		{
			// Error device is already on list mark as active!
			PERROR("Device is already on list mark as active!\n");
			o_device->me_device_disconnect(o_device);
		}
	}
	else
	{
		PDEBUG("New device.\n");
	}
	constructor = (me_constr_t) __symbol_get(constructor_name);
	if (!constructor)
	{
		PDEBUG("request_module: '%s'.\n", module_name);
		if (!request_module("%s", module_name))
		{
			constructor = (me_constr_t) __symbol_get(constructor_name);
			if (!constructor)
			{
				if(o_device)
				{
					PERROR_CRITICAL("Module loaded. Failed to get %s driver module constructor!\n", module_name);
				}
				else
				{
					PERROR("Can't get %s driver module constructor.\n", module_name);
				}
				err = -ENODEV;
				goto ERROR;
			}
		}
		else
		{
			PERROR("Can't get requested module: %s.\n", module_name);
			err = -ENODEV;
			goto ERROR;
		}
	}

	n_device = (*constructor)(dev, o_device);
	if (!n_device)
	{
		PERROR("Executing '%s()' failed.\n", constructor_name);
		__symbol_put(constructor_name);
		err = -ENODEV;
		goto ERROR;
	}
	else if (!o_device)
	{
		insert_to_device_list(n_device);
	}

	if (n_device->me_device_postinit)
	{
		if (n_device->me_device_postinit(n_device, NULL))
		{
			PERROR("Error while calling me_device_postinit().\n");
			/// This error can be ignored.
		}
		else
		{
			PDEBUG("me_device_postinit() was sucessful.\n");
		}
	}
	else
	{
		PERROR("me_device_postinit() not registred!\n");
	}

ERROR:
	if (dev)
		kfree(dev);
	dev = NULL;

	return err;
}

#   include <asm/atomic.h>
/// Init and exit of module.
static int __init memain_pci_init(void)
{
	int result = 0;
	dev_t dev = MKDEV(major, 0);

 	PDEBUG("executed.\n");

 	ME_INIT_LOCK(&me_lock);

	// Register the character device.
	if (major)
	{
		result = register_chrdev_region(dev, 1, ME_NAME_DRIVER);
	}
	else
	{
		result = alloc_chrdev_region(&dev, 0, 1, ME_NAME_DRIVER);
		major = MAJOR(dev);
	}

	if (result < 0)
	{
		PERROR("Can't get major driver no.\n");
		goto INIT_ERROR_1;
	}

	cdevp = cdev_alloc();
	if (!cdevp)
	{
		PERROR("Can't get character device structure.\n");
		result = -ENOMEM;
		goto INIT_ERROR_2;
	}

	cdevp->ops = &me_file_operations;
	cdevp->owner = THIS_MODULE;

	result = cdev_add(cdevp, dev, 1);
	if (result < 0)
	{
		PERROR("Cannot add character device structure.\n");
		goto INIT_ERROR_3;
	}

	// Register pci driver. This will return 0 if the PCI subsystem is not available.
	result = pci_register_driver(&me_pci_driver);
	if (result < 0)
	{
		PERROR("Can't register pci driver.\n");
		goto INIT_ERROR_3;
	}

	PLOG("Loaded: %s version: %08x\n", ME_NAME_DRIVER, ME_VERSION_DRIVER);

	memain_class = class_create(THIS_MODULE, ME_NAME_DRIVER);
	memain_dev = device_create(memain_class,
								NULL,
								dev,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
								NULL,
#endif
								ME_NAME_NODE);

	return 0;

// INIT_ERROR_4:
// 	pci_unregister_driver(&me_pci_driver);

INIT_ERROR_3:
	cdev_del(cdevp);

INIT_ERROR_2:
	unregister_chrdev_region(dev, 1);

INIT_ERROR_1:
	return result;
}

static void __exit memain_pci_exit(void)
{
	dev_t dev = MKDEV(major, 0);

 	PDEBUG("executed.\n");

	// Remove all boards
	pci_unregister_driver(&me_pci_driver);

	// Remove all instances.
	clear_device_list();

	// Remove hotplug.
	device_destroy(memain_class, dev);
	memain_dev = NULL;
	class_destroy(memain_class);
	memain_class = NULL;

	// Free reservations
	unregister_chrdev_region(dev, 1);

	// Deregister driver.
	cdev_del(cdevp);
}

static void me_remove_pci(struct pci_dev* raw_dev)
{
	me_device_t* device;
	struct pci_local_dev* dev;

 	PDEBUG("executed.\n");

 	dev = kzalloc(sizeof(struct pci_local_dev), GFP_KERNEL);
	if (!dev)
	{
		PERROR_CRITICAL("Can't get memory for device's instance.\n");
		return;
	}

	dev->dev = raw_dev;
	dev->vendor = raw_dev->vendor;
	dev->device = raw_dev->device;
	dev->serial_no = (raw_dev->subsystem_device << 16) | raw_dev->subsystem_vendor;


 	PLOG("DISCONNECT:    PCI VENDOR ID:0x%04x    PCI DEVICE ID:0x%04x    SERIAL NUMBER:0x%08x\n", dev->vendor, dev->device, dev->serial_no);

	device = find_device_on_list(dev, ME_PLUGGED_IN);
	if(device)
	{
		device->me_device_disconnect(device);
		// Mark as unplugged.
		device->bus.plugged = ME_PLUGGED_OUT;
	}
	else
	{
		PERROR("Device not found!\n");
	}

	kfree(dev);
}

///Local functions
static int me_pci_board_check(struct pci_local_dev* dev, struct pci_dev* raw_dev)
{
	PDEBUG("executed.\n");

	dev->dev = raw_dev;
	// Get Meilhaus specific device information.
	dev->device = raw_dev->device;
	dev->vendor = raw_dev->vendor;

	// Read serial number
	pci_read_config_dword(raw_dev, PCI_SUBSYSTEM_VENDOR_ID, &dev->serial_no);
	// Read revision number
	pci_read_config_byte(raw_dev, PCI_CLASS_REVISION, &dev->hw_revision);

	PINFO("PCI VENDOR ID: 0x%04x\n", dev->vendor);
	PINFO("PCI DEVICE ID: 0x%04x\n", dev->device);
	PINFO("HW   REVISION: 0x%02x\n", dev->hw_revision);
	PINFO("SERIAL NUMBER: 0x%08x\n", dev->serial_no);

	//Checking if this is Meilhaus board.
	return (dev->vendor == MEILHAUS_PCI_VENDOR_ID)?0:-ENODEV;
}

module_init(memain_pci_init);
module_exit(memain_pci_exit);

// Administrative stuff for modinfo.
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de) & Guenter Gebhardt");
MODULE_DESCRIPTION("Loader module for Meilhaus Driver System (PCI bus).");
MODULE_SUPPORTED_DEVICE("Meilhaus PCI/cPCI boards.");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_VERSION_DRIVER));
