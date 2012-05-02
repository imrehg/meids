/**
 * @file mesubdevice.c
 *
 * @brief Subdevice base class implemention.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
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
# include <asm/uaccess.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"

# include "me_spin_lock.h"

# include "mesubdevice.h"

static int me_subdevice_io_irq_start(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int irq_source,
    int irq_edge,
    int irq_arg,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_irq_wait(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int* irq_count,
    int* value,
    int time_out,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_irq_stop(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_irq_test(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_reset_subdevice(
	me_subdevice_t* subdevice,
	struct file* filep,
	int flags)
{
 	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	return ME_ERRNO_SUCCESS;
}


static int me_subdevice_io_single_config(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int single_config,
    int ref,
    int trig_chain,
    int trig_type,
    int trig_edge,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_single_read(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int* value,
    int time_out,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_single_write(
    me_subdevice_t* subdevice,
    struct file* filep,
    int channel,
    int value,
    int time_out,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}

static int me_subdevice_io_stream_config(
    me_subdevice_t* subdevice,
    struct file* filep,
    meIOStreamSimpleConfig_t *config_list,
    int count,
    meIOStreamSimpleTriggers_t *trigger,
    int fifo_irq_threshold,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}

static int me_subdevice_io_stream_new_values(
    me_subdevice_t* subdevice,
    struct file* filep,
    int time_out,
    int *count,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}

static int me_subdevice_io_stream_read(
    me_subdevice_t* subdevice,
    struct file* filep,
    int read_mode,
    int* values,
    int *count,
    int timeout,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_stream_start(
    me_subdevice_t* subdevice,
    struct file* filep,
    int start_mode,
    int time_out,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_stream_status(
    me_subdevice_t* subdevice,
    struct file* filep,
    int wait,
    int *status,
    int *count,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_stream_stop(
    me_subdevice_t* subdevice,
    struct file* filep,
    int stop_mode,
    int time_out,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_io_stream_write(
    me_subdevice_t* subdevice,
    struct file* filep,
    int write_mode,
    int* values,
    int* count,
    int timeout,
    int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_set_offset(struct me_subdevice* subdevice, struct file* filep,
		   								int channel, int range, int* offset, int flags)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_lock_subdevice(
    me_subdevice_t* subdevice,
    struct file* filep,
    int lock ,
    int flags)
{
	PDEBUG("executed.\n");
	return me_slock_lock(&subdevice->lock , filep, lock, flags);
}


static int me_subdevice_query_number_channels(
	me_subdevice_t* subdevice,
	int *number)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_number_ranges(
	me_subdevice_t* subdevice,
	int unit,
	int *count)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_range_by_min_max(
    me_subdevice_t* subdevice,
    int unit,
    int *min,
    int *max,
    int *maxdata,
    int *range)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_range_info(
    me_subdevice_t* subdevice,
    int range,
    int *unit,
    int *min,
    int *max,
    int *maxdata)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_subdevice_type(
	me_subdevice_t* subdevice,
	int *type,
	int *subtype)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");
	*caps = 0;
	return ME_ERRNO_SUCCESS;
}


static int me_subdevice_query_subdevice_caps_args(
    me_subdevice_t* subdevice,
    int cap,
    int* args,
    int* count)
{
	PDEBUG("executed.\n");
	*count = 0;
	*args = 0;
	return ME_ERRNO_NOT_SUPPORTED;
}


static int me_subdevice_query_timer(
    me_subdevice_t* subdevice,
    int timer,
    int* base_frequency,
    uint64_t* min_ticks,
    uint64_t* max_ticks)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_NOT_SUPPORTED;
}

static int me_subdevice_query_version_firmware(
    me_subdevice_t* subdevice,
    int* version)
{
	PDEBUG("executed.\n");
	version = 0;
	return ME_ERRNO_SUCCESS;
}

static int me_subdevice_config_load(me_subdevice_t* subdevice, struct file* filep, void* config)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_SUCCESS;
}

static int me_subdevice_postinit(me_subdevice_t* subdevice, void* args)
{
	PDEBUG("executed.\n");

	// By default just do unconditional reset.
	return subdevice->me_subdevice_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
}

static int me_subdevice_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{
	PDEBUG("executed.\n");
	return ME_ERRNO_SUCCESS;
}

static void me_subdevice_destructor(me_subdevice_t* subdevice)
{
	PDEBUG("executed.\n");
	me_subdevice_deinit(subdevice);
}

int me_subdevice_init(me_subdevice_t* subdevice)
{
	int err = 0;

	PDEBUG("executed.\n");

	// Init list head
	INIT_LIST_HEAD(&subdevice->list);

	// Initialize the subdevice lock instance
	err = me_slock_init(&subdevice->lock);
	if (err)
	{
		PERROR("Cannot initialize subdevice lock instance.\n");
	}
	else
	{
		// Subdevice base class methods.
		subdevice->me_subdevice_io_irq_start = me_subdevice_io_irq_start;
		subdevice->me_subdevice_io_irq_wait = me_subdevice_io_irq_wait;
		subdevice->me_subdevice_io_irq_stop = me_subdevice_io_irq_stop;
		subdevice->me_subdevice_io_irq_test = me_subdevice_io_irq_test;
		subdevice->me_subdevice_io_reset_subdevice = me_subdevice_io_reset_subdevice;
		subdevice->me_subdevice_io_single_config = me_subdevice_io_single_config;
		subdevice->me_subdevice_io_single_read = me_subdevice_io_single_read;
		subdevice->me_subdevice_io_single_write = me_subdevice_io_single_write;
		subdevice->me_subdevice_io_stream_config = me_subdevice_io_stream_config;
		subdevice->me_subdevice_io_stream_new_values = me_subdevice_io_stream_new_values;
		subdevice->me_subdevice_io_stream_read = me_subdevice_io_stream_read;
		subdevice->me_subdevice_io_stream_start = me_subdevice_io_stream_start;
		subdevice->me_subdevice_io_stream_status = me_subdevice_io_stream_status;
		subdevice->me_subdevice_io_stream_stop = me_subdevice_io_stream_stop;
		subdevice->me_subdevice_io_stream_write = me_subdevice_io_stream_write;
		subdevice->me_subdevice_lock_subdevice = me_subdevice_lock_subdevice;
		subdevice->me_subdevice_query_number_channels = me_subdevice_query_number_channels;
		subdevice->me_subdevice_query_number_ranges = me_subdevice_query_number_ranges;
		subdevice->me_subdevice_query_range_by_min_max = me_subdevice_query_range_by_min_max;
		subdevice->me_subdevice_query_range_info = me_subdevice_query_range_info;
		subdevice->me_subdevice_query_subdevice_type = me_subdevice_query_subdevice_type;
		subdevice->me_subdevice_query_subdevice_caps = me_subdevice_query_subdevice_caps;
		subdevice->me_subdevice_query_subdevice_caps_args = me_subdevice_query_subdevice_caps_args;
		subdevice->me_subdevice_query_timer = me_subdevice_query_timer;
		subdevice->me_subdevice_query_version_firmware = me_subdevice_query_version_firmware;
		subdevice->me_subdevice_config_load = me_subdevice_config_load;
		subdevice->me_subdevice_destructor = me_subdevice_destructor;
		subdevice->me_subdevice_irq_handle = me_subdevice_irq_handle;
		subdevice->me_subdevice_postinit = me_subdevice_postinit;

		subdevice->me_subdevice_set_offset = me_subdevice_set_offset;
	}

	// Init interrupt protection.
#  if !defined(ME_USB)
	ME_INIT_LOCK(&subdevice->subdevice_lock.subdevice_lock);
#if !defined(SCALE_RT)
#   if !defined(ME_ATRENATIVE_LOCKS)
	PDEBUG_MUTEX("Spinlock status: %x.\n", subdevice->subdevice_lock.subdevice_lock.raw_lock.slock);
#   else	// ME_ATRENATIVE_LOCKS
	PDEBUG_MUTEX("Spinlock status: %x->%x.\n", atomic_read(&subdevice->subdevice_lock.subdevice_lock.enter_lock), atomic_read(&subdevice->subdevice_lock.subdevice_lock.exit_lock));
#   endif	// ME_ATRENATIVE_LOCKS
#endif	// SCALE_RT
#  else
#    ifndef init_MUTEX
    sema_init(&(subdevice->subdevice_lock.subdevice_semaphore), 1);
#    else
	init_MUTEX(&(subdevice->subdevice_lock.subdevice_semaphore));
#    endif
#  endif
# ifdef PROTECTOR_CHECK
	subdevice->subdevice_lock.status = 0;
	subdevice->subdevice_lock.blocked_irq = 0;
	subdevice->subdevice_lock.pid = -1;
#  endif

	return err;
}

void me_subdevice_deinit(me_subdevice_t* subdevice)
{
	PDEBUG("executed.\n");
	if (subdevice && &subdevice->lock)
	{
		me_slock_deinit(&subdevice->lock);
	}
}
