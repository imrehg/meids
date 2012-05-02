/**
 * @file me0600_ext_irq.h
 *
 * @brief The ME-0600(ME-630) external interrupt implementation.
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

# ifndef _ME0600_EXT_IRQ_H_
#  define _ME0600_EXT_IRQ_H_

#  include "mesubdevice.h"
#  include "me_interrupt_types.h"
#  include "me_spin_lock.h"

#  define me0600_EXT_IRQ_CAPS			ME_CAPS_EXT_IRQ_EDGE_RISING

/**
 * @brief The ME-0600(ME-630) external interrupt subdevice class.
 */
typedef struct //me0600_ext_irq_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t *intcsr_lock;		/**< Spin lock to protect intcsr. */

	wait_queue_head_t wait_queue;	/**< Queue to put on threads waiting for an interrupt. */

	volatile enum ME_IRQ_STATUS status;
	int value;
	volatile int count;
	volatile int reset_count;

	void* intcsr;					/**< The PLX interrupt control and status register. */
	void* reset_reg;				/**< The control register. */
} me0600_ext_irq_subdevice_t;


/**
 * @brief The constructor to generate a ME-630 external interrupt instance.
 *
 * @param plx_reg_base The register base address of the PLX chip as returned by the PCI BIOS.
 * @param reg_base The register base address of the ME-630 device as returned by the PCI BIOS.
 * @param irq The irq assigned by the PCI BIOS.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me0600_ext_irq_subdevice_t* me0600_ext_irq_constr(void* plx_reg_base, void* reg_base, unsigned int idx, me_lock_t* intcsr_lock);

# endif
#endif
