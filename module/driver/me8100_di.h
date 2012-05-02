/**
 * @file me8100_di.h
 *
 * @brief The ME-8100 digital input subdevice class header file.
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
# ifndef _ME8100_DI_H_
#  define _ME8100_DI_H_

#  include "mesubdevice.h"

#  define me8100_DI_CAPS				(ME_CAPS_DIO_BIT_PATTERN_IRQ | ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY)

/**
 * @brief The ME-8100 digital input subdevice class.
 */
typedef struct //me8100_di_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t* irq_ctrl_lock;		/**< Spin lock to protect the control register from concurrent access. */
	uint16_t* ctrl_reg_copy;		/**< Copy of the control register (unfortunataly CTRL is WRITE ONLY). */

	volatile int rised;
	unsigned int irq_count;

	uint status_flag;				/**< Default interupt status flag */
	uint status_value;				/**< Interupt status */
	uint status_value_edges;		/**< Extended interupt status */
	uint line_value;

	uint16_t compare_value;
	uint8_t filtering_flag;

	wait_queue_head_t wait_queue;

	void* ctrl_reg;
	void* port_reg;
	void* mask_reg;
	void* pattern_reg;
	void* din_int_reg;
	void* irq_reset_reg;

} me8100_di_subdevice_t;


/**
 * @brief The constructor to generate a ME-8100 digital input subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me8100_di_subdevice_t* me8100_di_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, uint16_t* ctrl_reg_copy);

# endif
#endif
