/**
 * @file me4600_ai_CosateQ.h
 *
 * @brief The ME-4600 analog input subdevice class.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
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

# ifndef _ME4600_AI_H_
#  define _ME4600_AI_H_

#  include <linux/version.h>

#  include "medevice.h"
#  include "mesubdevice.h"
#  include "mecirc_buf.h"
#  include "me_interrupt_types.h"

#  define ME4600_AI_MAX_DATA			0xFFFF

#define ME4600_AI_CALIB_START			0x32

#define ME4600_AI_CALIB_UNIPOLAR_10_A	32702
#define ME4600_AI_CALIB_UNIPOLAR_10_B	32440

#define ME4600_AI_CALIB_BIPOLAR_10_A	0
#define ME4600_AI_CALIB_BIPOLAR_10_B	32604

#define ME4600_AI_CALIB_UNIPOLAR_2_5_A	32505
#define ME4600_AI_CALIB_UNIPOLAR_2_5_B	31457

#define ME4600_AI_CALIB_BIPOLAR_2_5_A	0
#define ME4600_AI_CALIB_BIPOLAR_2_5_B	32113

# define ME4600_RANGE_INACCURACY		14



#  ifdef ME_SYNAPSE
#   define ME4600_AI_CIRC_BUF_SIZE_ORDER	8											/* 2^n PAGES =>> Maximum value of 1MB for Synapse */
#  else
#   define ME4600_AI_CIRC_BUF_SIZE_ORDER	5											/* 2^n PAGES =>> 128KB */
#  endif
#  define ME4600_AI_CIRC_BUF_SIZE 		PAGE_SIZE<<ME4600_AI_CIRC_BUF_SIZE_ORDER		/* Buffer size in bytes */

#  ifdef _CBUFF_32b_t
#   define ME4600_AI_CIRC_BUF_COUNT		((ME4600_AI_CIRC_BUF_SIZE) / sizeof(uint32_t))	/* Size in values */
#  else
#   define ME4600_AI_CIRC_BUF_COUNT		((ME4600_AI_CIRC_BUF_SIZE) / sizeof(uint16_t))	/* Size in values */
#  endif

#  define me4600_AI_CAPS				(ME_CAPS_AI_FIFO | ME_CAPS_AI_FIFO_THRESHOLD/* | ME_CAPS_AI_TRIG_DIGITAL*/)

	enum ME4600_AI_STATUS
	{
		ai_status_none,
		ai_status_configured,
		ai_status_run,
		ai_status_end,
		ai_status_configured_polling,
		ai_status_run_polling,
		ai_status_end_polling,
		ai_status_error,
		ai_status_last
	};

	typedef struct //me4600_range_entry
	{
		int min;
		int max;
	} me4600_range_entry_t;

	typedef struct //me4600_ai_timeout
	{
		unsigned long start_time;
		unsigned long int delay;
	}me4600_ai_timeout_t;

	typedef struct //me_calibration_entry
	{
		long int constant;
		long int multiplier;
		long int divisor;
	} me_calibration_entry_t;

	typedef struct //me4600_ai_calibration
	{
		me_calibration_entry_t unipolar_10;
		me_calibration_entry_t bipolar_10;
		me_calibration_entry_t differential_10;

		me_calibration_entry_t unipolar_2_5;
		me_calibration_entry_t bipolar_2_5;
		me_calibration_entry_t differential_2_5;
	} me4600_ai_calibration_t;

	typedef struct //me4600_ai_eeprom
	{
		uint16_t year;
		uint16_t month;
		int16_t unipolar_10A;
		int16_t unipolar_10B;
		int16_t bipolar_10A;
		int16_t bipolar_10B;
		int16_t differential_10A;
		int16_t differential_10B;
		int16_t unipolar_2_5A;
		int16_t unipolar_2_5B;
		int16_t bipolar_2_5A;
		int16_t bipolar_2_5B;
		int16_t differential_2_5A;
		int16_t differential_2_5B;
	}__attribute__((packed)) me4600_ai_eeprom_t;

	/**
	* @brief The ME-4600 analog input subdevice class.
	*/
	typedef struct //me4600_ai_subdevice
	{
		// Inheritance
		me_subdevice_t base;							/**< The subdevice base class. */

		// Attributes

		// Hardware feautres
		int isolated;									/**< Marks if this subdevice is on an optoisolated device. */
		int features;									/**< Marks if this subdevice has extra features: sample and hold devices or analog triggering. */

		unsigned int channels;							/**< The number of channels available on this subdevice. */

		uint32_t single_ctrl;
		uint32_t single_status;
		uint32_t single_entry[32];
		unsigned int single_value[32];						/**< The number of data request by user. */

		volatile enum ME4600_AI_STATUS status;			/**< The current stream status flag. */
		me4600_ai_timeout_t timeout;					/**< The timeout for start in blocking and non-blocking mode. */
		int me4600_ai_error_confirm;					/**< The safety timeout for signaling an error (FIFO not empty and ISM not working). */

		// Registers									/**< All registers are 32 bits long. */
		void* ctrl_reg;
		void* status_reg;
		void* channel_list_reg;
		void* data_reg;
		void* chan_timer_reg;
		void* chan_pre_timer_reg;
		void* scan_timer_low_reg;
		void* scan_timer_high_reg;
		void* scan_pre_timer_low_reg;
		void* scan_pre_timer_high_reg;
		void* start_reg;
		void* sample_counter_reg;

		void* DMA_base;
		void* PLX_base;

		void* irq_status_reg;

		unsigned int ranges_len;
		me4600_range_entry_t ranges[4];					/**< The ranges available on this subdevice. */

		me4600_ai_calibration_t calibration;
		int raw_values;

#ifdef MEDEBUG_SPEED_TEST
		volatile uint64_t int_start, int_end;
#endif
	} me4600_ai_CosateQ_subdevice_t;

	/**
	* @brief The constructor to generate a ME-4600 analog input subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx Subdevice number.
	* @param channels The number of analog input channels available on this subdevice.
	* @param ranges The number of analog input ranges available on this subdevice.
	* @param isolated Flag indicating if this device is opto isolated.
	* @param sh Flag indicating if sample and hold devices are available.
	* @param me4600_wq Queue for asynchronous task (1 queue for all subdevices on 1 board).
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	me4600_ai_CosateQ_subdevice_t* me4600_ai_constr(void* reg_base,
												void* DMA_base,
												void* PLX_base,
												unsigned int idx,
												unsigned int channels, unsigned int ranges,
												int features,
												/* me4600_interrupt_status_t* int_status, */
												struct workqueue_struct* me4600_wq);


#define ME4600_NONE				0x0000
#define ME4600_SAMPLE_HOLD		0x0001
#define ME4600_ANALOG_TRIGGER	0x0002
#define ME4600_DIFFERENTIAL		0x0004
#define ME4600_ISOLATED			0x0008

# endif
#endif
