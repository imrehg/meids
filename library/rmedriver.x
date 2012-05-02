/*
 * rmedriver.x: Remote intelligent Meilhaus Driver System protocol
 */


/*===========================================================================
  Data types for lock functions
  =========================================================================*/

struct me_lock_driver_params {
	int lock;
	int flags;
};


struct me_lock_device_params {
	int device;
	int lock;
	int flags;
};


struct me_lock_subdevice_params {
	int device;
	int subdevice;
	int lock;
	int flags;
};


/*===========================================================================
  Data types for input/output functions
  =========================================================================*/

struct me_io_irq_start_params {
	int device;
	int subdevice;
	int channel;
	int irq_source;
	int irq_edge;
	int irq_arg;
	int flags;
};


struct me_io_irq_stop_params {
	int device;
	int subdevice;
	int channel;
	int flags;
};


struct me_io_irq_wait_params {
	int device;
	int subdevice;
	int channel;
	int time_out;
	int flags;
};


struct me_io_irq_wait_res {
	int error;
	int irq_count;
	int value;
};


struct me_io_reset_device_params {
	int device;
	int flags;
};


struct me_io_reset_subdevice_params {
	int device;
	int subdevice;
	int flags;
};


struct me_io_single_config_params {
	int device;
	int subdevice;
	int channel;
	int single_config;
	int ref;
	int trig_chan;
	int trig_type;
	int trig_edge;
	int flags;
};


struct me_io_single_entry_params {
	int device;
	int subdevice;
	int channel;
	int dir;
	int value;
	int time_out;
	int flags;
	int error;
};


struct me_io_single_params {
	me_io_single_entry_params single_list<>;
	int flags;
};


struct me_io_single_entry_res {
	int value;
	int error;
};


struct me_io_single_res {
	int error;
	me_io_single_entry_res single_list<>;
};


struct me_io_stream_config_entry_params {
	int channel;
	int stream_config;
	int ref;
	int flags;
};


struct me_io_stream_trigger_params {
	int acq_start_trig_type;
	int acq_start_trig_edge;
	int acq_start_trig_chan;
	int acq_start_ticks_low;
	int acq_start_ticks_high;
	int acq_start_args<10>;
	int scan_start_trig_type;
	int scan_start_ticks_low;
	int scan_start_ticks_high;
	int scan_start_args<10>;
	int conv_start_trig_type;
	int conv_start_ticks_low;
	int conv_start_ticks_high;
	int conv_start_args<10>;
	int scan_stop_trig_type;
	int scan_stop_count;
	int scan_stop_args<10>;
	int acq_stop_trig_type;
	int acq_stop_count;
	int acq_stop_args<10>;
	int flags;
};


struct me_io_stream_config_params {
	int device;
	int subdevice;
	me_io_stream_config_entry_params config_list<>;
	me_io_stream_trigger_params trigger;
	int fifo_irq_threshold;
	int flags;
};


struct me_io_stream_new_values_params {
	int device;
	int subdevice;
	int time_out;
	int flags;
};

struct me_io_stream_new_values_res {
	int error;
	int count;
};


struct me_io_stream_read_params {
	int device;
	int subdevice;
	int read_mode;
	int count;
	int flags;
};


struct me_io_stream_read_res {
	int error;
	int values<>;
};


struct me_io_stream_write_params {
	int device;
	int subdevice;
	int write_mode;
	int values<>;
	int flags;
};


struct me_io_stream_write_res {
	int error;
	int count;
};


struct me_io_stream_start_entry_params {
	int device;
	int subdevice;
	int start_mode;
	int time_out;
	int flags;
	int error;
};


struct me_io_stream_start_params {
	me_io_stream_start_entry_params start_list<>;
	int flags;
};


struct me_io_stream_start_res {
	int error;
	int start_list<>;
};


struct me_io_stream_stop_entry_params {
	int device;
	int subdevice;
	int stop_mode;
	int flags;
	int error;
};


struct me_io_stream_stop_params {
	me_io_stream_stop_entry_params stop_list<>;
	int flags;
};


struct me_io_stream_stop_res {
	int error;
	int stop_list<>;
};


struct me_io_stream_status_params {
	int device;
	int subdevice;
	int wait;
	int flags;
};

struct me_io_stream_status_res {
	int error;
	int status;
	int count;
};


struct me_io_stream_frequency_to_ticks_params {
	int device;
	int subdevice;
	int timer;
	double frequency;
	int flags;
};


struct me_io_stream_frequency_to_ticks_res {
	int error;
	double frequency;
	int ticks_low;
	int ticks_high;
};


struct me_io_stream_time_to_ticks_params {
	int device;
	int subdevice;
	int timer;
	double time;
	int flags;
};


struct me_io_stream_time_to_ticks_res {
	int error;
	double time;
	int ticks_low;
	int ticks_high;
};


/*===========================================================================
  Data types for query functions
  =========================================================================*/

struct me_query_description_device_res {
	int error;
	string description<>;
};

struct me_query_info_device_res {
	int error;
	int vendor_id;
	int device_id;
	int serial_no;
	int bus_type;
	int bus_no;
	int dev_no;
	int func_no;
	int plugged;
};

struct me_query_name_device_res {
	int error;
	string name<>;
};

struct me_query_name_device_driver_res {
	int error;
	string name<>;
};

struct me_query_number_devices_res {
	int error;
	int number;
};

struct me_query_number_subdevices_res {
	int error;
	int number;
};

struct me_query_number_channels_params {
	int device;
	int subdevice;
};

struct me_query_number_channels_res {
	int error;
	int number;
};

struct me_query_number_ranges_params {
	int device;
	int subdevice;
	int unit;
};

struct me_query_number_ranges_res {
	int error;
	int number;
};

struct me_query_range_by_min_max_params {
	int device;
	int subdevice;
	int unit;
	double min;
	double max;
};

struct me_query_range_by_min_max_res {
	int error;
	double min;
	double max;
	int max_data;
	int range;
};

struct me_query_range_info_params {
	int device;
	int subdevice;
	int range;
};

struct me_query_range_info_res {
	int error;
	int unit;
	double min;
	double max;
	int max_data;
};

struct me_query_subdevice_by_type_params {
	int device;
	int start_subdevice;
	int type;
	int subtype;
};

struct me_query_subdevice_by_type_res {
	int error;
	int subdevice;
};

struct me_query_subdevice_type_params {
	int device;
	int subdevice;
};

struct me_query_subdevice_type_res {
	int error;
	int type;
	int subtype;
};

struct me_query_subdevice_caps_params {
	int device;
	int subdevice;
};

struct me_query_subdevice_caps_res {
	int error;
	int caps;
};

struct me_query_subdevice_caps_args_params {
	int device;
	int subdevice;
	int cap;
	int count;
};

struct me_query_subdevice_caps_args_res {
	int error;
	int args<>;
};

struct me_query_version_library_res {
	int error;
	int ver;
};

struct me_query_version_main_driver_res {
	int error;
	int ver;
};

struct me_query_version_device_driver_res {
	int error;
	int ver;
};

program RMEDRIVER_PROG {
	version RMEDRIVER_VERS {
		int ME_CLOSE_PROC(int) = 1;
		int ME_OPEN_PROC(int) = 2;

		int ME_LOCK_DRIVER_PROC(me_lock_driver_params) = 3;
		int ME_LOCK_DEVICE_PROC(me_lock_device_params) = 4;
		int ME_LOCK_SUBDEVICE_PROC(me_lock_subdevice_params) = 5;

		int ME_IO_IRQ_START_PROC(me_io_irq_start_params) = 6;
		int ME_IO_IRQ_STOP_PROC(me_io_irq_stop_params) = 7;
		me_io_irq_wait_res ME_IO_IRQ_WAIT_PROC(me_io_irq_wait_params) = 8;
		int ME_IO_RESET_DEVICE_PROC(me_io_reset_device_params) = 9;
		int ME_IO_RESET_SUBDEVICE_PROC(me_io_reset_subdevice_params) = 10;
		int ME_IO_SINGLE_CONFIG_PROC(me_io_single_config_params) = 11;
		me_io_single_res ME_IO_SINGLE_PROC(me_io_single_params) = 12;
		int ME_IO_STREAM_CONFIG_PROC(me_io_stream_config_params) = 13;
		me_io_stream_read_res ME_IO_STREAM_READ_PROC(me_io_stream_read_params) = 14;
		me_io_stream_write_res ME_IO_STREAM_WRITE_PROC(me_io_stream_write_params) = 15;
		me_io_stream_start_res ME_IO_STREAM_START_PROC(me_io_stream_start_params) = 16;
		me_io_stream_stop_res ME_IO_STREAM_STOP_PROC(me_io_stream_stop_params) = 17;
		me_io_stream_status_res ME_IO_STREAM_STATUS_PROC(me_io_stream_status_params) = 18;
		me_io_stream_frequency_to_ticks_res ME_IO_STREAM_FREQUENCY_TO_TICKS_PROC(me_io_stream_frequency_to_ticks_params) = 19;
		me_io_stream_time_to_ticks_res ME_IO_STREAM_TIME_TO_TICKS_PROC(me_io_stream_time_to_ticks_params) = 20;
		me_io_stream_new_values_res ME_IO_STREAM_NEW_VALUES_PROC(me_io_stream_new_values_params) = 21;

		me_query_description_device_res ME_QUERY_DESCRIPTION_DEVICE_PROC(int) = 22;
		me_query_info_device_res ME_QUERY_INFO_DEVICE_PROC(int) = 23;
		me_query_name_device_res ME_QUERY_NAME_DEVICE_PROC(int) = 24;
		me_query_name_device_driver_res ME_QUERY_NAME_DEVICE_DRIVER_PROC(int) = 25;
		me_query_number_devices_res ME_QUERY_NUMBER_DEVICES_PROC() = 26;
		me_query_number_subdevices_res ME_QUERY_NUMBER_SUBDEVICES_PROC(int) = 27;
		me_query_number_channels_res ME_QUERY_NUMBER_CHANNELS_PROC(me_query_number_channels_params) = 28;
		me_query_number_ranges_res ME_QUERY_NUMBER_RANGES_PROC(me_query_number_ranges_params) = 29;
		me_query_range_by_min_max_res ME_QUERY_RANGE_BY_MIN_MAX_PROC(me_query_range_by_min_max_params) = 30;
		me_query_range_info_res ME_QUERY_RANGE_INFO_PROC(me_query_range_info_params) = 31;
		me_query_subdevice_by_type_res ME_QUERY_SUBDEVICE_BY_TYPE_PROC(me_query_subdevice_by_type_params) = 32;
		me_query_subdevice_type_res ME_QUERY_SUBDEVICE_TYPE_PROC(me_query_subdevice_type_params) = 33;
		me_query_subdevice_caps_res ME_QUERY_SUBDEVICE_CAPS_PROC(me_query_subdevice_caps_params) = 34;
		me_query_subdevice_caps_args_res ME_QUERY_SUBDEVICE_CAPS_ARGS_PROC(me_query_subdevice_caps_args_params) = 35;
		me_query_version_library_res ME_QUERY_VERSION_LIBRARY_PROC() = 36;
		me_query_version_main_driver_res ME_QUERY_VERSION_MAIN_DRIVER_PROC() = 37;
		me_query_version_device_driver_res ME_QUERY_VERSION_DEVICE_DRIVER_PROC(int) = 38;
	} = 1;
} = 0x20000001;
