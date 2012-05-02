/**
 * @file me4700_fio_reg.h
 *
 * @brief ME-4f00 frequency input/output subdevice register definitions.
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

#ifndef _ME4700_FIO_REG_H_
# define _ME4700_FIO_REG_H_

# ifdef __KERNEL__

/** @NOTE: Clearing FI IRQ is done by reading counter (one of them). */

					/**< FI control register. */
#  define ME4700_FI_START_STOP_REG				0xB0	/* R/W */
#  define ME4700_FI_START_STOP_BIT_BASE			0
#  define ME4700_FI_START_STOP_BIT				0x01
#  define ME4700_FI_START_STOP_INTERRUPTS		0x02
#  define ME4700_FI_START_STOP_MASK				0x03
#  define ME4700_FI_START_STOP_BIT_SHIFT		2

					/**< Data registers. */
#  define ME4700_FIO_PORT_A_HIGH_REG			0x5C
#  define ME4700_FIO_PORT_A_LOW_REG				0x60
#  define ME4700_FIO_PORT_B_HIGH_REG			0x64
#  define ME4700_FIO_PORT_B_LOW_REG				0x68
#  define ME4700_FIO_PORT_C_HIGH_REG			0x6c
#  define ME4700_FIO_PORT_C_LOW_REG				0x70
#  define ME4700_FIO_PORT_D_HIGH_REG			0xD4
#  define ME4700_FIO_PORT_D_LOW_REG				0xD8

					/**< Control register. */
#  define ME4700_FIO_CTRL_REG					0xB8

					/**< Control register definitions. */
#  define ME4700_FIO_CTRL_BIT_MODE_MASK			0x0003
#  define ME4700_FIO_CTRL_BIT_FRQUENCY_MODE		0x0002
#  define ME4700_FI_CTRL_BIT_SHIFT				0x0004
#  define ME4700_FO_CTRL_BIT_SHIFT				0x0006

					/**< FO control register. */
#  define ME4700_FO_START_STOP_BIT_BASE			24
#  define ME4700_FO_START_STOP_BIT				0x01
#  define ME4700_FO_START_STOP_INVERT			0x02
#  define ME4700_FO_START_STOP_MASK				(ME4700_FO_START_STOP_BIT | ME4700_FO_START_STOP_INVERT)
#  define ME4700_FO_START_STOP_BIT_SHIFT		2

#  define ME4700_FI_STATUS						0xD0	/* R/_ */
#  define ME4700_FI_STATUS_BASE					0x1

// #  define ME4700_FI_IRQ_STATUS					0x9C
#  define ME4700_FI_IRQ_CTRL_BIT_BASE			0x9
// #  define ME4700_FIO_IRQ_CTRL_BIT_PORT_A		(0x1 << 9)
// #  define ME4700_FIO_IRQ_CTRL_BIT_PORT_B		(0x1 << 10)
// #  define ME4700_FIO_IRQ_CTRL_BIT_PORT_C		(0x1 << 11)
// #  define ME4700_FIO_IRQ_CTRL_BIT_PORT_D		(0x1 << 12)


#  define ME4700_FO_TRIGGER_STATUS_BIT_SHIFT	4
#  define ME4700_FO_SYNCHRO_STATUS_BIT			0x01
#  define ME4700_FO_INVERT_STATUS_BIT			0x02
#  define ME4700_FO_TRIGGER_STATUS_BIT			0x04
#  define ME4700_FO_SOFT_START_STATUS_BIT		0x08
#  define ME4700_FO_STATUS_BIT_MASK				(ME4700_FO_TRIGGER_STATUS_BIT | ME4700_FO_SOFT_START_STATUS_BIT | ME4700_FO_SYNCHRO_STATUS_BIT | ME4700_FO_INVERT_STATUS_BIT)

# endif	// __KERNEL__
#endif	// _ME4700_FIO_REG_H_
