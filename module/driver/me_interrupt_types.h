/**
 * @file me_interrupt_types.h
 *
 * @brief Structs for keeping data for interrupt handling.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# ifndef _ME_INTERRUPT_TYPES_H_
#  define _ME_INTERRUPT_TYPES_H_

/**
 * @brief Structure holding easy structure. Only interrupt status register.
 *
 * @note This may looks as unnecessary. None of existing boards need anything more than just address of interrupt status register, \n
 *			however I prefer this way for future board that can have more complicated interrupt controler.
 */
typedef struct //me_basic_interrupt_status
{
	void* intcsr;
} me_basic_interrupt_status_t;

typedef me_basic_interrupt_status_t me0600_interrupt_status_t;
typedef me_basic_interrupt_status_t me0700_interrupt_status_t;
typedef me_basic_interrupt_status_t me0900_interrupt_status_t;
typedef me_basic_interrupt_status_t me1000_interrupt_status_t;
typedef me_basic_interrupt_status_t me1400_interrupt_status_t;
typedef me_basic_interrupt_status_t me1600_interrupt_status_t;
typedef me_basic_interrupt_status_t me4600_interrupt_status_t;
typedef me_basic_interrupt_status_t me4700_interrupt_status_t;
typedef me_basic_interrupt_status_t me6000_interrupt_status_t;
typedef me_basic_interrupt_status_t me8100_interrupt_status_t;
typedef me_basic_interrupt_status_t me8200_interrupt_status_t;

typedef me_basic_interrupt_status_t me9300_interrupt_status_t;

enum ME_IRQ_STATUS
{
	irq_status_none = 0,
	irq_status_stop,
	irq_status_run,
	irq_status_error,
	irq_status_last
};

# endif
#endif

