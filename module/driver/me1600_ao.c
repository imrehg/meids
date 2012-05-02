/**
 * @file me1600_ao.c
 *
 * @brief TheME-1600 analog output subdevice instance.
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

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "me1600_ao_reg.h"
# include "me1600_ao.h"

static void me1600_ao_trigger_synchronous_list(me1600_ao_subdevice_t* instance);

static void me1600_ao_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void* subdevice
#else
											struct work_struct* work
#endif
										);

int me1600_ao_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me1600_ao_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me1600_ao_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me1600_ao_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me1600_ao_query_number_channels(me_subdevice_t* subdevice, int* number);
int me1600_ao_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me1600_ao_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int me1600_ao_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
int me1600_ao_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
int me1600_ao_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
void me1600_ao_destructor(struct me_subdevice* subdevice);

/// Implementations
int me1600_ao_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me1600_ao_subdevice_t* instance;
	uint16_t tmp;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			//Cancel control task
			PDEBUG("Cancel control task. idx=%d\n", instance->base.idx);
			atomic_set(&instance->ao_control_task_flag, 0);
			cancel_delayed_work(&instance->ao_control_task);

			// Reset all settings.
			ME_SPIN_LOCK(instance->ao_shadows_lock);
				(instance->ao_regs_shadows)->shadow[instance->base.idx] = 0;
				(instance->ao_regs_shadows)->mirror[instance->base.idx] = 0;
				(instance->ao_regs_shadows)->trigger &= ~(0x1 << instance->base.idx);		//Not waiting for triggering.
				(instance->ao_regs_shadows)->synchronous &= ~(0x1 << instance->base.idx);	//Individual triggering.

				// Set output to default (safe) state.
				ME_SPIN_LOCK(instance->config_regs_lock);
					me_readw(instance->base.dev, &tmp, instance->uni_bi_reg);		// unipolar
					tmp |= (0x1 << instance->base.idx);
					me_writew(instance->base.dev, tmp, instance->uni_bi_reg);

					me_readw(instance->base.dev, &tmp, instance->current_on_reg);	// Volts only!
					tmp &= ~(0x1 << instance->base.idx);
					tmp &= 0x00FF;
					me_writew(instance->base.dev, tmp, instance->current_on_reg);

					me_readw(instance->base.dev, &tmp, instance->i_range_reg);		// 0..20mA <= If exists.
					tmp &= ~(0x1 << instance->base.idx);
					me_writew(instance->base.dev, tmp, instance->i_range_reg);

					me_writew(instance->base.dev, 0, (instance->ao_regs_shadows)->registers[instance->base.idx]);

					// Trigger output.
					me_writew(instance->base.dev, 0x0000, instance->sim_output_reg);
					me_writew(instance->base.dev, 0xFFFF, instance->sim_output_reg);
				ME_SPIN_UNLOCK(instance->config_regs_lock);
			ME_SPIN_UNLOCK(instance->ao_shadows_lock);

			// Set status to 'none'
			instance->status = ao_status_none;
		ME_SUBDEVICE_UNLOCK;

		//Signal reset if user is on wait.
		wake_up_interruptible_all(&instance->wait_queue);

	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me1600_ao_subdevice_t* instance;
	uint16_t tmp;
	int err = ME_ERRNO_SUCCESS;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	// Checking parameters.
	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (trig_edge)
	{
		PERROR("Invalid trigger edge. Software trigger has not edge. Must be ME_TRIG_EDGE_NONE\n");
		return ME_ERRNO_INVALID_TRIG_EDGE;
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
			if ((trig_chain != ME_TRIG_CHAN_DEFAULT) && (trig_chain != ME_TRIG_CHAN_SYNCHRONOUS))
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT or ME_TRIG_CHAN_SYNCHRONOUS.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;

		default:
			PERROR("Invalid trigger type.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	if (ref != ME_REF_AO_GROUND)
	{
		PERROR("Invalid reference. Analog outputs have to have got REF_AO_GROUND.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if ((single_config >= (instance->u_ranges_count + instance->i_ranges_count)) || (single_config < 0))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->u_ranges_count + instance->i_ranges_count - 1);
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	// Checking parameters - done. All is fine. Do config.

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			if (instance->status == ao_status_single_run)
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
				goto ERROR;
			}
			ME_SPIN_LOCK(instance->ao_shadows_lock);
				(instance->ao_regs_shadows)->trigger &= ~(0x1 << instance->base.idx);		//Cancell waiting for trigger.
				(instance->ao_regs_shadows)->shadow[instance->base.idx] = 0;
				(instance->ao_regs_shadows)->mirror[instance->base.idx] = 0;

				ME_SPIN_LOCK(instance->config_regs_lock);
					switch (single_config)
					{
						case 0:	// 0V 10V
							me_readw(instance->base.dev, &tmp, instance->current_on_reg);	// Volts
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->current_on_reg);

							// 0V
							me_writew(instance->base.dev, 0, (instance->ao_regs_shadows)->registers[instance->base.idx]);

							me_readw(instance->base.dev, &tmp, instance->uni_bi_reg);		// unipolar
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->uni_bi_reg);

							me_readw(instance->base.dev, &tmp, instance->i_range_reg);		// 0..20mA <= If exists.
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->i_range_reg);
							break;

						case 1: // -10V 10V
							me_readw(instance->base.dev, &tmp, instance->current_on_reg);	// Volts
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->current_on_reg);

							// 0V
							me_writew(instance->base.dev, 0x0800, (instance->ao_regs_shadows)->registers[instance->base.idx]);

							me_readw(instance->base.dev, &tmp, instance->uni_bi_reg);		// bipolar
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->uni_bi_reg);

							me_readw(instance->base.dev, &tmp, instance->i_range_reg);		// 0..20mA <= If exists.
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->i_range_reg);
							break;

						case 2:	// 0mA 20mA
							me_readw(instance->base.dev, &tmp, instance->current_on_reg);	// mAmpers
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->current_on_reg);

							me_readw(instance->base.dev, &tmp, instance->i_range_reg);		// 0..20mA
							tmp &= ~(0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->i_range_reg);

							// 0mA
							me_writew(instance->base.dev, 0, (instance->ao_regs_shadows)->registers[instance->base.idx]);

							me_readw(instance->base.dev, &tmp, instance->uni_bi_reg);		// unipolar
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->uni_bi_reg);
							break;

						case 3:	// 4mA 20mA
							me_readw(instance->base.dev, &tmp, instance->current_on_reg);	// mAmpers
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->current_on_reg);

							me_readw(instance->base.dev, &tmp, instance->i_range_reg);	// 4..20mA
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->i_range_reg);

							// 4mA
							me_writew(instance->base.dev, 0, (instance->ao_regs_shadows)->registers[instance->base.idx]);

							me_readw(instance->base.dev, &tmp, instance->uni_bi_reg);	// unipolar
							tmp |= (0x1 << instance->base.idx);
							me_writew(instance->base.dev, tmp, instance->uni_bi_reg);
							break;
					}

					// Trigger output.
					me_writew(instance->base.dev, 0x0000, instance->sim_output_reg);
					me_writew(instance->base.dev, 0xFFFF, instance->sim_output_reg);

					if (trig_chain == ME_TRIG_CHAN_SYNCHRONOUS)
					{// Synchronous triggering.
						(instance->ao_regs_shadows)->synchronous |= (0x1 << instance->base.idx);
						PDEBUG("Synchronous triggering.\n");
					}
					else //if (trig_chain == ME_TRIG_CHAN_DEFAULT)
					{// Individual triggering.
						(instance->ao_regs_shadows)->synchronous &= ~(0x1 << instance->base.idx);
						PDEBUG("Individual triggering.\n");
					}
				ME_SPIN_UNLOCK(instance->config_regs_lock);
			ME_SPIN_UNLOCK(instance->ao_shadows_lock);

		instance->status = ao_status_single_configured;
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me1600_ao_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me1600_ao_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long j;
	int err = ME_ERRNO_SUCCESS;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (flags & ~(ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS))
	{
		PERROR("Invalid flag specified. Can be ME_IO_SINGLE_TYPE_NO_FLAGS, ME_IO_SINGLE_TYPE_NONBLOCKING and ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_SPIN_LOCK(instance->ao_shadows_lock);
			if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
			{// Trigger all outputs from synchronous list.
				me1600_ao_trigger_synchronous_list(instance);
			}
		ME_SPIN_UNLOCK(instance->ao_shadows_lock);

		if ((!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING)) && ((instance->ao_regs_shadows)->trigger & instance->base.idx))
		{//Blocking mode. Wait for software trigger.
			if (time_out)
			{
				delay = (time_out * HZ) / 1000;
				if (!delay)
					delay = 1;
				if (delay>LONG_MAX - 2)
					delay = LONG_MAX - 2;
			}
			j = jiffies;
			//Only runing process will interrupt this call. Events are signaled when status change. This procedure has own timeout.
			wait_event_interruptible_timeout(
				instance->wait_queue,
				(!((instance->ao_regs_shadows)->trigger & instance->base.idx)),
				delay);

			if ((delay) && ((jiffies - j) >= delay))
			{
				PDEBUG("Timeout reached.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (instance == ao_status_none)
			{// Reset was called.
				PDEBUG("Single canceled.\n");
				err = ME_ERRNO_CANCELLED;
			}

			if (signal_pending(current))
			{
				PERROR("Wait on start of state machine interrupted.\n");
				err = ME_ERRNO_SIGNAL;
			}
		}
		*value = (instance->ao_regs_shadows)->mirror[instance->base.idx];
	ME_SUBDEVICE_EXIT;

	return err;
}

int me1600_ao_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me1600_ao_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long j;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (flags & ~(ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS))
	{
		PERROR("Invalid flag specified. Can be ME_IO_SINGLE_TYPE_NO_FLAGS, ME_IO_SINGLE_TYPE_NONBLOCKING and ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
#ifdef SCALE_RT
	if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
	{
		PERROR("Invalid flag specified. ME_IO_SINGLE_TYPE_NONBLOCKING has to be set.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
#endif

	if(value & ~ME1600_AO_MAX_DATA)
	{
		PERROR("Invalid value provided. Maximum value %d.\n", ME1600_AO_MAX_DATA);
		return ME_ERRNO_VALUE_OUT_OF_RANGE;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			if (instance->status == ao_status_single_run)
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
				goto ERROR;
			}
			(instance->ao_regs_shadows)->trigger &= ~(0x1 << instance->base.idx);		//Cancell waiting for trigger.

			if (time_out)
			{
				delay = (time_out * HZ) / 1000;

				if (!delay)
					delay = 1;
				if (delay>LONG_MAX - 2)
					delay = LONG_MAX - 2;
			}
			j = jiffies;

			//Write value.
			ME_SPIN_LOCK(instance->ao_shadows_lock);
				(instance->ao_regs_shadows)->shadow[instance->base.idx] = (uint16_t)value;

				if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
				{// Trigger all outputs from synchronous list.
					(instance->ao_regs_shadows)->synchronous |= 0x1 << instance->base.idx;
					me1600_ao_trigger_synchronous_list(instance);
					instance->status = ao_status_single_end;
				}
				else
				{// Individual mode.
					if ((instance->ao_regs_shadows)->synchronous & (0x1 << instance->base.idx))
					{// Put on synchronous start list. Set output as waiting for trigger.
						PDEBUG("Add to synchronous list. idx=%d\n", instance->base.idx);
						(instance->ao_regs_shadows)->trigger |= (0x1 << instance->base.idx);
						instance->status = ao_status_single_run;
						PDEBUG("Synchronous list: 0x%x.\n", (instance->ao_regs_shadows)->synchronous);
					}
					else
					{// Fired this one.
						PDEBUG("Triggering. idx=%d\n", instance->base.idx);
						(instance->ao_regs_shadows)->mirror[instance->base.idx] = (instance->ao_regs_shadows)->shadow[instance->base.idx];

						me_writew(instance->base.dev, (instance->ao_regs_shadows)->shadow[instance->base.idx], (instance->ao_regs_shadows)->registers[instance->base.idx]);

						// Set output as triggered.
						(instance->ao_regs_shadows)->trigger &= ~(0x1 << instance->base.idx);

						// Trigger output.
						me_writew(instance->base.dev, 0x0000, instance->sim_output_reg);
						me_writew(instance->base.dev, 0xFFFF, instance->sim_output_reg);
						instance->status = ao_status_single_end;
					}
				}
			ME_SPIN_UNLOCK(instance->ao_shadows_lock);
#ifdef SCALE_RT
			instance->status = ao_status_single_end;
			goto ERROR;
#endif	//SCALE_RT

			//Init control task
			instance->timeout.delay = delay;
			instance->timeout.start_time = j;
			PDEBUG("Schedule control task.\n");
			atomic_set(&instance->ao_control_task_flag, 1);
			queue_delayed_work(instance->me1600_workqueue, &instance->ao_control_task, 1);

			if ((!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING)) && ((instance->ao_regs_shadows)->trigger & instance->base.idx))
			{//Blocking mode. Wait for software trigger.
				ME_SUBDEVICE_UNLOCK;
					//Only runing process will interrupt this call. Events are signaled when status change. This procedure has own timeout.
					wait_event_interruptible_timeout(
						instance->wait_queue,
						(!((instance->ao_regs_shadows)->trigger & instance->base.idx)),
						delay);
				ME_SUBDEVICE_UNLOCK;

				if ((delay) && ((jiffies - j) >= delay))
				{
					PDEBUG("Timeout reached.\n");
					err = ME_ERRNO_TIMEOUT;
				}

				if (instance == ao_status_none)
				{
					PDEBUG("Single canceled.\n");
					err = ME_ERRNO_CANCELLED;
				}

				if (signal_pending(current))
				{
					PERROR("Wait on start of state machine interrupted.\n");
					err = ME_ERRNO_SIGNAL;
				}
			}

ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me1600_ao_query_number_channels(me_subdevice_t* subdevice, int *number)
{
	me1600_ao_subdevice_t* instance;
	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*number = 1;	//Every subdevice has only 1 channel.

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_query_subdevice_type(me_subdevice_t* subdevice, int *type, int *subtype)
{
	me1600_ao_subdevice_t* instance;
	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*type = ME_TYPE_AO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = ME_CAPS_AO_TRIG_SYNCHRONOUS;

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{
	me1600_ao_subdevice_t* instance;
	int i;
	int r = -1;
	int diff = 21E6;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	if (*max < *min)
	{
		PERROR("Invalid minimum and maximum values specified. MIN: %d > MAX: %d\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	// Maximum ranges are slightly less then 10V or 20mA. For convenient we accepted this value as valid one.
	if (unit == ME_UNIT_VOLT)
	{
		for (i = 0; i < instance->u_ranges_count; i++)
		{
			if ((instance->u_ranges[i].min <= *min) && ((instance->u_ranges[i].max + 5000) >= *max))
			{
				if ((instance->u_ranges[i].max - instance->u_ranges[i].min) - (*max - *min) < diff)
				{
					r = i;
					diff = (instance->u_ranges[i].max - instance->u_ranges[i].min) - (*max - *min);
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
			*min = instance->u_ranges[r].min;
			*max = instance->u_ranges[r].max;
			*range = r;
		}
	}
	else if (unit == ME_UNIT_AMPERE)
	{
		for (i = 0; i < instance->i_ranges_count; i++)
		{
			if ((instance->i_ranges[i].min <= *min) && (instance->i_ranges[i].max + 5000 >= *max))
			{
				if ((instance->i_ranges[i].max - instance->i_ranges[i].min) - (*max - *min) < diff)
				{
					r = i;
					diff = (instance->i_ranges[i].max - instance->i_ranges[i].min) - (*max - *min);
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
			*min = instance->i_ranges[r].min;
			*max = instance->i_ranges[r].max;
			*range = r + instance->u_ranges_count;
		}
	}
	else
	{
		PERROR("Invalid physical unit specified.\n");
		return ME_ERRNO_INVALID_UNIT;
	}
	*maxdata = ME1600_AO_MAX_DATA;

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{
	me1600_ao_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me1600_ao_subdevice_t *) subdevice;
	switch (unit)
	{
		case ME_UNIT_VOLT:
			*count = instance->u_ranges_count;
			break;
		case ME_UNIT_AMPERE:
			*count = instance->i_ranges_count;
			break;
		case ME_UNIT_ANY:
			*count = instance->u_ranges_count + instance->i_ranges_count;
			break;
		default:
			*count = 0;
	}

	return ME_ERRNO_SUCCESS;
}

int me1600_ao_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	me1600_ao_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me1600_ao_subdevice_t *) subdevice;

	if ((range >= (instance->u_ranges_count + instance->i_ranges_count)) || (range < 0))
	{
		PERROR("Invalid range number specified. Must be between 0 and %d.\n", instance->u_ranges_count + instance->i_ranges_count - 1);
		*unit = ME_UNIT_INVALID;
		return ME_ERRNO_INVALID_RANGE;
	}

	if (range < instance->u_ranges_count)
	{
		*unit = ME_UNIT_VOLT;
		*min = instance->u_ranges[range].min;
		*max = instance->u_ranges[range].max;
	}
	else if (range < instance->u_ranges_count + instance->i_ranges_count)
	{
		*unit = ME_UNIT_AMPERE;
		*min = instance->i_ranges[range - instance->u_ranges_count].min;
		*max = instance->i_ranges[range - instance->u_ranges_count].max;
	}
	*maxdata = ME1600_AO_MAX_DATA;

	return ME_ERRNO_SUCCESS;
}

void me1600_ao_destructor(struct me_subdevice* subdevice)
{
	me1600_ao_subdevice_t* instance;

	instance = (me1600_ao_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	atomic_set(&instance->ao_control_task_flag, 0);

	// Reset subdevice to asure clean exit.
	me1600_ao_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);

	ME_SUBDEVICE_LOCK;
		// Remove any tasks from work queue. This is paranoic because it was done allready in reset().
		atomic_set(&instance->ao_control_task_flag, 0);
		cancel_delayed_work(&instance->ao_control_task);
	ME_SUBDEVICE_UNLOCK;
	//Wait 2 ticks to be sure that control task is removed from queue.
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(2);

	me_subdevice_deinit(subdevice);
}

me1600_ao_subdevice_t* me1600_ao_constr(void* reg_base, unsigned int idx,
	   										me_lock_t* config_regs_lock, me_lock_t* ao_shadows_lock, me1600_ao_shadow_t* ao_regs_shadows,
											int curr, struct workqueue_struct* me1600_wq)
{
	me1600_ao_subdevice_t *subdevice;

	PDEBUG("executed. idx=%d\n", idx);

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me1600_ao_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for analog output subdevice instance.\n");
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
	subdevice->config_regs_lock = config_regs_lock;
	subdevice->ao_shadows_lock = ao_shadows_lock;

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize range lists. (Static initialization.)
	subdevice->u_ranges_count = 2;

	subdevice->u_ranges[0].min = 0;				//0V
	subdevice->u_ranges[0].max = 9997558;		//10V

	subdevice->u_ranges[1].min = -10000000;		//-10V
	subdevice->u_ranges[1].max = 9995117;		//10V

	if (curr)
	{// This is version with current outputs.
		subdevice->i_ranges_count = 2;

		subdevice->i_ranges[0].min = 0;			//0mA
		subdevice->i_ranges[0].max = 19995117;	//20mA

		subdevice->i_ranges[1].min = 4000000;	//4mA
		subdevice->i_ranges[1].max = 19995118;	//20mA
	}
	else
	{// This is version without current outputs.
		subdevice->i_ranges_count = 0;

		subdevice->i_ranges[0].min = 0;			//0mA
		subdevice->i_ranges[0].max = 0;			//0mA

		subdevice->i_ranges[1].min = 0;			//0mA
		subdevice->i_ranges[1].max = 0;			//0mA
	}

	atomic_set(&subdevice->ao_control_task_flag, 0);

	// Initialize registers.
	subdevice->uni_bi_reg = reg_base + ME1600_UNI_BI_REG;
	subdevice->i_range_reg = reg_base + ME1600_020_420_REG;
	subdevice->sim_output_reg = reg_base + ME1600_SIM_OUTPUT_REG;
	subdevice->current_on_reg = reg_base + ME1600_CURRENT_ON_REG;

	// Initialize shadow structure.
	subdevice->ao_regs_shadows = ao_regs_shadows;

	// Override base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me1600_ao_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me1600_ao_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me1600_ao_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me1600_ao_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me1600_ao_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me1600_ao_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me1600_ao_query_subdevice_caps;
	subdevice->base.me_subdevice_query_range_by_min_max = me1600_ao_query_range_by_min_max;
	subdevice->base.me_subdevice_query_number_ranges = me1600_ao_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = me1600_ao_query_range_info;

	subdevice->base.me_subdevice_destructor = me1600_ao_destructor;

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Prepare work queue.
	subdevice->me1600_workqueue = me1600_wq;

	// workqueue API changed in kernel 2.6.20
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&subdevice->ao_control_task, me1600_ao_work_control_task, (void *)subdevice);
#else
	INIT_DELAYED_WORK(&subdevice->ao_control_task, me1600_ao_work_control_task);
#endif

	return subdevice;
}

static void me1600_ao_trigger_synchronous_list(me1600_ao_subdevice_t* instance)
{
	int i;

	for (i=0; i< (instance->ao_regs_shadows)->count; i++)
	{
		if ((instance->ao_regs_shadows)->synchronous & (0x1 << i))
		{// Set all from synchronous list to correct state.
			PDEBUG("Synchronous triggering: output %d. idx=%d\n", i, instance->base.idx);
			(instance->ao_regs_shadows)->mirror[i] = (instance->ao_regs_shadows)->shadow[i];
			me_writew(instance->base.dev, (instance->ao_regs_shadows)->shadow[i], (instance->ao_regs_shadows)->registers[i]);
			(instance->ao_regs_shadows)->trigger &= ~(0x1 << i);
		}
	}

	// Trigger output.
	me_writew(instance->base.dev, 0x0000, instance->sim_output_reg);
	me_writew(instance->base.dev, 0xFFFF, instance->sim_output_reg);
}

static void me1600_ao_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void *subdevice
#else
											struct work_struct* work
#endif
										)
{
	me1600_ao_subdevice_t* instance;
	int reschedule = 1;
	int signaling = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	instance = (me1600_ao_subdevice_t *) subdevice;
#else
	instance = container_of((void *)work, me1600_ao_subdevice_t, ao_control_task);
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
		return;
	}

	ME_SUBDEVICE_LOCK;
		if (!atomic_read(&instance->ao_control_task_flag))
		{
			goto EXIT;
		}
		PINFO("<%s: %ld> executed. idx=%d\n", __FUNCTION__, jiffies, instance->base.idx);

		if (instance->status != ao_status_single_run)
		{
			signaling = 0;
			reschedule = 0;
		}

		if (!((instance->ao_regs_shadows)->trigger & instance->base.idx))
		{// Output was triggerd.
			// Signal the end.
			signaling = 1;
			reschedule = 0;
			if(instance->status == ao_status_single_run)
			{
				instance->status = ao_status_single_end;
			}
		}

		if ((instance->timeout.delay) && ((jiffies - instance->timeout.start_time) >= instance->timeout.delay))
		{// Timeout
			PDEBUG("Timeout reached.\n");
			ME_SPIN_LOCK (instance->ao_shadows_lock);
				// Restore old settings.
				PDEBUG("Write old value back to register.\n");
				(instance->ao_regs_shadows)->shadow[instance->base.idx] = (instance->ao_regs_shadows)->mirror[instance->base.idx];

				me_writew(instance->base.dev, (instance->ao_regs_shadows)->mirror[instance->base.idx], (instance->ao_regs_shadows)->registers[instance->base.idx]);

				//Remove from synchronous start list.
				(instance->ao_regs_shadows)->trigger &= ~(0x1 << instance->base.idx);
				if(instance->status == ao_status_single_run)
				{
					instance->status = ao_status_single_end;
				}
			ME_SPIN_UNLOCK (instance->ao_shadows_lock);

			// Signal the end.
			signaling = 1;
			reschedule = 0;
		}
EXIT:
	ME_SUBDEVICE_UNLOCK;

	if (signal_pending(current))
	{
		PERROR("Control task interrupted. Ending.\n");
		instance->status = ao_status_none;
		return;
	}

	if (atomic_read(&instance->ao_control_task_flag) && reschedule)
	{// Reschedule task
		queue_delayed_work(instance->me1600_workqueue, &instance->ao_control_task, 1);
	}
	else
	{
		PINFO("<%s> Ending control task.\n", __FUNCTION__);
	}

	if (signaling)
	{//Signal it.
		wake_up_interruptible_all(&instance->wait_queue);
	}

}
