/**
 * @file me0700_bus.h
 *
 * @brief The ME-0700 (Axon) parallel bus protocol.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author KH (Keith Hartley)
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

#ifdef __KERNEL__

# ifndef _ME0700_BUS_H_
#  define _ME0700_BUS_H_

#  include "medevice.h"
#  include "me0700_ai.h"

// Write port bits

#define ME0700_WRITE_PORT_RESET					0x01
#define ME0700_WRITE_PORT_ADDDRESS				0x02
#define ME0700_WRITE_PORT_READ					0x04
#define ME0700_WRITE_PORT_WRITE					0x08
#define ME0700_WRITE_PORT_MASK					(ME0700_WRITE_PORT_RESET | ME0700_WRITE_PORT_ADDDRESS | ME0700_WRITE_PORT_READ | ME0700_WRITE_PORT_WRITE)

#define ME0700_I_CHANNELS_NUMBER				4
#define ME0700_WRITE_PORT_ADDRESS_CHANNEL_1		0x00
#define ME0700_WRITE_PORT_ADDRESS_CHANNEL_2		0x10
#define ME0700_WRITE_PORT_ADDRESS_CHANNEL_3		0x20
#define ME0700_WRITE_PORT_ADDRESS_CHANNEL_4		0x30
#define ME0700_WRITE_PORT_ADDRESS_IRQ_STATUS	0x40
#define ME0700_WRITE_PORT_ADDRESS_IRQ_RESET		0x50
#define ME0700_WRITE_PORT_ADDRESS_EEPROM		0x60

#define ME0700_I_STATUS_MASK					(0x03 << 6)
#define ME0700_I_OVERFLOW_HIGH					(0x01 << 6)
#define ME0700_I_OVERFLOW_LOW					(0x02 << 6)
#define ME0700_I_READY							(0x00 << 6)
#define ME0700_I_IDLE							(0x03 << 6)

/// Constants
	static const int ME0700_AI_CHANNEL_ADDR[ME0700_I_CHANNELS_NUMBER] = {ME0700_WRITE_PORT_ADDRESS_CHANNEL_1, ME0700_WRITE_PORT_ADDRESS_CHANNEL_2, ME0700_WRITE_PORT_ADDRESS_CHANNEL_3, ME0700_WRITE_PORT_ADDRESS_CHANNEL_4};

/**
 * @brief The ME-0700 (Axon) read function.
 */
int me0700_get_range_relay_status(me0700_ai_subdevice_t* instance, uint8_t*  val, uint8_t addr, int lock);

/**
 * @brief The ME-0700 (Axon) write function.
 */
int me0700_set_range_relay(me0700_ai_subdevice_t* instance, uint8_t  val, uint8_t addr, int lock);
int me0700_update_range_relay(me0700_ai_subdevice_t* instance, uint8_t  val, uint8_t addr);

/**
 * @brief The ME-0700 (Axon) reset function.
 */
int me0700_reset_relays(me0700_ai_subdevice_t* instance);

/**
 * @brief The ME-0700 (Axon)synchronization function.
 */
int me0700_synchronize_all_relays(me0700_ai_subdevice_t* instance);

/**
 * @brief The ME-0700 (Axon) clear status function.
 */
int me0700_reset_OF_bits(me0700_ai_subdevice_t* instance, uint8_t* mask);
#endif
#endif
