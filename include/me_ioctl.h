/**
 * @file me_ioctl.h
 *
 * @brief ME-1000 device class header file.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
 * Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
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

#include "me_types.h"
#include "me_structs.h"

///  The ioctls of the board
#ifndef _ME_IOCTL_H_
# define _ME_IOCTL_H_

# define MEMAIN_MAGIC 'y'

# define ME_IO_IRQ_ENABLE					_IOR (MEMAIN_MAGIC, 1, me_io_irq_start_t)
# define ME_IO_IRQ_DISABLE					_IOR (MEMAIN_MAGIC, 2, me_io_irq_stop_t)
# define ME_IO_IRQ_WAIT						_IOR (MEMAIN_MAGIC, 3, me_io_irq_wait_t)
# define ME_IO_IRQ_CHECK					_IOR (MEMAIN_MAGIC, 3, me_io_irq_test_t)

# define ME_IO_RESET_DEVICE					_IOW (MEMAIN_MAGIC, 4, me_io_reset_device_t)
# define ME_IO_RESET_SUBDEVICE				_IOW (MEMAIN_MAGIC, 5, me_io_reset_subdevice_t)

# define ME_IO_SINGLE_CONFIG				_IOW (MEMAIN_MAGIC, 6, me_io_single_config_t)
# define ME_IO_SINGLE						_IOWR(MEMAIN_MAGIC, 7, me_io_single_t)
# define ME_IO_SINGLE_SIMPLE				_IOWR(MEMAIN_MAGIC, 8, me_io_single_simple_t)

# define ME_IO_STREAM_CONFIG				_IOW (MEMAIN_MAGIC, 9, me_io_stream_config_t)
# define ME_IO_STREAM_READ					_IOR (MEMAIN_MAGIC, 10, me_io_stream_timeout_read_t)
# define ME_IO_STREAM_TIMEOUT_READ			_IOR (MEMAIN_MAGIC, 11, me_io_stream_read_t)
# define ME_IO_STREAM_WRITE					_IOW (MEMAIN_MAGIC, 12, me_io_stream_write_t)
# define ME_IO_STREAM_TIMEOUT_WRITE			_IOW (MEMAIN_MAGIC, 13, me_io_stream_timeout_write_t)
# define ME_IO_STREAM_NEW_VALUES			_IOR (MEMAIN_MAGIC, 14, me_io_stream_new_values_t)
# define ME_IO_STREAM_START					_IOW (MEMAIN_MAGIC, 15, me_io_stream_start_t)
# define ME_IO_STREAM_START_SIMPLE			_IOW (MEMAIN_MAGIC, 16, me_io_stream_start_simple_t)
# define ME_IO_STREAM_STOP					_IOW (MEMAIN_MAGIC, 17, me_io_stream_stop_t)
# define ME_IO_STREAM_STOP_SIMPLE			_IOW (MEMAIN_MAGIC, 18, me_io_stream_stop_simple_t)
# define ME_IO_STREAM_STATUS				_IOR (MEMAIN_MAGIC, 19, me_io_stream_status_t)

# define ME_LOCK_DRIVER						_IOW (MEMAIN_MAGIC, 20, me_lock_driver_t)
# define ME_LOCK_DEVICE						_IOW (MEMAIN_MAGIC, 21, me_lock_device_t)
# define ME_LOCK_SUBDEVICE					_IOW (MEMAIN_MAGIC, 22, me_lock_subdevice_t)

# define ME_QUERY_DESCRIPTION_DEVICE		_IOR (MEMAIN_MAGIC, 23, me_query_description_device_t)

# define ME_QUERY_INFO_DEVICE				_IOR (MEMAIN_MAGIC, 24, me_query_info_device_t)

# define ME_QUERY_NAME_DEVICE				_IOR (MEMAIN_MAGIC, 25, me_query_name_device_t)
# define ME_QUERY_NAME_DEVICE_DRIVER		_IOR (MEMAIN_MAGIC, 26, me_query_name_device_driver_t)

# define ME_QUERY_NUMBER_DEVICES			_IOR (MEMAIN_MAGIC, 27, me_query_number_devices_t)
# define ME_QUERY_NUMBER_SUBDEVICES			_IOR (MEMAIN_MAGIC, 28, me_query_number_subdevices_t)
# define ME_QUERY_NUMBER_CHANNELS			_IOR (MEMAIN_MAGIC, 29, me_query_number_channels_t)
# define ME_QUERY_NUMBER_RANGES				_IOR (MEMAIN_MAGIC, 30, me_query_number_ranges_t)

# define ME_QUERY_RANGE_BY_MIN_MAX			_IOR (MEMAIN_MAGIC, 31, me_query_range_by_min_max_t)
# define ME_QUERY_RANGE_INFO				_IOR (MEMAIN_MAGIC, 32, me_query_range_info_t)

# define ME_QUERY_SUBDEVICE_BY_TYPE			_IOR (MEMAIN_MAGIC, 33, me_query_subdevice_by_type_t)
# define ME_QUERY_SUBDEVICE_TYPE			_IOR (MEMAIN_MAGIC, 34, me_query_subdevice_type_t)
# define ME_QUERY_SUBDEVICE_CAPS			_IOR (MEMAIN_MAGIC, 35, me_query_subdevice_caps_t)
# define ME_QUERY_SUBDEVICE_CAPS_ARGS		_IOR (MEMAIN_MAGIC, 36, me_query_subdevice_caps_args_t)

# define ME_QUERY_TIMER						_IOR (MEMAIN_MAGIC, 37, me_query_timer_t)

# define ME_QUERY_VERSION_DEVICE_DRIVER		_IOR (MEMAIN_MAGIC, 38, me_query_version_device_driver_t)
# define ME_QUERY_VERSION_MAIN_DRIVER		_IOR (MEMAIN_MAGIC, 39, me_query_version_main_driver_t)
# define ME_QUERY_NAME_MAIN_DRIVER			_IOR (MEMAIN_MAGIC, 40, me_query_name_main_driver_t)

# define ME_QUERY_DEVICE_RELEASE			_IOR (MEMAIN_MAGIC, 41, me_query_device_release_t)
# define ME_QUERY_VERSION_FIRMWARE			_IOR (MEMAIN_MAGIC, 42, me_query_version_firmware_t)

# define ME_QUERY_TYPE_DRIVER				_IOR (MEMAIN_MAGIC, 43, me_query_type_driver_t)

# define ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE	_IOR (MEMAIN_MAGIC, 44, me_query_number_subdevices_by_type_t)

# define ME_SET_OFFSET						_IOW (MEMAIN_MAGIC, 45, me_set_offset_t)

# define ME_CONFIG_LOAD						_IOWR(MEMAIN_MAGIC, 63, me_extra_param_set_t)

#endif
