/**
 * @file mephisto.h
 *
 * @brief Header for loader module for Meilhaus Driver System (SynapseUSB).
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
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

#ifdef __KERNEL__

# ifndef _MEPHISTO_H_
#  define _MEPHISTO_H_

#  include "me_internal.h"
//#  include "memain_common.h"
// #  include "mephisto_access.h"

	static struct usb_device_id mephisto_usb_table[] __devinitdata =
	{
		{ USB_DEVICE(0x0403, 0xdcd0) },
		{ 0 }
	};

	MODULE_DEVICE_TABLE(usb, mephisto_usb_table);

	static int mephisto_probe(struct usb_interface *interface, const struct usb_device_id *id);
	static void mephisto_disconnect(struct usb_interface *interface);
// 	static int me_usb_hardware_check(struct NET2282_usb_device* dev);
// 	static int me_usb_board_check(struct NET2282_usb_device* dev);

# endif	//_MEPHISTO_H_
#endif	//__KERNEL__
