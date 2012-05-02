/**
 * @file me6000_dio_reg.h
 *
 * @brief ME-6000 digital input/output subdevice register definitions.
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

# ifndef _ME6000_DIO_REG_H_
#  define _ME6000_DIO_REG_H_

#define ME6000_DIO_CTRL_REG				0x00 /**< R/W */
#  define ME6000_DIO_PORT_REG			0x1	/**< Base for port's register. R/W */
#  define ME6000_PORT_STEP				1	/**< Distance between port's register. */

#define ME6000_DIO_CTRL_BIT_MODE_INPUT		0x0000
#define ME6000_DIO_CTRL_BIT_MODE_OUTPUT		0x0001

#  define ME6000_DIO_CTRL_BIT_MODE_MASK		0x03

#  define ME6000_DIO_MODE_SHIFT				2


# endif
#endif
