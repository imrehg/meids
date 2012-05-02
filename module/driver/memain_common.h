/**
 * @file memain_common.h
 *
 * @brief Part of memain. Implementation of the common functions (PCI and SynapseUSB).
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

#ifdef __KERNEL__

# if !defined(ME_PCI) && !defined(ME_USB) && !defined(ME_MEPHISTO)
#  error NO VALID DRIVER TYPE declared!
# endif

# ifndef _MEMAIN_COMMON_H_
#  define _MEMAIN_COMMON_H_

#  include <linux/fs.h>

#  include "me_debug.h"
#  include "medevice.h"
// #  include "me_spin_lock.h"

extern struct file* me_filep;
extern int me_count;

extern me_lock_t me_lock;
extern struct rw_semaphore me_rwsem;

// Board instances are kept in a global list.
extern struct list_head me_device_list;

///Implementation
	//menagment
	int get_medevice(int dev_no, me_device_t** device);
	#if defined(ME_PCI)
	me_device_t* find_device_on_list(struct pci_local_dev* n_device, int state);
	#elif defined(ME_USB)
	me_device_t* find_device_on_list(struct NET2282_usb_device* n_device, int state);
	#elif defined(ME_MEPHISTO)
	me_device_t* find_device_on_list(struct mephisto_usb_device* n_device, int state);
	#else
		//Only to mask parser warning.
		#error neither ME_PCI, ME_USB nor ME_MEPHISTO defined!
	#endif

	void insert_to_device_list(me_device_t *n_device);
	void release_instance(me_device_t *device);
	void clear_device_list(void);

	//ACCESS
	int me_open(struct inode* inode_ptr, struct file* filep);
	int me_release(struct inode *inode_ptr, struct file* filep);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	int me_ioctl(struct inode *inodep, struct file* filep, unsigned int service, unsigned long arg);
#else
	long me_ioctl(struct file* filep, unsigned int service, unsigned long arg);
#endif

	//RESET
	int me_io_reset_device(struct file* filep, me_io_reset_device_t* arg);
	int me_io_reset_subdevice(struct file* filep, me_io_reset_subdevice_t* arg);

	// IRQ
	int me_io_irq_start(struct file* filep, me_io_irq_start_t* arg);
	int me_io_irq_wait(struct file* filep, me_io_irq_wait_t* arg);
	int me_io_irq_stop(struct file* filep, me_io_irq_stop_t* arg);
	int me_io_irq_test(struct file* filep, me_io_irq_test_t* arg);

	//SINGLE
	int me_io_single_config(struct file* filep, me_io_single_config_t* arg);
	int me_io_single(struct file* filep, me_io_single_t* arg);
	int me_io_single_simple(struct file* filep, me_io_single_simple_t* arg);

	//STREAM
	int me_io_stream_config(struct file* filep, me_io_stream_config_t* arg);
	int me_io_stream_read(struct file* filep, me_io_stream_read_t* arg);
	int me_io_stream_timeout_read(struct file* filep, me_io_stream_timeout_read_t* arg);
	int me_io_stream_write(struct file* filep, me_io_stream_write_t* arg);
	int me_io_stream_timeout_write(struct file* filep, me_io_stream_timeout_write_t* arg);
	int me_io_stream_new_values(struct file* filep, me_io_stream_new_values_t* arg);
	int me_io_stream_start_simple(struct file* filep, me_io_stream_start_simple_t* arg);
	int me_io_stream_stop_simple(struct file* filep, me_io_stream_stop_simple_t* arg);
	int me_io_stream_start(struct file* filep, me_io_stream_start_t* arg);
	int me_io_stream_stop(struct file* filep, me_io_stream_stop_t* arg);
	int me_io_stream_status(struct file* filep, me_io_stream_status_t* arg);

	int me_set_offset(struct file* filep, me_set_offset_t* arg);

	//LOCKS
	int me_lock_driver(struct file* filep, me_lock_driver_t* arg);
	int me_lock_device(struct file* filep, me_lock_device_t* arg);
	int me_lock_subdevice(struct file* filep, me_lock_subdevice_t* arg);

	//QUERY
	int me_query_info_device(struct file* filep, me_query_info_device_t* arg);
	int me_query_description_device(struct file* filep, me_query_description_device_t* arg);
	int me_query_name_device(struct file* filep, me_query_name_device_t* arg);
	int me_query_name_device_driver(struct file* filep, me_query_name_device_driver_t* arg);
	int me_query_name_main_driver(struct file* filep, me_query_name_main_driver_t* arg);
	int me_query_number_devices(struct file* filep, me_query_number_devices_t* arg);
	int me_query_number_subdevices(struct file* filep, me_query_number_subdevices_t* arg);
	int me_query_number_subdevices_by_type(struct file* filep, me_query_number_subdevices_by_type_t* arg);
	int me_query_number_channels(struct file* filep, me_query_number_channels_t* arg);
	int me_query_number_ranges(struct file* filep, me_query_number_ranges_t* arg);
	int me_query_range_by_min_max(struct file* filep, me_query_range_by_min_max_t* arg);
	int me_query_range_info(struct file* filep, me_query_range_info_t* arg);
	int me_query_subdevice_by_type(struct file* filep, me_query_subdevice_by_type_t* arg);
	int me_query_subdevice_type(struct file* filep, me_query_subdevice_type_t* arg);
	int me_query_subdevice_caps(struct file* filep, me_query_subdevice_caps_t* arg);
	int me_query_subdevice_caps_args(struct file* filep, me_query_subdevice_caps_args_t* arg);
	int me_query_timer(struct file* filep, me_query_timer_t* arg);
	int me_query_version_main_driver(struct file* filep, me_query_version_main_driver_t *arg);
	int me_query_version_device_driver(struct file* filep, me_query_version_device_driver_t* arg);
	int me_query_device_release(struct file* filep, me_query_device_release_t* arg);
	int me_query_version_firmware(struct file* filep, me_query_version_firmware_t* arg);

	int me_config_load(struct file* filep, me_extra_param_set_t* arg);

# endif	//_MEMAIN_COMMON_H_
#endif	//__KERNEL__
