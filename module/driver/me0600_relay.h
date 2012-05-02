/**
 * @file me0600_relay.h
 *
 * @brief ME-630 relay subdevice class.
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

# ifndef _ME0600_RELAY_H_
#  define _ME0600_RELAY_H_

#  include "mesubdevice.h"

/**
 * @brief The ME-0600(ME-630) relays subdevice class.
 */
typedef struct //me0600_relay_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes.
	void* port_0_reg;				/**< Register holding the port status. */
	void* port_1_reg;				/**< Register holding the port status. */
} me0600_relay_subdevice_t;


/**
 * @brief The constructor to generate a ME-630 relay subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param ctrl_reg_lock Spin lock protecting the control register.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me0600_relay_subdevice_t* me0600_relay_constr(void* reg_base, unsigned int idx);

# endif
#endif
