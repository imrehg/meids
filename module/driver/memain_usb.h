/**
 * @file memain_usb.h
 *
 * @brief Header for loader module for Meilhaus Driver System (SynapseUSB).
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifdef __KERNEL__

# ifndef _MEMAIN_H_
#  define _MEMAIN_H_

#  include "me_internal.h"
#  include "memain_common.h"
#  include "NET2282_access.h"

/**
* USB_VENDOR - macro used to describe a specific usb vendor
* @vend: the 16 bit USB Vendor ID
*
* This macro is used to create a struct usb_device_id that matches a specific vendor.
*/
#  define USB_VENDOR(vend) \
        .match_flags = USB_DEVICE_ID_MATCH_VENDOR, .idVendor = (vend)

	static struct usb_device_id me_usb_table[] __devinitdata =
	{
		{ USB_VENDOR(MEILHAUS_USB_VENDOR_ID) },
		{ 0 }
	};

	MODULE_DEVICE_TABLE(usb, me_usb_table);

	static int me_probe_usb(struct usb_interface *interface, const struct usb_device_id *id);
	static void me_disconnect_usb(struct usb_interface *interface);
	static int me_usb_hardware_check(struct NET2282_usb_device* dev);
	static int me_usb_board_check(struct NET2282_usb_device* dev);

# endif
#endif	//__KERNEL__
