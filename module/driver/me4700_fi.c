/**
 * @file me4700_fi.c
 * @brief The ME-4700 frequency input subdevice instance.
 * @note Copyright (C) 20079 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# include <linux/sched.h>


# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

#include "me4700_fio_reg.h"
#include "me4700_fi.h"

int me4700_fi_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me4700_fi_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me4700_fi_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);

int me4700_fi_query_number_channels(me_subdevice_t* subdevice, int* number);
int me4700_fi_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);

int me4700_fi_query_timer(me_subdevice_t* subdevice,  int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);

static int inline check_fi_status(me4700_fi_subdevice_t* instance);
static int me4700_fi_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);

static int me4700_fi_io_single_read_single(me4700_fi_subdevice_t* instance, int* value, int time_out, int flags);
static int me4700_fi_io_single_read_continous(me4700_fi_subdevice_t* instance, int* value, int time_out, int flags);

int me4700_fi_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4700_fi_subdevice_t* instance;
	uint32_t tmp;

	instance = (me4700_fi_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			instance->status = fi_status_none;
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			// Disable FI line. Disable interrupt.
			tmp &= ~(ME4700_FI_START_STOP_MASK << (ME4700_FI_START_STOP_BIT_BASE + (instance->base.idx << 1)));
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			instance->divider = 0x0;
			instance->first_counter = 0x0;
			instance->period = 0x0;
			instance->low = 0x0;
			instance->high = 0x0;
		ME_UNLOCK_PROTECTOR;
		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me4700_fi_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4700_fi_subdevice_t* instance;
	uint32_t tmp;
	uint32_t dummy;

	instance = (me4700_fi_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags & ~ME_IO_SINGLE_CONFIG_FREQ_SINGLE_MODE)
	{
		PERROR("Invalid flags. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS or ME_IO_SINGLE_CONFIG_FREQ_SINGLE_MODE.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (trig_edge)
	{
		PERROR("Invalid trigger edge. Must be ME_TRIG_EDGE_NONE.\n");
		return ME_ERRNO_INVALID_TRIG_EDGE;
	}

	if (trig_type)
	{
		PERROR("Invalid trigger type. Must be ME_TRIG_TYPE_NONE.\n");
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

	if ((single_config != ME_SINGLE_CONFIG_FIO_INPUT) && (single_config != ME_SINGLE_CONFIG_FIO_INPUT_LINUX))
	{
		PERROR("Invalid configuration specified. Must be ME_SINGLE_CONFIG_FIO_INPUT.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	ME_SUBDEVICE_ENTER
		ME_LOCK_PROTECTOR;
			// Starting frequency measurment.
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			// Reset.
			tmp &= ~(ME4700_FI_START_STOP_MASK << (ME4700_FI_START_STOP_BIT_BASE + (instance->base.idx << 1)));
			me_readl(instance->base.dev, &dummy, instance->low_level_reg);
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			// Enable FI line.
			tmp |= ME4700_FI_START_STOP_BIT << (ME4700_FI_START_STOP_BIT_BASE + (instance->base.idx << 1));
			if (!(flags & ME_IO_SINGLE_CONFIG_FREQ_SINGLE_MODE))
			{// Enable interrupt. Continous mode.
				tmp |= ME4700_FI_START_STOP_INTERRUPTS << (ME4700_FI_START_STOP_BIT_BASE + (instance->base.idx << 1));
			}
			me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			instance->status = (flags & ME_IO_SINGLE_CONFIG_FREQ_SINGLE_MODE) ? fi_status_configured_single : fi_status_configured;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

static int me4700_fi_io_single_read_continous(me4700_fi_subdevice_t* instance, int* value, int time_out, int flags)
{
	uint64_t divider_64b;
	unsigned long int j;
	unsigned long int delay = LONG_MAX - 2;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	//If blocking mode -> wait for data.
	if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
	{
		if (time_out)
		{
			delay = (time_out * HZ) / 1000;
			if (!delay)
				delay = 1;
			if (delay>LONG_MAX - 2)
				delay = LONG_MAX - 2;
		}

		j = jiffies;
		interruptible_sleep_on_timeout(&instance->wait_queue, delay);

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
			goto ERROR;
		}
		else if(instance->status == fi_status_none)
		{
			PDEBUG("Single canceled.\n");
			err = ME_ERRNO_CANCELLED;
			goto ERROR;
		}
		else if(instance->status != fi_status_configured)
		{
			PERROR("FI error!\n");
			err = ME_ERRNO_INTERNAL;
			goto ERROR;
		}
	}

	if (!(flags & ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE))
	{
		ME_LOCK_PROTECTOR;
			if ((instance->low == 0x0) || (instance->high == 0x0))
			{
				PINFO("Period undefined or overload. low=0x%x high=0x%x\n", instance->low, instance->high);
				// Period undefined or overload.
				instance->period = 0x7FFFFFFF;
				instance->divider = 0x0;
				instance->first_counter = 0x0;
			}
			else
			{
				instance->first_counter = instance->high;
				instance->period = instance->low + instance->high;
				divider_64b = instance->high;
				divider_64b <<= 32;
				(void) do_div(divider_64b,instance->low + instance->high);
				instance->divider = (uint)divider_64b;
			}
		ME_UNLOCK_PROTECTOR;
	}
	if (flags & ME_IO_SINGLE_TYPE_FREQ_DIVIDER)
	{
		*value = (int)instance->divider;
	}
	else if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
	{
		*value = (int)instance->first_counter;
	}
	else
	{
		*value = (int)instance->period;
	}
ERROR:
	return err;
}

static int me4700_fi_io_single_read_single(me4700_fi_subdevice_t* instance, int* value, int time_out, int flags)
{
	uint32_t tmp;
	uint64_t divider_64b;
	unsigned long int j;
	unsigned long int delay = LONG_MAX - 2;
	int err = ME_ERRNO_SUCCESS;


	PDEBUG("executed\n");

	if ((instance->status == fi_status_configured_single_end) && !(flags & ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE))
	{
		// Measure has finished in previous read. You can only read stored values. ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE and ME_IO_SINGLE_TYPE_NONBLOCKING are request!
		return ME_ERRNO_SUBDEVICE_NOT_RUNNING;
	}

	//If blocking mode -> wait for data.
	if (!(flags & ME_IO_SINGLE_TYPE_NONBLOCKING))
	{
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
		while (check_fi_status(instance))
		{//Pollin machine...
			//Wait for measure to finish.
			set_current_state(TASK_UNINTERRUPTIBLE);
			schedule_timeout(1);

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
				goto ERROR;
			}
			else if(instance->status == fi_status_none)
			{
				PDEBUG("Single canceled.\n");
				err = ME_ERRNO_CANCELLED;
				goto ERROR;
			}
			else if(instance->status != fi_status_configured_single)
			{
				PERROR("FI error!\n");
				err = ME_ERRNO_INTERNAL;
				goto ERROR;
			}
		}
	}

	if (!(flags & ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE))
	{
		if (!check_fi_status(instance))
		{
			me_readl(instance->base.dev, &tmp, instance->low_level_reg);
			instance->low = tmp;
			me_readl(instance->base.dev, &tmp, instance->high_level_reg);
			instance->high = tmp;
			PINFO("low=0x%x high=0x%x\n", instance->low, instance->high);

			ME_LOCK_PROTECTOR;
				if ((instance->low == 0x0) || (instance->high == 0x0))
				{
					// Period undefined or overload.
					instance->period = 0x7FFFFFFF;
					instance->divider = 0x0;
					instance->first_counter = 0;
				}
				else
				{
					instance->first_counter = instance->high;
					instance->period = instance->low + instance->high;
					divider_64b = instance->high;
					divider_64b <<= 32;
					(void) do_div(divider_64b,instance->low + instance->high);
					instance->divider = (uint)divider_64b;
				}

				instance->status = fi_status_configured_single_end;
			ME_UNLOCK_PROTECTOR;
		}
		else
		{
			PINFO("Working...\n");
			err = ME_ERRNO_SUBDEVICE_BUSY;
			goto ERROR;
		}
	}

	if (flags & ME_IO_SINGLE_TYPE_FREQ_DIVIDER)
	{
		*value = (int)instance->divider;
	}
	else if (flags & ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE)
	{
		*value = (int)instance->first_counter;
	}
	else
	{
		*value = (int)instance->period;
	}
ERROR:

	return err;
}

int me4700_fi_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me4700_fi_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4700_fi_subdevice_t *)subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags & ~(ME_IO_SINGLE_TYPE_FREQ_DIVIDER | ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE | ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE | ME_IO_SINGLE_TYPE_FIO_TICKS_TOTAL))
	{
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	if ((flags & (ME_IO_SINGLE_TYPE_NONBLOCKING | ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE)) == ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE)
	{
			PERROR("Invalid flags specified. ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE works only in NON-blocking mode.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	switch (instance->status)
	{
		case fi_status_none:
			PERROR("FI isn't working. Config must be called before read.\n");
			return ME_ERRNO_PREVIOUS_CONFIG;
		break;

		case fi_status_configured:
			// All OK.
			ME_SUBDEVICE_ENTER;
				err = me4700_fi_io_single_read_continous(instance, value, time_out, flags);
			ME_SUBDEVICE_EXIT;
		break;
		case fi_status_configured_single:
		case fi_status_configured_single_end:
			// All OK.
			ME_SUBDEVICE_ENTER;
				err = me4700_fi_io_single_read_single(instance, value, time_out, flags);
			ME_SUBDEVICE_EXIT;
		break;

		case fi_status_error:
		default:
			PERROR("Error detected. Reset and config must be called for clearing situation.\n");
			return ME_ERRNO_PREVIOUS_CONFIG;
		break;
	}


	return err;
}

int me4700_fi_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed idx=%d.\n", ((me4700_fi_subdevice_t *) subdevice)->base.idx);

	*number = 1;

	return ME_ERRNO_SUCCESS;
}

int me4700_fi_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	me4700_fi_subdevice_t* instance;
	instance = (me4700_fi_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	*type = ME_TYPE_FREQ_I;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me4700_fi_query_timer(me_subdevice_t* subdevice,  int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	me4700_fi_subdevice_t* instance;

	instance = (me4700_fi_subdevice_t *) subdevice;

	PDEBUG("executed. idx=%d\n", instance->base.idx);

	*base_frequency = ME4700_FI_BASE_FREQUENCY;
	if ((timer != ME_TIMER_FIO_TOTAL) && (timer != ME_TIMER_FIO_FIRST_PHASE))
	{
		*min_ticks = 0;
		*max_ticks = 0;
		PERROR("Invalid timer specified.\n");
		return ME_ERRNO_INVALID_TIMER;
	}

	*min_ticks = 0x0000000000000002L;
	*max_ticks = 0x000000007FFFFFFEL;

	return ME_ERRNO_SUCCESS;
}

me4700_fi_subdevice_t* me4700_fi_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock)
{
	me4700_fi_subdevice_t* subdevice;
	uint32_t tmp;

	PDEBUG("executed idx=%d.\n", idx);

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4700_fi_subdevice_t), GFP_KERNEL);
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

	// Set the subdevice ports' registers.
	switch (idx)
	{
		case 0:
			subdevice->low_level_reg = reg_base + ME4700_FIO_PORT_A_LOW_REG;
			subdevice->high_level_reg = reg_base + ME4700_FIO_PORT_A_HIGH_REG;
		break;

		case 1:
			subdevice->low_level_reg = reg_base + ME4700_FIO_PORT_B_LOW_REG;
			subdevice->high_level_reg = reg_base + ME4700_FIO_PORT_B_HIGH_REG;
		break;

		case 2:
			subdevice->low_level_reg = reg_base + ME4700_FIO_PORT_C_LOW_REG;
			subdevice->high_level_reg = reg_base + ME4700_FIO_PORT_C_HIGH_REG;
		break;

		case 3:
			subdevice->low_level_reg = reg_base + ME4700_FIO_PORT_D_LOW_REG;
			subdevice->high_level_reg = reg_base + ME4700_FIO_PORT_D_HIGH_REG;
		break;

	}

	subdevice->status_reg = reg_base + ME4700_FI_STATUS;
	subdevice->ctrl_reg = reg_base + ME4700_FI_START_STOP_REG;

	// Override base class methods.
	subdevice->base.me_subdevice_io_single_config = me4700_fi_io_single_config;
	subdevice->base.me_subdevice_query_number_channels = me4700_fi_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4700_fi_query_subdevice_type;
	subdevice->base.me_subdevice_io_reset_subdevice = me4700_fi_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_read = me4700_fi_io_single_read;
	subdevice->base.me_subdevice_query_timer = me4700_fi_query_timer;
	subdevice->base.me_subdevice_irq_handle = me4700_fi_irq_handle;

	// Set FIO feature active.
	ME_SPIN_LOCK(subdevice->ctrl_reg_lock);
		me_readl(subdevice->base.dev, &tmp, reg_base + ME4700_FIO_CTRL_REG);
		tmp &= ~(ME4700_FIO_CTRL_BIT_MODE_MASK << ME4700_FI_CTRL_BIT_SHIFT);
		tmp |= ME4700_FIO_CTRL_BIT_FRQUENCY_MODE << ME4700_FI_CTRL_BIT_SHIFT;
		me_writel(subdevice->base.dev, tmp, reg_base + ME4700_FIO_CTRL_REG);
	ME_SPIN_UNLOCK(subdevice->ctrl_reg_lock);

	return subdevice;
}

static inline int check_fi_status(me4700_fi_subdevice_t* instance)
{
	uint32_t tmp;

	me_readl(instance->base.dev, &tmp, instance->status_reg);
	PINFO("fi_status_val 0x%08x\n", tmp);
	return tmp & (0x01 << (ME4700_FI_STATUS_BASE + instance->base.idx));
}

static int me4700_fi_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{/// @note This is time critical function!
	uint32_t tmp;
	me4700_fi_subdevice_t* instance = (me4700_fi_subdevice_t *)subdevice;
	int err = ME_ERRNO_SUCCESS;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed idx=%d.\n", instance->base.idx);

// 		if (check_fi_status(instance))
// 		{
// 			PINFO("OVERLOAD (STATUS)!\n");
// 		}

		me_readl(instance->base.dev, &tmp, instance->low_level_reg);
		instance->low = tmp;
		me_readl(instance->base.dev, &tmp, instance->high_level_reg);
		instance->high = tmp;
		PINFO("low=0x%x high=0x%x\n", instance->low, instance->high);

		if ((!instance->low) || (!instance->high))
		{
			PINFO("OVERLOAD!\n");
		}

		// Reenable FI.
		me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
		tmp &= ~(ME4700_FI_START_STOP_BIT << (ME4700_FI_START_STOP_BIT_SHIFT * instance->base.idx));
		me_writel(instance->base.dev, tmp, instance->ctrl_reg);
		tmp |= ME4700_FI_START_STOP_BIT << (ME4700_FI_START_STOP_BIT_SHIFT * instance->base.idx);
		me_writel(instance->base.dev, tmp, instance->ctrl_reg);
	ME_FREE_HANDLER_PROTECTOR;

	if (!err)
	{
		//Signal it.
		wake_up_interruptible_all(&instance->wait_queue);
	}
	return err;
}

