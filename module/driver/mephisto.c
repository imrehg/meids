/**
 * @file mephisto.c
 *
 * @brief MephistoScope module for Meilhaus Driver System (SynapseUSB).
 * @note Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# define __KERNEL__
#endif

#ifndef MODULE
# define MODULE
#endif

#ifndef ME_MEPHISTO
# error ME_MEPHISTO driver flag not defined!
#endif

# include <linux/module.h>
# include <linux/usb.h>

# include <linux/cdev.h>
# include <linux/kernel.h>

# include "me_common.h"
# include "me_internal.h"
# include "me_defines.h"
# include "melock_defines.h"
# include "me_error.h"
# include "me_debug.h"
# include "me_ioctl.h"

# include "medevice.h"
# include "memain_common.h"

# include "mephisto.h"
# include "mephisto_device.h"

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
static struct usb_driver me_usb_driver =
{
	.name = ME_NAME_DRIVER,
	.id_table = mephisto_usb_table,
	.probe = mephisto_probe,
	.disconnect = mephisto_disconnect
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


static int mephisto_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int err = ME_ERRNO_SUCCESS;
	mephisto_usb_device_t* dev;	/// The usb device.
	me_device_t* n_device = NULL;
	me_device_t* o_device = NULL;
	long unsigned int serial_no;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	char* tmp;
#endif

	PDEBUG("executed.\n");

	/// Allocate structures.
	dev = kzalloc(sizeof(mephisto_usb_device_t), GFP_KERNEL);
	if (!dev)
	{
		PERROR_CRITICAL("Can't get memory for device's instance.\n");
		err = -ENOMEM;
		goto ERROR_0;
	}

	/// Initialize USB lock.
	dev->usb_semaphore = kzalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!dev->usb_semaphore)
	{
		PERROR_CRITICAL("Can't get memory for usb lock.\n");
		err = -ENOMEM;
		goto ERROR_1;
	}
	init_MUTEX(dev->usb_semaphore);

	/// Initialize variables.
	dev->dev = usb_get_dev(interface_to_usbdev(interface));
	if(!dev->dev)
	{
		PERROR("Error while request for usb device.\n");
		err = -ENODEV;
		goto ERROR_2;
	}

	/// Initialize hardware
	usb_set_intfdata(interface, dev);

	/// Read serial number
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
	tmp = (dev->dev->serial + strlen(dev->dev->serial));
	serial_no = simple_strtoul(dev->dev->serial + 2, &tmp, 16);
#else
	if (strict_strtoul(dev->dev->serial + 2, 16, &serial_no))
	{
		serial_no = 0;
	}
#endif
	dev->serial_no = serial_no;

	/// Hardware init
	mephisto_endpoints_reset(dev);

/**
	Choice:
	a) New device connected.  Add to device list.
	b) Old device reconected. Refresh device structure.
*/
	o_device = find_device_on_list(dev, ME_PLUGGED_ANY);
	if(o_device)
	{
		PDEBUG("Old device.\n");
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


 	PINFO("CALLING %s constructor\n", "mephisto_constr");

	n_device = mephisto_constr(dev, o_device);
	if (!n_device)
	{
		PERROR("Executing '%s()' failed.\n", "mephisto_constr");
		err = -ENODEV;
		goto ERROR_3;
	}
	else if (!o_device)
	{
	 	PINFO("Adding new entry to device list.\n");
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


	return 0;

ERROR_3:
	usb_put_dev(interface_to_usbdev(interface));
ERROR_2:
	kfree(dev->usb_semaphore);
ERROR_1:
	kfree(dev);
ERROR_0:
	return err;
}

// Init and exit of module.
static int __init mephisto_init(void)
{
	int result = 0;
	dev_t dev = MKDEV(major, 0);

 	PDEBUG("executed.\n");

 	ME_INIT_LOCK(&me_lock);

	// Register usb driver. This will return 0 if the USB subsystem is not available.
	result = usb_register(&me_usb_driver);
	if (result < 0)
	{
		if(result == -ENODEV)
		{
			PERROR("No USB subsystem available.\n");
		}
		else
		{
			PERROR("Can't register usb driver.\n");
		}
		goto INIT_ERROR_1;
	}

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
		goto INIT_ERROR_3;
	}

	cdevp = cdev_alloc();
	if (!cdevp)
	{
		PERROR("Can't get character device structure.\n");
		result = -ENOMEM;
		goto INIT_ERROR_4;
	}

	cdevp->ops = &me_file_operations;
	cdevp->owner = THIS_MODULE;

	result = cdev_add(cdevp, dev, 1);
	if (result < 0)
	{
		PERROR("Cannot add character device structure.\n");
		goto INIT_ERROR_5;
	}

	PLOG("Loaded: %s version: 0x%08x\n", ME_NAME_DRIVER, ME_VERSION_DRIVER);

	memain_class = class_create(THIS_MODULE, ME_NAME_DRIVER);
	memain_dev = device_create(memain_class,
								NULL,
								dev,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
								NULL,
#endif
								ME_NAME_NODE);

	return 0;

INIT_ERROR_5:
	cdev_del(cdevp);

INIT_ERROR_4:
	unregister_chrdev_region(dev, 1);

INIT_ERROR_3:
	usb_deregister(&me_usb_driver);

INIT_ERROR_1:
	return result;
}

static void __exit mephisto_exit(void)
{
	dev_t dev = MKDEV(major, 0);

 	PDEBUG("executed.\n");

	// Remove all instances.
	clear_device_list();

	// Remove hotplug.
	device_destroy(memain_class, dev);
	memain_dev = NULL;
	class_destroy(memain_class);
	memain_class = NULL;

	// Remove all synapses
	usb_deregister(&me_usb_driver);

	// Free reservations
	unregister_chrdev_region(dev, 1);

	// Deregister driver.
	cdev_del(cdevp);
}

static void mephisto_disconnect(struct usb_interface *interface)
{
	me_device_t* device;
	mephisto_usb_device_t* dev;

	PDEBUG("executed.\n");

	dev = usb_get_intfdata(interface);
	if (!dev)
	{
		PERROR("Instance not found!\n");
		usb_set_intfdata(interface, NULL);
		usb_put_dev(interface_to_usbdev(interface));
		return;
	}


	PLOG("DISCONNECT: MephistoScope SERIAL NUMBER:0x%08x\n", dev->serial_no);

	device = find_device_on_list(dev, ME_PLUGGED_IN);
	if(device)
	{
		device->me_device_disconnect(device);
		// Mark as unplugged.
		device->bus.plugged = ME_PLUGGED_OUT;
		PDEBUG("Mark as unplugged.\n");
	}
	else
	{
		PERROR("Device not found!\n");
	}

	usb_set_intfdata(interface, NULL);

	kfree(dev);
	dev = NULL;

	usb_put_dev(interface_to_usbdev(interface));
}

module_init(mephisto_init);
module_exit(mephisto_exit);

// Administrative stuff for modinfo.
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_DESCRIPTION("MephistoScope module for Meilhaus Driver System.");

MODULE_SUPPORTED_DEVICE("Meilhaus MephistoScope.");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_VERSION_DRIVER));
