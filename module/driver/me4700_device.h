/**
 * @file me4700_device.h
 *
 * @brief ME-4700 device class header file.
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

# ifndef _ME4700_DEVICE_H_
#  define _ME4700_DEVICE_H_

#  include <linux/pci.h>

#  include "medevice.h"

#  include "me4600_ai.h"
#  include "me4700_fo.h"
#  include "me4700_fio_reg.h"

#  define ME4700_VERSION	0x00020201

# define ME4500_FIRMWARE	"me4500.bin"
# define ME4700_FIRMWARE	"me4700.bin"


/**
 * @brief Structure holding ME-4700 (ME-4500) device capabilities.
 */
typedef struct //me4700_version
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
	unsigned int fi_subdevices;
	unsigned int fo_subdevices;
} me4700_version_t;

/**
 * @brief ME-4700 device capabilities.
 */
static me4700_version_t me4700_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME4550,	0, 0, 2, 0, 1, 16, 4, (ME4600_NONE), 0, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4550I,	1, 1, 0, 0, 1, 16, 4, (ME4600_ISOLATED), 0, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4550S,	0, 0, 2, 0, 1, 16, 4, (ME4600_SAMPLE_HOLD), 0, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4550IS,	1, 1, 0, 0, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 0, 0, 1, 2, 2 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4560,	0, 0, 2, 3, 1, 16, 4, (ME4600_NONE), 2, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4560I,	1, 1, 0, 3, 1, 16, 4, (ME4600_ISOLATED), 2, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4560S,	0, 0, 2, 3, 1, 16, 4, (ME4600_SAMPLE_HOLD), 2, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4560IS,	1, 1, 0, 3, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 2, 0, 1, 2, 2 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4570,	0, 0, 2, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4570I,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4570S,	0, 0, 2, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 2, 2 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4570IS,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 2, 2 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4750,	0, 0, 2, 0, 1, 16, 4, (ME4600_NONE), 0, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4750I,	1, 1, 0, 0, 1, 16, 4, (ME4600_ISOLATED), 0, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4750S,	0, 0, 2, 0, 1, 16, 4, (ME4600_SAMPLE_HOLD), 0, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4750IS,	1, 1, 0, 0, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 0, 0, 1, 4, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4760,	0, 0, 2, 3, 1, 16, 4, (ME4600_NONE), 2, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4760I,	1, 1, 0, 3, 1, 16, 4, (ME4600_ISOLATED), 2, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4760S,	0, 0, 2, 3, 1, 16, 4, (ME4600_SAMPLE_HOLD), 2, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4760IS,	1, 1, 0, 3, 1, 16, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD), 2, 0, 1, 4, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4770,	0, 0, 2, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4770I,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4770S,	0, 0, 2, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4770IS,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 0, 1, 4, 4 },

	{ PCI_DEVICE_ID_MEILHAUS_ME4780,	0, 0, 2, 3, 1, 32, 4, (ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4780I,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4780S,	0, 0, 2, 3, 1, 32, 4, (ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1, 4, 4 },
	{ PCI_DEVICE_ID_MEILHAUS_ME4780IS,	1, 1, 0, 3, 1, 32, 4, (ME4600_ISOLATED | ME4600_SAMPLE_HOLD | ME4600_ANALOG_TRIGGER | ME4600_DIFFERENTIAL), 4, 4, 1, 4, 4 },

	{ 0 },
};

/// Returns the number of entries in me4700_versions.
#define ME4700_DEVICE_VERSIONS (sizeof(me4700_versions) / sizeof(me4700_version_t) - 1)


/**
 * @brief Returns the index of the device entry in me4700_versions.
 *
 * @param device_id The PCI device id of the device to query.
 * @return The index of the device in me4700_versions.
 */
static inline unsigned int me4700_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME4700_DEVICE_VERSIONS; i++)
		if(me4700_versions[i].device_id == device_id)
			break;
	return i;
}


/**
 * @brief The ME-4700 (ME-4500) device class structure.
 */
typedef struct //me4700_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child class attributes.

	// Local locks.
	me_lock_t preload_reg_lock;		// Guards the preload register of the anaolog output devices.
	me_lock_t dio_fio_lock;			// Locks the control register of the digital and frequency subdevices.
	me_lock_t ctr_ctrl_reg_lock;		// Locks the counter control register.
	me_lock_t ctr_clk_src_reg_lock;	// Not used on this device but needed for the me8254 subdevice constructor call.

	uint32_t preload_flags;				// Used in conjunction with preload_reg_lock. (AO)

	me4700_fo_context_t fo_shared_context; // FREQ_O context
} me4700_device_t;


/**
 * @brief The ME-4700 (ME-4500) device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-4700 (ME-4500) device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me4500_pci_constr(
#elif defined(ME_USB)
me_device_t* me4500_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me4500_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

#if defined(ME_PCI)
me_device_t* me4700_pci_constr(
#elif defined(ME_USB)
me_device_t* me4700_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me4700_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance)__attribute__((weak));

# endif
#endif
