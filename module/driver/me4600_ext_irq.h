/**
 * @file me4600_ext_irq.h
 *
 * @brief Meilhaus ME-4000 external interrupt subdevice class header file.
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

# ifndef _ME4600_EXT_IRQ_H_
#  define _ME4600_EXT_IRQ_H_

#  include "mesubdevice.h"
#  include "me_interrupt_types.h"

#  define me4600_EXT_IRQ_CAPS		(ME_CAPS_EXT_IRQ_EDGE_RISING | ME_CAPS_EXT_IRQ_EDGE_FALLING | ME_CAPS_EXT_IRQ_EDGE_ANY)

/**
 * @brief The subdevice class.
 */
typedef struct //me4600_ext_irq_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	wait_queue_head_t wait_queue;

	volatile enum ME_IRQ_STATUS status;
	int value;
	volatile int count;
	volatile int reset_count;

	uint32_t mode;	/// New firmware

	void* ext_irq_config_reg;
	void* ext_irq_value_reg;

#ifdef MEDEBUG_SPEED_TEST
	volatile uint64_t int_start, int_end;
#endif
} me4600_ext_irq_subdevice_t;


/**
 * @brief The constructor to generate a external interrupt subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param idx Subdevice number.
 * @param ctrl_reg_lock Pointer to spin lock protecting the control register from concurrent access.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me4600_ext_irq_subdevice_t* me4600_ext_irq_constr(void* reg_base, unsigned int idx);

# endif
#endif
