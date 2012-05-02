/**
 * @file me0700_device.h
 *
 * @brief ME-0700 device class header file.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# ifndef _ME0700_DEVICE_H_
#  define _ME0700_DEVICE_H_

#  include <linux/pci.h>

#  include "medevice.h"

#  include "me0700_ai.h"

#  define ME0700_VERSION	0x00020203


/**
 * @brief Structure holding ME-0700 device capabilities.
 */
typedef struct //me0700_version
{
	uint16_t device_id;
	unsigned int dio_subdevices;
	unsigned int ctr_subdevices;
	unsigned int ai_subdevices;
	unsigned int ai_channels;
	unsigned int ai_ranges;
	unsigned int ai_features;	//0x0001 - S&H; 0x0002 - analog trigger; 0x0004 - differential measurment; 0x0008 - isolated
	unsigned int ao_subdevices;
	unsigned int ao_fifo;	//How many devices have FIFO
	unsigned int ai_current_channels;
} me0700_version_t;

/**
 * @brief ME-0700 device capabilities.
 */
static me0700_version_t me0700_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME0752,	2, 0, 1, 16, 4, 0, 0, 0, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME0754,	2, 0, 1, 16, 4, 0, 0, 0, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME0762,	2, 3, 1, 16, 4, 0, 2, 0, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME0764,	2, 3, 1, 16, 4, 0, 2, 0, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME0772,	2, 3, 1, 32, 4, ME0700_ANALOG_TRIGGER, 4, 0, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME0774,	2, 3, 1, 32, 4, ME0700_ANALOG_TRIGGER, 4, 0, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME0782,	2, 3, 1, 32, 4, ME0700_ANALOG_TRIGGER, 4, 4, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME0784,	2, 3, 1, 32, 4, ME0700_ANALOG_TRIGGER, 4, 4, 4 },

	{ 0 },
};

/// Returns the number of entries in me0700_versions.
#define ME0700_DEVICE_VERSIONS (sizeof(me0700_versions) / sizeof(me0700_version_t) - 1)


/**
 * @brief Returns the index of the device entry in me0700_versions.
 *
 * @param device_id The PCI device id of the device to query.
 * @return The index of the device in me0700_versions.
 */
static inline unsigned int me0700_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME0700_DEVICE_VERSIONS; i++)
		if(me0700_versions[i].device_id == device_id)
			break;
	return i;
}


/**
 * @brief The ME-0700 device class structure.
 */
typedef struct //me0700_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child class attributes.

	// Local locks.
	me_lock_t preload_reg_lock;		// Guards the preload register of the anaolog output devices.
	me_lock_t dio_lock;				// Locks the control register of the digital input/output subdevices.
	me_lock_t ctr_ctrl_reg_lock;		// Locks the counter control register.
	me_lock_t ctr_clk_src_reg_lock;	// Not used on this device but needed for the me8254 subdevice constructor call.

	uint32_t preload_flags;				// Used in conjunction with preload_reg_lock.
} me0700_device_t;


/**
 * @brief The ME-0700 device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-4600 device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me0700_pci_constr(
#elif defined(ME_USB)
me_device_t* me0700_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me0700_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

# endif
#endif
