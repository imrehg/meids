/* Shared library for Meilhaus driver system.
 * ==========================================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author:	Guenter Gebhardt
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */
#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <stdio.h>

# include "me_error.h"
# include "rmedriver.h"

# include "meids_rpc_RQuery.h"

/// Functions to query a remote driver system. No context mode. They are only to use in MEiDC.

int meRQueryDescriptionDevice(	char* host, int iDevice,
								char* pcDescription, int iCount)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_description_device_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	if (iCount <= 0)
	{
		return ME_ERRNO_USER_BUFFER_SIZE;
	}

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		result = me_query_description_device_proc_1(&iDevice, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;
			if (!err)
			{
				strncpy(pcDescription, result->description, (iCount < strlen(result->description)) ? iCount : strlen(result->description)+1);
				pcDescription[iCount - 1] = '\0';
			}
			else
			{
				*pcDescription = '\0';
			}

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}


int meRQueryInfoDevice(	char* host, int iDevice,
						int* piVendorId, int* piDeviceId, int* piSerialNo,
						int* piBusType, int* piBusNo, int* piDevNo, int* piFuncNo,
						int* piPlugged)
{
	int flags = ME_VALUE_NOT_USED;
	int* rpc_err = NULL;
	CLIENT* clnt = NULL;
	me_query_info_device_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;

	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		result = me_query_info_device_proc_1(&iDevice, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;

		}
		else
		{
			err = result->error;

			*piVendorId	= result->vendor_id;
			*piDeviceId	= result->device_id;
			*piSerialNo	= result->serial_no;
			*piBusType	= result->bus_type;
			*piBusNo	= result->bus_no;
			*piDevNo	= result->dev_no;
			*piFuncNo	= result->func_no;
			*piPlugged	= result->plugged;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryNameDevice(	char* host, int iDevice,
						char* pcName, int iCount)
{
	int flags = ME_VALUE_NOT_USED;
	int* rpc_err = NULL;
	CLIENT* clnt = NULL;
	me_query_name_device_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	if (iCount <= 0)
	{
		return ME_ERRNO_USER_BUFFER_SIZE;
	}

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		result = me_query_name_device_proc_1(&iDevice, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;
			if (!err)
			{
				strncpy(pcName, result->name, (iCount < strlen(result->name)) ? iCount : strlen(result->name)+1);
				pcName[iCount - 1] = '\0';
			}
			else
			{
				*pcName = '\0';
			}

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryNumberDevices(char* host, int* piNumber)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_number_devices_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		result = me_query_number_devices_proc_1(NULL, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piNumber = result->number;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryNumberSubdevices(	char* host, int iDevice,
								int* piNumber)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_number_subdevices_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		result = me_query_number_subdevices_proc_1(&iDevice, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piNumber = result->number;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryNumberChannels(	char* host, int iDevice, int iSubdevice,
							int* piNumber)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_number_channels_params query_params;
	me_query_number_channels_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		query_params.device = iDevice;
		query_params.subdevice = iSubdevice;

		result = me_query_number_channels_proc_1(&query_params, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piNumber = result->number;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryNumberRanges(	char* host, int iDevice, int iSubdevice,
							int iUnit, int* piNumber)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_number_ranges_params query_params;
	me_query_number_ranges_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		query_params.device = iDevice;
		query_params.subdevice = iSubdevice;
		query_params.unit = iUnit;

		result = me_query_number_ranges_proc_1(&query_params, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piNumber = result->number;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQueryRangeInfo(	char* host, int iDevice, int iSubdevice, int iRange,
						int* piUnit, double* pdMin, double* pdMax, int* piMaxData)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_range_info_params query_params;
	me_query_range_info_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		query_params.device = iDevice;
		query_params.subdevice = iSubdevice;
		query_params.range = iRange;

		result = me_query_range_info_proc_1(&query_params, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piUnit = result->unit;
			*pdMin = result->min;
			*pdMax = result->max;
			*piMaxData = result->max_data;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}

int meRQuerySubdeviceType(	char* host, int iDevice, int iSubdevice,
							int* piType, int* piSubtype)
{
	int flags = ME_VALUE_NOT_USED;
	CLIENT* clnt = NULL;
	int* rpc_err = NULL;
	me_query_subdevice_type_params query_params;
	me_query_subdevice_type_res* result = NULL;
	int err = ME_ERRNO_SUCCESS;

	clnt = clnt_create(host, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!clnt)
	{
		return ME_ERRNO_CONNECT_REMOTE;
	}

	rpc_err = me_open_proc_1(&flags, clnt);
	if (!rpc_err)
	{
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		err = *rpc_err;
		free(rpc_err);
	}

	if (!err)
	{
		query_params.device = iDevice;
		query_params.subdevice = iSubdevice;

		result = me_query_subdevice_type_proc_1(&query_params, clnt);
		if (!result)
		{
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			err = result->error;

			*piType = result->type;
			*piSubtype = result->subtype;

			free(result);
		}

		rpc_err = me_close_proc_1(&flags, clnt);
		if (!err)
		{
			err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		}

		if (rpc_err)
		{
			free(rpc_err);
		}
	}

	clnt_destroy(clnt);

	return err;
}
