/**
 * @file me4700_fo.c
 * @brief The ME-4700 frequency output subdevice instance.
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/fs.h>
# include <linux/slab.h>
#include <linux/sched.h>


# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

#include "me4700_fio_reg.h"
#include "me4700_fo.h"

static void me4700_fo_trigger_synchronous_list(me4700_fo_subdevice_t* instance);
static void me4700_fo_low_and_high_from_divider(uint32_t period, uint32_t divider, uint32_t* low, uint32_t* high);
static void me4700_fo_low_and_high_from_counter(uint32_t period, uint32_t first_counter, uint32_t* low, uint32_t* high);

static void me4700_fo_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void *subdevice
#else
											struct work_struct* work
#endif
										);

int me4700_fo_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me4700_fo_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me4700_fo_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me4700_fo_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me4700_fo_query_number_channels(me_subdevice_t* subdevice, int* number);
int me4700_fo_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me4700_fo_query_timer(me_subdevice_t* subdevice,  int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
void me4700_fo_destructor(struct me_subdevice* subdevice);

/// Implementations
int me4700_fo_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4700_fo_subdevice_t* instance;
	uint32_t tmp;

	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			//Cancel control task
			PDEBUG("Cancel control task. idx=%d\n", instance->base.idx);
			atomic_set(&instance->fo_control_task_flag, 0);
			cancel_delayed_work(&instance->fo_control_task);

			// Reset all settings.
			ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
				instance->fo_shared_contex->shadow[instance->base.idx].period = 0;
				instance->fo_shared_contex->shadow[instance->base.idx].divider = 0;
				instance->fo_shared_contex->shadow[instance->base.idx].first_counter = 0;
				instance->fo_shared_contex->shadow[instance->base.idx].mode = fo_configuration_none;
				instance->fo_shared_contex->mirror[instance->base.idx].period = 0;
				instance->fo_shared_contex->mirror[instance->base.idx].divider = 0;
				instance->fo_shared_contex->mirror[instance->base.idx].first_counter = 0;
				instance->fo_shared_contex->mirror[instance->base.idx].mode = fo_configuration_none;
				//Clear all features: triggering, synchornous execution, soft restart, output negation.
				instance->fo_shared_contex->conditions &= ~(ME4700_FO_STATUS_BIT_MASK << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));

				ME_SPIN_LOCK(instance->ctrl_reg_lock);
					me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
					// Disable FO line.
					tmp &= ~(ME4700_FO_START_STOP_MASK << (ME4700_FO_START_STOP_BIT_BASE + instance->base.idx));
					me_writel(instance->base.dev, tmp, instance->ctrl_reg);
				ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
			ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);

			// Set status to 'none'
			instance->status = fo_status_none;
		ME_SUBDEVICE_UNLOCK;

		//Signal reset if user is on wait.
		wake_up_interruptible_all(&instance->wait_queue);

	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me4700_fo_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4700_fo_subdevice_t* instance;

	int err = ME_ERRNO_SUCCESS;

	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (trig_edge)
	{
		PERROR("Invalid trigger edge. Must be ME_TRIG_EDGE_NONE.\n");
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

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if (ref)
	{
		PERROR("Invalid ref specified. Must be 0.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if ((single_config != ME_SINGLE_CONFIG_FIO_OUTPUT) && (single_config != ME_SINGLE_CONFIG_FIO_OUTPUT_LINUX))
	{
		PERROR("Invalid configuration specified. Must be ME_SINGLE_CONFIG_FIO_OUTPUT.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	ME_SUBDEVICE_ENTER
		ME_SUBDEVICE_LOCK;
			if (instance->status == fo_status_single_run)
			{
				PERROR("Previous operation haven't finish, jet. Make meReset() or wait for finish.\n");
				err = ME_ERRNO_SUBDEVICE_BUSY;
				goto ERROR;
			}
			ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
				//Clear all features: triggering, synchornous execution, soft restart, output negation.
				instance->fo_shared_contex->conditions &= ~(ME4700_FO_STATUS_BIT_MASK << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
				if (trig_chain == ME_TRIG_CHAN_SYNCHRONOUS)
				{// Synchronous triggering.
					instance->fo_shared_contex->conditions |= (ME4700_FO_SYNCHRO_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
					PDEBUG("Synchronous triggering.\n");
				}
				else //if (trig_chain == ME_TRIG_CHAN_DEFAULT)
				{// Individual triggering.
					PDEBUG("Individual triggering.\n");
				}
			ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);
			instance->status = fo_status_configured;
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4700_fo_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me4700_fo_subdevice_t* instance;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long j;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4700_fo_subdevice_t *)subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags & ~(ME_IO_SINGLE_TYPE_FREQ_DIVIDER | ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS | ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE | ME_IO_SINGLE_TYPE_FIO_TICKS_TOTAL))
	{
			PERROR("Invalid flags specified. %x\n", flags);
			return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
		{// Trigger all outputs from synchronous list.
			me4700_fo_trigger_synchronous_list(instance);
		}

		if ((!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING)) && (instance->fo_shared_contex->conditions & (ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT))))
		{//Blocking mode. Wait for software trigger.
			PINFO("BLOCKING mode.\n");
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
				(!(instance->fo_shared_contex->conditions & (ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))),
				delay);

			if ((delay) && ((jiffies - j) >= delay))
			{
				PDEBUG("Timeout reached.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (instance->status == fo_status_none)
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

		if (flags & ME_IO_SINGLE_TYPE_FREQ_DIVIDER)
		{
			*value = (int)(instance->fo_shared_contex->mirror[instance->base.idx].divider);
		}
		else if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
		{
			*value = (int)(instance->fo_shared_contex->mirror[instance->base.idx].first_counter);
		}
		else
		{
			*value = (int)(instance->fo_shared_contex->mirror[instance->base.idx].period);
		}
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4700_fo_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me4700_fo_subdevice_t* instance;
	uint32_t high;
	uint32_t low;
	uint32_t tmp;
	unsigned long int delay = LONG_MAX - 2;
	unsigned long j;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags & ~(ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_FREQ_DIVIDER | ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE | ME_IO_SINGLE_TYPE_FIO_TICKS_TOTAL | ME_IO_SINGLE_TYPE_FREQ_START_LOW | ME_IO_SINGLE_TYPE_FREQ_START_SOFT | ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY | ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS))
	{
		PERROR("Invalid flags specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (flags & ME_IO_SINGLE_TYPE_FREQ_DIVIDER)
	{
		PDEBUG("divider=0x%x\n", (unsigned int)value);
	}
	else if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
	{
		PDEBUG("first counter=0x%x\n", (unsigned int)value);
	}
	else
	{
		PDEBUG("period=0x%x\n", (unsigned int)value);
	}

#ifdef SCALE_RT
	if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
	{
		PERROR("Invalid flag specified. ME_IO_SINGLE_TYPE_NONBLOCKING has to be set.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
#endif

	if (flags & ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY)
	{
		if (flags & (ME_IO_SINGLE_TYPE_FREQ_START_LOW | ME_IO_SINGLE_TYPE_FREQ_START_SOFT))
		{
			PERROR("Invalid flags specified. ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY can not be used in combination with ME_IO_SINGLE_TYPE_FREQ_START_SOFT or ME_IO_SINGLE_TYPE_FREQ_START_LOW.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
	{// Period have to be bigger than first counter
		if (value >= instance->fo_shared_contex->shadow[instance->base.idx].period)
		{
			PERROR("Unacceptable first counter value.\n");
			return ME_ERRNO_INTERNAL;
		}
	}
	else if (!(flags & (ME_IO_SINGLE_TYPE_FREQ_DIVIDER | ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)))
	{// Period can not smaller than 4
		if (((value >= 1) && (value <= 3)) || ((uint)value > 0x80000001U))
		{
			PERROR("Unacceptable period value.\n");
			return ME_ERRNO_INTERNAL;
		}
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
		if (instance->status == fo_status_single_run)
		{
			PERROR("Previous operation haven't finish, jet. Make meReset()+meConfig() or wait for finish.\n");
			err = ME_ERRNO_SUBDEVICE_BUSY;
			goto ERROR;
		}

		if (instance->status == fo_status_none)
		{
			PERROR("Subdevice is not configured. Always do meConfig() at begin.\n");
			err = ME_ERRNO_PREVIOUS_CONFIG;
			goto ERROR;
		}

		ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
			if (flags & ME_IO_SINGLE_TYPE_FREQ_DIVIDER)
			{
				instance->fo_shared_contex->shadow[instance->base.idx].divider = (uint)value;
				instance->fo_shared_contex->shadow[instance->base.idx].mode = fo_configuration_divider;
			}
			else if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
			{
				instance->fo_shared_contex->shadow[instance->base.idx].first_counter = (uint)value;
				instance->fo_shared_contex->shadow[instance->base.idx].mode = fo_configuration_first_counter;
			}
			else
			{
				instance->fo_shared_contex->shadow[instance->base.idx].period = (uint)value;
			}
		ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);

		if (!(flags & ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY))
		{// Do real work.
			PDEBUG("Do real work.\n");
			if (time_out)
			{
				delay = (time_out * HZ) / 1000;
				if (!delay)
					delay = 1;
				if (delay>LONG_MAX - 2)
					delay = LONG_MAX - 2;
			}
			j = jiffies;

			if ((flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
				||
				(instance->fo_shared_contex->conditions & (ME4700_FO_SYNCHRO_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT))))
			{
				PDEBUG("Add to waiting queue.\n");
				ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
					//Add to the list.
					if (flags & ME_IO_SINGLE_TYPE_FREQ_START_LOW)
					{
						instance->fo_shared_contex->conditions |= (ME4700_FO_INVERT_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
					}
					else
					{
						instance->fo_shared_contex->conditions &= ~(ME4700_FO_INVERT_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
					}
					if (flags & ME_IO_SINGLE_TYPE_FREQ_START_SOFT)
					{
						instance->fo_shared_contex->conditions |= (ME4700_FO_SOFT_START_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
					}
					else
					{
						instance->fo_shared_contex->conditions &= ~(ME4700_FO_SOFT_START_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
					}
					instance->fo_shared_contex->conditions |= (ME4700_FO_SYNCHRO_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
				ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);

				if (flags & ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS)
				{// Trigger synchronous list.
					me4700_fo_trigger_synchronous_list(instance);
					instance->status = fo_status_single_end;
				}
				else
				{
					instance->status = fo_status_single_run;
				}
			}
			else
			{// Fired this one.
				if ((!(flags & ME_IO_SINGLE_TYPE_FREQ_START_SOFT)) || (!instance->fo_shared_contex->shadow[instance->base.idx].period))
				{
					PDEBUG("Disable FO line.\n");
					// Disable FO line.
					ME_SPIN_LOCK(instance->ctrl_reg_lock);
						me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
						tmp &= ~(ME4700_FO_START_STOP_MASK << (ME4700_FO_START_STOP_BIT_BASE + (instance->base.idx * ME4700_FO_START_STOP_BIT_SHIFT)));
						if (flags & ME_IO_SINGLE_TYPE_FREQ_START_LOW)
						{
							tmp |= (ME4700_FO_START_STOP_INVERT << (ME4700_FO_START_STOP_BIT_BASE + (instance->base.idx * ME4700_FO_START_STOP_BIT_SHIFT)));
						}
						me_writel(instance->base.dev, tmp, instance->ctrl_reg);
					ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
				}

				ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
					instance->fo_shared_contex->mirror[instance->base.idx].period = instance->fo_shared_contex->shadow[instance->base.idx].period;
					instance->fo_shared_contex->mirror[instance->base.idx].divider = instance->fo_shared_contex->shadow[instance->base.idx].divider;
					instance->fo_shared_contex->mirror[instance->base.idx].first_counter = instance->fo_shared_contex->shadow[instance->base.idx].first_counter;
					instance->fo_shared_contex->mirror[instance->base.idx].mode = instance->fo_shared_contex->shadow[instance->base.idx].mode;

					if (instance->fo_shared_contex->shadow[instance->base.idx].period)
					{
						switch (instance->fo_shared_contex->shadow[instance->base.idx].mode)
						{
							case fo_configuration_divider:
								me4700_fo_low_and_high_from_divider(instance->fo_shared_contex->shadow[instance->base.idx].period, instance->fo_shared_contex->shadow[instance->base.idx].divider, &low, &high);
								break;

							case fo_configuration_first_counter:
								if (instance->fo_shared_contex->shadow[instance->base.idx].first_counter >= instance->fo_shared_contex->shadow[instance->base.idx].period)
								{
									PERROR("Unacceptable counters values.\n");
									err = ME_ERRNO_INTERNAL;
									break;
								}
								me4700_fo_low_and_high_from_counter(instance->fo_shared_contex->shadow[instance->base.idx].period, instance->fo_shared_contex->shadow[instance->base.idx].first_counter, &low, &high);
								break;

							case fo_configuration_none:
								// default: symetrical signal
								me4700_fo_low_and_high_from_counter(instance->fo_shared_contex->shadow[instance->base.idx].period, instance->fo_shared_contex->shadow[instance->base.idx].period >> 1, &low, &high);

							default:
								// Paranoid check - this is not possible!
								PERROR_CRITICAL("Corruption in subdevice structure!.\n");
								err = ME_ERRNO_INTERNAL;
								// Dummy - Remove compilator warning
								low = high = 0;
						}

						PDEBUG("FIRST=0x%x SECOND=0x%x\n", high, low);
						if (!err)
						{
							me_writel(instance->base.dev, low, instance->fo_shared_contex->regs[instance->base.idx].low_level_reg);
							me_writel(instance->base.dev, high, instance->fo_shared_contex->regs[instance->base.idx].high_level_reg);
							PDEBUG("Enable FO line.\n");
							// Enable FO line.
							ME_SPIN_LOCK(instance->ctrl_reg_lock);
								me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
								tmp &= ~(ME4700_FO_START_STOP_MASK << (ME4700_FO_START_STOP_BIT_BASE + (instance->base.idx * ME4700_FO_START_STOP_BIT_SHIFT)));
								if (flags & ME_IO_SINGLE_TYPE_FREQ_START_LOW)
								{
									tmp |= ME4700_FO_START_STOP_INVERT << (ME4700_FO_START_STOP_BIT_BASE + (instance->base.idx * ME4700_FO_START_STOP_BIT_SHIFT));
								}

								tmp |= ME4700_FO_START_STOP_BIT << (ME4700_FO_START_STOP_BIT_BASE + (instance->base.idx * ME4700_FO_START_STOP_BIT_SHIFT));
								me_writel(instance->base.dev, tmp, instance->ctrl_reg);
							ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
						}
					}
				ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);
				if (!err)
				{
					instance->status = fo_status_single_end;
				}
				else
				{
					instance->status = fo_status_error;
					goto ERROR;
				}
			}
#ifdef SCALE_RT
			instance->status = fo_status_single_end;
			goto ERROR;
#endif

			//Init control task
			instance->timeout.delay = delay;
			instance->timeout.start_time = j;
			PDEBUG("Schedule control task.\n");
			atomic_set(&instance->fo_control_task_flag, 1);
			queue_delayed_work(instance->me4700_workqueue, &instance->fo_control_task, 1);

			if ((!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING)) && (instance->fo_shared_contex->conditions & (ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT))))
			{//Blocking mode. Wait for software trigger.
				//Only runing process will interrupt this call. Events are signaled when status change. This procedure has own timeout.
				PINFO("BLOCKING mode.\n");
				ME_SUBDEVICE_UNLOCK;
					wait_event_interruptible_timeout(
						instance->wait_queue,
						(!(instance->fo_shared_contex->conditions & (ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))),
						delay);
				ME_SUBDEVICE_LOCK;

				if ((delay) && ((jiffies - j) >= delay))
				{
					PDEBUG("Timeout reached.\n");
					err = ME_ERRNO_TIMEOUT;
				}

				if (instance == fo_status_none)
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
		}
		else
		{
			PDEBUG("Update only.\n");
		}
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4700_fo_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed idx=%d.\n", ((me4700_fo_subdevice_t *) subdevice)->base.idx);

	*number = 1;

	return ME_ERRNO_SUCCESS;
}

int me4700_fo_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	me4700_fo_subdevice_t* instance;
	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	*type = ME_TYPE_FREQ_O;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me4700_fo_query_timer(me_subdevice_t* subdevice,  int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	me4700_fo_subdevice_t* instance;

	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*base_frequency = ME4700_FO_BASE_FREQUENCY;
	if ((timer != ME_TIMER_FIO_TOTAL) && (timer != ME_TIMER_FIO_FIRST_PHASE))
	{
		*min_ticks = 0x0;
		*max_ticks = 0x0;
		PERROR("Invalid timer specified.\n");
		return ME_ERRNO_INVALID_TIMER;
	}

	*min_ticks = 0x0000000000000004L;
	*max_ticks = 0x0000000080000000L;

	return ME_ERRNO_SUCCESS;
}

void me4700_fo_destructor(struct me_subdevice* subdevice)
{
	me4700_fo_subdevice_t* instance;

	instance = (me4700_fo_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	atomic_set(&instance->fo_control_task_flag, 0);

	// Reset subdevice to asure clean exit.
	me4700_fo_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);

	ME_SUBDEVICE_LOCK;
		// Remove any tasks from work queue. This is paranoic because it was done allready in reset().
		atomic_set(&instance->fo_control_task_flag, 0);
		cancel_delayed_work(&instance->fo_control_task);
	ME_SUBDEVICE_UNLOCK;
	//Wait 2 ticks to be sure that control task is removed from queue.
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(2);

	me_subdevice_deinit(subdevice);
}

me4700_fo_subdevice_t* me4700_fo_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, me4700_fo_context_t* fo_shared_contex, struct workqueue_struct* me4700_wq)
{
	me4700_fo_subdevice_t* subdevice;
	uint32_t tmp;

	PDEBUG("executed idx=%d.\n", idx);

	if ((!fo_shared_contex) || (!fo_shared_contex->regs) || (!fo_shared_contex->mirror) || (!fo_shared_contex->shadow) || (idx >= fo_shared_contex->count))
	{//Paranoid check. This should never, ever happend!
		PERROR_CRITICAL("Memory for shared context not reserved!\n");
		return NULL;
	}

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4700_fo_subdevice_t), GFP_KERNEL);
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
	subdevice->ctrl_reg_lock = ctrl_reg_lock;

	// Save digital i/o index.
	subdevice->base.idx = idx;


	// Initialize wait queue
	init_waitqueue_head(&subdevice->wait_queue);


	// Initialize common context structure.
	subdevice->fo_shared_contex = fo_shared_contex;

	// Set the subdevice ports' registers.
	switch (idx)
	{
		case 0:
			subdevice->fo_shared_contex->regs[0].low_level_reg = reg_base + ME4700_FIO_PORT_A_LOW_REG;
			subdevice->fo_shared_contex->regs[0].high_level_reg = reg_base + ME4700_FIO_PORT_A_HIGH_REG;
		break;

		case 1:
			subdevice->fo_shared_contex->regs[1].low_level_reg = reg_base + ME4700_FIO_PORT_B_LOW_REG;
			subdevice->fo_shared_contex->regs[1].high_level_reg = reg_base + ME4700_FIO_PORT_B_HIGH_REG;
		break;

		case 2:
			subdevice->fo_shared_contex->regs[2].low_level_reg = reg_base + ME4700_FIO_PORT_C_LOW_REG;
			subdevice->fo_shared_contex->regs[2].high_level_reg = reg_base + ME4700_FIO_PORT_C_HIGH_REG;
		break;

		case 3:
			subdevice->fo_shared_contex->regs[3].low_level_reg = reg_base + ME4700_FIO_PORT_D_LOW_REG;
			subdevice->fo_shared_contex->regs[3].high_level_reg = reg_base + ME4700_FIO_PORT_D_HIGH_REG;
		break;

	}
	subdevice->ctrl_reg = reg_base + ME4700_FIO_CTRL_REG;

	// Override base class methods.
	subdevice->base.me_subdevice_io_single_config = me4700_fo_io_single_config;
	subdevice->base.me_subdevice_query_number_channels = me4700_fo_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4700_fo_query_subdevice_type;
	subdevice->base.me_subdevice_io_reset_subdevice = me4700_fo_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_read = me4700_fo_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me4700_fo_io_single_write;
	subdevice->base.me_subdevice_query_timer = me4700_fo_query_timer;

	subdevice->base.me_subdevice_destructor = me4700_fo_destructor;

	// Set FIO feature active.
	ME_SPIN_LOCK(subdevice->ctrl_reg_lock);
		me_readl(subdevice->base.dev, &tmp, reg_base + ME4700_FIO_CTRL_REG);
		tmp &= ~(ME4700_FIO_CTRL_BIT_MODE_MASK << ME4700_FO_CTRL_BIT_SHIFT);
		tmp |= ME4700_FIO_CTRL_BIT_FRQUENCY_MODE << ME4700_FO_CTRL_BIT_SHIFT;
		me_writel(subdevice->base.dev, tmp, reg_base + ME4700_FIO_CTRL_REG);
	ME_SPIN_UNLOCK(subdevice->ctrl_reg_lock);

	// Initialize task security flag.
	atomic_set(&subdevice->fo_control_task_flag, 0);

	// Prepare work queue.
	subdevice->me4700_workqueue = me4700_wq;

	// workqueue API changed in kernel 2.6.20
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&subdevice->fo_control_task, me4700_fo_work_control_task, (void *)subdevice);
#else
	INIT_DELAYED_WORK(&subdevice->fo_control_task, me4700_fo_work_control_task);
#endif

	return subdevice;
}

/// Implementations - local auxilary functions (helpers)
static void me4700_fo_trigger_synchronous_list(me4700_fo_subdevice_t* instance)
{
	uint32_t tmp;
	uint32_t high;
	uint32_t low;
	uint32_t stop_mask = 0x00;
	uint32_t start_mask = 0x00;
	int idx;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	//Decode start/stop conditions for all outputs on synchronous list.
	for (idx = 0; idx < instance->fo_shared_contex->count; idx++)
	{
		if (instance->fo_shared_contex->conditions & (ME4700_FO_SYNCHRO_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))
		{// Is marked for synchronous execution.
			if (instance->fo_shared_contex->shadow[idx].period)
			{// Create starting entry.
				start_mask |= ME4700_FO_START_STOP_BIT << (idx * ME4700_FO_START_STOP_BIT_SHIFT);
				if (instance->fo_shared_contex->conditions & (ME4700_FO_INVERT_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))
				{// Is marked for output inversion.
					start_mask |= ME4700_FO_START_STOP_INVERT << (idx * ME4700_FO_START_STOP_BIT_SHIFT);
				}
			}
			if ((!(instance->fo_shared_contex->conditions & (ME4700_FO_SOFT_START_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))) || (!instance->fo_shared_contex->shadow[idx].period))
			{// Create stoping enty.
				stop_mask |= ME4700_FO_START_STOP_BIT << (idx * ME4700_FO_START_STOP_BIT_SHIFT);
			}
		}
	}
	stop_mask <<= ME4700_FO_START_STOP_BIT_BASE;
	start_mask <<= ME4700_FO_START_STOP_BIT_BASE;

	// Trigger FO outputs.
	ME_SPIN_LOCK(instance->ctrl_reg_lock);
		me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
		// Stop this engine befor changing value.
		me_writel(instance->base.dev, tmp & ~stop_mask, instance->ctrl_reg);
		//Write new values.
		for (idx = 0; idx < instance->fo_shared_contex->count; idx++)
		{
			if (instance->fo_shared_contex->conditions & (ME4700_FO_SYNCHRO_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))
			{
				if (instance->fo_shared_contex->shadow[idx].period)
				{
					switch (instance->fo_shared_contex->shadow[instance->base.idx].mode)
					{
						case fo_configuration_divider:
							me4700_fo_low_and_high_from_divider(instance->fo_shared_contex->shadow[idx].period, instance->fo_shared_contex->shadow[idx].divider, &low, &high);
							break;

						case fo_configuration_first_counter:
							me4700_fo_low_and_high_from_counter(instance->fo_shared_contex->shadow[idx].period, instance->fo_shared_contex->shadow[idx].first_counter, &low, &high);
							break;

						case fo_configuration_none:
							// default: symetrical signal
							me4700_fo_low_and_high_from_counter(instance->fo_shared_contex->shadow[idx].period, instance->fo_shared_contex->shadow[idx].period >> 1, &low, &high);

						default:
							// Paranoid check - this is not possible!
							PERROR_CRITICAL("Corruption in subdevice structure!.\n");
							ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
							return;
					}

					me_writew(instance->base.dev, low, instance->fo_shared_contex->regs[idx].low_level_reg);
					me_writew(instance->base.dev, high, instance->fo_shared_contex->regs[idx].high_level_reg);
				}
			}
		}
		//Synchronously start outputs.
		me_writel(instance->base.dev, tmp | start_mask, instance->ctrl_reg);
	ME_SPIN_UNLOCK(instance->ctrl_reg_lock);

	// Set logical status.
	for (idx = 0; idx < instance->fo_shared_contex->count; idx++)
	{
		if (instance->fo_shared_contex->conditions & (ME4700_FO_SYNCHRO_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT)))
		{// Set all from synchronous list to correct state.
			PDEBUG("Synchronous triggering: FO:%d.\n", idx);
			instance->fo_shared_contex->mirror[idx].period = instance->fo_shared_contex->shadow[idx].period;
			instance->fo_shared_contex->mirror[idx].divider = instance->fo_shared_contex->shadow[idx].divider;
			instance->fo_shared_contex->mirror[idx].first_counter = instance->fo_shared_contex->shadow[idx].first_counter;
			instance->fo_shared_contex->mirror[idx].mode = instance->fo_shared_contex->shadow[idx].mode;
			// Mark as triggered.
			instance->fo_shared_contex->conditions &= ~(ME4700_FO_TRIGGER_STATUS_BIT << (idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
		}
	}
}

static void me4700_fo_low_and_high_from_divider(uint32_t period, uint32_t divider, uint32_t* low, uint32_t* high)
{
	uint64_t period_64b;
	uint64_t divider_64b;

	period_64b = period;
	divider_64b = divider;
	divider_64b &= 0x00000000FFFFFFFFL;
	period_64b = period_64b * divider_64b;
	period_64b >>= 32;
	*low = period_64b;
	*high = period - *low;

	if (*high < 2)
	{
		*low -= 2 - *high;
		*high = 2;
	}
	else if (*low < 2)
	{
		*high -= 2 - *low;
		*low = 2;
	}

	PINFO("period:0x%x divider:0x%x =>> low:0x%x high:0x%x\n", period, divider, *low, *high);

	*high -= 1;
	*low  -= 1;
}


static void me4700_fo_low_and_high_from_counter(uint32_t period, uint32_t first_counter, uint32_t* low, uint32_t* high)
{
	*low = first_counter;
	*high = period - *low;

	if (*high < 2)
	{
		*low -= 2 - *high;
		*high = 2;
	}
	else if (*low < 2)
	{
		*high -= 2 - *low;
		*low = 2;
	}

	PINFO("period:0x%x first_counter:0x%x =>> low:0x%x high:0x%x\n", period, first_counter, *low, *high);

	*high -= 1;
	*low  -= 1;
}
static void me4700_fo_work_control_task(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
											void *subdevice
#else
											struct work_struct* work
#endif
										)
{
	me4700_fo_subdevice_t* instance;
	int reschedule = 1;
	int signaling = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	instance = (me4700_fo_subdevice_t *) subdevice;
#else
	instance = container_of((void *)work, me4700_fo_subdevice_t, fo_control_task);
#endif

	if (!instance)
	{
		return;
	}

	if (signal_pending(current))
	{
		PERROR("Control task interrupted.\n");
		instance->status = fo_status_none;
		return;
	}

	if (!atomic_read(&instance->fo_control_task_flag))
	{
		return;
	}

	ME_SUBDEVICE_LOCK;
		if (!atomic_read(&instance->fo_control_task_flag))
		{
			goto EXIT;
		}
		PINFO("<%s: %lu> executed. idx=%d\n", __FUNCTION__, jiffies, instance->base.idx);

		if (instance->status != fo_status_single_run)
		{
			signaling = 0;
			reschedule = 0;
		}

		if (!(instance->fo_shared_contex->conditions & (ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT))))
		{// Output was triggerd.
			// Signal the end.
			signaling = 1;
			reschedule = 0;
			if(instance->status == fo_status_single_run)
			{
				instance->status = fo_status_single_end;
			}
		}

		if ((instance->timeout.delay) && ((jiffies - instance->timeout.start_time) >= instance->timeout.delay))
		{// Timeout
			PDEBUG("Timeout reached.\n");
			ME_SPIN_LOCK(&instance->fo_shared_contex->fo_context_lock);
				// Restore old settings.
				PDEBUG("Write old value back to register.\n");
				instance->fo_shared_contex->shadow[instance->base.idx].period = instance->fo_shared_contex->mirror[instance->base.idx].period;
				instance->fo_shared_contex->shadow[instance->base.idx].divider = instance->fo_shared_contex->mirror[instance->base.idx].divider;
				instance->fo_shared_contex->shadow[instance->base.idx].first_counter = instance->fo_shared_contex->mirror[instance->base.idx].first_counter;
				instance->fo_shared_contex->shadow[instance->base.idx].mode = instance->fo_shared_contex->mirror[instance->base.idx].mode;

				//Remove from synchronous start list.
				instance->fo_shared_contex->conditions &= ~(ME4700_FO_TRIGGER_STATUS_BIT << (instance->base.idx * ME4700_FO_TRIGGER_STATUS_BIT_SHIFT));
				if(instance->status == fo_status_single_run)
				{
					instance->status = fo_status_single_end;
				}
			ME_SPIN_UNLOCK(&instance->fo_shared_contex->fo_context_lock);

			// Signal the end.
			signaling = 1;
			reschedule = 0;
		}
EXIT:
	ME_SUBDEVICE_UNLOCK;

	if (signal_pending(current))
	{
		PERROR("Control task interrupted. Ending.\n");
		instance->status = fo_status_none;
		return;
	}

	if (atomic_read(&instance->fo_control_task_flag) && reschedule)
	{// Reschedule task
		queue_delayed_work(instance->me4700_workqueue, &instance->fo_control_task, 1);
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

