/**
 * @file me1400_ext_irq_reg.h
 *
 * @brief ME-1400 external interrupt register definitions.
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
# ifndef _ME1400_EXT_IRQ_REG_H_
#  define _ME1400_EXT_IRQ_REG_H_

#  define ME1400AB_EXT_IRQ_CTRL_REG		0x11	/**< The external interrupt control register offset. */

#  define ME1400AB_EXT_IRQ_DIS			0x00
#  define ME1400AB_EXT_IRQ_CLK_EN		0x01	/**< If this bit is set, the clock output is enabled. */
#  define ME1400AB_EXT_IRQ_EN			0x02	/**< If set the external interrupt is enabled. Clearing this bit clears a pending interrupt. */

#  define ME1400D_EXT_IRQ_CTRL_REG		0x5E	/**< The external interrupt control register offset. */
#  define ME1400C_EXT_IRQ_CTRL_REG		0x1E	/**< The external interrupt control register offset. */
#  define ME1400CD_EXT_IRQ_CTRL_REG		ME1400C_EXT_IRQ_CTRL_REG
#  define ME1400CD_EXT_IRQ_CTRL_REG_SHIFT		(ME1400D_EXT_IRQ_CTRL_REG - ME1400C_EXT_IRQ_CTRL_REG)

#  define ME1400CD_EXT_IRQ_EN			0x10	/**< If set the external interrupt is enabled. Clearing this bit clears a pending interrupt.*/

# endif	//_ME1400_EXT_IRQ_REG_H_
#endif	//__KERNEL__
