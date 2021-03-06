/*
* Please do not edit this file.
* It was generated using rpcgen.
 */

#ifndef _RMEDRIVER_H_RPCGEN
#define _RMEDRIVER_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct me_lock_driver_params {
	int lock;
	int flags;
};
typedef struct me_lock_driver_params me_lock_driver_params;

struct me_lock_device_params {
	int device;
	int lock;
	int flags;
};
typedef struct me_lock_device_params me_lock_device_params;

struct me_lock_subdevice_params {
	int device;
	int subdevice;
	int lock;
	int flags;
};
typedef struct me_lock_subdevice_params me_lock_subdevice_params;

struct me_io_irq_start_params {
	int device;
	int subdevice;
	int channel;
	int irq_source;
	int irq_edge;
	int irq_arg;
	int flags;
};
typedef struct me_io_irq_start_params me_io_irq_start_params;

struct me_io_irq_stop_params {
	int device;
	int subdevice;
	int channel;
	int flags;
};
typedef struct me_io_irq_stop_params me_io_irq_stop_params;

struct me_io_irq_wait_params {
	int device;
	int subdevice;
	int channel;
	int time_out;
	int flags;
};
typedef struct me_io_irq_wait_params me_io_irq_wait_params;

struct me_io_irq_wait_res {
	int error;
	int irq_count;
	int value;
};
typedef struct me_io_irq_wait_res me_io_irq_wait_res;

struct me_io_reset_device_params {
	int device;
	int flags;
};
typedef struct me_io_reset_device_params me_io_reset_device_params;

struct me_io_reset_subdevice_params {
	int device;
	int subdevice;
	int flags;
};
typedef struct me_io_reset_subdevice_params me_io_reset_subdevice_params;

struct me_io_single_config_params {
	int device;
	int subdevice;
	int channel;
	int single_config;
	int ref;
	int trig_chain;
	int trig_type;
	int trig_edge;
	int flags;
};
typedef struct me_io_single_config_params me_io_single_config_params;

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
typedef struct me_io_single_entry_params me_io_single_entry_params;

struct me_io_single_params {
	struct {
		u_int single_list_len;
		me_io_single_entry_params *single_list_val;
	} single_list;
	int flags;
};
typedef struct me_io_single_params me_io_single_params;

struct me_io_single_entry_res {
	int value;
	int error;
};
typedef struct me_io_single_entry_res me_io_single_entry_res;

struct me_io_single_res {
	int error;
	struct {
		u_int single_list_len;
		me_io_single_entry_res *single_list_val;
	} single_list;
};
typedef struct me_io_single_res me_io_single_res;

struct me_io_stream_config_entry_params {
	int channel;
	int stream_config;
	int ref;
	int flags;
};
typedef struct me_io_stream_config_entry_params me_io_stream_config_entry_params;

struct me_io_stream_trigger_params {
	int acq_start_trig_type;
	int acq_start_trig_edge;
	int acq_start_trig_chain;
	int acq_start_ticks_low;
	int acq_start_ticks_high;
	struct {
		u_int acq_start_args_len;
		int *acq_start_args_val;
	} acq_start_args;
	int scan_start_trig_type;
	int scan_start_ticks_low;
	int scan_start_ticks_high;
	struct {
		u_int scan_start_args_len;
		int *scan_start_args_val;
	} scan_start_args;
	int conv_start_trig_type;
	int conv_start_ticks_low;
	int conv_start_ticks_high;
	struct {
		u_int conv_start_args_len;
		int *conv_start_args_val;
	} conv_start_args;
	int scan_stop_trig_type;
	int scan_stop_count;
	struct {
		u_int scan_stop_args_len;
		int *scan_stop_args_val;
	} scan_stop_args;
	int acq_stop_trig_type;
	int acq_stop_count;
	struct {
		u_int acq_stop_args_len;
		int *acq_stop_args_val;
	} acq_stop_args;
	int flags;
};
typedef struct me_io_stream_trigger_params me_io_stream_trigger_params;

struct me_io_stream_config_params {
	int device;
	int subdevice;
	struct {
		u_int config_list_len;
		me_io_stream_config_entry_params *config_list_val;
	} config_list;
	me_io_stream_trigger_params trigger;
	int fifo_irq_threshold;
	int flags;
};
typedef struct me_io_stream_config_params me_io_stream_config_params;

struct me_io_stream_new_values_params {
	int device;
	int subdevice;
	int time_out;
	int flags;
};
typedef struct me_io_stream_new_values_params me_io_stream_new_values_params;

struct me_io_stream_new_values_res {
	int error;
	int count;
};
typedef struct me_io_stream_new_values_res me_io_stream_new_values_res;

struct me_io_stream_read_params {
	int device;
	int subdevice;
	int read_mode;
	int count;
	int flags;
};
typedef struct me_io_stream_read_params me_io_stream_read_params;

struct me_io_stream_read_res {
	int error;
	struct {
		u_int values_len;
		int *values_val;
	} values;
};
typedef struct me_io_stream_read_res me_io_stream_read_res;

struct me_io_stream_write_params {
	int device;
	int subdevice;
	int write_mode;
	struct {
		u_int values_len;
		int *values_val;
	} values;
	int flags;
};
typedef struct me_io_stream_write_params me_io_stream_write_params;

struct me_io_stream_write_res {
	int error;
	int count;
};
typedef struct me_io_stream_write_res me_io_stream_write_res;

struct me_io_stream_start_entry_params {
	int device;
	int subdevice;
	int start_mode;
	int time_out;
	int flags;
	int error;
};
typedef struct me_io_stream_start_entry_params me_io_stream_start_entry_params;

struct me_io_stream_start_params {
	struct {
		u_int start_list_len;
		me_io_stream_start_entry_params *start_list_val;
	} start_list;
	int flags;
};
typedef struct me_io_stream_start_params me_io_stream_start_params;

struct me_io_stream_start_res {
	int error;
	struct {
		u_int start_list_len;
		int *start_list_val;
	} start_list;
};
typedef struct me_io_stream_start_res me_io_stream_start_res;

struct me_io_stream_stop_entry_params {
	int device;
	int subdevice;
	int stop_mode;
	int flags;
	int error;
};
typedef struct me_io_stream_stop_entry_params me_io_stream_stop_entry_params;

struct me_io_stream_stop_params {
	struct {
		u_int stop_list_len;
		me_io_stream_stop_entry_params *stop_list_val;
	} stop_list;
	int flags;
};
typedef struct me_io_stream_stop_params me_io_stream_stop_params;

struct me_io_stream_stop_res {
	int error;
	struct {
		u_int stop_list_len;
		int *stop_list_val;
	} stop_list;
};
typedef struct me_io_stream_stop_res me_io_stream_stop_res;

struct me_io_stream_status_params {
	int device;
	int subdevice;
	int wait;
	int flags;
};
typedef struct me_io_stream_status_params me_io_stream_status_params;

struct me_io_stream_status_res {
	int error;
	int status;
	int count;
};
typedef struct me_io_stream_status_res me_io_stream_status_res;

struct me_io_stream_frequency_to_ticks_params {
	int device;
	int subdevice;
	int timer;
	double frequency;
	int flags;
};
typedef struct me_io_stream_frequency_to_ticks_params me_io_stream_frequency_to_ticks_params;

struct me_io_stream_frequency_to_ticks_res {
	int error;
	double frequency;
	int ticks_low;
	int ticks_high;
};
typedef struct me_io_stream_frequency_to_ticks_res me_io_stream_frequency_to_ticks_res;

struct me_io_stream_time_to_ticks_params {
	int device;
	int subdevice;
	int timer;
	double time;
	int flags;
};
typedef struct me_io_stream_time_to_ticks_params me_io_stream_time_to_ticks_params;

struct me_io_stream_time_to_ticks_res {
	int error;
	double time;
	int ticks_low;
	int ticks_high;
};
typedef struct me_io_stream_time_to_ticks_res me_io_stream_time_to_ticks_res;

struct me_query_description_device_res {
	int error;
	char *description;
};
typedef struct me_query_description_device_res me_query_description_device_res;

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
typedef struct me_query_info_device_res me_query_info_device_res;

struct me_query_name_device_res {
	int error;
	char *name;
};
typedef struct me_query_name_device_res me_query_name_device_res;

struct me_query_name_device_driver_res {
	int error;
	char *name;
};
typedef struct me_query_name_device_driver_res me_query_name_device_driver_res;

struct me_query_number_devices_res {
	int error;
	int number;
};
typedef struct me_query_number_devices_res me_query_number_devices_res;

struct me_query_number_subdevices_res {
	int error;
	int number;
};
typedef struct me_query_number_subdevices_res me_query_number_subdevices_res;

struct me_query_number_channels_params {
	int device;
	int subdevice;
};
typedef struct me_query_number_channels_params me_query_number_channels_params;

struct me_query_number_channels_res {
	int error;
	int number;
};
typedef struct me_query_number_channels_res me_query_number_channels_res;

struct me_query_number_ranges_params {
	int device;
	int subdevice;
	int unit;
};
typedef struct me_query_number_ranges_params me_query_number_ranges_params;

struct me_query_number_ranges_res {
	int error;
	int number;
};
typedef struct me_query_number_ranges_res me_query_number_ranges_res;

struct me_query_range_by_min_max_params {
	int device;
	int subdevice;
	int unit;
	double min;
	double max;
};
typedef struct me_query_range_by_min_max_params me_query_range_by_min_max_params;

struct me_query_range_by_min_max_res {
	int error;
	double min;
	double max;
	int max_data;
	int range;
};
typedef struct me_query_range_by_min_max_res me_query_range_by_min_max_res;

struct me_query_range_info_params {
	int device;
	int subdevice;
	int range;
};
typedef struct me_query_range_info_params me_query_range_info_params;

struct me_query_range_info_res {
	int error;
	int unit;
	double min;
	double max;
	int max_data;
};
typedef struct me_query_range_info_res me_query_range_info_res;

struct me_query_subdevice_by_type_params {
	int device;
	int start_subdevice;
	int type;
	int subtype;
};
typedef struct me_query_subdevice_by_type_params me_query_subdevice_by_type_params;

struct me_query_subdevice_by_type_res {
	int error;
	int subdevice;
};
typedef struct me_query_subdevice_by_type_res me_query_subdevice_by_type_res;

struct me_query_subdevice_type_params {
	int device;
	int subdevice;
};
typedef struct me_query_subdevice_type_params me_query_subdevice_type_params;

struct me_query_subdevice_type_res {
	int error;
	int type;
	int subtype;
};
typedef struct me_query_subdevice_type_res me_query_subdevice_type_res;

struct me_query_subdevice_caps_params {
	int device;
	int subdevice;
};
typedef struct me_query_subdevice_caps_params me_query_subdevice_caps_params;

struct me_query_subdevice_caps_res {
	int error;
	int caps;
};
typedef struct me_query_subdevice_caps_res me_query_subdevice_caps_res;

struct me_query_subdevice_caps_args_params {
	int device;
	int subdevice;
	int cap;
	int count;
};
typedef struct me_query_subdevice_caps_args_params me_query_subdevice_caps_args_params;

struct me_query_subdevice_caps_args_res {
	int error;
	struct {
		u_int args_len;
		int *args_val;
	} args;
};
typedef struct me_query_subdevice_caps_args_res me_query_subdevice_caps_args_res;

struct me_query_version_library_res {
	int error;
	int ver;
};
typedef struct me_query_version_library_res me_query_version_library_res;

struct me_query_version_main_driver_res {
	int error;
	int ver;
};
typedef struct me_query_version_main_driver_res me_query_version_main_driver_res;

struct me_query_version_device_driver_res {
	int error;
	int ver;
};
typedef struct me_query_version_device_driver_res me_query_version_device_driver_res;

#define RMEDRIVER_PROG 0x20000001
#define RMEDRIVER_VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define ME_CLOSE_PROC 1
extern  int* me_close_proc_1(int*, CLIENT*);
extern  int* me_close_proc_1_svc(int*, struct svc_req*);
#define ME_OPEN_PROC 2
extern  int* me_open_proc_1(int*, CLIENT*);
extern  int* me_open_proc_1_svc(int*, struct svc_req*);
#define ME_LOCK_DRIVER_PROC 3
extern  int* me_lock_driver_proc_1(me_lock_driver_params*, CLIENT*);
extern  int* me_lock_driver_proc_1_svc(me_lock_driver_params*, struct svc_req*);
#define ME_LOCK_DEVICE_PROC 4
extern  int* me_lock_device_proc_1(me_lock_device_params*, CLIENT*);
extern  int* me_lock_device_proc_1_svc(me_lock_device_params*, struct svc_req*);
#define ME_LOCK_SUBDEVICE_PROC 5
extern  int* me_lock_subdevice_proc_1(me_lock_subdevice_params*, CLIENT*);
extern  int* me_lock_subdevice_proc_1_svc(me_lock_subdevice_params*, struct svc_req*);
#define ME_IO_IRQ_START_PROC 6
extern  int* me_io_irq_start_proc_1(me_io_irq_start_params*, CLIENT*);
extern  int* me_io_irq_start_proc_1_svc(me_io_irq_start_params*, struct svc_req*);
#define ME_IO_IRQ_STOP_PROC 7
extern  int* me_io_irq_stop_proc_1(me_io_irq_stop_params*, CLIENT*);
extern  int* me_io_irq_stop_proc_1_svc(me_io_irq_stop_params*, struct svc_req*);
#define ME_IO_IRQ_WAIT_PROC 8
extern  me_io_irq_wait_res* me_io_irq_wait_proc_1(me_io_irq_wait_params*, CLIENT*);
extern  me_io_irq_wait_res* me_io_irq_wait_proc_1_svc(me_io_irq_wait_params*, struct svc_req*);
#define ME_IO_RESET_DEVICE_PROC 9
extern  int* me_io_reset_device_proc_1(me_io_reset_device_params*, CLIENT*);
extern  int* me_io_reset_device_proc_1_svc(me_io_reset_device_params*, struct svc_req*);
#define ME_IO_RESET_SUBDEVICE_PROC 10
extern  int* me_io_reset_subdevice_proc_1(me_io_reset_subdevice_params*, CLIENT*);
extern  int* me_io_reset_subdevice_proc_1_svc(me_io_reset_subdevice_params*, struct svc_req*);
#define ME_IO_SINGLE_CONFIG_PROC 11
extern  int* me_io_single_config_proc_1(me_io_single_config_params*, CLIENT*);
extern  int* me_io_single_config_proc_1_svc(me_io_single_config_params*, struct svc_req*);
#define ME_IO_SINGLE_PROC 12
extern  me_io_single_res* me_io_single_proc_1(me_io_single_params*, CLIENT*);
extern  me_io_single_res* me_io_single_proc_1_svc(me_io_single_params*, struct svc_req*);
#define ME_IO_STREAM_CONFIG_PROC 13
extern  int* me_io_stream_config_proc_1(me_io_stream_config_params*, CLIENT*);
extern  int* me_io_stream_config_proc_1_svc(me_io_stream_config_params*, struct svc_req*);
#define ME_IO_STREAM_READ_PROC 14
extern  me_io_stream_read_res* me_io_stream_read_proc_1(me_io_stream_read_params*, CLIENT*);
extern  me_io_stream_read_res* me_io_stream_read_proc_1_svc(me_io_stream_read_params*, struct svc_req*);
#define ME_IO_STREAM_WRITE_PROC 15
extern  me_io_stream_write_res* me_io_stream_write_proc_1(me_io_stream_write_params*, CLIENT*);
extern  me_io_stream_write_res* me_io_stream_write_proc_1_svc(me_io_stream_write_params*, struct svc_req*);
#define ME_IO_STREAM_START_PROC 16
extern  me_io_stream_start_res* me_io_stream_start_proc_1(me_io_stream_start_params*, CLIENT*);
extern  me_io_stream_start_res* me_io_stream_start_proc_1_svc(me_io_stream_start_params*, struct svc_req*);
#define ME_IO_STREAM_STOP_PROC 17
extern  me_io_stream_stop_res* me_io_stream_stop_proc_1(me_io_stream_stop_params*, CLIENT*);
extern  me_io_stream_stop_res* me_io_stream_stop_proc_1_svc(me_io_stream_stop_params*, struct svc_req*);
#define ME_IO_STREAM_STATUS_PROC 18
extern  me_io_stream_status_res* me_io_stream_status_proc_1(me_io_stream_status_params*, CLIENT*);
extern  me_io_stream_status_res* me_io_stream_status_proc_1_svc(me_io_stream_status_params*, struct svc_req*);
#define ME_IO_STREAM_FREQUENCY_TO_TICKS_PROC 19
extern  me_io_stream_frequency_to_ticks_res* me_io_stream_frequency_to_ticks_proc_1(me_io_stream_frequency_to_ticks_params*, CLIENT*);
extern  me_io_stream_frequency_to_ticks_res* me_io_stream_frequency_to_ticks_proc_1_svc(me_io_stream_frequency_to_ticks_params*, struct svc_req*);
#define ME_IO_STREAM_TIME_TO_TICKS_PROC 20
extern  me_io_stream_time_to_ticks_res* me_io_stream_time_to_ticks_proc_1(me_io_stream_time_to_ticks_params*, CLIENT*);
extern  me_io_stream_time_to_ticks_res* me_io_stream_time_to_ticks_proc_1_svc(me_io_stream_time_to_ticks_params*, struct svc_req*);
#define ME_IO_STREAM_NEW_VALUES_PROC 21
extern  me_io_stream_new_values_res* me_io_stream_new_values_proc_1(me_io_stream_new_values_params*, CLIENT*);
extern  me_io_stream_new_values_res* me_io_stream_new_values_proc_1_svc(me_io_stream_new_values_params*, struct svc_req*);
#define ME_QUERY_DESCRIPTION_DEVICE_PROC 22
extern  me_query_description_device_res* me_query_description_device_proc_1(int*, CLIENT*);
extern  me_query_description_device_res* me_query_description_device_proc_1_svc(int*, struct svc_req*);
#define ME_QUERY_INFO_DEVICE_PROC 23
extern  me_query_info_device_res* me_query_info_device_proc_1(int*, CLIENT*);
extern  me_query_info_device_res* me_query_info_device_proc_1_svc(int*, struct svc_req*);
#define ME_QUERY_NAME_DEVICE_PROC 24
extern  me_query_name_device_res* me_query_name_device_proc_1(int*, CLIENT*);
extern  me_query_name_device_res* me_query_name_device_proc_1_svc(int*, struct svc_req*);
#define ME_QUERY_NAME_DEVICE_DRIVER_PROC 25
extern  me_query_name_device_driver_res* me_query_name_device_driver_proc_1(int*, CLIENT*);
extern  me_query_name_device_driver_res* me_query_name_device_driver_proc_1_svc(int*, struct svc_req*);
#define ME_QUERY_NUMBER_DEVICES_PROC 26
extern  me_query_number_devices_res* me_query_number_devices_proc_1(void*, CLIENT*);
extern  me_query_number_devices_res* me_query_number_devices_proc_1_svc(void*, struct svc_req*);
#define ME_QUERY_NUMBER_SUBDEVICES_PROC 27
extern  me_query_number_subdevices_res* me_query_number_subdevices_proc_1(int*, CLIENT*);
extern  me_query_number_subdevices_res* me_query_number_subdevices_proc_1_svc(int*, struct svc_req*);
#define ME_QUERY_NUMBER_CHANNELS_PROC 28
extern  me_query_number_channels_res* me_query_number_channels_proc_1(me_query_number_channels_params*, CLIENT*);
extern  me_query_number_channels_res* me_query_number_channels_proc_1_svc(me_query_number_channels_params*, struct svc_req*);
#define ME_QUERY_NUMBER_RANGES_PROC 29
extern  me_query_number_ranges_res* me_query_number_ranges_proc_1(me_query_number_ranges_params*, CLIENT*);
extern  me_query_number_ranges_res* me_query_number_ranges_proc_1_svc(me_query_number_ranges_params*, struct svc_req*);
#define ME_QUERY_RANGE_BY_MIN_MAX_PROC 30
extern  me_query_range_by_min_max_res* me_query_range_by_min_max_proc_1(me_query_range_by_min_max_params*, CLIENT*);
extern  me_query_range_by_min_max_res* me_query_range_by_min_max_proc_1_svc(me_query_range_by_min_max_params*, struct svc_req*);
#define ME_QUERY_RANGE_INFO_PROC 31
extern  me_query_range_info_res* me_query_range_info_proc_1(me_query_range_info_params*, CLIENT*);
extern  me_query_range_info_res* me_query_range_info_proc_1_svc(me_query_range_info_params*, struct svc_req*);
#define ME_QUERY_SUBDEVICE_BY_TYPE_PROC 32
extern  me_query_subdevice_by_type_res* me_query_subdevice_by_type_proc_1(me_query_subdevice_by_type_params*, CLIENT*);
extern  me_query_subdevice_by_type_res* me_query_subdevice_by_type_proc_1_svc(me_query_subdevice_by_type_params*, struct svc_req*);
#define ME_QUERY_SUBDEVICE_TYPE_PROC 33
extern  me_query_subdevice_type_res* me_query_subdevice_type_proc_1(me_query_subdevice_type_params*, CLIENT*);
extern  me_query_subdevice_type_res* me_query_subdevice_type_proc_1_svc(me_query_subdevice_type_params*, struct svc_req*);
#define ME_QUERY_SUBDEVICE_CAPS_PROC 34
extern  me_query_subdevice_caps_res* me_query_subdevice_caps_proc_1(me_query_subdevice_caps_params*, CLIENT*);
extern  me_query_subdevice_caps_res* me_query_subdevice_caps_proc_1_svc(me_query_subdevice_caps_params*, struct svc_req*);
#define ME_QUERY_SUBDEVICE_CAPS_ARGS_PROC 35
extern  me_query_subdevice_caps_args_res* me_query_subdevice_caps_args_proc_1(me_query_subdevice_caps_args_params*, CLIENT*);
extern  me_query_subdevice_caps_args_res* me_query_subdevice_caps_args_proc_1_svc(me_query_subdevice_caps_args_params*, struct svc_req*);
#define ME_QUERY_VERSION_LIBRARY_PROC 36
extern  me_query_version_library_res* me_query_version_library_proc_1(void*, CLIENT*);
extern  me_query_version_library_res* me_query_version_library_proc_1_svc(void*, struct svc_req*);
#define ME_QUERY_VERSION_MAIN_DRIVER_PROC 37
extern  me_query_version_main_driver_res* me_query_version_main_driver_proc_1(void*, CLIENT*);
extern  me_query_version_main_driver_res* me_query_version_main_driver_proc_1_svc(void*, struct svc_req*);
#define ME_QUERY_VERSION_DEVICE_DRIVER_PROC 38
extern  me_query_version_device_driver_res* me_query_version_device_driver_proc_1(int*, CLIENT*);
extern  me_query_version_device_driver_res* me_query_version_device_driver_proc_1_svc(int*, struct svc_req*);
extern int rmedriver_prog_1_freeresult (SVCXPRT*, xdrproc_t, caddr_t);

#else /* K&R C */
#define ME_CLOSE_PROC 1
extern  int* me_close_proc_1();
extern  int* me_close_proc_1_svc();
#define ME_OPEN_PROC 2
extern  int* me_open_proc_1();
extern  int* me_open_proc_1_svc();
#define ME_LOCK_DRIVER_PROC 3
extern  int* me_lock_driver_proc_1();
extern  int* me_lock_driver_proc_1_svc();
#define ME_LOCK_DEVICE_PROC 4
extern  int* me_lock_device_proc_1();
extern  int* me_lock_device_proc_1_svc();
#define ME_LOCK_SUBDEVICE_PROC 5
extern  int* me_lock_subdevice_proc_1();
extern  int* me_lock_subdevice_proc_1_svc();
#define ME_IO_IRQ_START_PROC 6
extern  int* me_io_irq_start_proc_1();
extern  int* me_io_irq_start_proc_1_svc();
#define ME_IO_IRQ_STOP_PROC 7
extern  int* me_io_irq_stop_proc_1();
extern  int* me_io_irq_stop_proc_1_svc();
#define ME_IO_IRQ_WAIT_PROC 8
extern  me_io_irq_wait_res* me_io_irq_wait_proc_1();
extern  me_io_irq_wait_res* me_io_irq_wait_proc_1_svc();
#define ME_IO_RESET_DEVICE_PROC 9
extern  int* me_io_reset_device_proc_1();
extern  int* me_io_reset_device_proc_1_svc();
#define ME_IO_RESET_SUBDEVICE_PROC 10
extern  int* me_io_reset_subdevice_proc_1();
extern  int* me_io_reset_subdevice_proc_1_svc();
#define ME_IO_SINGLE_CONFIG_PROC 11
extern  int* me_io_single_config_proc_1();
extern  int* me_io_single_config_proc_1_svc();
#define ME_IO_SINGLE_PROC 12
extern  me_io_single_res* me_io_single_proc_1();
extern  me_io_single_res* me_io_single_proc_1_svc();
#define ME_IO_STREAM_CONFIG_PROC 13
extern  int* me_io_stream_config_proc_1();
extern  int* me_io_stream_config_proc_1_svc();
#define ME_IO_STREAM_READ_PROC 14
extern  me_io_stream_read_res* me_io_stream_read_proc_1();
extern  me_io_stream_read_res* me_io_stream_read_proc_1_svc();
#define ME_IO_STREAM_WRITE_PROC 15
extern  me_io_stream_write_res* me_io_stream_write_proc_1();
extern  me_io_stream_write_res* me_io_stream_write_proc_1_svc();
#define ME_IO_STREAM_START_PROC 16
extern  me_io_stream_start_res* me_io_stream_start_proc_1();
extern  me_io_stream_start_res* me_io_stream_start_proc_1_svc();
#define ME_IO_STREAM_STOP_PROC 17
extern  me_io_stream_stop_res* me_io_stream_stop_proc_1();
extern  me_io_stream_stop_res* me_io_stream_stop_proc_1_svc();
#define ME_IO_STREAM_STATUS_PROC 18
extern  me_io_stream_status_res* me_io_stream_status_proc_1();
extern  me_io_stream_status_res* me_io_stream_status_proc_1_svc();
#define ME_IO_STREAM_FREQUENCY_TO_TICKS_PROC 19
extern  me_io_stream_frequency_to_ticks_res* me_io_stream_frequency_to_ticks_proc_1();
extern  me_io_stream_frequency_to_ticks_res* me_io_stream_frequency_to_ticks_proc_1_svc();
#define ME_IO_STREAM_TIME_TO_TICKS_PROC 20
extern  me_io_stream_time_to_ticks_res* me_io_stream_time_to_ticks_proc_1();
extern  me_io_stream_time_to_ticks_res* me_io_stream_time_to_ticks_proc_1_svc();
#define ME_IO_STREAM_NEW_VALUES_PROC 21
extern  me_io_stream_new_values_res* me_io_stream_new_values_proc_1();
extern  me_io_stream_new_values_res* me_io_stream_new_values_proc_1_svc();
#define ME_QUERY_DESCRIPTION_DEVICE_PROC 22
extern  me_query_description_device_res* me_query_description_device_proc_1();
extern  me_query_description_device_res* me_query_description_device_proc_1_svc();
#define ME_QUERY_INFO_DEVICE_PROC 23
extern  me_query_info_device_res* me_query_info_device_proc_1();
extern  me_query_info_device_res* me_query_info_device_proc_1_svc();
#define ME_QUERY_NAME_DEVICE_PROC 24
extern  me_query_name_device_res* me_query_name_device_proc_1();
extern  me_query_name_device_res* me_query_name_device_proc_1_svc();
#define ME_QUERY_NAME_DEVICE_DRIVER_PROC 25
extern  me_query_name_device_driver_res* me_query_name_device_driver_proc_1();
extern  me_query_name_device_driver_res* me_query_name_device_driver_proc_1_svc();
#define ME_QUERY_NUMBER_DEVICES_PROC 26
extern  me_query_number_devices_res* me_query_number_devices_proc_1();
extern  me_query_number_devices_res* me_query_number_devices_proc_1_svc();
#define ME_QUERY_NUMBER_SUBDEVICES_PROC 27
extern  me_query_number_subdevices_res* me_query_number_subdevices_proc_1();
extern  me_query_number_subdevices_res* me_query_number_subdevices_proc_1_svc();
#define ME_QUERY_NUMBER_CHANNELS_PROC 28
extern  me_query_number_channels_res* me_query_number_channels_proc_1();
extern  me_query_number_channels_res* me_query_number_channels_proc_1_svc();
#define ME_QUERY_NUMBER_RANGES_PROC 29
extern  me_query_number_ranges_res* me_query_number_ranges_proc_1();
extern  me_query_number_ranges_res* me_query_number_ranges_proc_1_svc();
#define ME_QUERY_RANGE_BY_MIN_MAX_PROC 30
extern  me_query_range_by_min_max_res* me_query_range_by_min_max_proc_1();
extern  me_query_range_by_min_max_res* me_query_range_by_min_max_proc_1_svc();
#define ME_QUERY_RANGE_INFO_PROC 31
extern  me_query_range_info_res* me_query_range_info_proc_1();
extern  me_query_range_info_res* me_query_range_info_proc_1_svc();
#define ME_QUERY_SUBDEVICE_BY_TYPE_PROC 32
extern  me_query_subdevice_by_type_res* me_query_subdevice_by_type_proc_1();
extern  me_query_subdevice_by_type_res* me_query_subdevice_by_type_proc_1_svc();
#define ME_QUERY_SUBDEVICE_TYPE_PROC 33
extern  me_query_subdevice_type_res* me_query_subdevice_type_proc_1();
extern  me_query_subdevice_type_res* me_query_subdevice_type_proc_1_svc();
#define ME_QUERY_SUBDEVICE_CAPS_PROC 34
extern  me_query_subdevice_caps_res* me_query_subdevice_caps_proc_1();
extern  me_query_subdevice_caps_res* me_query_subdevice_caps_proc_1_svc();
#define ME_QUERY_SUBDEVICE_CAPS_ARGS_PROC 35
extern  me_query_subdevice_caps_args_res* me_query_subdevice_caps_args_proc_1();
extern  me_query_subdevice_caps_args_res* me_query_subdevice_caps_args_proc_1_svc();
#define ME_QUERY_VERSION_LIBRARY_PROC 36
extern  me_query_version_library_res* me_query_version_library_proc_1();
extern  me_query_version_library_res* me_query_version_library_proc_1_svc();
#define ME_QUERY_VERSION_MAIN_DRIVER_PROC 37
extern  me_query_version_main_driver_res* me_query_version_main_driver_proc_1();
extern  me_query_version_main_driver_res* me_query_version_main_driver_proc_1_svc();
#define ME_QUERY_VERSION_DEVICE_DRIVER_PROC 38
extern  me_query_version_device_driver_res* me_query_version_device_driver_proc_1();
extern  me_query_version_device_driver_res* me_query_version_device_driver_proc_1_svc();
extern int rmedriver_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_me_lock_driver_params (XDR*, me_lock_driver_params*);
extern  bool_t xdr_me_lock_device_params (XDR*, me_lock_device_params*);
extern  bool_t xdr_me_lock_subdevice_params (XDR*, me_lock_subdevice_params*);
extern  bool_t xdr_me_io_irq_start_params (XDR*, me_io_irq_start_params*);
extern  bool_t xdr_me_io_irq_stop_params (XDR*, me_io_irq_stop_params*);
extern  bool_t xdr_me_io_irq_wait_params (XDR*, me_io_irq_wait_params*);
extern  bool_t xdr_me_io_irq_wait_res (XDR*, me_io_irq_wait_res*);
extern  bool_t xdr_me_io_reset_device_params (XDR*, me_io_reset_device_params*);
extern  bool_t xdr_me_io_reset_subdevice_params (XDR*, me_io_reset_subdevice_params*);
extern  bool_t xdr_me_io_single_config_params (XDR*, me_io_single_config_params*);
extern  bool_t xdr_me_io_single_entry_params (XDR*, me_io_single_entry_params*);
extern  bool_t xdr_me_io_single_params (XDR*, me_io_single_params*);
extern  bool_t xdr_me_io_single_entry_res (XDR*, me_io_single_entry_res*);
extern  bool_t xdr_me_io_single_res (XDR*, me_io_single_res*);
extern  bool_t xdr_me_io_stream_config_entry_params (XDR*, me_io_stream_config_entry_params*);
extern  bool_t xdr_me_io_stream_trigger_params (XDR*, me_io_stream_trigger_params*);
extern  bool_t xdr_me_io_stream_config_params (XDR*, me_io_stream_config_params*);
extern  bool_t xdr_me_io_stream_new_values_params (XDR*, me_io_stream_new_values_params*);
extern  bool_t xdr_me_io_stream_new_values_res (XDR*, me_io_stream_new_values_res*);
extern  bool_t xdr_me_io_stream_read_params (XDR*, me_io_stream_read_params*);
extern  bool_t xdr_me_io_stream_read_res (XDR*, me_io_stream_read_res*);
extern  bool_t xdr_me_io_stream_write_params (XDR*, me_io_stream_write_params*);
extern  bool_t xdr_me_io_stream_write_res (XDR*, me_io_stream_write_res*);
extern  bool_t xdr_me_io_stream_start_entry_params (XDR*, me_io_stream_start_entry_params*);
extern  bool_t xdr_me_io_stream_start_params (XDR*, me_io_stream_start_params*);
extern  bool_t xdr_me_io_stream_start_res (XDR*, me_io_stream_start_res*);
extern  bool_t xdr_me_io_stream_stop_entry_params (XDR*, me_io_stream_stop_entry_params*);
extern  bool_t xdr_me_io_stream_stop_params (XDR*, me_io_stream_stop_params*);
extern  bool_t xdr_me_io_stream_stop_res (XDR*, me_io_stream_stop_res*);
extern  bool_t xdr_me_io_stream_status_params (XDR*, me_io_stream_status_params*);
extern  bool_t xdr_me_io_stream_status_res (XDR*, me_io_stream_status_res*);
extern  bool_t xdr_me_io_stream_frequency_to_ticks_params (XDR*, me_io_stream_frequency_to_ticks_params*);
extern  bool_t xdr_me_io_stream_frequency_to_ticks_res (XDR*, me_io_stream_frequency_to_ticks_res*);
extern  bool_t xdr_me_io_stream_time_to_ticks_params (XDR*, me_io_stream_time_to_ticks_params*);
extern  bool_t xdr_me_io_stream_time_to_ticks_res (XDR*, me_io_stream_time_to_ticks_res*);
extern  bool_t xdr_me_query_description_device_res (XDR*, me_query_description_device_res*);
extern  bool_t xdr_me_query_info_device_res (XDR*, me_query_info_device_res*);
extern  bool_t xdr_me_query_name_device_res (XDR*, me_query_name_device_res*);
extern  bool_t xdr_me_query_name_device_driver_res (XDR*, me_query_name_device_driver_res*);
extern  bool_t xdr_me_query_number_devices_res (XDR*, me_query_number_devices_res*);
extern  bool_t xdr_me_query_number_subdevices_res (XDR*, me_query_number_subdevices_res*);
extern  bool_t xdr_me_query_number_channels_params (XDR*, me_query_number_channels_params*);
extern  bool_t xdr_me_query_number_channels_res (XDR*, me_query_number_channels_res*);
extern  bool_t xdr_me_query_number_ranges_params (XDR*, me_query_number_ranges_params*);
extern  bool_t xdr_me_query_number_ranges_res (XDR*, me_query_number_ranges_res*);
extern  bool_t xdr_me_query_range_by_min_max_params (XDR*, me_query_range_by_min_max_params*);
extern  bool_t xdr_me_query_range_by_min_max_res (XDR*, me_query_range_by_min_max_res*);
extern  bool_t xdr_me_query_range_info_params (XDR*, me_query_range_info_params*);
extern  bool_t xdr_me_query_range_info_res (XDR*, me_query_range_info_res*);
extern  bool_t xdr_me_query_subdevice_by_type_params (XDR*, me_query_subdevice_by_type_params*);
extern  bool_t xdr_me_query_subdevice_by_type_res (XDR*, me_query_subdevice_by_type_res*);
extern  bool_t xdr_me_query_subdevice_type_params (XDR*, me_query_subdevice_type_params*);
extern  bool_t xdr_me_query_subdevice_type_res (XDR*, me_query_subdevice_type_res*);
extern  bool_t xdr_me_query_subdevice_caps_params (XDR*, me_query_subdevice_caps_params*);
extern  bool_t xdr_me_query_subdevice_caps_res (XDR*, me_query_subdevice_caps_res*);
extern  bool_t xdr_me_query_subdevice_caps_args_params (XDR*, me_query_subdevice_caps_args_params*);
extern  bool_t xdr_me_query_subdevice_caps_args_res (XDR*, me_query_subdevice_caps_args_res*);
extern  bool_t xdr_me_query_version_library_res (XDR*, me_query_version_library_res*);
extern  bool_t xdr_me_query_version_main_driver_res (XDR*, me_query_version_main_driver_res*);
extern  bool_t xdr_me_query_version_device_driver_res (XDR*, me_query_version_device_driver_res*);

#else /* K&R C */
extern bool_t xdr_me_lock_driver_params ();
extern bool_t xdr_me_lock_device_params ();
extern bool_t xdr_me_lock_subdevice_params ();
extern bool_t xdr_me_io_irq_start_params ();
extern bool_t xdr_me_io_irq_stop_params ();
extern bool_t xdr_me_io_irq_wait_params ();
extern bool_t xdr_me_io_irq_wait_res ();
extern bool_t xdr_me_io_reset_device_params ();
extern bool_t xdr_me_io_reset_subdevice_params ();
extern bool_t xdr_me_io_single_config_params ();
extern bool_t xdr_me_io_single_entry_params ();
extern bool_t xdr_me_io_single_params ();
extern bool_t xdr_me_io_single_entry_res ();
extern bool_t xdr_me_io_single_res ();
extern bool_t xdr_me_io_stream_config_entry_params ();
extern bool_t xdr_me_io_stream_trigger_params ();
extern bool_t xdr_me_io_stream_config_params ();
extern bool_t xdr_me_io_stream_new_values_params ();
extern bool_t xdr_me_io_stream_new_values_res ();
extern bool_t xdr_me_io_stream_read_params ();
extern bool_t xdr_me_io_stream_read_res ();
extern bool_t xdr_me_io_stream_write_params ();
extern bool_t xdr_me_io_stream_write_res ();
extern bool_t xdr_me_io_stream_start_entry_params ();
extern bool_t xdr_me_io_stream_start_params ();
extern bool_t xdr_me_io_stream_start_res ();
extern bool_t xdr_me_io_stream_stop_entry_params ();
extern bool_t xdr_me_io_stream_stop_params ();
extern bool_t xdr_me_io_stream_stop_res ();
extern bool_t xdr_me_io_stream_status_params ();
extern bool_t xdr_me_io_stream_status_res ();
extern bool_t xdr_me_io_stream_frequency_to_ticks_params ();
extern bool_t xdr_me_io_stream_frequency_to_ticks_res ();
extern bool_t xdr_me_io_stream_time_to_ticks_params ();
extern bool_t xdr_me_io_stream_time_to_ticks_res ();
extern bool_t xdr_me_query_description_device_res ();
extern bool_t xdr_me_query_info_device_res ();
extern bool_t xdr_me_query_name_device_res ();
extern bool_t xdr_me_query_name_device_driver_res ();
extern bool_t xdr_me_query_number_devices_res ();
extern bool_t xdr_me_query_number_subdevices_res ();
extern bool_t xdr_me_query_number_channels_params ();
extern bool_t xdr_me_query_number_channels_res ();
extern bool_t xdr_me_query_number_ranges_params ();
extern bool_t xdr_me_query_number_ranges_res ();
extern bool_t xdr_me_query_range_by_min_max_params ();
extern bool_t xdr_me_query_range_by_min_max_res ();
extern bool_t xdr_me_query_range_info_params ();
extern bool_t xdr_me_query_range_info_res ();
extern bool_t xdr_me_query_subdevice_by_type_params ();
extern bool_t xdr_me_query_subdevice_by_type_res ();
extern bool_t xdr_me_query_subdevice_type_params ();
extern bool_t xdr_me_query_subdevice_type_res ();
extern bool_t xdr_me_query_subdevice_caps_params ();
extern bool_t xdr_me_query_subdevice_caps_res ();
extern bool_t xdr_me_query_subdevice_caps_args_params ();
extern bool_t xdr_me_query_subdevice_caps_args_res ();
extern bool_t xdr_me_query_version_library_res ();
extern bool_t xdr_me_query_version_main_driver_res ();
extern bool_t xdr_me_query_version_device_driver_res ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_RMEDRIVER_H_RPCGEN */
