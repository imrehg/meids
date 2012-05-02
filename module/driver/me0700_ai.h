/**
 * @file me0700_ai.h
 *
 * @brief The ME-0700 analog input subdevice class.
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

# ifndef _ME0700_AI_H_
#  define _ME0700_AI_H_

#  include <linux/version.h>

#  include "medevice.h"
#  include "mesubdevice.h"
#  include "mecirc_buf.h"
#  include "me_interrupt_types.h"
#  include "me_spin_lock.h"

# include "me4600_ai.h"
# include "me4600_dio.h"
# include "me4600_ext_irq.h"

// ME-0700 #defines
#define ME0700_AI_EXTRA_RANGE_NUMBER			8
#define ME0700_AI_RANGE_INVALID					0xF0
#define ME0700_AI_RANGE_NONE					0xE0

#define ME0700_AI_RANGE_500_AMPERE				0x00
#define ME0700_AI_RANGE_50_AMPERE				0x10
#define ME0700_AI_RANGE_25_AMPERE				0x20
#define ME0700_AI_RANGE_2500_MILLIAMPERE		0x30
#define ME0700_AI_RANGE_250_MILLIAMPERE			0x40
#define ME0700_AI_RANGE_250X_MILLIAMPERE		0x50
#define ME0700_AI_RANGE_25_MILLIAMPERE			0x60
#define ME0700_AI_RANGE_2500_MICROAMPERE		0x70
#define ME0700_AI_RANGE_250_MICROAMPERE			0x80
// #define ME0700_AI_RANGE_125_MICROAMPERE			0x90

#  define ME0700_AI_MAX_DATA					ME4600_AI_MAX_DATA
#  define ME0700_RANGE_INACCURACY				ME4600_RANGE_INACCURACY


	/**
	* @brief The ME-0700 analog input subdevice class.
	*/
	typedef struct //me0700_ai_subdevice
	{
		// Inheritance
		me_subdevice_t base;							/**< The subdevice base class. */

		// Attributes
		me4600_ai_subdevice_t*		ai_instance;
		me4600_dio_subdevice_t*		ctrl_port;
		me4600_dio_subdevice_t*		data_port;
		me4600_ext_irq_subdevice_t*	ext_irq;

		int number_I_channels;
		int me0700_ranges[4];
		me_lock_t me0700_bus_lock;				// Locks paraller bus.

		me4600_range_entry_t ranges[ME0700_AI_EXTRA_RANGE_NUMBER];					/**< The ranges available on this subdevice. */
		int me4600_ranges_number;
		int number_channels;
		int irq_status_flag;
		int irq_rised;
		uint8_t irq_status;

	} me0700_ai_subdevice_t;

	/**
	* @brief The constructor to generate a ME-0700 analog input subdevice instance.
	*
	* @param reg_base The register base address of the device as returned by the PCI BIOS.
	* @param idx Subdevice number.
	* @param channels The number of analog input channels available on this subdevice.
	* @param ranges The number of analog input ranges available on this subdevice.
	* @param isolated Flag indicating if this device is opto isolated.
	* @param sh Flag indicating if sample and hold devices are available.
	* @param me0700_wq Queue for asynchronous task (1 queue for all subdevices on 1 board).
	* @param dio_lock Spin lock protecting the DIO control register.
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	me0700_ai_subdevice_t* me0700_ai_constr(void* reg_base,
												void* DMA_base,
												void* PLX_base,
												unsigned int idx,
												unsigned int channels, unsigned int ranges,
												int features,
												struct workqueue_struct* me0700_wq,
												me_general_dev_t* dev,
												me_lock_t* dio_lock,
												unsigned int I_channels);


#define ME0700_SAMPLE_HOLD		0x0001
#define ME0700_ANALOG_TRIGGER	0x0002
#define ME0700_DIFFERENTIAL		0x0004
#define ME0700_ISOLATED			0x0008

# endif
#endif
