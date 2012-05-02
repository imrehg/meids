/**
 * @file me4600_ext_irq_reg.h
 *
 * @brief ME-4600 external interrupt subdevice register definitions.
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

#ifndef _ME4600_EXT_IRQ_REG_H_
# define _ME4600_EXT_IRQ_REG_H_


# ifdef __KERNEL__

#  define ME4600_EXT_IRQ_CONFIG_REG				0xCC // R/_
#  define ME4600_EXT_IRQ_VALUE_REG				0xD0 // R/_

#  define ME4600_EXT_IRQ_DISABLED				0x00
#  define ME4600_EXT_IRQ_CONFIG_MASK_RISING		0x01
#  define ME4600_EXT_IRQ_CONFIG_MASK_FALLING	0x02

#  define ME4600_EXT_IRQ_CONFIG_MASK_ANY		0x03
#  define ME4600_EXT_IRQ_CONFIG_MASK			0x03

#  define ME4600_EXT_IRQ_RESET					0x04

#  define ME4600_EXT_IRQ_VALUE_SHIFT			0

# endif	//__KERNEL__
#endif
