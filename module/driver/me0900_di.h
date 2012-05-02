/**
 * @file me0900_di.h
 *
 * @brief The ME-0900 (ME-9x) digital input subdevice class header file.
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

# ifndef _ME0900_DI_H_
#  define _ME0900_DI_H_

#  include "mesubdevice.h"

/**
 * @brief The ME-0900 (ME-9x) digital input subdevice class.
 */
typedef struct //me0900_di_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes.
	void* port_reg;
} me0900_di_subdevice_t;


/**
 * @brief The constructor to generate a ME-9x digital input subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me0900_di_subdevice_t* me0900_di_constr(void* reg_base, unsigned int idx, int start_port);

#endif
#endif
