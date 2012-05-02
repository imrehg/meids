/**
 * @file me6000_device.h
 *
 * @brief The ME-6000 device class.
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

# ifndef _ME6000_DEVICE_H
#  define _ME6000_DEVICE_H

#  include "medevice.h"

#  define ME6000_VERSION				0x00020201
#  define ME6000_MEMORY_MAP_MARKER		0x0080


/**
 * @brief Structure holding ME-6000 device capabilities.
 */
typedef struct //me6000_version
{
	uint16_t device_id;
	unsigned int dio_subdevices;
	unsigned int ao_subdevices;
	unsigned int ao_fifo;	//How many devices have FIFO
} me6000_version_t;

/**
 * @brief ME-6000 device capabilities.
 */
static me6000_version_t me6000_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME6004, 0,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6008, 0,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME600F, 0, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6014, 0,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6018, 0,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME601F, 0, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6034, 0,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6038, 0,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME603F, 0, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6104, 0,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6108, 0,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME610F, 0, 16 , 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6114, 0,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6118, 0,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME611F, 0, 16 , 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6134, 0,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6138, 0,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME613F, 0, 16 , 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6044, 2,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6048, 2,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME604F, 2, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6054, 2,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6058, 2,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME605F, 2, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6074, 2,  4 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6078, 2,  8 , 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME607F, 2, 16 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6144, 2,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6148, 2,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME614F, 2, 16 , 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6154, 2,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6158, 2,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME615F, 2, 16 , 4 },
// 	{ PCI_DEVICE_ID_MEILHAUS_ME615F, 2, 8 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6174, 2,  4 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME6178, 2,  8 , 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME617F, 2, 16 , 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6259, 2, 9 , 0 },

	{ PCI_DEVICE_ID_MEILHAUS_ME6359, 2, 9 , 4 },

	{ 0 },
};

/// Returns the number of entries in me6000_versions.
#define ME6000_DEVICE_VERSIONS (sizeof(me6000_versions) / sizeof(me6000_version_t) - 1)


/**
 * @brief Returns the index of the device entry in me6000_versions.
 *
 * @param device_id The PCI device id of the device to query.
 * @return The index of the device in me6000_versions.
 */
static inline unsigned int me6000_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME6000_DEVICE_VERSIONS; ++i)
		if((device_id &  ~ME6000_MEMORY_MAP_MARKER) == me6000_versions[i].device_id)
			break;
	return i;
}


/**
 * @brief The ME-6000 device class structure.
 */
typedef struct //me6000_device
{
	/// The Meilhaus device base class.
	me_device_t base;


	/// Child class attributes.

	// Lockal locks.
	me_lock_t preload_reg_lock;
	me_lock_t dio_ctrl_reg_lock;

	uint32_t preload_flags;			// Used in conjunction with preload_reg_lock.
	uint32_t triggering_flags;
} me6000_device_t;


/**
 * @brief The ME-6000 device class constructor.
 *
*  @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-6000 device instance. \n
 *         NULL on error.
 */

#if defined(ME_PCI)
me_device_t* me6000_pci_constr(
#elif defined(ME_USB)
me_device_t* me6000_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6000_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

#if defined(ME_PCI)
me_device_t* me6100_pci_constr(
#elif defined(ME_USB)
me_device_t* me6100_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6100_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

#if defined(ME_PCI)
me_device_t* me6200_pci_constr(
#elif defined(ME_USB)
me_device_t* me6200_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6200_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

#if defined(ME_PCI)
me_device_t* me6300_pci_constr(
#elif defined(ME_USB)
me_device_t* me6300_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me6300_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));


#endif
#endif
