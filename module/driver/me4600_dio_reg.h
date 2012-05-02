/**
 * @file me4600_dio_reg.h
 *
 * @brief ME-4600 digital input/output subdevice register definitions.
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

#ifndef _ME4600_DIO_REG_H_
#define _ME4600_DIO_REG_H_


#ifdef __KERNEL__


#define ME4600_DIO_PORT_REG					0xA0					/**< Base for port's register. */

#define ME4600_DIO_CTRL_REG					0xB8					/**< Control register. */

#define ME4600_DIO_CTRL_BIT_MODE_MASK		0x0003
#define ME4600_DIO_CTRL_BIT_MODE_INPUT		0x0000
#define ME4600_DIO_CTRL_BIT_MODE_OUTPUT		0x0001
#define ME4600_DIO_CTRL_BIT_MODE_FIFO		0x0003
#define ME4600_DIO_CTRL_BIT_FIFO_HIGH		0x0400

#  define ME4600_DIO_MODE_SHIFT				2



// #define ME4600_DIO_PORT_0_REG				0xA0					/**< Port 0 register. */
// #define ME4600_DIO_PORT_1_REG				0xA4					/**< Port 1 register. */
// #define ME4600_DIO_PORT_2_REG				0xA8					/**< Port 2 register. */
// #define ME4600_DIO_PORT_3_REG				0xAC					/**< Port 3 register. */
//
// /** Port A - DO/DIO */
// #define ME4600_DIO_CTRL_BIT_MODE_0			0x0001
// #define ME4600_DIO_CTRL_BIT_MODE_1			0x0002
// /** Port B - DI/DIO */
// #define ME4600_DIO_CTRL_BIT_MODE_2			0x0004
// #define ME4600_DIO_CTRL_BIT_MODE_3			0x0008
// /** Port C - DIO */
// #define ME4600_DIO_CTRL_BIT_MODE_4			0x0010
// #define ME4600_DIO_CTRL_BIT_MODE_5			0x0020
// /** Port D - DIO */
// #define ME4600_DIO_CTRL_BIT_MODE_6			0x0040
// #define ME4600_DIO_CTRL_BIT_MODE_7			0x0080

#define ME4600_DIO_CTRL_BIT_FUNCTION_MASK	0x0300
#define ME4600_DIO_CTRL_BIT_FUNCTION_BP		0x0000
#define ME4600_DIO_CTRL_BIT_FUNCTION_DEMUX	0x0100
#define ME4600_DIO_CTRL_BIT_FUNCTION_MUX	0x0200

// #define ME4600_DIO_CTRL_BIT_FUNCTION_0		0x0100
// #define ME4600_DIO_CTRL_BIT_FUNCTION_1		0x0200

// #define ME4600_DIO_CTRL_BIT_FIFO_HIGH_0		0x0400
// #define ME4600_DIO_CTRL_BIT_FIFO_HIGH_1		0x0800
// #define ME4600_DIO_CTRL_BIT_FIFO_HIGH_2		0x1000
// #define ME4600_DIO_CTRL_BIT_FIFO_HIGH_3		0x2000


#endif
#endif
