/**
 * @file mephisto_ai.h
 *
 * @brief The MephistoScope analog input subdevice class.
 * @note Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# ifndef _MEPHISTO_AI_H_
#  define _MEPHISTO_AI_H_

#  include <linux/version.h>

#  include "medevice.h"
#  include "mesubdevice.h"
#  include "meseg_buf.h"
#  include "me_interrupt_types.h"

#  define MEPHISTO_AI_MAX_DATA			0xFFFFFFFF

#  define MEPHISTO_AI_SEG_BUF_CHUNK_SIZE		(PAGE_SIZE)
#  define MEPHISTO_AI_SEG_BUF_CHUNK_COUNT		(128)

#  define mephisto_AI_CAPS				(ME_CAPS_AI_FIFO | ME_CAPS_AI_FIFO_THRESHOLD | ME_CAPS_AI_TRIG_ANALOG)

#define MEPHISTO_NUMBER_RANGES	7

	/**
	* @brief The MephistoScope analog input subdevice class.
	*/
	typedef struct //mephisto_ai_subdevice
	{
		// Inheritance
		me_subdevice_t base;							/**< The subdevice base class. */

		struct semaphore* 	device_semaphore;
		volatile mephisto_AI_status_e* status;

	// Configuration
		mephisto_mode_e		mode;
		unsigned long int	timeout;
		MEPHISTO_modes_tu	offset[2][MEPHISTO_NUMBER_RANGES];
		int 				range[2];
		int					channels_count;
		int					trigger_channel;

		MEPHISTO_modes_tu	trigger_type;

		long long int		time_base;

		unsigned int		trigger_point;

		int					trigger_upper_level;
		int					trigger_lower_level;

		unsigned int		single_range[2];
		unsigned int		use_RMS : 1;


		// Software buffer
		struct semaphore 	buffer_semaphore;
		me_seg_buf_t*		seg_buf;							/**< Segmented circular buffer holding measurment data. */
		wait_queue_head_t	wait_queue;					/**< Wait queue to put on tasks waiting for data to arrive. */


		// Processing
		unsigned int stream_start_count;
		unsigned int stream_stop_count;
		unsigned int empty_read_count : 1;

		unsigned int data_required;
		unsigned int data_recived;
		unsigned int threshold;

		struct workqueue_struct* mephisto_workqueue;
		struct work_struct mephisto_stream;

	} mephisto_ai_subdevice_t;

	/**
	* @brief The constructor to generate a MephistoScope analog input subdevice instance.
	*
	* @param idx Subdevice number.
	*
	* @return Pointer to new instance on success.\n
	* NULL on error.
	*/
	mephisto_ai_subdevice_t* mephisto_ai_constr(unsigned int idx, mephisto_AI_status_e* status, struct semaphore* device_semaphore);


# endif
#endif
