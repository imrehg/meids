#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <string.h>

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_internal.h"

# include "meids_config_structs.h"
# include "meids_local_calls.h"
# include "meids_debug.h"

# include "meids_local_config.h"

static int  build_me_drv_device_list(me_local_context_t* context, me_cfg_device_entry_t** device_list, unsigned int *count, int max_dev);
static int  build_me_drv_device_entry(me_local_context_t* context, me_cfg_device_entry_t* device, int number);
static int  build_me_drv_device_info(me_local_context_t* context, me_cfg_device_entry_t *device, int number);
static int  build_me_drv_subdevice_list(me_local_context_t* context, me_cfg_subdevice_entry_t** subdevice_list, unsigned int *count, int number, int max_subdev);
static int  build_me_drv_subdevice_entry(me_local_context_t* context, me_cfg_subdevice_entry_t* subdevice, int number, int subnumber);
static int  build_me_drv_subdevice_info(me_local_context_t* context, me_cfg_subdevice_info_t *info, int number, int subnumber);
static int  build_me_drv_range_list(me_local_context_t* context, me_cfg_range_info_t** range_list, unsigned int *count, int number, int subnumber, int max_ranges);
static int  build_me_drv_range_entry(me_local_context_t* context, me_cfg_range_info_t *range, int number, int subnumber, int rangenumber);

int ConfigRead_Local(me_local_context_t* context, me_config_t* cfg, int iFlags)
{
	int err=ME_ERRNO_SUCCESS;
	int no_devices = 0;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_VALUE_NOT_USED)
	{
		LIBPERROR("Invalid flag specified.\n");
		err = ME_ERRNO_INVALID_FLAGS;
	}

	err = QueryDevicesNumber_Local(context, &no_devices, ME_QUERY_NO_FLAGS);
	if (!err)
	{
		if (no_devices)
		{
			// Config has 'device_list' and 'device_entry' nodes. Reserve memory for device structure.
			cfg->device_list = calloc(no_devices, sizeof(me_cfg_device_entry_t*));
			if (cfg->device_list)
			{
				// Build the structure.
				err = build_me_drv_device_list(context, cfg->device_list, &cfg->device_list_count, no_devices);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for device_list structure.");
				err = ME_ERRNO_INTERNAL;
			}
		}
		else
		{
			err = ME_ERRNO_INTERNAL;
			LIBPERROR("No devices in system.\n");
		}
	}

	return err;
}

static int build_me_drv_device_list(me_local_context_t* context, me_cfg_device_entry_t** device_list, unsigned int *count, int max_dev)
{
	int err = ME_ERRNO_SUCCESS;
	unsigned int cnt = 0;
	me_cfg_device_entry_t* cur_device;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cnt = 0; cnt < max_dev; )
	{
			cur_device = calloc(1, sizeof(me_cfg_device_entry_t));
			if (cur_device)
			{
				cur_device->context = context;
				*device_list = cur_device;
				device_list++;

				err = build_me_drv_device_entry(context, cur_device, cnt);
				cnt++;
			}
			else
			{
				LIBPERROR("Can not get requestet memory for device_entry.");
				err = ME_ERRNO_INTERNAL;
			}

			if (err)
				break;
	}
	*count = cnt;
	return err;
}

static int build_me_drv_device_entry(me_local_context_t* context, me_cfg_device_entry_t* device, int number)
{
	int err;
	int no_subdevices = 0;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device->subdevice_list_count = 0;
	device->subdevice_list = NULL;

	err = QuerySubdevicesNumber_Local(context, number, &no_subdevices, ME_QUERY_NO_FLAGS);
	if (!err)
	{
		if (no_subdevices)
		{
			device->subdevice_list = calloc(no_subdevices, sizeof(me_cfg_subdevice_entry_t*));
			if (device->subdevice_list)
			{
				err = build_me_drv_device_info(context, device, number);
				if (!err)
					err = build_me_drv_subdevice_list(context, device->subdevice_list, &device->subdevice_list_count, number, no_subdevices);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for subdevice_list structure.\n");
				err = ME_ERRNO_INTERNAL;
			}
		}
	}

	return err;
}

static int build_me_drv_device_info(me_local_context_t* context, me_cfg_device_entry_t *device, int number)
{
	int err = ME_ERRNO_SUCCESS;
	char tmp[256];
	int plugged;
	unsigned int bus_type;
	unsigned int lenght;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device->info.device_name = NULL;
	device->info.device_description = NULL;

	err = QueryDeviceInfo_Local(context, number,
								&device->info.vendor_id,
								&device->info.device_id,
								&device->info.serial_no,
								&bus_type,
								&device->info.pci.bus_no,
								&device->info.pci.device_no,
								&device->info.pci.function_no,
								&plugged,
								ME_QUERY_NO_FLAGS);

	if (!err)
	{
		switch (bus_type)
		{
			case ME_BUS_TYPE_PCI:
				device->access_type = me_access_type_PCI;
				break;

			case ME_BUS_TYPE_USB:
				device->access_type = me_access_type_USB;
				break;

			default:
				device->access_type = me_access_type_invalid;
				LIBPERROR("Wrong bus type returned: 0x%x(%d)\n", bus_type, bus_type);
				err = ME_ERRNO_INTERNAL;
		}

		switch (plugged)
		{
			case ME_PLUGGED_IN:
				device->plugged = me_plugged_type_IN;
				break;

			case ME_PLUGGED_OUT:
				device->plugged = me_plugged_type_OUT;
				break;

			default:
				device->plugged = me_plugged_type_invalid;
		}
	}

	if (!err)
	{
		memset(tmp, 0, 256);
		err = QueryDeviceName_Local(context, number, tmp, 255, ME_QUERY_NO_FLAGS);
		if (!err)
		{
			lenght = strlen(tmp);
			if (lenght)
			{
				device->info.device_name = calloc(lenght+1, sizeof(char));
				if (device->info.device_name)
				{
					strcpy(device->info.device_name, tmp);
				}
				else
				{
					LIBPERROR("Can not get requestet memory for device_name.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
	}

	if (!err)
	{
		memset(tmp, 0, 256);
		err = QueryDeviceDescription_Local(context, number, tmp, 255, ME_QUERY_NO_FLAGS);
		if (!err)
		{
			lenght = strlen(tmp);
			if (lenght)
			{
				device->info.device_description = calloc(lenght+1, sizeof(char));
				if (device->info.device_description)
				{
					strcpy(device->info.device_description, tmp);
				}
				else
				{
					LIBPERROR("Can not get requestet memory for device_description.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
	}

	device->logical_device_no = -1;
	device->info.device_no = number;

	return err;
}

static int build_me_drv_subdevice_list(me_local_context_t* context, me_cfg_subdevice_entry_t** subdevice_list, unsigned int *count, int number, int max_subdev)
{
	int err = ME_ERRNO_SUCCESS;
	unsigned int cnt = 0;
	me_cfg_subdevice_entry_t* cur_subdevice;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cnt = 0; cnt < max_subdev; )
	{
		cur_subdevice = calloc(1, sizeof(me_cfg_subdevice_entry_t));
		if (cur_subdevice)
		{
			*subdevice_list = cur_subdevice;
			subdevice_list++;

			err = build_me_drv_subdevice_entry(context, cur_subdevice, number, cnt);
			cnt++;
		}
		else
		{
			LIBPERROR("Can not get requestet memory for subdevice_entry.");
			err = ME_ERRNO_INTERNAL;
		}

		if (err)
			break;
	}

	*count = cnt;
	return err;

}

static int build_me_drv_subdevice_entry(me_local_context_t* context, me_cfg_subdevice_entry_t* subdevice, int number, int subnumber)
{
	int err;
	int no_ranges;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	subdevice->info.range_list_count = 0;
	subdevice->info.range_list = NULL;

	subdevice->extention.type = me_cfg_extention_type_none;
	subdevice->locked = ME_LOCK_RELEASE;

	err = build_me_drv_subdevice_info(context, &subdevice->info, number, subnumber);
	if (!err)
	{
		err = QueryRangesNumber_Local(context, number, subnumber, ME_UNIT_ANY, &no_ranges, ME_QUERY_NO_FLAGS);
	}

	if (!err)
	{
		if (no_ranges)
		{
			subdevice->info.range_list = calloc(no_ranges, sizeof(me_cfg_range_info_t*));
			if (subdevice->info.range_list)
			{
				err = build_me_drv_range_list(context, subdevice->info.range_list, &subdevice->info.range_list_count, number, subnumber, no_ranges);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for range_list.");
				return ME_ERRNO_INTERNAL;
			}
		}


	}
	else if (err == ME_ERRNO_NOT_SUPPORTED)
	{
		err = ME_ERRNO_SUCCESS;
	}

	return err;
}

static int build_me_drv_subdevice_info(me_local_context_t* context, me_cfg_subdevice_info_t *info, int number, int subnumber)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = QuerySubdeviceType_Local(context, number, subnumber, &info->type, &info->sub_type, ME_QUERY_NO_FLAGS);
	if (!err)
	{
		err = QueryChannelsNumber_Local(context, number, subnumber, &info->channels, ME_QUERY_NO_FLAGS);
	}

	return err;
}

static int build_me_drv_range_list(me_local_context_t* context, me_cfg_range_info_t** range_list, unsigned int *count, int number, int subnumber, int max_ranges)
{
	int err = ME_ERRNO_SUCCESS;
	int cnt = 0;

	me_cfg_range_info_t* cur_ranges;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cnt = 0; cnt < max_ranges; )
	{
		cur_ranges = calloc(1, sizeof(me_cfg_range_info_t));
		if (cur_ranges)
		{
			*range_list = cur_ranges;
			range_list++;

			err = build_me_drv_range_entry(context, cur_ranges, number, subnumber, cnt);
			cnt++;
		}
		else
		{
			LIBPERROR("Can not get requestet memory for range_entry.");
			err = ME_ERRNO_INTERNAL;
		}

		if (err)
			break;
	}

	*count = cnt;
	return err;
}

static int build_me_drv_range_entry(me_local_context_t* context, me_cfg_range_info_t *range, int number, int subnumber, int rangenumber)
{
	int err;
	int unit;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = QueryRangeInfo_Local(context, number, subnumber, rangenumber, &unit, &range->min, &range->max, &range->max_data, ME_QUERY_NO_FLAGS);

	if (!err)
	{
		range->unit = (enum me_units_type) unit;
	}
	else
	{
		range->unit = me_units_type_invalid;
	}

	return err;
}

