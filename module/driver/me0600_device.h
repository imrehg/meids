/**
 * @file me0600_device.h
 *
 * @brief The ME-0600 (ME-630) device class header file.
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

#ifdef __KERNEL__

# if !defined(ME_PCI) && !defined(ME_USB) && !defined(ME_COMEDI)
#  error NO VALID DRIVER TYPE declared!
#  define ME_PCI
# endif

# ifndef _ME0600_DEVICE_H
#  define _ME0600_DEVICE_H

#  include "me_internal.h"

#  include "medevice.h"

#  define ME0600_VERSION	0x00020201


/**
 * @brief Structure holding ME-630 device capabilities.
 */
typedef struct //me0600_version
{
	uint16_t device_id;
	unsigned int relay_subdevices;
	unsigned int di_subdevices;
	unsigned int dio_subdevices;
	unsigned int ext_irq_subdevices;
} me0600_version_t;

/**
 * @brief Device capabilities.
 */
static me0600_version_t me0600_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME0630, 1, 2, 2, 2},
	{ 0 },
};

/// Returns the number of entries in me0600_versions.
#  define ME0600_DEVICE_VERSIONS (sizeof(me0600_versions) / sizeof(me0600_version_t) - 1)


/**
 * @brief Returns the index of the device entry in me0600_versions.
 *
 * @param device_id PCI_PRODUCT_ID of the device to query.
 *
 * @return The index of the device in me0600_versions.
 */
static inline unsigned int me0600_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME0600_DEVICE_VERSIONS; i++)
		if(me0600_versions[i].device_id == device_id)
			break;
	return i;
}

/**
 * @brief The ME-630 device class structure.
 */
typedef struct //me0600_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child attributes.

	/// Lockal locks.
	me_lock_t dio_ctrl_reg_lock;
	me_lock_t intcsr_lock;
} me0600_device_t;

/**
 * @brief The ME-630 device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a pointer to ME630 device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me0600_pci_constr(
#elif defined(ME_USB)
me_device_t* me0600_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me0600_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

# endif
#endif
