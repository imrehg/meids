/**
 * @file me1600_ao.h
 *
 * @brief Meilhaus ME-1600 analog output subdevice class header file.
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

# ifndef _ME1600_AO_H_
#  define _ME1600_AO_H_

#  include <linux/version.h>

#  include "mesubdevice.h"

#  define ME1600_MAX_RANGES	2	/**< Specifies the maximum number of ranges in me1600_ao_subdevice_t::u_ranges und me1600_ao_subdevice_t::i_ranges. */

	/**
	* @brief Defines a entry in the range table.
	*/
	typedef struct //me1600_ao_range_entry
	{
		int32_t min;
		int32_t max;
	} me1600_ao_range_entry_t;

	typedef struct //me1600_ao_timeout
	{
		unsigned long start_time;
		unsigned long int delay;
	}me1600_ao_timeout_t;

	typedef struct me1600_ao_shadow
	{
		int count;
		void** registers;
		uint16_t* shadow;
		uint16_t* mirror;
		uint16_t synchronous;									/**< Synchronization list. */
		uint16_t trigger;										/**< Synchronization flag. */
	}__attribute__((packed)) me1600_ao_shadow_t;

	enum ME1600_AO_STATUS
	{
		ao_status_none = 0,
		ao_status_single_configured,
		ao_status_single_run,
		ao_status_single_end,
		ao_status_last
	};

	/**
	* @brief The ME-1600 analog output subdevice class.
	*/
	typedef struct //me1600_ao_subdevice
	{
		// Inheritance
		me_subdevice_t base;									/**< The subdevice base class. */

		// Attributes
		int idx;												/**< The index of the analog output subdevice on the device. */

		me_lock_t *config_regs_lock;							/**< Spin lock to protect configuration registers from concurrent access. */

		int u_ranges_count;										/**< The number of voltage ranges available on this subdevice. */
		me1600_ao_range_entry_t u_ranges[ME1600_MAX_RANGES];	/**< Array holding the voltage ranges on this subdevice. */
		int i_ranges_count;										/**< The number of current ranges available on this subdevice. */
		me1600_ao_range_entry_t i_ranges[ME1600_MAX_RANGES];	/**< Array holding the current ranges on this subdevice. */

		// Registers
		void* uni_bi_reg;										/**< Register for switching between unipoar and bipolar output mode. */
		void* i_range_reg;										/**< Register for switching between ranges. */
		void* sim_output_reg;									/**< Register used in order to update all channels simultaneously. */
		void* current_on_reg;									/**< Register enabling current output on the fourth subdevice. */

		volatile enum ME1600_AO_STATUS status;
		me1600_ao_shadow_t* ao_regs_shadows;					/**< Addresses and shadows of output's registers. */
		me_lock_t *ao_shadows_lock;								/**< Protects the shadow's struct. */
		wait_queue_head_t wait_queue;							/**< Wait queue to put on tasks waiting for data to arrive. */
		me1600_ao_timeout_t timeout;							/**< The timeout for start in blocking and non-blocking mode. */
		struct workqueue_struct* me1600_workqueue;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
		struct work_struct ao_control_task;
#else
		struct delayed_work ao_control_task;
#endif

		atomic_t ao_control_task_flag;						/**< Flag controling reexecuting of control task */
	} me1600_ao_subdevice_t;


	/**
	* @brief The constructor to generate a subdevice template instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx The index of the analog output subdevice on the device.
	* @param current Flag indicating that analog output with #idx of 3 is capable of current output.
	* @param config_regs_lock Pointer to spin lock protecting the configuration registers and from concurrent access.
	*
	* @return Pointer to new instance on success.\n
	*			NULL on error.
	*/
	me1600_ao_subdevice_t* me1600_ao_constr(void* reg_base, unsigned int idx,
												me_lock_t* config_regs_lock, me_lock_t* ao_shadows_lock, me1600_ao_shadow_t* ao_regs_shadows,
												int curr, struct workqueue_struct *me1600_wq);

# endif	//_ME1600_AO_H_
#endif	//__KERNEL__
