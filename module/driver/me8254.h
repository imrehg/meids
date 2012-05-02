/**
 * @file me8254.h
 *
 * @brief The 8254 counter subdevice class header file.
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

# ifndef _ME8254_H_
#  define _ME8254_H_

#  include "mesubdevice.h"

/**
 * @brief The 8254 counter subdevice class.
 */
typedef struct //me8254_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t* ctrl_reg_lock;		/**< Spin lock to protect the control register from concurrent access. */
	me_lock_t *clk_src_reg_lock;	/**< Spin lock to protect the clock source register from concurrent access. */

	uint16_t device_id;				/**< The Meilhaus device type carrying the 8254 chip. */
	int me8254_idx;					/**< The index of the 8254 chip on the device. */
	int ctr_idx;					/**< The index of the counter on the 8254 chip. */

	int caps;						/**< Holds the device capabilities. */

	void* val_reg;					/**< Holds the actual counter value. */
	void* ctrl_reg;					/**< Register to configure the 8254 modes. */
	void* clk_src_reg;				/**< Register to configure the counter connections. */
} me8254_subdevice_t;


/**
 * @brief The constructor to generate a 8254 instance.
 *
 * @param device_id The kind of Meilhaus device holding the 8254.
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param me8254_idx The index of the 8254 chip on the Meilhaus device.
 * @param ctr_idx The index of the counter inside a 8254 chip.
 * @param ctrl_reg_lock Pointer to spin lock protecting the 8254 control register from concurrent access.
 * @param clk_src_reg_lock Pointer to spin lock protecting the clock source register from concurrent access.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me8254_subdevice_t* me8254_constr(uint16_t device_id, void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx,
											me_lock_t* ctrl_reg_lock, me_lock_t *clk_src_reg_lock);

# endif
#endif
