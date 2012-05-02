/**
 * @file memain_common.c
 *
 * @brief Part of memain driver. Header and definitions of the common functions (for PCI and SynapseUSB).
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (dev_no.gantzke@meilhaus.de)
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

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_ioctl.h"
# include "me_types.h"
# include "me_common.h"
# include "me_internal.h"
# include "me_spin_lock.h"
# include "medevice.h"

# include <linux/time.h>
# include <linux/errno.h>
# include <linux/fs.h>
# include <asm/uaccess.h>
# include <linux/cdev.h>

# include "memain_common.h"
# include "memain_common_templates.h"

static int me_query_type(struct file* filep, me_query_type_driver_t* arg);

static int lock_driver(struct file* filep, int lock , int flags);
static int lock_device(struct file* filep, int device, int lock , int flags);
static int lock_subdevice(struct file* filep, int device, int subdevice, int lock , int flags);

static int config_load(struct file* filep, me_extra_param_set_t* config);
static int custom_driver(struct file* filep, me_extra_param_set_t* config);

#ifdef ME_SYNAPSE
void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec);


void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec)
{
	ts->tv_sec = sec;
	if (nsec < 0)
	{
		set_normalized_timespec(ts, sec - 1, NSEC_PER_SEC - nsec);
	}
	else if (nsec > NSEC_PER_SEC)
	{
		set_normalized_timespec(ts, sec + 1, nsec - NSEC_PER_SEC);
	}
	else
	{
	      ts->tv_nsec = nsec;
	      ts->tv_sec = sec;
	}
}
#endif	//ME_SYNAPSE
///Implementation
int get_medevice(int dev_no, me_device_t** device)
{
	int location = 0;
	struct list_head* pos;
	int err = ME_ERRNO_SUCCESS;

	list_for_each(pos, &me_device_list)
	{
		if(location == dev_no)
		{
			// Get member 'list' from struct 'me_device_t'
			*device = list_entry(pos, me_device_t, list);
			return err;
		}
		location++;
	}

	PERROR("Device number %d is invalid.\n", dev_no);
	err = ME_ERRNO_INVALID_DEVICE;

	*device = NULL;
	return err;
}

me_device_t* find_device_on_list(me_general_dev_t* n_device, int state)
{
	struct list_head* pos;
	me_device_t* o_device = NULL;

	int vendor_id;
	int device_id;
	int serial_no;
	int bus_type;
	int bus_no;
	int dev_no;
	int func_no;
	int plugged;

	PDEBUG("executed.\n");

	down_write(&me_rwsem);
		list_for_each(pos, &me_device_list)
		{
			o_device = list_entry(pos, me_device_t, list);

			o_device->me_device_query_info_device(o_device,
												&vendor_id,
												&device_id,
												&serial_no,
												&bus_type,
												&bus_no,
												&dev_no,
												&func_no,
												&plugged);

			if (	(n_device->vendor == vendor_id)
				&&
					(n_device->device == device_id)
				&&
					(n_device->serial_no == serial_no)
				)
			{
				if((state == ME_PLUGGED_ANY) || (state == plugged))
				{
					// Device found.
					up_write(&me_rwsem);
					return o_device;
				}
			}
		}

	// Device not found.
	up_write(&me_rwsem);

	return NULL;
}

void insert_to_device_list(me_device_t* n_device)
{
	down_write(&me_rwsem);
		list_add_tail(&n_device->list, &me_device_list);
	up_write(&me_rwsem);
}

void release_instance(me_device_t* device)
{
# if defined(ME_MEPHISTO)
 	PDEBUG("executed.\n");

 	if (!device)
 	{
 		PERROR("NULL pointer!");
 		return;
 	}

	if (device->me_device_destructor)
	{
		device->me_device_destructor(device);
	}
	else
	{
		PERROR("Destructor not registred! device=0x%p device->me_device_destructor=0x%p\n", device, device->me_device_destructor);
	}

# else	//ME_MEPHISTO

	uint32_t dev_id;

# if defined(ME_PCI)
	char constructor_name[64]="me0000_pci_constr\0";
#elif  defined(ME_USB)
	char constructor_name[64]="me0000_usb_constr\0";
#endif

 	PDEBUG("executed.\n");

 	if (!device)
 	{
 		PERROR("NULL pointer!");
 		return;
 	}

	dev_id = device->bus.local_dev.device;
	constructor_name[2] += (char)((dev_id >> 12) & 0x000F);
	constructor_name[3] += (char)((dev_id >> 8) & 0x000F);

	if (device->info.custom_subname)
	{
		strncat(constructor_name, device->info.custom_subname, 32);
	}

	if (device->me_device_destructor)
	{
		device->me_device_destructor(device);
	}
	else
	{
		PERROR("Destructor not registred! device=0x%p device->me_device_destructor=0x%p\n", device, device->me_device_destructor);
	}

	PDEBUG("Release: %s\n", constructor_name);

	__symbol_put(constructor_name);
# endif
}

void clear_device_list(void)
{
	struct list_head* entry;
	me_device_t* dev = NULL;
	// Clear the device info list .
	down_write(&me_rwsem);
		while (!list_empty(&me_device_list))
		{
			entry = me_device_list.next;
			dev = list_entry(entry, me_device_t, list);
			if (dev)
			{
				release_instance(dev);
			}
			if (entry)
			{
				list_del(entry);
			}
			if (dev)
			{
				kfree(dev);
			}
			dev = NULL;
		}
	up_write(&me_rwsem);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
int me_ioctl(struct inode *inodep, struct file* filep, unsigned int service, unsigned long arg)
#else
long me_ioctl(struct file* filep, unsigned int service, unsigned long arg)
#endif
{
	PDEBUG("executed.\n");

	if (_IOC_TYPE(service) != MEMAIN_MAGIC)
	{
		PERROR("Invalid magic number.\n");
		return -ENOTTY;
	}

	switch (service)
	{
		///RESET
		case ME_IO_RESET_DEVICE:
			return me_io_reset_device(filep, (me_io_reset_device_t *)arg);

		case ME_IO_RESET_SUBDEVICE:
			return me_io_reset_subdevice(filep, (me_io_reset_subdevice_t *)arg);

		/// IRQ
		case ME_IO_IRQ_ENABLE:
			return me_io_irq_start(filep, (me_io_irq_start_t *)arg);

		case ME_IO_IRQ_WAIT:
			return me_io_irq_wait(filep, (me_io_irq_wait_t *)arg);

		case ME_IO_IRQ_DISABLE:
			return me_io_irq_stop(filep, (me_io_irq_stop_t *)arg);

		case ME_IO_IRQ_CHECK:
			return me_io_irq_test(filep, (me_io_irq_test_t *)arg);

		///SINGLE
		case ME_IO_SINGLE_CONFIG:
			return me_io_single_config(filep, (me_io_single_config_t *)arg);

		case ME_IO_SINGLE:
			return me_io_single(filep, (me_io_single_t *)arg);

		case ME_IO_SINGLE_SIMPLE:
			return me_io_single_simple(filep, (me_io_single_simple_t *)arg);

		///STREAM
		case ME_IO_STREAM_CONFIG:
			return me_io_stream_config(filep, (me_io_stream_config_t *)arg);

		case ME_IO_STREAM_READ:
			return me_io_stream_read(filep, (me_io_stream_read_t *)arg);

		case ME_IO_STREAM_TIMEOUT_READ:
			return me_io_stream_timeout_read(filep, (me_io_stream_timeout_read_t *)arg);

		case ME_IO_STREAM_WRITE:
			return me_io_stream_write(filep, (me_io_stream_write_t *)arg);

		case ME_IO_STREAM_TIMEOUT_WRITE:
			return me_io_stream_timeout_write(filep, (me_io_stream_timeout_write_t *)arg);

		case ME_IO_STREAM_NEW_VALUES:
			return me_io_stream_new_values(filep, (me_io_stream_new_values_t *)arg);

		case ME_IO_STREAM_START:
			return me_io_stream_start(filep, (me_io_stream_start_t *)arg);

		case ME_IO_STREAM_START_SIMPLE:
			return me_io_stream_start_simple(filep, (me_io_stream_start_simple_t *)arg);

		case ME_IO_STREAM_STOP:
			return me_io_stream_stop(filep, (me_io_stream_stop_t *)arg);

		case ME_IO_STREAM_STOP_SIMPLE:
			return me_io_stream_stop_simple(filep, (me_io_stream_stop_simple_t *)arg);

		case ME_IO_STREAM_STATUS:
			return me_io_stream_status(filep, (me_io_stream_status_t *)arg);

		case ME_SET_OFFSET:
			return me_set_offset(filep, (me_set_offset_t *)arg);

		///LOCKS
		case ME_LOCK_DRIVER:
			return me_lock_driver(filep, (me_lock_driver_t *)arg);

		case ME_LOCK_DEVICE:
			return me_lock_device(filep, (me_lock_device_t *)arg);

		case ME_LOCK_SUBDEVICE:
			return me_lock_subdevice(filep, (me_lock_subdevice_t *)arg);

		///QUERY
		case ME_QUERY_INFO_DEVICE:
			return me_query_info_device(filep, (me_query_info_device_t *)arg);

		case ME_QUERY_DESCRIPTION_DEVICE:
			return me_query_description_device(filep, (me_query_description_device_t *)arg);

		case ME_QUERY_NAME_DEVICE:
			return me_query_name_device(filep, (me_query_name_device_t *)arg);

		case ME_QUERY_NAME_DEVICE_DRIVER:
			return me_query_name_device_driver(filep, (me_query_name_device_driver_t *)arg);

		case ME_QUERY_NAME_MAIN_DRIVER:
			return me_query_name_main_driver(filep, (me_query_name_main_driver_t *)arg);

		case ME_QUERY_NUMBER_DEVICES:
			return me_query_number_devices(filep, (me_query_number_devices_t *)arg);

		case ME_QUERY_NUMBER_SUBDEVICES:
			return me_query_number_subdevices(filep, (me_query_number_subdevices_t *)arg);

		case ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE:
			return me_query_number_subdevices_by_type(filep, (me_query_number_subdevices_by_type_t *)arg);

		case ME_QUERY_NUMBER_CHANNELS:
			return me_query_number_channels(filep, (me_query_number_channels_t *)arg);

		case ME_QUERY_NUMBER_RANGES:
			return me_query_number_ranges(filep, (me_query_number_ranges_t *)arg);

		case ME_QUERY_RANGE_BY_MIN_MAX:
			return me_query_range_by_min_max(filep, (me_query_range_by_min_max_t *)arg);

		case ME_QUERY_RANGE_INFO:
			return me_query_range_info(filep, (me_query_range_info_t *)arg);

		case ME_QUERY_SUBDEVICE_BY_TYPE:
			return me_query_subdevice_by_type(filep, (me_query_subdevice_by_type_t *)arg);

		case ME_QUERY_SUBDEVICE_TYPE:
			return me_query_subdevice_type(filep, (me_query_subdevice_type_t *)arg);

		case ME_QUERY_SUBDEVICE_CAPS:
			return me_query_subdevice_caps(filep, (me_query_subdevice_caps_t *)arg);

		case ME_QUERY_SUBDEVICE_CAPS_ARGS:
			return me_query_subdevice_caps_args(filep, (me_query_subdevice_caps_args_t *)arg);

		case ME_QUERY_TIMER:
			return me_query_timer(filep, (me_query_timer_t *)arg);

		case ME_QUERY_VERSION_MAIN_DRIVER:
			return me_query_version_main_driver(filep, (me_query_version_main_driver_t *)arg);

		case ME_QUERY_VERSION_DEVICE_DRIVER:
			return me_query_version_device_driver(filep, (me_query_version_device_driver_t *)arg);

		case ME_QUERY_DEVICE_RELEASE:
			return me_query_device_release(filep, (me_query_device_release_t *)arg);

		case ME_QUERY_VERSION_FIRMWARE:
			return me_query_version_firmware(filep, (me_query_version_firmware_t *)arg);

		case ME_QUERY_TYPE_DRIVER:
			return me_query_type(filep, (me_query_type_driver_t *)arg);

		///CONFIG
		case ME_CONFIG_LOAD:
			return me_config_load(filep, (me_extra_param_set_t *)arg);
	}

	PERROR("Invalid ioctl number.\n");
	return -ENOTTY;
}


#ifdef ME_IO_MULTIPLEX_TEMPLATE
ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_irq_start",
    me_io_irq_start_t,
    me_io_irq_start,
    me_device_io_irq_start,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     karg.irq_source,
     karg.irq_edge,
     karg.irq_arg,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_irq_wait",
    me_io_irq_wait_t,
    me_io_irq_wait,
    me_device_io_irq_wait,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     &karg.irq_count,
     &karg.value,
     karg.time_out,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_irq_stop",
    me_io_irq_stop_t,
    me_io_irq_stop,
    me_device_io_irq_stop,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_irq_test",
    me_io_irq_test_t,
    me_io_irq_test,
    me_device_io_irq_test,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_reset_device",
    me_io_reset_device_t,
    me_io_reset_device,
    me_device_io_reset_device,
    (dev,
     filep,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_reset_subdevice",
    me_io_reset_subdevice_t,
    me_io_reset_subdevice,
    me_device_io_reset_subdevice,
    (dev,
     filep,
     karg.subdevice,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_single_config",
    me_io_single_config_t,
    me_io_single_config,
    me_device_io_single_config,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     karg.single_config,
     karg.ref,
     karg.trig_chain,
     karg.trig_type,
     karg.trig_edge,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_new_values",
    me_io_stream_new_values_t,
    me_io_stream_new_values,
    me_device_io_stream_new_values,
    (dev,
     filep,
     karg.subdevice,
     karg.time_out,
     &karg.count,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_read",
    me_io_stream_read_t,
    me_io_stream_read,
    me_device_io_stream_read,
    (dev,
     filep,
     karg.subdevice,
     karg.read_mode,
     karg.values,
     &karg.count,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_timeout_read",
    me_io_stream_timeout_read_t,
    me_io_stream_timeout_read,
    me_device_io_stream_timeout_read,
    (dev,
     filep,
     karg.subdevice,
     karg.read_mode,
     karg.values,
     &karg.count,
     karg.timeout,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_status",
    me_io_stream_status_t,
    me_io_stream_status,
    me_device_io_stream_status,
    (dev,
     filep,
     karg.subdevice,
     karg.wait,
     &karg.status,
     &karg.count,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_write",
    me_io_stream_write_t,
    me_io_stream_write,
    me_device_io_stream_write,
    (dev,
     filep,
     karg.subdevice,
     karg.write_mode,
     karg.values,
     &karg.count,
     karg.flags))

ME_IO_MULTIPLEX_TEMPLATE(
    "me_io_stream_timeout_write",
    me_io_stream_timeout_write_t,
    me_io_stream_timeout_write,
    me_device_io_stream_timeout_write,
    (dev,
     filep,
     karg.subdevice,
     karg.write_mode,
     karg.values,
     &karg.count,
     karg.timeout,
     karg.flags))

 ME_IO_MULTIPLEX_TEMPLATE(
    "me_set_offset",
    me_set_offset_t,
    me_set_offset,
    me_device_set_offset,
    (dev,
     filep,
     karg.subdevice,
     karg.channel,
     karg.range,
     &karg.offset,
     karg.flags))

#else
#error macro ME_IO_MULTIPLEX_TEMPLATE not defined
#endif

#ifdef ME_QUERY_MULTIPLEX_STR_TEMPLATE
ME_QUERY_MULTIPLEX_STR_TEMPLATE(
    "me_query_name_device",
    me_query_name_device_t,
    me_query_name_device,
    me_device_query_name_device,
    (dev,
    &msg))

ME_QUERY_MULTIPLEX_STR_TEMPLATE(
    "me_query_name_device_driver",
    me_query_name_device_driver_t,
    me_query_name_device_driver,
    me_device_query_name_device_driver,
    (dev,
    &msg))

ME_QUERY_MULTIPLEX_STR_TEMPLATE(
    "me_query_description_device",
    me_query_description_device_t,
    me_query_description_device,
    me_device_query_description_device,
    (dev,
    &msg))
#else
#error macro ME_QUERY_MULTIPLEX_STR_TEMPLATE not defined
#endif

#ifdef ME_QUERY_MULTIPLEX_TEMPLATE
ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_info_device",
    me_query_info_device_t,
    me_query_info_device,
    me_device_query_info_device,
    (dev,
     &karg.vendor_id,
     &karg.device_id,
     &karg.serial_no,
     &karg.bus_type,
     &karg.bus_no,
     &karg.dev_no,
     &karg.func_no,
     &karg.plugged))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_number_subdevices",
    me_query_number_subdevices_t,
    me_query_number_subdevices,
    me_device_query_number_subdevices,
    (dev,
     &karg.number))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_number_subdevices_by_type",
    me_query_number_subdevices_by_type_t,
    me_query_number_subdevices_by_type,
    me_device_query_number_subdevices_by_type,
    (dev,
     karg.type,
     karg.subtype,
     &karg.number))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_number_channels",
    me_query_number_channels_t,
    me_query_number_channels,
    me_device_query_number_channels,
    (dev,
     karg.subdevice,
     &karg.number))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_subdevice_by_type",
    me_query_subdevice_by_type_t,
    me_query_subdevice_by_type,
    me_device_query_subdevice_by_type,
    (dev,
     karg.start_subdevice,
     karg.type,
     karg.subtype,
     &karg.subdevice))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_subdevice_type",
    me_query_subdevice_type_t,
    me_query_subdevice_type,
    me_device_query_subdevice_type,
    (dev,
    karg.subdevice,
    &karg.type,
    &karg.subtype))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_subdevice_caps",
    me_query_subdevice_caps_t,
    me_query_subdevice_caps,
    me_device_query_subdevice_caps,
    (dev,
    karg.subdevice,
    &karg.caps))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_number_ranges",
    me_query_number_ranges_t,
    me_query_number_ranges,
    me_device_query_number_ranges,
    (dev,
     karg.subdevice,
     karg.unit,
     &karg.number))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_range_by_min_max",
    me_query_range_by_min_max_t,
    me_query_range_by_min_max,
    me_device_query_range_by_min_max,
    (dev,
     karg.subdevice,
     karg.unit,
     &karg.min,
     &karg.max,
     &karg.max_data,
     &karg.range))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_range_info",
    me_query_range_info_t,
    me_query_range_info,
    me_device_query_range_info,
    (dev,
     karg.subdevice,
     karg.range,
     &karg.unit,
     &karg.min,
     &karg.max,
     &karg.max_data))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_timer",
    me_query_timer_t,
    me_query_timer,
    me_device_query_timer,
    (dev,
     karg.subdevice,
     karg.timer,
     &karg.base_frequency,
     &karg.min_ticks,
     &karg.max_ticks))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_version_device_driver",
    me_query_version_device_driver_t,
    me_query_version_device_driver,
    me_device_query_version_device_driver,
    (dev,
    &karg.version))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_device_release",
    me_query_device_release_t,
    me_query_device_release,
    me_device_query_device_release,
    (dev,
    &karg.version))

ME_QUERY_MULTIPLEX_TEMPLATE(
    "me_query_version_firmware",
    me_query_version_firmware_t,
    me_query_version_firmware,
    me_device_query_version_firmware,
    (dev,
    karg.subdevice,
    &karg.version))
#else
#error macro ME_QUERY_MULTIPLEX_TEMPLATE not defined
#endif

int me_lock_driver(struct file* filep, me_lock_driver_t* arg)
{
	me_lock_driver_t lock ;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed. filep=%p\n", filep);

	getnstimeofday(&ts_pre);

	if (copy_from_user(&lock, arg, sizeof(me_lock_driver_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		err = -EFAULT;
		goto EXIT;
	}

	PDEBUG_LOCKS("lock:0x%x filep=%p pid=%d tgid=%d\n", lock.lock, filep, current_thread_info()->task->pid, current_thread_info()->task->tgid);
	lock.err_no = lock_driver(filep, lock.lock, lock.flags);

	if (copy_to_user(arg, &lock, sizeof(me_lock_driver_t)))
	{
		PERROR("Can't copy arguments back to user space.\n");
		err = -EFAULT;
	}

EXIT:
	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int lock_driver(struct file* filep, int lock , int flags)
{
	me_device_t* dev = NULL;
	int err_no;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	ME_SPIN_LOCK(&me_lock);
		if (!(flags & ME_LOCK_FORCE) || (lock == ME_LOCK_CHECK))
		{
			if ((me_filep != NULL) && (me_filep != filep))
			{
				PERROR("Driver is already locked by another process.\n");
				err = ME_ERRNO_LOCKED;
			}
		}

		if (!err)
		{
			down_read(&me_rwsem);
				if (!(flags & ME_LOCK_FORCE))
				{
					// Check current situation
					list_for_each_entry(dev, &me_device_list, list)
					{
						err_no = dev->me_device_lock_device(dev, filep, ME_LOCK_CHECK, flags);
						if (err_no)
						{
							err = err_no;
							PINFO("Lower level locks are %s by another process.\n", (err_no == ME_ERRNO_LOCKED) ? "already locked" : "in use");
							if (err_no == ME_ERRNO_LOCKED)
							{
								err = ME_ERRNO_LOCKED;
								goto EXIT;
							}
						}
					}
				}

				if (!err)
				{
					switch (lock)
					{
						case ME_LOCK_SET:
							if (me_count)
							{
								PERROR("Driver is currently in use by another process.\n");
								err = ME_ERRNO_USED;
								break;
							}

							// Lock driver
							me_filep = filep;
							break;

						case ME_LOCK_RELEASE:
							if (me_count)
							{
								PERROR("Driver is currently in use by another process.\n");
								err = ME_ERRNO_USED;
								break;
							}

							// Unlock driver
							me_filep = NULL;

							// Unlock also subdevices
							if (!flags)
							{
								list_for_each_entry(dev, &me_device_list, list)
								{
									dev->me_device_lock_device(dev, filep, ME_LOCK_RELEASE, flags);
								}
							}
							break;

						case ME_LOCK_CHECK:
							break;

						default:
							PERROR("Invalid lock specified.\n");
							err = ME_ERRNO_INVALID_LOCK;
					}
				}
EXIT:
			up_read(&me_rwsem);
		}
	ME_SPIN_UNLOCK(&me_lock);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_lock_device(struct file* filep, me_lock_device_t* arg)
{
	me_lock_device_t lock ;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed. filep=%p\n", filep);

	getnstimeofday(&ts_pre);

	if (copy_from_user(&lock, arg, sizeof(me_lock_device_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		err = -EFAULT;
		goto EXIT;
	}

	PDEBUG_LOCKS("lock:0x%x filep=%p pid=%d tgid=%d\n", lock.lock, filep, current_thread_info()->task->pid, current_thread_info()->task->tgid);
	lock.err_no = lock_device(filep, lock.device, lock.lock, lock.flags);

	if (copy_to_user(arg, &lock , sizeof(me_lock_device_t)))
	{
		PERROR("Can't copy arguments back to user space.\n");
		err = -EFAULT;
	}

EXIT:
	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int lock_device(struct file* filep, int device, int lock , int flags)
{
	me_device_t* dev = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is already locked by another process.\n");
			err = ME_ERRNO_LOCKED;
		}
		else
		{
			down_read(&me_rwsem);
				// Find device.
				err = get_medevice(device, &dev);
				if (!err)
				{
					// Lock it.
					err = dev->me_device_lock_device(dev, filep, lock, flags);
				}
			up_read(&me_rwsem);
		}
	ME_SPIN_UNLOCK(&me_lock);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_lock_subdevice(struct file* filep, me_lock_subdevice_t* arg)
{
	me_lock_subdevice_t lock ;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed. filep=%p\n", filep);

	getnstimeofday(&ts_pre);

	if (copy_from_user(&lock, arg, sizeof(me_lock_subdevice_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		err = -EFAULT;
		goto EXIT;
	}

	PDEBUG_LOCKS("lock:0x%x filep=%p pid=%d tgid=%d\n", lock.lock, filep, current_thread_info()->task->pid, current_thread_info()->task->tgid);
	lock.err_no = lock_subdevice(filep, lock.device, lock.subdevice, lock.lock, lock.flags);

	if (copy_to_user(arg, &lock , sizeof(me_lock_subdevice_t)))
	{
		PERROR("Can't copy arguments back to user space.\n");
		err = -EFAULT;
	}

EXIT:
	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int lock_subdevice(struct file* filep, int device, int subdevice, int lock, int flags)
{
	me_device_t* dev = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is already locked by another process.\n");
			err = ME_ERRNO_LOCKED;
		}
		else
		{
			down_read(&me_rwsem);
				// Find device.
				err = get_medevice(device, &dev);
				if (!err)
				{
					// Lock it.
					err = dev->me_device_lock_subdevice(dev, filep, subdevice, lock, flags);
				}
			up_read(&me_rwsem);
		}
	ME_SPIN_UNLOCK(&me_lock);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_open(struct inode* inode_ptr, struct file* filep)
{
	PDEBUG("executed.\n");
	// Nothing to do here.
	PEXECTIME("executed in %ld us\n", 0L);

	return ME_ERRNO_SUCCESS;
}

int me_release(struct inode* inode_ptr, struct file* filep)
{
	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	lock_driver(filep, ME_LOCK_RELEASE, ME_LOCK_DRIVER_NO_FLAGS);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return ME_ERRNO_SUCCESS;
}

int me_config_load(struct file* filep, me_extra_param_set_t* arg)
{
	me_extra_param_set_t config_entry;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	// Copy argument to kernel space.
	if (copy_from_user(&config_entry, arg, sizeof(me_extra_param_set_t)))
	{
		PERROR("Can't copy config to kernel space.\n");
		err = -EFAULT;
	}

	if (!err)
	{
		switch (config_entry.flags)
		{
			case 0:
				err = config_load(filep, &config_entry);
				break;

			case ME_CONF_LOAD_CUSTOM_DRIVER:
				err = custom_driver(filep, &config_entry);
				break;

			default:
				PERROR("Unrecognized flag.\n");
				config_entry.err_no = ME_ERRNO_CONFIG_LOAD_FAILED;

		}
	}

	// Copy back argument to user space.
	if (copy_to_user(arg, &config_entry, sizeof(me_extra_param_set_t)))
	{
		PERROR("Can't copy config list to user space.\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int config_load(struct file* filep, me_extra_param_set_t* config)
{
	me_device_t* dev = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Resource is locked by another process.\n");
			config->err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);

			down_read(&me_rwsem);
				// Find device.
				config->err_no = get_medevice(config->device, &dev);
				if (!config->err_no)
				{
					err = dev->me_device_config_load(dev, filep, config->arg, config->size);
					if (err > 0)
					{
						config->err_no = err;
						err = ME_ERRNO_SUCCESS;
					}
					else if (err < 0)
					{
						config->err_no = ME_ERRNO_CONFIG_LOAD_FAILED;
					}

				}
			up_read(&me_rwsem);

			ME_SPIN_LOCK(&me_lock);
			me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int custom_driver(struct file* filep, me_extra_param_set_t* config)
{
	me_custom_constr_t custom_constr;
	me_general_dev_t* dev;

# if defined(ME_PCI)
	char constructor_name[64]="me0000_pci_constr\0";
	char module_name[64]="me0000PCI\0";
# elif defined(ME_USB)
	char constructor_name[64]="me0000_usb_constr\0";
	char module_name[64]="me0000USB\0";
#else
	char constructor_name[64]="me0000_constr\0";
	char module_name[64]="me0000\0";
#endif

	me_constr_t constructor = NULL;
	me_device_t* n_device = NULL;
	me_device_t* o_device = NULL;

	struct list_head list_head_entry;

	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	/// Allocate structures.
	dev = kzalloc(sizeof(me_general_dev_t), GFP_KERNEL);
	if (!dev)
	{
		PERROR("Can't get memory for device's instance.");
		err = -ENOMEM;
		goto ERROR;
	}

	if (config->size != sizeof(me_custom_constr_t))
	{
		PERROR("Can't copy config list to user space.\n");
		config->err_no = ME_ERRNO_CONFIG_LOAD_FAILED;
		goto ERROR;
	}

	// Copy argument to kernel space.
	if (copy_from_user(&custom_constr, config->arg, sizeof(me_custom_constr_t)))
	{
		PERROR("Can't copy config argument to kernel space.\n");
		err = -EFAULT;
		goto ERROR;
	}

	config->err_no = get_medevice(config->device, &o_device);
	if (config->err_no)
	{
		goto ERROR;
	}
	if (!o_device)
	{
		goto ERROR;
	}
	constructor_name[2] += (char)((o_device->bus.local_dev.device >> 12) & 0x000F);
	constructor_name[3] += (char)((o_device->bus.local_dev.device >> 8) & 0x000F);
	module_name[2] += (char)((o_device->bus.local_dev.device >> 12) & 0x000F);
	module_name[3] += (char)((o_device->bus.local_dev.device >> 8) & 0x000F);
	if (module_name[2] == '6')
	{// Exceptions: me61xx, me62xx, me63xx are handled by one driver.
		module_name[3] = '0';
	}
	if (module_name[2] == '4')
	{
		if (module_name[3] == '8')
		{// Exceptions: me46xx and me48xx are handled by one driver.
			module_name[3] = '6';
		}
		else if (module_name[3] == '5')
		{// Exceptions: me45xx and me47xx are handled by one driver.
			module_name[3] = '7';
		}
	}

	if ((custom_constr.version) && strlen(custom_constr.version))
	{
		strncat(constructor_name, custom_constr.version, 32);
		strncat(module_name, custom_constr.version, 32);
	}

	PDEBUG("constructor_name: %s\n", constructor_name);
	PDEBUG("module_name: %s\n", module_name);
	constructor = (me_constr_t) __symbol_get(constructor_name);
	if (!constructor)
	{
		if (!request_module("%s", module_name))
		{
			constructor = (me_constr_t) __symbol_get(constructor_name);
			if (!constructor)
			{
				PERROR("Can't get %s driver module constructor.\n", module_name);
				err = -ENODEV;
				goto ERROR;
			}
		}
		else
		{
			PERROR("Can't get requested module: %s.\n", module_name);
			err = -ENODEV;
			goto ERROR;
		}
	}

	dev->dev = o_device->bus.local_dev.dev;
	// Get Meilhaus specific device information.
	dev->device = o_device->bus.local_dev.device;
	dev->vendor = o_device->bus.local_dev.vendor;
	dev->hw_revision = o_device->bus.local_dev.hw_revision;
	dev->serial_no = o_device->bus.local_dev.serial_no;

	list_head_entry.prev = o_device->list.prev;
	list_head_entry.next = o_device->list.next;

	o_device->me_device_disconnect(o_device);
	release_instance(o_device);

	n_device = (*constructor)(dev, NULL);
	if (!n_device)
	{
		goto ERROR;
	}

	if (o_device)
	{
		kfree(o_device);
		o_device = NULL;
	}

	list_head_entry.prev->next = &n_device->list;
	list_head_entry.next->prev = &n_device->list;
	n_device->list.prev = list_head_entry.prev;
	n_device->list.next = list_head_entry.next;

	if (n_device->me_device_postinit)
	{
		if (n_device->me_device_postinit(n_device, NULL))
		{
			PERROR("Error while calling me_device_postinit().\n");
			/// This error can be ignored.
		}
		else
		{
			PDEBUG("me_device_postinit() was sucessful.\n");
		}
	}
	else
	{
		PERROR("me_device_postinit() not registred!\n");
	}

ERROR:
	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_stream_start(struct file* filep, me_io_stream_start_t* arg)
{
	int i;
	int err = ME_ERRNO_SUCCESS;

	me_device_t* dev = NULL;
	me_io_stream_start_t karg;
	meIOStreamStart_t* start_list = NULL;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_stream_start_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	if (karg.count < 1)
	{
		PERROR("Invalid list size. %d\n", karg.count);
		karg.err_no = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR;
	}

	if (karg.flags & ~ME_IO_STREAM_START_NONBLOCKING)
	{
		PERROR("Invalid flag specified. %d\n", karg.flags);
		karg.err_no = ME_ERRNO_INVALID_FLAGS;
		goto ERROR;
	}

	karg.err_no = ME_ERRNO_SUCCESS;

	start_list = kmalloc(sizeof(meIOStreamStart_t) * karg.count, GFP_KERNEL);
	if (!start_list)
	{
		PERROR("Can't get buffer for start list.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	if (copy_from_user(start_list, karg.start_list, sizeof(meIOStreamStart_t) * karg.count))
	{
		PERROR("Can't copy start list to kernel space.\n");
		err = -EFAULT;
		goto ERROR;
	}

	karg.err_no = ME_ERRNO_SUCCESS;
	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is already locked by another process.\n");
			for (i = 0; i < karg.count; i++)
			{
				start_list[i].iErrno = ME_ERRNO_LOCKED;
			}
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);

			for (i = 0; i < karg.count; i++)
			{
				down_read(&me_rwsem);
					// Find device.
					start_list[i].iErrno = get_medevice(start_list[i].iDevice, &dev);
					if (!start_list[i].iErrno)
					{
							start_list[i].iErrno = dev->me_device_io_stream_start(
												dev,
												filep,
												start_list[i].iSubdevice,
												start_list[i].iStartMode,
												start_list[i].iTimeOut,
												start_list[i].iFlags);

					}

				up_read(&me_rwsem);
				if ((start_list[i].iErrno) && !(karg.flags & ME_IO_STREAM_START_NONBLOCKING))
				{
					karg.err_no = start_list[i].iErrno;
					break;
				}
			}

			ME_SPIN_LOCK(&me_lock);
				me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	if (copy_to_user(karg.start_list, start_list, sizeof(meIOStreamStart_t) * karg.count))
	{
		PERROR("Can't copy start list to user space.\n");
		err = -EFAULT;
	}

ERROR:
	if (copy_to_user(arg, &karg, sizeof(me_io_stream_start_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	if (start_list)
		kfree(start_list);
	start_list = NULL;

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_stream_start_simple(struct file* filep, me_io_stream_start_simple_t* arg)
{
	me_device_t* dev = NULL;
	me_io_stream_start_simple_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_stream_start_simple_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);
				down_read(&me_rwsem);
					// Find device.
					karg.err_no = get_medevice(karg.device, &dev);
					if (!karg.err_no)
					{
						karg.err_no = dev->me_device_io_stream_start(
												dev,
												filep,
												karg.subdevice,
												karg.mode,
												karg.timeout,
												karg.flags);
					}
				up_read(&me_rwsem);
			ME_SPIN_LOCK(&me_lock);
				me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	if (copy_to_user(arg, &karg, sizeof(me_io_stream_start_simple_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_single(struct file* filep, me_io_single_t* arg)
{
	int i;
	me_device_t* dev = NULL;
	me_io_single_t karg;
	meIOSingle_t* single_list = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_single_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	if (karg.count < 1)
	{
		PERROR("Invalid list size. %d\n", karg.count);
		karg.err_no = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR;
	}

	if (karg.flags & ~ME_IO_SINGLE_NONBLOCKING)
	{
		PERROR("Invalid flag specified. %d\n", karg.flags);
		karg.err_no = ME_ERRNO_INVALID_FLAGS;
		goto ERROR;
	}

	karg.err_no = ME_ERRNO_SUCCESS;

	single_list = kmalloc(sizeof(meIOSingle_t)*  karg.count, GFP_KERNEL);
	if (!single_list)
	{
		PERROR("Can't get buffer for single list.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	err = copy_from_user(single_list, karg.single_list, sizeof(meIOSingle_t) * karg.count);
	if (err)
	{
		PERROR("Can't copy single list to kernel space.\n");
		err = -EFAULT;
		goto ERROR;
	}

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");

			for (i = 0; i < karg.count; i++)
			{
				single_list[i].iErrno = ME_ERRNO_LOCKED;
			}
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);

			for (i = 0; i < karg.count; i++)
			{
				down_read(&me_rwsem);
					// Find device.
					single_list[i].iErrno = get_medevice(single_list[i].iDevice, &dev);
					if (!single_list[i].iErrno)
					{
						if (single_list[i].iDir == ME_DIR_OUTPUT)
						{
							single_list[i].iErrno = dev->me_device_io_single_write(
												dev,
												filep,
												single_list[i].iSubdevice,
												single_list[i].iChannel,
												single_list[i].iValue,
												single_list[i].iTimeOut,
												single_list[i].iFlags);

						}
						else if (single_list[i].iDir == ME_DIR_INPUT)
						{
							single_list[i].iErrno = dev->me_device_io_single_read(
												dev,
												filep,
												single_list[i].iSubdevice,
												single_list[i].iChannel,
												&single_list[i].iValue,
												single_list[i].iTimeOut,
												single_list[i].iFlags);

						}
						else
						{
							PERROR("Invalid single direction specified.\n");
							single_list[i].iErrno = ME_ERRNO_INVALID_DIR;
						}
					}

				up_read(&me_rwsem);
				if ((single_list[i].iErrno) && !(karg.flags & ME_IO_SINGLE_NONBLOCKING))
				{
					karg.err_no = single_list[i].iErrno;
					break;
				}
			}

			ME_SPIN_LOCK(&me_lock);
			me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	if (copy_to_user(karg.single_list, single_list, sizeof(meIOSingle_t) * karg.count))
	{
		PERROR("Can't copy single list to user space.\n");
		err = -EFAULT;
	}

ERROR:
	if (copy_to_user(arg, &karg, sizeof(me_io_single_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	if (single_list)
		kfree(single_list);
	single_list = NULL;

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_single_simple(struct file* filep, me_io_single_simple_t* arg)
{
	me_device_t* dev = NULL;
	me_io_single_simple_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_single_simple_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);
				down_read(&me_rwsem);
					// Find device.
					karg.err_no = get_medevice(karg.device, &dev);
					if(!karg.err_no)
					{
						if (karg.dir == ME_DIR_OUTPUT)
						{
							karg.err_no = dev->me_device_io_single_write(
													dev,
													filep,
													karg.subdevice,
													karg.channel,
													karg.value,
													karg.timeout,
													karg.flags);

						}
						else if (karg.dir == ME_DIR_INPUT)
						{
							karg.err_no = dev->me_device_io_single_read(
													dev,
													filep,
													karg.subdevice,
													karg.channel,
													&karg.value,
													karg.timeout,
													karg.flags);

						}
						else
						{
							PERROR("Invalid single direction specified.\n");
							karg.err_no = ME_ERRNO_INVALID_DIR;
						}
					}
				up_read(&me_rwsem);
			ME_SPIN_LOCK(&me_lock);
			me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	if (copy_to_user(arg, &karg, sizeof(me_io_single_simple_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_stream_config(struct file* filep, me_io_stream_config_t* arg)
{
	me_device_t* dev = NULL;
	me_io_stream_config_t karg;
	meIOStreamSimpleConfig_t* config_list = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_stream_config_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	karg.err_no = ME_ERRNO_SUCCESS;

	if (karg.count < 1)
	{
		goto ERROR;
	}

	config_list = kmalloc(sizeof(meIOStreamSimpleConfig_t) * karg.count, GFP_KERNEL);
	if (!config_list)
	{
		PERROR("Can't get buffer for config list.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	if (copy_from_user(config_list, karg.config_list, sizeof(meIOStreamSimpleConfig_t) * karg.count))
	{
		PERROR("Can't copy config list to kernel space.\n");
		err = -EFAULT;
		goto ERROR;
	}

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);
				down_read(&me_rwsem);
					// Find device.
					karg.err_no = get_medevice(karg.device, &dev);
					if (!karg.err_no)
					{
						karg.err_no = dev->me_device_io_stream_config(
															dev,
															filep,
															karg.subdevice,
															config_list,
															karg.count,
															&karg.trigger,
															karg.fifo_irq_threshold,
															karg.flags);
					}
				up_read(&me_rwsem);
			ME_SPIN_LOCK(&me_lock);
				me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

ERROR:
	if (copy_to_user(arg, &karg, sizeof(me_io_stream_config_t)))
	{
		PERROR("Can't copy back to user space.\n");
		err = -EFAULT;
	}

	if (config_list)
		kfree(config_list);
	config_list = NULL;

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_stream_stop(struct file* filep, me_io_stream_stop_t* arg)
{
	int i;
	me_device_t* dev = NULL;
	me_io_stream_stop_t karg;
	meIOStreamStop_t* stop_list = NULL;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_io_stream_stop_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	karg.err_no = ME_ERRNO_SUCCESS;

	if (karg.count < 1)
	{
		goto ERROR;
	}

	if (karg.flags & ~ME_IO_STREAM_STOP_NONBLOCKING)
	{
		PERROR("Invalid flag specified. %d\n", karg.flags);
		karg.err_no = ME_ERRNO_INVALID_FLAGS;
		goto ERROR;
	}

	stop_list = kmalloc(sizeof(meIOStreamStop_t) * karg.count, GFP_KERNEL);
	if (!stop_list)
	{
		PERROR("Can't get buffer for stop list.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	if (copy_from_user(stop_list, karg.stop_list, sizeof(meIOStreamStop_t) * karg.count))
	{
		PERROR("Can't copy stop list to kernel space.\n");
		err = -EFAULT;
		goto ERROR;
	}

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");
			for (i = 0; i < karg.count; i++)
			{
				stop_list[i].iErrno = ME_ERRNO_LOCKED;
			}
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);
				for (i = 0; i < karg.count; i++)
				{
					down_read(&me_rwsem);
						// Find device.
						stop_list[i].iErrno = get_medevice(stop_list[i].iDevice, &dev);
						if (!stop_list[i].iErrno)
						{
							stop_list[i].iErrno = dev->me_device_io_stream_stop(
												dev,
												filep,
												stop_list[i].iSubdevice,
												stop_list[i].iStopMode,
												0,	//No timeout in this mode
												stop_list[i].iFlags);

						}
					up_read(&me_rwsem);
					if ((stop_list[i].iErrno) && !(karg.flags & ME_IO_STREAM_STOP_NONBLOCKING))
					{
						karg.err_no = stop_list[i].iErrno;
						break;
					}
				}
			ME_SPIN_LOCK(&me_lock);
			me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	if (copy_to_user(karg.stop_list, stop_list, sizeof(meIOStreamStop_t) * karg.count))
	{
		PERROR("Can't copy stop list to user space.\n");
		err = -EFAULT;
	}

ERROR:
	if (copy_to_user(arg, &karg, sizeof(me_io_stream_stop_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	if (stop_list)
		kfree(stop_list);
	stop_list = NULL;

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_io_stream_stop_simple(struct file* filep, me_io_stream_stop_simple_t* arg)
{
	me_device_t* dev = NULL;
	me_io_stream_stop_simple_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	if (copy_from_user(&karg, arg, sizeof(me_io_stream_stop_simple_t)))
	{
		PERROR("Can't copy arguments to kernel space.\n");
		return -EFAULT;
	}

	getnstimeofday(&ts_pre);

	ME_SPIN_LOCK(&me_lock);
		if ((me_filep != NULL) && (me_filep != filep))
		{
			PERROR("Driver is locked by another process.\n");
			karg.err_no = ME_ERRNO_LOCKED;
		}
		else
		{
			me_count++;
			ME_SPIN_UNLOCK(&me_lock);
				down_read(&me_rwsem);
					// Find device.
					karg.err_no = get_medevice(karg.device, &dev);
					if (!karg.err_no)
					{
						karg.err_no = dev->me_device_io_stream_stop(
												dev,
												filep,
												karg.subdevice,
												karg.mode,
												karg.time_out,
												karg.flags);

					}
				up_read(&me_rwsem);
			ME_SPIN_LOCK(&me_lock);
			me_count--;
		}
	ME_SPIN_UNLOCK(&me_lock);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	if (copy_to_user(arg, &karg, sizeof(me_io_stream_stop_simple_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

	return err;
}

int me_query_name_main_driver(struct file* filep, me_query_name_main_driver_t* arg)
{
	me_query_name_main_driver_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if (copy_from_user(&karg, arg, sizeof(me_query_name_main_driver_t)))
	{
		PERROR("Can't copy arguments to kernel space\n");
		return -EFAULT;
	}

	if ((strlen(ME_NAME_DRIVER) + 1) > karg.count)
	{
		PERROR("User buffer for device name is to short.\n");
		karg.err_no = ME_ERRNO_USER_BUFFER_SIZE;
	}
	else
	{
		karg.err_no = ME_ERRNO_SUCCESS;
	}

	if (copy_to_user(karg.name, ME_NAME_DRIVER, ((strlen(ME_NAME_DRIVER) + 1) > karg.count) ? karg.count : strlen(ME_NAME_DRIVER) + 1))
	{
		PERROR("Can't copy driver name to user space\n");
		err = -EFAULT;
		goto EXIT;
	}

	if (copy_to_user(arg, &karg, sizeof(me_query_name_main_driver_t)))
	{
		PERROR("Can't copy arguments to user space.\n");
		err = -EFAULT;
	}

EXIT:
	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_query_number_devices(struct file* filep, me_query_number_devices_t* arg)
{
	me_query_number_devices_t karg;
	struct list_head* pos;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	karg.err_no = ME_ERRNO_SUCCESS;
	karg.number = 0;
	down_read(&me_rwsem);
		list_for_each(pos, &me_device_list)
		{
			karg.number++;
		}
	up_read(&me_rwsem);

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	if (copy_to_user(arg, &karg, sizeof(me_query_number_devices_t)))
	{
		PERROR("Can't copy query back to user space.\n");
		err = -EFAULT;
	}

	return err;
}

int me_query_version_main_driver(struct file* filep, me_query_version_main_driver_t* arg)
{
	me_query_version_main_driver_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	karg.version = ME_VERSION_DRIVER;
	karg.err_no = ME_ERRNO_SUCCESS;

	if (copy_to_user(arg, &karg, sizeof(me_query_version_main_driver_t)))
	{
		PERROR("Can't copy query back to user space.\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

static int me_query_type(struct file* filep, me_query_type_driver_t* arg)
{
	me_query_type_driver_t karg;
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

# if defined(ME_PCI)
	karg.type = ME_DRIVER_PCI;
# elif defined(ME_USB)
	karg.type = ME_DRIVER_USB;
# else
	karg.type = ME_DRIVER_UNKNOWN;
#endif

	if (copy_to_user(arg, &karg, sizeof(me_query_type_driver_t)))
	{
		PERROR("Can't copy query back to user space.\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}

int me_query_subdevice_caps_args(struct file* filep, me_query_subdevice_caps_args_t* arg)
{
	me_device_t* dev = NULL;
	me_query_subdevice_caps_args_t karg;
	int err = ME_ERRNO_SUCCESS;
	int* k_args_buf;
	int buff_size;

	struct timespec ts_pre;
	struct timespec ts_post;
	struct timespec ts_exec;

	PDEBUG("executed.\n");

	getnstimeofday(&ts_pre);

	if(copy_from_user(&karg, arg, sizeof(me_query_subdevice_caps_args_t)))
	{
		PERROR("Can't copy arguments from user space\n");
		return -EFAULT;
	}

	buff_size = karg.count * sizeof(int);
	k_args_buf = kmalloc(buff_size, GFP_KERNEL);
	if (k_args_buf)
	{
		down_read(&me_rwsem);
			karg.err_no = get_medevice(karg.device, &dev);
			if (!karg.err_no)
			{
				karg.err_no = dev->me_device_query_subdevice_caps_args(dev, karg.subdevice, karg.cap, k_args_buf, &karg.count);
			}
		up_read(&me_rwsem);

		if (copy_to_user(karg.args, k_args_buf, buff_size))
		{
			PERROR("Cannot copy cap argsuments to user.\n");
			karg.err_no = -EFAULT;
		}

		kfree(k_args_buf);
	}
	else
	{
			PERROR("Cannot get memeory for CAPS ARG buffer.\n");
			karg.err_no = -ENOMEM;
	}

	if(copy_to_user(arg, &karg, sizeof(me_query_subdevice_caps_args_t)))
	{
		PERROR("Can't copy arguments to user space\n");
		err = -EFAULT;
	}

	getnstimeofday(&ts_post);
	ts_exec = timespec_sub(ts_post, ts_pre);

	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC);

	return err;
}
