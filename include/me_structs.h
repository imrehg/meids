/**
 * @file me_structs.h
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH <support@meilhaus.de>
 *
 * @brief Provides the types for the input/output ioctls.
 *
 * @author KG (Krzysztof Gantzke) <k.gantzke@meilhaus.de>
 */

#ifndef _ME_STRUCTS_H_
# define _ME_STRUCTS_H_

#include "me_types.h"
#include "me_data_types.h"

///  Types for the irq ioctls
typedef struct //me_io_irq_start
{
	int device;
	int subdevice;
	int channel;
	int irq_source;
	int irq_edge;
	int irq_arg;
	int flags;
	int err_no;
} me_io_irq_start_t;

typedef struct //me_io_irq_stop
{
	int device;
	int subdevice;
	int channel;
	int flags;
	int err_no;
} me_io_irq_stop_t;

typedef struct //me_io_irq_wait
{
	int device;
	int subdevice;
	int channel;
	int irq_count;
	int value;
	int time_out;
	int flags;
	int err_no;
} me_io_irq_wait_t;

typedef struct //me_io_irq_test
{
	int device;
	int subdevice;
	int channel;
	int flags;
	int err_no;
} me_io_irq_test_t;


///  Types for the reset ioctls
typedef struct //me_io_reset_device
{
	int device;
	int flags;
	int err_no;
} me_io_reset_device_t;

typedef struct //me_io_reset_subdevice
{
	int device;
	int subdevice;
	int flags;
	int err_no;
} me_io_reset_subdevice_t;

///  Types for the single ioctls
typedef struct //me_io_single_config
{
	int device;
	int subdevice;
	int channel;
	int single_config;
	int ref;
	int trig_chain;
	int trig_type;
	int trig_edge;
	int flags;
	int err_no;
} me_io_single_config_t;

typedef struct //me_io_single_simple
{
	int device;
	int subdevice;
	int channel;
	int dir;
	int value;
	int timeout;
	int flags;
	int err_no;
} me_io_single_simple_t;

typedef struct //me_io_single
{
	meIOSingle_t *single_list;
	int count;
	int flags;
	int err_no;
} me_io_single_t;

///  Types for the stream ioctls
typedef struct //me_io_stream_config
{
	int device;
	int subdevice;
	meIOStreamSimpleConfig_t *config_list;
	int count;
	meIOStreamSimpleTriggers_t trigger;
	int fifo_irq_threshold;
	int flags;
	int err_no;
} me_io_stream_config_t;

typedef struct //me_io_stream_new_values
{
	int device;
	int subdevice;
	int time_out;
	int count;
	int flags;
	int err_no;
} me_io_stream_new_values_t;


typedef struct //me_io_stream_timeout_read
{
	int device;
	int subdevice;
	int read_mode;
	int *values;
	int count;
	int timeout;
	int flags;
	int err_no;
} me_io_stream_timeout_read_t;

typedef struct //me_io_stream_read
{
	int device;
	int subdevice;
	int read_mode;
	int *values;
	int count;
	int flags;
	int err_no;
} me_io_stream_read_t;

typedef struct //me_io_stream_start
{
	meIOStreamStart_t *start_list;
	int count;
	int flags;
	int err_no;
} me_io_stream_start_t;

typedef struct //me_io_stream_start_simple
{
	int device;
	int subdevice;
	int mode;
	int timeout;
	int flags;
	int err_no;
} me_io_stream_start_simple_t;

typedef struct //me_io_stream_status
{
	int device;
	int subdevice;
	int wait;
	int status;
	int count;
	int flags;
	int err_no;
} me_io_stream_status_t;

typedef struct //me_io_stream_stop_simple
{
	int device;
	int subdevice;
	int mode;
	int time_out;
	int flags;
	int err_no;
} me_io_stream_stop_simple_t;

typedef struct //me_io_stream_stop
{
	meIOStreamStop_t *stop_list;
	int count;
	int flags;
	int err_no;
} me_io_stream_stop_t;

typedef struct //me_io_stream_write
{
	int device;
	int subdevice;
	int write_mode;
	int *values;
	int count;
	int flags;
	int err_no;
} me_io_stream_write_t;

typedef struct //me_io_stream_timeout_write
{
	int device;
	int subdevice;
	int write_mode;
	int *values;
	int count;
	int timeout;
	int flags;
	int err_no;
} me_io_stream_timeout_write_t;

typedef struct //me_set_offset
{
	int device;
	int subdevice;
	int channel;
	int range;
	int offset;
	int flags;
	int err_no;
} me_set_offset_t;


///  Types for the lock ioctls
typedef struct //me_lock_driver
{
	int flags;
	int lock;
	int err_no;
} me_lock_driver_t;

typedef struct //me_lock_device
{
	int device;
	int lock;
	int flags;
	int err_no;
} me_lock_device_t;

typedef struct //me_lock_subdevice
{
	int device;
	int subdevice;
	int lock;
	int flags;
	int err_no;
} me_lock_subdevice_t;


///  Types for the query ioctls

typedef struct //me_query_info_device
{
	int device;
	int vendor_id;
	int device_id;
	int serial_no;
	int bus_type;
	int bus_no;
	int dev_no;
	int func_no;
	int plugged;
	int err_no;
} me_query_info_device_t;

typedef struct //me_query_description_device
{
	int device;
	char *name;
	int count;
	int err_no;
} me_query_description_device_t;

typedef struct //me_query_name_device
{
	int device;
	char *name;
	int count;
	int err_no;
} me_query_name_device_t;

typedef struct //me_query_name_device_driver
{
	int device;
	char *name;
	int count;
	int err_no;
} me_query_name_device_driver_t;

typedef struct //me_query_version_main_driver
{
	int version;
	int err_no;
} me_query_version_main_driver_t;

typedef struct //me_query_version_device_driver
{
	int device;
	int version;
	int err_no;
} me_query_version_device_driver_t;

typedef struct //me_query_number_devices
{
	int number;
	int err_no;
} me_query_number_devices_t;

typedef struct //me_query_number_subdevices
{
	int device;
	int number;
	int err_no;
} me_query_number_subdevices_t;

typedef struct //me_query_number_subdevices_by_type
{
	int device;
	int type;
	int subtype;
	int number;
	int err_no;
} me_query_number_subdevices_by_type_t;

typedef struct //me_query_number_channels
{
	int device;
	int subdevice;
	int number;
	int err_no;
} me_query_number_channels_t;

typedef struct //me_query_number_ranges
{
	int device;
	int subdevice;
	int channel;
	int unit;
	int number;
	int err_no;
} me_query_number_ranges_t;

typedef struct //me_query_subdevice_by_type
{
	int device;
	int start_subdevice;
	int type;
	int subtype;
	int subdevice;
	int err_no;
} me_query_subdevice_by_type_t;

typedef struct //me_query_subdevice_type
{
	int device;
	int subdevice;
	int type;
	int subtype;
	int err_no;
} me_query_subdevice_type_t;

typedef struct //me_query_subdevice_caps
{
	int device;
	int subdevice;
	int caps;
	int err_no;
} me_query_subdevice_caps_t;

typedef struct //me_query_subdevice_caps_args
{
	int device;
	int subdevice;
	int cap;
	int* args;
	int count;
	int err_no;
} me_query_subdevice_caps_args_t;

typedef struct //me_query_timer
{
	int device;
	int subdevice;
	int timer;
	int base_frequency;
	long long min_ticks;
	long long max_ticks;
	int err_no;
} me_query_timer_t;

typedef struct //me_query_range_by_min_max
{
	int device;
	int subdevice;
	int channel;
	int unit;
	int min;
	int max;
	int max_data;
	int range;
	int err_no;
} me_query_range_by_min_max_t;

typedef struct //me_query_range_info
{
	int device;
	int subdevice;
	int channel;
	int unit;
	int range;
	int min;
	int max;
	int max_data;
	int err_no;
} me_query_range_info_t;

typedef struct //me_query_name_main_driver
{
	char *name;
	int count;
	int err_no;
} me_query_name_main_driver_t;

typedef struct //me_query_device_release
{
	int device;
	int version;
	int err_no;
} me_query_device_release_t;

typedef struct //me_query_version_firmware
{
	int device;
	int subdevice;
	int version;
	int err_no;
} me_query_version_firmware_t;

typedef struct //me_query_type_driver
{
	int type;
} me_query_type_driver_t;

#endif	//_ME_STRUCTS_H_
