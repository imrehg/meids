/**
 * @file me4700_fo.h
 *
 * @brief ME-4700 frequency output subdevice class.
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

# ifndef _ME4700_FO_H_
#  define _ME4700_FO_H_

#  include "mesubdevice.h"

#  define ME4700_FO_BASE_FREQUENCY		33000000LL


	enum me4700_fo_configuration
	{
		fo_configuration_none = 0,
		fo_configuration_divider,
		fo_configuration_first_counter,
		fo_configuration_last
	};

	typedef struct //me4700_fo_params
	{
		uint32_t period;
		enum me4700_fo_configuration mode;
		uint32_t divider;
		uint32_t first_counter;
	} me4700_fo_params_t;

	typedef struct //me4700_fo_regs
	{
		void* low_level_reg;					/**< Register for low level counter of pulse. */
		void* high_level_reg;					/**< Register for high level counter of pulse. */
	} me4700_fo_regs_t;

	typedef struct me4700_fo_context
	{
		me_lock_t fo_context_lock;							/**< Protects the context struct. */
		int count;
		me4700_fo_regs_t*	regs;
		me4700_fo_params_t* shadow;
		me4700_fo_params_t* mirror;
		uint32_t conditions;						/**< Triggering and synchronization flags. */
	} me4700_fo_context_t;

	enum me4700_fo_status
	{
		fo_status_none = 0,
		fo_status_configured,
		fo_status_single_run,
		fo_status_single_end,
		fo_status_error,
		fo_status_last
	};

	typedef struct //me4700_fo_timeout
	{
		unsigned long start_time;
		unsigned long int delay;
	}me4700_fo_timeout_t;

	/**
	* @brief The ME-4700 frequency input/output subdevice class.
	*/
	typedef struct //me4700_fo_subdevice
	{
		// Inheritance
		me_subdevice_t base;					/**< The subdevice base class. */
		wait_queue_head_t wait_queue;


		// Attributes
		me_lock_t* ctrl_reg_lock;				/**< Spin lock to protect ctrl_reg from concurrent access. */

		volatile enum me4700_fo_status status;	/**< The current stream status flag. */

		// Registers							/**< All registers are 32 bits long. */
		void* ctrl_reg;							/**< Register to start/stop action. */

		me4700_fo_timeout_t timeout;			/**< The timeout for start in blocking and non-blocking mode. */
		struct workqueue_struct* me4700_workqueue;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
		struct work_struct fo_control_task;
#else
		struct delayed_work fo_control_task;
#endif
		me4700_fo_context_t* fo_shared_contex;
		atomic_t fo_control_task_flag;	/**< Flag controling reexecuting of control task */
	} me4700_fo_subdevice_t;


	/**
	* @brief The constructor to generate a ME-4700 frequency ouput subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx The index of the digital i/o port on the device.
	* @param ctrl_reg_lock Spin lock protecting the control register.
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	me4700_fo_subdevice_t* me4700_fo_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, me4700_fo_context_t* fo_shared_contex, struct workqueue_struct* me4700_wq);

# endif
#endif
