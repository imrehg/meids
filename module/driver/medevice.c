/**
 * @file medevice.c
 *
 * @brief Meilhaus device base class.
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
# include <linux/workqueue.h>

# include "me_spin_lock.h"
# include "me_common.h"
# include "me_internal.h"
# include "me_defines.h"
# include "me_error.h"
# include "me_debug.h"

# if defined(ME_PCI)
#  include "me_plx9052_reg.h"
# endif

# if defined(ME_USB)
#  include <linux/wait.h>
#  include "NET2282_access.h"
#  include "mehardware_access.h"
# endif

# if defined(ME_COMEDI)
#  include "me_plx9052_reg.h"
# endif

# include "medevice.h"

/// Static headrs
static void me_device_destructor(me_device_t* me_device);

static int me_device_io_irq_start(me_device_t* device, struct file* filep, int subdevice, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
static int me_device_io_irq_wait(me_device_t* device, struct file* filep, int subdevice, int channel, int* irq_count, int* value, int time_out, int flags);
static int me_device_io_irq_stop(me_device_t* device, struct file* filep, int subdevice, int channel, int flags);
static int me_device_io_irq_test(me_device_t* device, struct file* filep, int subdevice, int channel, int flags);

static int me_device_io_reset_device(me_device_t* device, struct file* filep, int flags);
static int me_device_io_reset_subdevice(me_device_t* device, struct file* filep, int subdevice, int flags);

static int me_device_io_single_config(me_device_t* device, struct file* filep, int subdevice,
										int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
static int me_device_io_single_read(me_device_t* device, struct file* filep, int subdevice, int channel, int* value, int time_out, int flags);
static int me_device_io_single_write(me_device_t* device, struct file* filep, int subdevice, int channel, int value, int time_out, int flags);

/*static int me_device_io_stream_config(me_device_t* device, struct file* filep, int subdevice,
										meIOStreamConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);*/
static int me_device_io_stream_config(me_device_t* device, struct file* filep, int subdevice,
										meIOStreamSimpleConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);
static int me_device_io_stream_new_values(me_device_t* device, struct file* filep, int subdevice, int time_out, int* count, int flags);
static int me_device_io_stream_read(me_device_t* device, struct file* filep, int subdevice, int read_mode, int* values, int* count, int flags);
static int me_device_io_stream_timeout_read(me_device_t* device, struct file* filep, int subdevice,
											int read_mode, int* values, int* count, int time_out, int flags);
static int me_device_io_stream_start(me_device_t* device, struct file* filep, int subdevice, int start_mode, int time_out, int flags);
static int me_device_io_stream_status(me_device_t* device, struct file* filep, int subdevice, int wait, int* status, int* count, int flags);
static int me_device_io_stream_stop(me_device_t* device, struct file* filep, int subdevice, int stop_mode, int time_out, int flags);
static int me_device_io_stream_write(me_device_t* device, struct file* filep, int subdevice, int write_mode, int* values, int* count, int flags);
static int me_device_io_stream_timeout_write(me_device_t* device, struct file* filep, int subdevice,
												int write_mode, int* values, int* count, int time_out, int flags);

static int me_device_lock_device( me_device_t* device, struct file* filep, int lock, int flags);
static int me_device_lock_subdevice(me_device_t* device, struct file* filep, int subdevice, int lock, int flags);

static int me_device_query_description_device(me_device_t* device, char** description);
static int me_device_query_info_device(me_device_t* device,
									int* vendor_id, int* device_id, int* serial_no, int* bus_type, int* bus_no, int* dev_no, int* func_no, int* plugged);
static int me_device_query_name_device(me_device_t* device, char** name);
static int me_device_query_name_device_driver(me_device_t* device,char** name);
static int me_device_query_number_subdevices(me_device_t* device, int* number);
static int me_device_query_number_subdevices_by_type(me_device_t* device, int type, int subtype, int* number);
static int me_device_query_number_channels(me_device_t* device, int subdevice, int* number);
static int me_device_query_number_ranges(me_device_t* device, int subdevice, int unit, int* count);
static int me_device_query_range_by_min_max(me_device_t* device, int subdevice, int unit, int* min, int* max, int* maxdata, int* range);
static int me_device_query_range_info(me_device_t* device, int subdevice, int range, int* unit, int* min,int* max,int* maxdata);
static int me_device_query_subdevice_by_type(me_device_t* device, int start_subdevice, int type, int subtype, int* subdevice);
static int me_device_query_subdevice_type(me_device_t* device,int subdevice,int* type,int* subtype);
static int me_device_query_subdevice_caps(me_device_t* device,int subdevice, int* caps);
static int me_device_query_subdevice_caps_args( me_device_t* device, int subdevice, int cap, int* args, int* count);
static int me_device_query_timer(me_device_t* device, int subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);
static int me_device_query_version_device_driver( me_device_t* device, int* version);
static int me_device_query_device_release(me_device_t* device, int* version);
static int me_device_query_version_firmware(me_device_t* device, int subdevice, int* version);

static int me_device_config_load(me_device_t* device, struct file* filep, void* config, unsigned int size);
static int me_device_set_offset(me_device_t* device, struct file* filep, int subdevice, int channel, int range, int* offset, int flags);

/// Implementations
static int me_device_io_irq_start(me_device_t* device, struct file* filep, int subdevice, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

# if defined(ME_USB)
	if (!device->irq_context.irq_urb)
	{
		PERROR_CRITICAL("Interrupt URB not initialized!\n");
		return ME_ERRNO_INTERNAL;
	}
# endif

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_irq_start(
		          s,
		          filep,
		          channel,
		          irq_source,
		          irq_edge,
		          irq_arg,
		          flags);

	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_irq_wait(me_device_t* device, struct file* filep, int subdevice, int channel, int* irq_count, int* value, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.

	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_irq_wait(
		          s,
		          filep,
		          channel,
		          irq_count,
		          value,
		          time_out,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_irq_stop(me_device_t* device, struct file* filep, int subdevice, int channel, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

# if defined(ME_USB)
	if (!device->irq_context.irq_urb)
	{
		if(device->irq_context.complete_context.complete_status == 0x00)
		{
			PERROR_CRITICAL("Interrupt URB not initialized!\n");
			return ME_ERRNO_INTERNAL;
		}
		PDEBUG("USB device not connected!\n");
		return ME_ERRNO_SUCCESS;
	}
# endif

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_irq_stop(
		          s,
		          filep,
		          channel,
		          flags);

	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);
	return err;
}

static int me_device_io_irq_test(me_device_t* device, struct file* filep, int subdevice, int channel, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_irq_test(
		          s,
		          filep,
		          channel,
		          flags);

	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_io_reset_device(me_device_t* device, struct file* filep, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;
	int i;

	PDEBUG("executed.\n");

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Check subdevice locks.
	if (!(flags & ME_IO_RESET_DEVICE_UNPROTECTED))
	{
		err = me_dlock_lock(&device->dlock, &device->slist, filep, ME_LOCK_CHECK, ME_NO_FLAGS);

		if(err)
		{
			PERROR("Cannot reset device. Something is locked.\n");
			return err;
		}
	}

	// Reset every subdevice in list.
	for (i = 0; i < me_slist_get_number_subdevices(&device->slist); i++)
	{
		s = me_slist_get_subdevice(&device->slist, i);
		err = s->me_subdevice_io_reset_subdevice(s, filep, flags & ~ME_IO_RESET_DEVICE_UNPROTECTED);

		if (err && (err != ME_ERRNO_LOCKED))
		{
			PERROR("Cannot reset %d subdevice.\n", i);
		}
	}

	// Reset apply only for not blocked subdevices. err == ME_ERRNO_LOCKED is not an error.
	if (err == ME_ERRNO_LOCKED)
		err = ME_ERRNO_SUCCESS;

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_reset_subdevice(me_device_t* device, struct file* filep, int subdevice, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_reset_subdevice(
		          s,
		          filep,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_single_config(me_device_t* device, struct file* filep, int subdevice, int channel, int single_config,
    																		int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s = NULL;

	PDEBUG("executed.\n");

	// Check subdevice index.

	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_single_config(
		          s,
		          filep,
		          channel,
		          single_config,
		          ref,
		          trig_chain,
		          trig_type,
		          trig_edge,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_single_read(me_device_t* device, struct file* filep, int subdevice, int channel, int* value, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.

	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_single_read(
		          s,
		          filep,
		          channel,
		          value,
		          time_out,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_single_write(me_device_t* device, struct file* filep, int subdevice, int channel, int value, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.

	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_single_write(
		          s,
		          filep,
		          channel,
		          value,
		          time_out,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_config(me_device_t* device, struct file* filep, int subdevice, meIOStreamSimpleConfig_t* config_list, int count,
																		meIOStreamSimpleTriggers_t* trigger,  int fifo_irq_threshold, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_config(
		          s,
		          filep,
		          config_list,
		          count,
		          trigger,
		          fifo_irq_threshold,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_new_values(me_device_t* device, struct file* filep, int subdevice, int time_out, int* count, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_new_values(
		          s,
		          filep,
		          time_out,
		          count,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_timeout_read(me_device_t* device, struct file* filep, int subdevice, int read_mode, int* values, int* count, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_read(
		          s,
		          filep,
		          read_mode,
		          values,
		          count,
		          time_out,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_read(me_device_t* device, struct file* filep, int subdevice, int read_mode, int* values, int *count, int flags)
{
	PDEBUG("executed.\n");

	return me_device_io_stream_timeout_read(device, filep, subdevice, read_mode, values, count, 0, flags);
}

static int me_device_io_stream_start(me_device_t* device, struct file* filep, int subdevice, int start_mode, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

# if defined(ME_USB)
	if (!device->irq_context.irq_urb)
	{
		PERROR_CRITICAL("Interrupt URB not initialized!\n");
		return ME_ERRNO_INTERNAL;
	}
# endif

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}
	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_start(s, filep, start_mode, time_out, flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_status(me_device_t* device, struct file* filep, int subdevice, int wait, int* status, int* count, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_status(
		          s,
		          filep,
		          wait,
		          status,
		          count,
		          flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_stop(me_device_t* device, struct file* filep, int subdevice, int stop_mode, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_stop(s, filep, stop_mode, time_out, flags);
	}
	else
	{
		// Something really bad happened.
		PERROR_CRITICAL("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_timeout_write(me_device_t* device, struct file* filep, int subdevice, int write_mode, int* values,  int* count, int time_out, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	if (time_out < 0)
	{
		PERROR("Invalid timeout specified. Should be at least 0.\n");
		return ME_ERRNO_INVALID_TIMEOUT;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);

	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_io_stream_write(
					s,
					filep,
					write_mode,
					values,
					count,
					time_out,
					flags);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_io_stream_write(me_device_t* device, struct file* filep, int subdevice, int write_mode, int* values, int* count, int flags)
{
	PDEBUG("executed.\n");

	return me_device_io_stream_timeout_write(device, filep, subdevice, write_mode, values, count, 0, flags);
}

static int me_device_set_offset(me_device_t* device, struct file* filep, int subdevice, int channel, int range, int* offset, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_set_offset(s, filep, channel, range, offset, flags);
	}
	else
	{
		// Something really bad happened.
		PERROR_CRITICAL("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}


static int me_device_lock_device(me_device_t* device, struct file* filep, int lock, int flags)
{
	PDEBUG("executed.\n");

	return me_dlock_lock(
	           &device->dlock,
	           &device->slist,
	           filep,
	           lock ,
	           flags);
}

static int me_device_lock_subdevice(me_device_t* device, struct file* filep, int subdevice, int lock, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* instance;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	if (!(flags & ME_LOCK_FORCE))
	{
		// Check upper level.
		if ((device->dlock.filep != NULL) && (device->dlock.filep != filep))
		{
			PERROR("Device is locked by another process.\n");
			err = ME_ERRNO_LOCKED;
		}
	}

	if (!err)
	{
		// Get subdevice instance.
		instance = me_slist_get_subdevice(&device->slist, subdevice);
		if (instance)
		{
			// Call subdevice method.
			err = instance->me_subdevice_lock_subdevice(
					instance,
					filep,
					lock ,
					flags);
		}
		else
		{
			// Next paranoid check.
			PERROR_CRITICAL("Cannot get subdevice instance.\n");
			err = ME_ERRNO_INTERNAL;
		}
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int me_device_query_description_device(me_device_t* device, char** description)
{
	PDEBUG("executed.\n");
	*description = device->info.device_description;
	return ME_ERRNO_SUCCESS;
}

static int me_device_query_info_device(me_device_t* device, int* vendor_id, int* device_id, int* serial_no, int* bus_type, int* bus_no,  int* dev_no, int* func_no, int* plugged)
{
	PDEBUG("executed.\n");

	*bus_no = device->bus.bus_no;
	*dev_no = device->bus.dev_no;
	*func_no = device->bus.func_no;

	*vendor_id = device->bus.local_dev.vendor;
	*device_id = device->bus.local_dev.device;
	*serial_no = device->bus.local_dev.serial_no;

	*plugged = device->bus.plugged;

#if defined(ME_PCI)
	*bus_type =  ME_BUS_TYPE_PCI;
#elif defined(ME_USB)
	*bus_type = ME_BUS_TYPE_USB;
#elif defined(ME_COMEDI)
	*bus_type = ME_BUS_TYPE_PCI;
#elif defined(ME_MEPHISTO)
	*bus_type = ME_BUS_TYPE_USB;
#else
	//Only PCI and USB supported!
	*bus_type = ME_BUS_TYPE_INVALID;
#endif

	return ME_ERRNO_SUCCESS;
}

static int me_device_query_name_device(me_device_t* device, char** name)
{
	PDEBUG("executed.\n");
	*name = device->info.device_name;
	return ME_ERRNO_SUCCESS;
}

static int me_device_query_name_device_driver(me_device_t* device, char** name)
{
	PDEBUG("executed.\n");
	*name = device->info.driver_name;
	return ME_ERRNO_SUCCESS;
}

static int me_device_query_number_subdevices(me_device_t* device, int* number)
{
	PDEBUG("executed.\n");
	return me_slist_query_number_subdevices(&device->slist, number);
}

static int me_device_query_number_subdevices_by_type(me_device_t* device, int type, int subtype, int* number)
{
	PDEBUG("executed.\n");
	return me_slist_query_number_subdevices_by_type(&device->slist, type, subtype, number);
}

static int me_device_query_number_channels(me_device_t* device, int subdevice, int* number)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_number_channels(s, number);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_number_ranges(me_device_t* device, int subdevice, int unit, int* count)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_number_ranges(s, unit, count);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_range_by_min_max(me_device_t* device, int subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_range_by_min_max(
		          s,
		          unit,
		          min,
		          max,
		          maxdata,
		          range);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_range_info(me_device_t* device, int subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_range_info(
		          s,
		          range,
		          unit,
		          min,
		          max,
		          maxdata);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_subdevice_by_type(me_device_t* device, int start_subdevice, int type, int subtype, int* subdevice)
{
	PDEBUG("executed.\n");

	return me_slist_get_subdevice_by_type(
	           &device->slist,
	           start_subdevice,
	           type,
	           subtype,
	           subdevice);
}

static int me_device_query_subdevice_type(me_device_t* device, int subdevice, int* type, int* subtype)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_subdevice_type(s, type, subtype);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_subdevice_caps(me_device_t* device, int subdevice, int* caps)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_subdevice_caps(s, caps);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_subdevice_caps_args(me_device_t* device, int subdevice, int cap, int* args, int* count)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_subdevice_caps_args(
		          s,
		          cap,
		          args,
		          count);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_timer(me_device_t* device, int subdevice, int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);

	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_timer(
		          s,
		          timer,
		          base_frequency,
		          min_ticks,
		          max_ticks);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_query_version_device_driver(me_device_t* device, int* version)
{
	PDEBUG("executed.\n");
	*version = device->info.device_version;
	return ME_ERRNO_SUCCESS;
}

static int me_device_query_device_release(me_device_t* device, int* version)
{
	/// @todo Implement reading of release for each devices.
	PDEBUG("executed.\n");
	*version = ME_UNKNOWN_RELEASE;
	return ME_ERRNO_SUCCESS;
}

static int me_device_query_version_firmware(me_device_t* device, int subdevice, int* version)
{
	/// @todo Implement reading of firmware version in each subdevice.
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;

	PDEBUG("executed.\n");

	// Check subdevice index.
	if ((subdevice < 0) || (subdevice >= me_slist_get_number_subdevices(&device->slist)))
	{
		PERROR("Invalid subdevice.\n");
		return ME_ERRNO_INVALID_SUBDEVICE;
	}

	// Get subdevice instance.
	s = me_slist_get_subdevice(&device->slist, subdevice);
	if (s)
	{
		// Call subdevice method.
		err = s->me_subdevice_query_version_firmware(
		          s,
		          version);
	}
	else
	{
		// Something really bad happened.
		PERROR("Cannot get subdevice instance.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int me_device_config_load(me_device_t* device, struct file* filep, void* config, unsigned int size)
{
	PDEBUG("executed.\n");
	// If no need to configure anything than return success.
	return ME_ERRNO_SUCCESS;
}

static int me_device_postinit(me_device_t* device, struct file* filep)
{
	me_subdevice_t* s;
	int i;
	int err;
	int ret = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Call every subdevice in list.
	for (i = 0; i < me_slist_get_number_subdevices(&device->slist); i++)
	{
		s = me_slist_get_subdevice(&device->slist, i);
		err = s->me_subdevice_postinit(s, NULL);
		if (err)
		{
			PERROR_CRITICAL("Cannot run postinit for %d subdevice.\n", i);
			ret = err;
		}
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return ret;
}

/// Menagment
static void me_device_destructor(me_device_t* me_device)
{
	PDEBUG("executed.\n");

	if(me_device)
	{
		me_device_deinit(me_device);

		if(me_device->irq_context.subdevice)
		{
			kfree(me_device->irq_context.subdevice);
			me_device->irq_context.subdevice = NULL;
		}

		if(me_device->irq_context.int_status)
		{
			kfree(me_device->irq_context.int_status);
			me_device->irq_context.int_status = NULL;
		}

		if (me_device->info.custom_subname)
		{
			kfree(me_device->info.custom_subname);
			me_device->info.custom_subname = NULL;
		}
	}
}

int me_device_init(me_device_t* me_device, me_general_dev_t* hw_device)
{/// @note If this function fail than caller must take care of clearing.
	int err = ME_ERRNO_SUCCESS;
	int i;

	PDEBUG("executed.\n");

	// Initialize device list head.
	INIT_LIST_HEAD(&me_device->list);

	// Initialize device lock instance.
	me_dlock_init(&me_device->dlock);

	// Initialize subdevice list instance.
	me_slist_init(&me_device->slist);

	me_device->bus.plugged = ME_PLUGGED_INVALID;

	// Store the information for later usage.
	memcpy(&me_device->bus.local_dev, hw_device, sizeof(me_general_dev_t));

#if defined(ME_PCI)
	// Enable the pci device.
	err = pci_enable_device(me_device->bus.local_dev.dev);
	if (err < 0)
	{
		PERROR("Cannot enable PCI device.\n");
		return err;
	}

	// Request the PCI register regions.
	err = pci_request_regions(me_device->bus.local_dev.dev, me_device->info.driver_name);
	if (err < 0)
	{
		pci_disable_device(me_device->bus.local_dev.dev);
		PERROR("Cannot request PCI regions.\n");
		return err;
	}


	// Get PCI register bases and sizes.
	/// @note Useing "pci_iomap()", "pci_iounmap()" and "ioreadX() / iowriteX()" for supprt io and memmory mapped devices.
	/**
	* We encode the physical PIO addresses (0-0xffff) into the
	* pointer by offsetting them with a constant (0x10000) and
	* assuming that all the low addresses are always PIO. That means
	* we can do some sanity checks on the low bits, and don't
	* need to just take things for granted.

	#ifndef HAVE_ARCH_PIO_SIZE
		#define PIO_OFFSET      0x10000UL
		#define PIO_MASK        0x0ffffUL
		#define PIO_RESERVED    0x40000UL
	#endif

	*/
	for (i = 0; i < 6; i++)
	{
		me_device->bus.PCI_Base[i] = pci_iomap(me_device->bus.local_dev.dev, i, 0);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the PCI location.
	me_device->bus.dev_no = PCI_SLOT(me_device->bus.local_dev.dev->devfn);
	me_device->bus.func_no = PCI_FUNC(me_device->bus.local_dev.dev->devfn);
	me_device->bus.bus_no = me_device->bus.local_dev.dev->bus->number;

	PINFO("PCI IRQ      = %d\n", (me_device->bus.local_dev.dev)->irq);
	// Mark irq as unused.
	me_device->bus.local_dev.irq_no = 0;

#elif defined(ME_USB)
	// Get PCI register bases and sizes.
	for(i=0; i<6; i++)
	{
		uint32_t tmp;
		err = NET2282_PLX_cfg_read(&me_device->bus.local_dev, &tmp, (NET2282_PCIBASE0 + (i << 2)));

		if (err)
		{
			return err;
		}

		///Last bit is always set for ports. Remove it.
		me_device->bus.PCI_Base[i] = (void *)((long int)tmp & ~0x01);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the USB location.
	me_device->bus.dev_no = 0;
	me_device->bus.func_no = 0;
	me_device->bus.bus_no = hw_device->dev->bus->busnum;

#elif defined(ME_COMEDI)
	// Enable the pci device.
	err = pci_enable_device(me_device->bus.local_dev.dev);
	if (err < 0)
	{
		PERROR("Cannot enable PCI device.\n");
		return err;
	}

	// Request the PCI register regions.
	err = pci_request_regions(me_device->bus.local_dev.dev, me_device->info.driver_name);
	if (err < 0)
	{
		pci_disable_device(me_device->bus.local_dev.dev);
		PERROR("Cannot request PCI regions.\n");
		return err;
	}


	// Get PCI register bases and sizes.
	/// @note Useing "pci_iomap()", "pci_iounmap()" and "ioreadX() / iowriteX()" for supprt io and memmory mapped devices.
	/**
	* We encode the physical PIO addresses (0-0xffff) into the
	* pointer by offsetting them with a constant (0x10000) and
	* assuming that all the low addresses are always PIO. That means
	* we can do some sanity checks on the low bits, and don't
	* need to just take things for granted.

	#ifndef HAVE_ARCH_PIO_SIZE
		#define PIO_OFFSET      0x10000UL
		#define PIO_MASK        0x0ffffUL
		#define PIO_RESERVED    0x40000UL
	#endif

	*/
	for (i = 0; i < 6; i++)
	{
		me_device->bus.PCI_Base[i] = pci_iomap(me_device->bus.local_dev.dev, i, 0);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the PCI location.
	me_device->bus.dev_no = PCI_SLOT(me_device->bus.local_dev.dev->devfn);
	me_device->bus.func_no = PCI_FUNC(me_device->bus.local_dev.dev->devfn);
	me_device->bus.bus_no = me_device->bus.local_dev.dev->bus->number;

	PINFO("PCI IRQ      = %d\n", (me_device->bus.local_dev.dev)->irq);
	// Mark irq as unused.
	me_device->bus.local_dev.irq_no = 0;

#elif defined(ME_MEPHISTO)
	//Dummy. Clears warning.
	i=0;
	// Get the USB location.
	me_device->bus.dev_no = 0;
	me_device->bus.func_no = 0;
	me_device->bus.bus_no = hw_device->dev->bus->busnum;
# endif

	PINFO("PCI SLOT     = %d\n", me_device->bus.dev_no);
	PINFO("PCI FUNCTION = %d\n", me_device->bus.func_no);
	PINFO("PCI BUS      = %d\n", me_device->bus.bus_no);

	me_device->bus.plugged = ME_PLUGGED_IN;

	me_install_default_routines(me_device);

	// As default standard version.
	me_device->info.custom_subname = NULL;
	return err;
}

void me_install_default_routines(me_device_t* me_device)
{
	// Initialize method pointers.
 	me_device->me_device_reinit								= me_device_reinit;
	me_device->me_device_disconnect							= me_device_disconnect;
	me_device->me_device_destructor							= me_device_destructor;

	me_device->me_device_io_irq_start						= me_device_io_irq_start;
	me_device->me_device_io_irq_wait						= me_device_io_irq_wait;
	me_device->me_device_io_irq_stop						= me_device_io_irq_stop;
	me_device->me_device_io_irq_test						= me_device_io_irq_test;

	me_device->me_device_io_reset_device					= me_device_io_reset_device;
	me_device->me_device_io_reset_subdevice					= me_device_io_reset_subdevice;

	me_device->me_device_io_single_config					= me_device_io_single_config;
	me_device->me_device_io_single_read						= me_device_io_single_read;
	me_device->me_device_io_single_write					= me_device_io_single_write;

	me_device->me_device_io_stream_config					= me_device_io_stream_config;
	me_device->me_device_io_stream_new_values				= me_device_io_stream_new_values;
	me_device->me_device_io_stream_read						= me_device_io_stream_read;
	me_device->me_device_io_stream_timeout_read				= me_device_io_stream_timeout_read;
	me_device->me_device_io_stream_start					= me_device_io_stream_start;
	me_device->me_device_io_stream_status					= me_device_io_stream_status;
	me_device->me_device_io_stream_stop						= me_device_io_stream_stop;
	me_device->me_device_io_stream_write					= me_device_io_stream_write;
	me_device->me_device_io_stream_timeout_write			= me_device_io_stream_timeout_write;

	me_device->me_device_set_offset							= me_device_set_offset;

	me_device->me_device_lock_device						= me_device_lock_device;
	me_device->me_device_lock_subdevice						= me_device_lock_subdevice;

	me_device->me_device_query_description_device			= me_device_query_description_device;
	me_device->me_device_query_info_device					= me_device_query_info_device;
	me_device->me_device_query_name_device					= me_device_query_name_device;
	me_device->me_device_query_name_device_driver			= me_device_query_name_device_driver;
	me_device->me_device_query_number_subdevices			= me_device_query_number_subdevices;
	me_device->me_device_query_number_subdevices_by_type	= me_device_query_number_subdevices_by_type;
	me_device->me_device_query_number_channels				= me_device_query_number_channels;
	me_device->me_device_query_number_ranges				= me_device_query_number_ranges;
	me_device->me_device_query_range_by_min_max				= me_device_query_range_by_min_max;
	me_device->me_device_query_range_info					= me_device_query_range_info;
	me_device->me_device_query_subdevice_by_type			= me_device_query_subdevice_by_type;
	me_device->me_device_query_subdevice_type				= me_device_query_subdevice_type;
	me_device->me_device_query_subdevice_caps				= me_device_query_subdevice_caps;
	me_device->me_device_query_subdevice_caps_args			= me_device_query_subdevice_caps_args;
	me_device->me_device_query_timer						= me_device_query_timer;
	me_device->me_device_query_version_device_driver		= me_device_query_version_device_driver;
	me_device->me_device_query_device_release				= me_device_query_device_release;
	me_device->me_device_query_version_firmware				= me_device_query_version_firmware;

	me_device->me_device_config_load						= me_device_config_load;
	me_device->me_device_postinit							= me_device_postinit;

	me_device->irq_context.me_device_irq_handle				= NULL;
}

int me_device_reinit(me_device_t* me_device, me_general_dev_t* hw_device)
{
	int i;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

# if defined(ME_PCI)

	// Enable the pci device.
	if (pci_enable_device(hw_device->dev) < 0)
	{
		me_device->bus.plugged = ME_PLUGGED_INVALID;
		return -EIO;
	}
	// Request the PCI register regions.
	if (pci_request_regions(hw_device->dev, me_device->info.driver_name) < 0)
	{
		pci_disable_device(hw_device->dev);
		me_device->bus.plugged = ME_PLUGGED_INVALID;
		return -EIO;
	}
	// Store the PCI information for later usage.
	me_device->bus.local_dev.dev = hw_device->dev;

	// Get PCI register bases and sizes.
	for (i = 0; i < 6; i++)
	{
		me_device->bus.PCI_Base[i] = pci_iomap(hw_device->dev, i, 0);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the PCI location.
	me_device->bus.bus_no = hw_device->dev->bus->number;
	me_device->bus.dev_no = PCI_SLOT(hw_device->dev->devfn);
	me_device->bus.func_no = PCI_FUNC(hw_device->dev->devfn);

	// Get the interrupt request number.
	if (&me_device->irq_context)
	{
		if (me_device->irq_context.me_device_irq_handle && me_device->irq_context.int_status)
		{
			// Request interrupt line.
			err = request_irq(
					(me_device->bus.local_dev.dev)->irq,
					me_device->irq_context.me_device_irq_handle,
# if defined(IRQF_DISABLED) || defined(IRQF_SHARED)
#  if defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
			IRQF_DISABLED |
#  endif
#  if defined(IRQF_SHARED)
			IRQF_SHARED |
#  endif
# elif defined(SA_INTERRUPT) || defined(SA_SHIRQ)
#  if defined(SA_INTERRUPT)
			SA_INTERRUPT |
#  endif
#  if defined(SA_SHIRQ)
			SA_SHIRQ |
#  endif
# else
#  error 	Interrupt flags not defined!
# endif
			0,
			me_device->info.driver_name,
			(void *) &me_device->irq_context);

			if (err)
			{
				PERROR("Cannot registred interrupt number %d.\n", (me_device->bus.local_dev.dev)->irq);
			}
			else
			{
				PINFO("Registered irq=%d.\n", (me_device->bus.local_dev.dev)->irq);
				me_device->bus.local_dev.irq_no = (me_device->bus.local_dev.dev)->irq;
			}

		}
	}
	PINFO("PCI IRQ      = %d\n", (me_device->bus.local_dev.dev)->irq);
# elif defined(ME_USB)
	// Store the USB information for later usage.
	memcpy(&me_device->bus.local_dev, hw_device, sizeof(struct NET2282_usb_device));

	// Get PCI register bases and sizes.
	for(i=0; i<6; i++)
	{
		uint32_t tmp;
		err = NET2282_PLX_cfg_read(&me_device->bus.local_dev, &tmp, (NET2282_PCIBASE0 + (i << 2)));
		if (err)
		{
			me_device->bus.plugged = ME_PLUGGED_INVALID;
			return -EIO;
		}
		/// Last bit is always set for ports. Remove it.
		me_device->bus.PCI_Base[i] = (void *)((long int)tmp & ~0x01);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the USB location.
	me_device->bus.bus_no = hw_device->dev->bus->busnum;
	me_device->bus.dev_no = 0;
	me_device->bus.func_no = 0;

	me_device->irq_context.irq_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(me_device->irq_context.irq_urb)
	{
		usb_fill_int_urb(	me_device->irq_context.irq_urb,
							me_device->bus.local_dev.dev,
							usb_rcvintpipe(me_device->bus.local_dev.dev, NET2282_EP_IRQ),
							&me_device->irq_context.statin_val.IRQSTAT,
							sizeof(interrupt_usb_struct_t),
							(usb_complete_t)NET2282_IRQ_complete,
							(void *)&me_device->irq_context,
							ME_IRQ_INTERVAL);
	}
# elif defined(ME_COMEDI)
	// Enable the pci device.
	if (pci_enable_device(hw_device->dev) < 0)
	{
		me_device->bus.plugged = ME_PLUGGED_INVALID;
		return -EIO;
	}
	// Request the PCI register regions.
	if (pci_request_regions(hw_device->dev, me_device->info.driver_name) < 0)
	{
		pci_disable_device(hw_device->dev);
		me_device->bus.plugged = ME_PLUGGED_INVALID;
		return -EIO;
	}
	// Store the PCI information for later usage.
	me_device->bus.local_dev.dev = hw_device->dev;

	// Get PCI register bases and sizes.
	for (i = 0; i < 6; i++)
	{
		me_device->bus.PCI_Base[i] = pci_iomap(hw_device->dev, i, 0);
		PINFO("PCI Base[%d]  = 0x%p\n", i, me_device->bus.PCI_Base[i]);
	}

	// Get the PCI location.
	me_device->bus.bus_no = hw_device->dev->bus->number;
	me_device->bus.dev_no = PCI_SLOT(hw_device->dev->devfn);
	me_device->bus.func_no = PCI_FUNC(hw_device->dev->devfn);

	// Get the interrupt request number.
	if (&me_device->irq_context)
	{
		if (me_device->irq_context.me_device_irq_handle && me_device->irq_context.int_status)
		{
			// Request interrupt line.
			err = request_irq(
					(me_device->bus.local_dev.dev)->irq,
					me_device->irq_context.me_device_irq_handle,
# if defined(IRQF_DISABLED) || defined(IRQF_SHARED)
#  if defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
			IRQF_DISABLED |
#  endif
#  if defined(IRQF_SHARED)
			IRQF_SHARED |
#  endif
# elif defined(SA_INTERRUPT) || defined(SA_SHIRQ)
#  if defined(SA_INTERRUPT)
			SA_INTERRUPT |
#  endif
#  if defined(SA_SHIRQ)
			SA_SHIRQ |
#  endif
# else
#  error 	Interrupt flags not defined!
# endif
			0,
			me_device->info.driver_name,
			(void *) &me_device->irq_context);

			if (err)
			{
				PERROR("Cannot registred interrupt number %d.\n", (me_device->bus.local_dev.dev)->irq);
			}
			else
			{
				PINFO("Registered irq=%d.\n", (me_device->bus.local_dev.dev)->irq);
				me_device->bus.local_dev.irq_no = (me_device->bus.local_dev.dev)->irq;
			}

		}
	}
	PINFO("PCI IRQ      = %d\n", (me_device->bus.local_dev.dev)->irq);
# elif defined(ME_MEPHISTO)
	// Store the USB information for later usage.
	memcpy(&me_device->bus.local_dev, hw_device, sizeof(struct mephisto_usb_device));

	// Dummy. Clears warning
	i=0;

	// Get the USB location.
	me_device->bus.bus_no = hw_device->dev->bus->busnum;
	me_device->bus.dev_no = 0;
	me_device->bus.func_no = 0;

# endif

	PINFO("PCI SLOT     = %d\n", me_device->bus.dev_no);
	PINFO("PCI FUNCTION = %d\n", me_device->bus.func_no);
	PINFO("PCI BUS      = %d\n", me_device->bus.bus_no);

	me_device->bus.plugged = ME_PLUGGED_IN;

	return err;
}

void me_device_deinit(me_device_t* me_device)
{
	PDEBUG("executed.\n");

	if (me_device && (me_device->bus.plugged == ME_PLUGGED_IN))
	{
#if defined(ME_USB)
		if(me_device->irq_context.irq_urb)
		{
			if(me_device->irq_context.complete_context.complete_status != 0x00)
			{
				PINFO("Cancel IRQ task.\n");
				if(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
				{
					PDEBUG("Destroying IRQ URB.\n");
					usb_kill_urb(me_device->irq_context.irq_urb);
					set_current_state(TASK_UNINTERRUPTIBLE);
					schedule_timeout(1);
				}

				if(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
				{
					int i=0;
					while(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
					{// Paranoid cancel.
						PERROR("Can not destroy IRQ URB! %x\n", me_device->irq_context.complete_context.complete_status);
						me_device->irq_context.complete_context.complete_status = USB_CANCEL_INTERRUPT;
						wake_up_interruptible_all(&me_device->irq_context.complete_context.irq_internal_queue);
						set_current_state(TASK_UNINTERRUPTIBLE);
						schedule_timeout(1);
						if (++i > (10*HZ))
						{
							PERROR_CRITICAL("Can not cancel IRQ task! Emergency exit.\n");
							me_device->irq_context.complete_context.complete_status = USB_ENDED_INTERRUPT;
							break;
						}
					}
				}

				if(me_device->irq_context.complete_context.complete_status == USB_ENDED_INTERRUPT)
				{
					PDEBUG("IRQ URB Destroyed.\n");
				}
			}
			else
			{
				PERROR_CRITICAL("IRQ task is not initializated!");
			}

			usb_free_urb(me_device->irq_context.irq_urb);
			me_device->irq_context.irq_urb = NULL;

			cancel_delayed_work(&me_device->irq_context.irq_task);
			if (me_device->irq_context.irq_queue)
			{
				flush_workqueue(me_device->irq_context.irq_queue);
				destroy_workqueue(me_device->irq_context.irq_queue);
				me_device->irq_context.irq_queue = NULL;
			}
		}
#endif
	}

	me_slist_deinit(&me_device->slist);
	me_dlock_deinit(&me_device->dlock);
}

void me_device_disconnect(me_device_t* me_device)
{
	int i;

	PDEBUG("executed.\n");

	if (!me_device)
	{
		PERROR_CRITICAL("No device!");
		return;
	}

# if defined(ME_PCI)

	/// Disable interrupts on PLX
	iowrite32(ME_PLX9052_PCI_INTS_BLOCKED, me_device->bus.PCI_Base[1] + PLX9052_INTCSR);
	PDEBUG_REG("me_writel(0x%p : 0x%x)=0\n",	(void *)(me_device->bus.PCI_Base[1] + PLX9052_INTCSR), ME_PLX9052_PCI_INTS_BLOCKED);

	for(i=0; i<6; i++)
	{
		if (me_device->bus.PCI_Base[i])
		{
			pci_iounmap(me_device->bus.local_dev.dev, me_device->bus.PCI_Base[i]);
		}
	}

	if (me_device->bus.local_dev.irq_no)
	{
		free_irq(me_device->bus.local_dev.irq_no, (void *) &me_device->irq_context);
		me_device->bus.local_dev.irq_no = 0;
	}
	pci_release_regions(me_device->bus.local_dev.dev);
	pci_disable_device(me_device->bus.local_dev.dev);

# elif defined(ME_USB)

	if (me_device->irq_context.irq_urb)
	{
		if(me_device->irq_context.complete_context.complete_status != 0x00)
		{
			PINFO("Cancel IRQ task.\n");
			if(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
			{
				PDEBUG("Destroying IRQ URB.\n");
				usb_kill_urb(me_device->irq_context.irq_urb);
				set_current_state(TASK_UNINTERRUPTIBLE);
				schedule_timeout(1);
			}

			if(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
			{
				int i=0;
				while(me_device->irq_context.complete_context.complete_status != USB_ENDED_INTERRUPT)
				{// Paranoid cancel.
					me_device->irq_context.complete_context.complete_status = USB_CANCEL_INTERRUPT;
					wake_up_interruptible_all(&me_device->irq_context.complete_context.irq_internal_queue);
					set_current_state(TASK_UNINTERRUPTIBLE);
					schedule_timeout(1);
					if (++i > (10*HZ))
					{
						PERROR_CRITICAL("Can not cancel IRQ task! Emergency exit.\n");
						me_device->irq_context.complete_context.complete_status = USB_ENDED_INTERRUPT;
						break;
					}
				}
			}

			if(me_device->irq_context.complete_context.complete_status == USB_ENDED_INTERRUPT)
			{
				PDEBUG("IRQ URB Destroyed.\n");
			}
		}
		else
		{
			PERROR_CRITICAL("IRQ task is not initializated!");
		}

		usb_free_urb(me_device->irq_context.irq_urb);
		me_device->irq_context.irq_urb = NULL;

		cancel_delayed_work(&me_device->irq_context.irq_task);

		if (me_device->irq_context.irq_queue)
		{
			flush_workqueue(me_device->irq_context.irq_queue);
			destroy_workqueue(me_device->irq_context.irq_queue);
			me_device->irq_context.irq_queue = NULL;
		}
	}

# elif defined(ME_COMEDI)

	/// Disable interrupts on PLX
	iowrite32(ME_PLX9052_PCI_INTS_BLOCKED, me_device->bus.PCI_Base[1] + PLX9052_INTCSR);
	PDEBUG_REG("me_writel(0x%p : 0x%x)=0\n",	(void *)(me_device->bus.PCI_Base[1] + PLX9052_INTCSR), ME_PLX9052_PCI_INTS_BLOCKED);

	for(i=0; i<6; i++)
	{
		if (me_device->bus.PCI_Base[i])
		{
			pci_iounmap(me_device->bus.local_dev.dev, me_device->bus.PCI_Base[i]);
		}
	}

	if (me_device->bus.local_dev.irq_no)
	{
		free_irq(me_device->bus.local_dev.irq_no, (void *) &me_device->irq_context);
		me_device->bus.local_dev.irq_no = 0;
	}
	pci_release_regions(me_device->bus.local_dev.dev);
	pci_disable_device(me_device->bus.local_dev.dev);
# endif

	me_device->bus.local_dev.dev = NULL;
	me_device->bus.bus_no = ~0;
	me_device->bus.dev_no = ~0;
	me_device->bus.func_no = ~0;

	for (i = 0; i < 6; i++)
	{
		me_device->bus.PCI_Base[i] = 0;
	}
}

int me_device_init_irq(me_device_t* me_device)
{
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	if (me_device->irq_context.no_subdev <= 0)
	{
		PINFO("No interrupt's sources registred. (%d)\n", me_device->irq_context.no_subdev);
		return ME_ERRNO_SUCCESS;
	}

# if defined(ME_PCI)
	PINFO("PCI IRQ = %d.\n", (me_device->bus.local_dev.dev)->irq);
	// Request interrupt line.
	err = request_irq(
			(me_device->bus.local_dev.dev)->irq,
			me_device->irq_context.me_device_irq_handle,
# if defined(IRQF_DISABLED) || defined(IRQF_SHARED)
#  if defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
			IRQF_DISABLED |
#  endif
#  if defined(IRQF_SHARED)
			IRQF_SHARED |
#  endif
# elif defined(SA_INTERRUPT) || defined(SA_SHIRQ)
#  if defined(SA_INTERRUPT)
			SA_INTERRUPT |
#  endif
#  if defined(SA_SHIRQ)
			SA_SHIRQ |
#  endif
# else
#  error 	Interrupt flags not defined!
# endif
			0,
			me_device->info.driver_name,
			(void *) &me_device->irq_context);
	if (err)
	{
		PERROR("Cannot registred interrupt number %d.\n", (me_device->bus.local_dev.dev)->irq);
		err = ME_ERRNO_INTERNAL;
	}
	else
	{
		PINFO("Registered irq=%d.\n", (me_device->bus.local_dev.dev)->irq);
		me_device->bus.local_dev.irq_no = (me_device->bus.local_dev.dev)->irq;
	}
# endif	//defined(ME_PCI)

# if defined(ME_USB)
	init_waitqueue_head(&me_device->irq_context.complete_context.irq_internal_queue);

	me_device->irq_context.irq_queue = create_workqueue(me_device->info.driver_name);

	// workqueue API changed in kernel 2.6.20
#  if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&me_device->irq_context.irq_task, NET2282_IRQ_handle, (void *)me_device);
#  else
	INIT_DELAYED_WORK(&me_device->irq_context.irq_task, NET2282_IRQ_handle);
#  endif
	init_MUTEX(&(me_device->bus.local_dev.usb_IRQ_semaphore));

	me_device->irq_context.irq_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(me_device->irq_context.irq_urb)
	{
		usb_fill_int_urb(	me_device->irq_context.irq_urb,
							me_device->bus.local_dev.dev,
							usb_rcvintpipe(me_device->bus.local_dev.dev, NET2282_EP_IRQ),
							&me_device->irq_context.statin_val.IRQSTAT,
							sizeof(interrupt_usb_struct_t),
							(usb_complete_t)NET2282_IRQ_complete,
							(void *)&me_device->irq_context,
							ME_IRQ_INTERVAL);

		PINFO("IRQ URB has been created.\n");

		queue_delayed_work(me_device->irq_context.irq_queue, &me_device->irq_context.irq_task, 0);

	}
	else
	{
		PERROR_CRITICAL("Cannot get memory for interrupt urb.\n");
		err = -ENOMEM;
	}
# endif	// defined(ME_USB)

# if defined(ME_COMEDI)
	PINFO("IRQ = %d.\n", (me_device->bus.local_dev.dev)->irq);
	// Request interrupt line.
	err = request_irq(
			(me_device->bus.local_dev.dev)->irq,
			me_device->irq_context.me_device_irq_handle,
# if defined(IRQF_DISABLED) || defined(IRQF_SHARED)
#  if defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
			IRQF_DISABLED |
#  endif
#  if defined(IRQF_SHARED)
			IRQF_SHARED |
#  endif
# elif defined(SA_INTERRUPT) || defined(SA_SHIRQ)
#  if defined(SA_INTERRUPT)
			SA_INTERRUPT |
#  endif
#  if defined(SA_SHIRQ)
			SA_SHIRQ |
#  endif
# else
#  error 	Interrupt flags not defined!
# endif
			0,
			me_device->info.driver_name,
			(void *) &me_device->irq_context);
	if (err)
	{
		PERROR("Cannot registred interrupt number %d.\n", (me_device->bus.local_dev.dev)->irq);
		err = ME_ERRNO_INTERNAL;
	}
	else
	{
		PINFO("Registered irq=%d.\n", (me_device->bus.local_dev.dev)->irq);
		me_device->bus.local_dev.irq_no = (me_device->bus.local_dev.dev)->irq;
	}
# endif	//defined(ME_COMEDI)

	return err;
}

# if defined(ME_USB)
void NET2282_IRQ_handle(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
						void* device
#else
						struct work_struct* work
#endif
						)
{
	uint32_t status = USB_ENDED_INTERRUPT;
    int err = ME_ERRNO_SUCCESS;
    irqreturn_t (*me_isr)(int irq, void* context
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
						, struct pt_regs* regs
# endif
					);
    me_device_t* me_device;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	me_device = (me_device_t *)device;
#else
	me_device = container_of((void *)work, me_device_t, irq_context.irq_task);
#endif

	if (!me_device)
	{
		return;
	}

	me_isr = me_device->irq_context.me_device_irq_handle;

	PDEBUG("USB IRQ TASK STARTED\n");

	if(!me_device->irq_context.irq_urb)
	{//Paranoid check. This should never, ever happend!
		PERROR_CRITICAL("IRQ URB wasn't created!\n");
		me_device->irq_context.complete_context.complete_status = USB_ENDED_INTERRUPT;
		return;
	}

	// Start interrupt endpoint
	usb_clear_halt(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, usb_rcvintpipe(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, NET2282_EP_IRQ));
	usb_clear_halt(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, usb_sndintpipe(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, NET2282_EP_IRQ));
	NET2282_NET2282_reg_write(&me_device->bus.local_dev, 0, NET2282_USBIRQENB1);
	NET2282_NET2282_reg_write(&me_device->bus.local_dev, ~NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE, NET2282_IRQSTAT1);
	NET2282_NET2282_reg_write(&me_device->bus.local_dev, NET2282_USB_INTERRUPT_ENABLE | NET2282_PCI_INTA_INTERRUPT_ENABLE, NET2282_USBIRQENB1);

	me_device->irq_context.complete_context.complete_status = USB_CONTEXT_INTERRUPT;
	err = usb_submit_urb(me_device->irq_context.irq_urb, GFP_KERNEL);
	if(err)
	{
		PERROR("Couldn't submit IRQ URB: %d\n", err);
		me_device->irq_context.complete_context.complete_status = EPIPE;
	}
	else
	{
		PDEBUG("IRQ URB has been submited.\n");
	}

	while(1)
	{
		PINFO("USB IRQ TASK: Going to sleep.\n");

/*		if(wait_event_interruptible(me_device->irq_context.complete_context.irq_internal_queue,
									me_device->irq_context.complete_context.complete_status != USB_CONTEXT_INTERRUPT) < 0)*/
		if(wait_event_interruptible_timeout(me_device->irq_context.complete_context.irq_internal_queue,
									me_device->irq_context.complete_context.complete_status != USB_CONTEXT_INTERRUPT, HZ) < 0)
		{
			err = -ECANCELED;
			PINFO("USB IRQ TASK: Cancel.\n");
			break;
		}

		if (signal_pending(current))
		{
			err = -ESHUTDOWN;
			PINFO("USB IRQ TASK: Shutdown.\n");
			break;
		}

		status = me_device->irq_context.complete_context.complete_status;
		switch (status)
		{

			case USB_CONTEXT_INTERRUPT:
				// This should never, ever happend!
// 				PERROR_CRITICAL("%ld: USB INTERRUPT CONTEXT CALL!\n", jiffies);

				// New conception - security loop
				PINFO("%ld: USB INTERRUPT CONTEXT CALL!\n", jiffies);
				break;

			case EINPROGRESS:
				// This should never, ever happend!
				PERROR_CRITICAL("%ld: Still in progress!\n", jiffies);
				break;

			case USB_CANCEL_INTERRUPT:
				PDEBUG("%ld: USB INTERRUPT CANCEL CALL!\n", jiffies);
				err = ECANCELED;
				break;

			case EOVERFLOW:
				PERROR("URB internal buffer overflow.\n");
			case 0:
				PDEBUG("%ld: USB IRQ TASK: %s\n", jiffies, (!status) ? "IRQ detected." : "Ignore it and procced as correct one.");
				ME_IRQ_LOCK(me_device->bus.local_dev.usb_IRQ_semaphore);
				// Handle IRQs

					err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, 0, NET2282_USBIRQENB1);
					if (me_isr)
					{
						me_isr(0, (void *)&me_device->irq_context
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
											, NULL
# endif
								);

					}
					else
					{
						PERROR("%ld: NET2282 IS NOT accessable! (err=%d)\n", jiffies, err);

					}

					if (!err)
						err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, ~NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE, NET2282_IRQSTAT1);
					if (!err)
						err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, NET2282_USB_INTERRUPT_ENABLE | NET2282_PCI_INTA_INTERRUPT_ENABLE, NET2282_USBIRQENB1);

					if(!err)
					{
						me_device->irq_context.complete_context.complete_status = USB_CONTEXT_INTERRUPT;
						err = usb_submit_urb(me_device->irq_context.irq_urb, GFP_KERNEL);
					}
					if(err)
					{
						PERROR("Couldn't resubmit IRQ URB: %d\n", err);
						me_device->irq_context.complete_context.complete_status = EPIPE;
					}
					else
					{
						PDEBUG("IRQ URB has been resubmited.\n");
					}
				ME_IRQ_UNLOCK(me_device->bus.local_dev.usb_IRQ_semaphore);
				break;

			case USB_GHOST_INTERRUPT:
				PDEBUG("%ld: Ghost call. NET2282 internal error.\n", jiffies);
				err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, 0, NET2282_USBIRQENB1);
				if (!err)
					err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, ~NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE, NET2282_IRQSTAT1);
				if (!err)
					err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, NET2282_USB_INTERRUPT_ENABLE | NET2282_PCI_INTA_INTERRUPT_ENABLE, NET2282_USBIRQENB1);
				if (!err)
				{
					me_device->irq_context.complete_context.complete_status = USB_CONTEXT_INTERRUPT;
					err = usb_submit_urb(me_device->irq_context.irq_urb, GFP_KERNEL);
					if(err)
					{
						PERROR("Couldn't resubmit IRQ URB: %d\n", err);
						me_device->irq_context.complete_context.complete_status = EPIPE;
					}
					else
					{
						PDEBUG("IRQ URB has been resubmited.\n");
					}
				}
				break;

			case ENOENT:
				err = ENOENT;
				PDEBUG("%ld: usb_kill_usb!\n", jiffies);
				break;

			case ECONNRESET:
				err = ECONNRESET;
				PDEBUG("%ld: usb_unlink_urb!\n", jiffies);
				break;

			case ECOMM:
			case EPROTO:
			case EILSEQ:
			case EPIPE:
				PERROR("%ld: Transmision error %d!\n", jiffies, status);
				err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, 0, NET2282_USBIRQENB1);
				if (!err)
					err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, ~NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE, NET2282_IRQSTAT1);
				if (!err)
					err = NET2282_NET2282_reg_write(&me_device->bus.local_dev, NET2282_USB_INTERRUPT_ENABLE | NET2282_PCI_INTA_INTERRUPT_ENABLE, NET2282_USBIRQENB1);

				if (!err)
				{
					usb_clear_halt(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, usb_rcvintpipe(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, NET2282_EP_IRQ));
					usb_clear_halt(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, usb_sndintpipe(((struct NET2282_usb_device *)&me_device->bus.local_dev)->dev, NET2282_EP_IRQ));

					me_device->irq_context.complete_context.complete_status = USB_CONTEXT_INTERRUPT;
					err = usb_submit_urb(me_device->irq_context.irq_urb, GFP_KERNEL);
					if(err)
					{
						PERROR("Couldn't resubmit IRQ URB: %d\n", err);
						me_device->irq_context.complete_context.complete_status = EPIPE;
					}
					else
					{
						PDEBUG("IRQ URB has been resubmited.\n");
					}
				}
				break;

			case ESHUTDOWN:
				err = ESHUTDOWN;
				PDEBUG("%ld: SHUTDOWN or  NO DEVICE CONNECTED!\n", jiffies);
				break;

			case ENODEV:
				err = ENODEV;
				PDEBUG("%ld: NO DEVICE CONNECTED!\n", jiffies);
				break;

			case EINVAL:
				err = EINVAL;
				PERROR_CRITICAL("%ld: URB is incorrectly set!\n", jiffies);
				break;

			default:
				PERROR_CRITICAL("WRONG status=%d (0x%08x). Continue to normal work.\n", status, status);
		}

		if (err)
		{
			break;
		}
	}

	me_device->irq_context.complete_context.complete_status = USB_ENDED_INTERRUPT;
	PDEBUG("NET2282 IRQ TASK ENDED %d.\n", err);
}

void NET2282_IRQ_complete(struct urb* urb, struct pt_regs* regs)
{
	me_irq_context_t* irq_context = (me_irq_context_t *)urb->context;
	uint32_t status = irq_context->complete_context.complete_status;

	switch (status)
	{
		case 0:
			PERROR_CRITICAL("USB INTERRUPT task is not instaled!\n");
			return;
			break;

		case USB_ENDED_INTERRUPT:
			PERROR("USB INTERRUPT task is deinstaled!\n");
			return;
			break;

		case USB_CANCEL_INTERRUPT:
			PDEBUG("USB INTERRUPT task is under deinstalation procedure!\n");
			return;
			break;

		case USB_CONTEXT_INTERRUPT:
			break;

		default:
			PDEBUG("USB INTERRUPT CLASHED! STATUS:%x URB STATUS:%x\n", status, -urb->status);
	}

	if ((!urb->status) && (!(irq_context->statin_val.IRQSTAT && NET2282_PCI_INTA_INTERRUPT_ENABLE)))
	{
		irq_context->complete_context.complete_status = USB_GHOST_INTERRUPT;
	}
	else
	{
		irq_context->complete_context.complete_status = -urb->status;
		irq_context->complete_context.complete_status &= 0x0000FFFF;
	}


	if (!irq_context->complete_context.complete_status)
	{
		PDEBUG("USB INTERRUPT DETECTED!\n");
	}
	else
	{
		PDEBUG("USB INTERRUPT STATUS:%x=>%x URB STATUS:%x\n", status, irq_context->complete_context.complete_status, -urb->status);
	}

	wake_up_interruptible_all(&irq_context->complete_context.irq_internal_queue);
}

# endif
