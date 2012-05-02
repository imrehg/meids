/**
 * @file me6000_ao.h
 *
 * @brief The ME-6000 analog output subdevice class header file.
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

# ifndef _ME6000_AO_H_
#  define _ME6000_AO_H_

#  include <linux/version.h>

#  include "mesubdevice.h"
#  include "mecirc_buf.h"

#  define ME6000_AO_MAX_SUBDEVICES	16
#  define ME6000_AO_FIFO_COUNT		8192

#  define ME6000_AO_BASE_FREQUENCY	33000000L

#  define ME6000_AO_MIN_ACQ_TICKS		0LL
#  define ME6000_AO_MAX_ACQ_TICKS		0LL

#  define ME6000_AO_MIN_CHAN_TICKS	66LL
#  define ME6000_AO_MAX_CHAN_TICKS	0xFFFFFFFFLL

#  define ME6000_AO_MIN_RANGE			-10000000
#  define ME6000_AO_MAX_RANGE			9999695

#  define ME6000_AO_MIN_RANGE_HIGH	0
#  define ME6000_AO_MAX_RANGE_HIGH	49999237

#  define ME6000_AO_MAX_DATA			0xFFFF

#  define ME6000_AO_OPTIMAL_CHAN_TICKS	(ME6000_AO_BASE_FREQUENCY >> 7)


#  ifdef ME_SYNAPSE
// 2^n PAGES =>> Maximum value of 1MB for Synapse
#   define ME6000_AO_CIRC_BUF_SIZE_ORDER 		8
#  else
// 2^n PAGES =>> 128KB
#   define ME6000_AO_CIRC_BUF_SIZE_ORDER 		5
#  endif
// Buffer size in bytes.
#  define ME6000_AO_CIRC_BUF_SIZE 		PAGE_SIZE<<ME6000_AO_CIRC_BUF_SIZE_ORDER

// Size in values
#  ifdef _CBUFF_32b_t
#   define ME6000_AO_CIRC_BUF_COUNT	((ME6000_AO_CIRC_BUF_SIZE) / sizeof(uint32_t))
#  else
#   define ME6000_AO_CIRC_BUF_COUNT	((ME6000_AO_CIRC_BUF_SIZE) / sizeof(uint16_t))
#  endif

#  define ME6000_AO_CONTINUOUS					0x0
#  define ME6000_AO_WRAP_MODE					0x1
#  define ME6000_AO_HW_MODE						0x2

#  define ME6000_AO_HW_WRAP_MODE				(ME6000_AO_WRAP_MODE | ME6000_AO_HW_MODE)
#  define ME6000_AO_SW_WRAP_MODE				ME6000_AO_WRAP_MODE

#  define ME6000_AO_INF_STOP_MODE				0x0
#  define ME6000_AO_LIST_STOP_MODE				0x1
#  define ME6000_AO_SCAN_STOP_MODE				0x2

#  define ME6000_AO_EXTRA_HARDWARE				0x1
#  define ME6000_AO_HAS_FIFO					0x2

#  define ME6000_CONTROL_TASK_SCHEDULE			1

	enum ME6000_AO_STATUS
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

	typedef struct //me6000_ao_timeout
	{
		unsigned long start_time;
		unsigned long int delay;
	}me6000_ao_timeout_t;

	/**
	* @brief The ME-6000 analog output subdevice class.
	*/
	typedef struct //me6000_ao_subdevice
	{
		// Inheritance
		me_subdevice_t base;					/**< The subdevice base class. */

		// Attributes
		me_lock_t *preload_reg_lock;			/**< Spin lock to protect preload_reg from concurrent access. */

		uint32_t *preload_flags;
		uint32_t *triggering_flags;

		// Hardware feautres
		int fifo;								/**< If 'ME6000_AO_HAS_FIFO' is set this device has a FIFO.
													 If 'ME6000_AO_EXTRA_HARDWARE' is set this device has a architecture of steaming device but without FIFO.*/

		//Range
		int min;
		int max;

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

		volatile enum ME6000_AO_STATUS status;	/**< The current stream status flag. */
		me6000_ao_timeout_t timeout;			/**< The timeout for start in blocking and non-blocking mode. */

		// Registers							/**< All registers are 32 bits long. */
		void* ctrl_reg;
		void* status_reg;
		void* fifo_reg;
		void* single_reg;
		void* timer_reg;
		void* irq_status_reg;
		void* preload_reg;
		void* irq_reset_reg;
		void* DMA_base;

		// Software buffer
		me_circ_buf_t circ_buf;					/**< Circular buffer holding measurment data. */
		wait_queue_head_t wait_queue;			/**< Wait queue to put on tasks waiting for data to arrive. */

		struct workqueue_struct* me6000_workqueue;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
		struct work_struct ao_control_task;
#else
		struct delayed_work ao_control_task;
#endif

		atomic_t ao_control_task_flag;		/**< Flag controling reexecuting of control task */

		int stream_start_count;
		int stream_stop_count;

		int fifo_size;
	} me6000_ao_subdevice_t;


	/**
	* @brief The constructor to generate a ME-6000 analog output subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param ctrl_reg_lock Pointer to spin lock protecting the control register from concurrent access.
	* @param preload_flags Flag for managing hold&trigger features.
	* @param idx Subdevice number.
	* @param fifo Flag set if subdevice has hardware FIFO.
	* @param irq IRQ number.
	* @param high_range Flag set if subdevice has high curren output.
	* @param me6000_wq Queue for asynchronous task (1 queue for all subdevice on 1 board).
	*
	* @return Pointer to new instance on success.\n
	*			NULL on error.
	*/
	me6000_ao_subdevice_t* me6000_ao_constr(void* reg_base,
												void* DMA_base,
												unsigned int idx,
												me_lock_t *preload_reg_lock,
												uint32_t *preload_flags, uint32_t *triggering_flags,
												int fifo, int high_range,
												struct workqueue_struct* me6000_wq);

# endif //_ME6000_AO_H_
#endif //__KERNEL__
