/**
 * @file me1600_device.h
 *
 * @brief The ME-1600 device class header file.
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

# ifndef _ME1600_H
#  define _ME1600_H


#  include <linux/pci.h>

#  include "medevice.h"
#  include "me1600_ao.h"
#  include "me1600_ao_reg.h"

#  define ME1600_VERSION	0x00020201


/**
 * @brief Structure to store device capabilities.
 */
typedef struct //me1600_version
{
	uint16_t device_id;
	unsigned int ao_chips;

	int curr;	// Flag to identify amounts of current output.
} me1600_version_t;


/**
  * @brief Defines for each ME-1600 device version its capabilities.
 */
static me1600_version_t me1600_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME1600_4U,     4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME1600_8U,     8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME1600_12U,    12 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME1600_16U,    16 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME1600_16U_8I, 16 , 8 },
	{ 0 }
};

/// Returns the number of entries in me1600_versions.
#  define ME1600_DEVICE_VERSIONS (sizeof(me1600_versions) / sizeof(me1600_version_t) - 1)

/**
 * @brief Returns the index of the device entry in #me1600_versions.
 *
 * @param device_id The device id of the device to query.
 * @return The index of the device in me1600_versions.
 */
static inline unsigned int me1600_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME1600_DEVICE_VERSIONS; i++)
		if(me1600_versions[i].device_id == device_id) break;
	return i;
}


/**
 * @brief The ME-1600 device class structure.
 */
typedef struct //me1600_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child class attributes.

	// Lockal locks.
	me_lock_t config_regs_lock;
	me_lock_t ao_shadows_lock;

	// Addresses and shadows of output's registers.
	me1600_ao_shadow_t ao_regs_shadows;
} me1600_device_t;


/**
 * @brief The ME-1600 device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-1600 device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me1600_pci_constr(
#elif defined(ME_USB)
me_device_t* me1600_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1600_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));


#endif
#endif
