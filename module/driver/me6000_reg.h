/**
 * @file me6000_reg.h
 *
 * @brief ME-6000 device register definitions.
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

# ifndef _ME6000_REG_H_
#  define _ME6000_REG_H_

#  define ME6000_INIT_XILINX_REG		0xAC  // R/-

#  ifndef ME6000_AO_IRQ_STATUS_REG
#   define ME6000_AO_IRQ_STATUS_REG				0x60  // R/_

#   define ME6000_IRQ_STATUS_BIT_0				(0x01 << 0)
#   define ME6000_IRQ_STATUS_BIT_1				(0x01 << 1)
#   define ME6000_IRQ_STATUS_BIT_2				(0x01 << 2)
#   define ME6000_IRQ_STATUS_BIT_3				(0x01 << 3)

#   define ME6000_IRQ_STATUS_BIT_AO_HF			ME6000_IRQ_STATUS_BIT_0
#  endif

# endif
#endif
