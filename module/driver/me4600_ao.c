/**
 * @file me4600_ao.c
 *
 * @brief ME-4600 analog output subdevice instance.
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


# include <linux/sched.h>
# include <linux/workqueue.h>
# include <asm/uaccess.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "me4600_reg.h"
# include "me4600_ao_reg.h"
# include "me4600_ao.h"
# include "medevice.h"

int me4600_ao_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
int me4600_ao_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
int me4600_ao_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
int me4600_ao_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
int me4600_ao_query_number_channels(me_subdevice_t* subdevice, int* number);
int me4600_ao_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me4600_ao_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int me4600_ao_query_subdevice_caps_args(me_subdevice_t* subdevice,int cap, int* args, int* count);

int me4600_ao_postinit(me_subdevice_t* subdevice, void* args);

/// Remove subdevice.
void me4600_ao_destructor(struct me_subdevice* subdevice);
/// Reset subdevice. Stop all actions. Reset registry. Disable FIFO. Set output to 0V and status to 'none'.
int me4600_ao_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
/// Set output as single
static int me4600_ao_io_single_config_check(me4600_ao_subdevice_t* instance, int channel,
    									int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me4600_ao_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
    									int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
/// Pass to user actual value of output.
static int me4600_ao_io_single_read_check(me4600_ao_subdevice_t* instance, int channel, int* value, int time_out, int flags);
int me4600_ao_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel,  int* value, int time_out, int flags);
/// Write to output requed value.
static int me4600_ao_io_single_write_check(me4600_ao_subdevice_t* instance, int channel, int value, int time_out, int flags);
int me4600_ao_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
/// Configure output as streamed device.
static int me4600_ao_io_stream_config_check(me4600_ao_subdevice_t* instance,
										meIOStreamSimpleConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);
int me4600_ao_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);
/// Helper for 'ao_io_stream_new_values'. Check HF bit.
inline static int me4600_ao_FSM_test(me4600_ao_subdevice_t* instance);
/// Wait for / Check empty space in buffer.
int me4600_ao_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags);
/// Start streaming.
int me4600_ao_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags);
/// Check actual state. / Wait for end.
int me4600_ao_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags);
/// Stop streaming.
int me4600_ao_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags);
/// Write datas to buffor.
int me4600_ao_io_stream_write(me_subdevice_t* subdevice, struct file* filep, int write_mode, int* values, int* count, int time_out, int flags);
/// Interrupt handler. Copy from buffer to FIFO.
int me4600_ao_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);
/// Copy data from circular buffer to fifo (fast) in wraparound mode.
int inline ao_write_data_wraparound(me4600_ao_subdevice_t* instance, int count, int start_pos);
/// Copy data from circular buffer to fifo (fast).
int inline ao_write_data(me4600_ao_subdevice_t* instance, int count, int start_pos);
/// Copy data from circular buffer to fifo (slow).
int inline ao_write_data_pooling(me4600_ao_subdevice_t* instance, int count, int start_pos);
/// Copy data from user space to circular buffer.
int inline ao_get_data_from_user(me4600_ao_subdevice_t* instance, int count, int* user_values);
/// Stop presentation. Preserve FIFOs.
int inline ao_stop_immediately(me4600_ao_subdevice_t* instance);
/// Task for asynchronical state verifying.
void me4600_ao_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
									void* subdevice
#else
									struct work_struct* work
#endif
								);

static int inline ao_isnt_fifo_full(me4600_ao_subdevice_t* instance);
static void ao_determine_FIFO_size(me4600_ao_subdevice_t* instance);

int me4600_ao_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4600_ao_subdevice_t* instance;
	uint32_t tmp;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			instance->status = ao_status_none;

			atomic_set(&instance->ao_control_task_flag, 0);
			instance->timeout.delay = 0;
			instance->timeout.start_time = jiffies;

			//Cancel control task
			PDEBUG("Cancel control task. idx=%d\n", instance->base.idx);
			cancel_delayed_work(&instance->ao_control_task);

			//Stop state machine.
			ao_stop_immediately(instance);

			//Remove from synchronous start.
			ME_SPIN_LOCK(instance->preload_reg_lock);
				me_readl(instance->base.dev, &tmp, instance->preload_reg);
				tmp &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
				me_writel(instance->base.dev, tmp, instance->preload_reg);
				*instance->preload_flags &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
			ME_SPIN_UNLOCK(instance->preload_reg_lock);

			//Set single mode, dissable FIFO, dissable external trigger, set output to analog, block interrupt.
			me_writel(instance->base.dev, ME4600_AO_MODE_SINGLE | ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ, instance->ctrl_reg);

			if (instance->fifo)
			{// Set speed for single operations.
				me_writel(instance->base.dev, ME4600_AO_MIN_CHAN_TICKS-1, instance->timer_reg);
			}

			//Set output to 0V
			me_writel(instance->base.dev, 0x8000, instance->single_reg);

			instance->circ_buf.head = 0;
			instance->circ_buf.tail = 0;
			instance->preloaded_count = 0;
			instance->data_count = 0;
			instance->single_value = 0x8000;
			instance->single_value_in_fifo = 0x8000;

			//Set status to signal that device is unconfigured.
			instance->stream_start_count = 0;
			instance->stream_stop_count = 0;

			instance->ctrl_trg = 0x0;
		ME_UNLOCK_PROTECTOR;

		//Signal reset if user is on wait.
		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ao_io_single_config_check(me4600_ao_subdevice_t* instance, int channel,
    									int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
		case ME_TRIG_TYPE_SW:
			if (trig_edge != ME_TRIG_EDGE_NONE)
			{
				PERROR("Invalid trigger edge. Software trigger has not edge! Must be ME_TRIG_EDGE_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_EDGE;
			}
			break;

		case ME_TRIG_TYPE_EXT_DIGITAL:
			switch(trig_edge)
			{
				case ME_TRIG_EDGE_ANY:
				case ME_TRIG_EDGE_RISING:
				case ME_TRIG_EDGE_FALLING:
					break;

				default:
					PERROR("Invalid trigger edge. Must be ME_TRIG_EDGE_RISING, ME_TRIG_EDGE_FALLING orME_TRIG_EDGE_ANY.\n");
					return ME_ERRNO_INVALID_TRIG_EDGE;
			}
			break;

		default:
			PERROR("Invalid trigger type. Should be ME_TRIG_TYPE_SW or ME_TRIG_TYPE_EXT_DIGITAL.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
			if (trig_chain != ME_TRIG_CHAN_NONE)
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;

		case ME_TRIG_TYPE_SW:
		case ME_TRIG_TYPE_EXT_DIGITAL:
			if ((trig_chain != ME_TRIG_CHAN_DEFAULT) && (trig_chain != ME_TRIG_CHAN_SYNCHRONOUS))
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT or ME_TRIG_CHAN_SYNCHRONOUS.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;
	}

	if (ref != ME_REF_AO_GROUND)
	{
		PERROR("Invalid reference. Must be REF_AO_GROUND.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if (single_config)
	{
		PERROR("Invalid range specified. Must be 0.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel  specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
    									int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4600_ao_subdevice_t* instance;
	uint32_t tmp;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	// Checking parameters
	err = me4600_ao_io_single_config_check(instance, channel, single_config, ref, trig_chain, trig_type, trig_edge, flags);
    if (err)
    	return err;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ao_status_none:
				case ao_status_single_configured:
				case ao_status_single_end:
				case ao_status_single_error:
				case ao_status_stream_configured:
				case ao_status_stream_end:
				case ao_status_stream_fifo_error:
				case ao_status_stream_buffer_error:
				case ao_status_stream_timeout:
					// OK - subdevice in idle
					break;

				case ao_status_single_run:
					//Subdevice running in single mode!
				case ao_status_stream_run_wait:
				case ao_status_stream_run:
				case ao_status_stream_end_wait:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ao_status_none;
			}

			/// @note For single all calls (config and write) are erasing previous state!

			// Correct single mirrors
			instance->single_value_in_fifo = instance->single_value;

			// Set control register. Set stop bit. Stop streaming mode.
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			//Reset all bits.
			tmp = ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_STOP;
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);

			if (trig_type == ME_TRIG_TYPE_EXT_DIGITAL)
			{
				PINFO("External digital trigger.\n");

				if (trig_edge == ME_TRIG_EDGE_ANY)
				{
					instance->ctrl_trg = ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH;
				}
				else if (trig_edge == ME_TRIG_EDGE_FALLING)
				{
					instance->ctrl_trg = ME4600_AO_CTRL_BIT_EX_TRIG_EDGE;
				}
				else if (trig_edge == ME_TRIG_EDGE_RISING)
				{
					instance->ctrl_trg = 0x0;
				}
			}
			else	//if (trig_type == ME_TRIG_TYPE_NONE)
			{
				PDEBUG("Software trigger.\n");
				instance->ctrl_trg = 0x0;
			}


			// Set preload/synchronization register.
			ME_SPIN_LOCK(instance->preload_reg_lock);
				if (trig_type == ME_TRIG_TYPE_EXT_DIGITAL)
				{
					*instance->preload_flags |= ME4600_AO_SYNC_EXT_TRIG << instance->base.idx;
				}
				else //if (trig_type == ME_TRIG_TYPE_NONE)
				{
					*instance->preload_flags &= ~(ME4600_AO_SYNC_EXT_TRIG << instance->base.idx);
				}

				if (trig_chain == ME_TRIG_CHAN_SYNCHRONOUS)
				{
					*instance->preload_flags |= ME4600_AO_SYNC_HOLD << instance->base.idx;
				}
				else //if (trig_chain == ME_TRIG_CHAN_DEFAULT)
				{
					*instance->preload_flags &= ~(ME4600_AO_SYNC_HOLD << instance->base.idx);
				}

				//Reset hardware register
				me_readl(instance->base.dev, &tmp, instance->preload_reg);
				tmp &= ~(ME4600_AO_SYNC_EXT_TRIG << instance->base.idx);
				tmp |= ME4600_AO_SYNC_HOLD << instance->base.idx;
				//Output configured in default (safe) mode.
				me_writel(instance->base.dev, tmp, instance->preload_reg);
			ME_SPIN_UNLOCK(instance->preload_reg_lock);

			instance->status = ao_status_single_configured;
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ao_io_single_read_check(me4600_ao_subdevice_t* instance, int channel, int* value, int time_out, int flags)
{
	if (flags & ~(ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS))
	{
		PERROR("Invalid flag specified. Must be ME_IO_SINGLE_TYPE_NO_FLAGS, ME_IO_SINGLE_TYPE_NONBLOCKING or ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me4600_ao_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	unsigned long int delay = LONG_MAX - 2;

	uint32_t tmp;
	uint32_t sync_mask;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	err = me4600_ao_io_single_read_check(instance, channel, value, time_out, flags);
	if (err)
	{
    	return err;
	}

	ME_SUBDEVICE_ENTER;
		/// @note When flag 'ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS' is set than output is triggered. ALWAYS!
		if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
		{//Fired all software synchronous outputs.
			me_readl(instance->base.dev, &sync_mask, instance->preload_reg);
			if (!instance->fifo)
			{//No FIFO
				tmp = ~(*instance->preload_flags | 0xFFFF0000);
				PINFO("Fired all software synchronous outputs. mask:0x%08x\n", tmp);
				tmp |= sync_mask & 0xFFFF0000;

				//Fire
				PINFO("Software trigger.\n");
				me_writel(instance->base.dev, tmp, instance->preload_reg);
			}
			else
			{// mix-mode - begin
				//Fire
				PINFO("Fired all software synchronous outputs by software trigger.\n");
				me_writel(instance->base.dev, 0x8000, instance->single_reg);

			}
			//Restore save settings
			me_writel(instance->base.dev, sync_mask, instance->preload_reg);
		}

		switch (instance->status)
		{
			case ao_status_single_run:
				if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
				{//Blocking mode. Wait for trigger.
					PINFO("BLOCKING MODE\n");
					if (time_out)
					{
						delay = (time_out * HZ) / 1000;
						if (!delay)
							delay = 1;
						if (delay>LONG_MAX - 2)
							delay = LONG_MAX - 2;
					}

					//Only runing process will interrupt this call. Events are signaled when status change. This procedure has own timeout.
					wait_event_interruptible_timeout(
						instance->wait_queue,
						(instance->status != ao_status_single_run),
						delay);

					if (signal_pending(current))
					{
						PERROR("Interrupted by signal.\n");
						err = ME_ERRNO_SIGNAL;
					}
					else
					{
						switch (instance->status)
						{
							case ao_status_single_run:
								PDEBUG("Timeout reached.\n");
								err = ME_ERRNO_TIMEOUT;
								break;

							case ao_status_single_error:
								PDEBUG("Start timeout reached.\n");
								err = ME_ERRNO_CANCELLED;
								break;

							default:
								PDEBUG("Single canceled.\n");
								err = ME_ERRNO_CANCELLED;
								break;
						}
					}
				}
				else
				{
					PINFO("NON_BLOCKING MODE\n");
				}
				break;

			case ao_status_none:
			case ao_status_single_configured:
			case ao_status_single_end:
			case ao_status_single_error:
			case ao_status_stream_configured:
				// OK - subdevice is in single mode
				break;

			case ao_status_stream_run_wait:
			case ao_status_stream_run:
			case ao_status_stream_end_wait:
			case ao_status_stream_end:
			case ao_status_stream_fifo_error:
			case ao_status_stream_buffer_error:
			case ao_status_stream_timeout:
			//Subdevice is in stream mode!
				PERROR("Subdevice not in single mode!\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				break;

			default:
				PERROR_CRITICAL("WRONG STATUS!\n");
				instance->status = ao_status_none;
				goto ERROR;
		}

		*value = instance->single_value;
ERROR:
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ao_io_single_write_check(me4600_ao_subdevice_t* instance, int channel, int value, int time_out, int flags)
{
	if (flags & ~(ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS | ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING))
	{
		PERROR("Invalid flag specified. Must be ME_IO_SINGLE_TYPE_NO_FLAGS, ME_IO_SINGLE_TYPE_NONBLOCKING or ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
#ifdef SCALE_RT
	if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
	{
		PERROR("Invalid flag specified. ME_IO_SINGLE_TYPE_NONBLOCKING has to be set.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
#endif

	if(value & ~ME4600_AO_MAX_DATA)
	{
		PERROR("Invalid value provided. Must be between 0 and %d.\n", ME4600_AO_MAX_DATA);
		return ME_ERRNO_VALUE_OUT_OF_RANGE;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me4600_ao_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;

	//Registry handling variables.
	uint32_t sync_mask;
	uint32_t mode;
	uint32_t tmp;
	uint32_t ctrl;
	uint32_t status;
	unsigned long int j;
	uint32_t synch;

	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	err = me4600_ao_io_single_write_check(instance, channel, value, time_out, flags);
    if (err)
    {
    	return err;
    }

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (instance->status)
			{
				case ao_status_single_configured:
				case ao_status_single_end:
				case ao_status_single_error:
					// OK - subdevice is in single mode
					break;

				case ao_status_single_run:
					//Subdevice running in single mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				case ao_status_none:
				case ao_status_stream_configured:
				case ao_status_stream_run_wait:
				case ao_status_stream_run:
				case ao_status_stream_end_wait:
				case ao_status_stream_end:
				case ao_status_stream_fifo_error:
				case ao_status_stream_buffer_error:
				case ao_status_stream_timeout:
				//Subdevice is in stream mode!
					PERROR("Subdevice not in single mode!\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ao_status_none;
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
			}

			if (time_out)
			{
				delay = (time_out * HZ) / 1000;
				if (!delay)
					delay = 1;
				if (delay>LONG_MAX - 2)
					delay = LONG_MAX - 2;
			}

			/// @note For single all calls (config and write) are erasing previous state!

			// Correct single mirrors
			instance->single_value_in_fifo = value;

			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);

			if (instance->fifo)
			{// mix-mode
				//Set speed
				me_writel(instance->base.dev, ME4600_AO_MIN_CHAN_TICKS-1, instance->timer_reg);
				me_readl(instance->base.dev, &status, instance->status_reg);

				//Set the continous mode.
				ctrl &= ~ME4600_AO_CTRL_MODE_MASK;
				ctrl |= ME4600_AO_MODE_CONTINUOUS;

				//Prepare FIFO
				if (!(ctrl & ME4600_AO_CTRL_BIT_ENABLE_FIFO))
				{//FIFO wasn't enabeled. Do it.
					PINFO("Enableing FIFO.\n");
					ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
					ctrl |= ME4600_AO_CTRL_BIT_ENABLE_FIFO | ME4600_AO_CTRL_BIT_RESET_IRQ;
				}
				else
				{//Check if FIFO is empty
					if (status & ME4600_AO_STATUS_BIT_EF)
					{//FIFO not empty
						PINFO("Reseting FIFO.\n");
						ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_FIFO | ME4600_AO_CTRL_BIT_ENABLE_IRQ);
						ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ;
						me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
						ctrl |= ME4600_AO_CTRL_BIT_ENABLE_FIFO | ME4600_AO_CTRL_BIT_RESET_IRQ;
					}
					else
					{//FIFO empty, only interrupt needs to be disabled!
						ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
						ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ;
					}
				}
				me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

				//Write output - 1 value to FIFO
				PINFO("Write value :%x\n", value);
				me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);
			}
			else
			{//No FIFO
				//Set the single mode.
				ctrl &= ~ME4600_AO_CTRL_MODE_MASK;

				//Write value
				PINFO("Write value :%x\n", value);
				me_writel(instance->base.dev, value | (value << 16), instance->single_reg);
			}

			mode = *instance->preload_flags >> instance->base.idx;
			mode &= (ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG);
			PINFO("Triggering mode: 0x%x\n", mode);

			//Set speed to optimum
			if (mode)
			{
				me_writel(instance->base.dev, ME4600_AO_OPTIMAL_CHAN_TICKS, instance->timer_reg);
			}
			else
			{
				me_writel(instance->base.dev, ME4600_AO_MIN_CHAN_TICKS-1, instance->timer_reg);
			}

			ME_SPIN_LOCK(instance->preload_reg_lock);
				me_readl(instance->base.dev, &sync_mask, instance->preload_reg);
				switch(mode)
				{
					case 0:													//Individual software
						ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG;

						if (!instance->fifo)
						{// No FIFO - In this case resetting 'ME4600_AO_SYNC_HOLD' will trigger output.
							if((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != ME4600_AO_SYNC_HOLD)
							{//Now we can set correct mode. This is exception. It is set to synchronous and triggered later.
								sync_mask &= ~(ME4600_AO_SYNC_EXT_TRIG << instance->base.idx);
								sync_mask |= ME4600_AO_SYNC_HOLD << instance->base.idx;
								me_writel(instance->base.dev, sync_mask, instance->preload_reg);
							}
						}
						else
						{// FIFO
							if ((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != 0x0)
							{//Now we can set correct mode.
								sync_mask &= ~((ME4600_AO_SYNC_EXT_TRIG | ME4600_AO_SYNC_HOLD) << instance->base.idx);
								me_writel(instance->base.dev, sync_mask, instance->preload_reg);
							}
						}
					break;

					case ME4600_AO_SYNC_EXT_TRIG:							//Individual hardware
						ctrl |= ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG;

						if (!instance->fifo)
						{// No FIFO - In this case resetting 'ME4600_AO_SYNC_HOLD' will trigger output.
							if ((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != ME4600_AO_SYNC_HOLD)
							{ //Now we can set correct mode
								sync_mask &= ~(ME4600_AO_SYNC_EXT_TRIG << instance->base.idx);
								sync_mask |= ME4600_AO_SYNC_HOLD << instance->base.idx;
								me_writel(instance->base.dev, sync_mask, instance->preload_reg);
							}
						}
						else
						{// FIFO
							if ((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != 0x0)
							{//Now we can set correct mode.
								sync_mask &= ~((ME4600_AO_SYNC_EXT_TRIG | ME4600_AO_SYNC_HOLD) << instance->base.idx);
								me_writel(instance->base.dev, sync_mask, instance->preload_reg);
							}
						}
					break;

					case ME4600_AO_SYNC_HOLD:								//Synchronous software
						ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG;

						if((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != (ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG))
						{ //Now we can set correct mode
							sync_mask |= ME4600_AO_SYNC_EXT_TRIG << instance->base.idx;
							sync_mask |= ME4600_AO_SYNC_HOLD << instance->base.idx;
							me_writel(instance->base.dev, sync_mask, instance->preload_reg);
						}
					break;

					case (ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG): 	//Synchronous hardware
						ctrl |= ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG;

						if ((sync_mask & ((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx)) != (ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG))
						{//Now we can set correct mode
							sync_mask |= (ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx;
							me_writel(instance->base.dev, sync_mask, instance->preload_reg);
						}
					break;
				}

				//Activate ISM (remove 'stop' bits)
				ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
				ctrl |= instance->ctrl_trg;
				ctrl &= ~(ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP);
				me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

				/// @note When flag 'ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS' is set than output is triggered. ALWAYS!
				if (!instance->fifo)
				{//No FIFO
					if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
					{//Fired all software synchronous outputs.
						tmp = ~(*instance->preload_flags | 0xFFFF0000);
						PINFO("Fired all software synchronous outputs. mask:0x%08x\n", tmp);
						tmp |= sync_mask & 0xFFFF0000;
						// Add this channel to list
						tmp &= ~(ME4600_AO_SYNC_HOLD << instance->base.idx);

						//Fire
						PINFO("Software trigger.\n");
						me_writel(instance->base.dev, tmp, instance->preload_reg);

						//Restore save settings
						me_writel(instance->base.dev, sync_mask, instance->preload_reg);
					}
					else if (!mode)
					{// Add this channel to list
						me_writel(instance->base.dev, sync_mask & ~(ME4600_AO_SYNC_HOLD << instance->base.idx), instance->preload_reg);

						//Fire
						PINFO("Software trigger.\n");

						//Restore save settings
						me_writel(instance->base.dev, sync_mask, instance->preload_reg);
					}
				}
				else
				{// mix-mode - begin
					if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
					{//Trigger outputs
						//Add channel to start list
						me_writel(instance->base.dev, sync_mask | (ME4600_AO_SYNC_HOLD << instance->base.idx), instance->preload_reg);

						//Fire
						PINFO("Fired all software synchronous outputs by software trigger.\n");
						me_writel(instance->base.dev, 0x8000, instance->single_reg);

						//Restore save settings
						me_writel(instance->base.dev, sync_mask, instance->preload_reg);
					}
					else if (!mode)
					{//Trigger outputs
			/*			//Remove channel from start list //<== Unnecessary. Removed.
						me_writel(instance->base.dev, sync_mask & ~(ME4600_AO_SYNC_HOLD << instance->base.idx), instance->preload_reg);
			*/
						//Fire
						PINFO("Software trigger.\n");
						me_writel(instance->base.dev, 0x8000, instance->single_reg);

			/*			//Restore save settings //<== Unnecessary. Removed.
						me_writel(instance->base.dev, sync_mask, instance->preload_reg);
			*/
					}
				}

				if (!mode || (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS))
				{/// @note BUGFIX for bug with queue_delayed_work() In this case we do not need synchronization.
					// Mark as ended
					instance->status = ao_status_single_end;
					instance->single_value = value;
				}
				else
				{
					instance->status = ao_status_single_run;
				}
			ME_SPIN_UNLOCK(instance->preload_reg_lock);
#ifdef SCALE_RT
			instance->status = ao_status_single_end;
			goto ERROR;
#endif

			if (instance->status != ao_status_single_end)
			{
				j = jiffies;
				instance->timeout.delay = delay;
				instance->timeout.start_time = j;
				PDEBUG("Schedule control task.\n");
				atomic_set(&instance->ao_control_task_flag, 1);
				queue_delayed_work(instance->me4600_workqueue, &instance->ao_control_task, 1);
				if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
				{
					PINFO("BLOCKING MODE\n");
					ME_SUBDEVICE_UNLOCK;
					//Only runing process will interrupt this call. Events are signaled when status change.
					wait_event_interruptible_timeout(
						instance->wait_queue,
						(instance->status != ao_status_single_run),
						delay+1);
					ME_SUBDEVICE_LOCK;

					if (signal_pending(current))
					{
						PERROR("Interrupted by signal.\n");
						err = ME_ERRNO_SIGNAL;
					}
					else
					{
						switch (instance->status)
						{
							case ao_status_single_end:
								break;

							case ao_status_single_run:
							PERROR("Timeout reached. Not handled by control task.\n");
							err = ME_ERRNO_TIMEOUT;
							// Override control task.
							atomic_set(&instance->ao_control_task_flag, 0);
							instance->timeout.delay = 0;
							instance->timeout.start_time = jiffies;

							//Cancel control task
							PDEBUG("Cancel control task. idx=%d\n", instance->base.idx);
							cancel_delayed_work(&instance->ao_control_task);

							// Stop all actions. No conditions! Block interrupts and trigger.
							me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
							ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
							ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
							ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
							if (instance->fifo)
							{//Disabling FIFO
								ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_FIFO;
							}
							me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

							ME_SPIN_LOCK(instance->preload_reg_lock);
								//Remove from synchronous start. Block triggering from this output.
								me_readl(instance->base.dev, &synch, instance->preload_reg);
								synch &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
								if (!(instance->fifo))
								{// No FIFO - set to single safe mode
									synch |= ME4600_AO_SYNC_HOLD << instance->base.idx;
								}
								me_writel(instance->base.dev, synch, instance->preload_reg);
							ME_SPIN_UNLOCK(instance->preload_reg_lock);

							// Set correct value for single_read();
							instance->single_value_in_fifo = instance->single_value;

							if (!(instance->fifo))
							{// No FIFO
								PDEBUG("Write old value back to register.\n");
								me_writel(instance->base.dev, instance->single_value | (instance->single_value << 16), instance->single_reg);
							}
							instance->status = ao_status_single_error;
							break;

						case ao_status_single_error:
							PDEBUG("Timeout reached.\n");
							err = ME_ERRNO_TIMEOUT;
							break;

						default:
							PDEBUG("Single canceled.\n");
							err = ME_ERRNO_CANCELLED;
							break;
					}
					}
				}
				else
				{
					PINFO("NON_BLOCKING MODE\n");
				}
			}
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;
	return err;
}

static int me4600_ao_io_stream_config_check(me4600_ao_subdevice_t* instance, meIOStreamSimpleConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	if (!instance->fifo)
	{
		PERROR("Not a streaming ao.\n");
		return ME_ERRNO_NOT_SUPPORTED;
	}

	if (flags & ~(ME_IO_STREAM_CONFIG_HARDWARE_ONLY | ME_IO_STREAM_CONFIG_WRAPAROUND | ME_IO_STREAM_CONFIG_BIT_PATTERN))
	{
		PERROR("Invalid flags. Must be ME_IO_STREAM_CONFIG_NO_FLAGS, ME_IO_STREAM_CONFIG_HARDWARE_ONLY, ME_IO_STREAM_CONFIG_WRAPAROUND or ME_IO_STREAM_CONFIG_BIT_PATTERN.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (flags & ME_IO_STREAM_CONFIG_HARDWARE_ONLY)
	{
		if(!(flags & ME_IO_STREAM_CONFIG_WRAPAROUND))
		{
			PERROR("ME_IO_STREAM_CONFIG_HARDWARE_ONLY has to be combined ME_IO_STREAM_CONFIG_WRAPAROUND.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}

		if((trigger->stop_type != ME_STREAM_STOP_TYPE_MANUAL))
		{
			PERROR("Hardware wraparound mode must be in infinite mode.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}
	}

	if (fifo_irq_threshold)
	{
		PERROR("Invalid iFifoIrqThreshold. Must be 0.\n");
		return ME_ERRNO_INVALID_STREAM_CONFIG;
	}

	if (count != 1)
	{
		PERROR("Invalid meIOStreamConfig list size. Must be 1.\n");
		return ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
	}

	if (config_list[0].iChannel != 0)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if (config_list[0].iRange != 0)
	{
		PERROR("Invalid range specified. Must be 0.\n");
		return ME_ERRNO_INVALID_STREAM_CONFIG;
	}

	switch (trigger->trigger_type)
	{
		case ME_TRIGGER_TYPE_SOFTWARE:
			if (trigger->trigger_edge != ME_TRIG_EDGE_NONE)
			{
				PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_NONE.\n");
				return ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
			}
			break;

		case ME_TRIGGER_TYPE_ACQ_DIGITAL:
			switch (trigger->trigger_edge)
			{
				case ME_TRIG_EDGE_RISING:
				case ME_TRIG_EDGE_FALLING:
				case ME_TRIG_EDGE_ANY:
					break;

				default:
					PERROR("Invalid acquisition start trigger edge specified. Must be ME_TRIG_EDGE_RISING, ME_TRIG_EDGE_FALLING or ME_TRIG_EDGE_ANY.\n");
					return ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE;
			}
			break;

		case ME_TRIGGER_TYPE_ACQ_ANALOG:
		case ME_TRIGGER_TYPE_LIST_DIGITAL:
		case ME_TRIGGER_TYPE_LIST_ANALOG:
		case ME_TRIGGER_TYPE_CONV_DIGITAL:
		case ME_TRIGGER_TYPE_CONV_ANALOG:
		default:
			PERROR("Invalid acquisition trigger type specified.\n");
			return ME_ERRNO_INVALID_ACQ_START_TRIG_TYPE;
	}

	if ((trigger->conv_ticks < ME4600_AO_MIN_CHAN_TICKS) || (trigger->conv_ticks > ME4600_AO_MAX_CHAN_TICKS))
	{
		PERROR("Invalid conv start trigger argument specified. Must be between %lld and %lld.\n", ME4600_AO_MIN_CHAN_TICKS, ME4600_AO_MAX_CHAN_TICKS);
		return ME_ERRNO_INVALID_CONV_START_ARG;
	}

	if (trigger->acq_ticks)
	{
		PERROR("Invalid acq start trigger argument specified. Must be 0.\n");
		return ME_ERRNO_INVALID_ACQ_START_ARG;
	}

	if (trigger->scan_ticks)
	{
		PERROR("Invalid scan start trigger argument specified. Must be 0.\n");
		return ME_ERRNO_INVALID_SCAN_START_ARG;
	}

	switch (trigger->stop_type)
	{
		case ME_STREAM_STOP_TYPE_MANUAL:
			if (trigger->stop_count)
			{
				PERROR("Invalid stop count specified. Must be 0.\n");
				return ME_ERRNO_INVALID_ACQ_STOP_ARG;
			}
			break;

		case ME_STREAM_STOP_TYPE_SCAN_VALUE:
			if (trigger->stop_count <= 0)
			{
				PERROR("Invalid stop count specified. Must be at least 1.\n");
				return ME_ERRNO_INVALID_SCAN_STOP_ARG;
			}
			break;

		case ME_STREAM_STOP_TYPE_ACQ_LIST:
			if (trigger->stop_count <= 0)
			{
				PERROR("Invalid stop count specified. Must be at least 1.\n");
				return ME_ERRNO_INVALID_ACQ_STOP_ARG;
			}
			break;

		default:
			PERROR("Invalid scan stop trigger type specified.\n");
			return ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE;
	}

	switch (trigger->synchro)
	{
		case ME_TRIG_CHAN_NONE:
		case ME_TRIG_CHAN_DEFAULT:
		case ME_TRIG_CHAN_SYNCHRONOUS:
			break;

		default:
			PERROR("Invalid acq start trigger channel specified. Must be ME_TRIG_CHAN_NONE, ME_TRIG_CHAN_DEFAULT or ME_TRIG_CHAN_SYNCHRONOUS.\n");
			return ME_ERRNO_INVALID_ACQ_START_TRIG_CHAN;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
    									meIOStreamSimpleConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{
	me4600_ao_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint32_t ctrl;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	err = me4600_ao_io_stream_config_check(instance, config_list, count, trigger, fifo_irq_threshold, flags);
    if (err)
    	return err;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			if ((flags & ME_IO_STREAM_CONFIG_BIT_PATTERN) && !instance->bitpattern)
			{
				PERROR("This subdevice doesn't support output redirection.\n");
				err = ME_ERRNO_INVALID_FLAGS;
				goto ERROR;
			}

			switch (instance->status)
			{
				case ao_status_none:
				case ao_status_single_configured:
				case ao_status_single_end:
				case ao_status_single_error:
				case ao_status_stream_configured:
				case ao_status_stream_end:
				case ao_status_stream_fifo_error:
				case ao_status_stream_buffer_error:
				case ao_status_stream_timeout:
					// OK - subdevice in idle
					break;

				case ao_status_single_run:
					//Subdevice running in single mode!
				case ao_status_stream_run_wait:
				case ao_status_stream_run:
				case ao_status_stream_end_wait:
					//Subdevice running in stream mode!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ao_status_none;
			}

			//Reset control register. Block all actions. Disable IRQ. Disable FIFO.
			ctrl = ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			//This is paranoic, but to be sure.
			instance->preloaded_count = 0;
			instance->data_count = 0;
			instance->circ_buf.head = 0;
			instance->circ_buf.tail = 0;

			// Set mode.
			if (flags & ME_IO_STREAM_CONFIG_WRAPAROUND)
			{//Wraparound
				if (flags & ME_IO_STREAM_CONFIG_HARDWARE_ONLY)
				{//Hardware wraparound
					PINFO("Hardware wraparound.\n");
					ctrl |= ME4600_AO_MODE_WRAPAROUND;
					instance->mode = ME4600_AO_HW_WRAP_MODE;
				}
				else
				{//Software wraparound
					PINFO("Software wraparound.\n");
					ctrl |= ME4600_AO_MODE_CONTINUOUS;
					instance->mode = ME4600_AO_SW_WRAP_MODE;
				}
			}
			else
			{//Continous
				PINFO("Continous.\n");
				ctrl |= ME4600_AO_MODE_CONTINUOUS;
				instance->mode = ME4600_AO_CONTINUOUS;
			}
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			switch (trigger->trigger_type)
			{
				case ME_TRIGGER_TYPE_SOFTWARE:
					PINFO("Internal software trigger.\n");
					instance->start_mode = 0;
					break;

				case ME_TRIGGER_TYPE_ACQ_DIGITAL:
					//Set the trigger edge.
					PINFO("External digital trigger.\n");
					instance->start_mode = ME4600_AO_EXT_TRIG;

					switch (trigger->trigger_edge)
					{
						case ME_TRIG_EDGE_RISING:
						PINFO("Set the trigger edge: rising.\n");
							break;

						case ME_TRIG_EDGE_FALLING:
						PINFO("Set the trigger edge: falling.\n");
							ctrl |= ME4600_AO_CTRL_BIT_EX_TRIG_EDGE;
							break;

						case ME_TRIG_EDGE_ANY:
						PINFO("Set the trigger edge: both edges.\n");
							ctrl |= ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH;
							break;
					}
					break;
			}

			//Set the stop mode and value.
			switch (trigger->stop_type)
			{
				case ME_STREAM_STOP_TYPE_SCAN_VALUE:
					instance->stop_mode = ME4600_AO_SCAN_STOP_MODE;
					instance->stop_count = trigger->stop_count;
					break;

				case ME_STREAM_STOP_TYPE_ACQ_LIST:
					instance->stop_mode = ME4600_AO_LIST_STOP_MODE;
					instance->stop_count = trigger->stop_count;
					break;

				default:
					instance->stop_mode = ME4600_AO_INF_STOP_MODE;
					instance->stop_count = 0;
			}

			PINFO("Stop mode:%d count:%d.\n", instance->stop_mode, instance->stop_count);

			if (trigger->synchro == ME_TRIG_CHAN_SYNCHRONOUS)
			{//Synchronous start
				instance->start_mode |= ME4600_AO_SYNC_HOLD;
				if (trigger->trigger_type == ME_TRIGGER_TYPE_ACQ_DIGITAL)
				{//Externaly triggered
					PINFO("Synchronous start. Externaly trigger active.\n");
					instance->start_mode |= ME4600_AO_SYNC_EXT_TRIG;
				}
				else
				{
					PINFO("Synchronous start. External trigger dissabled.\n");
				}
			}

			//Set speed
			me_writel(instance->base.dev, trigger->conv_ticks - 2, instance->timer_reg);

			//Connect outputs to analog or digital port.
			if (flags & ME_IO_STREAM_CONFIG_BIT_PATTERN)
			{
				ctrl |= ME4600_AO_CTRL_BIT_ENABLE_DO;
			}

			// Write the control word
			instance->ctrl_trg = ctrl;

			//Set status.
			instance->status = ao_status_stream_configured;
ERROR:
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

inline static int me4600_ao_FSM_test(me4600_ao_subdevice_t* instance)
{
	uint32_t tmp;

	me_readl(instance->base.dev, &tmp, instance->status_reg);

	return tmp;
}

int me4600_ao_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags)
{
	me4600_ao_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long int j;
	int tail;
	int signaling = 0;
	int status;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (!instance->fifo)
	{
		PERROR("Not a streaming ao.\n");
		return ME_ERRNO_NOT_SUPPORTED;
	}

	if (flags & ~(ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG | ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG))
	{
		PERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (!instance->circ_buf.buf)
	{
		PERROR("Circular buffer not exists.\n");
		return ME_ERRNO_INTERNAL;
	}

	ME_SUBDEVICE_ENTER;
		if (time_out)
		{
			delay = (time_out * HZ) / 1000;
			if (!delay)
				delay = 1;
			if (delay>LONG_MAX - 2)
				delay = LONG_MAX - 2;
		}

		j = jiffies;
		while(!signaling)
		{
			// Correct timeout.
			delay -= jiffies - j;

			status = instance->status;
			tail = instance->circ_buf.tail;
			if (flags &  ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG)
			{// Report errors
				switch (instance->status)
				{
					case ao_status_none:
						signaling = 1;
						err = ME_ERRNO_CANCELLED;
						break;

					case ao_status_stream_fifo_error:
						signaling = 1;
						err = ME_ERRNO_HARDWARE_BUFFER_UNDERFLOW;
						break;

					case ao_status_stream_buffer_error:
						signaling = 1;
						err = ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
						break;

					default:
						break;
				}

				if (signaling)
				{
					break;
				}
			}
			//Only runing process will interrupt this call. Interrupts are when FIFO HF is signaled.
			if (flags & ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG)
			{// Read from software buffer.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(
						(tail != instance->circ_buf.tail)
						||
						(status != instance->status)
					),
					delay);
			}
			else
			{// Space in buffer.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(
						me_circ_buf_space(&instance->circ_buf)
						||
						(status != instance->status)
					),
					delay);
			}

			if (signal_pending(current))
			{
				PERROR("Interrupted by signal.\n");
				err = ME_ERRNO_SIGNAL;
				break;
			}
			else
			{
				switch (instance->status)
				{
					case ao_status_none:
						signaling = 1;
						err = ME_ERRNO_CANCELLED;
						break;

					case ao_status_stream_fifo_error:
						signaling = 1;
						err = ME_ERRNO_HARDWARE_BUFFER_UNDERFLOW;
						break;

					case ao_status_stream_buffer_error:
						signaling = 1;
						err = ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
						break;

					case ao_status_stream_run_wait:
					case ao_status_stream_run:
						signaling = 1;
						break;

					default:
						break;
				}

				if ((jiffies - j) >= delay)
				{
					PERROR("Wait on values timed out.\n");
					err = ME_ERRNO_TIMEOUT;
					break;
				}
			}
		}

		// Inform user about empty space.
		*count = me_circ_buf_space(&instance->circ_buf);
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_ao_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags)
{
	me4600_ao_subdevice_t* instance;
	uint32_t status;
	uint32_t ctrl;
	uint32_t synch;
	int count = 0;
	int circ_buffer_count;
	unsigned long ref;
	unsigned long int delay = LONG_MAX -2;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (!instance->fifo)
	{
		PERROR("Not a streaming ao.\n");
		return ME_ERRNO_NOT_SUPPORTED;
	}

	if (flags & ~ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS)
	{
		PERROR("Invalid flags. Must be ME_IO_STREAM_START_TYPE_NO_FLAGS or ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((start_mode != ME_START_MODE_BLOCKING) && (start_mode != ME_START_MODE_NONBLOCKING))
	{
		PERROR("Invalid start mode specified. Must be ME_START_MODE_BLOCKING or ME_START_MODE_NONBLOCKING.\n");
		return ME_ERRNO_INVALID_START_MODE;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			if (time_out)
			{
				delay = (time_out * HZ) / 1000;
				if (!delay)
					delay = 1;
				if (delay>LONG_MAX - 2)
					delay = LONG_MAX - 2;
			}

			switch (instance->status)
			{//Checking actual mode.
				case ao_status_stream_configured:
				case ao_status_stream_timeout:
				case ao_status_stream_end:
					//Correct modes!
					break;

				//The device is in wrong mode.
				case ao_status_none:
				case ao_status_single_configured:
				case ao_status_single_run:
				case ao_status_single_end:
				case ao_status_single_error:
					PERROR("Subdevice must be preinitialize correctly for streaming.\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;

				case ao_status_stream_fifo_error:
				case ao_status_stream_buffer_error:
					PDEBUG("Before restart broken stream 'STOP' must be called.\n");
					err = ME_STATUS_ERROR;
					goto ERROR;

				case ao_status_stream_run_wait:
				case ao_status_stream_run:
				case ao_status_stream_end_wait:
					PDEBUG("Stream is already working.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;

				default:
					instance->status = ao_status_none;
					PERROR_CRITICAL("Status is in wrong state!\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
			}

			if (instance->mode == ME4600_AO_CONTINUOUS)
			{//Continous
				instance->circ_buf.tail += instance->preloaded_count;
				instance->circ_buf.tail &= instance->circ_buf.mask;
			}
			circ_buffer_count = me_circ_buf_values(&instance->circ_buf);

			if (!circ_buffer_count && !instance->preloaded_count)
			{//No values in buffer
				PERROR("No values in buffer!\n");
				err  = ME_ERRNO_LACK_OF_RESOURCES;
				goto ERROR;
			}

			//Set values for single_read()
			instance->single_value = ME4600_AO_MAX_DATA + 1;
			instance->single_value_in_fifo = ME4600_AO_MAX_DATA + 1;

			//Setting stop points
			if (instance->stop_mode == ME4600_AO_LIST_STOP_MODE)
			{
				instance->stop_data_count = instance->stop_count * circ_buffer_count;
			}
			else
			{
				instance->stop_data_count = instance->stop_count;
			}

			if ((instance->stop_data_count != 0) && (instance->stop_data_count < circ_buffer_count))
			{
				PERROR("More data in buffer than previously set limit!\n");
			}

			// Check FIFO
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
			if (!(ctrl & ME4600_AO_CTRL_BIT_ENABLE_FIFO))
			{//FIFO wasn't enabeled. Do it. <= This should be done by user call with ME_WRITE_MODE_PRELOAD
				PINFO("Enableing FIFO.\n");

				instance->preloaded_count = 0;
				instance->data_count = 0;
			}

			// Set ctrl
			ctrl = instance->ctrl_trg & ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
			//Block IRQ
			ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
			ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ | ME4600_AO_CTRL_BIT_ENABLE_FIFO;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			//Fill FIFO <= Generaly this should be done by user pre-load call but this is second place to do it.
			me_readl(instance->base.dev, &status, instance->status_reg);
			if (!(status & ME4600_AO_STATUS_BIT_EF))
			{//FIFO empty
				if (instance->stop_data_count == 0)
				{
					count = instance->fifo_size;
				}
				else
				{
					count = (instance->fifo_size < instance->stop_data_count) ? instance->fifo_size : instance->stop_data_count;
				}

				//Copy data
				count = ao_write_data(instance, count, instance->preloaded_count);
				if (count < 0)
				{//This should never happend!
					PERROR_CRITICAL("COPY FINISH WITH ERROR!\n");
					err = -count;
					goto ERROR;
				}
			}

			//Set pre-load features.
			ME_SPIN_LOCK(instance->preload_reg_lock);
				me_readl(instance->base.dev, &synch, instance->preload_reg);
				synch &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
				synch |= (instance->start_mode & ~ME4600_AO_EXT_TRIG) << instance->base.idx;
				me_writel(instance->base.dev, synch, instance->preload_reg);
			ME_SPIN_UNLOCK(instance->preload_reg_lock);

			//Default count is '0'
			if (instance->mode == ME4600_AO_CONTINUOUS)
			{//Continous
				instance->preloaded_count = 0;
				instance->circ_buf.tail += count;
				instance->circ_buf.tail &= instance->circ_buf.mask;
			}
			else if ((instance->mode == ME4600_AO_SW_WRAP_MODE) || (instance->mode == ME4600_AO_HW_WRAP_MODE))
			{//Wraparound
				instance->preloaded_count += count;
				instance->data_count += count;

				//Special case: Infinite wraparound with less than FIFO datas always should runs in hardware mode.
				if ((instance->stop_mode == ME4600_AO_INF_STOP_MODE)
						&&
					(circ_buffer_count <= instance->fifo_size))
				{//Change to hardware wraparound
					PDEBUG("Changeing mode from software wraparound to hardware wraparound.\n");
					status |= ME4600_AO_STATUS_BIT_HF;
					//Copy all data
					count = ao_write_data(instance, circ_buffer_count, instance->preloaded_count);
					ctrl &= ~ME4600_AO_CTRL_MODE_MASK;
					ctrl |= ME4600_AO_MODE_WRAPAROUND;
					instance->mode = ME4600_AO_HW_WRAP_MODE;
				}

				if (instance->preloaded_count == me_circ_buf_values(&instance->circ_buf))
				{//Reset position indicator.
					instance->preloaded_count = 0;
				}
				else if (instance->preloaded_count > me_circ_buf_values(&instance->circ_buf))
				{//This should never happend!
					PERROR_CRITICAL("PRELOADED MORE VALUES THAN ARE IN BUFFER!\n");
					err = ME_ERRNO_INTERNAL;
					goto ERROR;
				}
			}

			//Set status to 'wait for start'
			instance->status = ao_status_stream_run_wait;

			me_readl(instance->base.dev, &status, instance->status_reg);
			//Start state machine and interrupts
			ctrl &= ~(ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP);
			if (instance->start_mode & ME4600_AO_EXT_TRIG)
			{// External trigger.
				PINFO("External trigger.\n");
				ctrl |= ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG;
				ctrl |= instance->ctrl_trg & (ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
			}
			if (!(status & ME4600_AO_STATUS_BIT_HF))
			{//More than half!
				if ((ctrl & ME4600_AO_CTRL_MODE_MASK) == ME4600_AO_MODE_CONTINUOUS)
				{//Enable IRQ only when hardware_continous is set and FIFO is more than half
					ctrl &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
					ctrl |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
				}
			}
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			//Trigger output
			if (flags & ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS)
			{//Trigger outputs
				ME_SPIN_LOCK(instance->preload_reg_lock);
					me_readl(instance->base.dev, &synch, instance->preload_reg);
					//Add channel to start list
					me_writel(instance->base.dev, synch | (ME4600_AO_SYNC_HOLD << instance->base.idx), instance->preload_reg);

					//Fire
					PINFO("Fired all software synchronous outputs by software trigger.\n");
					me_writel(instance->base.dev, 0x8000, instance->single_reg);

					//Restore save settings
					me_writel(instance->base.dev, synch, instance->preload_reg);
				ME_SPIN_UNLOCK(instance->preload_reg_lock);
			}
			else if (!instance->start_mode)
			{//Trigger outputs
		/*
				//Remove channel from start list.	// <== Unnecessary. Removed.
				ME_SPIN_LOCK(instance->preload_reg_lock);
					me_readl(instance->base.dev, &synch, instance->preload_reg);
					me_writel(instance->base.dev, synch & ~(ME4600_AO_SYNC_HOLD << instance->base.idx), instance->preload_reg);
		*/
					//Fire
					PINFO("Software trigger.\n");
					me_writel(instance->base.dev, 0x8000, instance->single_reg);

		/*
					//Restore save settings.	// <== Unnecessary. Removed.
					me_writel(instance->base.dev, synch, instance->preload_reg);
				ME_SPIN_UNLOCK(instance->preload_reg_lock);
		*/
			}

			// Set control task's timeout
			ref = jiffies;
			instance->timeout.delay = delay;
			instance->timeout.start_time = ref;

			if ((instance->mode == ME4600_AO_CONTINUOUS) || (instance->mode == ME4600_AO_SW_WRAP_MODE))
			{
				if (status & ME4600_AO_STATUS_BIT_HF)
				{//Less than half but not empty!
					PINFO("Less than half.\n");
					if (instance->stop_data_count == 0)
					{
						count = instance->fifo_size>>1;
					}
					else
					{
						count = ((instance->fifo_size>>1) < instance->stop_data_count) ? instance->fifo_size/2 : instance->stop_data_count;
					}

					//Copy data
					count = ao_write_data(instance, count, instance->preloaded_count);
					if (count < 0)
					{//This should never happend!
						PERROR_CRITICAL("COPY FINISH WITH ERROR!\n");
						err = -count;
						goto ERROR;
					}

					if (instance->mode == ME4600_AO_CONTINUOUS)
					{//Continous
						instance->circ_buf.tail += count;
						instance->circ_buf.tail &= instance->circ_buf.mask;
					}
					else if (instance->mode == ME4600_AO_SW_WRAP_MODE)
					{//Wraparound
						instance->data_count += count;
						instance->preloaded_count += count;

						if (instance->preloaded_count == me_circ_buf_values(&instance->circ_buf))
						{//Reset position indicator.
							instance->preloaded_count = 0;
						}
						else if (instance->preloaded_count > me_circ_buf_values(&instance->circ_buf))
						{//This should never happend!
							PERROR_CRITICAL("PRELOADED MORE VALUES THAN ARE IN BUFFER!\n");
							err = ME_ERRNO_INTERNAL;
							goto ERROR;
						}
					}

					me_readl(instance->base.dev, &status, instance->status_reg);
					if (!(status & ME4600_AO_STATUS_BIT_HF))
					{//More than half!
						me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
						ctrl &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
						ctrl |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
						me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
					}
				}

				//Special case: Limited wraparound with less than HALF FIFO datas need work around to generate first interrupt.
				if ((instance->stop_mode != ME4600_AO_INF_STOP_MODE)
						&&
					(instance->mode == ME4600_AO_SW_WRAP_MODE)
						&&
					(circ_buffer_count <= (instance->fifo_size/2)))
				{//Put more data to FIFO
					PINFO("Limited wraparound with less than HALF FIFO datas.\n");
					if (instance->preloaded_count)
					{//This should never happend!
						PERROR_CRITICAL("ERROR WHEN LOADING VALUES FOR WRAPAROUND!\n");
						err = ME_ERRNO_INTERNAL;
						goto ERROR;
					}

					while(instance->stop_data_count > instance->data_count)
					{//Maximum data not set jet.
						//Copy to buffer
						if(circ_buffer_count != ao_write_data_wraparound(instance, circ_buffer_count, 0))
						{//This should never happend!
							PERROR_CRITICAL("ERROR WHEN LOADING VALUES FOR WRAPAROUND!\n");
							err = ME_ERRNO_INTERNAL;
							goto ERROR;
						}
						instance->data_count += circ_buffer_count;

						me_readl(instance->base.dev, &status, instance->status_reg);
						if (!(status & ME4600_AO_STATUS_BIT_HF))
						{//FIFO is more than half. Enable IRQ and end copy.
							me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
							ctrl &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
							ctrl |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
							me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
							break;
						}
					}
				}
			}

			// Schedule control task.
			PDEBUG("Schedule control task.\n");
			atomic_set(&instance->ao_control_task_flag, 1);
			queue_delayed_work(instance->me4600_workqueue, &instance->ao_control_task, 1);

			if (start_mode == ME_START_MODE_BLOCKING)
			{//Wait for start.
				ME_UNLOCK_PROTECTOR;
				//Only runing process will interrupt this call. Events are signaled when status change. Extra timeout add for safe reason.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(instance->status != ao_status_stream_run_wait),
					delay + 1);
				ME_LOCK_PROTECTOR;

				if (signal_pending(current))
				{
					PERROR("Interrupted by signal.\n");
					instance->status = ao_status_none;
					err = ME_ERRNO_SIGNAL;
				}
				else
				{
					if (instance->status == ao_status_stream_run_wait)
					{
						PERROR("Timeout reached.\n");
						ao_stop_immediately(instance);
						instance->status = ao_status_stream_timeout;
						err = ME_ERRNO_TIMEOUT;
					}

					if (instance->status == ao_status_stream_timeout)
					{
						PDEBUG("Timeout reached.\n");
						err = ME_ERRNO_TIMEOUT;
					}

					if (instance->status == ao_status_none)
					{
						PDEBUG("Starting stream canceled. %d\n", instance->status);
						err = ME_ERRNO_CANCELLED;
					}
				}
			}
ERROR:
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;
	return err;
}

int me4600_ao_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags)
{
	me4600_ao_subdevice_t* instance;
	uint32_t tmp;
	int old_count;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (!instance->fifo)
	{
		PERROR("Not a streaming ao.\n");
		return ME_ERRNO_NOT_SUPPORTED;
	}

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_STATUS_NO_FLAGS.\n");
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
			PERROR("Invalid wait argument specified. Should be ME_WAIT_BUSY or ME_WAIT_IDLE.\n");
			*status = ME_STATUS_INVALID;
			*values = 0;
			return ME_ERRNO_INVALID_WAIT;
	}

	ME_SUBDEVICE_ENTER;

	switch (instance->status)
	{
		case ao_status_single_configured:
		case ao_status_single_end:
		case ao_status_stream_configured:
		case ao_status_stream_end:
		case ao_status_stream_fifo_error:
		case ao_status_stream_buffer_error:
		case ao_status_stream_timeout:
			*status = ME_STATUS_IDLE;
			break;

		case ao_status_single_run:
		case ao_status_stream_run_wait:
		case ao_status_stream_run:
		case ao_status_stream_end_wait:
			*status = ME_STATUS_BUSY;
			break;

		case ao_status_none:
		default:
			me_readl(instance->base.dev, &tmp, instance->status_reg);
			*status = (tmp & ME4600_AO_STATUS_BIT_FSM) ? ME_STATUS_BUSY : ME_STATUS_IDLE;
			break;
	}

	if ((wait == ME_WAIT_IDLE) && (*status == ME_STATUS_BUSY))
	{
		//Only runing process will interrupt this call. Events are signaled when status change. Extra timeout add for safe reason.
		wait_event_interruptible(
			instance->wait_queue,
			(
				(instance->status != ao_status_single_run)
				&&
				(instance->status != ao_status_stream_run_wait)
				&&
				(instance->status != ao_status_stream_run)
				&&
				(instance->status != ao_status_stream_end_wait)
			));

		if (instance->status != ao_status_stream_end)
		{
			PDEBUG("Wait for IDLE canceled. %d\n", instance->status);
			err = ME_ERRNO_CANCELLED;
		}

		if (signal_pending(current))
		{
			PERROR("Interrupted by signal.\n");
			instance->status = ao_status_none;
			err = ME_ERRNO_SIGNAL;
		}

		*status = ME_STATUS_IDLE;
	}
	else if ((wait == ME_WAIT_BUSY) && (*status == ME_STATUS_IDLE))
	{
		// Only runing process will interrupt this call. Events are signaled when status change. Extra timeout add for safe reason.
		wait_event_interruptible(
			instance->wait_queue,
			(
				(instance->status == ao_status_single_run)
				||
				(instance->status == ao_status_stream_run_wait)
				||
				(instance->status == ao_status_stream_run)
				||
				(instance->status == ao_status_stream_end_wait)
				||
				(instance->status == ao_status_none)
			));

		if (instance->status == ao_status_none)
		{
			PDEBUG("Wait for BUSY canceled. %d\n", instance->status);
			err = ME_ERRNO_CANCELLED;
		}

		if (signal_pending(current))
		{
			PERROR("Interrupted by signal.\n");
			err = ME_ERRNO_SIGNAL;
		}

		*status = ME_STATUS_BUSY;
	}
	else if (wait == ME_WAIT_START)
	{
		old_count = (*values) ? *values : instance->stream_start_count;
		// Only runing process will interrupt this call. Events are signaled when status change.
		if ((old_count == instance->stream_start_count) || (instance->status == ao_status_none))
		{
			wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_start_count) || (instance->status == ao_status_none)));
		}

		*status = ME_STATUS_BUSY;

		if (instance->status == ao_status_none)
		{
			PDEBUG("Wait for START canceled. %d\n", instance->status);
			err = ME_ERRNO_CANCELLED;
			*status = ME_STATUS_IDLE;
		}

		if (signal_pending(current))
		{
			PERROR("Interrupted by signal.\n");
			err = ME_ERRNO_SIGNAL;
			*status = ME_STATUS_IDLE;
		}
	}
	else if (wait == ME_WAIT_STOP)
	{
		old_count = (*values) ? *values : instance->stream_stop_count;
		// Only runing process will interrupt this call. Events are signaled when status change.
		if ((old_count == instance->stream_stop_count) || (instance->status == ao_status_none))
		{
			wait_event_interruptible(instance->wait_queue,((old_count != instance->stream_stop_count) || (instance->status == ao_status_none)));
		}

		if (instance->status == ao_status_none)
		{
			PDEBUG("Wait for STOP canceled. %d\n", instance->status);
			err = ME_ERRNO_CANCELLED;
		}

		if (signal_pending(current))
		{
			PERROR("Interrupted by signal.\n");
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
			*values = me_circ_buf_space(&instance->circ_buf);
			PDEBUG("me_circ_buf_space(&instance->circ_buf)=%d.\n", *values);
	}

	ME_SUBDEVICE_EXIT;

	return err;
}

///@todo Implement NONBLOCKING mode for stream_stop
int me4600_ao_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags)
{// Stop work and empty buffer and FIFO
	me4600_ao_subdevice_t* instance;
	uint32_t tmp;
	unsigned long int delay = LONG_MAX - 2;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (flags & ~ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS)
	{
		PERROR("Invalid flag specified. Should be ME_IO_STREAM_STOP_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((stop_mode != ME_STOP_MODE_IMMEDIATE) && (stop_mode != ME_STOP_MODE_LAST_VALUE))
	{
		PERROR("Invalid stop mode specified. Must be ME_STOP_MODE_IMMEDIATE or ME_STOP_MODE_LAST_VALUE.\n");
		return ME_ERRNO_INVALID_STOP_MODE;
	}

	if (!instance->fifo)
	{
		PERROR("Not a streaming ao. instance->fifo=%x\n", instance->fifo);
		return ME_ERRNO_NOT_SUPPORTED;
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
		ME_LOCK_PROTECTOR;
			switch (instance->status)
			{//Checking actual mode.
				case ao_status_single_run:
					PDEBUG("Subdevice is working in single mode.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

					//The device is in wrong mode.
					PERROR("Subdevice isn't in stream mode.\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					goto ERROR;
					break;


				case ao_status_stream_end:
				case ao_status_stream_timeout:
				case ao_status_stream_fifo_error:
				case ao_status_stream_buffer_error:
					instance->status = ao_status_stream_end;
					goto END;
					break;

				case ao_status_stream_run_wait:
				case ao_status_stream_run:
				case ao_status_stream_end_wait:
					//Correct modes!
					break;

				case ao_status_none:
				case ao_status_stream_configured:
				case ao_status_single_end:
				case ao_status_single_error:
				case ao_status_single_configured:
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					instance->status = ao_status_none;
					goto ERROR;
			}

			// ME_STOP_MODE_LAST_VALUE has meaning only if device is working.
			if (instance->status != ao_status_stream_run)
			{
				stop_mode = ME_STOP_MODE_IMMEDIATE;
			}

			//Mark as stopping. => Software stop.
			instance->status = ao_status_stream_end_wait;

			if (stop_mode == ME_STOP_MODE_IMMEDIATE)
			{//Stopped now!
				ao_stop_immediately(instance);

				//Mark as stopped.
				instance->status = ao_status_stream_end;
			}
			else if (stop_mode == ME_STOP_MODE_LAST_VALUE)
			{
				me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
				if ((tmp & ME4600_AO_CTRL_MODE_MASK) == ME4600_AO_MODE_WRAPAROUND)
				{//Hardware wraparound => Hardware stop.
					me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
					tmp |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
					tmp &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);
				}

				ME_UNLOCK_PROTECTOR;
				//Only runing process will interrupt this call.
				wait_event_interruptible_timeout(
					instance->wait_queue,
					(instance->status != ao_status_stream_end_wait),
					delay);
				ME_LOCK_PROTECTOR;

				if (signal_pending(current))
				{
					PERROR("Interrupted by signal.\n");
					instance->status = ao_status_none;
					err = ME_ERRNO_SIGNAL;
				}
				else
				{
					if (instance->status == ao_status_stream_end_wait)
					{
						PDEBUG("Timeout reached.\n");
						err = ME_ERRNO_TIMEOUT;
					}

					if (instance->status == ao_status_none)
					{
						PDEBUG("Stopping stream canceled.\n");
						err = ME_ERRNO_CANCELLED;
					}
				}
			}

END:
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
			tmp &= ~(ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
			tmp &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
			if (!flags)
			{//Reset FIFO
				tmp &= ~ME4600_AO_CTRL_BIT_ENABLE_FIFO;
			}
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);

			if (!flags)
			{//Reset software buffer
				instance->circ_buf.head = 0;
				instance->circ_buf.tail = 0;
				instance->preloaded_count = 0;
				instance->data_count = 0;
			}

			instance->stream_stop_count++;
ERROR:
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_ao_io_stream_write(me_subdevice_t* subdevice, struct file* filep, int write_mode, int* values, int* count, int time_out, int flags)
{
	me4600_ao_subdevice_t* instance;
	uint32_t tmp;
	long j_timeout = 0;	// Timeout in jiffies
	long j_timeout_left = 0; // Timeout (in jiffies) that left after pre-load.
	long j_start;
	int err = ME_ERRNO_SUCCESS;

	int copied_from_user = 0;
	int left_to_copy_from_user=*count;

	int copied_values;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	//Checking arguments
	if (!instance->fifo)
	{
		PERROR("Not a streaming ao.\n");
		return ME_ERRNO_NOT_SUPPORTED;
	}

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_WRITE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (*count < 0)
	{
		PERROR("Invalid count of values specified.\n");
		return ME_ERRNO_INVALID_VALUE_COUNT;
	}

	if (*count == 0)
	{
  		//You get what you want! Nothing more or less.
		return ME_ERRNO_SUCCESS;
	}

	if (!values)
	{
		PERROR("Invalid address of values specified.\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if ((instance->status == ao_status_none) || (instance->status == ao_status_single_configured))
	{//The device is in single mode.
		PERROR("Subdevice must be preinitialize correctly for streaming.\n");
		return ME_ERRNO_PREVIOUS_CONFIG;
	}

/// @note If no 'pre-load' is used. stream_start() will move data to FIFO.
	switch (write_mode)
	{
		case ME_WRITE_MODE_PRELOAD:
			//Device must be stopped.
			if ((instance->status != ao_status_stream_configured) && (instance->status != ao_status_stream_end))
			{
				PERROR("Subdevice mustn't be runing when 'pre-load' mode is used.\n");
				return ME_ERRNO_PREVIOUS_CONFIG;
			}
			break;
		case ME_WRITE_MODE_NONBLOCKING:
		case ME_WRITE_MODE_BLOCKING:
			/// @note In blocking mode: When device is not runing and there is not enought space call will blocked up!
			/// @note Some other thread must empty buffer by starting engine.
			break;

		default:
			PERROR("Invalid write mode specified. Must be ME_WRITE_MODE_PRELOAD, ME_WRITE_MODE_BLOCKING or ME_WRITE_MODE_NONBLOCKING.\n");
			return ME_ERRNO_INVALID_WRITE_MODE;
	}

	if (instance->mode & ME4600_AO_WRAP_MODE)
	{//Wraparound mode. Device must be stopped.
		if ((instance->status != ao_status_stream_configured) && (instance->status != ao_status_stream_end))
		{
			PERROR("Subdevice mustn't be runing when 'pre-load' mode is used.\n");
			return ME_ERRNO_INVALID_WRITE_MODE;
		}
	}

	if ((instance->mode == ME4600_AO_HW_WRAP_MODE) && (write_mode != ME_WRITE_MODE_PRELOAD))
	{ //This is transparent for user.
		PDEBUG("Changing write_mode to ME_WRITE_MODE_PRELOAD.\n");
		write_mode = ME_WRITE_MODE_PRELOAD;
	}

	j_timeout = (time_out > 0) ? (time_out * HZ) / 1000 : LONG_MAX - 2;
	j_start = jiffies;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			if (write_mode == ME_WRITE_MODE_PRELOAD)
			{//Init enviroment - preload
				me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
				//Check FIFO
				if (!(tmp & ME4600_AO_CTRL_BIT_ENABLE_FIFO))
				{//FIFO not active. Enable it.
					tmp |= ME4600_AO_CTRL_BIT_ENABLE_FIFO;
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);
					instance->preloaded_count = 0;
				}
			}

			while(!err)
			{
				//Copy to buffer. This step is common for all modes.
				copied_from_user = ao_get_data_from_user(instance, left_to_copy_from_user, values + (*count - left_to_copy_from_user));
				if (copied_from_user<0)
				{
					err = -copied_from_user;
					break;
				}
				left_to_copy_from_user -= copied_from_user;

				me_readl(instance->base.dev, &tmp, instance->status_reg);
				if ((instance->status == ao_status_stream_run) && !(tmp & ME4600_AO_STATUS_BIT_FSM))
				{//BROKEN PIPE! The state machine is stoped but logical status show that should be working.
					PERROR("Broken pipe in write.\n");
					err = ME_ERRNO_SUBDEVICE_NOT_RUNNING;
					break;
				}

				if ((instance->status == ao_status_stream_run) && (instance->mode == ME4600_AO_CONTINUOUS)  && (tmp & ME4600_AO_STATUS_BIT_HF))
				{//Continous mode runing and data are below half!
					// Block interrupts.
					me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
					//tmp &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
					tmp |= ME4600_AO_CTRL_BIT_RESET_IRQ;
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);

					//Fast copy
					copied_values = ao_write_data(instance, instance->fifo_size/2, 0);
					if (copied_values > 0)
					{
						instance->circ_buf.tail += copied_values;
						instance->circ_buf.tail &= instance->circ_buf.mask;
						continue;
					}

					// Activate interrupts.
					me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
					//tmp |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
					tmp &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);

					if (copied_values == 0)
					{//This was checked and never should happend!
						PERROR_CRITICAL("COPING FINISH WITH 0!\n");
						break;
					}

					if (copied_values < 0)
					{//This was checked and never should happend!
						PERROR_CRITICAL("COPING FINISH WITH AN ERROR!\n");
						instance->status = ao_status_stream_fifo_error;
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
						break;
					}
				}

				if (!left_to_copy_from_user)
				{//All datas were copied.
					break;
				}
				else
				{//Not all datas were copied.
					if (instance->mode & ME4600_AO_WRAP_MODE)
					{//Error too much datas! Wraparound is limited in size!
						PERROR("Too much data for wraparound mode!  Exceeded size of %ld.\n", ME4600_AO_CIRC_BUF_COUNT-1);
						err = ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
						break;
					}

					if (write_mode != ME_WRITE_MODE_BLOCKING)
					{//Non blocking calls
						break;
					}

					j_timeout_left = j_timeout - (jiffies - j_start);
					if (j_timeout_left <= 0)
						j_timeout_left = 1;

					ME_UNLOCK_PROTECTOR;
					wait_event_interruptible_timeout(instance->wait_queue,
													me_circ_buf_space(&instance->circ_buf)
													||
													(
														(instance->status < ao_status_stream_configured)
														||
														(instance->status > ao_status_stream_run)
													),
													j_timeout_left);

					ME_LOCK_PROTECTOR;

					if (signal_pending(current))
					{
						PERROR("Interrupted by signal.\n");
						err = ME_ERRNO_SIGNAL;
						break;
					}

					switch (instance->status)
					{
						case ao_status_none:
							//Reset
							PERROR("Writing interrupted by reset.\n");
							err = ME_ERRNO_CANCELLED;
							break;

						case ao_status_stream_end_wait:
						case ao_status_stream_end:
							PDEBUG("The end. Stream is not working.\n");
							err = ME_ERRNO_CANCELLED;
							break;

						case ao_status_stream_fifo_error:
							PERROR("Writing failed. FIFO underflow\n");
							err = ME_ERRNO_HARDWARE_BUFFER_UNDERFLOW;
							break;

						case ao_status_stream_buffer_error:
							PERROR("Writing failed. Buffer underflow\n");
							err = ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
							break;

						case ao_status_stream_timeout:
							PERROR("Start failed.\n");
							err = ME_ERRNO_CANCELLED;
							break;

						case ao_status_stream_run:
						case ao_status_stream_configured:
						case ao_status_stream_run_wait:
							// Normal conditions.
							if ((jiffies - j_start) >= j_timeout)
							{// Check timeout.
								PERROR("Wait on values timed out.\n");
								err = ME_ERRNO_TIMEOUT;
							}
							break;

						default:
							// Not in normal use.
							PERROR_CRITICAL("Writing breaked with status %d\n", instance->status);
							err = ME_ERRNO_INTERNAL;
					}
				}
			}

			if (!err)
			{
				if (write_mode == ME_WRITE_MODE_PRELOAD)
				{//Copy data to FIFO - preload
					copied_values = ao_write_data_pooling(instance, instance->fifo_size, instance->preloaded_count);
					instance->preloaded_count += copied_values;
					instance->data_count +=  copied_values;

					if ((instance->mode == ME4600_AO_HW_WRAP_MODE) && (me_circ_buf_values(&instance->circ_buf) > instance->fifo_size))
					{
						PERROR("Too much data for hardware wraparound mode! Exceeded size of %d.\n", instance->fifo_size);
						err = ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
					}
				}
			}

			*count = *count - left_to_copy_from_user;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_ao_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{
	me4600_ao_subdevice_t *instance = (me4600_ao_subdevice_t *)subdevice;
	uint32_t ctrl;
	uint32_t status;
	int count = 0;
	int signal_irq = 0;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed. idx=%d\n", instance->base.idx);

		if (!instance->circ_buf.buf)
		{
			instance->status = ao_status_stream_buffer_error;
			instance->stream_stop_count++;
			PERROR_CRITICAL("CIRCULAR BUFFER NOT EXISTS!\n");
			//Block interrupts. Stop machine.
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
			ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
			ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_STOP;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			instance->status = ao_status_stream_buffer_error;
			signal_irq = 1;
			goto ERROR;
		}

#ifdef MEDEBUG_INFO
		PINFO("ISR: Buffer count: %d.(T:%d H:%d)\n", me_circ_buf_values(&instance->circ_buf), instance->circ_buf.tail, instance->circ_buf.head);
		PINFO("ISR: Stop count: %d.\n", instance->stop_count);
		PINFO("ISR: Stop data count: %d.\n", instance->stop_data_count);
		PINFO("ISR: Data count: %d.\n", instance->data_count);
#endif

		me_readl(instance->base.dev, &status, instance->status_reg);
		if (!(status & ME4600_AO_STATUS_BIT_FSM))
		{//Too late. Not working! END? BROKEN PIPE?
			PERROR("Interrupt come but ISM is not working!\n");
			//Block interrupts. Stop machine.
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
			ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
			ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ | ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
			goto ERROR;
		}

		// General procedure. Process more datas.
#ifdef MEDEBUG_DEBUG
		if (!me_circ_buf_values(&instance->circ_buf))
		{//Buffer is empty!
			PDEBUG("Circular buffer empty!\n");
		}
#endif

		//Check FIFO
		if (status & ME4600_AO_STATUS_BIT_HF)
		{//OK less than half
			//Block interrupts
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
			ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_IRQ;
			ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

			do
			{
				//Calculate how many should be copied.
				count = (instance->stop_data_count) ? instance->stop_data_count - instance->data_count : instance->fifo_size/2;
				if ((instance->fifo_size>>1) < count)
				{
					count = instance->fifo_size>>1;
				}
				PDEBUG("Less than half in FIFO. Copy %d values.\n", count);

				//Copy data
				if (instance->mode == ME4600_AO_CONTINUOUS)
				{//Continous
					count = ao_write_data(instance, count, 0);
					if (count > 0)
					{
						instance->circ_buf.tail += count;
						instance->circ_buf.tail &= instance->circ_buf.mask;
						instance->data_count += count;

						if ((instance->status == ao_status_stream_end_wait) && !me_circ_buf_values(&instance->circ_buf))
						{//Stoping. Whole buffer was copied.
							break;
						}
					}
				}
				else if ((instance->mode == ME4600_AO_SW_WRAP_MODE) && ((ctrl & ME4600_AO_CTRL_MODE_MASK) == ME4600_AO_MODE_CONTINUOUS))
				{//Wraparound (software)
					if (instance->status == ao_status_stream_end_wait)
					{//We stoping => Copy to the end of the buffer.
						count = ao_write_data(instance, count, 0);
					}
					else
					{//Copy in wraparound mode.
						count = ao_write_data_wraparound(instance, count, instance->preloaded_count);
					}

					if (count > 0)
					{
						instance->data_count += count;
						instance->preloaded_count += count;
						instance->preloaded_count %= me_circ_buf_values(&instance->circ_buf);

						if ((instance->status == ao_status_stream_end_wait) && !instance->preloaded_count)
						{//Stoping. Whole buffer was copied.
							break;
						}
					}
				}

				if ((count <= 0) || (instance->stop_data_count && (instance->stop_data_count <= instance->data_count)))
				{//End of work.
					break;
				}
			}//Repeat if is still under half fifo
			while ((status = me4600_ao_FSM_test(instance)) & ME4600_AO_STATUS_BIT_HF);

			//Unblock interrupts
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);

			if (count >= 0)
			{//Copy was successful.
				if (instance->stop_data_count && (instance->stop_data_count <= instance->data_count))
				{//Finishing work. No more interrupts.
					PDEBUG("Finishing work. Interrupt disabled.\n");
					instance->status = ao_status_stream_end_wait;
				}
				else if (count > 0)
				{//Normal work. Enable interrupt.
					PDEBUG("Normal work. Enable interrupt.\n");
					ctrl &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
					ctrl |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
				}
				else
				{//Normal work but there are no more data in buffer. Interrupt active but blocked. stream_write() will unblock it.
					PDEBUG("No data in software buffer. Interrupt blocked.\n");
					ctrl |= ME4600_AO_CTRL_BIT_ENABLE_IRQ;
				}
			}
			else
			{//Error during copy.
				instance->status = ao_status_stream_fifo_error;
				instance->stream_stop_count++;
			}
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		}
		else
		{//?? more than half
			PDEBUG("Interrupt come but FIFO more than half full! Reset interrupt.\n");
			//Reset pending interrupt
			me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
			ctrl |= ME4600_AO_CTRL_BIT_RESET_IRQ;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
			ctrl &= ~ME4600_AO_CTRL_BIT_RESET_IRQ;
			me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		}

		signal_irq = 1;

#ifdef MEDEBUG_INFO
		PINFO("ISR: Buffer count: %d.(T:%d H:%d)\n", me_circ_buf_values(&instance->circ_buf), instance->circ_buf.tail, instance->circ_buf.head);
		PINFO("ISR: Stop count: %d.\n", instance->stop_count);
		PINFO("ISR: Stop data count: %d.\n", instance->stop_data_count);
		PINFO("ISR: Data count: %d.\n", instance->data_count);
#endif

ERROR:
	ME_FREE_HANDLER_PROTECTOR;

	if (signal_irq)
	{
		PDEBUG("Signal. me_circ_buf_space(&instance->circ_buf)=%d\n", me_circ_buf_space(&instance->circ_buf));
		//Signal it.
		wake_up_interruptible_all(&instance->wait_queue);
	}

	return ME_ERRNO_SUCCESS;
}

void me4600_ao_destructor(struct me_subdevice* subdevice)
{
	me4600_ao_subdevice_t* instance;

	if (!subdevice)
	{
		PERROR_CRITICAL("NULL pointer to subdevice instance!\n");
		return;
	}

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	atomic_set(&instance->ao_control_task_flag, 0);

	if (((me_general_dev_t *)instance->base.dev)->dev)
	{
		// Reset subdevice to asure clean exit.
		me4600_ao_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
	}

	ME_SUBDEVICE_LOCK;
		// Remove any tasks from work queue. This is paranoic because it was done allready in reset().
		cancel_delayed_work(&instance->ao_control_task);
	ME_SUBDEVICE_UNLOCK;
	//Wait 2 ticks to be sure that control task is removed from queue.
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(2);

	if(instance->fifo)
	{
		if(instance->circ_buf.buf)
		{
			PDEBUG("free circ_buf = %p size=%d\n", instance->circ_buf.buf, PAGE_SHIFT<<ME4600_AO_CIRC_BUF_SIZE_ORDER);
			free_pages((unsigned long)instance->circ_buf.buf, ME4600_AO_CIRC_BUF_SIZE_ORDER);
		}
		instance->circ_buf.buf = NULL;
	}

	me_subdevice_deinit(subdevice);
}

/** @brief Stop presentation. Preserve FIFOs.
*
* @param instance The subdevice instance (pointer).
*/
int inline ao_stop_immediately(me4600_ao_subdevice_t *instance)
{
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	// Stop all actions. No conditions! Block interrupts. Leave FIFO untouched!
	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	tmp |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
	tmp &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);

	return ME_ERRNO_SUCCESS;
}

/** @brief Copy data from circular buffer to fifo (fast) in wraparound.
* @note This is time critical function. Checking is done at begining and end only.
* @note The is not reasonable way to check how many walues was in FIFO at begining. The count must be managed externaly.
*
* @param instance The subdevice instance (pointer).
* @param count Maximum number of copied data.
* @param start_pos Position of the firs value in buffer.
*
* @return On success: Number of copied data.
* @return On error/success: 0.	No datas were copied => no data in buffer.
* @return On error: -ME_ERRNO_HARDWARE_BUFFER_OVERFLOW.
*/
int inline ao_write_data_wraparound(me4600_ao_subdevice_t* instance, int count, int start_pos)
#if defined(ME_USB) && defined(ME_USE_DMA)
{
	uint32_t status;
	uint32_t value;
	int pos = (instance->circ_buf.tail + start_pos) & instance->circ_buf.mask;
	int local_count = count;
	int i = 0;
	int err;
	uint32_t ctrl;

	uint32_t* buffer;

	if (count <= 0)
	{//Wrong count!
		PERROR("No data in buffer!\n");
		return 0;
	}

	buffer = (uint32_t *)kmalloc(local_count * sizeof(uint32_t), GFP_KERNEL);
	if (!buffer)
		return -ME_ERRNO_INTERNAL;

	while(i < local_count)
	{
		//Get value from buffer
		value = *(instance->circ_buf.buf + pos);
		//Prepare it. Put value to local buffer.
// 		value &= 0xFFFF;
		buffer[i] = value | (value << 16);

		pos++;
		pos &= instance->circ_buf.mask;
		if (pos == instance->circ_buf.head)
		{
			pos = instance->circ_buf.tail;
		}
		i++;
	}

	// Block registry - Enable DMA
	err = me_DMA_lock(instance->base.dev, instance->ctrl_reg, &ctrl);
	if (!err)
	{
		if (me_DMA_write(instance->base.dev, buffer, local_count-1, instance->DMA_base))
		{
			PERROR("me_DMA_write error\n");
			local_count = 0;
		}
		// Unblock registry - Disable DMA
		err = me_DMA_unlock(instance->base.dev, instance->ctrl_reg, ctrl);
		if (err)
		{
			PERROR("me_DMA_unlock error=%d\n", err);
		}
	}
	else
	{
		PERROR("me_DMA_lock error=%d\n", err);
		local_count = 0;
	}


	if (local_count && !err)
	{
		me_readl(instance->base.dev, &status, instance->status_reg);
		if (!(status & ME4600_AO_STATUS_BIT_FF))
		{//FIFO is full before all datas were copied!
			PERROR("idx=%d FIFO is full before all datas were copied!\n", instance->base.idx);
			err = -ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
		}
		else
		{//Add last value. Put value to FIFO
			me_writel(instance->base.dev, buffer[local_count-1], instance->fifo_reg);
		}
	}

	if (err > 0)
	{
		err = -ME_ERRNO_INTERNAL;
	}

	PINFO("idx=%d WRAPAROUND LOADED %d values\n", instance->base.idx, local_count);
	kfree(buffer);

	return (err) ? err : local_count;
}
#else
{/// @note This is time critical function!
	uint32_t status;
	uint32_t value;
	int pos = (instance->circ_buf.tail + start_pos) & instance->circ_buf.mask;
	int local_count = count;
	int i = 1;

	if (count <= 0)
	{//Wrong count!
		PERROR("No data in buffer!\n");
		return 0;
	}

	while(i < local_count)
	{
		//Get value from buffer
		value = *(instance->circ_buf.buf + pos);
		//Prepare it
// 		value &= 0xFFFF;

		//Put value to FIFO
		me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);

		pos++;
		pos &= instance->circ_buf.mask;
		if (pos == instance->circ_buf.head)
		{
			pos = instance->circ_buf.tail;
		}
		i++;
	}

 	me_readl(instance->base.dev, &status, instance->status_reg);
	if (!(status & ME4600_AO_STATUS_BIT_FF))
	{//FIFO is full before all datas were copied!
		PERROR("FIFO was full before all datas were copied! idx=%d\n", instance->base.idx);
		return -ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
	}
	else
	{//Add last value
		value = *(instance->circ_buf.buf + pos);
		// Prepare it.
// 		value &= 0xFFFF;

		//Put value to FIFO
		me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);
	}

	PINFO("WRAPAROUND LOADED %d values. idx=%d\n", local_count, instance->base.idx);
	return local_count;
}
#endif

/** @brief Copy data from software buffer to fifo (fast).
* @note This is time critical function. Checking is done at begining and end only.
* @note PCI version do checking at begining and end only.
* @note USB version doesn't check fifo overflow condition.
* @note The is not reasonable way to check how many walues was in FIFO at begining. The count must be managed externaly.
*
* @param instance The subdevice instance (pointer).
* @param count Maximum number of copied data.
* @param start_pos Position of the firs value in buffer.
*
* @return On success: Number of copied data.
* @return On error/success: 0.	No datas were copied => no data in buffer.
* @return On error: -ME_ERRNO_HARDWARE_BUFFER_OVERFLOW.
*/
int inline ao_write_data(me4600_ao_subdevice_t* instance, int count, int start_pos)
#if defined(ME_USB) && defined(ME_USE_DMA)
{
	uint32_t value;
	int pos = (instance->circ_buf.tail + start_pos) & instance->circ_buf.mask;
	int local_count = count;
	int max_count;
	int i = 0;
	int err;
	uint32_t ctrl;

	uint32_t* buffer;

	if (count <= 0)
	{//Wrong count!
		return 0;
	}

	max_count = me_circ_buf_values(&instance->circ_buf) - start_pos;
	if (max_count <= 0)
	{//No data to copy!
		PINFO("No data in buffer!\n");
		return 0;
	}

	if (max_count < count)
	{
		local_count = max_count;
	}

	buffer = (uint32_t *)kmalloc(local_count * sizeof(uint32_t), GFP_KERNEL);
	if (!buffer)
		return -ME_ERRNO_INTERNAL;

	while(i < local_count)
	{
		//Get value from buffer
		value = *(instance->circ_buf.buf + pos);
		//Prepare it
// 		value &= 0xFFFF;
		//Put value to local buffer
		buffer[i] = value | (value << 16);

		pos++;
		pos &= instance->circ_buf.mask;
		i++;
	}

	// Block registry - Enable DMA
	err = me_DMA_lock(instance->base.dev, instance->ctrl_reg, &ctrl);
	if (!err)
	{
		if (me_DMA_write(instance->base.dev, buffer, local_count, instance->DMA_base))
		{
			PERROR("me_DMA_write error\n");
			local_count = 0;
		}
		// Unblock registry - Disable DMA
		err = me_DMA_unlock(instance->base.dev, instance->ctrl_reg, ctrl);
		if (err)
		{
			PERROR("me_DMA_unlock error=%d\n", err);
		}
	}
	else
	{
		PERROR("me_DMA_lock error=%d\n", err);
		local_count = 0;
	}

	PINFO("idx=%d FAST: UPLOADED %d values\n", instance->base.idx, local_count);
	kfree(buffer);

	return (err) ? -ME_ERRNO_INTERNAL : local_count;
}
#else
{/// @note This is time critical function!
	uint32_t status;
	uint32_t value;
	int pos = (instance->circ_buf.tail + start_pos) & instance->circ_buf.mask;
	int local_count = count;
	int max_count;
	int i = 1;

	if (count <= 0)
	{//Wrong count!
		return 0;
	}

	max_count = me_circ_buf_values(&instance->circ_buf) - start_pos;
	if (max_count <= 0)
	{//No data to copy!
		PINFO("No data in buffer!\n");
		return 0;
	}

	if (max_count < count)
	{
		local_count = max_count;
	}

	while(i < local_count)
	{
		//Get value from buffer
		value = *(instance->circ_buf.buf + pos);
		//Prepare it
// 		value &= 0xFFFF;

		//Put value to FIFO
		me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);

		pos++;
		pos &= instance->circ_buf.mask;
		i++;
	}

 	me_readl(instance->base.dev, &status, instance->status_reg);
	if (!(status & ME4600_AO_STATUS_BIT_FF))
	{//FIFO is full before all datas were copied!
		PERROR("idx=%d FIFO full before all datas were copied!\n", instance->base.idx);
		return -ME_ERRNO_HARDWARE_BUFFER_OVERFLOW;
	}
	else
	{//Add last value
		value = *(instance->circ_buf.buf + pos);
// 		value &= 0xFFFF;

		//Put value to FIFO
		me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);
	}

	PINFO("idx=%d FAST: UPLOADED %d values\n", instance->base.idx, local_count);
	return local_count;
}
#endif

/** @brief Copy data from software buffer to fifo (slow).
* @note This is slow function that copy all data from buffer to FIFO with full control.
*
* @param instance The subdevice instance (pointer).
* @param count Maximum number of copied data.
* @param start_pos Position of the firs value in buffer.
*
* @return On success: Number of copied values.
* @return On error/success: 0.	FIFO was full at begining.
*/
int inline ao_write_data_pooling(me4600_ao_subdevice_t* instance, int count, int start_pos)
{/// @note This is slow function!
	uint32_t status;
	uint32_t value;
	int pos = (instance->circ_buf.tail + start_pos) & instance->circ_buf.mask;
	int local_count = count;
	int i;
	int max_count;

	if (count <= 0)
	{//Wrong count!
		PERROR("SLOW: UPLOADED: Wrong count! idx=%d\n", instance->base.idx);
		return 0;
	}

	max_count = me_circ_buf_values(&instance->circ_buf) - start_pos;
	if (max_count <= 0)
	{//No data to copy!
		PERROR("SLOW: UPLOADED: No data to copy! idx=%d\n", instance->base.idx);
		return 0;
	}

	if (max_count < count)
	{
		local_count = max_count;
	}

	for (i=0; i<local_count; i++)
	{
  		me_readl(instance->base.dev, &status, instance->status_reg);
		if (!(status & ME4600_AO_STATUS_BIT_FF))
		{//FIFO is full!
			return i;
		}

		//Get value from buffer
		value = *(instance->circ_buf.buf + pos);
		//Prepare it
// 		value &= 0xFFFF;

		//Put value to FIFO
		me_writel(instance->base.dev, value | (value << 16), instance->fifo_reg);

		pos++;
		pos &= instance->circ_buf.mask;
	}

	PINFO("SLOW: UPLOADED %d values. idx=%d\n", local_count, instance->base.idx);
	return local_count;
}

/** @brief Copy data from user space to circular buffer.
* @param instance The subdevice instance (pointer).
* @param count Number of datas in user space.
* @param user_values Buffer's pointer.
*
* @return On success: Number of copied values.
* @return On error: -ME_ERRNO_INTERNAL.
*/
int inline ao_get_data_from_user(me4600_ao_subdevice_t* instance, int count, int* user_values)
{
	int i;
	int empty_space;
	int copied;
	int value;
	int err;

	empty_space = me_circ_buf_space(&instance->circ_buf);
	//We have only this space free.
	copied = (count < empty_space) ? count : empty_space;
	for (i = 0; i < copied; i++)
	{//Copy from user to buffer
		if ((err = get_user(value, (int *)(user_values + i))))
		{
			PERROR("BUFFER LOADED: get_user(0x%p) return an error: %d. idx=%d\n", user_values + i, err, instance->base.idx);
			return -ME_ERRNO_INTERNAL;
		}

		/// @note The analog output in me4600 series has size of 16 bits.
		*(instance->circ_buf.buf + instance->circ_buf.head) = (uint16_t) value;
		instance->circ_buf.head++;
		instance->circ_buf.head &= instance->circ_buf.mask;
	}

	PINFO("BUFFER LOADED %d values. idx=%d\n", copied, instance->base.idx);

	return copied;
}

/** @brief Checking actual hardware and logical state.
* @param instance The subdevice instance (pointer).
*/
void me4600_ao_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
									void* subdevice
#else
									struct work_struct* work
#endif
								)
{
	me4600_ao_subdevice_t* instance;
	uint32_t status;
	uint32_t ctrl;
	uint32_t synch;
	int reschedule = 0;
	int signaling = 0;


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	instance = (me4600_ao_subdevice_t *) subdevice;
#else
	instance = container_of((void *)work, me4600_ao_subdevice_t, ao_control_task);
#endif

	if (!instance)
	{
		return;
	}

	if (signal_pending(current))
	{
		PERROR("Control task interrupted.\n");
		instance->status = ao_status_none;
		return;
	}

	if (!atomic_read(&instance->ao_control_task_flag))
	{
		PINFO("Forced quiting detected! (before LOCK)\n");
		return;
	}

#if defined(ME_USB)
	if (instance && instance->base.dev)
	{
		ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
	}

	if (!atomic_read(&instance->ao_control_task_flag))
	{
		goto EXIT_USB;
	}
#endif

	PINFO("<%s: %ld> executed. idx=%d STATUS:%d\n", __FUNCTION__, jiffies, instance->base.idx, instance->status);
	ME_LOCK_PROTECTOR;
		if (!atomic_read(&instance->ao_control_task_flag))
		{
			PINFO("Forced quiting detected! (after LOCK)\n");
			goto EXIT;
		}
		me_readl(instance->base.dev, &status, instance->status_reg);
		switch (instance->status)
		{// Checking actual mode.

			// Not configured for work.
			case ao_status_none:
				break;

			//This are stable modes. No need to do anything. (?)
			case ao_status_single_configured:
			case ao_status_single_end:
			case ao_status_single_error:
			case ao_status_stream_configured:
			case ao_status_stream_end:
			case ao_status_stream_fifo_error:
			case ao_status_stream_buffer_error:
			case ao_status_stream_timeout:
			case ao_status_error:
				break;

			// Single mode
			case ao_status_single_run:
				if (!(status & ME4600_AO_STATUS_BIT_FSM))
				{// State machine is not working.
					if (((instance->fifo) && (!(status & ME4600_AO_STATUS_BIT_EF)))
						||
						(!(instance->fifo)))
					{// Single is in end state.
						PDEBUG("Single call has been complited.\n");

						// Stop all actions. No conditions! Block interrupts and trigger.
						me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
						ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
						ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
						ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
						if (instance->fifo)
						{//Disabling FIFO
							ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_FIFO;
						}
						me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

						// Set correct value for single_read();
						instance->single_value = instance->single_value_in_fifo;

						// Set status as 'ao_status_single_end'
						instance->status = ao_status_single_end;

						// Signal the end.
						signaling = 1;
						break;
					}
				}

				// Check timeout.
				if ((instance->timeout.delay) && ((jiffies - instance->timeout.start_time) >= instance->timeout.delay))
				{// Timeout
					PDEBUG("Timeout reached.\n");
					// Stop all actions. No conditions! Block interrupts and trigger. Leave FIFO untouched!
					me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
					ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
					ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
					ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
					if (instance->fifo)
					{//Disabling FIFO
						ctrl &= ~ME4600_AO_CTRL_BIT_ENABLE_FIFO;
					}
					me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

					ME_SPIN_LOCK(instance->preload_reg_lock);
						//Remove from synchronous start. Block triggering from this output.
						me_readl(instance->base.dev, &synch, instance->preload_reg);
						synch &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
						if (!(instance->fifo))
						{// No FIFO - set to single safe mode
							synch |= ME4600_AO_SYNC_HOLD << instance->base.idx;
						}
						me_writel(instance->base.dev, synch, instance->preload_reg);
					ME_SPIN_UNLOCK(instance->preload_reg_lock);

					// Set correct value for single_read();
					instance->single_value_in_fifo = instance->single_value;

					if (!(instance->fifo))
					{// No FIFO
						// Restore old settings.
						PINFO("Write old value back to register.\n");
						me_writel(instance->base.dev, instance->single_value, instance->single_reg);
					}

					// Set status as 'ao_status_single_error'
					instance->status = ao_status_single_error;

					// Signal the end.
					signaling = 1;

					break;
				}

				// Still waiting.
				reschedule = 1;
				break;

			// Stream modes
			case ao_status_stream_run_wait:
				if (!instance->fifo)
				{// No FIFO
					PERROR_CRITICAL("Streaming on single device! This feature is not implemented in this version!\n");
					instance->status = ao_status_none;
					// Signal the end.
					signaling = 1;
					break;
				}

				if (status & ME4600_AO_STATUS_BIT_FSM)
				{// State machine is working. Waiting for start finish.
					PDEBUG("ISM is working.\n");

					instance->status = ao_status_stream_run;
					instance->stream_start_count++;

					// Signal end of this step
					signaling = 1;

					// Wait for stop.
					reschedule = 1;

					break;
				}
				else
				{// State machine is not working.
					if (!(status & ME4600_AO_STATUS_BIT_EF))
					{// FIFO is empty. Procedure has started and finish already!
						instance->status = ao_status_stream_end;

						// Signal the end.
						signaling = 1;
						break;
					}
				}

				// Check timeout.
				if ((instance->timeout.delay) && ((jiffies - instance->timeout.start_time) >= instance->timeout.delay))
				{// Timeout
					PDEBUG("Timeout reached.\n");
					// Stop all actions. No conditions! Block interrupts. Leave FIFO untouched!
					me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
					ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
					ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
					ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
					me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
					ME_SPIN_LOCK(instance->preload_reg_lock);
						//Remove from synchronous start. Block triggering from this output.
						me_readl(instance->base.dev, &synch, instance->preload_reg);
						synch &= ~((ME4600_AO_SYNC_HOLD | ME4600_AO_SYNC_EXT_TRIG) << instance->base.idx);
						me_writel(instance->base.dev, synch, instance->preload_reg);
					ME_SPIN_UNLOCK(instance->preload_reg_lock);

					instance->status = ao_status_stream_timeout;

					// Signal the end.
					signaling = 1;
					break;
				}

				// Wait for stop.
				reschedule = 1;
				break;

			case ao_status_stream_run:
				if (!instance->fifo)
				{
					PERROR_CRITICAL("Streaming on single device! This feature is not implemented in this version!\n");
					instance->status = ao_status_none;

					// Signal the end.
					signaling = 1;
					break;
				}

				if (!(status & ME4600_AO_STATUS_BIT_FSM))
				{// State machine is not working. This is an error.
					// BROKEN PIPE!
					if (!(status & ME4600_AO_STATUS_BIT_EF))
					{// FIFO is empty.
						if (me_circ_buf_values(&instance->circ_buf))
						{// Software buffer is not empty.
							if (instance->stop_data_count && (instance->stop_data_count <= instance->data_count))
							{//Finishing work. Requed data shown.
								PDEBUG("ISM stoped. No data in FIFO. Buffer is not empty.\n");
								instance->status = ao_status_stream_end;
							}
							else
							{
								PERROR("Output stream has been broken. ISM stoped. No data in FIFO. Buffer is not empty.\n");
								instance->status = ao_status_stream_buffer_error;
							}
						}
						else
						{// Software buffer is empty.
							PDEBUG("ISM stoped. No data in FIFO. Buffer is empty.\n");
							instance->status = ao_status_stream_end;
						}
					}
					else
					{// There are still datas in FIFO.
						if (me_circ_buf_values(&instance->circ_buf))
						{// Software buffer is not empty.
							PERROR("Output stream has been broken. ISM stoped but some data in FIFO and buffer.\n");
						}
						else
						{// Software buffer is empty.
							PERROR("Output stream has been broken. ISM stoped but some data in FIFO. Buffer is empty.\n");
						}
						instance->status = ao_status_stream_fifo_error;
					}

					// Stop all actions. Leave FIFO untouched!
					me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
					ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP;
					ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
					me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

					instance->stream_stop_count++;

					// Signal the failure.
					signaling = 1;
					break;
				}

				// Wait for stop.
				reschedule = 1;
				break;

			case ao_status_stream_end_wait:
				if (!instance->fifo)
				{
					PERROR_CRITICAL("Streaming on single device! This feature is not implemented in this version!\n");
					instance->status = ao_status_none;

					// Signal the end.
					signaling = 1;
					break;
				}

				if (!(status & ME4600_AO_STATUS_BIT_FSM))
				{// State machine is not working. Waiting for stop finish.
					instance->status = ao_status_stream_end;

					// Stop all actions. Leave FIFO untouched!
					me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
					ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP;
					ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
					me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

					instance->stream_stop_count++;

					signaling = 1;

					break;
				}

				// State machine is working.
				reschedule = 1;

				break;

			default:
				PERROR_CRITICAL("Status is in wrong state (%d)!\n", instance->status);
				me_readl(instance->base.dev, &ctrl, instance->ctrl_reg);
				ctrl |= ME4600_AO_CTRL_BIT_STOP | ME4600_AO_CTRL_BIT_IMMEDIATE_STOP | ME4600_AO_CTRL_BIT_RESET_IRQ;
				ctrl &= ~(ME4600_AO_CTRL_BIT_ENABLE_IRQ | ME4600_AO_CTRL_BIT_ENABLE_EX_TRIG);
				ctrl &= ~(ME4600_AO_CTRL_BIT_EX_TRIG_EDGE | ME4600_AO_CTRL_BIT_EX_TRIG_EDGE_BOTH);
				me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
				instance->status = ao_status_none;
				// Signal the end.
				signaling = 1;
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
		PERROR("Control task interrupted. Quiting.\n");
		instance->status = ao_status_none;
		return;
	}

	if (atomic_read(&instance->ao_control_task_flag) && reschedule)
	{// Reschedule task
		PINFO("<%s> Rescheduling control task. idx=%d\n", __FUNCTION__, instance->base.idx);
		queue_delayed_work(instance->me4600_workqueue, &instance->ao_control_task, 1);
	}
	else
	{
		PINFO("<%s> Ending control task. idx=%d\n", __FUNCTION__, instance->base.idx);
	}
	if (signaling)
	{//Signal it.
		PINFO("<%s> Signal event. idx=%d\n", __FUNCTION__, instance->base.idx);
		wake_up_interruptible_all(&instance->wait_queue);
	}
}

int me4600_ao_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{
	me4600_ao_subdevice_t* instance;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (*max < *min)
	{
		PERROR("Invalid minimum and maximum values specified. MIN: %d > MAX: %d\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	if (unit == ME_UNIT_VOLT)
	{
		if ((*max <= (ME4600_AO_MAX_RANGE + 1000)) && (*min >= ME4600_AO_MIN_RANGE))
		{
			*min = ME4600_AO_MIN_RANGE;
			*max = ME4600_AO_MAX_RANGE;
			*maxdata = ME4600_AO_MAX_DATA;
			*range = 0;
		}
		else
		{
			PERROR("No matching range available.\n");
			return ME_ERRNO_NO_RANGE;
		}
	}
	else
	{
		PERROR("Invalid physical unit specified.\n");
		return ME_ERRNO_INVALID_UNIT;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{
	me4600_ao_subdevice_t* instance;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*count = ((unit == ME_UNIT_VOLT) || (unit == ME_UNIT_ANY)) ? 1 : 0;

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	me4600_ao_subdevice_t* instance;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (range == 0)
	{
		*unit = ME_UNIT_VOLT;
		*min = ME4600_AO_MIN_RANGE;
		*max = ME4600_AO_MAX_RANGE;
		*maxdata = ME4600_AO_MAX_DATA;
	}
	else
	{
		PERROR("Invalid range number specified.\n");
		return ME_ERRNO_INVALID_RANGE;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_timer(me_subdevice_t* subdevice,  int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	me4600_ao_subdevice_t* instance;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (instance->fifo)
	{//Streaming device.
		*base_frequency = ME4600_AO_BASE_FREQUENCY;
		if ((timer != ME_TIMER_ACQ_START) && (timer != ME_TIMER_CONV_START))
		{
			*min_ticks = 0;
			*max_ticks = 0;
			PERROR("Invalid timer specified.\n");
			return ME_ERRNO_INVALID_TIMER;
		}

		if (timer == ME_TIMER_ACQ_START)
		{
			*min_ticks = ME4600_AO_MIN_ACQ_TICKS;
			*max_ticks = ME4600_AO_MAX_ACQ_TICKS;
		}
		else if (timer == ME_TIMER_CONV_START)
		{
			*min_ticks = ME4600_AO_MIN_CHAN_TICKS;
			*max_ticks = ME4600_AO_MAX_CHAN_TICKS;
		}
	}
	else
	{//Not streaming device!
		*base_frequency = 0;
		*min_ticks = 0;
		*max_ticks = 0;
	}

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	me4600_ao_subdevice_t* instance;
	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*number = 1;

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_subdevice_type(me_subdevice_t* subdevice, int *type, int *subtype)
{
	me4600_ao_subdevice_t* instance;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*type = ME_TYPE_AO;
	*subtype = (instance->fifo) ? ME_SUBTYPE_STREAMING : ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	me4600_ao_subdevice_t* instance;
	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*caps = ME_CAPS_AO_TRIG_SYNCHRONOUS | ((instance->fifo) ? ME_CAPS_AO_FIFO : ME_CAPS_NONE);

	return ME_ERRNO_SUCCESS;
}

int me4600_ao_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count)
{
	me4600_ao_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (*count < 1)
	{
		PERROR("Invalid capability argument count.\n");
		return ME_ERRNO_INVALID_CAP_ARG_COUNT;
	}

	if (!instance->fifo)
	{
		*count = 0;
		*args = 0;
		PERROR("Single device. Capabilities not supported.\n");
		return ME_ERRNO_INVALID_CAP;
	}

	*count = 1;
	switch (cap)
	{
		case ME_CAP_AO_FIFO_SIZE:
			*args = instance->fifo_size;
		break;

		case ME_CAP_AO_BUFFER_SIZE:
			*args = (instance->circ_buf.buf) ? ME4600_AO_CIRC_BUF_COUNT : 0;
		break;

		case ME_CAP_AO_CHANNEL_LIST_SIZE:
			*args = 1;
		break;

		case ME_CAP_AO_MAX_THRESHOLD_SIZE:
			*args = 0;
		break;

		default:
			*count = 0;
			*args = 0;
			PERROR("Invalid capability.\n");
			err = ME_ERRNO_INVALID_CAP;
	}

	return err;
}

int me4600_ao_postinit(me_subdevice_t* subdevice, void* args)
{
	ao_determine_FIFO_size((me4600_ao_subdevice_t *)subdevice);

	// Reset subdevice.
	return me4600_ao_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
}

me4600_ao_subdevice_t* me4600_ao_constr(void* reg_base,
											void* DMA_base,
											unsigned int idx,
											me_lock_t *preload_reg_lock, uint32_t *preload_flags,
											int fifo, struct workqueue_struct* me4600_wq)
{
	me4600_ao_subdevice_t* subdevice;

	PDEBUG("executed. idx=%d\n", idx);

	if (idx > 3)
	{
		PERROR_CRITICAL("WRONG SUBDEVICE ID=%d!", idx);
		return NULL;
	}

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4600_ao_subdevice_t), GFP_KERNEL);
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

	// Initialize spin locks.
	subdevice->preload_reg_lock = preload_reg_lock;
	subdevice->preload_flags = preload_flags;

	// Store analog output index.
	subdevice->base.idx = idx;

	// Store if analog output has fifo.
	subdevice->fifo = (idx < fifo) ? 1 : 0;
	if(subdevice->fifo)
	{// Allocate and initialize circular buffer.
		subdevice->circ_buf.mask = ME4600_AO_CIRC_BUF_COUNT - 1;
		subdevice->circ_buf.buf = (void *)__get_free_pages(GFP_KERNEL, ME4600_AO_CIRC_BUF_SIZE_ORDER);
		if (!subdevice->circ_buf.buf)
		{
			PERROR("Cannot initialize subdevice base class instance.\n");
			me_subdevice_deinit((me_subdevice_t *) subdevice);
			kfree(subdevice);
			return NULL;
		}
		PINFO("circ_buf = %p size=%ld\n", subdevice->circ_buf.buf, ME4600_AO_CIRC_BUF_SIZE);

		memset(subdevice->circ_buf.buf, 0, ME4600_AO_CIRC_BUF_SIZE);

		subdevice->fifo_size = ME4600_AO_FIFO_COUNT;
	}
	else
	{// No FIFO.
		subdevice->circ_buf.mask = 0;
		subdevice->circ_buf.buf = NULL;

		subdevice->fifo_size = 0;
	}

	subdevice->circ_buf.head = 0;
	subdevice->circ_buf.tail = 0;

	subdevice->status = ao_status_none;
	subdevice->stream_start_count = 0;
	subdevice->stream_stop_count = 0;

	atomic_set(&subdevice->ao_control_task_flag, 0);
	subdevice->timeout.delay = 0;
	subdevice->timeout.start_time = jiffies;

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Initialize single value to 0V.
	subdevice->single_value =  0x8000;
	subdevice->single_value_in_fifo =  0x8000;

	// Initialize registers.
	subdevice->ctrl_reg = reg_base + ME4600_AO_CTRL_REG + (ME4600_AO_PORT_OFFSET * idx);
	subdevice->status_reg = reg_base + ME4600_AO_STATUS_REG + (ME4600_AO_PORT_OFFSET * idx);
	subdevice->fifo_reg = reg_base + ME4600_AO_FIFO_REG + (ME4600_AO_PORT_OFFSET * idx);
	subdevice->single_reg = reg_base + ME4600_AO_SINGLE_REG + (ME4600_AO_PORT_OFFSET * idx);
	subdevice->timer_reg = reg_base + ME4600_AO_TIMER_REG + (ME4600_AO_PORT_OFFSET * idx);
	subdevice->preload_reg = reg_base + ME4600_AO_SYNC_REG;
	subdevice->DMA_base = DMA_base;

	if (idx == 3)
	{
		subdevice->bitpattern = 1;
	}

	// Override base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me4600_ao_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me4600_ao_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me4600_ao_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me4600_ao_io_single_write;
	subdevice->base.me_subdevice_io_stream_config = me4600_ao_io_stream_config;
	subdevice->base.me_subdevice_io_stream_new_values = me4600_ao_io_stream_new_values;
	subdevice->base.me_subdevice_io_stream_write = me4600_ao_io_stream_write;
	subdevice->base.me_subdevice_io_stream_start = me4600_ao_io_stream_start;
	subdevice->base.me_subdevice_io_stream_status = me4600_ao_io_stream_status;
	subdevice->base.me_subdevice_io_stream_stop = me4600_ao_io_stream_stop;
	subdevice->base.me_subdevice_query_number_channels = me4600_ao_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4600_ao_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me4600_ao_query_subdevice_caps;
	subdevice->base.me_subdevice_query_subdevice_caps_args = me4600_ao_query_subdevice_caps_args;
	subdevice->base.me_subdevice_query_range_by_min_max = me4600_ao_query_range_by_min_max;
	subdevice->base.me_subdevice_query_number_ranges = me4600_ao_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = me4600_ao_query_range_info;
	subdevice->base.me_subdevice_query_timer = me4600_ao_query_timer;

	subdevice->base.me_subdevice_postinit = me4600_ao_postinit;
	subdevice->base.me_subdevice_irq_handle = me4600_ao_irq_handle;
	subdevice->base.me_subdevice_destructor = me4600_ao_destructor;

	// Prepare work queue and work function
	subdevice->me4600_workqueue = me4600_wq;

	// workqueue API changed in kernel 2.6.20
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&subdevice->ao_control_task, me4600_ao_work_control_task, (void *)subdevice);
#else
	INIT_DELAYED_WORK(&subdevice->ao_control_task, me4600_ao_work_control_task);
#endif

	return subdevice;
}

static int inline ao_isnt_fifo_full(me4600_ao_subdevice_t* instance)
{
	uint32_t tmp;
	me_readl(instance->base.dev, &tmp, instance->status_reg);
	return tmp & ME4600_AO_STATUS_BIT_FF;
}

static void ao_determine_FIFO_size(me4600_ao_subdevice_t* instance)
{
	uint32_t ctrl;
	int fifo_size  = 0;

	if (!instance->fifo)
	{
		instance->fifo_size = 0;
		return;
	}

	ME_LOCK_PROTECTOR;
		PDEBUG("executed. idx=%d\n", instance->base.idx);

		// Clear all features. Dissable interrupts.
		ctrl = (0x0000
			| ME4600_AO_CTRL_BIT_MODE_WRAPAROUND
			| ME4600_AO_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AO_CTRL_BIT_RESET_IRQ);
		// Clear fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		// Enable fifo.
		ctrl |= ME4600_AO_CTRL_BIT_ENABLE_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);

		// Write entries.
		while (ao_isnt_fifo_full(instance) && (fifo_size<(0x1<<20)))
		{
			me_writel(instance->base.dev, 0, instance->fifo_reg);
			fifo_size++;
		}
		if (!(fifo_size<(0x1<<20)))
		{
			PERROR ("Determing size of FIFO failed.\n");
			goto EXIT;
		}

		instance->fifo_size = fifo_size;

		PINFO("fifo_size=%d\n", instance->fifo_size);
EXIT:
		ctrl = (0x0000
			| ME4600_AO_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AO_CTRL_BIT_RESET_IRQ);
		// Clear fifo.
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
	ME_UNLOCK_PROTECTOR;
}
