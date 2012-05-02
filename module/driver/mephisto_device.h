/**
 * @file mephisto_device.h
 *
 * @brief MephistoScope device class header file.
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

#ifdef __KERNEL__

# if !defined(ME_MEPHISTO)
#  error NO VALID DRIVER TYPE declared!
# endif

# ifndef _MEPHISTO_DEVICE_H_
#  define _MEPHISTO_DEVICE_H_

#  include "medevice.h"

#  define MEPHISTO_VERSION	0x00010104


/**
 * @brief The MephistoScope device class structure.
 */
typedef struct //mephisto_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child class attributes.
	struct semaphore 	device_semaphore;
	mephisto_AI_status_e AI_status;

} mephisto_device_t;


/**
 * @brief The MephistoScope device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new MephistoScope device instance. \n
 *         NULL on error.
 */
me_device_t* mephisto_constr(mephisto_usb_device_t* device, me_device_t* instance);

# endif	//_MEPHISTO_DEVICE_H_
#endif	//__KERNEL__
