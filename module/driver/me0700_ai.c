/**
 * @file me0700_ai.c
 *
 * @brief The ME-0700 analog input subdevice instance.
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

# include "me0700_reg.h"
# include "me0700_ai_reg.h"
# include "me0700_bus.h"
# include "me0700_ai.h"
# include "medevice.h"

extern const int ME0700_AI_CHANNEL_ADDR[];
/// Declarations

static void me0700_ai_destructor(me_subdevice_t* subdevice);
static int me0700_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);

static int me0700_ai_io_single_config_check(me0700_ai_subdevice_t* instance, int channel,
										int single_config, int ref);
static int me0700_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);

static int me0700_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
static int me0700_ai_io_stream_config_check(me0700_ai_subdevice_t* instance, meIOStreamSimpleConfig_t* config_list, int count, int flags);
static int me0700_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t *config_list, int count, meIOStreamSimpleTriggers_t *trigger, int fifo_irq_threshold, int flags);

static int me0700_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags);

static int me0700_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags);
static int me0700_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags);
static int me0700_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags);
static int me0700_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags);

static int me0700_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
static int me0700_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
static int me0700_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
static int me0700_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
static int me0700_ai_query_number_channels(me_subdevice_t* subdevice, int *number);
static int me0700_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
static int me0700_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
static int me0700_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count);

int me0700_ai_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
int me0700_ai_io_irq_wait(me_subdevice_t* subdevice, struct file *filep, int channel, int* irq_count, int* value, int time_out, int flags);
int me0700_ai_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me0700_ai_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);

static int me0700_ai_postinit(me_subdevice_t* subdevice, void* args);

static int me0700_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);

static int convert_to_me0700_range(me4600_ai_subdevice_t* instance, int range, int me4600_ranges_number);

static void me0700_ai_destructor(me_subdevice_t* subdevice)
{
	me0700_ai_subdevice_t* instance;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (!instance)
	{
		return;
	}

	if (instance->ai_instance)
	{
		instance->ai_instance->base.me_subdevice_destructor((struct me_subdevice*)instance->ai_instance);
		kfree(instance->ai_instance);
		instance->ai_instance = NULL;
	}

	if (instance->ctrl_port)
	{
		instance->ctrl_port->base.me_subdevice_destructor((struct me_subdevice*)instance->ctrl_port);
		kfree(instance->ctrl_port);
		instance->ctrl_port = NULL;
	}

	if (instance->data_port)
	{
		instance->data_port->base.me_subdevice_destructor((struct me_subdevice*)instance->data_port);
		kfree(instance->data_port);
		instance->data_port = NULL;
	}

	if (instance->ext_irq)
	{
		instance->ext_irq->base.me_subdevice_destructor((struct me_subdevice*)instance->data_port);
		kfree(instance->ext_irq);
		instance->ext_irq = NULL;
	}

	me_subdevice_deinit(&instance->base);
}

static int me0700_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;
	int i;
	int err = ME_ERRNO_SUCCESS;

	instance = (me0700_ai_subdevice_t *)subdevice;

	PDEBUG("executed. idx=0\n");

	ME_SUBDEVICE_ENTER;
		// Set secure setting (500A).
		for (i=0; i<ME0700_I_CHANNELS_NUMBER; ++i)
		{
			instance->me0700_ranges[i] = ME0700_AI_RANGE_INVALID;
		}
		err = me0700_reset_relays(instance);
		if (err)
			goto ERROR;

		instance->irq_rised = 0;
		instance->irq_status = 0x00;
		instance->irq_status_flag = 0;

		err = instance->ai_instance->base.me_subdevice_io_reset_subdevice((me_subdevice_t*)instance->ai_instance, filep, flags);
		if (err)
			goto ERROR;
		err = instance->ctrl_port->base.me_subdevice_io_reset_subdevice((me_subdevice_t*)instance->ctrl_port, filep, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
		if (err)
			goto ERROR;
		err = instance->data_port->base.me_subdevice_io_reset_subdevice((me_subdevice_t*)instance->data_port, filep, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
		if (err)
			goto ERROR;
		err = instance->ext_irq->base.me_subdevice_io_reset_subdevice((me_subdevice_t*)instance->ext_irq, filep, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
		if (err)
			goto ERROR;

ERROR:
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me0700_ai_io_single_config_check(me0700_ai_subdevice_t* instance, int channel, int single_config, int ref)
{
	if (ref != ME_REF_AI_GROUND)
	{
		PERROR("Invalid reference specified. Must be ME_REF_AI_GROUND.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if ((channel < 0) || (channel >= instance->number_channels))
	{
		PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->number_channels - 1);
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if (channel < instance->number_I_channels)
	{// me0700 spacific AI channels
		if (convert_to_me0700_range(instance->ai_instance, single_config, instance->me4600_ranges_number) == ME0700_AI_RANGE_INVALID)
			return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	return ME_ERRNO_SUCCESS;
}

static int convert_to_me0700_range(me4600_ai_subdevice_t* instance, int range, int me4600_ranges_number)
{
	int me0700_range;

	switch (range - me4600_ranges_number)
	{
		case 0:
			me0700_range = ME0700_AI_RANGE_500_AMPERE;
		break;

		case 1:
			me0700_range = ME0700_AI_RANGE_50_AMPERE;
		break;

		case 2:
			me0700_range = ME0700_AI_RANGE_25_AMPERE;
		break;

		case 3:
			me0700_range = ME0700_AI_RANGE_2500_MILLIAMPERE;
		break;

		case 4:
			me0700_range = ME0700_AI_RANGE_250_MILLIAMPERE;
		break;

		case 0x1000004:
			me0700_range = ME0700_AI_RANGE_250X_MILLIAMPERE;
		break;

		case 5:
			me0700_range = ME0700_AI_RANGE_25_MILLIAMPERE;
		break;

		case 6:
			me0700_range = ME0700_AI_RANGE_2500_MICROAMPERE;
		break;

		case 7:
			me0700_range = ME0700_AI_RANGE_250_MICROAMPERE;
		break;

		default:
			me0700_range = ME0700_AI_RANGE_INVALID;
	}
	return me0700_range;
}

static int me0700_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	ME_SUBDEVICE_ENTER;
		err = me0700_ai_io_single_config_check(instance, channel, single_config, ref);
		if (err)
			return err;

		if (channel < instance->number_I_channels)
		{// Pre-set relays. -> Give hardware time to settle down.
			instance->me0700_ranges[channel] = convert_to_me0700_range(instance->ai_instance, single_config, instance->me4600_ranges_number);
			ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
				(void)me0700_set_range_relay(instance, instance->me0700_ranges[channel], ME0700_AI_CHANNEL_ADDR[channel]);
			ME_IRQ_UNLOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
			err = instance->ai_instance->base.me_subdevice_io_single_config((me_subdevice_t*)instance->ai_instance, filep, channel, 2, ME_REF_AI_GROUND, trig_chain, trig_type, trig_edge, flags);
		}
		else
		{
			err = instance->ai_instance->base.me_subdevice_io_single_config((struct me_subdevice* )instance->ai_instance, filep, channel + (4 - instance->number_I_channels), single_config, ME_REF_AI_GROUND, trig_chain, trig_type, trig_edge, flags);
		}
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me0700_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance;
	int me4600_channel = channel;
	int err = ME_ERRNO_SUCCESS;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	ME_SUBDEVICE_ENTER;
		if (channel < instance->number_I_channels)
		{// Re-set relays. -> Remove any overflows.
			if (instance->me0700_ranges[channel] == ME0700_AI_RANGE_INVALID)
			{
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}
			ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
				err = me0700_update_range_relay(instance, instance->me0700_ranges[channel], ME0700_AI_CHANNEL_ADDR[channel]);
			ME_IRQ_UNLOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
			if (err)
				goto ERROR;
		}
		else
		{
			me4600_channel += 4 - instance->number_I_channels;
		}

		err = instance->ai_instance->base.me_subdevice_io_single_read((struct me_subdevice* )instance->ai_instance, filep, me4600_channel, value, time_out, flags);
ERROR:
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me0700_ai_io_stream_read(me_subdevice_t* subdevice, struct file* filep, int read_mode, int* values, int* count, int time_out, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_io_stream_read((struct me_subdevice* )instance->ai_instance, filep, read_mode, values, count, time_out, flags);
}

static int me0700_ai_io_stream_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;
	int i;
	int err;

	PDEBUG("executed. idx=0\n");

	// Set currents ranges (for I channels)
	for (i=0; i<ME0700_I_CHANNELS_NUMBER; ++i)
	{
		if (instance->me0700_ranges[i] == ME0700_AI_RANGE_INVALID)
		{
			return ME_ERRNO_PREVIOUS_CONFIG;
		}
		else if (instance->me0700_ranges[i] != ME0700_AI_RANGE_NONE)
		{// Re-set relays. -> Remove any overflows.
			ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
				err = me0700_update_range_relay(instance, instance->me0700_ranges[i], ME0700_AI_CHANNEL_ADDR[i]);
			ME_IRQ_UNLOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
			if (err)
				return err;
		}
	}
	return instance->ai_instance->base.me_subdevice_io_stream_start((struct me_subdevice* )instance->ai_instance, filep, start_mode, time_out, flags);
}

static int me0700_ai_io_stream_stop(me_subdevice_t* subdevice, struct file* filep, int stop_mode, int time_out, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_io_stream_stop((struct me_subdevice* )instance->ai_instance, filep, stop_mode, time_out, flags);
}

static int me0700_ai_io_stream_status(me_subdevice_t* subdevice, struct file* filep, int wait, int* status, int* values, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_io_stream_status((struct me_subdevice* )instance->ai_instance, filep, wait, status, values, flags);
}

static int me0700_ai_io_stream_new_values(me_subdevice_t* subdevice, struct file* filep, int time_out, int* count, int flags)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_io_stream_new_values((struct me_subdevice* )instance->ai_instance, filep, time_out, count, flags);
}

static int me0700_ai_io_stream_config_check(me0700_ai_subdevice_t* instance, meIOStreamSimpleConfig_t* config_list, int count, int flags)
{/// TODO DONE?
	int i;
	int tmp_range;
	int err = ME_ERRNO_SUCCESS;

	if (flags & ME_STREAM_CONFIG_DIFFERENTIAL)
	{
		PERROR("Invalid reference specified. Must be ME_REF_AI_GROUND.\n");
		return ME_ERRNO_INVALID_REF;
	}
	for (i = 0; i < count; i++)
	{
		err = me0700_ai_io_single_config_check(instance, config_list[i].iChannel, config_list[i].iRange, ME_REF_AI_GROUND);
		if (err)
		{
			return (err == ME_ERRNO_INVALID_SINGLE_CONFIG) ? ME_ERRNO_INVALID_STREAM_CONFIG : err;
		}

		if (config_list[i].iChannel < instance->number_I_channels)
		{
			tmp_range = convert_to_me0700_range(instance->ai_instance, config_list[i].iRange, instance->me4600_ranges_number);
			if (tmp_range == ME0700_AI_RANGE_INVALID)
			{
				PERROR("Invalid range specified. Must be current range.\n");
				instance->me0700_ranges[config_list[i].iChannel] = ME0700_AI_RANGE_INVALID;
				return ME_ERRNO_INVALID_STREAM_CONFIG;
			}

			if (instance->me0700_ranges[config_list[i].iChannel] == ME0700_AI_RANGE_NONE)
			{
				instance->me0700_ranges[config_list[i].iChannel] = tmp_range;
			}
			else
			{
				if (instance->me0700_ranges[config_list[i].iChannel] != tmp_range)
				{
					instance->me0700_ranges[config_list[i].iChannel] = ME0700_AI_RANGE_INVALID;
					PERROR("Invalid range specified. Settings are limited to one current range per channel.\n");
					return ME_ERRNO_INVALID_STREAM_CONFIG;
				}
			}
		}
	}

 	return err;
}

static int me0700_ai_io_stream_config(me_subdevice_t* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t* config_list, int count,  meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;
	meIOStreamSimpleConfig_t* config_list_copy;
	int i;
	int err = ME_ERRNO_SUCCESS;

	instance = (me0700_ai_subdevice_t *) subdevice;

	for (i=0; i<ME0700_I_CHANNELS_NUMBER; ++i)
	{
		instance->me0700_ranges[i] = ME0700_AI_RANGE_NONE;
	}

	PDEBUG("executed. idx=0\n");

	ME_SUBDEVICE_ENTER;
		// Check me0700 extra limitations. Set ranges' table.
		err = me0700_ai_io_stream_config_check(instance, config_list, count, flags);
		if (err)
			return err;

		config_list_copy = kzalloc(sizeof(meIOStreamSimpleConfig_t) * count, GFP_KERNEL);
		if (!config_list_copy)
		{
			PERROR("Cannot get memory for ME-0700 AI config list copy.\n");
			err = -ENOMEM;
			goto ERROR;
		}
		memcpy(config_list_copy, config_list, sizeof(meIOStreamSimpleConfig_t) * count);
		// Map current channels to +/-2.5V (for I channels). Convert logical channels' number to phisical.
		for (i=0; i<count; ++i)
		{
			if (config_list_copy[i].iChannel < instance->number_I_channels)
			{
				config_list_copy[i].iRange = 2;
			}
			else
			{
				config_list_copy[i].iChannel += 4 - instance->number_I_channels;
			}
		}

		err = instance->ai_instance->base.me_subdevice_io_stream_config((struct me_subdevice* )instance->ai_instance, filep, config_list_copy, count, trigger, fifo_irq_threshold, flags);
		if (err)
			return err;

		// Set currents ranges (for I channels)
		for (i=0; i<ME0700_I_CHANNELS_NUMBER; ++i)
		{// Pre-set relays. -> Give hardware time to settle down.
			switch(instance->me0700_ranges[i])
			{
				case ME0700_AI_RANGE_INVALID:
					err = ME_ERRNO_INVALID_STREAM_CONFIG;
					break;

				case ME0700_AI_RANGE_NONE:
					break;

				default:
					ME_IRQ_LOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
						(void)me0700_set_range_relay(instance, instance->me0700_ranges[i], ME0700_AI_CHANNEL_ADDR[i]);
					ME_IRQ_UNLOCK(((struct NET2282_usb_device *)instance->base.dev)->usb_IRQ_semaphore);
			}
		}
ERROR:
		if (config_list_copy)
		{
			kfree(config_list_copy);
		}
	ME_SUBDEVICE_EXIT;

	return err;
}


static int me0700_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;
	int i;
	int r = -1;
	int diff = 21E6;

	PDEBUG("executed. idx=0\n");

	instance = (me0700_ai_subdevice_t *) subdevice;

	if (*max < *min)
	{
		PERROR("Invalid minimum and maximum values specified. MIN: %d > MAX: %d\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	if (unit == ME_UNIT_VOLT)
	{
		return instance->ai_instance->base.me_subdevice_query_range_by_min_max((struct me_subdevice* )instance->ai_instance, unit, min, max, maxdata, range);
	}
	else if (unit == ME_UNIT_AMPERE)
	{
		for (i = 0; i < ME0700_AI_EXTRA_RANGE_NUMBER; i++)
		{
			if ((instance->ranges[i].min <= *min) && ((instance->ranges[i].max + (instance->ranges[i].max >> ME0700_RANGE_INACCURACY)) >= *max))
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
			*maxdata = ME0700_AI_MAX_DATA;
			*range = r + instance->me4600_ranges_number;
		}
	}

	return ME_ERRNO_SUCCESS;
}

static int me0700_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;
	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	instance->ai_instance->base.me_subdevice_query_number_ranges((struct me_subdevice* )instance->ai_instance, unit, count);

	if ((unit == ME_UNIT_AMPERE) || (unit == ME_UNIT_ANY))
	{
		*count += ME0700_AI_EXTRA_RANGE_NUMBER;
	}

	return ME_ERRNO_SUCCESS;
}

static int me0700_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{/// TODO DONE?
	me0700_ai_subdevice_t* instance;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (range < instance->me4600_ranges_number)
	{
		instance->ai_instance->base.me_subdevice_query_range_info((struct me_subdevice* )instance->ai_instance, range, unit, min, max, maxdata);
	}
	else if (range < instance->me4600_ranges_number + ME0700_AI_EXTRA_RANGE_NUMBER)
	{
		*unit = ME_UNIT_AMPERE;
		*min = instance->ranges[range - instance->me4600_ranges_number].min;
		*max = instance->ranges[range - instance->me4600_ranges_number].max;
		*maxdata = ME0700_AI_MAX_DATA;
	}
	else
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->me4600_ranges_number + ME0700_AI_EXTRA_RANGE_NUMBER - 1);
		return ME_ERRNO_INVALID_RANGE;
	}
	return ME_ERRNO_SUCCESS;
}

static int me0700_ai_query_number_channels(me_subdevice_t* subdevice, int *number)
{
	me0700_ai_subdevice_t* instance;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	*number = instance->number_channels;

	return ME_ERRNO_SUCCESS;
}

static int me0700_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_query_subdevice_type((struct me_subdevice* )instance->ai_instance, type, subtype);
}

static int me0700_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{/// DONE
	me0700_ai_subdevice_t* instance;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	instance->ai_instance->base.me_subdevice_query_subdevice_caps((struct me_subdevice* )instance->ai_instance, caps);
	*caps = *caps & ~ME_CAPS_AI_DIFFERENTIAL;

	return ME_ERRNO_SUCCESS;
}

static int me0700_ai_query_subdevice_caps_args(me_subdevice_t* subdevice, int cap, int* args, int* count)
{/// DONE
	me0700_ai_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	instance->ai_instance->base.me_subdevice_query_subdevice_caps_args((struct me_subdevice* )instance->ai_instance, cap, args, count);

	return err;
}

static int me0700_ai_query_timer(me_subdevice_t* subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{/// DONE
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	return instance->ai_instance->base.me_subdevice_query_timer((struct me_subdevice* )instance->ai_instance, timer, base_frequency, min_ticks, max_ticks);
}

int me0700_ai_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{/// TODO Implement
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	if (flags & ~ME_IO_IRQ_START_EXTENDED_STATUS)
	{
		PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_NO_FLAGS or ME_IO_IRQ_START_EXTENDED_STATUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (irq_edge)
	{
		PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_NOT_USED.\n");
		return ME_ERRNO_INVALID_IRQ_EDGE;
	}

	if (irq_source && (irq_source != ME_IRQ_SOURCE_HIGH_CURRENT))
	{
		PERROR("Invalid irq source specified. Should be ME_IRQ_SOURCE_HIGH_CURRENT.\n");
		return ME_ERRNO_INVALID_IRQ_SOURCE;
	}

	ME_SUBDEVICE_ENTER;
		err = instance->ext_irq->base.me_subdevice_io_irq_start((struct me_subdevice* )instance->ext_irq, filep, channel, ME_IRQ_SOURCE_DIO_LINE, ME_IRQ_EDGE_RISING, irq_arg, flags);
		if (err)
			goto ERROR;
		instance->irq_rised = 0;
		instance->irq_status = 0;
		instance->irq_status_flag = flags & ME_IO_IRQ_START_EXTENDED_STATUS;
ERROR:
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0700_ai_io_irq_wait(me_subdevice_t* subdevice, struct file *filep, int channel, int* irq_count, int* value, int time_out, int flags)
{/// TODO Implement
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;
	int dummy_value;
	int i;
	int err = ME_ERRNO_SUCCESS;
	int irq_status_low = 0x0;
	int irq_status_high = 0x0;

	PDEBUG("executed. idx=0\n");

	if (flags & ~(ME_IO_IRQ_WAIT_NORMAL_STATUS | ME_IO_IRQ_WAIT_EXTENDED_STATUS))
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_WAIT_NO_FLAGS, ME_IO_IRQ_WAIT_NORMAL_STATUS or ME_IO_IRQ_WAIT_EXTENDED_STATUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		err = instance->ext_irq->base.me_subdevice_io_irq_wait((struct me_subdevice* )instance->ext_irq, filep, channel, irq_count, &dummy_value, time_out, ME_IO_IRQ_WAIT_NO_FLAGS);
		if (err)
			goto ERROR;

		for (i=0; i<4; ++i)
		{
			irq_status_low |= (instance->irq_status >> i) & (0x01<<i);
			irq_status_high |= (instance->irq_status >> (i+1)) & (0x01<<i) ;
		}

		if ((flags & ME_IO_IRQ_WAIT_EXTENDED_STATUS) || instance->irq_status_flag)
		{
			irq_status_high <<= 16;
		}
		*value = irq_status_high | irq_status_low;
		instance->irq_rised = 0;
ERROR:
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0700_ai_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{/// TODO Implement
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	ME_SUBDEVICE_ENTER;
		err = instance->ext_irq->base.me_subdevice_io_irq_stop((struct me_subdevice* )instance->ext_irq, filep, channel, flags);
		instance->irq_rised = -1;
		instance->irq_status = 0;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0700_ai_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{/// TODO Implement
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (flags & ~(ME_IO_IRQ_WAIT_NORMAL_STATUS | ME_IO_IRQ_WAIT_EXTENDED_STATUS))
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_WAIT_NO_FLAGS, ME_IO_IRQ_WAIT_NORMAL_STATUS or ME_IO_IRQ_WAIT_EXTENDED_STATUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}


static int me0700_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{/// TODO Locks are missing
	me0700_ai_subdevice_t* instance = (me0700_ai_subdevice_t *) subdevice;
	int err = ME_ERRNO_SUCCESS;
	uint8_t irq_val = 0xFF;

	PDEBUG("executed. idx=0\n");

	if (irq_status & ME4600_IRQ_STATUS_BIT_EX)
	{// External IRQ
		if (!spin_trylock(&instance->me0700_bus_lock))
		{//Try again later
			PERROR("Can not change internal bus now. I'll try later.\n");
			return ME_ERRNO_SUCCESS;
		}
			me0700_reset_OF_bits(instance, &irq_val);
			PERROR("OVERLOAD reported!: 0x%x\n", irq_val);
		ME_SPIN_UNLOCK(&instance->me0700_bus_lock);
		if (instance->irq_rised <= 0)
		{
			instance->irq_status = irq_val;
		}
		else
		{
			instance->irq_status |= irq_val;
		}
		instance->irq_rised = 1;

		err =  instance->ext_irq->base.me_subdevice_irq_handle((struct me_subdevice* )instance->ext_irq, irq_status);
	}
	if (irq_status & (ME0700_IRQ_STATUS_BIT_AI_HF | ME0700_IRQ_STATUS_BIT_SC | ME0700_IRQ_STATUS_BIT_LE | ME0700_IRQ_STATUS_BIT_AI_OF))
	{// AI IRQ
		err = instance->ai_instance->base.me_subdevice_irq_handle((struct me_subdevice* )instance->ai_instance, irq_status);
	}
	return err;
}

static int me0700_ai_postinit(me_subdevice_t* subdevice, void* args)
{/// DONE
	me0700_ai_subdevice_t* instance;

	instance = (me0700_ai_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	instance->ai_instance->base.dev = subdevice->dev;
	instance->ctrl_port->base.dev = subdevice->dev;
	instance->data_port->base.dev = subdevice->dev;
	instance->ext_irq->base.dev = subdevice->dev;

	instance->ai_instance->base.me_subdevice_postinit((struct me_subdevice* )instance->ai_instance, args);

	instance->ctrl_port->base.me_subdevice_postinit((struct me_subdevice* )instance->ctrl_port, args);
	instance->data_port->base.me_subdevice_postinit((struct me_subdevice* )instance->data_port, args);

	instance->ai_instance->base.me_subdevice_query_number_ranges((struct me_subdevice* )instance->ai_instance, ME_UNIT_ANY, &instance->me4600_ranges_number);

	instance->ai_instance->base.me_subdevice_query_number_channels((struct me_subdevice* )instance->ai_instance, &instance->number_channels);
	instance->number_channels -= 4;
	instance->number_channels += instance->number_I_channels;

	me0700_reset_relays(instance);

	// Reset subdevice.
	return me0700_ai_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);

}

me0700_ai_subdevice_t* me0700_ai_constr(void* reg_base,
											void* DMA_base,
											void* PLX_base,
											unsigned int idx,
											unsigned int channels, unsigned int ranges,
											int features,
											struct workqueue_struct* me0700_wq,
											me_general_dev_t* dev,
											me_lock_t* dio_lock,
											unsigned int I_channels)
{/// TODO DONE?
	me0700_ai_subdevice_t* subdevice;

	PDEBUG("executed. idx=0\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me0700_ai_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		goto ERROR;
	}

	// Initialize subdevice base class.
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		goto ERROR;
	}

	// Store analog input index.
	subdevice->base.idx = idx;

	//Initialize lock
	ME_INIT_LOCK(&subdevice->me0700_bus_lock);

	subdevice->ai_instance = me4600_ai_constr(reg_base, DMA_base, PLX_base, idx, channels, ranges, features, /*int_status,*/ me0700_wq);
	if (!subdevice->ai_instance)
	{
		PERROR("Cannot initialize 4600 AI object.\n");
		goto ERROR;
	}

	subdevice->ctrl_port = me4600_dio_constr(reg_base, 0, dio_lock, ME_TYPE_DO);
	if (!subdevice->ctrl_port)
	{
		PERROR("Cannot initialize 4600 DO object.\n");
		goto ERROR;
	}

	subdevice->data_port = me4600_dio_constr(reg_base, 1, dio_lock, ME_TYPE_DI);
	if (!subdevice->data_port)
	{
		PERROR("Cannot initialize 4600 DI object.\n");
		goto ERROR;
	}

	subdevice->ext_irq = me4600_ext_irq_constr(reg_base, 0);
	if (!subdevice->ext_irq)
	{
		PERROR("Cannot initialize 4600 EXT_IRQ object.\n");
		goto ERROR;
	}

	// Save the number of current lines.
	subdevice->number_I_channels = I_channels;

	subdevice->me4600_ranges_number = 0;
	subdevice->number_channels = 0;

	// Initialize ranges.
	// +/-500A
	subdevice->ranges[0].min = -500000000;
	subdevice->ranges[0].max = 499992370;
	// +/-50A
	subdevice->ranges[1].min = -50000000;
	subdevice->ranges[1].max = 49999237;
	// +/-25A
	subdevice->ranges[2].min = -25000000;
	subdevice->ranges[2].max = 24999237;
	// +/-2.5A
	subdevice->ranges[3].min = -2500000;
	subdevice->ranges[3].max = 2499924;
	// +/-0.25A
	subdevice->ranges[4].min = -250000;
	subdevice->ranges[4].max = 249992;
	// +/-0.025A
	subdevice->ranges[5].min = -25000;
	subdevice->ranges[5].max = 24999;
	// +/-0.0025A
	subdevice->ranges[6].min = -2500;
	subdevice->ranges[6].max = 2500;
	// +/-0.00025A
	subdevice->ranges[7].min = -250;
	subdevice->ranges[7].max = 250;

	// Override base class methods.
	subdevice->base.me_subdevice_destructor = me0700_ai_destructor;
	subdevice->base.me_subdevice_io_reset_subdevice = me0700_ai_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me0700_ai_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me0700_ai_io_single_read;
	subdevice->base.me_subdevice_io_stream_config = me0700_ai_io_stream_config;
	subdevice->base.me_subdevice_io_stream_new_values = me0700_ai_io_stream_new_values;
	subdevice->base.me_subdevice_io_stream_read = me0700_ai_io_stream_read;
	subdevice->base.me_subdevice_io_stream_start = me0700_ai_io_stream_start;
	subdevice->base.me_subdevice_io_stream_status = me0700_ai_io_stream_status;
	subdevice->base.me_subdevice_io_stream_stop = me0700_ai_io_stream_stop;
	subdevice->base.me_subdevice_io_irq_start = me0700_ai_io_irq_start;
	subdevice->base.me_subdevice_io_irq_wait = me0700_ai_io_irq_wait;
	subdevice->base.me_subdevice_io_irq_stop = me0700_ai_io_irq_stop;
	subdevice->base.me_subdevice_io_irq_test = me0700_ai_io_irq_test;
	subdevice->base.me_subdevice_query_number_channels = me0700_ai_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me0700_ai_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me0700_ai_query_subdevice_caps;
	subdevice->base.me_subdevice_query_subdevice_caps_args = me0700_ai_query_subdevice_caps_args;
	subdevice->base.me_subdevice_query_range_by_min_max = me0700_ai_query_range_by_min_max;
	subdevice->base.me_subdevice_query_number_ranges = me0700_ai_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = me0700_ai_query_range_info;
	subdevice->base.me_subdevice_query_timer = me0700_ai_query_timer;

	subdevice->base.me_subdevice_irq_handle = me0700_ai_irq_handle;

	subdevice->base.me_subdevice_postinit = me0700_ai_postinit;

	return subdevice;

ERROR:
	me0700_ai_destructor((me_subdevice_t* )subdevice);
	if (subdevice)
	{
		kfree(subdevice);
	}

	return NULL;
}

