/**
 * @file me4600_dio.h
 *
 * @brief ME-4600 digital input/output subdevice class.
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

# ifndef _ME4600_DIO_H_
#  define _ME4600_DIO_H_

#  include "mesubdevice.h"

#  define me4600_DIO_CAPS				ME_CAPS_DIO_DIR_BYTE

/**
 * @brief The ME-4600 digital input/output subdevice class.
 */
typedef struct //me4600_dio_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	int port_type;					/**< The index of the digital i/o on the device. */

	me_lock_t* ctrl_reg_lock;		/**< Spin lock to protect ctrl_reg from concurrent access. */

	void* port_reg;					/**< Register holding the port status. */
	void* ctrl_reg;					/**< Register to configure the port direction. */
} me4600_dio_subdevice_t;


/**
 * @brief The constructor to generate a ME-4600 digital input/ouput subdevice instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param idx The index of the digital i/o port on the device.
 * @param ctrl_reg_lock Spin lock protecting the control register.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
me4600_dio_subdevice_t* me4600_dio_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, int port_type);

# endif
#endif
