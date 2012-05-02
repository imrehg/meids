/**
 * @file me1000_device.h
 *
 * @brief ME-1000 device class header file.
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

# ifdef __KERNEL__

# if !defined(ME_PCI) && !defined(ME_USB) && !defined(ME_COMEDI)
#  error NO VALID DRIVER TYPE declared!
#  define ME_PCI
# endif

# ifndef _ME1000_H_
#  define _ME1000_H_

#  include "medevice.h"

#  define ME1000_VERSION	0x00020201

#  define ME1000_DEFAULT_SUBDEVICE_COUNT	4

/**
 * @brief The ME-1000 device class structure.
 */
typedef struct //me1000_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child class attributes.

	// Lockal locks.
	me_lock_t ctrl_lock;
} me1000_device_t;


/**
 * @brief The ME-1000 device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-1000 device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me1000_pci_constr(
#elif defined(ME_USB)
me_device_t* me1000_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1000_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance);

# endif
#endif
