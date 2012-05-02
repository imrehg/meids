/**
 * @file me4600_device.h
 *
 * @brief ME-4600 device class header file.
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

# ifndef _ME4600_DEVICE_H_
#  define _ME4600_DEVICE_H_

#  include <linux/pci.h>

#  include "medevice.h"

#  include "me4600_ai.h"

#  define ME4600_VERSION	0x00020201

# define ME4600_FIRMWARE	"me4600.bin"
# define ME4610_FIRMWARE	"me4610.bin"
# define ME4800_FIRMWARE	"me4800.bin"


/**
 * @brief Structure holding ME-4600 device capabilities.
 */
typedef struct //me4600_version
{
	uint16_t device_id;
	unsigned int do_subdevices;
	unsigned int di_subdevices;
	unsigned int dio_subdevices;
	unsigned int ctr_subdevices;
	unsigned int ai_subdevices;
	unsigned int ai_channels;
	unsigned int ai_ranges;
	unsigned int ai_features;	//0x0001 - S&H; 0x0002 - analog trigger; 0x0004 - differential measurment; 0x0008 - isolated
	unsigned int ao_subdevices;
	unsigned int ao_fifo;	//How many devices have FIFO
	unsigned int ext_irq_subdevices;
} me4600_version_t;

/**
 * @brief ME-4600 device capabilities.
 */
static me4600_version_t me4600_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME4610,	0, 0, 4, 3, 1, 16, 1, (ME4600_NONE), 0, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4650,	0, 0, 4, 0, 1, 16, 4, (ME4600_NONE), 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4650I,	1, 1, 2, 0, 1, 16, 4, (ME4600_ISOLATED), 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4650S,	0, 0, 4, 0, 1, 16, 4, (ME4600_SAMPLE_HOLD), 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4650IS,	1, 1, 2, 0, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 0, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4660,	0, 0, 4, 3, 1, 16, 4, (ME4600_NONE), 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4660I,	1, 1, 2, 3, 1, 16, 4, (ME4600_ISOLATED), 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4660S,	0, 0, 4, 3, 1, 16, 4, (ME4600_SAMPLE_HOLD), 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4660IS,	1, 1, 2, 3, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 2, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4670,	0, 0, 4, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4670I,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4670S,	0, 0, 4, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4670IS,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4680,	0, 0, 4, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4680I,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4680S,	0, 0, 4, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4680IS,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4850,	0, 0, 4, 0, 1, 16, 4, 0, 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4850I,	1, 1, 2, 0, 1, 16, 4, (ME4600_ISOLATED), 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4850S,	0, 0, 4, 0, 1, 16, 4, (ME4600_SAMPLE_HOLD), 0, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4850IS,	1, 1, 2, 0, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 0, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4860,	0, 0, 4, 3, 1, 16, 4, 0, 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4860I,	1, 1, 2, 3, 1, 16, 4, (ME4600_ISOLATED), 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4860S,	0, 0, 4, 3, 1, 16, 4, (ME4600_SAMPLE_HOLD), 2, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4860IS,	1, 1, 2, 3, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 2, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4870,	0, 0, 4, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4870I,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4870S,	0, 0, 4, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4870IS,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4880,	0, 0, 4, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4880I,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4880S,	0, 0, 4, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4880IS,	1, 1, 2, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1 },

	{ 0 },
};

/// Returns the number of entries in me4600_versions.
#define ME4600_DEVICE_VERSIONS (sizeof(me4600_versions) / sizeof(me4600_version_t) - 1)


/**
 * @brief Returns the index of the device entry in me4600_versions.
 *
 * @param device_id The PCI device id of the device to query.
 * @return The index of the device in me4600_versions.
 */
static inline unsigned int me4600_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME4600_DEVICE_VERSIONS; i++)
		if(me4600_versions[i].device_id == device_id)
			break;
	return i;
}


/**
 * @brief The ME-4600 (ME-4800) device class structure.
 */
typedef struct //me4600_device
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
} me4600_device_t;


/**
 * @brief The ME-4600 (ME-4800) device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-4600 (ME-4800) device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me4600_pci_constr(
#elif defined(ME_USB)
me_device_t* me4600_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me4600_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

#if defined(ME_PCI)
me_device_t* me4800_pci_constr(
#elif defined(ME_USB)
me_device_t* me4800_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me4800_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

# endif
#endif
