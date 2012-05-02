/**
 * @file me4600_ao.h
 *
 * @brief The ME-4600 analog output subdevice class header file.
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

# ifndef _ME4600_AO_H_
#  define _ME4600_AO_H_

#  include <linux/version.h>
#  include <linux/workqueue.h>

#  include "mesubdevice.h"
#  include "mecirc_buf.h"

#  define ME4600_AO_MAX_SUBDEVICES		4
#  define ME4600_AO_FIFO_COUNT			4096

#  define ME4600_AO_BASE_FREQUENCY		33000000LL

#  define ME4600_AO_MIN_ACQ_TICKS		0LL
#  define ME4600_AO_MAX_ACQ_TICKS		0LL

#  define ME4600_AO_MIN_CHAN_TICKS		66LL
#  define ME4600_AO_MAX_CHAN_TICKS		0xFFFFFFFFLL


#  define ME4600_AO_MIN_RANGE			-10000000
#  define ME4600_AO_MAX_RANGE			9999695

#  define ME4600_AO_MAX_DATA			0xFFFF

#  define ME4600_AO_OPTIMAL_CHAN_TICKS	(ME4600_AO_BASE_FREQUENCY >> 7)

#  ifdef ME_SYNAPSE
#   define ME4600_AO_CIRC_BUF_SIZE_ORDER 		8 // 2^n PAGES =>> Maximum value of 1MB for Synapse
#  else
#   define ME4600_AO_CIRC_BUF_SIZE_ORDER 		5 // 2^n PAGES =>> 128KB
#  endif
#  define ME4600_AO_CIRC_BUF_SIZE 		PAGE_SIZE<<ME4600_AO_CIRC_BUF_SIZE_ORDER // Buffer size in bytes.

#  ifdef _CBUFF_32b_t
#   define ME4600_AO_CIRC_BUF_COUNT	((ME4600_AO_CIRC_BUF_SIZE) / sizeof(uint32_t))	// Size in values
#  else
#   define ME4600_AO_CIRC_BUF_COUNT	((ME4600_AO_CIRC_BUF_SIZE) / sizeof(uint16_t))	// Size in values
#  endif


#  define ME4600_AO_CONTINUOUS					0x0
#  define ME4600_AO_WRAP_MODE					0x1
#  define ME4600_AO_HW_MODE						0x2

#  define ME4600_AO_HW_WRAP_MODE				(ME4600_AO_WRAP_MODE | ME4600_AO_HW_MODE)
#  define ME4600_AO_SW_WRAP_MODE				ME4600_AO_WRAP_MODE

#  define ME4600_AO_INF_STOP_MODE				0x0
#  define ME4600_AO_LIST_STOP_MODE				0x1
#  define ME4600_AO_SCAN_STOP_MODE				0x2

	enum ME4600_AO_STATUS
	{
		ao_status_none = 0,
		ao_status_single_configured,
		ao_status_single_run,
		ao_status_single_end,
		ao_status_single_error,
		ao_status_stream_configured,
		ao_status_stream_run_wait,
		ao_status_stream_run,
		ao_status_stream_end_wait,
		ao_status_stream_end,
		ao_status_stream_fifo_error,
		ao_status_stream_buffer_error,
		ao_status_stream_timeout,
		ao_status_error,
		ao_status_last
	};

	typedef struct //me4600_ao_timeout
	{
		unsigned long start_time;
		unsigned long int delay;
	} me4600_ao_timeout_t;

	/**
	* @brief The ME-4600 analog output subdevice class.
	*/
	typedef struct //me4600_ao_subdevice
	{
		// Inheritance
		me_subdevice_t base;					/**< The subdevice base class. */

		// Attributes
		me_lock_t* preload_reg_lock;			/**< Spin lock to protect preload_reg from concurrent access. */

		uint32_t* preload_flags;

		// Hardware feautres
		int fifo;								/**< If set this device has a FIFO. */
		int bitpattern;							/**< If set this device use bitpattern. */

		int single_value;						/**< Mirror of the output value in single mode. */
		int single_value_in_fifo;				/**< Mirror of the value written in single mode. */
		uint32_t ctrl_trg;						/**< Mirror of the trigger settings. */

		volatile int mode;						/**< Flags used for storing SW wraparound setup*/
		int stop_mode;							/**< The user defined stop condition flag. */
		unsigned int start_mode;
		unsigned int stop_count;				/**< The user defined dates presentation end count. */
		unsigned int stop_data_count;			/**< The stop presentation count. */
		unsigned int data_count;				/**< The real presentation count. */
		unsigned int preloaded_count;			/**< The next data addres in buffer. <= for wraparound mode. */

		volatile enum ME4600_AO_STATUS status;	/**< The current stream status flag. */
		me4600_ao_timeout_t timeout;			/**< The timeout for start in blocking and non-blocking mode. */

		// Registers							/**< All registers are 32 bits long. */
		void* ctrl_reg;
		void* status_reg;
		void* fifo_reg;
		void* single_reg;
		void* timer_reg;
		void* preload_reg;
		void* DMA_base;

		// Software buffer
		me_circ_buf_t circ_buf;					/**< Circular buffer holding measurment data. 32 bit long */
		wait_queue_head_t wait_queue;			/**< Wait queue to put on tasks waiting for data to arrive. */

		struct workqueue_struct* me4600_workqueue;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
		struct work_struct ao_control_task;
#else
		struct delayed_work ao_control_task;
#endif

		atomic_t ao_control_task_flag;		/**< Flag controling re-executing of control task */

		int stream_start_count;
		int stream_stop_count;

		int fifo_size;
	} me4600_ao_subdevice_t;

	/**
	* @brief The constructor to generate a ME-4600 analog output subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx Subdevice number.
	* @param ctrl_reg_lock Pointer to spin lock protecting the control register from concurrent access.
	* @param preload_flags Pointer to shared synchro flag.
	* @param fifo Flag set if subdevice has hardware FIFO.
	* @param me4600_wq Queue for asynchronous task (1 queue for all subdevices on 1 board).
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	me4600_ao_subdevice_t* me4600_ao_constr(void* reg_base,
												void* DMA_base,
												unsigned int idx,
												me_lock_t* preload_reg_lock, uint32_t* preload_flags,
												int fifo, struct workqueue_struct* me4600_wq);

# endif // ~_ME4600_AO_H_
#endif //__KERNEL__
