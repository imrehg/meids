/**
 * @file me0900_device.h
 *
 * @brief The ME-0900 (ME-9x) device class.
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

# ifndef _ME0900_DEVICE_H
#  define _ME0900_DEVICE_H

#  include "medevice.h"

#  define ME0900_VERSION	0x00020201

/**
 * @brief Structure holding ME-0900 (ME-9x) device capabilities.
 */
typedef struct //me0900_version
{
	uint16_t device_id;
	unsigned int di_subdevices;
	unsigned int do_subdevices;
} me0900_version_t;


/**
 * @brief Device capabilities.
 */
static me0900_version_t me0900_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME0940 , 2, 0},
	{ PCI_DEVICE_ID_MEILHAUS_ME0950 , 0, 2},
	{ PCI_DEVICE_ID_MEILHAUS_ME0960 , 1, 1},
	{ 0 },
};

/// Returns the number of entries in me0900_versions.
#define ME0900_DEVICE_VERSIONS (sizeof(me0900_versions) / sizeof(me0900_version_t) - 1)


/**
 * @brief Returns the index of the device entry in #me0900_versions.
 *
 * @param device_id The PCI device id of the device to query.
 * @return The index of the device in me0900_versions.
 */
static inline unsigned int me0900_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME0900_DEVICE_VERSIONS; ++i)
		if(me0900_versions[i].device_id == device_id)
			break;
	return i;
}


/**
 * @brief The ME-0900 (ME-9x) device class structure.
 */
typedef struct //me0900_device
{
	/// The Meilhaus device base class.
	me_device_t base;
} me0900_device_t;


/**
 * @brief The ME-9x device class constructor.
 *
 * @param pci_device The pci device structure given by the PCI subsystem.
 *
 * @return On succes a new ME-0900 (ME-9x) device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me0900_pci_constr(
#elif defined(ME_USB)
me_device_t* me0900_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me0900_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance);

 #endif
#endif
