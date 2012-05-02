/**
 * @file me1400_device.h
 *
 * @brief The ME-1400 device class header file.
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

# ifndef _ME1400_DEVICE_H_
#  define _ME1400_DEVICE_H_

#  include "me_internal.h"

#  include "medevice.h"

#  define ME1400_VERSION	0x00020201


/**
 * @brief Structure to store device capabilities.
 */
typedef struct //me1400_version
{
	uint16_t device_id;					/**< The PCI device id of the device. */
	unsigned int dio_chips;				/**< The number of 8255 chips on the device. */
	unsigned int dio_subdevices;		/**< The number of subdevices on one 8255 chip. */
	unsigned int ctr_chips;				/**< The number of 8254 chips on the device. */
	unsigned int ctr_subdevices;		/**< The number of subdevices on one 8254 chip. */
	unsigned int ext_irq_subdevices;	/**< The number of external interrupt inputs on the device. */
} me1400_version_t;

/**
  * @brief Defines for each ME-1400 device version its capabilities.
 */
static me1400_version_t me1400_versions[] =
{
	{ PCI_DEVICE_ID_MEILHAUS_ME1400, 1, 3,  0, 0, 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME140A, 1, 3,  1, 3, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME140B, 2, 3,  2, 3, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME14E0, 1, 3,  0, 0, 0 },
	{ PCI_DEVICE_ID_MEILHAUS_ME14EA, 1, 3,  1, 3, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME14EB, 2, 3,  2, 3, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME140C, 1, 3,  5, 3, 1 },
	{ PCI_DEVICE_ID_MEILHAUS_ME140D, 2, 3, 10, 3, 2 },
	{ 0 }
};
#define ME1400_DEVICE_VERSIONS (sizeof(me1400_versions) / sizeof(me1400_version_t) - 1) /**< Returns the number of entries in me1400_versions. */


/**
 * @brief Returns the index of the device entry in #me1400_versions.
 *
 * @param device_id The device id of the device to query.
 * @return The index of the device in me1400_versions.
 */
static inline unsigned int me1400_versions_get_device_index(uint16_t device_id)
{
	unsigned int i;
	for(i = 0; i < ME1400_DEVICE_VERSIONS; i++)
		if(me1400_versions[i].device_id == device_id) break;
	return i;
}


#define ME1400_MAX_8254		10	/**< The maximum number of 8254 counter subdevices available on any ME-1400 device. */
#define ME1400_MAX_8255		2	/**< The maximum number of 8255 digital i/o subdevices available on any ME-1400 device. */


/**
 * @brief The ME-1400 device class.
 */
typedef struct //me1400_device
{
	/// The Meilhaus device base class.
	me_device_t base;

	/// Child attributes.
    me_lock_t clk_src_reg_lock;							/**< Guards the 8254 clock source registers. */
    me_lock_t ctr_ctrl_reg_lock[ME1400_MAX_8254];		/**< Guards the 8254 ctrl registers. */

	uint32_t dio_output_value[ME1400_MAX_8255];			/**< Saves the output values of a single 8255 DIO chip. */
	int dio_current_mode[ME1400_MAX_8255];				/**< Saves the current mode setting of a single 8255 DIO chip. */
    me_lock_t dio_ctrl_reg_lock[ME1400_MAX_8255];		/**< Guards the 8255 registers, dio_output_value and dio_current_mode. */
} me1400_device_t;


/**
 * @brief The ME-1400 device class constructor.
 *
 * @param device   The unified device structure given by the local bus subsystem.
 * @param instance The instance to init. NULL for new device, pointer for existing one.
 *
 * @return On succes a new ME-1400 device instance. \n
 *         NULL on error.
 */
#if defined(ME_PCI)
me_device_t* me1400_pci_constr(
#elif defined(ME_USB)
me_device_t* me1400_usb_constr(
#elif defined(ME_COMEDI)
me_device_t* me1400_comedi_constr(
#endif
				me_general_dev_t* device, me_device_t* instance);

# endif
#endif
