#include <stdlib.h>

#include "rmedriver.h"
#include "medriver.h"

#include "meids_debug.h"

int * me_open_proc_1_svc(int *flags, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meOpen(*flags);
	}

	return err;
}


int * me_close_proc_1_svc(int *flags, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meClose(*flags);
	}

	return err;
}


int * me_lock_driver_proc_1_svc(me_lock_driver_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meLockDriver(params->lock , params->flags);
	}

	return err;
}


int * me_lock_device_proc_1_svc(me_lock_device_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meLockDevice(params->device, params->lock , params->flags);
	}

	return err;
}


int * me_lock_subdevice_proc_1_svc(me_lock_subdevice_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meLockSubdevice(params->device, params->subdevice, params->lock , params->flags);
	}

	return err;
}


int * me_io_irq_stop_proc_1_svc(me_io_irq_stop_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meIOIrqStop(
				params->device,
				params->subdevice,
				params->channel,
				params->flags);
	}

	return err;
}


int * me_io_irq_start_proc_1_svc(me_io_irq_start_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meIOIrqStart(
				params->device,
				params->subdevice,
				params->channel,
				params->irq_source,
				params->irq_edge,
				params->irq_arg,
				params->flags);
	}

	return err;
}


me_io_irq_wait_res * me_io_irq_wait_proc_1_svc(me_io_irq_wait_params *params, struct svc_req *dummy)
{
	me_io_irq_wait_res* result = malloc(sizeof(me_io_irq_wait_res));

	if (result)
	{
		result->error = meIOIrqWait(
							params->device,
							params->subdevice,
							params->channel,
							&result->irq_count,
							&result->value,
							params->time_out,
							params->flags);
	}

	return result;
}


int * me_io_reset_device_proc_1_svc(me_io_reset_device_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meIOResetDevice(params->device, params->flags);
	}

	return err;
}


int * me_io_reset_subdevice_proc_1_svc(me_io_reset_subdevice_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meIOResetSubdevice(params->device, params->subdevice, params->flags);
	}

	return err;
}


int * me_io_single_config_proc_1_svc(me_io_single_config_params *params, struct svc_req *dummy)
{
	int* err = malloc(sizeof(int));

	if (err)
	{
		*err = meIOSingleConfig(
				params->device,
				params->subdevice,
				params->channel,
				params->single_config,
				params->ref,
				params->trig_chain,
				params->trig_type,
				params->trig_edge,
				params->flags);
	}

	return err;
}


me_io_single_res * me_io_single_proc_1_svc(me_io_single_params *params, struct svc_req *dummy)
{
	me_io_single_res* result;
	me_io_single_entry_params* args;
	int i;

	if ((params->single_list.single_list_len <= 0) || (params->single_list.single_list_val == NULL))
	{
		LIBPCRITICALERROR("Empty argument in meIOSingle()\n");
		return NULL;
	}

	args = malloc(sizeof(me_io_single_entry_params) * params->single_list.single_list_len);
	if (!args)
	{
		LIBPCRITICALERROR("Can get memmory for meIOSingle()\n");
		return NULL;
	}

	result = malloc(sizeof(me_io_single_res));
	if (result)
	{
		memcpy(args, params->single_list.single_list_val, sizeof(me_io_single_entry_params) * params->single_list.single_list_len);

		result->error = meIOSingle(
						(meIOSingle_t *) args,
						params->single_list.single_list_len,
						params->flags);

		result->single_list.single_list_val = malloc(sizeof(me_io_single_entry_res) * params->single_list.single_list_len);
		if (result->single_list.single_list_val)
		{
			result->single_list.single_list_len = params->single_list.single_list_len;
			for (i = 0; i < params->single_list.single_list_len; i++)
			{
				result->single_list.single_list_val[i].value = args[i].value;
				result->single_list.single_list_val[i].error = args[i].error;
			}

		}
		else
		{
			LIBPCRITICALERROR("Can get memmory for meIOSingle() arguments\n");
			result->single_list.single_list_len = 0;
			result->error = ME_ERRNO_INTERNAL;
		}
	}

	free(args);

	return result;
}


int * me_io_stream_config_proc_1_svc(me_io_stream_config_params *params, struct svc_req *dummy)
{
	meIOStreamTrigger_t trigger;
	int* err = malloc(sizeof(int));

	if (err)
	{
		trigger.iAcqStartTrigType = params->trigger.acq_start_trig_type;
		trigger.iAcqStartTrigEdge = params->trigger.acq_start_trig_edge;
		trigger.iAcqStartTrigChan = params->trigger.acq_start_trig_chain;
		trigger.iAcqStartTicksLow = params->trigger.acq_start_ticks_low;
		trigger.iAcqStartTicksHigh = params->trigger.acq_start_ticks_high;
		memcpy(trigger.iAcqStartArgs, params->trigger.acq_start_args.acq_start_args_val, sizeof(trigger.iAcqStartArgs));
		trigger.iScanStartTrigType = params->trigger.scan_start_trig_type;
		trigger.iScanStartTicksLow = params->trigger.scan_start_ticks_low;
		trigger.iScanStartTicksHigh = params->trigger.scan_start_ticks_high;
		memcpy(trigger.iScanStartArgs, params->trigger.scan_start_args.scan_start_args_val, sizeof(trigger.iScanStartArgs));
		trigger.iConvStartTrigType = params->trigger.conv_start_trig_type;
		trigger.iConvStartTicksLow = params->trigger.conv_start_ticks_low;
		trigger.iConvStartTicksHigh = params->trigger.conv_start_ticks_high;
		memcpy(trigger.iConvStartArgs, params->trigger.conv_start_args.conv_start_args_val, sizeof(trigger.iConvStartArgs));
		trigger.iScanStopTrigType = params->trigger.scan_stop_trig_type;
		trigger.iScanStopCount = params->trigger.scan_stop_count;
		memcpy(trigger.iScanStopArgs, params->trigger.scan_stop_args.scan_stop_args_val, sizeof(trigger.iScanStopArgs));
		trigger.iAcqStopTrigType = params->trigger.acq_stop_trig_type;
		trigger.iAcqStopCount = params->trigger.acq_stop_count;
		memcpy(trigger.iAcqStopArgs, params->trigger.acq_stop_args.acq_stop_args_val, sizeof(trigger.iAcqStopArgs));
		trigger.iFlags = params->trigger.flags;

		*err = meIOStreamConfig(
					params->device,
					params->subdevice,
					(meIOStreamConfig_t *) params->config_list.config_list_val,
					params->config_list.config_list_len,
					&trigger,
					params->fifo_irq_threshold,
					params->flags);
	}

	return err;
}


me_io_stream_new_values_res * me_io_stream_new_values_proc_1_svc(me_io_stream_new_values_params *params, struct svc_req *dummy)
{
	me_io_stream_new_values_res* result = malloc(sizeof(me_io_stream_new_values_res));

	if (result)
	{
		result->error = meIOStreamNewValues(
							params->device,
							params->subdevice,
							params->time_out,
							&result->count,
							params->flags);
	}

	return result;
}


me_io_stream_read_res * me_io_stream_read_proc_1_svc(me_io_stream_read_params *params, struct svc_req *dummy)
{
	me_io_stream_read_res* result = malloc(sizeof(me_io_stream_read_res));
	int lenght;

	if (result)
	{
		lenght = params->count;
		result->values.values_val = malloc(sizeof(int) * lenght);
		if (result->values.values_val)
		{
			result->values.values_len = lenght;
			result->error = meIOStreamRead(
								params->device,
								params->subdevice,
								params->read_mode,
								result->values.values_val,
								&lenght,
								params->flags);
		}
		else
		{
			result->values.values_len = 0;
			result->error = ME_ERRNO_INTERNAL;
		}
	}

	return result;
}


me_io_stream_write_res * me_io_stream_write_proc_1_svc(me_io_stream_write_params *params, struct svc_req *dummy)
{
	me_io_stream_write_res* result = malloc(sizeof(me_io_stream_write_res));

	if (result)
	{
		result->count = params->values.values_len;
		result->error = meIOStreamWrite(
							params->device,
							params->subdevice,
							params->write_mode,
							params->values.values_val,
							&result->count,
							params->flags);
	}

	return result;
}


me_io_stream_start_res * me_io_stream_start_proc_1_svc(me_io_stream_start_params *params, struct svc_req *dummy)
{
	me_io_stream_start_res* result;
	me_io_stream_start_entry_params* args;
	int i;

	if ((params->start_list.start_list_len <= 0) || (params->start_list.start_list_val == NULL))
	{
		LIBPCRITICALERROR("Empty argument in meIOStreamStart()");
		return NULL;
	}

	args = malloc(sizeof(me_io_stream_start_entry_params) * params->start_list.start_list_len);
	if (!args)
	{
		return NULL;
	}

	result = malloc(sizeof(me_io_stream_start_res));
	if (result)
	{
		memcpy(args, params->start_list.start_list_val, sizeof(me_io_stream_start_entry_params) * params->start_list.start_list_len);

		result->error = meIOStreamStart(
							(meIOStreamStart_t *) args,
							params->start_list.start_list_len,
							params->flags);

		result->start_list.start_list_val = malloc(sizeof(int) * params->start_list.start_list_len);
		if (result->start_list.start_list_val)
		{
			result->start_list.start_list_len = params->start_list.start_list_len;

			for (i = 0; i < params->start_list.start_list_len; i++)
			{
				result->start_list.start_list_val[i] = args[i].error;
			}
		}
		else
		{
			result->start_list.start_list_len = 0;
			result->error = ME_ERRNO_INTERNAL;
		}
	}

	free(args);

	return result;
}


me_io_stream_stop_res * me_io_stream_stop_proc_1_svc(me_io_stream_stop_params *params, struct svc_req *dummy)
{
	me_io_stream_stop_res* result;
	me_io_stream_stop_entry_params* args;
	int i;

	if ((params->stop_list.stop_list_len <= 0) || (params->stop_list.stop_list_val == NULL))
	{
		LIBPCRITICALERROR("Empty argument in meIOStreamStart()");
		return NULL;
	}

	args = malloc(sizeof(me_io_stream_stop_entry_params) * params->stop_list.stop_list_len);
	if (!args)
	{
		return NULL;
	}

	result = malloc(sizeof(me_io_stream_stop_res));
	if (result)
	{
		memcpy(args, params->stop_list.stop_list_val, sizeof(me_io_stream_stop_entry_params) * params->stop_list.stop_list_len);

		result->error = meIOStreamStop(
							(meIOStreamStop_t *) args,
							params->stop_list.stop_list_len,
							params->flags);

		result->stop_list.stop_list_val = malloc(sizeof(int) * params->stop_list.stop_list_len);
		if (result->stop_list.stop_list_val)
		{
			result->stop_list.stop_list_len = params->stop_list.stop_list_len;

			for (i = 0; i < params->stop_list.stop_list_len; i++)
			{
				result->stop_list.stop_list_val[i] = args[i].error;
			}
		}
		else
		{
			result->stop_list.stop_list_len = 0;
			result->error = ME_ERRNO_INTERNAL;
		}
	}

	free(args);

	return result;
}


me_io_stream_status_res * me_io_stream_status_proc_1_svc(me_io_stream_status_params *params, struct svc_req *dummy)
{
	me_io_stream_status_res* result = malloc(sizeof(me_io_stream_status_res));

	if (result)
	{
		result->error = meIOStreamStatus(
							params->device,
							params->subdevice,
							params->wait,
							&result->status,
							&result->count,
							params->flags);
	}

	return result;
}


me_io_stream_frequency_to_ticks_res * me_io_stream_frequency_to_ticks_proc_1_svc(me_io_stream_frequency_to_ticks_params *params, struct svc_req *dummy)
{
	me_io_stream_frequency_to_ticks_res* result = malloc(sizeof(me_io_stream_frequency_to_ticks_res));

	if (result)
	{
		result->frequency = params->frequency;
		result->error = meIOStreamFrequencyToTicks(
							params->device,
							params->subdevice,
							params->timer,
							&result->frequency,
							&result->ticks_low,
							&result->ticks_high,
							params->flags);
	}

	return result;
}


me_io_stream_time_to_ticks_res * me_io_stream_time_to_ticks_proc_1_svc(me_io_stream_time_to_ticks_params *params, struct svc_req *dummy)
{
	me_io_stream_time_to_ticks_res* result = malloc(sizeof(me_io_stream_time_to_ticks_res));

	if (result)
	{
		result->time = params->time;
		result->error = meIOStreamTimeToTicks(
							params->device,
							params->subdevice,
							params->timer,
							&result->time,
							&result->ticks_low,
							&result->ticks_high,
							params->flags);
	}

	return result;
}


me_query_description_device_res * me_query_description_device_proc_1_svc(int *device, struct svc_req *dummy)
{
	/// @note Can not change it easly. This is seldom in use and not importand, so no big harm can be done... . If someone will complain than we will see what can be done.
	static char description[ME_DEVICE_DESCRIPTION_MAX_COUNT] =
	    {'\0'
	    };

	me_query_description_device_res* result = malloc(sizeof(me_query_description_device_res));
	if (result)
	{
		result->error = meQueryDescriptionDevice(*device, description, ME_DEVICE_DESCRIPTION_MAX_COUNT);
		result->description = description;

	}

	return result;
}


me_query_info_device_res * me_query_info_device_proc_1_svc(int *device, struct svc_req *dummy)
{
	me_query_info_device_res* result = malloc(sizeof(me_query_info_device_res));

	if (result)
	{
		result->error = meQueryInfoDevice(
							*device,
							&result->vendor_id,
							&result->device_id,
							&result->serial_no,
							&result->bus_type,
							&result->bus_no,
							&result->dev_no,
							&result->func_no,
							&result->plugged);
	}

	return result;
}


me_query_name_device_res * me_query_name_device_proc_1_svc(int *device, struct svc_req *dummy)
{
	/// @note Can not change it easly. This is seldom in use and not importand, so no big harm can be done... . If someone will complain than we will see what can be done.
	static char name[ME_DEVICE_NAME_MAX_COUNT] =
	    {'\0'
	    };

	me_query_name_device_res* result = malloc(sizeof(me_query_name_device_res));
	if (result)
	{
		result->error = meQueryNameDevice(*device, name, ME_DEVICE_DESCRIPTION_MAX_COUNT);
		result->name = name;

	}

	return result;
}


me_query_name_device_driver_res * me_query_name_device_driver_proc_1_svc(int *device, struct svc_req *dummy)
{
	/// @note Can not change it easly. This is seldom in use and not importand, so no big harm can be done... . If someone will complain than we will see what can be done.
	static char name[ME_DEVICE_DRIVER_NAME_MAX_COUNT] =
	    {'\0'
	    };

	me_query_name_device_driver_res* result = malloc(sizeof(me_query_name_device_driver_res));
	if (result)
	{
		result->error = meQueryNameDeviceDriver(*device, name, ME_DEVICE_DESCRIPTION_MAX_COUNT);
		result->name = name;

	}

	return result;
}


me_query_number_devices_res * me_query_number_devices_proc_1_svc(void *params, struct svc_req *dummy)
{
	me_query_number_devices_res* result = malloc(sizeof(me_query_number_devices_res));

	if (result)
	{
		result->error = meQueryNumberDevices(&result->number);
	}

	return result;
}


me_query_number_subdevices_res * me_query_number_subdevices_proc_1_svc(int *device, struct svc_req *dummy)
{
	me_query_number_subdevices_res* result = malloc(sizeof(me_query_number_subdevices_res));

	if (result)
	{
		result->error = meQueryNumberSubdevices(*device, &result->number);
	}

	return result;
}


me_query_number_channels_res * me_query_number_channels_proc_1_svc(me_query_number_channels_params *params, struct svc_req *dummy)
{
	me_query_number_channels_res* result = malloc(sizeof(me_query_number_channels_res));

	if (result)
	{
		result->error = meQueryNumberChannels(params->device, params->subdevice, &result->number);
	}

	return result;
}


me_query_number_ranges_res * me_query_number_ranges_proc_1_svc(me_query_number_ranges_params *params, struct svc_req *dummy)
{
	me_query_number_ranges_res* result = malloc(sizeof(me_query_number_ranges_res));

	if (result)
	{
		result->error = meQueryNumberRanges(params->device, params->subdevice, params->unit, &result->number);
	}

	return result;
}


me_query_range_by_min_max_res * me_query_range_by_min_max_proc_1_svc(me_query_range_by_min_max_params *params, struct svc_req *dummy)
{
	me_query_range_by_min_max_res* result = malloc(sizeof(me_query_range_by_min_max_res));

	if (result)
	{
		result->min = params->min;
		result->max = params->max;
		result->error = meQueryRangeByMinMax(
							params->device,
							params->subdevice,
							params->unit,
							&result->min,
							&result->max,
							&result->max_data,
							&result->range);
	}

	return result;
}


me_query_range_info_res * me_query_range_info_proc_1_svc(me_query_range_info_params *params, struct svc_req *dummy)
{
	me_query_range_info_res* result = malloc(sizeof(me_query_range_info_res));

	if (result)
	{
		result->error = meQueryRangeInfo(
							params->device,
							params->subdevice,
							params->range,
							&result->unit,
							&result->min,
							&result->max,
							&result->max_data);
	}

	return result;
}


me_query_subdevice_by_type_res * me_query_subdevice_by_type_proc_1_svc(me_query_subdevice_by_type_params *params, struct svc_req *dummy)
{
	me_query_subdevice_by_type_res* result = malloc(sizeof(me_query_subdevice_by_type_res));

	if (result)
	{
		result->error = meQuerySubdeviceByType(
							params->device,
							params->start_subdevice,
							params->type,
							params->subtype,
							&result->subdevice);
	}

	return result;
}


me_query_subdevice_type_res * me_query_subdevice_type_proc_1_svc(me_query_subdevice_type_params *params, struct svc_req *dummy)
{
	me_query_subdevice_type_res* result = malloc(sizeof(me_query_subdevice_type_res));

	if (result)
	{
		result->error = meQuerySubdeviceType(
							params->device,
							params->subdevice,
							&result->type,
							&result->subtype);
	}

	return result;
}


me_query_subdevice_caps_res * me_query_subdevice_caps_proc_1_svc(me_query_subdevice_caps_params *params, struct svc_req *dummy)
{
	me_query_subdevice_caps_res* result = malloc(sizeof(me_query_subdevice_caps_res));

	if (result)
	{
		result->error = meQuerySubdeviceCaps(
							params->device,
							params->subdevice,
							&result->caps);
	}

	return result;
}


me_query_subdevice_caps_args_res * me_query_subdevice_caps_args_proc_1_svc(me_query_subdevice_caps_args_params *params, struct svc_req *dummy)
{
	me_query_subdevice_caps_args_res* result = malloc(sizeof(me_query_subdevice_caps_args_res));

	result = malloc(sizeof(me_query_subdevice_caps_args_res));
	if (result)
	{
		if (params->count > 0)
		{
			result->args.args_val = malloc(sizeof(int) * params->count);
			if (result->args.args_val)
			{
				result->args.args_len = params->count;
				result->error = meQuerySubdeviceCapsArgs(
								params->device,
								params->subdevice,
								params->cap,
								result->args.args_val,
								result->args.args_len);
			}
			else
			{
				result->args.args_len = 0;
				result->error = ME_ERRNO_INTERNAL;
			}
		}
		else
		{
			LIBPCRITICALERROR("Empty argument in meQuerySubdeviceCapsArgs()");
			result->args.args_len = 0;
			result->error = ME_ERRNO_INTERNAL;
		}


	}

	return result;
}


me_query_version_library_res * me_query_version_library_proc_1_svc(void *params, struct svc_req *dummy)
{
	me_query_version_library_res* result = malloc(sizeof(me_query_version_library_res));

	if (result)
	{
		result->error = meQueryVersionLibrary(&result->ver);
	}

	return result;
}


me_query_version_main_driver_res * me_query_version_main_driver_proc_1_svc(void *params, struct svc_req *dummy)
{
	me_query_version_main_driver_res* result = malloc(sizeof(me_query_version_main_driver_res));

	if (result)
	{
		result->error = meQueryVersionMainDriver(&result->ver);
	}

	return result;
}


me_query_version_device_driver_res * me_query_version_device_driver_proc_1_svc(int *device, struct svc_req *dummy)
{
	me_query_version_device_driver_res* result = malloc(sizeof(me_query_version_device_driver_res));

	if (result)
	{
		result->error = meQueryVersionDeviceDriver(*device, &result->ver);
	}

	return result;
}
