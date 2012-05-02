/**
 * @file me8200_di.h
 *
 * @brief The ME-8200 digital input subdevice class.
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
# ifndef _ME8200_DI_H_
#  define _ME8200_DI_H_

#  include "medevice.h"
#  include "mesubdevice.h"

/**
 * @brief The ME-8200 digital input subdevice class.
 */
typedef struct //me8200_di_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t* irq_ctrl_lock;
	me_lock_t* irq_mode_lock;

	unsigned int	version;

	volatile int	rised;				/**< Flag to indicate if an interrupt occured. */
	unsigned int	status_flag;				/**< Default interupt status flag */
	unsigned int	status_value;				/**< Interupt status */
	unsigned int	status_value_edges;		/**< Extended interupt status */
	unsigned int	line_value;
	int 			count;						/**< Counts the number of interrupts occured. */
	uint8_t			compare_value;
	uint8_t			filtering_flag;

	wait_queue_head_t wait_queue;	/**< To wait on interrupts. */

	void* port_reg;					/**< The digital input port. */
	void* compare_reg;				/**< The register to hold the value to compare with. */
	void* mask_reg;					/**< The register to hold the mask. */
	void* irq_mode_reg;				/**< The interrupt mode register. */
	void* irq_ctrl_reg;				/**< The interrupt control register. */
	void* irq_status_reg;			/**< The interrupt status register. Also interrupt reseting register (firmware version 7 and later).*/
	void* firmware_version_reg;		/**< The interrupt reseting register. */

	void* irq_status_low_reg;		/**< The interrupt extended status register (low part). */
	void* irq_status_high_reg;		/**< The interrupt extended status register (high part). */

} me8200_di_subdevice_t;


/**
 * @brief The constructor to generate a ME-8200 digital input subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param idx The index of the digital output subdevice on this device.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me8200_di_subdevice_t* me8200_di_constr(me_general_dev_t* device, void* reg_base, unsigned int idx, me_lock_t* irq_ctrl_lock, me_lock_t* irq_mode_lock);

# endif
#endif
