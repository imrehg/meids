/**
 * @file mephisto_ai.c
 *
 * @brief The MephistoScope analog input subdevice instance.
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/fs.h>
# include <linux/slab.h>


# include <linux/delay.h>
# include <linux/sched.h>
# include <linux/workqueue.h>
# include <asm/uaccess.h>
# include <asm/msr.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mephisto_access.h"
# include "mephisto_ai.h"

const MEPHISTO_modes_tu MEPHISTO_modes[] = {
	{.text = "0DMV"},	// 0
	{.text = "1DMV"},	// 1
	{.text = "0AMV"},	// 2
	{.text = "1AMV"},	// 3
	{.text = "0ASO"},	// 4
	{.text = "0ALD"},	// 5
	{.text = "OIAL"},	// 6
	{.text = "IDLD"},	// 7
	{0}
};

const unsigned int mephisto_ranges[MEPHISTO_NUMBER_RANGES] = {200000, 500000, 1000000, 2000000, 5000000, 10000000, 20000000};

/// Declarations

static void mephisto_ai_destructor(me_subdevice_t* subdevice);
int mephisto_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);

static int mephisto_ai_io_single_config_check(mephisto_ai_subdevice_t* instance, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int mephisto_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);

static int mephisto_ai_io_single_read_check(mephisto_ai_subdevice_t* instance, int channel,  int* value, int time_out, int flags);
int mephisto_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
static int mephisto_ai_io_stream_config_check(mephisto_ai_subdevice_t* instance,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);
int mephisto_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t *config_list, int count, meIOStreamSimpleTriggers_t *trigger, int fifo_irq_threshold, int flags);

static int mephisto_ai_io_stream_read_check(mephisto_ai_subdevice_t* instance, int read_mode, int* values, int* count, int time_out, int flags);
int mephisto_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags);

int mephisto_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags);
int mephisto_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags);
int mephisto_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags);

int mephisto_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags);
static int inline mephisto_ai_io_stream_read_get_value(mephisto_ai_subdevice_t* instance, int* values, const int count, const int flags);

int mephisto_ai_set_offset(me_subdevice_t* subdevice, struct file* filep, int channel, int range, int* offset, int flags);

int mephisto_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
int mephisto_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
int mephisto_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
int mephisto_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
int mephisto_ai_query_number_channels(me_subdevice_t* subdevice, int *number);
int mephisto_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int mephisto_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int mephisto_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count);
int mephisto_ai_postinit(me_subdevice_t* subdevice, void* args);


void mephisto_stream(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
									void* subdevice
#else
									struct work_struct* work
#endif
								);

static void mephisto_ai_destructor(me_subdevice_t* subdevice)
{
	mephisto_ai_subdevice_t* instance;

	instance = (mephisto_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (!instance)
	{
		return;
	}

	down(instance->device_semaphore);
		if ((*instance->status == MEPHISTO_AI_STATUS_start) || (*instance->status == MEPHISTO_AI_STATUS_run))
		{
			PDEBUG("Cancel stream task.\n");
			mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Break, NULL, 0, NULL, 0);
			if (wait_event_interruptible_timeout(instance->wait_queue, *instance->status < MEPHISTO_AI_STATUS_start, HZ << 2) <= 0)
			{
				mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Restart, NULL, 0, NULL, 0);
				// work canceling API changed in kernel 2.6.26
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
				cancel_delayed_work(&instance->mephisto_stream);
#else
				cancel_work_sync(&instance->mephisto_stream);
#endif
			}
		}
	up(instance->device_semaphore);

	//Wait 2 ticks to be sure that control task is removed from queue.
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(2);

	destroy_seg_buffer(&instance->seg_buf);
	me_subdevice_deinit(&instance->base);

	flush_workqueue(instance->mephisto_workqueue);
	destroy_workqueue(instance->mephisto_workqueue);
}

int mephisto_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	mephisto_ai_subdevice_t* instance;

	int i;

	AmplitudeOffset_arg_send_t		amplitude_send;
	AmplitudeOffset_arg_recive_t	amplitude_recive;
	AmplitudeOffset_arg_send_t		offset_send;
	AmplitudeOffset_arg_recive_t	offset_recive;
	Trigger_arg_send_t				trigger_send;
	Trigger_arg_recive_t			trigger_recive;
	SetMode_arg_t					mode_send;
	SetMode_arg_t					mode_recive;

	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (mephisto_ai_subdevice_t *)subdevice;

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if ((*instance->status == MEPHISTO_AI_STATUS_start) || (*instance->status == MEPHISTO_AI_STATUS_run))
			{
				PDEBUG("Cancel stream task.\n");
				mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Break, NULL, 0, NULL, 0);
				if (wait_event_interruptible_timeout(instance->wait_queue, *instance->status < MEPHISTO_AI_STATUS_start, HZ << 2) <= 0)
				{
					// work canceling API changed in kernel 2.6.26
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
					cancel_delayed_work(&instance->mephisto_stream);
#else
					cancel_work_sync(&instance->mephisto_stream);
#endif
				}
			}
			*instance->status = MEPHISTO_AI_STATUS_idle;

			memcpy(&mode_send, &MEPHISTO_modes[1], sizeof(SetMode_arg_t));
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

			amplitude_send.channel.value = 0;
			amplitude_send.value = uvolts_to_float(mephisto_ranges[MEPHISTO_NUMBER_RANGES - 1]);
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetAmplitude, (void *)&amplitude_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&amplitude_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

			amplitude_send.channel.value = 1;
			amplitude_send.value = uvolts_to_float(mephisto_ranges[MEPHISTO_NUMBER_RANGES - 1]);
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetAmplitude, (void *)&amplitude_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&amplitude_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

			offset_send.channel.value = 0;
			offset_send.value.value = 0;
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetOffset, (void *)&offset_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&offset_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

			offset_send.channel.value = 1;
			offset_send.value.value = 0;
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetOffset, (void *)&offset_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&offset_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

			memset(&trigger_send.trigger_type, 'M', 4);
			trigger_send.channel.value = 0;
			trigger_send.upper_trigger_level.value = 0;
			trigger_send.lower_trigger_level.value = 0;
			if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetTrigger, (void *)&trigger_send, sizeof(Trigger_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&trigger_recive, sizeof(Trigger_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
			{
				err = ME_ERRNO_COMMUNICATION;
				goto ERROR;
			}

ERROR:
			//Set parameters
			instance->mode = MEPHISTO_MODE_voltmeter_raw;
			instance->channels_count = 0;
			instance->range[0] = - 1;
			instance->range[1] = - 1;

			for (i=0; i<MEPHISTO_NUMBER_RANGES; ++i)
			{
				instance->offset[0][i].value = 0;
				instance->offset[1][i].value = 0;
			}

 			instance->threshold = 0;
			instance->data_required = 0;
			instance->data_recived = 0;

			me_seg_buf_reset(instance->seg_buf);

		up(instance->device_semaphore);

		//Signal reset if user is on wait.
		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return err;
}

static int mephisto_ai_io_single_config_check(mephisto_ai_subdevice_t* instance, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	if (flags & ~(ME_IO_SINGLE_CONFIG_AI_RMS | ME_IO_SINGLE_CONFIG_CONTINUE))
	{
		PERROR("Invalid flags. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS, ME_IO_SINGLE_CONFIG_AI_RMS or ME_IO_SINGLE_CONFIG_CONTINUE.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
		case ME_TRIG_TYPE_SW:
			if (trig_edge != ME_TRIG_EDGE_NONE)
			{
				PERROR("Invalid trigger edge. Software trigger has not edge. Must be ME_TRIG_EDGE_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_EDGE;
			}
			break;

		default:
			PERROR("Invalid trigger type specified.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	if ((trig_type == ME_TRIG_TYPE_NONE) && (trig_chain != ME_TRIG_CHAN_NONE))
	{
		PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_NONE.\n");
		return ME_ERRNO_INVALID_TRIG_CHAN;
	}

	if ((trig_type != ME_TRIG_TYPE_NONE) && (trig_chain != ME_TRIG_CHAN_DEFAULT))
	{
		PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT.\n");
		return ME_ERRNO_INVALID_TRIG_CHAN;
	}

	if ((single_config < 0) || (single_config >= MEPHISTO_NUMBER_RANGES))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", MEPHISTO_NUMBER_RANGES - 1);
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (ref != ME_REF_AI_GROUND)
	{
		PERROR("Invalid reference specified. Must be ME_REF_AI_GROUND.\n");
		return ME_ERRNO_INVALID_REF;
	}


	if (!((channel == 0) || (channel == 1)))
	{
		PERROR("Invalid channel specified. Must be between 0 or 1.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	mephisto_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (mephisto_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	err = mephisto_ai_io_single_config_check(instance, channel, single_config, ref, trig_chain, trig_type, trig_edge, flags);
	if (err)
		return err;

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			instance->single_range[channel] = single_config;
			if ((channel == 0) && (flags | ME_IO_SINGLE_CONFIG_CONTINUE))
			{
				instance->single_range[1] = single_config;
			}
			instance->use_RMS = (flags | ME_IO_SINGLE_CONFIG_AI_RMS) ? 1 : 0;
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

static int mephisto_ai_io_single_read_check(mephisto_ai_subdevice_t* instance, int channel,  int* value, int time_out, int flags)
{
	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (!((channel == 0) || (channel == 1)))
	{
		PERROR("Invalid channel specified. Must be between 0 or 1.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel,  int* value, int time_out, int flags)
{
	mephisto_ai_subdevice_t* instance;
	int mode;
	int err = ME_ERRNO_SUCCESS;

	SetMode_arg_t					mode_send;
	SetMode_arg_t					mode_recive;
	AmplitudeOffset_arg_send_t		amplitude_send;
	AmplitudeOffset_arg_recive_t	amplitude_recive;
	AmplitudeOffset_arg_send_t		offset_send;
	AmplitudeOffset_arg_recive_t	offset_recive;
	Trigger_arg_send_t				trigger_send;
	Trigger_arg_recive_t			trigger_recive;
	MEPHISTO_modes_tu				packet[2];

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	/// In this case I HAVE TO ignore non_blocking flag
	flags &= ~ME_IO_SINGLE_TYPE_NONBLOCKING;

	err = mephisto_ai_io_single_read_check(instance, channel,  value, time_out, flags);
	if (err)
	{
    	return err;
	}

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				mode = (instance->use_RMS == 0) ? 1 : 3;
				memcpy(&mode_send, &MEPHISTO_modes[mode], sizeof(SetMode_arg_t));
				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				amplitude_send.channel.value = channel;
				amplitude_send.value = uvolts_to_float(mephisto_ranges[instance->single_range[channel]]);
				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetAmplitude, (void *)&amplitude_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&amplitude_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				offset_send.channel.value = channel;
				offset_send.value = uvolts_to_float(instance->offset[channel][instance->single_range[channel]].value);
				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetOffset, (void *)&offset_send, sizeof(AmplitudeOffset_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&offset_recive, sizeof(AmplitudeOffset_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				memset(&trigger_send.trigger_type, 'M', 4);
				trigger_send.channel.value = 0;
				trigger_send.upper_trigger_level.value = 0;
				trigger_send.lower_trigger_level.value = 0;
				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetTrigger, (void *)&trigger_send, sizeof(Trigger_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&trigger_recive, sizeof(Trigger_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Run, NULL, 0, (void *)packet, 2))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}
				if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Break, NULL, 0, NULL, 0))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				*value = packet[channel].svalue[0];
			}
			else
			{
				PERROR("Subdevice is streaming!\n");
				err = ME_ERRNO_SUBDEVICE_BUSY;

			}

ERROR:
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

static int mephisto_ai_io_stream_config_check(mephisto_ai_subdevice_t* instance, meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	int i;
	int err = ME_ERRNO_SUCCESS;

	if (flags & ~ME_IO_STREAM_CONFIG_BIT_PATTERN)
	{
		PERROR("Invalid flags. Should be ME_IO_STREAM_CONFIG_NO_FLAGS or ME_IO_STREAM_CONFIG_BIT_PATTERN.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	switch (trigger->trigger_type)
	{
		case ME_TRIGGER_TYPE_SOFTWARE:
			if (trigger->trigger_edge != ME_TRIG_EDGE_NONE)
			{
				PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_NONE.\n");
				err = ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
				goto ERROR;
			}
			if (trigger->trigger_point != 0)
			{
				PERROR("Invalid acquisition start trigger argument specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_ACQ_START_ARG;
				goto ERROR;
			}
			break;

		case ME_TRIGGER_TYPE_ACQ_DIGITAL:
		case ME_TRIGGER_TYPE_ACQ_EDGE:
		case ME_TRIGGER_TYPE_ACQ_SLOPE:
			switch (trigger->trigger_edge)
			{
				case ME_TRIG_EDGE_RISING:
				case ME_TRIG_EDGE_FALLING:
					break;

				default:
					PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_RISING or ME_TRIG_EDGE_FALLING.\n");
					err = ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
					goto ERROR;
			}
			break;

		case ME_TRIGGER_TYPE_ACQ_THRESHOLD:
			switch (trigger->trigger_edge)
			{
				case ME_TRIG_EDGE_ABOVE:
				case ME_TRIG_EDGE_BELOW:
					break;

				default:
					PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_ABOVE or ME_TRIG_EDGE_BELOW.\n");
					err = ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
					goto ERROR;
			}
			break;

		case ME_TRIGGER_TYPE_ACQ_WINDOW:
			switch (trigger->trigger_edge)
			{
				case ME_TRIG_EDGE_ENTRY:
				case ME_TRIG_EDGE_EXIT:
					break;

				default:
					PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_ENTRY or ME_TRIG_EDGE_EXIT.\n");
					err = ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
					goto ERROR;
			}
			break;

		default:
			PERROR("Invalid acquisition trigger type specified.\n");
			err = ME_ERRNO_INVALID_ACQ_START_TRIG_TYPE;
			goto ERROR;
	}

	if ((trigger->synchro != ME_TRIG_CHAN_DEFAULT) && (trigger->synchro != ME_TRIG_CHAN_NONE))
	{
		PERROR("Invalid acquisition start trigger channel specified. Should be ME_TRIG_CHAN_DEFAULT.\n");
		err = ME_ERRNO_INVALID_ACQ_START_TRIG_CHAN;
		goto ERROR;
	}

	// Start delay
	if (trigger->acq_ticks != 0)
	{
		PERROR("Invalid acquisition start trigger argument specified. Must be 0.\n");
		err = ME_ERRNO_INVALID_ACQ_START_ARG;
		goto ERROR;
	}

	if (trigger->scan_ticks != 0)
	{
		PERROR("Invalid scan start argument specified. Must be 0.\n");
		err = ME_ERRNO_INVALID_SCAN_START_ARG;
		goto ERROR;
	}

	if (flags)
	{
		if ((trigger->conv_ticks != 0)
			&&
			(
				(trigger->conv_ticks < 10)
				||
				(trigger->conv_ticks > 2500000))
			)
		{
			PERROR("Invalid conv start trigger argument specified. Must be between %d and %d.\n", 10, 2500000);
			err = ME_ERRNO_INVALID_CONV_START_ARG;
			goto ERROR;
		}
	}
	else
	{
		if ((trigger->conv_ticks != 0)
			&&
			(
				(trigger->conv_ticks < 1)
				||
				(trigger->conv_ticks > 2500000))
			)
		{
			PERROR("Invalid conv start trigger argument specified. Must be between %d and %d.\n", 1, 2500000);
			err = ME_ERRNO_INVALID_CONV_START_ARG;
			goto ERROR;
		}
	}

	switch (trigger->stop_type)
	{
		case ME_STREAM_STOP_TYPE_SCAN_VALUE:
			if (!flags)
			{
				if ((trigger->stop_count < (100 * count)) || (trigger->stop_count > (131000 * count)))
				{
					PERROR("Invalid scan stop argument specified (%d). Minimum value is 100. Maximum value is 131000.\n", trigger->stop_count);
					err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
					goto ERROR;
				}
			}
			else
			{
				if ((trigger->stop_count < 100) || (trigger->stop_count > 262000))
				{
					PERROR("Invalid scan stop argument specified (%d). Minimum value is 100. Maximum value is 262000.\n", trigger->stop_count);
					err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
					goto ERROR;
				}
			}
			break;

		case ME_STREAM_STOP_TYPE_ACQ_LIST:
			if (!flags)
			{
				if ((trigger->stop_count < 100) || (trigger->stop_count > 131000))
				{
					PERROR("Invalid scan stop argument specified (%d). Minimum value is 100. Maximum value is 131000.\n", trigger->stop_count);
					err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
					goto ERROR;
				}
			}
			else
			{
				PERROR("Invalid stop trigger type specified.\n");
				err = ME_ERRNO_INVALID_SCAN_STOP_TRIG_TYPE;
				goto ERROR;
			}
			break;

		case ME_STREAM_STOP_TYPE_MANUAL:
			if (trigger->stop_count != 0)
			{
				PERROR("Invalid stop argument specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
				goto ERROR;
			}
			break;

		default:
			PERROR("Invalid stop trigger type specified.\n");
			err = ME_ERRNO_INVALID_SCAN_STOP_TRIG_TYPE;
			goto ERROR;
			break;
	}

	if((count <= 0)||(count > 2))
	{
		PERROR("Invalid channel list count specified. Must be 1 or 2.\n");
		err = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR;
	}

	if (fifo_irq_threshold < 0 || fifo_irq_threshold >= me_seg_buf_size(instance->seg_buf))
	{
		PERROR("Invalid fifo irq threshold specified. Must be between 0 and %d.\n", me_seg_buf_size(instance->seg_buf) - 1);
		err = ME_ERRNO_INVALID_FIFO_IRQ_THRESHOLD;
		goto ERROR;
	}

	if (count == 1)
	{
		if ((config_list[0].iChannel < 0) || (config_list[0].iChannel >= 2))
		{
			PERROR("Invalid channel specified. Must be 0 or 1.\n");
			err = ME_ERRNO_INVALID_CHANNEL;
			goto ERROR;
		}

		if (flags && config_list[0].iChannel)
		{
			PERROR("Invalid channel specified. Channel has to be set to 0 when flag ME_IO_STREAM_CONFIG_BIT_PATTERN is set.\n");
			err = ME_ERRNO_INVALID_CHANNEL;
			goto ERROR;
		}
	}
	else
	{
		if (flags)
		{
			PERROR("Invalid stream configuration. Only one entry is possible.\n");
			err = ME_ERRNO_INVALID_STREAM_CONFIG;
			goto ERROR;
		}

		if ((config_list[0].iChannel < 0) || (config_list[0].iChannel >= 2) || (config_list[1].iChannel < 0) || (config_list[1].iChannel >= 2))
		{
			PERROR("Invalid channel specified. Must be 0 or 1.\n");
			err = ME_ERRNO_INVALID_CHANNEL;
			goto ERROR;
		}
		if (config_list[0].iChannel == config_list[1].iChannel)
		{
			PERROR("Invalid stream configuration. The same channel in both entries.\n");
			err = ME_ERRNO_INVALID_STREAM_CONFIG;
			goto ERROR;
		}
	}

	for (i = 0; i < count; i++)
	{
		if ((config_list[i].iRange < 0) || (config_list[i].iRange >= MEPHISTO_NUMBER_RANGES))
		{
			PERROR("Invalid range specified. Must be between 0 and %d.\n", MEPHISTO_NUMBER_RANGES - 1);
			err = ME_ERRNO_INVALID_STREAM_CONFIG;
			goto ERROR;
		}
	}

ERROR:
	return err;
}

int mephisto_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	mephisto_ai_subdevice_t* instance;
	int i;			// internal multipurpose variable

	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	err = mephisto_ai_io_stream_config_check(instance, config_list, count,  trigger, fifo_irq_threshold, flags);
	if (err)
		return err;

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				if (!flags)
				{
					if (trigger->stop_type == ME_STREAM_STOP_TYPE_MANUAL)
					{
						instance->mode = MEPHISTO_MODE_data_logger;
					}
					else
					{
						instance->mode = MEPHISTO_MODE_oscilloscope;
					}
				}
				else
				{
					if (trigger->stop_type == ME_STREAM_STOP_TYPE_MANUAL)
					{
						instance->mode = MEPHISTO_MODE_digital_logger;
					}
					else
					{
						instance->mode = MEPHISTO_MODE_logic_analizer;
					}
				}

				instance->range[0] = -1;
				instance->range[1] = -1;

				for (i=0; i<count; ++i)
				{
					instance->range[config_list[i].iChannel] = config_list[i].iRange;
				}
				instance->channels_count = count;

				switch (trigger->trigger_type)
				{
					case ME_TRIGGER_TYPE_ACQ_DIGITAL:
						switch (trigger->trigger_edge)
						{
							case ME_TRIG_EDGE_RISING:
								memcpy(instance->trigger_type.text, "XXXX", 4);
								break;

							case ME_TRIG_EDGE_FALLING:
								memcpy(instance->trigger_type.text, "xxxx", 4);
								break;
						}
						break;

					case ME_TRIGGER_TYPE_ACQ_THRESHOLD:
						switch (trigger->trigger_edge)
						{
							case ME_TRIG_EDGE_ABOVE:
								memcpy(instance->trigger_type.text, "TTTT", 4);
								break;

							case ME_TRIG_EDGE_BELOW:
								memcpy(instance->trigger_type.text, "tttt", 4);
								break;
						}
						break;

					case ME_TRIGGER_TYPE_ACQ_EDGE:
						switch (trigger->trigger_edge)
						{
							case ME_TRIG_EDGE_RISING:
								memcpy(instance->trigger_type.text, "EEEE", 4);
								break;

							case ME_TRIG_EDGE_FALLING:
								memcpy(instance->trigger_type.text, "eeee", 4);
								break;
						}
						break;

					case ME_TRIGGER_TYPE_ACQ_SLOPE:
						switch (trigger->trigger_edge)
						{
							case ME_TRIG_EDGE_RISING:
								memcpy(instance->trigger_type.text, "DDDD", 4);
								break;

							case ME_TRIG_EDGE_FALLING:
								memcpy(instance->trigger_type.text, "dddd", 4);
								break;
						}
						break;

					case ME_TRIGGER_TYPE_ACQ_WINDOW:
						switch (trigger->trigger_edge)
						{
							case ME_TRIG_EDGE_ENTRY:
								memcpy(instance->trigger_type.text, "WWWW", 4);
								break;

							case ME_TRIG_EDGE_EXIT:
								memcpy(instance->trigger_type.text, "wwww", 4);
								break;
						}
						break;

					case ME_TRIGGER_TYPE_SOFTWARE:
					default:
						memcpy(instance->trigger_type.text, "MMMM", 4);
						break;
				}

				switch (trigger->stop_type)
				{
					case ME_STREAM_STOP_TYPE_SCAN_VALUE:
						instance->data_required = (unsigned int)trigger->stop_count;
						break;

					case ME_STREAM_STOP_TYPE_ACQ_LIST:
						instance->data_required = (unsigned int)trigger->stop_count * instance->channels_count;
						break;

					default:
						instance->data_required = 0;
				}

				instance->time_base = trigger->conv_ticks;
				instance->trigger_lower_level = trigger->trigger_level_lower;
				instance->trigger_upper_level = trigger->trigger_level_upper;
				instance->trigger_point = trigger->trigger_point;

				instance->trigger_channel = trigger->synchro;
				instance->threshold = fifo_irq_threshold;

				*instance->status = MEPHISTO_AI_STATUS_configured;
			}
			else
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
			}
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags)
{
	mephisto_ai_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long int j;
	unsigned int writes_count;
	mephisto_AI_status_e status;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");
	PDEBUG("flags:%s%s\n", (flags & ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG)?" NEW_VALUES_SCREEN_FLAG":"", (flags & ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG)?" NEW_VALUES_ERROR_REPORT_FLAG":"");

	if (flags & ~(ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG | ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG))
	{
		PERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (time_out)
	{
		delay = (time_out * HZ) / 1000;
		if (!delay)
			delay = 1;
		if (delay>LONG_MAX - 2)
			delay = LONG_MAX - 2;
	}

	instance = (mephisto_ai_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		j = jiffies;

		while(1)
		{
			status = *instance->status;
			if (flags &  ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG)
			{// Report errors
				if (status == MEPHISTO_AI_STATUS_fifo_error)
				{
					err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
				}
				else if (status == MEPHISTO_AI_STATUS_buffer_error)
				{
					err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
				}
				else if (status == MEPHISTO_AI_STATUS_timeout)
				{
					err = ME_ERRNO_CANCELLED;
				}
				else if (status == MEPHISTO_AI_STATUS_idle)
				{
					err = ME_ERRNO_CANCELLED;
				}
				if (err)
				{
					break;
				}
			}
			// Only runing device can generate break.
			writes_count = instance->seg_buf->header.writes_count;
			if (flags & ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG)
			{
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(
						(writes_count != instance->seg_buf->header.writes_count)
						||
						(status != *instance->status)
					),
					delay);
			}
			else
			{
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(
						me_seg_buf_values(instance->seg_buf)
						||
						(status != *instance->status)
					),
					delay);
			}

			if (signal_pending(current))
			{
				PERROR("Wait on values interrupted.\n");
				err = ME_ERRNO_SIGNAL;
				*count = 0;
				goto ERROR;
			}

			if (*instance->status == MEPHISTO_AI_STATUS_fifo_error)
			{
				err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
			}
			else if (*instance->status == MEPHISTO_AI_STATUS_buffer_error)
			{
				err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
			}
			else if (*instance->status == MEPHISTO_AI_STATUS_idle)
			{
				err = ME_ERRNO_CANCELLED;
			}
			else if ((jiffies - j) >= delay)
			{
				PERROR("Wait on values timed out.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if ((writes_count != instance->seg_buf->header.writes_count) || (!flags && me_seg_buf_values(instance->seg_buf)))
			{// New data in buffer.
				break;
			}

			if (err || (*instance->status == MEPHISTO_AI_STATUS_configured))
			{
				break;
			}
			// Correct timeout.
			delay -= jiffies - j;
		}

		*count = me_seg_buf_values(instance->seg_buf);
ERROR:
	ME_SUBDEVICE_EXIT;

	PDEBUG("count=%d err = %d\n", *count, err);
	return err;
}

static int inline mephisto_ai_io_stream_read_get_value(mephisto_ai_subdevice_t* instance, int* values, const int count, const int flags)
{
	unsigned int n;
	int i;
	uint16_t tmp;
	int value;

	///Checking how many datas can be copied.
	n = me_seg_buf_values(instance->seg_buf);
	if (n <= 0)
		return 0;

	if (n > count)
		n = count;

	if (flags & ME_IO_STREAM_READ_FRAMES)
	{
		if (n < instance->channels_count)	//Not enough data!
			return 0;
		n -= n % instance->channels_count;
	}

	for (i=0; i<n; i++)
	{
		down(&instance->buffer_semaphore);
			me_seg_buf_get(instance->seg_buf, &tmp);
		up(&instance->buffer_semaphore);
		value = tmp;
		if(put_user(value, values + i))
		{
			PERROR("Cannot copy new values to user.\n");
			return -ME_ERRNO_INTERNAL;
		}
	}
	return n;
}

static int mephisto_ai_io_stream_read_check(mephisto_ai_subdevice_t* instance, int read_mode, int* values, int* count, int time_out, int flags)
{
	if((flags != ME_IO_STREAM_READ_NO_FLAGS) && (flags != ME_IO_STREAM_READ_FRAMES))
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_READ_NO_FLAGS or ME_IO_STREAM_READ_FRAMES.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (!values || !count)
	{
		PERROR("Request has invalid pointer.\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (*count < 0)
	{
		PERROR("Request has invalid value's counter. Should be at least 1.\n");
		return ME_ERRNO_INVALID_VALUE_COUNT;
	}

	if((read_mode != ME_READ_MODE_BLOCKING) && (read_mode != ME_READ_MODE_NONBLOCKING))
	{
		PERROR("Invalid read mode specified. Must be ME_READ_MODE_BLOCKING or ME_READ_MODE_NONBLOCKING.\n");
		return ME_ERRNO_INVALID_READ_MODE;
	}

	return 0;
}

int mephisto_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags)
{
	mephisto_ai_subdevice_t* instance;
	int ret;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long int j;
	int err = ME_ERRNO_SUCCESS;

	int c = *count;
	int min = c;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	err = mephisto_ai_io_stream_read_check(instance, read_mode, values, count, time_out, flags);
	if (err)
		return err;

	if (c == 0)
	{	//For Windows compatibility. You get what you want! Nothing more or less.
		PDEBUG("C=0 -> return\n");
		return ME_ERRNO_SUCCESS;
	}

	if (time_out)
	{
		delay = (time_out * HZ) / 1000;
		if (!delay)
			delay = 1;
		if (delay>LONG_MAX - 2)
			delay = LONG_MAX - 2;
	}


	ME_SUBDEVICE_ENTER;
		if (flags & ME_IO_STREAM_READ_FRAMES)
		{
			//Check if subdevice is configured.
			if( instance->channels_count <= 0)
			{
				PERROR("Subdevice wasn't configured.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}

			if (c < instance->channels_count)
			{	//Not enough data requested.
				PERROR("When using FRAME_READ mode minimal size is defined by channel list.\n");
				err = ME_ERRNO_INVALID_VALUE_COUNT;
				goto ERROR;
			}

			//Wait for whole list.
			if (read_mode == ME_READ_MODE_BLOCKING)
			{
				min = c - (c % instance->channels_count);
			}

			if (read_mode == ME_READ_MODE_NONBLOCKING)
			{
				min = instance->channels_count;
			}
		}
		else
		{
			if (c > me_seg_buf_size(instance->seg_buf))
			{// To return acceptable amount of data when user pass too big value. For security this is not bigger than ME4600_AI_CIRC_BUF_COUNT - 2
				min = me_seg_buf_size(instance->seg_buf);
				min -= (instance->channels_count > 2) ? instance->channels_count : 2;
			}

		}

		if ((*instance->status >= MEPHISTO_AI_STATUS_start))
		{//Working
			//If blocking mode -> wait for data.
			if ((me_seg_buf_values(instance->seg_buf) < min) && (read_mode == ME_READ_MODE_BLOCKING))
			{
				j = jiffies;
				wait_event_interruptible_timeout(
					instance->wait_queue,
					((me_seg_buf_values(instance->seg_buf) >= min) || (*instance->status < MEPHISTO_AI_STATUS_start)),
					delay);

				if (signal_pending(current))
				{
					PERROR("Wait on values interrupted from signal.\n");
					err = ME_ERRNO_SIGNAL;
					goto ERROR;
				}
				else if ((jiffies - j) >= delay)
				{
					PERROR("Wait on values timed out.\n");
					err = ME_ERRNO_TIMEOUT;
				}
			}
		}

		ret = mephisto_ai_io_stream_read_get_value(instance, values, c, flags);
		if (ret < 0)
		{
			err = -ret;
			*count = 0;
		}
		else if (ret == 0)
		{
			down(instance->device_semaphore);
				*count = 0;
				switch (*instance->status)
				{
					case MEPHISTO_AI_STATUS_configured:
						if (instance->empty_read_count)
						{
							err = ME_ERRNO_SUBDEVICE_NOT_RUNNING;
						}
						instance->empty_read_count = 1;
						break;

					case MEPHISTO_AI_STATUS_fifo_error:
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
						*instance->status = MEPHISTO_AI_STATUS_configured;
						break;

					case MEPHISTO_AI_STATUS_buffer_error:
						err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
						*instance->status = MEPHISTO_AI_STATUS_configured;
						break;

					case MEPHISTO_AI_STATUS_idle:
					case MEPHISTO_AI_STATUS_timeout:
						err = ME_ERRNO_CANCELLED;
						break;

					case MEPHISTO_AI_STATUS_error:
						err = ME_ERRNO_COMMUNICATION;
						break;

					default:
						break;
				}
			up(instance->device_semaphore);
		}
		else
		{
			*count = ret;
		}

		if (ret || (*instance->status != MEPHISTO_AI_STATUS_configured))
		{
			instance->empty_read_count = 0;
		}
ERROR:
	ME_SUBDEVICE_EXIT;
	return err;
}

int mephisto_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags)
{
	mephisto_ai_subdevice_t* instance;
	unsigned long int delay = LONG_MAX-2;

	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_START_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if((start_mode != ME_START_MODE_BLOCKING) && (start_mode != ME_START_MODE_NONBLOCKING))
	{
		PERROR("Invalid start mode specified. Must be ME_START_MODE_BLOCKING or ME_START_MODE_NONBLOCKING.\n");
		return ME_ERRNO_INVALID_START_MODE;
	}

	if (time_out)
	{
		delay = (time_out * HZ) / 1000;
		if (!delay)
			delay = 1;
		if (delay>LONG_MAX - 2)
			delay = LONG_MAX - 2;
	}

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			switch (*instance->status)
			{
				case MEPHISTO_AI_STATUS_idle:
					PERROR("Subdevice is in idle.\n");
				case MEPHISTO_AI_STATUS_error:
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;

				case MEPHISTO_AI_STATUS_timeout:
				case MEPHISTO_AI_STATUS_fifo_error:
				case MEPHISTO_AI_STATUS_buffer_error:
				case MEPHISTO_AI_STATUS_configured:
					// OK - subdevice in idle
					break;

				case MEPHISTO_AI_STATUS_start:
				case MEPHISTO_AI_STATUS_run:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					goto ERROR;
			}

			instance->timeout = (start_mode == ME_START_MODE_BLOCKING) ? LONG_MAX - 2 : delay;

			// Schedule stream task
			PDEBUG("Schedule stream task.\n");
			*instance->status = MEPHISTO_AI_STATUS_start;
			queue_work(instance->mephisto_workqueue, &instance->mephisto_stream);

			if (start_mode == ME_START_MODE_BLOCKING)
			{//Wait for start.
				up(instance->device_semaphore);
				//Only runing process will interrupt this call. Events are signaled when status change. Extra 1 tick timeout added for safe reason.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(*instance->status != MEPHISTO_AI_STATUS_start),
					delay+1);
				PDEBUG("stream starts\n");

				down(instance->device_semaphore);
				if (signal_pending(current))
				{
					PERROR("Wait on start of state machine interrupted.\n");
					err = ME_ERRNO_SIGNAL;
					goto ERROR;
				}

				switch (*instance->status)
				{
					case MEPHISTO_AI_STATUS_start:
					case MEPHISTO_AI_STATUS_timeout:
						PERROR("Timeout reached.\n");
						if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Break, NULL, 0, NULL, 0))
						{
							err = ME_ERRNO_COMMUNICATION;
						}
						err = ME_ERRNO_TIMEOUT;
						break;

					case MEPHISTO_AI_STATUS_fifo_error:
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
						break;

					case MEPHISTO_AI_STATUS_buffer_error:
						err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
						break;

					case MEPHISTO_AI_STATUS_configured:
					case MEPHISTO_AI_STATUS_run:
						// OK
						break;

					case MEPHISTO_AI_STATUS_idle:
					default:
						PDEBUG("Stream canceled.\n");
						err = ME_ERRNO_CANCELLED;
				}
			}

ERROR:
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags)
{
	mephisto_ai_subdevice_t* instance;
	int old_count;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_STOP_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	switch (wait)
	{
		case ME_WAIT_NONE:
		case ME_WAIT_IDLE:
		case ME_WAIT_BUSY:
		case ME_WAIT_START:
		case ME_WAIT_STOP:
			break;

		default:
			PERROR("Invalid wait argument specified.\n");
			*status = ME_STATUS_INVALID;
			*values = 0;
			return ME_ERRNO_INVALID_WAIT;
	}

	ME_SUBDEVICE_ENTER;
		switch (*instance->status)
		{
			case MEPHISTO_AI_STATUS_idle:
			case MEPHISTO_AI_STATUS_configured:
			case MEPHISTO_AI_STATUS_error:
			case MEPHISTO_AI_STATUS_fifo_error:
			case MEPHISTO_AI_STATUS_buffer_error:
			case MEPHISTO_AI_STATUS_timeout:
				*status = ME_STATUS_IDLE;
				break;

			case MEPHISTO_AI_STATUS_start:
			case MEPHISTO_AI_STATUS_run:
				*status = ME_STATUS_BUSY;
				break;

			default:
				PERROR_CRITICAL("WRONG STATUS!\n");
				break;
		}

		if ((wait == ME_WAIT_IDLE) && (*status == ME_STATUS_BUSY))
		{
			// Only runing process will interrupt this call. Events are signaled when status change.
			wait_event_interruptible(instance->wait_queue, *instance->status < MEPHISTO_AI_STATUS_start);

			if (*instance->status == MEPHISTO_AI_STATUS_idle)
			{
				PDEBUG("Wait for IDLE canceled. 0x%x\n", *instance->status);
				err = ME_ERRNO_CANCELLED;
			}

			if (signal_pending(current))
			{
				PERROR("Wait for IDLE interrupted.\n");
				err = ME_ERRNO_SIGNAL;
			}

			*status = ME_STATUS_IDLE;
		}
		else if ((wait == ME_WAIT_BUSY) && (*status == ME_STATUS_IDLE))
		{
			// Only runing process will interrupt this call. Events are signaled when status change.
			wait_event_interruptible(instance->wait_queue, (*instance->status >= MEPHISTO_AI_STATUS_start) || *instance->status == MEPHISTO_AI_STATUS_idle);

			if (*instance->status == MEPHISTO_AI_STATUS_idle)
			{
				PDEBUG("Wait for BUSY canceled. 0x%x\n", *instance->status);
				err = ME_ERRNO_CANCELLED;
			}

			if (signal_pending(current))
			{
				PERROR("Wait for BUSY interrupted.\n");
				err = ME_ERRNO_SIGNAL;
			}

			*status = ME_STATUS_BUSY;
		}
		else if (wait == ME_WAIT_START)
		{
			old_count = (*values) ? *values : instance->stream_start_count;
			// Only runing process will interrupt this call. Events are signaled when status change.
			if ((old_count == instance->stream_start_count) || (*instance->status == MEPHISTO_AI_STATUS_idle))
			{
				wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_start_count) || (*instance->status == MEPHISTO_AI_STATUS_idle)));
			}

			if (*instance->status == MEPHISTO_AI_STATUS_idle)
			{
				PDEBUG("Wait for START canceled.\n");
				err = ME_ERRNO_CANCELLED;
			}

			if (*instance->status == MEPHISTO_AI_STATUS_timeout)
			{
				PDEBUG("Wait for START: report starting timeout.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (signal_pending(current))
			{
				PERROR("Wait for START interrupted.\n");
				err = ME_ERRNO_SIGNAL;
			}

			*status = ME_STATUS_BUSY;
		}
		else if (wait == ME_WAIT_STOP)
		{
			old_count = (*values) ? *values : instance->stream_stop_count;
			// Only runing process will interrupt this call. Events are signaled when status change.
			if ((old_count == instance->stream_stop_count) || (*instance->status == MEPHISTO_AI_STATUS_idle))
			{
				wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_stop_count) || (*instance->status == MEPHISTO_AI_STATUS_idle)));
			}

			if (*instance->status == MEPHISTO_AI_STATUS_idle)
			{
				PDEBUG("Wait for STOP canceled. 0x%x\n", *instance->status);
				err = ME_ERRNO_CANCELLED;
			}

			if (*instance->status == MEPHISTO_AI_STATUS_timeout)
			{
				PDEBUG("Wait for STOP: report stoping timeout.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (signal_pending(current))
			{
				PERROR("Wait for STOP interrupted.\n");
				err = ME_ERRNO_SIGNAL;
			}

			*status = ME_STATUS_IDLE;
		}

		switch (wait)
		{
			case ME_WAIT_START:
				*values = instance->stream_start_count;
				break;

			case ME_WAIT_STOP:
				*values = instance->stream_stop_count;
				break;

			case ME_WAIT_IDLE:
			case ME_WAIT_BUSY:
			default:
				*values = me_seg_buf_values(instance->seg_buf);
				PDEBUG("me_seg_buf_values(instance->seg_buf)=%d.\n", *values);
		}
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags)
{
/**
 @note Stop is implemented only in blocking mode.
 @note Function return when state machine is stoped.
*/
	mephisto_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	if (flags & ~ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_STOP_TYPE_NO_FLAGS.\n");
		if (flags & ME_IO_STREAM_STOP_NONBLOCKING)
		{
			PERROR("ME_IO_STREAM_STOP_NONBLOCKING not supported.\n");
		}
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (stop_mode != ME_STOP_MODE_IMMEDIATE)
	{
		PERROR("Invalid stop mode specified. Must be ME_STOP_MODE_IMMEDIATE.\n");
		return ME_ERRNO_INVALID_STOP_MODE;
	}

	instance = (mephisto_ai_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			switch (*instance->status)
			{
				case MEPHISTO_AI_STATUS_idle:
				case MEPHISTO_AI_STATUS_configured:
				case MEPHISTO_AI_STATUS_timeout:
				case MEPHISTO_AI_STATUS_error:
				case MEPHISTO_AI_STATUS_fifo_error:
				case MEPHISTO_AI_STATUS_buffer_error:
					break;

				case MEPHISTO_AI_STATUS_start:
				case MEPHISTO_AI_STATUS_run:
					// Send stop command.
					if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Break, NULL, 0, NULL, 0))
					{
						err = ME_ERRNO_COMMUNICATION;
					}
					else
					{
						// Wait for control task.
						up(instance->device_semaphore);
						// Only runing process will interrupt this call. Events are signaled when status change.
						wait_event_interruptible(instance->wait_queue, *instance->status <= MEPHISTO_AI_STATUS_start);
						down(instance->device_semaphore);
					}

					if (signal_pending(current))
					{
						PERROR("Stopping stream interrupted.\n");
						err = ME_ERRNO_SIGNAL;
					}

					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
			}
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_ai_set_offset(me_subdevice_t* subdevice, struct file* filep, int channel, int range, int* offset, int flags)
{
	mephisto_ai_subdevice_t* instance;
	MEPHISTO_modes_tu	tmp_offset;
	MEPHISTO_modes_tu	tmp_range;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	if (flags)
	{
		PERROR("Invalid flags. Must be ME_IO_SET_CHANNEL_OFFSET_NO_FLAGS.\n");

		return ME_ERRNO_INVALID_FLAGS;
	}

	if( (range < 0) || (range >= MEPHISTO_NUMBER_RANGES) )
	{
		PERROR("Invalid range.\n");

		return ME_ERRNO_INVALID_RANGE;
	}

	if( (channel < 0) || (channel > 1) )
	{
		PERROR("Invalid channel.\n");

		return ME_ERRNO_INVALID_CHANNEL;
	}

	instance = (mephisto_ai_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				tmp_offset.value = *offset;
				tmp_range.value = mephisto_ranges[range];
				err = set_offset(instance->base.dev, channel, tmp_range, &tmp_offset);
				if (err)
				{
					instance->offset[channel][range].value = 0;
					*offset = 0;
				}
				else
				{
					instance->offset[channel][range].value = *offset;
					*offset = tmp_offset.value;
				}
			}
			else
			{
				PERROR("Subdevice is streaming!\n");
				err = ME_ERRNO_SUBDEVICE_BUSY;

			}
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{
	mephisto_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	if ((unit == ME_UNIT_VOLT) || (unit == ME_UNIT_ANY))
	{
		*count = MEPHISTO_NUMBER_RANGES;
	}
	else
	{
		*count = 0;
	}

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	mephisto_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	if ((range < 0) || (range >= MEPHISTO_NUMBER_RANGES))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", MEPHISTO_NUMBER_RANGES - 1);
		return ME_ERRNO_INVALID_RANGE;
	}

	*unit = ME_UNIT_VOLT;
	*max = mephisto_ranges[range] >> 1;
	*min = *max * -1;
	*maxdata = MEPHISTO_AI_MAX_DATA;

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	mephisto_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (mephisto_ai_subdevice_t *) subdevice;

	*base_frequency = 1000000;
	if ((timer & ~ME_TIMER_OSCILLOSCOPE_FLAG) != ME_TIMER_CONV_START)
	{
		PERROR("Invalid timer specified. Must be ME_TIMER_CONV_START. %x != %x\n", timer & ~ME_TIMER_OSCILLOSCOPE_FLAG, ME_TIMER_CONV_START);
		PERROR("Invalid timer specified. Must be ME_TIMER_CONV_START.\n");
		return ME_ERRNO_INVALID_TIMER;
	}

	if (timer & ME_TIMER_OSCILLOSCOPE_FLAG)
	{
			*min_ticks = 1;
			*max_ticks = 2500000;
	}
	else
	{
			*min_ticks = 10;
			*max_ticks = 2500000;
	}

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed. idx=0\n");

	*number = 2;

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed. idx=0\n");

	*type = ME_TYPE_AI;
	*subtype = ME_SUBTYPE_STREAMING;

	return ME_ERRNO_SUCCESS;
}

int mephisto_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed. idx=0\n");

	*caps = mephisto_AI_CAPS;

	PINFO("CAPS: %x\n", *caps);

	return ME_ERRNO_SUCCESS;
}
int mephisto_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count)
{
	mephisto_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (mephisto_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (*count < 1)
	{
		PERROR("Invalid capability argument count. Should be at least 1.\n");
		return ME_ERRNO_INVALID_CAP_ARG_COUNT;
	}

	*count = 1;
	switch (cap)
	{
		case ME_CAP_AI_FIFO_SIZE:
			*args = 131000;
		break;

		case ME_CAP_AI_BUFFER_SIZE:
			*args = (instance->seg_buf) ? me_seg_buf_size(instance->seg_buf) : 0;
		break;

		case ME_CAP_AI_CHANNEL_LIST_SIZE:
			*args = 2;
		break;

		case ME_CAP_AI_MAX_THRESHOLD_SIZE:
			*args = (instance->seg_buf) ? me_seg_buf_size(instance->seg_buf) : 0;
		break;

		default:
			*count = 0;
			*args = 0;
	}

	return err;
}


mephisto_ai_subdevice_t* mephisto_ai_constr(unsigned int idx, mephisto_AI_status_e* status, struct semaphore* device_semaphore)
{
	mephisto_ai_subdevice_t* subdevice;

	PDEBUG("executed. idx=0\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(mephisto_ai_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		return NULL;
	}

	// Initialize subdevice base class.
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		return NULL;
	}

	subdevice->status = status;

	// Initialize spin locks.
	subdevice->device_semaphore = device_semaphore;

	// Store analog input index.
	subdevice->base.idx = idx;

	// Initialize segmented buffer.
	init_MUTEX(&subdevice->buffer_semaphore);
	subdevice->seg_buf = create_seg_buffer(MEPHISTO_AI_SEG_BUF_CHUNK_COUNT, MEPHISTO_AI_SEG_BUF_CHUNK_SIZE);
	if (!subdevice->seg_buf)
	{
		PERROR("Cannot initialize segmented buffer.\n");
		me_subdevice_deinit((me_subdevice_t *) subdevice);
		kfree(subdevice);
		return NULL;
	}

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Override base class methods.
	subdevice->base.me_subdevice_destructor = mephisto_ai_destructor;
	subdevice->base.me_subdevice_io_reset_subdevice = mephisto_ai_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = mephisto_ai_io_single_config;
	subdevice->base.me_subdevice_io_single_read = mephisto_ai_io_single_read;
	subdevice->base.me_subdevice_io_stream_config = mephisto_ai_io_stream_config;
	subdevice->base.me_subdevice_io_stream_new_values = mephisto_ai_io_stream_new_values;
	subdevice->base.me_subdevice_io_stream_read = mephisto_ai_io_stream_read;
	subdevice->base.me_subdevice_io_stream_start = mephisto_ai_io_stream_start;
	subdevice->base.me_subdevice_io_stream_status = mephisto_ai_io_stream_status;
	subdevice->base.me_subdevice_io_stream_stop = mephisto_ai_io_stream_stop;
	subdevice->base.me_subdevice_query_number_channels = mephisto_ai_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = mephisto_ai_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = mephisto_ai_query_subdevice_caps;
	subdevice->base.me_subdevice_query_subdevice_caps_args = mephisto_ai_query_subdevice_caps_args;
	subdevice->base.me_subdevice_query_number_ranges = mephisto_ai_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = mephisto_ai_query_range_info;
	subdevice->base.me_subdevice_query_timer = mephisto_ai_query_timer;

	subdevice->base.me_subdevice_set_offset = mephisto_ai_set_offset;

	// Prepare work queue.
	subdevice->mephisto_workqueue = create_workqueue("mephisto");
	// workqueue API changed in kernel 2.6.20
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&subdevice->mephisto_stream, mephisto_stream, (void *)subdevice);
#else
	INIT_WORK(&subdevice->mephisto_stream, mephisto_stream);
#endif

	return subdevice;
}

#  define MEPHISTO_AI_TRANSFER_BUF		(64 * 8454)
void mephisto_stream(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
									void* subdevice
#else
									struct work_struct* work
#endif
								)
{
	mephisto_ai_subdevice_t* instance;

	SetMode_arg_t		mode_send;
	SetMode_arg_t		mode_recive;

	Setup_arg_recive_t setup_recive;
	Setup_arg_send_t setup_send;
	Setup_arg_recive_t setup_send_return;

	struct urb *urb = NULL;
	uint16_t* buf = NULL;
	unsigned int count;
	unsigned int signals_count;
	unsigned int recived = 0;

	int i;
	int idx;
	unsigned int end_marker = 0;
	unsigned int remove_number = 0;
	uint16_t marker[8] = {0,0,0,0,0,0,0,0};

	unsigned long long int stream_start;

	unsigned int signal_event;

	uint64_t data_required;

	int ret_val;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	instance = (mephisto_ai_subdevice_t *) subdevice;
#else
	instance = container_of((void *)work, mephisto_ai_subdevice_t, mephisto_stream);
#endif

	PDEBUG("Executed.\n");

	if (!instance)
	{
		return;
	}

	if (signal_pending(current))
	{
		PERROR("Control task interrupted.\n");
		*instance->status = MEPHISTO_AI_STATUS_idle;
		return;
	}

	if (instance->channels_count == 0)
	{
		PERROR("No channel defined.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		wake_up_interruptible_all(&instance->wait_queue);
		return;
	}

	stream_start = jiffies;

	instance->data_recived = 0;

	PDEBUG("send mode:%c%c%c%c\n", MEPHISTO_modes[instance->mode].text[0], MEPHISTO_modes[instance->mode].text[1], MEPHISTO_modes[instance->mode].text[2], MEPHISTO_modes[instance->mode].text[3]);
	memcpy(&mode_send, MEPHISTO_modes[instance->mode].text, sizeof(SetMode_arg_t));
	if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Seting stream mode failed.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}
	PDEBUG("recive mode:%s\n", mode_recive.mode.text);

	if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetupRead, NULL, 0, (void *)&setup_recive, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Reading configuration failed.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}

	if (instance->range[0] >= 0)
	{
		setup_send.amplitude_0 = uvolts_to_float(mephisto_ranges[instance->range[0]]);
		setup_send.offset_0 = uvolts_to_float(instance->offset[0][instance->range[0]].value);
	}
	else
	{
		setup_send.amplitude_0.fvalue = setup_recive.amplitude_0.fvalue;
		setup_send.offset_0.fvalue = setup_recive.offset_0.fvalue;
	}

	if (instance->range[1] >= 0)
	{
		setup_send.amplitude_1 = uvolts_to_float(mephisto_ranges[instance->range[1]]);
		setup_send.offset_1 = uvolts_to_float(instance->offset[1][instance->range[1]].value);
	}
	else
	{
		setup_send.amplitude_1.fvalue = setup_recive.amplitude_1.fvalue;
		setup_send.offset_1.fvalue = setup_recive.offset_1.fvalue;
	}

	setup_send.time_base = uvolts_to_float(instance->time_base);
	setup_send.memory_depth = int_to_float((instance->channels_count == 1) ? instance->data_required : instance->data_required >> 1);
	setup_send.trigger_point = uvolts_to_float(instance->trigger_point * 10000);
	setup_send.trigger_channel.value = instance->trigger_channel & 0x01;
	setup_send.trigger_type.value = instance->trigger_type.value;
	setup_send.lower_trigger_level = uvolts_to_float(instance->trigger_lower_level);
	setup_send.upper_trigger_level = uvolts_to_float(instance->trigger_upper_level);
	setup_send.data_GPIO.value = setup_recive.data_GPIO.value | ~(setup_recive.dir_GPIO.value);
	setup_send.dir_GPIO.value = setup_recive.dir_GPIO.value;

	if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetupWrite, (void *)&setup_send, sizeof(Setup_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&setup_send_return, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Writing configuration failed.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}

	data_required = float_to_int(setup_send_return.memory_depth).value;

	if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_Run, NULL, 0, NULL, 0))
	{
		PERROR("Starting stream failed.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}

	urb=usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
	{
		PERROR("Cann't allocate urb structure.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}

	buf = kmalloc(MEPHISTO_AI_TRANSFER_BUF, GFP_KERNEL);
	if (!buf)
	{
		PERROR("Cann't allocate buffer.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
		goto ERROR;
	}

	do
	{
		signal_event = 0;
		count = MEPHISTO_AI_TRANSFER_BUF;
		ret_val = mephisto_get_packet(instance->base.dev, urb, buf, &count);
		down(instance->device_semaphore);
			if (ret_val == USB_PACKET_RECIVED)
			{
				signals_count = instance->data_recived - ((instance->threshold > 0) ? (instance->data_recived % instance->threshold) : (instance->data_recived % 1000));
				//Copy results to buffer.
				for (i=1; i<count; ++i)
				{
					if (i % 32 == 0)
					{
						continue;
					}
					if (instance->range[recived % 2] >= 0)
					{
						down(&instance->buffer_semaphore);
							me_seg_buf_put(instance->seg_buf, buf[i]);
							++instance->data_recived;
						up(&instance->buffer_semaphore);
					}
					++recived;

					if (instance->data_required == 0)
					{
						for (idx = 0; idx < 7; ++idx)
						{
							marker[idx] = marker[idx + 1];
						}
						marker[7] = buf[i];

						if ((marker[0] == 0x0000) &&
							(marker[1] == 0xFFFF) &&
							(marker[2] == 0xFFFF) &&
							(marker[3] == 0x0000) &&
							(marker[4] == 0x0000) &&
							(marker[5] == 0xFFFF) &&
							(marker[6] == 0xFFFF) &&
							(marker[7] == 0x0000))
						{
							end_marker = 1;
							PDEBUG("END MARKER!\n");
							remove_number = instance->channels_count << 1;
							for (idx = 0; idx < remove_number; ++idx)
							{
								me_seg_buf_unget(instance->seg_buf);
							}
						}
					}
				}

				if (*instance->status == MEPHISTO_AI_STATUS_start)
				{
					*instance->status = MEPHISTO_AI_STATUS_run;
					signal_event = 1;
				}

				if (instance->data_required)
				{
					if (instance->data_recived >= data_required)
					{
						ret_val = USB_PACKET_FINISH;
						PDEBUG("instance->data_recived >= data_required: %d >= %lld\n", instance->data_recived, data_required);
					}
				}
				else if (end_marker)
				{
					ret_val = USB_PACKET_FINISH;
				}
				else if ((instance->threshold > 0) && (signals_count != (instance->data_recived - (instance->data_recived % instance->threshold))))
				{
					signal_event = 1;
				}
				else if ((instance->threshold == 0) && (signals_count != (instance->data_recived - (instance->data_recived % 1000))))
				{
					signal_event = 1;
				}
			}
			else if ((*instance->status == MEPHISTO_AI_STATUS_start) && (jiffies - stream_start > instance->timeout))
			{
				*instance->status = MEPHISTO_AI_STATUS_timeout;
			}
		up(instance->device_semaphore);

		if ((signal_event) && ((ret_val == USB_PACKET_WAIT) || (ret_val == USB_PACKET_RECIVED)))
		{
			wake_up_interruptible_all(&instance->wait_queue);
		}
// 		schedule_timeout(1);
	}
	while ((ret_val == USB_PACKET_WAIT) || (ret_val == USB_PACKET_RECIVED));


ERROR:
	if (buf)
	{
		kfree(buf);
	}

	if (urb)
	{
		usb_free_urb(urb);
	}

	memcpy(&mode_send, MEPHISTO_modes[instance->mode].text, sizeof(SetMode_arg_t));
	if (mephisto_cmd(instance->base.dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Seting stream mode failed.\n");
		*instance->status = MEPHISTO_AI_STATUS_error;
	}

	if (*instance->status == MEPHISTO_AI_STATUS_run)
	{
		*instance->status = MEPHISTO_AI_STATUS_configured;
	}

	//Signal it.
	PINFO("<%s> Signal event. idx=%d\n", __FUNCTION__, instance->base.idx);
	wake_up_interruptible_all(&instance->wait_queue);

	PDEBUG("terminated.\n");
}

