/**
 * @file me6000_ao_reg.h
 *
 * @brief ME-6000 analog output subdevice register definitions.
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

#ifndef _ME6000_AO_REG_H_
# define _ME6000_AO_REG_H_

// AO
# define ME6000_AO_CTRL_REG						0x00	/* R/W */
# define ME6000_AO_STATUS_REG					0x04	/* R/_ */
# define ME6000_AO_FIFO_REG						0x08	/* _/W */
# define ME6000_AO_SINGLE_REG					0x0C	/* R/W */
# define ME6000_AO_TIMER_REG					0x10	/* _/W */

# define ME6000_AO_PORT_OFFSET					0x18

//ME6000_AO_CTRL_REG
# define ME6000_AO_MODE_SINGLE					0x00000000
# define ME6000_AO_MODE_WRAPAROUND				0x00000001
# define ME6000_AO_MODE_CONTINUOUS				0x00000002
# define ME6000_AO_CTRL_MODE_MASK				(ME6000_AO_MODE_WRAPAROUND | ME6000_AO_MODE_CONTINUOUS)

# define ME6000_AO_CTRL_BIT_MODE_WRAPAROUND		(0x01 << 0)
# define ME6000_AO_CTRL_BIT_MODE_CONTINUOUS		(0x01 << 1)
# define ME6000_AO_CTRL_BIT_STOP				(0x01 << 2)
# define ME6000_AO_CTRL_BIT_ENABLE_FIFO			(0x01 << 3)
# define ME6000_AO_CTRL_BIT_ENABLE_EX_TRIG		(0x01 << 4)
# define ME6000_AO_CTRL_BIT_EX_TRIG_EDGE		(0x01 << 5)
# define ME6000_AO_CTRL_BIT_ENABLE_IRQ			(0x01 << 6)
# define ME6000_AO_CTRL_BIT_IMMEDIATE_STOP		(0x01 << 7)
//# define ME6000_AO_CTRL_b8						(0x01 << 8)
//# define ME6000_AO_CTRL_b9						(0x01 << 9)
//# define ME6000_AO_CTRL_b10						(0x01 << 10)
# define ME6000_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH 	(0x01 << 11)
//# define ME6000_AO_CTRL_b12						(0x01 << 12)
//# define ME6000_AO_CTRL_b13						(0x01 << 13)
//# define ME6000_AO_CTRL_b14						(0x01 << 14)
//# define ME6000_AO_CTRL_b15						(0x01 << 15)
//# define ME6000_AO_CTRL_b16						(0x01 << 16)
//# define ME6000_AO_CTRL_b17						(0x01 << 17)
//# define ME6000_AO_CTRL_b18						(0x01 << 18)
//# define ME6000_AO_CTRL_b19						(0x01 << 19)
//# define ME6000_AO_CTRL_b20						(0x01 << 20)
//# define ME6000_AO_CTRL_b21						(0x01 << 21)
//# define ME6000_AO_CTRL_b22						(0x01 << 22)
//# define ME6000_AO_CTRL_b23						(0x01 << 23)
//# define ME6000_AO_CTRL_b24						(0x01 << 24)
//# define ME6000_AO_CTRL_b25						(0x01 << 25)
//# define ME6000_AO_CTRL_b26						(0x01 << 26)
//# define ME6000_AO_CTRL_b27						(0x01 << 27)
//# define ME6000_AO_CTRL_b28						(0x01 << 28)
//# define ME6000_AO_CTRL_b29						(0x01 << 29)
/// @note Setting this bit dissable ALL OTHER registry! Always set to zero!
# define ME6000_AO_CTRL_BIT_DMA				(0x01 << 30)
//# define ME6000_AO_CTRL_b31						(0x01 << 31)

//ME6000_AO_STATUS_REG
# define ME6000_AO_STATUS_BIT_FSM				(0x01 << 0)
# define ME6000_AO_STATUS_BIT_FF				(0x01 << 1)
# define ME6000_AO_STATUS_BIT_HF				(0x01 << 2)
# define ME6000_AO_STATUS_BIT_EF				(0x01 << 3)

# define ME6000_AO_EXT_TRIG						0x80000000

# define ME6000_AO_IRQ_RESET_REG				0x64
# define ME6000_AO_IRQ_RESET_OFFSET				0x4


# define ME6000_AO_SINGLE_STATUS_REG			0xA4 	/* R/_ */
# define ME6000_AO_SINGLE_STATUS_OFFSET			0x4		/* The first single subdevice => bit 0 in ME6000_AO_SINGLE_STATUS_REG. */

# define ME6000_AO_SINGLE_SINGLE_REG			0x74 	/* _/W */
# define ME6000_AO_SINGLE_PORT_OFFSET			0x4

# define ME6000_AO_PRELOAD_REG					0xA8  /* R/W */	/** ME6000_AO_SYNC_REG <==> ME6000_AO_PRELOAD_REG */
# define ME6000_AO_SYNC_HOLD					0x00000001
# define ME6000_AO_SYNC_EXT_TRIG				0x00010000

// AO-IRQ
# ifndef ME6000_AO_IRQ_STATUS_REG
#  define ME6000_AO_IRQ_STATUS_REG				0x60  /* R/_ */

#  define ME6000_IRQ_STATUS_BIT_0				(0x01 << 0)
#  define ME6000_IRQ_STATUS_BIT_1				(0x01 << 1)
#  define ME6000_IRQ_STATUS_BIT_2				(0x01 << 2)
#  define ME6000_IRQ_STATUS_BIT_3				(0x01 << 3)

#  define ME6000_IRQ_STATUS_BIT_AO_HF			ME6000_IRQ_STATUS_BIT_0
# endif

//DUMY register
# define ME6000_AO_DUMY							0xFC

#endif
