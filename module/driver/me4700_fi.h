/**
 * @file me4700_fi.h
 *
 * @brief ME-4700 frequency input subdevice class.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
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

# ifndef _ME4700_FIO_H_
#  define _ME4700_FIO_H_

#  include "mesubdevice.h"

#  define ME4700_FI_BASE_FREQUENCY		33000000LL

	enum me4700_fi_status
	{
		fi_status_none = 0,
		fi_status_configured,
		fi_status_configured_single,
		fi_status_configured_single_end,
		fi_status_error,
		fi_status_last
	};

	/**
	* @brief The ME-4700 frequency input subdevice class.
	*/
	typedef struct //me4700_fi_subdevice
	{
		// Inheritance
		me_subdevice_t base;			/**< The subdevice base class. */
		wait_queue_head_t wait_queue;


		// Attributes
		uint period;
		uint divider;
		uint first_counter;

		volatile uint low;
		volatile uint high;

		me_lock_t* ctrl_reg_lock;		/**< Spin lock to protect ctrl_reg from concurrent access. */

		volatile enum me4700_fi_status status;	/**< The current stream status flag. */

		// Registers							/**< All registers are 32 bits long. */
		void* low_level_reg;			/**< Register for low level counter of pulse. */
		void* high_level_reg;			/**< Register for high level counter of pulse. */
		void* ctrl_reg;					/**< Register to start/stop action. */
		void* status_reg;				/**< Register to configure the port direction. */
	} me4700_fi_subdevice_t;


	/**
	* @brief The constructor to generate a ME-4700 frequency input/ouput subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx The index of the digital i/o port on the device.
	* @param ctrl_reg_lock Spin lock protecting the control register.
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	me4700_fi_subdevice_t* me4700_fi_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock);

# endif
#endif
