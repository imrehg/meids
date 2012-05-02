/**
 * @file me4600_ai.c
 *
 * @brief The ME-4600 analog input subdevice instance.
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
# include "mehardware_access.h"
# include "me_spin_lock.h"
# include "meeeprom.h"

# include "me4600_reg.h"
# include "me4600_ai_reg.h"
# include "me4600_ai.h"
# include "medevice.h"

# include "me_data_types.h"

# define me4600_AI_ERROR_TIMEOUT	((HZ<<7)+2)

/// Declarations

static void me4600_ai_destructor(me_subdevice_t* subdevice);
int me4600_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);

static int me4600_ai_io_single_config_check(me4600_ai_subdevice_t* instance, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me4600_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);

static int me4600_ai_io_single_read_check(me4600_ai_subdevice_t* instance, int channel,  int* value, int time_out, int flags);
int me4600_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
static int me4600_ai_io_stream_config_check(me4600_ai_subdevice_t* instance,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);
int me4600_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t *config_list, int count, meIOStreamSimpleTriggers_t *trigger, int fifo_irq_threshold, int flags);

static int me4600_ai_io_stream_read_check(me4600_ai_subdevice_t* instance, int read_mode, int* values, int* count, int time_out, int flags);
int me4600_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags);

int me4600_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags);
int me4600_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags);
int me4600_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags);

static int me4600_ai_FSM_test(me4600_ai_subdevice_t* instance);
int me4600_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags);
static int inline me4600_ai_io_stream_read_get_value(me4600_ai_subdevice_t* instance, int* values, const int count, const int flags);

int me4600_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
int me4600_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
int me4600_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
int me4600_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
int me4600_ai_query_number_channels(me_subdevice_t* subdevice, int *number);
int me4600_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me4600_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int me4600_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count);
int me4600_ai_postinit(me_subdevice_t* subdevice, void* args);

static int me4600_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);

static void ai_mux_toggler(me4600_ai_subdevice_t* instance);

static int inline ai_isnt_fifo_empty(me4600_ai_subdevice_t* instance);
static void ai_determine_FIFO_size(me4600_ai_subdevice_t* instance);
static int inline ai_isnt_LE_fifo_full(me4600_ai_subdevice_t* instance);
static void ai_determine_LE_size(me4600_ai_subdevice_t* instance);
static void ai_read_calibration(me4600_ai_subdevice_t* instance);
static void ai_default_calibration_entry(me_calibration_entry_t* entry);
static void ai_default_calibration(me4600_ai_subdevice_t* instance);
static void ai_calculate_calibration(me4600_ai_subdevice_t* instance, me4600_ai_eeprom_t raw_cal);
static void ai_calculate_calibration_entry(int64_t nominal_A, int64_t actual_A, int64_t nominal_B, int64_t actual_B, me_calibration_entry_t* entry);

static uint16_t inline ai_calculate_end_value(const me_calibration_entry_t calibration, int16_t value);
static uint16_t inline ai_calculate_calibrated_value(me4600_ai_subdevice_t* instance, int entry, int value);

static int me4600_ai_config_load(me_subdevice_t* subdevice, struct file* filep, void* config);

/** Immidiate stop.
* Reset all IRQ's sources. (block laches)
* Preserve FIFO
*/
static int ai_stop_immediately(me4600_ai_subdevice_t* instance);

/** Immidiate stop.
* Reset all IRQ's sources. (block laches)
* Reset data FIFO
*/
void inline ai_stop_isr(me4600_ai_subdevice_t* instance);

/** Interrupt logics.
* Read datas
* Reset latches
*/
int ai_limited_isr(me4600_ai_subdevice_t* instance, const uint32_t irq_status, const uint32_t ctrl_status);
int ai_infinite_isr(me4600_ai_subdevice_t* instance, const uint32_t irq_status, const uint32_t ctrl_status);

/** Last chunck of datas. We must reschedule sample counter.
* Leaving SC_RELOAD doesn't do any harm, but in some bad case can make extra interrupts.
* When threshold is wrongly set some IRQ are lost.(!!!)
*/
void inline ai_reschedule_SC(me4600_ai_subdevice_t* instance);

/** Read datas from FIFO and copy them to buffer */
static int inline ai_read_data(me4600_ai_subdevice_t* instance, const int count);

/** Copy rest of data from fifo to circular buffer.*/
static int inline ai_read_data_pooling(me4600_ai_subdevice_t* instance);

/** Set ISM to next state for infinite data aqusation mode*/
void inline ai_infinite_ISM(me4600_ai_subdevice_t* instance);

/** Set ISM to next state for define amount of data aqusation mode*/
void inline ai_limited_ISM(me4600_ai_subdevice_t* instance, uint32_t irq_status);

/** Set ISM to next stage for limited mode */
void inline ai_data_acquisition_logic(me4600_ai_subdevice_t* instance);

static void me4600_ai_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void* subdevice
#else
											struct work_struct* work
#endif
										);

static void me4600_ai_destructor(me_subdevice_t* subdevice)
{
	me4600_ai_subdevice_t* instance;

	instance = (me4600_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (!instance)
	{
		return;
	}

	if (instance->base.dev && (((me_general_dev_t *)instance->base.dev)->dev))
	{
		// Reset subdevice to asure clean exit.
		me4600_ai_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
	}

	atomic_set(&instance->ai_control_task_flag, 0);
	ME_SUBDEVICE_LOCK;
		// Remove any tasks from work queue. This is paranoic because it was done allready in reset().
		PDEBUG("Cancel control task.\n");
		atomic_set(&instance->ai_control_task_flag, 0);
		cancel_delayed_work(&instance->ai_control_task);
	ME_SUBDEVICE_UNLOCK;
	//Wait 2 ticks to be sure that control task is removed from queue.
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(2);

	if (instance->chan_list_copy)
	{
		kfree(instance->chan_list_copy);
		instance->chan_list_copy = NULL;
		instance->chan_list_copy_pos=0;
	}

	destroy_seg_buffer(&instance->seg_buf);
	me_subdevice_deinit(&instance->base);
}

int me4600_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4600_ai_subdevice_t* instance;
	uint32_t tmp;
	int i;

	PDEBUG("executed. idx=0\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (me4600_ai_subdevice_t *)subdevice;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			instance->status = ai_status_none;

			//Cancel control task
			PDEBUG("Cancel control task.\n");
			atomic_set(&instance->ai_control_task_flag, 0);
			cancel_delayed_work(&instance->ai_control_task);

			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			//Stop DMA
			tmp &= ~ME4600_AI_CTRL_BIT_DMA;
			// Stop all actions. No conditions!
			tmp &= ~ME4600_AI_CTRL_BIT_STOP;
			tmp |= ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;

			// Set safe state
			tmp &= ~(ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2);
			tmp &= ~(ME4600_AI_CTRL_BIT_FULLSCALE | ME4600_AI_CTRL_BIT_OFFSET | ME4600_AI_CTRL_BIT_SAMPLE_HOLD);

			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			// Clear all features. Dissable interrupts.
			tmp = (   ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
					| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
					| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
					| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);

			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
			me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);
			me_writel(instance->base.dev, 0, instance->scan_timer_low_reg);
			me_writel(instance->base.dev, 0, instance->scan_timer_high_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
			me_writel(instance->base.dev, 0xEFFFFFFF, instance->sample_counter_reg);

			me_seg_buf_reset(instance->seg_buf);
			instance->ISM.next = 0;

			instance->fifo_irq_threshold = 0;
			instance->data_required = 0;
			instance->chan_list_len = 0;

			if (instance->chan_list_copy)
			{
				memset(instance->chan_list_copy, 0, instance->LE_size * sizeof(uint16_t));
			}
			instance->chan_list_copy_pos=0;

			// Initialize the single config entries to reset values.
			for (i = 0; i < instance->channels; i++)
			{
				instance->single_config[i].status = ME_SINGLE_CHANNEL_NOT_CONFIGURED;
			}

			//Set status to signal that device is unconfigured.
			instance->stream_start_count = 0;
			instance->stream_stop_count = 0;

			instance->raw_values = 0;
		ME_UNLOCK_PROTECTOR;
		//Signal reset if user is on wait.
		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_io_single_config_check(me4600_ai_subdevice_t* instance, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	if ((flags != ME_IO_SINGLE_CONFIG_CONTINUE) && (flags != ME_IO_SINGLE_CONFIG_NO_FLAGS))
	{
		PERROR("Invalid flags. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS or ME_IO_SINGLE_CONFIG_CONTINUE.\n");
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

		case ME_TRIG_TYPE_EXT_ANALOG:
			if (!(instance->features & ME4600_ANALOG_TRIGGER))
			{
				PERROR("Invalid trigger type specified. ANALOG TRIGGER not supported.\n");
				return ME_ERRNO_INVALID_TRIG_TYPE;
			}

		case ME_TRIG_TYPE_EXT_DIGITAL:
			if ((trig_edge != ME_TRIG_EDGE_ANY) && (trig_edge != ME_TRIG_EDGE_RISING) && (trig_edge != ME_TRIG_EDGE_FALLING))
			{
				PERROR("Invalid trigger edge specified. Must be ME_TRIG_EDGE_RISING, ME_TRIG_EDGE_FALLING or ME_TRIG_EDGE_ANY.\n");
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

	if ((single_config < 0) || (single_config >= instance->ranges_len))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->ranges_len - 1);
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if ((ref != ME_REF_AI_GROUND) && (ref != ME_REF_AI_DIFFERENTIAL))
	{
		PERROR("Invalid reference specified. Must be ME_REF_AI_GROUND or ME_REF_AI_DIFFERENTIAL.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if ((single_config % 2) && (ref != ME_REF_AI_GROUND))
	{
		PERROR("Invalid reference specified. Only ME_REF_AI_GROUND supported by range %d.\n", single_config);
		return ME_ERRNO_INVALID_REF;
	}

	if (ref == ME_REF_AI_DIFFERENTIAL)
	{
		if (!(instance->features & ME4600_DIFFERENTIAL))
		{
			PERROR("Invalid reference specified. ME_REF_AI_DIFFERENTIAL not supported.\n");
			return ME_ERRNO_INVALID_REF;
		}
		if (channel >= (instance->channels / 2))
		{
			PERROR("Invalid channel specified. Only %d channels available in differential mode.\n", instance->channels / 2);
			return ME_ERRNO_INVALID_CHANNEL;
		}
	}

	if ((channel < 0) || (channel >= instance->channels))
	{
		PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->channels - 1);
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4600_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	int i;

	instance = (me4600_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	err = me4600_ai_io_single_config_check(instance, channel, single_config, ref, trig_chain, trig_type, trig_edge, flags);
	if (err)
		return err;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ai_status_none:
				case ai_status_single_configured:
				case ai_status_single_end:
				case ai_status_stream_configured:
				case ai_status_stream_timeout:
				case ai_status_stream_end:
				case ai_status_stream_fifo_error:
				case ai_status_stream_buffer_error:
					// OK - subdevice in idle
					break;

				case ai_status_single_run:
					//Subdevice running in single mode!
				case ai_status_stream_run_wait:
				case ai_status_stream_run:
				case ai_status_stream_end_wait:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				case ai_status_error:
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ai_status_none;
			}

			//Prepare data entry.
			// Common for all modes.
			instance->single_config[channel].entry = channel | ME4600_AI_LIST_LAST_ENTRY;

			if (ref == ME_REF_AI_DIFFERENTIAL)
			{// ME_REF_AI_DIFFERENTIAL
				instance->single_config[channel].entry |= ME4600_AI_LIST_INPUT_DIFFERENTIAL;
			}
			/*
			// ME4600_AI_LIST_INPUT_SINGLE_ENDED = 0x0000
			// 'entry |= ME4600_AI_LIST_INPUT_SINGLE_ENDED' <== Do nothing. Removed.
			else
			{// ME_REF_AI_GROUND
				instance->single_config[channel].entry |= ME4600_AI_LIST_INPUT_SINGLE_ENDED;
			}
			*/
			switch (single_config)
			{
				case 0:	//-10V..10V
						/*
						// ME4600_AI_LIST_RANGE_BIPOLAR_10 = 0x0000
						// 'entry |= ME4600_AI_LIST_RANGE_BIPOLAR_10' <== Do nothing. Removed.
						instance->single_config[channel].entry |= ME4600_AI_LIST_RANGE_BIPOLAR_10;
						*/
					break;

				case 1:	//0V..10V
					instance->single_config[channel].entry |= ME4600_AI_LIST_RANGE_UNIPOLAR_10;
					break;

				case 2:	//-2.5V..2.5V
					instance->single_config[channel].entry |= ME4600_AI_LIST_RANGE_BIPOLAR_2_5;
					break;

				case 3:	//0V..2.5V
					instance->single_config[channel].entry |= ME4600_AI_LIST_RANGE_UNIPOLAR_2_5;
					break;
			}

			// Prepare control register.
			// Common for all modes.
			// Use 'single' AI hardware mode.
			instance->single_config[channel].ctrl = ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;

			switch (trig_type)
			{
				case ME_TRIG_TYPE_NONE:
				case ME_TRIG_TYPE_SW:
					// Nothing to set.
					break;

				case ME_TRIG_TYPE_EXT_ANALOG:
					instance->single_config[channel].ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_ANALOG;

				case ME_TRIG_TYPE_EXT_DIGITAL:
					instance->single_config[channel].ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG;
					break;
			}

			switch (trig_edge)
			{
				case ME_TRIG_EDGE_RISING:
					// Nothing to set.
					break;

				case ME_TRIG_EDGE_ANY:
						instance->single_config[channel].ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_BOTH;

				case ME_TRIG_EDGE_FALLING:
						instance->single_config[channel].ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_FALLING;
					break;
			}

			// Enable this channel
			instance->single_config[channel].status = ME_SINGLE_CHANNEL_CONFIGURED;

			// Copy this settings to other outputs.
			if(flags == ME_IO_SINGLE_CONFIG_CONTINUE)
			{
				for(i=channel+1; i<instance->channels; i++)
				{
					instance->single_config[i].ctrl = instance->single_config[channel].ctrl;
					instance->single_config[i].entry = (instance->single_config[channel].entry & ~0x1F) | i;
					instance->single_config[i].status = ME_SINGLE_CHANNEL_CONFIGURED;
				}
			}

			instance->status = ai_status_single_configured;
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ai_io_single_read_check(me4600_ai_subdevice_t* instance, int channel,  int* value, int time_out, int flags)
{
	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((instance->status != ai_status_single_configured) && (instance->status != ai_status_single_end))
	{
		PERROR("Subdevice not configured to work in single mode!\n");
		return ME_ERRNO_PREVIOUS_CONFIG;
	}

	if ((channel < 0) || (channel >= instance->channels))
	{
		PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->channels - 1);
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if(instance->single_config[channel].status != ME_SINGLE_CHANNEL_CONFIGURED)
	{
		PERROR("Channel is not configured to work in single mode! (mode: %d)\n", instance->single_config[channel].status);
		return ME_ERRNO_PREVIOUS_CONFIG;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel,  int* value, int time_out, int flags)
{
	me4600_ai_subdevice_t* instance;
	uint32_t tmp;
	unsigned long int j;
	unsigned long int delay = LONG_MAX - 2;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	/// In this case I HAVE TO ignore non_blocking flag
	flags &= ~ME_IO_SINGLE_TYPE_NONBLOCKING;

	err = me4600_ai_io_single_read_check(instance, channel,  value, time_out, flags);
	if (err)
	{
    	return err;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ai_status_single_end:
					instance->status = ai_status_single_configured;
				case ai_status_single_configured:
					// OK - subdevice is ready
					break;

				case ai_status_none:
				case ai_status_stream_configured:
				case ai_status_stream_end:
				case ai_status_stream_timeout:
				case ai_status_stream_fifo_error:
				case ai_status_stream_buffer_error:
					//Subdevice is not ready!
					PERROR("Subdevice isn't prepare.\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;

				case ai_status_single_run:
					//Subdevice is running in single mode!
				case ai_status_stream_run_wait:
				case ai_status_stream_run:
				case ai_status_stream_end_wait:
					//Subdevice is running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				case ai_status_error:
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ai_status_none;
			}

			// Mark that StreamConfig is removed.
			instance->chan_list_len = 0;
			if (instance->chan_list_copy)
			{
				memset(instance->chan_list_copy, 0, instance->LE_size * sizeof(uint16_t));
			}
			instance->chan_list_copy_pos=0;

			me_readl(instance->base.dev, &tmp, instance->status_reg);
			if (tmp & ME4600_AI_STATUS_BIT_FSM)
			{
				// Status doesn't corespond with reality. Bad luck for reality.
				ai_stop_immediately(instance);
			}

			tmp = ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_LE_IRQ_RESET | ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
			me_writel(instance->base.dev, 0, instance->scan_timer_low_reg);
			me_writel(instance->base.dev, 0, instance->scan_timer_high_reg);
			me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
			me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);
			// Reactive FIFOs.
			tmp |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			me_writel(instance->base.dev, instance->single_config[channel].entry, instance->channel_list_reg);
			// Start work.
			me_writel(instance->base.dev, instance->single_config[channel].ctrl, instance->ctrl_reg);
			if (!(instance->single_config[channel].ctrl & ME4600_AI_CTRL_BIT_EX_TRIG))
			{// Software start
				me_readl(instance->base.dev, &tmp, instance->start_reg);
				delay = 2;
			}
			else
			{
				if (time_out)
				{
					delay = (time_out * HZ) / 1000;
					if (!delay)
						delay = 1;
					if (delay>LONG_MAX - 2)
						delay = LONG_MAX - 2;
				}
			}


			// Mark that single read is in progress.
			instance->status = ai_status_single_run;
			if (!(instance->single_config[channel].ctrl & ME4600_AI_CTRL_BIT_EX_TRIG))
			{// Software start.
				udelay(5);
				if (!(me4600_ai_FSM_test(instance) & ME4600_AI_STATUS_BIT_EF_DATA))
				{
					PERROR("Value not available after wait!\n");
					err = ME_ERRNO_INTERNAL;
				}
				instance->status = ai_status_single_end;
			}
			else
			{// External triggering
				ME_SUBDEVICE_UNLOCK;

				j = jiffies;
				while (!(me4600_ai_FSM_test(instance) & ME4600_AI_STATUS_BIT_EF_DATA))
				{
					if ((jiffies - j) >= delay)
					{
						PINFO("Timeout.\n");
						err = ME_ERRNO_TIMEOUT;
						break;
					}

					// Wait
					set_current_state(TASK_INTERRUPTIBLE);
					schedule_timeout(1);

					if (signal_pending(current))
					{
						PERROR("Wait interrupted by signal.\n");
						err = ME_ERRNO_SIGNAL;
						break;
					}

					if(instance->status != ai_status_single_run)
					{
						PERROR("Wait interrupted by reset.\n");
						err = ME_ERRNO_CANCELLED;
						break;
					}
				}
				ME_SUBDEVICE_LOCK;
			}

			// Read value.
			if(!err)
			{
				me_readl(instance->base.dev, &tmp, instance->data_reg);
				tmp = ai_calculate_calibrated_value(instance, instance->single_config[channel].entry, tmp);
				*value = tmp & ME4600_AI_MAX_DATA;

			}
			else
			{
				*value = 0xFFFFFFFF;
			}

			// Restore settings.
			tmp = ME4600_AI_CTRL_BIT_SC_IRQ | ME4600_AI_CTRL_BIT_HF_IRQ | ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_LE_IRQ_RESET | ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			instance->status = ai_status_single_end;
ERROR:
		ME_SUBDEVICE_UNLOCK
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ai_io_stream_config_check(me4600_ai_subdevice_t* instance, meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	int i;
	int err = ME_ERRNO_SUCCESS;

	if (flags & ~(ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD | ME_STREAM_CONFIG_DIFFERENTIAL))
	{
		PERROR("Invalid flags. Should be ME_IO_STREAM_CONFIG_NO_FLAGS, ME_STREAM_CONFIG_DIFFERENTIAL or ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD.\n");
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
			break;

		case ME_TRIGGER_TYPE_ACQ_ANALOG:
		case ME_TRIGGER_TYPE_LIST_ANALOG:
		case ME_TRIGGER_TYPE_CONV_ANALOG:
		if (!(instance->features & ME4600_ANALOG_TRIGGER))
		{
			PERROR("Invalid trigger type specified. ANALOG TRIGGER not supported.\n");
			err = ME_ERRNO_INVALID_TRIG_TYPE;
			goto ERROR;
		}
		case ME_TRIGGER_TYPE_ACQ_DIGITAL:
		case ME_TRIGGER_TYPE_LIST_DIGITAL:
		case ME_TRIGGER_TYPE_CONV_DIGITAL:
			switch (trigger->trigger_edge)
			{
				case ME_TRIG_EDGE_RISING:
				case ME_TRIG_EDGE_FALLING:
				case ME_TRIG_EDGE_ANY:
					break;

				default:
					PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_RISING, ME_TRIG_EDGE_FALLING or ME_TRIG_EDGE_ANY.\n");
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
	if ((trigger->acq_ticks != 0)
		&&
		(
			(trigger->acq_ticks < (uint64_t)ME4600_AI_MIN_ACQ_TICKS)
			||
			(trigger->acq_ticks > (uint64_t)ME4600_AI_MAX_ACQ_TICKS)
		)
		)
	{
		PERROR("Invalid acquisition start trigger argument specified. Must be between %lld and %lld.\n", ME4600_AI_MIN_ACQ_TICKS, ME4600_AI_MAX_ACQ_TICKS);
		err = ME_ERRNO_INVALID_ACQ_START_ARG;
		goto ERROR;
	}

	switch (trigger->trigger_type)
	{
		case ME_TRIGGER_TYPE_SOFTWARE:
		case ME_TRIGGER_TYPE_ACQ_DIGITAL:
		case ME_TRIGGER_TYPE_ACQ_ANALOG:
			if ((trigger->scan_ticks != 0)
				&&
				(
					(trigger->scan_ticks < (uint64_t)ME4600_AI_MIN_SCAN_TICKS)
					||
					(trigger->scan_ticks > (uint64_t)ME4600_AI_MAX_SCAN_TICKS)
				)
				)
			{
				PERROR("Invalid scan start argument specified. Must be between %lld and %lld.\n", ME4600_AI_MIN_SCAN_TICKS, ME4600_AI_MAX_SCAN_TICKS);
				err = ME_ERRNO_INVALID_SCAN_START_ARG;
				goto ERROR;
			}
			if ((trigger->conv_ticks != 0)
				&&
				(
					(trigger->conv_ticks < (uint64_t)ME4600_AI_MIN_CHAN_TICKS)
					||
					(trigger->conv_ticks > (uint64_t)ME4600_AI_MAX_CHAN_TICKS))
				)
			{
				PERROR("Invalid conv start trigger argument specified. Must be between %lld and %lld.\n", ME4600_AI_MIN_CHAN_TICKS, ME4600_AI_MAX_CHAN_TICKS);
				err = ME_ERRNO_INVALID_CONV_START_ARG;
				goto ERROR;
			}
			break;

		case ME_TRIGGER_TYPE_LIST_DIGITAL:
		case ME_TRIGGER_TYPE_LIST_ANALOG:
			if (trigger->scan_ticks)
			{
				PERROR("Invalid scan start argument specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_SCAN_START_ARG;
				goto ERROR;
			}
			if ((trigger->conv_ticks != 0)
				&&
				(
					(trigger->conv_ticks < (uint64_t)ME4600_AI_MIN_CHAN_TICKS)
					||
					(trigger->conv_ticks > (uint64_t)ME4600_AI_MAX_CHAN_TICKS))
				)
			{
				PERROR("Invalid conv start trigger argument specified. Must be between %lld and %lld.\n", ME4600_AI_MIN_CHAN_TICKS, ME4600_AI_MAX_CHAN_TICKS);
				err = ME_ERRNO_INVALID_CONV_START_ARG;
				goto ERROR;
			}
			break;

		case ME_TRIGGER_TYPE_CONV_DIGITAL:
		case ME_TRIGGER_TYPE_CONV_ANALOG:
			if (trigger->scan_ticks)
			{
				PERROR("Invalid scan start argument specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_SCAN_START_ARG;
				goto ERROR;
			}
			if (trigger->conv_ticks)
			{
				PERROR("Invalid conv start trigger type specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_CONV_START_TRIG_TYPE;
				goto ERROR;
			}
			break;
	}

	switch (trigger->stop_type)
	{
		case ME_STREAM_STOP_TYPE_ACQ_LIST:
			if (trigger->stop_count <= 0)
			{
				PERROR("Invalid stop count specified. Must be at least 1.\n");
				err = ME_ERRNO_INVALID_ACQ_STOP_ARG;
				goto ERROR;
			}
			break;

		case ME_STREAM_STOP_TYPE_SCAN_VALUE:
			if (trigger->stop_count <= 0)
			{
				PERROR("Invalid scan stop argument specified. Must be at least 1.\n");
				err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
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

	if((count <= 0)||(count > instance->LE_size))
	{
		PERROR("Invalid channel list count specified. Must be between 1 and %d.\n", instance->LE_size);
		err = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR;
	}

#ifdef ME_SYNAPSE
	/// This is general limitation. For Synapse it is a better choice.
	if (fifo_irq_threshold < 0 || fifo_irq_threshold >= me_seg_buf_size(instance->seg_buf))
	{
		PERROR("Invalid fifo irq threshold specified. Must be between 0 and %d.\n", me_seg_buf_size(instance->seg_buf) - 1);
		err = ME_ERRNO_INVALID_FIFO_IRQ_THRESHOLD;
		goto ERROR;
	}
#else
	/// This is limitation from Windows. I use it for compatibility.
	if (fifo_irq_threshold < 0 || fifo_irq_threshold > instance->fifo_size)
	{
		PERROR("Invalid fifo irq threshold specified. Must be between 0 and %d.\n", instance->fifo_size);
		err = ME_ERRNO_INVALID_FIFO_IRQ_THRESHOLD;
		goto ERROR;
	}
#endif

	if (flags & ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD)
	{
		if (!(instance->features & ME4600_SAMPLE_HOLD))
		{
			PERROR("Invalid flag specified. ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD not supported.\n");
			err = ME_ERRNO_INVALID_FLAGS;
			goto ERROR;
		}
		if (flags & ME_STREAM_CONFIG_DIFFERENTIAL)
		{
			PERROR("Sample and hold is not available in differential mode.\n");
			err = ME_ERRNO_INVALID_FLAGS;
			goto ERROR;
		}
	}

	for (i = 0; i < count; i++)
	{
		if ((config_list[i].iRange < 0) || (config_list[i].iRange >= instance->ranges_len))
		{
			PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->ranges_len - 1);
			err = ME_ERRNO_INVALID_STREAM_CONFIG;
			goto ERROR;
		}

		if (config_list[i].iRange % 2)
		{// StreamConfig: 1 or 3
			if (flags & ME_STREAM_CONFIG_DIFFERENTIAL)
			{
				PERROR("Only bipolar modes support differential measurement.\n");
				err = ME_ERRNO_INVALID_REF;
				goto ERROR;
			}
		}

		if ((flags & ME_STREAM_CONFIG_DIFFERENTIAL) && (config_list[i].iChannel >= (instance->channels / 2)))
		{
			PERROR("Invalid channel specified. Only %d channels available in differential mode.\n", instance->channels / 2);
			err = ME_ERRNO_INVALID_CHANNEL;
			goto ERROR;
		}

		if ((config_list[i].iChannel < 0) || (config_list[i].iChannel >= instance->channels))
		{
			PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->channels - 1);
			err = ME_ERRNO_INVALID_CHANNEL;
			goto ERROR;
		}
	}

ERROR:
	return err;
}

int me4600_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	me4600_ai_subdevice_t* instance;
	int i;			// internal multipurpose variable
	unsigned long long data_required;

	volatile uint32_t entry;
	uint32_t ctrl;
	uint32_t tmp;	// use when current copy of register's value needed

	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	err = me4600_ai_io_stream_config_check(instance, config_list, count,  trigger, fifo_irq_threshold, flags);
	if (err)
		return err;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ai_status_none:
				case ai_status_single_configured:
				case ai_status_single_end:
				case ai_status_stream_configured:
				case ai_status_stream_end:
				case ai_status_stream_fifo_error:
				case ai_status_stream_buffer_error:
				case ai_status_stream_timeout:
					// OK - subdevice in idle
					break;

				case ai_status_single_run:
					//Subdevice running in single mode!
				case ai_status_stream_run_wait:
				case ai_status_stream_run:
				case ai_status_stream_end_wait:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				case ai_status_error:
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ai_status_none;
			}

			// Default (minimal) start delay
			if (trigger->acq_ticks < (uint64_t)ME4600_AI_MIN_ACQ_TICKS)
			{
				trigger->acq_ticks = (uint64_t)ME4600_AI_MIN_ACQ_TICKS;
			}

			// Default (minimal) conversion's time
			if (trigger->conv_ticks < (uint64_t)ME4600_AI_MIN_CHAN_TICKS)
			{
				trigger->conv_ticks = (uint64_t)ME4600_AI_MIN_CHAN_TICKS;
			}

			switch (trigger->trigger_type)
			{
				case ME_TRIGGER_TYPE_ACQ_DIGITAL:
				case ME_TRIGGER_TYPE_ACQ_ANALOG:
				case ME_TRIGGER_TYPE_SOFTWARE:

					if(count == 1)
					{
						// The hardware does not work properly with a non-zero scan time
						// if there is only ONE channel in the channel list. In this case
						// we must set the scan time to zero and use the channel time.

						trigger->conv_ticks = trigger->scan_ticks;
						trigger->scan_ticks = 0;
					}

					// Too short time
					if(trigger->scan_ticks <= count * trigger->conv_ticks)
					{
						// Another hardware problem. If the number of scan ticks is
						// exactly equal to the number of channel ticks multiplied by
						// the number of channels then the sampling rate is reduced
						// by half.
						trigger->scan_ticks = 0;
					}
					break;
			}

			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			// Preserve OFFSET settings. Clean other bits.
			tmp &= (ME4600_AI_CTRL_BIT_FULLSCALE | ME4600_AI_CTRL_BIT_OFFSET);

			// Stop all actions. Block all interrupts. Clear (disable) FIFOs.
			ctrl = ME4600_AI_CTRL_BIT_IMMEDIATE_STOP |ME4600_AI_CTRL_BIT_LE_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
			tmp |= ctrl;

			// Send it to register.
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			// Enable channel fifo -> data fifo in stream_start().
			ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO;
			me_writel(instance->base.dev, tmp | ctrl, instance->ctrl_reg);
			// Write the channel list
			for (i = 0; i < count; i++)
			{
				entry = config_list[i].iChannel;
				switch (config_list[i].iRange)
				{
					case 0:						//BIPOLAR 10V
	/*
						// ME4600_AI_LIST_RANGE_BIPOLAR_10 = 0x0000
						// 'entry |= ME4600_AI_LIST_RANGE_BIPOLAR_10' <== Do nothing. Removed.
						entry |= ME4600_AI_LIST_RANGE_BIPOLAR_10;
	*/
						break;
					case 1:						//UNIPOLAR 10V
						entry |= ME4600_AI_LIST_RANGE_UNIPOLAR_10;
						break;
					case 2:						//BIPOLAR 2.5V
						entry |= ME4600_AI_LIST_RANGE_BIPOLAR_2_5;
						break;
					case 3:						//UNIPOLAR 2.5V
						entry |= ME4600_AI_LIST_RANGE_UNIPOLAR_2_5;
						break;
				}

				if (flags & ME_STREAM_CONFIG_DIFFERENTIAL)
				{ //DIFFERENTIAL
					entry |= ME4600_AI_LIST_INPUT_DIFFERENTIAL;
				}

				//Add last entry flag
				if (i == (count - 1))
				{
					entry |= ME4600_AI_LIST_LAST_ENTRY;
				}
				me_writel(instance->base.dev, entry, instance->channel_list_reg);
				if (instance->chan_list_copy)
				{
					*(instance->chan_list_copy + i) = (uint16_t)(entry & ME4600_AI_LIST_CONFIG_MASK);
				}
			}
			instance->chan_list_copy_pos=0;

			--trigger->acq_ticks;
			--trigger->conv_ticks;
			if (trigger->scan_ticks)
			{
				--trigger->scan_ticks;
			}


			// Set triggers
			switch (trigger->trigger_type)
			{
				case ME_TRIGGER_TYPE_SOFTWARE:
					PINFO("Internal software trigger.\n");
					// Nothing to set.
					break;

				case ME_TRIGGER_TYPE_ACQ_ANALOG:
				case ME_TRIGGER_TYPE_LIST_ANALOG:
				case ME_TRIGGER_TYPE_CONV_ANALOG:
					ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_ANALOG;

				case ME_TRIGGER_TYPE_ACQ_DIGITAL:
				case ME_TRIGGER_TYPE_LIST_DIGITAL:
				case ME_TRIGGER_TYPE_CONV_DIGITAL:
					ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG;
					// External trigger needs edge's definition
					switch (trigger->trigger_edge)
					{
						case ME_TRIG_EDGE_RISING:
							// Nothing to set.
							break;

						case ME_TRIG_EDGE_FALLING:
							ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_FALLING;
							break;

						case ME_TRIG_EDGE_ANY:
							ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_FALLING | ME4600_AI_CTRL_BIT_EX_TRIG_BOTH;
							break;
					}
					PINFO("External %s trigger. Mode: %s.\n", (trigger->trigger_type & ME_TRIGGER_TYPE_ANALOG) ? "analog" : "digital", (trigger->trigger_type & ME_TRIGGER_TYPE_ACQ) ? "ACQ" : (trigger->trigger_type & ME_TRIGGER_TYPE_LIST) ? "LIST" : "CONV");
					break;
			}

			switch (trigger->trigger_type)
			{
				case ME_TRIGGER_TYPE_SOFTWARE:
					ctrl |= ME4600_AI_CTRL_BIT_MODE_0;
					break;

				case ME_TRIGGER_TYPE_ACQ_DIGITAL:
				case ME_TRIGGER_TYPE_ACQ_ANALOG:
					ctrl |= ME4600_AI_CTRL_BIT_MODE_1;
					break;

				case ME_TRIGGER_TYPE_LIST_DIGITAL:
				case ME_TRIGGER_TYPE_LIST_ANALOG:
					ctrl |= ME4600_AI_CTRL_BIT_MODE_2;
					break;

				case ME_TRIGGER_TYPE_CONV_DIGITAL:
				case ME_TRIGGER_TYPE_CONV_ANALOG:
					ctrl |= ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1;
					break;
			}

			// Set triggers registers
			me_writel(instance->base.dev, trigger->acq_ticks, instance->chan_pre_timer_reg);

			me_writel(instance->base.dev, trigger->acq_ticks, instance->scan_pre_timer_low_reg);
			me_writel(instance->base.dev, (trigger->acq_ticks>>32), instance->scan_pre_timer_high_reg);

			me_writel(instance->base.dev, trigger->scan_ticks, instance->scan_timer_low_reg);
			me_writel(instance->base.dev, (trigger->scan_ticks >> 32), instance->scan_timer_high_reg);

			me_writel(instance->base.dev, trigger->conv_ticks, instance->chan_timer_reg);

			//Sample & Hold feature
			if ((flags & ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD) && (instance->features & ME4600_SAMPLE_HOLD))
			{
				ctrl |= ME4600_AI_CTRL_BIT_SAMPLE_HOLD;
			}

			//Everything is good. Finalize
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			//Preserve OFFSET settings. Clean other bits.
			tmp &= (ME4600_AI_CTRL_BIT_FULLSCALE | ME4600_AI_CTRL_BIT_OFFSET);
			//Enable IRQs sources but leave latches blocked.
			ctrl |= (ME4600_AI_CTRL_BIT_HF_IRQ | ME4600_AI_CTRL_BIT_SC_IRQ | ME4600_AI_CTRL_BIT_LE_IRQ);	//The last IRQ source (ME4600_AI_CTRL_BIT_LE_IRQ) is unused!
			// write the control word
			me_writel(instance->base.dev, tmp | ctrl, instance->ctrl_reg);

			//Set the global parameters end exit.
			instance->chan_list_len = count;
			instance->fifo_irq_threshold = fifo_irq_threshold;


			switch (trigger->stop_type)
			{
				case ME_STREAM_STOP_TYPE_SCAN_VALUE:
					data_required = (unsigned long long)trigger->stop_count;
					break;

				case ME_STREAM_STOP_TYPE_ACQ_LIST:
					data_required = (unsigned long long)trigger->stop_count * (unsigned long long)count;
					break;

				default:
					data_required = 0;
			}
			if(data_required > UINT_MAX)
				data_required = UINT_MAX;
			instance->data_required = (unsigned int)data_required;

			// Deinit single config. Set all entries to NOT_CONFIGURED.
			for (i = 0; i < instance->channels; i++)
			{
				instance->single_config[i].status = ME_SINGLE_CHANNEL_NOT_CONFIGURED;
			}

			// Mark subdevice as configured to work in stream mode.
			instance->status = ai_status_stream_configured;
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}


static int me4600_ai_FSM_test(me4600_ai_subdevice_t* instance)
{
	uint32_t tmp;

	me_readl(instance->base.dev, &tmp, instance->status_reg);

	return tmp;
}

int me4600_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags)
{
	me4600_ai_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long int j;
// 	int volatile head;
	unsigned int writes_count;
	int status;
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

	instance = (me4600_ai_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		j = jiffies;

		while(1)
		{
			status = instance->status;
			if (flags &  ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG)
			{// Report errors
				if (status == ai_status_stream_fifo_error)
				{
					err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
				}
				else if (status == ai_status_stream_buffer_error)
				{
					err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
				}
				else if (status == ai_status_stream_timeout)
				{
					err = ME_ERRNO_CANCELLED;
				}
				else if (status == ai_status_none)
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
						(status != instance->status)
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
						(status != instance->status)
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

			if (instance->status == ai_status_stream_fifo_error)
			{
				err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
			}
			else if (instance->status == ai_status_stream_buffer_error)
			{
				err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
			}
			else if (instance->status == ai_status_none)
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

			if (err || (instance->status == ai_status_stream_end))
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

static int inline me4600_ai_io_stream_read_get_value(me4600_ai_subdevice_t* instance, int* values, const int count, const int flags)
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
		if (n < instance->chan_list_len)	//Not enough data!
			return 0;
		n -= n % instance->chan_list_len;
	}

	for (i=0; i<n; i++)
	{
		ME_LOCK_PROTECTOR;
			me_seg_buf_get(instance->seg_buf, &tmp);
		ME_UNLOCK_PROTECTOR;
		value = tmp;
		if(put_user(value, values + i))
		{
			PERROR("Cannot copy new values to user.\n");
			return -ME_ERRNO_INTERNAL;
		}
	}
	return n;
}

static int me4600_ai_io_stream_read_check(me4600_ai_subdevice_t* instance, int read_mode, int* values, int* count, int time_out, int flags)
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

int me4600_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags)
{
	me4600_ai_subdevice_t* instance;
	int ret;
	uint32_t tmp;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long int j;
	int err = ME_ERRNO_SUCCESS;

	int c = *count;
	int min = c;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	err = me4600_ai_io_stream_read_check(instance, read_mode, values, count, time_out, flags);
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
			if( instance->chan_list_len <= 0)
			{
				PERROR("Subdevice wasn't configured.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}

			if (c < instance->chan_list_len)
			{	//Not enough data requested.
				PERROR("When using FRAME_READ mode minimal size is defined by channel list.\n");
				err = ME_ERRNO_INVALID_VALUE_COUNT;
				goto ERROR;
			}

			//Wait for whole list.
			if (read_mode == ME_READ_MODE_BLOCKING)
			{
				min = c - (c % instance->chan_list_len);
			}

			if (read_mode == ME_READ_MODE_NONBLOCKING)
			{
				min = instance->chan_list_len;
			}
		}
		else
		{
			if (c > me_seg_buf_size(instance->seg_buf))
			{// To return acceptable amount of data when user pass too big value. For security this is not bigger than ME4600_AI_CIRC_BUF_COUNT - 2
				min = me_seg_buf_size(instance->seg_buf);
				min -= (instance->chan_list_len > 2) ? instance->chan_list_len : 2;
			}

		}

		me_readl(instance->base.dev, &tmp, instance->status_reg);
		if ((tmp & ME4600_AI_STATUS_BIT_FSM))
		{//Working
			//If blocking mode -> wait for data.
			if ((me_seg_buf_values(instance->seg_buf) < min) && (read_mode == ME_READ_MODE_BLOCKING))
			{
				j = jiffies;
				wait_event_interruptible_timeout(
					instance->wait_queue,
					((me_seg_buf_values(instance->seg_buf) >= min) || !(me4600_ai_FSM_test(instance) & ME4600_AI_STATUS_BIT_FSM)),
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

		ret = me4600_ai_io_stream_read_get_value(instance, values, c, flags);
		if (ret < 0)
		{
			err = -ret;
			*count = 0;
		}
		else if (ret == 0)
		{
			ME_SUBDEVICE_LOCK;
				*count = 0;
				switch (instance->status)
				{
					case ai_status_stream_end:
						if (instance->empty_read_count)
						{
							err = ME_ERRNO_SUBDEVICE_NOT_RUNNING;
						}
						instance->empty_read_count = 1;
						break;

					case ai_status_stream_fifo_error:
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
						instance->status = ai_status_stream_end;
						break;

					case ai_status_stream_buffer_error:
						err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
						instance->status = ai_status_stream_end;
						break;

					case ai_status_none:
					case ai_status_stream_timeout:
						err = ME_ERRNO_CANCELLED;
						break;

					case ai_status_error:
						err = ME_ERRNO_COMMUNICATION;
						break;

					default:
						break;
				}
			ME_SUBDEVICE_UNLOCK;
		}
		else
		{
			*count = ret;
		}

		if (ret || (instance->status != ai_status_stream_end))
		{
			instance->empty_read_count = 0;
		}
ERROR:
	ME_SUBDEVICE_EXIT;
	return err;
}

/** @brief Stop aqusation. Preserve FIFOs.
 *
 * @param instance The subdevice instance (pointer).
 */

static int ai_stop_immediately(me4600_ai_subdevice_t* instance)
{
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	tmp &= ~ME4600_AI_CTRL_BIT_STOP;
	tmp |= (ME4600_AI_CTRL_BIT_IMMEDIATE_STOP | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags)
{
	me4600_ai_subdevice_t* instance;
	unsigned long int end_wait;
	unsigned long int ref;
	unsigned long int delay = LONG_MAX-2;

	uint32_t tmp;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

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
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ai_status_none:
				case ai_status_single_configured:
				case ai_status_single_end:
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;

				case ai_status_stream_configured:
				case ai_status_stream_timeout:
				case ai_status_stream_end:
				case ai_status_stream_fifo_error:
				case ai_status_stream_buffer_error:
					// OK - subdevice in idle
					break;

				case ai_status_single_run:
					//Subdevice running in single mode!
				case ai_status_stream_run_wait:
				case ai_status_stream_run:
				case ai_status_stream_end_wait:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ai_status_none;
					goto ERROR;
			}

			if(instance->chan_list_len == 0)
			{//Not configured!
				PERROR("Subdevice is not configured to work in stream mode!\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);


			if(!(tmp & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2)))
			{//Mode 0 = single work => no stream config
				PERROR_CRITICAL("Subdevice is configured to work in single mode.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}

			if ((tmp & ME4600_AI_STATUS_BIT_FSM))
			{
				PERROR_CRITICAL("Conversion is already running.\n");
				err = ME_ERRNO_INTERNAL;
				goto ERROR;
			}

			//Reset status bits.
			tmp &= ~(  ME4600_AI_STATUS_BIT_EF_CHANNEL
					 | ME4600_AI_STATUS_BIT_HF_CHANNEL
					 | ME4600_AI_STATUS_BIT_FF_CHANNEL
					 | ME4600_AI_STATUS_BIT_EF_DATA
					 | ME4600_AI_STATUS_BIT_HF_DATA
					 | ME4600_AI_STATUS_BIT_LE
					 | ME4600_AI_STATUS_BIT_FSM);
			//Reset stop bits.
			tmp |= ME4600_AI_CTRL_BIT_IMMEDIATE_STOP | ME4600_AI_CTRL_BIT_STOP;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);

			//Start datas' FIFO.
			tmp |= ME4600_AI_CTRL_BIT_DATA_FIFO;
			//Free stop bits.
			tmp &= ~(ME4600_AI_CTRL_BIT_IMMEDIATE_STOP | ME4600_AI_CTRL_BIT_STOP);
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);

			instance->ISM.global_read = 0;
			instance->ISM.read = 0;
			//Clear circular buffer
			me_seg_buf_reset(instance->seg_buf);

			//Set everything.
			ai_data_acquisition_logic(instance);

			//Set status to 'wait for start'
			instance->status = ai_status_stream_run_wait;

			// Set control task's timeout
			instance->timeout.delay = delay;
			instance->timeout.start_time = jiffies;

			if(tmp & (ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2))
			{//Modes 2,3,4 = hardware trigger
				PDEBUG("Hardware trigger. Lets go! Start work.\n");
			}
			else
			{
				PDEBUG("Software trigger. Lets go! Start work.\n");
				//Lets go! Start work
				me_readl(instance->base.dev, &tmp, instance->start_reg);
			}

			// Schedule control task
			PDEBUG("Schedule control task.\n");
			atomic_set(&instance->ai_control_task_flag, 1);
			queue_delayed_work(instance->me4600_workqueue, &instance->ai_control_task, 1);

			if (start_mode == ME_START_MODE_BLOCKING)
			{//Wait for start.
				ME_SUBDEVICE_UNLOCK;
				ref = jiffies;
				//Only runing process will interrupt this call. Events are signaled when status change. Extra 1 tick timeout added for safe reason.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(instance->status != ai_status_stream_run_wait),
					delay+1);

				end_wait = jiffies;
				ME_SUBDEVICE_LOCK;
				if (signal_pending(current))
				{
					PERROR("Wait on start of state machine interrupted.\n");
					err = ME_ERRNO_SIGNAL;
					goto ERROR;
				}

				switch (instance->status)
				{
					case ai_status_stream_run_wait:
						PERROR("Timeout reached. Not handled by control task! %d\n", instance->status);
						ai_stop_isr(instance);
						atomic_set(&instance->ai_control_task_flag, 0);
						cancel_delayed_work(&instance->ai_control_task);
						instance->status = ai_status_stream_timeout;
						err = ME_ERRNO_TIMEOUT;
						break;

					case ai_status_stream_timeout:
						PDEBUG("Timeout reached.\n");
						err = ME_ERRNO_TIMEOUT;
						break;

					case ai_status_stream_fifo_error:
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
						break;

					case ai_status_stream_buffer_error:
						err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
						break;

					case ai_status_stream_run:
					case ai_status_stream_end_wait:
					case ai_status_stream_end:
						// OK
						break;

					default:
						PDEBUG("Stream canceled.\n");
						err = ME_ERRNO_CANCELLED;
				}
			}

#ifdef MEDEBUG_INFO
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);

			PINFO("%s:\n", __FUNCTION__);
			PINFO("STATUS_BIT_FSM=%s.\n", (tmp & ME4600_AI_STATUS_BIT_FSM)?"on":"off");
			PINFO("CTRL_BIT_HF_IRQ=%s.\n", (tmp & ME4600_AI_CTRL_BIT_HF_IRQ)?"enable":"disable");
			PINFO("CTRL_BIT_HF_IRQ_RESET=%s.\n", (tmp & ME4600_AI_CTRL_BIT_HF_IRQ_RESET)?"reset":"work");
			PINFO("CTRL_BIT_SC_IRQ=%s.\n", (tmp & ME4600_AI_CTRL_BIT_SC_IRQ)?"enable":"disable");
			PINFO("CTRL_BIT_SC_RELOAD=%s.\n", (tmp & ME4600_AI_CTRL_BIT_SC_RELOAD)?"on":"off");
			PINFO("CTRL_BIT_SC_IRQ_RESET=%s.\n", (tmp & ME4600_AI_CTRL_BIT_SC_IRQ_RESET)?"reset":"work");
#endif

ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags)
{
	me4600_ai_subdevice_t* instance;
	uint32_t tmp;
	int old_count;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

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
		switch (instance->status)
		{
			case ai_status_single_configured:
			case ai_status_single_end:
			case ai_status_stream_configured:
			case ai_status_stream_end:
			case ai_status_stream_fifo_error:
			case ai_status_stream_buffer_error:
			case ai_status_stream_timeout:
				*status = ME_STATUS_IDLE;
				break;

			case ai_status_stream_run_wait:
			case ai_status_stream_run:
			case ai_status_stream_end_wait:
				*status = ME_STATUS_BUSY;
				break;

			default:
				PERROR_CRITICAL("WRONG STATUS!\n");
				instance->status = ai_status_none;
			case ai_status_none:
				me_readl(instance->base.dev, &tmp, instance->status_reg);
				*status = (tmp & ME4600_AI_STATUS_BIT_FSM) ? ME_STATUS_BUSY : ME_STATUS_IDLE;
				break;
		}

		if ((wait == ME_WAIT_IDLE) && (*status == ME_STATUS_BUSY))
		{
			// Only runing process will interrupt this call. Events are signaled when status change.
			wait_event_interruptible(
				instance->wait_queue,
				(
					(instance->status != ai_status_stream_run_wait)
					&&
					(instance->status != ai_status_stream_run)
					&&
					(instance->status != ai_status_stream_end_wait)
				));

			if (instance->status != ai_status_stream_end)
			{
				PDEBUG("Wait for IDLE canceled. 0x%x\n", instance->status);
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
			wait_event_interruptible(
				instance->wait_queue,
				(
					(instance->status == ai_status_stream_run_wait)
					||
					(instance->status == ai_status_stream_run)
					||
					(instance->status == ai_status_stream_end_wait)
					||
					(instance->status == ai_status_none)
				));

			if (instance->status == ai_status_none)
			{
				PDEBUG("Wait for BUSY canceled. 0x%x\n", instance->status);
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
			if ((old_count == instance->stream_start_count) || (instance->status == ai_status_none))
			{
				wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_start_count) || (instance->status == ai_status_none)));
			}

			if (instance->status == ai_status_none)
			{
				PDEBUG("Wait for START canceled.\n");
				err = ME_ERRNO_CANCELLED;
			}

			if (instance->status == ai_status_stream_timeout)
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
			if ((old_count == instance->stream_stop_count) || (instance->status == ai_status_none))
			{
				wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_stop_count) || (instance->status == ai_status_none)));
			}

			if (instance->status != ai_status_stream_end)
			{
				PDEBUG("Wait for STOP canceled. 0x%x\n", instance->status);
				err = ME_ERRNO_CANCELLED;
			}

			if (instance->status == ai_status_stream_timeout)
			{
				PDEBUG("Wait for STOP: report starting timeout.\n");
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

int me4600_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags)
{
/**
 @note Stop is implemented only in blocking mode.
 @note Function return when state machine is stoped.
*/
	me4600_ai_subdevice_t* instance;
	uint32_t tmp;
	unsigned long int delay = LONG_MAX - 2;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	if (flags & ~ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_STOP_TYPE_NO_FLAGS or ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS.\n");
		if (flags & ME_IO_STREAM_STOP_NONBLOCKING)
		{
			PERROR("ME_IO_STREAM_STOP_NONBLOCKING not supported.\n");
		}
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((stop_mode != ME_STOP_MODE_IMMEDIATE) && (stop_mode != ME_STOP_MODE_LAST_VALUE))
	{
		PERROR("Invalid stop mode specified. Must be ME_STOP_MODE_IMMEDIATE or ME_STOP_MODE_LAST_VALUE.\n");
		return ME_ERRNO_INVALID_STOP_MODE;
	}

	instance = (me4600_ai_subdevice_t *) subdevice;

	if (time_out)
	{
		delay = (time_out * HZ) / 1000;
		if (!delay)
			delay = 1;
		if (delay>LONG_MAX - 2)
			delay = LONG_MAX - 2;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			switch (instance->status)
			{
				case ai_status_stream_fifo_error:
				case ai_status_stream_buffer_error:
				case ai_status_stream_timeout:
					if (flags)
					{// Safe buffer.
						ai_stop_immediately(instance);
					}
					else
					{// Delete buffer.
						ai_stop_isr(instance);
					}
					instance->status = ai_status_stream_end;
					goto END;
					break;

				case ai_status_none:
				case ai_status_single_configured:
				case ai_status_single_end:
				case ai_status_stream_configured:
				case ai_status_stream_end:
					goto END;
					break;

				case ai_status_single_run:
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;

				case ai_status_stream_run_wait:
				case ai_status_stream_run:
				case ai_status_stream_end_wait:
					// Mark as stopping. => Software stop.
					instance->status = ai_status_stream_end_wait;
					if (stop_mode == ME_STOP_MODE_IMMEDIATE)
					{
						if (ai_stop_immediately(instance))
						{
							goto ERROR;
						}
					}
					else if (stop_mode == ME_STOP_MODE_LAST_VALUE)
					{
						// Set stop bit in registry.
						me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
						tmp |= ME4600_AI_CTRL_BIT_STOP;
						me_writel(instance->base.dev, tmp, instance->ctrl_reg);
					}
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ai_status_none;
					goto END;

			}

			// Wait for control task.
			ME_UNLOCK_PROTECTOR;
			// Only runing process will interrupt this call. Events are signaled when status change.
			wait_event_interruptible_timeout(
				instance->wait_queue,
				(instance->status != ai_status_stream_end_wait),
				delay);
			ME_LOCK_PROTECTOR;

			if (signal_pending(current))
			{
				PERROR("Stopping stream interrupted.\n");
				err = ME_ERRNO_SIGNAL;
				goto ERROR;
			}
			else
			{
				if (instance->status == ai_status_stream_end_wait)
				{
					PDEBUG("Timeout reached.\n");
					err = ME_ERRNO_TIMEOUT;
					ai_stop_immediately(instance);
				}

				if (instance->status == ai_status_none)
				{
					PDEBUG("Stopping stream canceled.\n");
					err = ME_ERRNO_CANCELLED;
				}
			}
END:

			if (flags || (stop_mode == ME_STOP_MODE_LAST_VALUE))
			{
				if(ai_read_data_pooling(instance) < 0)
				{// Buffer is overflow.
					instance->status = ai_status_stream_buffer_error;
				}
			}

			ai_stop_isr(instance);
			instance->stream_stop_count++;

			// Signal that we put last data to software buffer.
			wake_up_interruptible_all(&instance->wait_queue);

ERROR:
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{
	me4600_ai_subdevice_t* instance;
	int i;
	int r = -1;
	int diff = 21E6;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	if (*max < *min)
	{
		PERROR("Invalid minimum and maximum values specified. MIN: %d > MAX: %d\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	if (unit == ME_UNIT_VOLT)
	{
		for (i = 0; i < instance->ranges_len; i++)
		{
			if ((instance->ranges[i].min <= *min) && ((instance->ranges[i].max + (instance->ranges[i].max >> ME4600_RANGE_INACCURACY)) >= *max))
			{
				if ((instance->ranges[i].max - instance->ranges[i].min) - (*max - *min) < diff)
				{
					r = i;
					diff = (instance->ranges[i].max - instance->ranges[i].min) - (*max - *min);
				}
			}
		}

		if (r < 0)
		{
			PERROR("No matching range found.\n");
			return ME_ERRNO_NO_RANGE;
		}
		else
		{
			*min = instance->ranges[r].min;
			*max = instance->ranges[r].max;
			*maxdata = ME4600_AI_MAX_DATA;
			*range = r;
		}
	}
	else
	{
		PERROR("Invalid physical unit specified. Should be ME_UNIT_VOLT.\n");
		return ME_ERRNO_INVALID_UNIT;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{
	me4600_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	if ((unit == ME_UNIT_VOLT) || (unit == ME_UNIT_ANY))
	{
		*count = instance->ranges_len;
	}
	else
	{
		*count = 0;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	me4600_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	if ((range < 0) || (range >= instance->ranges_len))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->ranges_len - 1);
		return ME_ERRNO_INVALID_RANGE;
	}

	*unit = ME_UNIT_VOLT;
	*min = instance->ranges[range].min;
	*max = instance->ranges[range].max;
	*maxdata = ME4600_AI_MAX_DATA;

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	me4600_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;

	*base_frequency = ME4600_AI_BASE_FREQUENCY;
	switch (timer)
	{
		case ME_TIMER_ACQ_START:
			*min_ticks = ME4600_AI_MIN_ACQ_TICKS;
			*max_ticks = ME4600_AI_MAX_ACQ_TICKS;
			break;

		case ME_TIMER_SCAN_START:
			*min_ticks = ME4600_AI_MIN_SCAN_TICKS;
			*max_ticks = ME4600_AI_MAX_SCAN_TICKS;
			break;

		case ME_TIMER_CONV_START:
			*min_ticks = ME4600_AI_MIN_CHAN_TICKS;
			*max_ticks = ME4600_AI_MAX_CHAN_TICKS;
			break;

		default:
			PERROR("Invalid timer specified. Must be ME_TIMER_ACQ_START, ME_TIMER_SCAN_START or ME_TIMER_CONV_START.\n");

			return ME_ERRNO_INVALID_TIMER;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	me4600_ai_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *) subdevice;
	*number = instance->channels;

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed. idx=0\n");

	*type = ME_TYPE_AI;
	*subtype = ME_SUBTYPE_STREAMING;

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	me4600_ai_subdevice_t* instance;

	instance = (me4600_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	*caps = me4600_AI_CAPS;
	if (instance->features & ME4600_SAMPLE_HOLD)
	{
		*caps = *caps | ME_CAPS_AI_SAMPLE_HOLD;
	}

/*
	if (instance->features & ME4600_ANALOG_TRIGGER)
	{
		*caps = *caps | ME_CAPS_AI_TRIG_ANALOG;
	}
*/
	if (instance->features & ME4600_DIFFERENTIAL)
	{
		*caps = *caps | ME_CAPS_AI_DIFFERENTIAL;
	}

	PINFO("CAPS: %x\n", *caps);

	return ME_ERRNO_SUCCESS;
}

int me4600_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count)
{
	me4600_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ai_subdevice_t *) subdevice;

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
			*args = instance->fifo_size;
		break;

		case ME_CAP_AI_BUFFER_SIZE:
			*args = (instance->seg_buf) ? me_seg_buf_size(instance->seg_buf) : 0;
		break;

		case ME_CAP_AI_CHANNEL_LIST_SIZE:
			*args = instance->LE_size;
		break;

		case ME_CAP_AI_MAX_THRESHOLD_SIZE:
#ifdef ME_SYNAPSE
			*args = (instance->seg_buf) ? me_seg_buf_size(instance->seg_buf) : 0;
#else
			*args = instance->fifo_size;
#endif
		break;

		default:
			*count = 0;
			*args = 0;
	}

	return err;
}

int ai_limited_isr(me4600_ai_subdevice_t* instance, const uint32_t irq_status, const uint32_t ctrl_status)
{
	int to_read;
	int ret = 0;

	PDEBUG("executed. idx=0\n");

	if (!instance->fifo_irq_threshold)
	{//No threshold provided. SC ends work. HF need reseting.
		if (irq_status & ME4600_IRQ_STATUS_BIT_SC)
		{
			instance->ISM.global_read += instance->ISM.next;
			if (ai_read_data(instance, instance->ISM.next) != instance->ISM.next)
			{//ERROR!
				PERROR("Limited amounts aqusition with TH=0: Circular buffer full!\n");
				instance->status = ai_status_stream_buffer_error;
			}
			else
			{
				instance->status = ai_status_stream_end;
			}
			instance->stream_stop_count++;
			//End of work.
			ai_stop_isr(instance);
		}
		else if (irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
		{
			instance->ISM.global_read += instance->fifo_half_size;
			if (ai_read_data(instance, instance->fifo_half_size) != instance->fifo_half_size)
			{//ERROR!
				PERROR("Limited amounts aqusition with TH = 0: Circular buffer full!\n");
				//End of work.
				ai_stop_isr(instance);
				instance->status = ai_status_stream_buffer_error;
				instance->stream_stop_count++;
			}
			else
			{
				//Continue.
				ai_limited_ISM(instance, irq_status);
			}
		}

		//Signal user.
		ret = 1;
	}
	else	//if(instance->fifo_irq_threshold)
	{
		if (irq_status & ME4600_IRQ_STATUS_BIT_SC)
		{
			instance->ISM.read = 0;
			if((instance->fifo_irq_threshold < instance->fifo_half_size) && (!(ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)))
			{
				to_read = instance->fifo_half_size - (instance->fifo_half_size % instance->fifo_irq_threshold);
				PDEBUG("Limited amounts aqusition with TH != 0: Not fast enough data aqusition! correction=%d\n", to_read);
			}
			else
			{
				to_read = instance->ISM.next;
			}
			instance->ISM.global_read += to_read;


			ai_reschedule_SC(instance);

			if (ai_read_data(instance, to_read) != to_read)
			{//ERROR!
				PERROR("Limited amounts aqusition with TH != 0: Circular buffer full!\n");
				//End of work.
				ai_stop_isr(instance);
				instance->status = ai_status_stream_buffer_error;
				instance->stream_stop_count++;
			}
			else
			{
				//Continue.
				ai_limited_ISM(instance, irq_status);
			}

			//Signal user.
			ret = 1;
		}
		else if (irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
		{
			instance->ISM.read += instance->fifo_half_size;
			instance->ISM.global_read += instance->fifo_half_size;

			if (ai_read_data(instance, instance->fifo_half_size) != instance->fifo_half_size)
			{//ERROR!
				PERROR("Limited amounts aqusition with TH != 0: Circular buffer full!\n");
				ai_stop_isr(instance);
				instance->status = ai_status_stream_buffer_error;
				instance->stream_stop_count++;
				//Signal user.
				ret = 1;
			}
			else
			{
				//Countinue.
				ai_limited_ISM(instance, irq_status);
			}
		}


		if(instance->ISM.global_read >= instance->data_required)
		{//End of work. Next paranoid pice of code: '>=' instead od '==' only to be sure.
			ai_stop_isr(instance);
			if(instance->status < ai_status_stream_end)
			{
				instance->status = ai_status_stream_end;
				instance->stream_stop_count++;
			}
#ifdef MEDEBUG_ERROR
			if(instance->ISM.global_read > instance->data_required)
			{//This is security check case. This should never ever happend!
				PERROR("Limited amounts aqusition: Read more data than necessary! data_required=%d < read=%d\n",
					instance->data_required, instance->ISM.global_read);
			}
#endif
			//Signal user.
			ret = 1;
		}
	}

	return ret;
}


int ai_infinite_isr(me4600_ai_subdevice_t* instance, const uint32_t irq_status, const uint32_t ctrl_status)
{
	int to_read;
	int ret = 0;

	PDEBUG("executed. idx=0\n");

	if (irq_status & ME4600_IRQ_STATUS_BIT_SC)
	{//next chunck of data -> read fifo
		/// @note When SC is set bellow but close to 0.5 * FIFO size than ME4600_AI_STATUS_BIT_HF_DATA can be set or not. This depend on aqusation and interrupt handling speed.
		//Set new state in ISM.
		if((instance->fifo_irq_threshold < instance->fifo_half_size) && (!(ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)))
		{//There is more data than we excpected. Propably we aren't fast enough. Read as many as possible.
			if(instance->fifo_irq_threshold)
			{
				to_read = instance->fifo_half_size - (instance->fifo_half_size % instance->fifo_irq_threshold);
				if(to_read > instance->fifo_irq_threshold)
				{
					PDEBUG("Infinite aqusition: Not fast enough data aqusition! TH != 0: correction=%d\n", to_read);
				}
			}
			else
			{//No threshold specified.
				to_read = instance->fifo_half_size;
			}
		}
		else
		{
			to_read = instance->ISM.next;
		}

		instance->ISM.read += to_read;

		//Get data
		if (ai_read_data(instance, to_read) != to_read)
		{//ERROR!
			PERROR("Infinite aqusition: Circular buffer full!\n");
			ai_stop_isr(instance);
			instance->status = ai_status_stream_buffer_error;
			instance->stream_stop_count++;
		}
		else
		{
			ai_infinite_ISM(instance);
			instance->ISM.global_read += instance->ISM.read;
			instance->ISM.read = 0;
		}

		//Signal data to user
		ret = 1;
	}
	else if (irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
	{//fifo is half full -> read fifo	Large blocks only!
		instance->ISM.read += instance->fifo_half_size;

		if (ai_read_data(instance, instance->fifo_half_size) != instance->fifo_half_size)
		{//ERROR!
			PERROR("Infinite aqusition: Circular buffer full!\n");
			ai_stop_isr(instance);
			instance->status = ai_status_stream_buffer_error;
			instance->stream_stop_count++;

			//Signal it.
			ret = 1;
		}
		else
		{
			ai_infinite_ISM(instance);
		}
	}

	return ret;
}

static int me4600_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{/// @note This is time critical function!
	uint32_t ctrl_status;
	uint16_t tmp;
	me4600_ai_subdevice_t* instance = (me4600_ai_subdevice_t *)subdevice;
	int err = ME_ERRNO_SUCCESS;
	int signal_irq = 0;

#ifdef MEDEBUG_SPEED_TEST
	uint64_t execuction_time;

	rdtscll(instance->int_end);
#endif

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed. idx=0\n");

		if (!(irq_status & (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC | ME4600_IRQ_STATUS_BIT_AI_OF)))
		{
			//This is security check case. This should never ever happend because of 'LE' is unused.
			if ((irq_status & (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC | ME4600_IRQ_STATUS_BIT_AI_OF | ME4600_IRQ_STATUS_BIT_LE)) == ME4600_IRQ_STATUS_BIT_LE)
			{
				me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
				me_writel(instance->base.dev, ctrl_status | ME4600_AI_CTRL_BIT_LE_IRQ_RESET, instance->ctrl_reg);
				PERROR_CRITICAL("%ld Shared interrupt. %s(): irq_status_reg=LE_IRQ\n", jiffies, __FUNCTION__);
			}
			else
			{
				PINFO("%ld Shared interrupt. %s(): irq_status_reg=0x%04X\n", jiffies, __FUNCTION__, irq_status);
				err = ME_ERRNO_INTERNAL;
			}
			goto ERROR;
		}

		if (!instance->seg_buf)
		{//Security check.
			PERROR_CRITICAL("SEGMENTED BUFFER DOESN'T EXISTS!\n");
			ai_stop_isr(instance);
			signal_irq = 1;
			instance->status = ai_status_stream_buffer_error;
			goto ERROR;
		}

		//Get the status register.
		me_readl(instance->base.dev, &ctrl_status, instance->status_reg);

#ifdef MEDEBUG_INFO
		if (irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
				PINFO("HF interrupt active\n");
		if (irq_status & ME4600_IRQ_STATUS_BIT_SC)
				PINFO("SC interrupt active\n");
		if (irq_status & ME4600_IRQ_STATUS_BIT_LE)
				PINFO("LE interrupt active\n");
		if (irq_status & ME4600_IRQ_STATUS_BIT_AI_OF)
				PINFO("OF interrupt active\n");
#endif

		//This is safety check!
		if ((irq_status & ME4600_IRQ_STATUS_BIT_AI_HF) && (ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA))
		{
			PDEBUG("HF interrupt active but FIFO under half\n");
			//Reset HF interrupt latch.
			me_writel(instance->base.dev, ctrl_status | ME4600_AI_CTRL_BIT_HF_IRQ_RESET,   instance->ctrl_reg);
			me_writel(instance->base.dev, ctrl_status,   instance->ctrl_reg);
			goto END;
		}

#ifdef MEDEBUG_INFO
		PINFO("%s IN:\n", __FUNCTION__);
# ifdef MEDEBUG_EXTENDED_INFO
		PINFO("mode: %d.\n", ctrl_status & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2) );
		PINFO("CTRL_BIT_SAMPLE_HOLD=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SAMPLE_HOLD)?"on":"off");
		PINFO("CTRL_BIT_IMMEDIATE_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_IMMEDIATE_STOP)?"on":"off");
		PINFO("CTRL_BIT_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_STOP)?"on":"off");
		PINFO("CTRL_BIT_CHANNEL_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_CHANNEL_FIFO)?"enable":"disable");
		PINFO("CTRL_BIT_DATA_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_DATA_FIFO)?"enable":"disable");
		PINFO("CTRL_BIT_FULLSCALE=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_FULLSCALE)?"on":"off");
		PINFO("CTRL_BIT_OFFSET=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_OFFSET)?"on":"off");
# endif
		PINFO("STATUS_BIT_FSM: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FSM)?"on":"off");

		PINFO("STATUS_BIT_EF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_CHANNEL)?"not empty":"empty");
		PINFO("STATUS_BIT_HF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_CHANNEL)?" <= HF":" > HF");
		PINFO("STATUS_BIT_FF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_CHANNEL)?"not full":"full");

		PINFO("STATUS_BIT_EF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_DATA)?"not empty":"empty");
		PINFO("STATUS_BIT_HF_DATA:%s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)?" <= HF":" > HF");
		PINFO("STATUS_BIT_FF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_DATA)?"not full":"full");

		PINFO("CTRL_BIT_HF_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ)?"enable":"disable");
		PINFO("CTRL_BIT_HF_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ_RESET)?"reset":"work");
		PINFO("CTRL_BIT_SC_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ)?"enable":"disable");
		PINFO("CTRL_BIT_SC_RELOAD: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_RELOAD)?"on":"off");
		PINFO("CTRL_BIT_SC_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ_RESET)?"reset":"work");
#endif

		if(!instance->data_required)
		{//This is infinite aqusition.
#ifdef MEDEBUG_ERROR
			if ((irq_status & (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC)) == (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC))
			{
				///In infinite mode only one interrupt source should be reported!
				PERROR("Error in ISM! Infinite aqusition: HF and SC interrupts active! threshold=%d next=%d tmp=0x%04X irq_status_reg=0x%04X",
					instance->fifo_irq_threshold, instance->ISM.next, ctrl_status, irq_status);
			}
#endif

			signal_irq = ai_infinite_isr(instance, irq_status, ctrl_status);
		}
		else
		{

			signal_irq = ai_limited_isr(instance, irq_status, ctrl_status);
			me_readl(instance->base.dev, &ctrl_status, instance->status_reg);
			if(!(ctrl_status & (ME4600_AI_STATUS_BIT_HF_DATA | ME4600_AI_CTRL_BIT_HF_IRQ_RESET)))
			{//HF active, but we have more than half already => HF will never come
				PDEBUG("MISSED HF. data_required=%d ISM.read=%d ISM.global=%d ISM.next=%d\n", instance->data_required, instance->ISM.read, instance->ISM.global_read, instance->ISM.next);
				signal_irq = ai_limited_isr(instance, ME4600_IRQ_STATUS_BIT_AI_HF, ctrl_status);
			}
		}


		//Look for overflow error.
		if (irq_status & ME4600_IRQ_STATUS_BIT_AI_OF)
		{
			uint32_t tmp_status;
			PERROR("FIFO overflow reported.\n");

			me_readl(instance->base.dev, &tmp_status, instance->irq_status_reg);
			if (tmp_status & ME4600_IRQ_STATUS_BIT_AI_OF)
			{
				int needed_values;
				instance->stream_stop_count++;
				//Signal it.
				signal_irq = 1;
				//FIFO is overfull. Read datas and reset all settings.
				if(!instance->data_required)
				{//This is infinite aqusition.
					needed_values = instance->fifo_size;
				}
				else
				{
					if(instance->ISM.global_read >= instance->data_required)
					{//End of work. Next paranoid pice of code: '>=' instead od '==' only to be sure.
						needed_values = 0;
					}
					else
					{
						needed_values = instance->data_required - instance->ISM.global_read;
					}
				}

				if (needed_values)
				{
					if (!ai_read_data(instance, needed_values))
					{
						me_readw(instance->base.dev, &tmp, instance->data_reg);
					}
				}
			}
			ai_stop_isr(instance);
			goto END;
		}
END:
#ifdef MEDEBUG_INFO
		me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
		PINFO("%s OUT:\n", __FUNCTION__);
# ifdef MEDEBUG_EXTENDED_INFO
		PINFO("mode: %d.\n", ctrl_status & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2) );
		PINFO("CTRL_BIT_SAMPLE_HOLD=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SAMPLE_HOLD)?"on":"off");
		PINFO("CTRL_BIT_IMMEDIATE_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_IMMEDIATE_STOP)?"on":"off");
		PINFO("CTRL_BIT_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_STOP)?"on":"off");
		PINFO("CTRL_BIT_CHANNEL_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_CHANNEL_FIFO)?"enable":"disable");
		PINFO("CTRL_BIT_DATA_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_DATA_FIFO)?"enable":"disable");
		PINFO("CTRL_BIT_FULLSCALE=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_FULLSCALE)?"on":"off");
		PINFO("CTRL_BIT_OFFSET=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_OFFSET)?"on":"off");
# endif
		PINFO("STATUS_BIT_FSM: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FSM)?"on":"off");

		PINFO("STATUS_BIT_EF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_CHANNEL)?"not empty":"empty");
		PINFO("STATUS_BIT_HF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_CHANNEL)?" <= HF":" > HF");
		PINFO("STATUS_BIT_FF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_CHANNEL)?"not full":"full");

		PINFO("STATUS_BIT_EF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_DATA)?"not empty":"empty");
		PINFO("STATUS_BIT_HF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)?" <= HF":" > HF");
		PINFO("STATUS_BIT_FF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_DATA)?"not full":"full");

		PINFO("CTRL_BIT_HF_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ_RESET)?"reset":"work");
		PINFO("CTRL_BIT_SC_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ)?"enable":"disable");
		PINFO("CTRL_BIT_SC_RELOAD: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_RELOAD)?"on":"off");
		PINFO("CTRL_BIT_SC_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ_RESET)?"reset":"work");
#endif

ERROR:
	ME_FREE_HANDLER_PROTECTOR;

	if (signal_irq)
	{
		//Signal it.
		wake_up_interruptible_all(&instance->wait_queue);
	}

#ifdef MEDEBUG_SPEED_TEST
	rdtscll(execuction_time);
	PSPEED("me4600 AI:  %lld %lld %lld\n", instance->int_end, instance->int_end - instance->int_start, execuction_time - instance->int_end);
	instance->int_start = instance->int_end;
#endif
	return err;
}

/** @brief Stop aqusation of data. Reset interrupts' laches. Clear data's FIFO.
*
* @param instance The subdevice instance (pointer).
*/
void inline ai_stop_isr(me4600_ai_subdevice_t* instance)
{/// @note This is soft time critical function!
	uint32_t tmp;

	//Stop all. Reset interrupt laches. Reset data FIFO.
	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	tmp |= (ME4600_AI_CTRL_BIT_IMMEDIATE_STOP | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_LE_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
	tmp &= ~ME4600_AI_CTRL_BIT_DATA_FIFO;
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);
}

/** @brief Copy data from fifo to circular buffer.
*
* @param instance The subdevice instance (pointer).
* @param count The number of requested data.
*
* @return On success: Number of copied values.
* @return On error: -ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW.
*/
static int inline ai_read_data(me4600_ai_subdevice_t* instance, const int count)
#if defined(ME_USB) && defined(ME_USE_DMA)
{/// @note This is time critical function!
	int local_count;
	int empty_space;
	int i;
	uint32_t* buffer;
	uint32_t ctrl;

	PDEBUG("executed. idx=0\n");
	PINFO("FAST: REQUESTED %d values.\n", count);

	if (count <= 0)
	{
		return 0;
	}

	empty_space = me_seg_buf_space(instance->seg_buf);
	if (empty_space <= 0)
	{
		PERROR("Circular buffer full.\n");
		return -ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
	}

	local_count = (count < empty_space) ? count : empty_space;

	buffer = (uint32_t *)kzalloc(local_count * sizeof(uint32_t), GFP_KERNEL);
	if (!buffer)
		return -ME_ERRNO_INTERNAL;

	// Block registry - Enable DMA
	if (!me_DMA_lock(instance->base.dev, instance->ctrl_reg, &ctrl))
	{
		me_DMA_read(instance->base.dev, buffer, local_count, instance->DMA_base);
	}
	else
	{
		local_count = 0;
	}
	// Unblock registry - Disable DMA
	me_DMA_unlock(instance->base.dev, instance->ctrl_reg, ctrl);

	for (i = 0; i < local_count; i++)
	{
		if (instance->chan_list_copy)
		{
			(void)me_seg_buf_put(instance->seg_buf, ai_calculate_calibrated_value(instance, instance->chan_list_copy[instance->chan_list_copy_pos], tmp));
		}
		else
		{
			(void)me_seg_buf_put(instance->seg_buf, tmp ^ 0x8000);
		}
		instance->chan_list_copy_pos++;
		instance->chan_list_copy_pos %= instance->chan_list_len;
	}

	PINFO("FAST: DOWNLOADED %d values\n", local_count);
	kfree(buffer);
	return local_count;
}
#else
{/// @note This is time critical function!
	int local_count;
	int empty_space;
	int copied = 0;
	uint16_t tmp;

	PDEBUG("executed. idx=0\n");
	PINFO("FAST: REQUESTED %d values.\n", count);

	if (count <= 0)
	{
		return 0;
	}

	empty_space = me_seg_buf_space(instance->seg_buf);
	if (empty_space == 0)
	{
		PDEBUG("Segmented buffer full.\n");
		return -ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
	}
	local_count = (count < empty_space) ? count : empty_space;

	for (copied = 0; copied < local_count; ++copied)
	{
		me_readw(instance->base.dev, &tmp, instance->data_reg);
		if (instance->chan_list_copy)
		{
			(void)me_seg_buf_put(instance->seg_buf, ai_calculate_calibrated_value(instance, instance->chan_list_copy[instance->chan_list_copy_pos], tmp));
		}
		else
		{
			(void)me_seg_buf_put(instance->seg_buf, tmp ^ 0x8000);
		}
		instance->chan_list_copy_pos++;
		instance->chan_list_copy_pos %= instance->chan_list_len;
	}

	PINFO("FAST: DOWNLOADED %d values.\n", copied);
	return copied;
}
#endif

void inline ai_infinite_ISM(me4600_ai_subdevice_t* instance)
{/// @note This is time critical function!
	uint32_t ctrl_set, ctrl_reset, tmp;

	PDEBUG("executed. idx=0\n");

	if (instance->fifo_irq_threshold < instance->fifo_max_sc)
	{// Only sample counter with reloadnig is working. Reset it.
		PINFO("Only sample counter with reloadnig is working. Reset it.\n");
		ctrl_set   =  ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
		ctrl_reset = ~ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
	}
	else if (instance->fifo_irq_threshold == instance->ISM.read)
	{//This is SC interrupt for large block. The whole section is done. Reset SC_IRQ an HF_IRQ and start everything again from beginning.
		PINFO("This is SC interrupt for large block. The whole section is done. Reset SC_IRQ an HF_IRQ and start everything again from beginning.\n");
		ctrl_set   =  ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
		ctrl_reset = ~(ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET);
	}
	else if (instance->fifo_irq_threshold >= (instance->fifo_max_sc + instance->ISM.read))
	{//This is HF interrupt for large block.The next interrupt should be from HF, also. Reset HF.
		PINFO("This is HF interrupt for large block.The next interrupt should be from HF, also. Reset HF.\n");
		ctrl_set   =  ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
		ctrl_reset = ~ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
	}
	else
	{//This is HF interrupt for large block.The next interrupt should be from SC. Don't reset HF!
		PINFO("This is HF interrupt for large block.The next interrupt should be from SC. Don't reset HF!\n");
		ctrl_set   =  ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
		ctrl_reset = 0xFFFFFFFF;
	}

	//Reset interrupt latch.
	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	PINFO("tmp=0x%x ctrl_set=0x%x ctrl_reset=0x%x\n", tmp, ctrl_set, ctrl_reset);
	tmp |= ctrl_set;
	me_writel(instance->base.dev, tmp,   instance->ctrl_reg);
	if(ctrl_reset != 0xFFFFFFFF)
	{
		me_writel(instance->base.dev, tmp & ctrl_reset, instance->ctrl_reg);
	}
}

void inline ai_limited_ISM(me4600_ai_subdevice_t* instance, uint32_t irq_status)
{/// @note This is time critical function!
	uint32_t ctrl_set, ctrl_reset= 0xFFFFFFFF, tmp;

	if (!instance->fifo_irq_threshold)
	{//No threshold provided. SC ends work.
 		PINFO("No threshold provided. SC ends work.\n");
		ctrl_set = ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
		if (instance->data_required > (instance->fifo_size - 1 + instance->ISM.global_read))
		{//HF need reseting.
 			ctrl_reset &= ~ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
		}
	}
	else	//if(instance->fifo_irq_threshold)
	{
		if(irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
		{
 			PINFO("Threshold provided. Clear HF latch.\n");
			ctrl_set = ME4600_AI_CTRL_BIT_HF_IRQ_RESET;

			if (instance->fifo_irq_threshold >= (instance->fifo_max_sc + instance->ISM.read))
			{//This is not the last one. HF need reseting.
		 		PINFO("The next interrupt is HF. HF need be activating.\n");
				ctrl_reset = ~ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
			}
		}

		if(irq_status & ME4600_IRQ_STATUS_BIT_SC)
		{
 			PINFO("Threshold provided. Restart SC.\n");
			ctrl_set    =  ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
			ctrl_reset &= ~ME4600_AI_CTRL_BIT_SC_IRQ_RESET;

			if (instance->fifo_irq_threshold >= instance->fifo_max_sc)
			{//This is not the last one. HF need to be activating.
 				PINFO("The next interrupt is HF. HF need to be activating.\n");
				ctrl_reset &= ~ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
			}
		}
	}

	//Reset interrupt latch.
	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	tmp |= ctrl_set;
	me_writel(instance->base.dev, tmp,   instance->ctrl_reg);
	if(ctrl_reset != 0xFFFFFFFF)
	{
		me_writel(instance->base.dev, tmp & ctrl_reset, instance->ctrl_reg);
	}
}

/** @brief Last chunck of datas. We must reschedule sample counter.
*	@note Last chunck.
*	Leaving SC_RELOAD doesn't do any harm, but in some bad case can make extra interrupts.
*	@warning When threshold is wrongly set some IRQ are lost.(!!!)
*/
void inline ai_reschedule_SC(me4600_ai_subdevice_t* instance)
{
	uint32_t rest;

	if(instance->data_required <= instance->ISM.global_read)
		return;

	rest = instance->data_required - instance->ISM.global_read;
	if(rest < instance->fifo_irq_threshold)
	{//End of work soon ....
		PDEBUG("Rescheduling SC from %d to %d.\n", instance->fifo_irq_threshold, rest);
		/// @note Write new value to SC <==  DANGER! This is not safe solution! We can miss some inputs.
		me_writel(instance->base.dev, rest, instance->sample_counter_reg);
		instance->fifo_irq_threshold = rest;

		if (rest < instance->fifo_max_sc)
		{
			instance->ISM.next = rest;
		}
		else
		{
			instance->ISM.next = rest % instance->fifo_half_size;
			if(instance->ISM.next + instance->fifo_half_size < instance->fifo_max_sc)
			{
				instance->ISM.next += instance->fifo_half_size;
			}
		}
	}
}

/** Start the ISM. All must be reseted before enter to this function. */
void inline ai_data_acquisition_logic(me4600_ai_subdevice_t* instance)
{
	uint32_t tmp;

	if(!instance->data_required)
	{//This is infinite aqusition.
		if (!instance->fifo_irq_threshold)
		{//No threshold provided. Set SC to 0.5*FIFO. Clear the SC's latch.
			//Set the sample counter
			me_writel(instance->base.dev, instance->fifo_half_size, instance->sample_counter_reg);
		}
		else
		{//Threshold provided. Set SC to treshold. Clear the SC's latch.
			//Set the sample counter
			me_writel(instance->base.dev, instance->fifo_irq_threshold, instance->sample_counter_reg);
		}

		if (instance->fifo_irq_threshold < instance->fifo_max_sc)
		{//Enable only sample counter's interrupt. Set reload bit. Clear the SC's latch.
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp |= ME4600_AI_CTRL_BIT_SC_RELOAD;
			tmp &= ~ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			if (!instance->fifo_irq_threshold)
			{//No threshold provided. Set ISM.next to 0.5*FIFO.
				instance->ISM.next = instance->fifo_half_size;
			}
			else
			{//Threshold provided. Set ISM.next to treshold.
				instance->ISM.next = instance->fifo_irq_threshold;
			}
		}
		else
		{//Enable sample counter's and HF's interrupts.
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp |= ME4600_AI_CTRL_BIT_SC_RELOAD;
			tmp &= ~(ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET);
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			instance->ISM.next = instance->fifo_irq_threshold % instance->fifo_half_size;
			if(instance->ISM.next + instance->fifo_half_size < instance->fifo_max_sc)
			{
				instance->ISM.next += instance->fifo_half_size;
			}
		}
	}
	else
	{//This aqusition is limited to set number of data.
		if (instance->fifo_irq_threshold >= instance->data_required)
		{//Stupid situation.
			instance->fifo_irq_threshold = 0;
			PDEBUG("Stupid situation: data_required(%d) < threshold(%d).\n", instance->fifo_irq_threshold, instance->data_required);
		}

		if (!instance->fifo_irq_threshold)
		{//No threshold provided. Easy case: HF=read and SC=end.
			//Set the sample counter to data_required.
			me_writel(instance->base.dev, instance->data_required, instance->sample_counter_reg);
			//Reset the latches of sample counter and HF (if SC>FIFO).
			//No SC reload!
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp &= ~(ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_RELOAD);
			if (instance->data_required > (instance->fifo_size - 1))
			{
				tmp &= ~ME4600_AI_CTRL_BIT_HF_IRQ_RESET;
				instance->ISM.next = instance->data_required % instance->fifo_half_size;
				instance->ISM.next += instance->fifo_half_size;

			}
			else
			{
				instance->ISM.next = instance->data_required;
			}
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
		}
		else
		{//The most general case. We have concret number of the required data and threshold. SC=TH
			//Set the sample counter to threshold.
			me_writel(instance->base.dev, instance->fifo_irq_threshold, instance->sample_counter_reg);
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			//In this moment we are sure that SC will come more than once.
			tmp |= ME4600_AI_CTRL_BIT_SC_RELOAD;

			if (instance->fifo_irq_threshold < instance->fifo_max_sc)
			{//The threshold is so small that we don't need HF.
				tmp &= ~ME4600_AI_CTRL_BIT_SC_IRQ_RESET;
				instance->ISM.next = instance->fifo_irq_threshold;
			}
			else
			{//The threshold is large. The HF must be use.
				tmp &= ~(ME4600_AI_CTRL_BIT_SC_IRQ_RESET | ME4600_AI_CTRL_BIT_HF_IRQ_RESET);
				instance->ISM.next = instance->fifo_irq_threshold % instance->fifo_half_size;
				if(instance->ISM.next + instance->fifo_half_size < instance->fifo_max_sc)
				{
					instance->ISM.next += instance->fifo_half_size;
				}
			}
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
		}
	}
}

static void ai_mux_toggler(me4600_ai_subdevice_t* instance)
{
	uint32_t ctrl;
	uint32_t tmp;

	ME_LOCK_PROTECTOR;
		PDEBUG("executed. idx=0\n");
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_high_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);
		// Clear all features. Dissable interrupts.
		ctrl = ( 0x00
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Turn on internal reference.
		ctrl |= ME4600_AI_CTRL_BIT_FULLSCALE;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Clear data and channel fifo.
		ctrl &= ~(ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO);
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Write channel entry.
		me_writel(instance->base.dev, ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_UNIPOLAR_2_5 | 31, instance->channel_list_reg);
		ctrl &= ~ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Start conversion.
		me_readl(instance->base.dev, &tmp, instance->start_reg);
		udelay(10);

		// Clear data and channel fifo.
		ctrl &= ~(ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO);
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Write channel entry.
		// ME4600_AI_LIST_INPUT_SINGLE_ENDED | ME4600_AI_LIST_RANGE_BIPOLAR_10 <= 0x0000
		me_writel(instance->base.dev, ME4600_AI_LIST_INPUT_SINGLE_ENDED | ME4600_AI_LIST_RANGE_BIPOLAR_10, instance->channel_list_reg);
		// Start conversion.
		me_readl(instance->base.dev, &tmp, instance->start_reg);
		udelay(10);

		// Clear control register.
		// Turn off internal reference (reset ME4600_AI_CTRL_BIT_FULLSCALE !).
		ctrl = ( 0x00
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		me_writel(instance->base.dev, tmp, instance->ctrl_reg);
	ME_UNLOCK_PROTECTOR;
}

/** @brief Copy rest of data from fifo to circular buffer.
* @note Helper for STOP command. After FSM is stopped.
* @note This is slow function that copy all remainig data from FIFO to buffer.
*
* @param instance The subdevice instance (pointer).
*
* @return On success: Number of copied values.
* @return On error: Negative error code -ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW.
*/
static int inline ai_read_data_pooling(me4600_ai_subdevice_t* instance)
{/// @note This is time critical function!
	int copied = 0;
	uint16_t tmp;
	uint32_t status = 0;

	PDEBUG("executed. idx=0\n");
	PDEBUG("Space left in segmented buffer = %d.\n", me_seg_buf_space(instance->seg_buf));

	while(me_seg_buf_space(instance->seg_buf))
	{
		me_readl(instance->base.dev, &status, instance->status_reg);
		status &= ME4600_AI_STATUS_BIT_EF_DATA;
		if(!status)
		{//No more data. status = ME_ERRNO_SUCCESS = 0
			break;
		}
		me_readw(instance->base.dev, &tmp, instance->data_reg);
		if (instance->chan_list_copy)
		{
			me_seg_buf_put(instance->seg_buf, ai_calculate_calibrated_value(instance, instance->chan_list_copy[instance->chan_list_copy_pos], tmp));
		}
		else
		{
			me_seg_buf_put(instance->seg_buf,  tmp ^ 0x8000);
		}
		instance->chan_list_copy_pos++;
		instance->chan_list_copy_pos %= instance->chan_list_len;
		++copied;
	}

#ifdef MEDEBUG_ERROR
	if(!status)
		PINFO("SLOW: DOWNLOADED %d values\n", copied);
	else
	{
		PDEBUG("No more empty space in buffer.\n");
		PDEBUG("SLOW: DOWNLOADED %d values\n", copied);
		PDEBUG("FIFO still not empty.\n");
	}
#endif
	return (!status) ? copied : -ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
}

static void me4600_ai_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void* subdevice
#else
											struct work_struct* work
#endif
										)
{
	me4600_ai_subdevice_t* instance;
	uint32_t status;
	uint32_t tmp;
	int reschedule = 0;
	int signaling = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	instance = (me4600_ai_subdevice_t *) subdevice;
#else
	instance = container_of((void *)work, me4600_ai_subdevice_t, ai_control_task);
#endif

	if (!instance)
	{
		return;
	}

	if (signal_pending(current))
	{
		PERROR("Control task interrupted.\n");
		instance->status = ai_status_none;
		return;
	}

	if (!atomic_read(&instance->ai_control_task_flag))
	{
		return;
	}

#if defined(ME_USB)
	if (instance && instance->base.dev)
	{
		ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
	}

	if (!atomic_read(&instance->ai_control_task_flag))
	{
		goto EXIT_USB;
	}
#endif

	PINFO("<%s: %ld> executed. idx=0\n", __FUNCTION__, jiffies);
	ME_LOCK_PROTECTOR;
		if (!atomic_read(&instance->ai_control_task_flag))
		{
			goto EXIT;
		}
		if (instance->status != ai_status_stream_run)
		{
			instance->me4600_ai_error_confirm = 0;
		}

		me_readl(instance->base.dev, &status, instance->status_reg);

		switch (instance->status)
		{// Checking actual mode.
			// Not configured for work.
			case ai_status_none:
				instance->me4600_ai_error_confirm = 0;
				break;

			//This are stable modes. No need to do anything. (?)
			case ai_status_single_configured:
			case ai_status_single_end:
			case ai_status_stream_configured:
			case ai_status_stream_fifo_error:
			case ai_status_stream_buffer_error:
			case ai_status_stream_timeout:
			case ai_status_error:
				break;

			// Stream modes
			case ai_status_stream_run_wait:
				if (status & ME4600_AI_STATUS_BIT_FSM)
				{// ISM started..
					instance->status = ai_status_stream_run;
					// Signal the end of wait for start.
					signaling = 1;
					// Wait now for stop.
					reschedule = 1;
					instance->stream_start_count++;
					break;
				}

				// Check timeout.
				if ((instance->timeout.delay) && ((jiffies - instance->timeout.start_time) >= instance->timeout.delay))
				{// Timeout
					PDEBUG("Timeout reached.\n");
					// Stop all actions. No conditions! Block interrupts. Reset FIFO => Too late!
					ai_stop_isr(instance);

					instance->status = ai_status_stream_timeout;
					instance->stream_start_count++;
					instance->stream_stop_count++;

					// Signal the end.
					signaling = 1;
					break;
				}

				reschedule = 1;
				break;

			case ai_status_stream_run:
				if (!(status & ME4600_AI_STATUS_BIT_FSM))
				{// ISM stoped. Overwrite ISR.
					instance->me4600_ai_error_confirm++;
					if (instance->me4600_ai_error_confirm > me4600_AI_ERROR_TIMEOUT)
					{
						instance->status = ai_status_stream_fifo_error;
						instance->stream_stop_count++;
						// Signal the end of wait for stop.
						signaling = 1;
						break;
					}
					instance->me4600_ai_error_confirm = 0;
				}

				reschedule = 1;
				break;

			case ai_status_stream_end_wait:
				if (!(status & ME4600_AI_STATUS_BIT_FSM))
				{// ISM stoped. Overwrite ISR.
					instance->status = ai_status_stream_end;
					instance->stream_stop_count++;
					// Signal the end of wait for stop.
					signaling = 1;
				}

				reschedule = 1;
				break;

			case ai_status_stream_end:
				//End work.
				if (status & ME4600_AI_STATUS_BIT_FSM)
				{// Still working? Stop it!
					PERROR("Status is 'ai_status_stream_end' but hardware is still working!\n");
					me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
					tmp |= (ME4600_AI_CTRL_BIT_IMMEDIATE_STOP | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);
				}
				break;

			default:
				PERROR_CRITICAL("WRONG STATUS!\n");
				instance->status = ai_status_none;
		}
EXIT:
	ME_UNLOCK_PROTECTOR;

#if defined(ME_USB)
EXIT_USB:
	if (instance && instance->base.dev)
	{
		ME_IRQ_UNLOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
	}
#endif

	if (signal_pending(current))
	{
		PERROR("Control task interrupted. Ending.\n");
		instance->status = ai_status_none;
		return;
	}

	if (atomic_read(&instance->ai_control_task_flag) && reschedule)
	{// Reschedule task
		queue_delayed_work(instance->me4600_workqueue, &instance->ai_control_task, 1);
	}
	else
	{
		PINFO("<%s> Ending control task. idx=0\n", __FUNCTION__);
	}

	if (signaling)
	{//Signal it.
		wake_up_interruptible_all(&instance->wait_queue);
	}
}

int me4600_ai_postinit(me_subdevice_t* subdevice, void* args)
{
	PDEBUG("executed. idx=0\n");

	// We have to switch the mux in order to get it work correctly.
	ai_mux_toggler((me4600_ai_subdevice_t *)subdevice);

	ai_determine_FIFO_size((me4600_ai_subdevice_t *)subdevice);
	ai_determine_LE_size((me4600_ai_subdevice_t *)subdevice);

	ai_read_calibration((me4600_ai_subdevice_t *)subdevice);

	// Reset subdevice.
	return me4600_ai_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
}

me4600_ai_subdevice_t* me4600_ai_constr(void* reg_base,
											void* DMA_base,
											void* PLX_base,
											unsigned int idx,
											unsigned int channels, unsigned int ranges,
											int features,
											/* me4600_interrupt_status_t* int_status, */
											struct workqueue_struct* me4600_wq)
{
	me4600_ai_subdevice_t* subdevice;
	unsigned int i;

	PDEBUG("executed. idx=0\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4600_ai_subdevice_t), GFP_KERNEL);
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

	// Store analog input index.
	subdevice->base.idx = idx;

	// Initialize segmented buffer.
	subdevice->seg_buf = create_seg_buffer(ME4600_AI_SEG_BUF_CHUNK_COUNT, ME4600_AI_SEG_BUF_CHUNK_SIZE);
	if (!subdevice->seg_buf)
	{
		PERROR("Cannot initialize segmented buffer.\n");
		me_subdevice_deinit((me_subdevice_t *) subdevice);
		kfree(subdevice);
		return NULL;
	}

	subdevice->status = ai_status_none;
	subdevice->stream_start_count = 0;
	subdevice->stream_stop_count = 0;
	subdevice->empty_read_count = 0;

	atomic_set(&subdevice->ai_control_task_flag, 0);

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Save the number of channels.
	subdevice->channels = channels;

	// Initialize the single config entries to reset values
	for (i = 0; i < channels; i++)
	{
		subdevice->single_config[i].status = ME_SINGLE_CHANNEL_NOT_CONFIGURED;	//not configured
	}

	// Save sub-device features: "sample and hold", "isolated" and "analog trigger".
	subdevice->features = features;

	// Set stream config to not configured state.
	subdevice->fifo_irq_threshold = 0;
	subdevice->data_required = 0;
	subdevice->chan_list_len = 0;

	subdevice->chan_list_copy = NULL;
	subdevice->chan_list_copy_pos = 0;

// 	subdevice->int_status = int_status;
	subdevice->raw_values = 0;

	// Initialize registers addresses.
	subdevice->ctrl_reg = reg_base + ME4600_AI_CTRL_REG;
	subdevice->status_reg = reg_base + ME4600_AI_STATUS_REG;
	subdevice->channel_list_reg = reg_base + ME4600_AI_CHANNEL_LIST_REG;
	subdevice->data_reg = reg_base + ME4600_AI_DATA_REG;
	subdevice->chan_timer_reg = reg_base + ME4600_AI_CHAN_TIMER_REG;
	subdevice->chan_pre_timer_reg = reg_base + ME4600_AI_CHAN_PRE_TIMER_REG;
	subdevice->scan_timer_low_reg = reg_base + ME4600_AI_SCAN_TIMER_LOW_REG;
	subdevice->scan_timer_high_reg = reg_base + ME4600_AI_SCAN_TIMER_HIGH_REG;
	subdevice->scan_pre_timer_low_reg = reg_base + ME4600_AI_SCAN_PRE_TIMER_LOW_REG;
	subdevice->scan_pre_timer_high_reg = reg_base + ME4600_AI_SCAN_PRE_TIMER_HIGH_REG;
	subdevice->start_reg = reg_base + ME4600_AI_START_REG;
	subdevice->sample_counter_reg = reg_base + ME4600_AI_SAMPLE_COUNTER_REG;

	subdevice->DMA_base = DMA_base;
	subdevice->PLX_base = PLX_base;

	subdevice->irq_status_reg = reg_base + ME4600_IRQ_STATUS_REG;

	// Initialize ranges.
	subdevice->ranges_len = ranges;
	subdevice->ranges[0].min = -10E6;
	subdevice->ranges[0].max = 9999695;

	subdevice->ranges[1].min = 0;
	subdevice->ranges[1].max = 9999847;

	subdevice->ranges[2].min = -25E5;
	subdevice->ranges[2].max = 2499924;

	subdevice->ranges[3].min = 0;
	subdevice->ranges[3].max = 2499962;

	// Override base class methods.
	subdevice->base.me_subdevice_destructor = me4600_ai_destructor;
	subdevice->base.me_subdevice_io_reset_subdevice = me4600_ai_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me4600_ai_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me4600_ai_io_single_read;
	subdevice->base.me_subdevice_io_stream_config = me4600_ai_io_stream_config;
	subdevice->base.me_subdevice_io_stream_new_values = me4600_ai_io_stream_new_values;
	subdevice->base.me_subdevice_io_stream_read = me4600_ai_io_stream_read;
	subdevice->base.me_subdevice_io_stream_start = me4600_ai_io_stream_start;
	subdevice->base.me_subdevice_io_stream_status = me4600_ai_io_stream_status;
	subdevice->base.me_subdevice_io_stream_stop = me4600_ai_io_stream_stop;
	subdevice->base.me_subdevice_query_number_channels = me4600_ai_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4600_ai_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me4600_ai_query_subdevice_caps;
	subdevice->base.me_subdevice_query_subdevice_caps_args = me4600_ai_query_subdevice_caps_args;
	subdevice->base.me_subdevice_query_range_by_min_max = me4600_ai_query_range_by_min_max;
	subdevice->base.me_subdevice_query_number_ranges = me4600_ai_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = me4600_ai_query_range_info;
	subdevice->base.me_subdevice_query_timer = me4600_ai_query_timer;

	subdevice->base.me_subdevice_irq_handle = me4600_ai_irq_handle;

	subdevice->base.me_subdevice_postinit = me4600_ai_postinit;

	subdevice->base.me_subdevice_config_load = me4600_ai_config_load;
	// Prepare work queue.
	subdevice->me4600_workqueue = me4600_wq;

	// workqueue API changed in kernel 2.6.20
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&subdevice->ai_control_task, me4600_ai_work_control_task, (void *)subdevice);
#else
	INIT_DELAYED_WORK(&subdevice->ai_control_task, me4600_ai_work_control_task);
#endif

	// Default FIFO settings.
	subdevice->fifo_size = ME4600_AI_FIFO_COUNT;
	subdevice->fifo_half_size = ME4600_AI_FIFO_COUNT>>1;
	subdevice->fifo_max_sc = (3*ME4600_AI_FIFO_COUNT)>>2;

	subdevice->LE_size = ME4600_AI_LIST_COUNT;

	return subdevice;
}

static int inline ai_isnt_fifo_empty(me4600_ai_subdevice_t* instance)
{
	uint32_t tmp;
	me_readl(instance->base.dev, &tmp, instance->status_reg);
	return tmp & ME4600_AI_STATUS_BIT_EF_DATA;
}

static int inline ai_isnt_LE_fifo_full(me4600_ai_subdevice_t* instance)
{
	uint32_t tmp;
	me_readl(instance->base.dev, &tmp, instance->status_reg);
	return tmp & ME4600_AI_STATUS_BIT_FF_CHANNEL;
}

static void ai_determine_FIFO_size(me4600_ai_subdevice_t* instance)
{
	uint32_t ctrl;
	uint32_t tmp;
	int fifo_size  = 0;
	unsigned long int wait_start;

	ME_LOCK_PROTECTOR;
		PDEBUG("executed. idx=0\n");
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_high_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);

		// Clear all features. Dissable interrupts.
		ctrl = (0x0000
			| ME4600_AI_CTRL_BIT_MODE_0
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_SC_RELOAD
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Clear data and channel fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
		ctrl &= ~ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		// Write channel entry.
		me_writel(instance->base.dev, ME4600_AI_LIST_RANGE_BIPOLAR_10 | 1, instance->channel_list_reg);
		me_writel(instance->base.dev, ME4600_AI_LIST_INPUT_SINGLE_ENDED | ME4600_AI_LIST_RANGE_BIPOLAR_10 | 0, instance->channel_list_reg);
		// Start conversion.
		me_readl(instance->base.dev, &tmp, instance->start_reg);

		// Wait for FIFO to be full.
		wait_start = jiffies;
		do
		{// Use blocking mode (max. 125ms).
			// In progress.
			me_readl(instance->base.dev, &tmp, instance->status_reg);
		}
		while ((tmp & ME4600_AI_STATUS_BIT_FF_DATA) && (tmp & ME4600_AI_STATUS_BIT_FSM) && ((jiffies - wait_start)<(HZ>>3)));

		if ((jiffies - wait_start)>=(HZ>>3))
		{
			PERROR ("Determing FIFO's size failed.\n");
			goto EXIT;
		}

		while (ai_isnt_fifo_empty(instance) && (fifo_size<(0x1<<20)))
		{
			fifo_size++;
			me_readl(instance->base.dev, &tmp, instance->data_reg);
		}
		if (!(fifo_size<(0x1<<20)))
		{
			PERROR ("Determing FIFO's size failed.\n");
			goto EXIT;
		}

		instance->fifo_size = fifo_size;
		instance->fifo_half_size = fifo_size >> 1;
		instance->fifo_max_sc = ((3*fifo_size)>>2);

		PINFO("fifo_size=%d fifo_max_sc=%d \n", instance->fifo_size, instance->fifo_max_sc);
EXIT:
		ctrl = (0x0000
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_SC_RELOAD
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Clear data and channel fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
	ME_UNLOCK_PROTECTOR;

}

static void ai_determine_LE_size(me4600_ai_subdevice_t* instance)
{
	uint32_t ctrl;
	int LE_size  = 0;

	ME_LOCK_PROTECTOR;
		PDEBUG("executed. idx=0\n");
		// Clear all features. Dissable interrupts.
		ctrl = (0x0000
			| ME4600_AI_CTRL_BIT_MODE_0
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_SC_RELOAD
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Clear channel fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		// Enable channel fifo.
		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		// Write channel entry.
		while (ai_isnt_LE_fifo_full(instance) && (LE_size<(0x1<<20)))
		{
			me_writel(instance->base.dev, 0, instance->channel_list_reg);
			LE_size++;
		}
		if (!(LE_size<(0x1<<20)))
		{
			PERROR ("Determing size of Channel List failed.\n");
			goto EXIT;
		}

		instance->LE_size = LE_size;

		PINFO("LE_size=%d\n", instance->LE_size);

		if (instance->chan_list_copy)
		{
			kfree(instance->chan_list_copy);
		}
		instance->chan_list_copy = kzalloc(LE_size * sizeof(uint16_t), GFP_KERNEL);
EXIT:
		ctrl = (0x0000
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_SC_RELOAD
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Clear data and channel fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
	ME_UNLOCK_PROTECTOR;
}

static void ai_read_calibration(me4600_ai_subdevice_t* instance)
{
	int address_lenght = 0;
	me4600_ai_eeprom_t raw_cal = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	if (!me_plx_eeprom_check(instance->base.dev, instance->PLX_base))
	{
		me_plx_eeprom_address_lenght_check(instance->base.dev, instance->PLX_base, &address_lenght);
	}
	if ((address_lenght < 4) || (address_lenght > 32))
	{// No correct EEPROM
		return;
	}

	me_plx_eeprom_read(instance->base.dev, instance->PLX_base, address_lenght, ME4600_AI_CALIB_START, (uint16_t *) &raw_cal, sizeof(me4600_ai_eeprom_t)/sizeof(uint16_t));

	PINFO("year: %d\n", raw_cal.year);
	PINFO("month: %d\n", raw_cal.month);
	PINFO("unipolar_10A: 0x%04x\n", raw_cal.unipolar_10A);
	PINFO("unipolar_10B: 0x%04x\n", raw_cal.unipolar_10B);
	PINFO("bipolar_10A: 0x%04x\n", raw_cal.bipolar_10A);
	PINFO("bipolar_10B: 0x%04x\n", raw_cal.bipolar_10B);
	PINFO("differential_10A: 0x%04x\n", raw_cal.differential_10A);
	PINFO("differential_10B: 0x%04x\n", raw_cal.differential_10B);
	PINFO("unipolar_2_5A: 0x%04x\n", raw_cal.unipolar_2_5A);
	PINFO("unipolar_2_5B: 0x%04x\n", raw_cal.unipolar_2_5B);
	PINFO("bipolar_2_5A: 0x%04x\n", raw_cal.bipolar_2_5A);
	PINFO("bipolar_2_5B: 0x%04x\n", raw_cal.bipolar_2_5B);
	PINFO("differential_2_5A: 0x%04x\n", raw_cal.differential_2_5A);
	PINFO("differential_2_5B: 0x%04x\n", raw_cal.differential_2_5B);

	ai_calculate_calibration(instance, raw_cal);
	PINFO("CAL: unipolar_10.constant: %ld\n", instance->calibration.unipolar_10.constant);
	PINFO("CAL: unipolar_10.multiplier: %ld\n", instance->calibration.unipolar_10.multiplier);
	PINFO("CAL: unipolar_10.divisor: %ld\n", instance->calibration.unipolar_10.divisor);
	PINFO("CAL: bipolar_10.constant: %ld\n", instance->calibration.bipolar_10.constant);
	PINFO("CAL: bipolar_10.multiplier: %ld\n", instance->calibration.bipolar_10.multiplier);
	PINFO("CAL: bipolar_10.divisor: %ld\n", instance->calibration.bipolar_10.divisor);
	PINFO("CAL: differential_10.constant: %ld\n", instance->calibration.differential_10.constant);
	PINFO("CAL: differential_10.multiplier: %ld\n", instance->calibration.differential_10.multiplier);
	PINFO("CAL: differential_10.divisor: %ld\n", instance->calibration.differential_10.divisor);
	PINFO("CAL: unipolar_2_5.constant: %ld\n", instance->calibration.unipolar_2_5.constant);
	PINFO("CAL: unipolar_2_5.multiplier: %ld\n", instance->calibration.unipolar_2_5.multiplier);
	PINFO("CAL: unipolar_2_5.divisor: %ld\n", instance->calibration.unipolar_2_5.divisor);
	PINFO("CAL: bipolar_2_5.constant: %ld\n", instance->calibration.bipolar_2_5.constant);
	PINFO("CAL: bipolar_2_5.multiplier: %ld\n", instance->calibration.bipolar_2_5.multiplier);
	PINFO("CAL: bipolar_2_5.divisor: %ld\n", instance->calibration.bipolar_2_5.divisor);
	PINFO("CAL: differential_2_5.constant: %ld\n", instance->calibration.differential_2_5.constant);
	PINFO("CAL: differential_2_5.multiplier: %ld\n", instance->calibration.differential_2_5.multiplier);
	PINFO("CAL: differential_2_5.divisor: %ld\n", instance->calibration.differential_2_5.divisor);
}

static void ai_default_calibration_entry(me_calibration_entry_t* entry)
{
	entry->constant = 0;
	entry->multiplier = 1;
	entry->divisor = 1;
}

static void ai_default_calibration(me4600_ai_subdevice_t* instance)
{
	ai_default_calibration_entry (&instance->calibration.unipolar_10);
	ai_default_calibration_entry (&instance->calibration.bipolar_10);
	ai_default_calibration_entry (&instance->calibration.differential_10);

	ai_default_calibration_entry (&instance->calibration.unipolar_2_5);
	ai_default_calibration_entry (&instance->calibration.bipolar_2_5);
	ai_default_calibration_entry (&instance->calibration.differential_2_5);
}

static void ai_calculate_calibration_entry(int64_t nominal_A, int64_t actual_A, int64_t nominal_B, int64_t actual_B, me_calibration_entry_t* entry)
{
/**
	Because of both actual and desired characteristics of the transformer are linear, there is a
	linear relationship between the two. It is enough for the DIGITAL debit and
	actual values at two different (the sake of accuracy as far as possible disparate)
	points to know about freely between debit and actual values back and forth to counts.

	There is a relationship SOLL = IST * F + K between DIGITAL Transformer values.

	Suppose that the nominal value SOLL_a the actual value IST_a and setpoint
	SOLL_b corresponds to the actual value IST_b

	SOLL_a --> IST_a
	SOLL_b --> IST_b

	Then we have;

	SOLL_a = IST_a * F + K
	SOLL_b = IST_b * F + K

	Resulting:

	F = (SOLL_b - SOLL_a ) / (IST_b - IST_a)
	and
	K = (SOLL_a * IST_b - IST_a * SOLL_b) / (IST_b - IST_a)
	and so the relationship between projected and actual value is as follows:
	SOLL = ( IST *(SOLL_b - SOLL_a) + (SOLL_a * IST_b - IST_a * SOLL_b) ) / (IST_b - IST_a)

	Putting:
	X = SOLL_b - SOLL_a
	Y = SOLL_a * IST_b - IST_a * SOLL_b
	Z = IST_b - IST_a

	then we have:
	SOLL = (IST * X + Y) / Z
*/

	if(actual_A != actual_B)
	{
		entry->constant = -((nominal_A * actual_B) + (actual_A * nominal_B));
		entry->multiplier = nominal_B + nominal_A;
		entry->divisor = actual_B - actual_A;
	}
	else
	{
		ai_default_calibration_entry(entry);
	}
}

static void ai_calculate_calibration(me4600_ai_subdevice_t* instance, me4600_ai_eeprom_t raw_cal)
{
	int calibration_exist = 0;

	if ((raw_cal.year > 1990) && (raw_cal.year < 2100))
	{
		if ((raw_cal.month > 0) && (raw_cal.month < 13))
		{
			calibration_exist = 1;
		}
	}
	else if ((raw_cal.month > 1990) && (raw_cal.month < 2100))
	{	/// @note: Year and month may have been swapped due to an error in early versions of the calibration software.

		if ((raw_cal.year > 0) && (raw_cal.year < 13))
		{
			calibration_exist = 1;
		}
	}

	if (calibration_exist)
	{
# define AI_CAL_VALIDATE(val) ((val >= -1000) && (val <= 1000))

		if (!AI_CAL_VALIDATE(raw_cal.unipolar_10A + ME4600_AI_CALIB_UNIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.unipolar_10B - ME4600_AI_CALIB_UNIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 1!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.bipolar_10A + ME4600_AI_CALIB_BIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.bipolar_10B - ME4600_AI_CALIB_BIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 0!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.differential_10A + ME4600_AI_CALIB_BIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.differential_10B - ME4600_AI_CALIB_BIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 0 in differential mode!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.unipolar_2_5A + ME4600_AI_CALIB_UNIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.unipolar_2_5B - ME4600_AI_CALIB_UNIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 3!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.bipolar_2_5A + ME4600_AI_CALIB_BIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.bipolar_2_5B - ME4600_AI_CALIB_BIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 2!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.differential_2_5A + ME4600_AI_CALIB_BIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.differential_2_5B - ME4600_AI_CALIB_BIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 2 in differential mode!\n");
		}
	}
	else
	{
		PDEBUG("EEPROM doesn't contain valid calibration!\n");
	}

	if (calibration_exist)
	{
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_UNIPOLAR_10_A, raw_cal.unipolar_10A,
										ME4600_AI_CALIB_UNIPOLAR_10_B, raw_cal.unipolar_10B,
										&instance->calibration.unipolar_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_10_A, raw_cal.bipolar_10A,
										ME4600_AI_CALIB_BIPOLAR_10_B, raw_cal.bipolar_10B,
										&instance->calibration.bipolar_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_10_A, raw_cal.differential_10A,
										ME4600_AI_CALIB_BIPOLAR_10_B, raw_cal.differential_10B,
										&instance->calibration.differential_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_UNIPOLAR_2_5_A, raw_cal.unipolar_2_5A,
										ME4600_AI_CALIB_UNIPOLAR_2_5_B, raw_cal.unipolar_2_5B,
										&instance->calibration.unipolar_2_5);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_2_5_A, raw_cal.bipolar_2_5A,
										ME4600_AI_CALIB_BIPOLAR_2_5_B, raw_cal.bipolar_2_5B,
										&instance->calibration.bipolar_2_5);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_2_5_A, raw_cal.differential_2_5A,
										ME4600_AI_CALIB_BIPOLAR_2_5_B, raw_cal.differential_2_5B,
										&instance->calibration.differential_2_5);
	}
	else
	{
		ai_default_calibration(instance);
	}
}

static uint16_t inline ai_calculate_end_value(const me_calibration_entry_t calibration, int16_t value)
{
	long int cal_val;

	if (((value & 0xFFFF) == 0x8000) || ((value & 0xFFFF) == 0x7FFF))
	{// We shouldn't calibrate border values.
		return (uint16_t)(value ^ 0x8000);
	}

	cal_val = value;
	cal_val *= calibration.multiplier;
	cal_val += calibration.constant;
	cal_val /= calibration.divisor;

	if(cal_val < -0x8000)
	{
		cal_val = -0x8000;
	}
	else if(cal_val > 0x7FFF)
	{
		cal_val = 0x7FFF;
	}

	cal_val ^= 0x8000;

	return (uint16_t)cal_val;

}

static uint16_t inline ai_calculate_calibrated_value(me4600_ai_subdevice_t* instance, int entry, int value)
{
	uint16_t tmp;

	if(instance->raw_values)
	{
		PINFO("Returning raw values.\n");
		return value ^ 0x8000;
	}

	switch (entry & ME4600_AI_LIST_CONFIG_MASK)
	{
		case ME4600_AI_LIST_RANGE_BIPOLAR_10:
			tmp = ai_calculate_end_value(instance->calibration.bipolar_10, value);
			break;

		case ME4600_AI_LIST_RANGE_BIPOLAR_2_5:
			tmp = ai_calculate_end_value(instance->calibration.bipolar_2_5, value);
			break;

		case (ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_BIPOLAR_10):
			tmp = ai_calculate_end_value(instance->calibration.unipolar_10, value);
			break;

		case ME4600_AI_LIST_RANGE_UNIPOLAR_10:
			tmp = ai_calculate_end_value(instance->calibration.unipolar_2_5, value);
			break;

		case ME4600_AI_LIST_RANGE_UNIPOLAR_2_5:
			tmp = ai_calculate_end_value(instance->calibration.differential_10, value);
			break;

		case (ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_BIPOLAR_2_5):
			tmp = ai_calculate_end_value(instance->calibration.differential_2_5, value);
			break;

		default:
			PERROR("Unrecognized mode:0x%x\n", entry & ME4600_AI_LIST_CONFIG_MASK);
			tmp = value ^ 0x8000;
	}
	return tmp;
}

static int me4600_ai_config_load(me_subdevice_t* subdevice, struct file* filep, void* config)
{
	me4600_ai_subdevice_t* instance;
	me4600_config_load_t* ai_config;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_subdevice_t *)subdevice;

	ai_config = (me4600_config_load_t *)config;

	if (ai_config->comand_id != AI_BUFFER_RESIZE)
	{
		return ME_ERRNO_CONFIG_LOAD_FAILED;
	}

	err = me4600_ai_io_reset_subdevice(subdevice, filep, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
	if (!err)
	{
		ME_SUBDEVICE_ENTER;
			ME_LOCK_PROTECTOR;
			destroy_seg_buffer(&instance->seg_buf);
			instance->seg_buf = create_seg_buffer(ai_config->config.chunks_count, ai_config->config.chunk_size);
			ME_UNLOCK_PROTECTOR;
		ME_SUBDEVICE_EXIT;
	}

	return err;
}
