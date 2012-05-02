/**
 * @file memain_usb.c
 *
 * @brief Loader module for Meilhaus Driver System (SynapseUSB).
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
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

#ifdef ME_PCI
# error PCI driver defined!
#endif

#ifndef ME_USB
# error USB driver not defined!
#endif

#ifdef BOSCH
# warning BOSCH build is available only for PCI!
# undef BOSCH
#endif //BOSCH

# include <linux/module.h>
# include <linux/usb.h>

# include <linux/cdev.h>

# include "me_common.h"
# include "me_internal.h"
# include "me_defines.h"
# include "melock_defines.h"
# include "me_error.h"
# include "me_debug.h"
# include "me_ioctl.h"

# include "medevice.h"

# include "memain_common.h"
# include "memain_usb.h"

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
	.id_table = me_usb_table,
	.probe = me_probe_usb,
	.disconnect = me_disconnect_usb
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


static int me_probe_usb(struct usb_interface *interface, const struct usb_device_id *id)
{
	int err = ME_ERRNO_SUCCESS;
	struct NET2282_usb_device* dev;					/// The usb device.

	me_constr_t constructor = NULL;
	me_device_t* n_device = NULL;
	me_device_t* o_device = NULL;

	char constructor_name[20]="me0000_usb_constr\0";
	char module_name[16]="me0000USB\0";

 	PDEBUG("executed.\n");

	/// Allocate structures.
	dev = kzalloc(sizeof(struct NET2282_usb_device), GFP_KERNEL);
	if (!dev)
	{
		PERROR_CRITICAL("Can't get memory for device's instance.\n");
		err = -ENOMEM;
		goto ERROR_0;
	}

	/// Initialize USB lock.
	dev->usb_DMA_semaphore = kzalloc(sizeof(struct semaphore), GFP_KERNEL);
	if (!dev->usb_DMA_semaphore)
	{
		PERROR_CRITICAL("Can't get memory for usb DMA lock.\n");
		err = -ENOMEM;
		goto ERROR_1;
	}
	init_MUTEX(dev->usb_DMA_semaphore);

	/// Initialize USB transfer status.
	atomic_set(&dev->usb_transfer_status, 0);

	/// Initialize variables.
	dev->dev = usb_get_dev(interface_to_usbdev(interface));
	if(!dev->dev)
	{
		PERROR("Error while request for usb device.\n");
		err = -ENODEV;
		goto ERROR_2;
	}

	/// Initialize hardware
	if (me_usb_hardware_check(dev))
	{
		PERROR("NOT SUPPORTED! This is not SynapseUSB.\n");
		err = -ENODEV;
		goto ERROR_3;
	}

	/// Check this board
	if (me_usb_board_check(dev))
	{
		PERROR("NOT SUPPORTED! This is not Meilhaus board.\n");
		err = -ENODEV;
		goto ERROR_3;
	}

	err = NET2282_hardware_init(dev);
	if (!err)
	{//Hardware initialized.
		PDEBUG("Hardware initialized.\n");
		usb_set_intfdata(interface, dev);

		NET2282_ENDPOINTS_reset(dev);

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

	}
	else
	{
		PERROR("Cann't start SynapseUSB.\n");
		goto ERROR_3;
	}

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
				goto ERROR_3;
			}
		}
		else
		{
			PERROR("Can't get requested module: %s.\n", module_name);
			err = -ENODEV;
			goto ERROR_3;
		}
	}

 	PINFO("CALLING %s constructor\n", constructor_name);

	n_device = (*constructor)(dev, o_device);
	if (!n_device)
	{
		PERROR("Executing '%s()' failed.\n", constructor_name);
		err = -ENODEV;
		goto ERROR_4;
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

ERROR_4:
	__symbol_put(constructor_name);
ERROR_3:
	usb_put_dev(interface_to_usbdev(interface));
ERROR_2:
	kfree(dev->usb_DMA_semaphore);
ERROR_1:
	kfree(dev);
ERROR_0:
	return err;
}

// Init and exit of module.
static int __init memain_usb_init(void)
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

static void __exit memain_usb_exit(void)
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

static void me_disconnect_usb(struct usb_interface *interface)
{
	me_device_t* device;
	struct NET2282_usb_device* dev;

 	PDEBUG("executed.\n");

	dev = usb_get_intfdata(interface);
	if (!dev)
	{
		PERROR("Instance not found!\n");
		usb_set_intfdata(interface, NULL);
		usb_put_dev(interface_to_usbdev(interface));
		return;
	}

 	PLOG("DISCONNECT:    PCI VENDOR ID:0x%04x    PCI DEVICE ID:0x%04x    SERIAL NUMBER:0x%08x\n", dev->vendor, dev->device, dev->serial_no);

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
	if (dev->usb_DMA_semaphore)
	{
		kfree(dev->usb_DMA_semaphore);
		dev->usb_DMA_semaphore=NULL;
	}

	kfree(dev);
	dev = NULL;

	usb_put_dev(interface_to_usbdev(interface));
}

static int me_usb_hardware_check(struct NET2282_usb_device* dev)
{
	uint32_t tmp;
 	PDEBUG("executed.\n");

	if (NET2282_NET2282_cfg_read(dev, &tmp, NET2282_PCIVENDID))
	{
		PDEBUG("Config read failed. ==> Not SynapseUSB.\n");
		//Doesn't work.==> Not SynapseUSB. Reject it!
		return -ENODEV;
	}

	PDEBUG("NET2282_PCIVENDID: 0x%04x\n", tmp & 0x00FFFF);
	PDEBUG("NET2282_PCIDEVID: 0x%04x\n", (tmp >> 16) & 0x00FFFF);

	//Checking if this is SynapseUSB (NET2282 chip).
	return (tmp == NET2282_PCI_VENDOR_ID)?0:-ENODEV;
}

static int me_usb_board_check(struct NET2282_usb_device* dev)
{
 	uint32_t tmp;
	PDEBUG("executed.\n");

	if (NET2282_PLX_cfg_read(dev, &dev->IDs, NET2282_PCIVENDID))
	{
		return -ENODEV;	//Doesn't work.==> Reject this board!
	}

	// Read serial number from NET2282 PCI section.
	if (NET2282_PLX_cfg_read(dev, &dev->serial_no, NET2282_PCISUBVID))
	{
		return -ENODEV;	//Doesn't work.==> Reject this board.
	}
	// Read revision number from NET2282 PCI section.
	if (NET2282_PLX_cfg_read(dev, &tmp, NET2282_PCIDEVREV))
	{
		return -ENODEV;	//Doesn't work.==> Reject this board.
	}
	dev->hw_revision = (uint8_t)tmp;

	PDEBUG("PCI VENDOR ID: 0x%04x\n", dev->vendor);
	PDEBUG("PCI DEVICE ID: 0x%04x\n", dev->device);
	PDEBUG("HW   REVISION: 0x%02x\n", dev->hw_revision);
	PDEBUG("SERIAL NUMBER: 0x%08x\n", dev->serial_no);

	//Checking if this is Meilhaus board.
	return (dev->vendor == MEILHAUS_PCI_VENDOR_ID)?0:-ENODEV;
}

module_init(memain_usb_init);
module_exit(memain_usb_exit);

// Administrative stuff for modinfo.
MODULE_AUTHOR("Krzysztof Gantzke (k.gantzke@meilhaus.de)");
MODULE_DESCRIPTION("Loader module for Meilhaus Driver System (SynapseUSB).");

MODULE_SUPPORTED_DEVICE("Meilhaus PCI/cPCI boards via SynapseUSB.");
MODULE_LICENSE("GPL");
MODULE_VERSION(__stringify(ME_VERSION_DRIVER));
