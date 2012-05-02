/* Shared library for Meilhaus driver system (local).
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
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <fcntl.h>
# include <errno.h>

# include <float.h>
# include <math.h>

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_internal.h"
# include "meids_debug.h"
# include "meids_local_calls.h"

static int   doCreateThread_Local(me_local_context_t* context, int device, int subdevice, void* fnThread, void* fnCB, void* contextCB, int iFlags);
static int   doDestroyAllThreads_Local(me_local_context_t* context);
static int   doDestroyThreads_Local(me_local_context_t* context, int device);
static int   doDestroyThread_Local(me_local_context_t* context, int device, int subdevice);

static void* irqThread_Local(void* arg);
static void* streamStartThread_Local(void* arg);
static void* streamStopThread_Local(void* arg);
static void* streamNewValuesThread_Local(void* arg);

static int QueryRangeByMinMax_calculate(void* context, int device, int subdevice, int unit, double min_val, double max_val, int* range, int iFlags);

// Access
int Open_Local(me_local_context_t* context, const char* address, int iFlags)
{

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_OPEN_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (context->fd >= 0)
	{// Already open
		LIBPDEBUG("Already open!\n");
		return ME_ERRNO_SUCCESS;
	}

	if (address)
	{
		context->fd = open(address, O_RDWR);
	}
	else
	{// Use default one (general).
		context->fd = open("/dev/medriver", O_RDWR);
	}

	if (context->fd < 0)
	{
		LIBPERROR("open()=ME_ERRNO_OPEN\n");
		return ME_ERRNO_OPEN;
	}

	context->activeThreads = NULL;
	pthread_mutex_init(&context->callbackContextMutex, NULL);
	return ME_ERRNO_SUCCESS;
}

int Close_Local(me_local_context_t* context, int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_CLOSE_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		err = ME_ERRNO_INVALID_FLAGS;
	}
	else
	{
		doDestroyAllThreads_Local(context);

		if (context->fd < 0)
		{
			// Already closed. This is not always an error. Standard routine call this function for every device on list.
			LIBPDEBUG("Already close!\n");
		}
		else
		{
			if (close(context->fd))
			{
				err = ME_ERRNO_CLOSE;
			}
			context->fd = -1;
		}
	}
	return err;
}

// Locks
int LockDriver_Local(void* context, int lock, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_lock_driver_t cmd;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("fd=%d lock=%d iFlags=0x%x\n", local_context->fd, lock, iFlags);

	cmd.lock = lock ;
	cmd.flags = iFlags;
	cmd.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_LOCK_DRIVER, &cmd);
	if (!err)
	{
		if (cmd.err_no)
		{
			LIBPWARNING("ioctl(ME_LOCK_DRIVER,...)=%d\n", cmd.err_no);
			err = cmd.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_LOCK_DRIVER,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int LockDevice_Local(void* context, int device, int lock, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_lock_device_t cmd;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("fd=%d device=%d lock=%d flags=0x%x\n", local_context->fd, device, lock, iFlags);

	cmd.device = device;
	cmd.lock = lock ;
	cmd.flags = iFlags;
	cmd.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_LOCK_DEVICE, &cmd);

	if (!err)
	{
		if (cmd.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_LOCK_DEVICE,...)=%d\n", device, cmd.err_no);
			err = cmd.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_LOCK_DEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int LockSubdevice_Local(void* context, int device, int subdevice, int lock, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_lock_subdevice_t cmd;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d device=%d lock=%d flags=0x%x\n", local_context->fd, device, lock, iFlags);

	cmd.device = device;
	cmd.subdevice = subdevice;
	cmd.lock = lock ;
	cmd.flags = iFlags;
	cmd.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_LOCK_SUBDEVICE, &cmd);

	if (!err)
	{
		if (cmd.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_LOCK_SUBDEVICE,...)=%d\n", device, subdevice, cmd.err_no);
			err = cmd.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_LOCK_SUBDEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

// Query
int QueryDriverVersion_Local(void* context, int* version, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_version_main_driver_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(version);


	LIBPDEBUG("fd=%d pVersion=%p\n", local_context->fd, version);

	query.err_no = ME_ERRNO_SUCCESS;
	err = ioctl(local_context->fd, ME_QUERY_VERSION_MAIN_DRIVER, &query);
	if (!err)
	{
		*version = query.version;

		if (query.err_no)
		{
			LIBPWARNING("ioctl(..., ME_QUERY_VERSION_MAIN_DRIVER,...)=%d\n", query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_VERSION_MAIN_DRIVER,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QueryDriverName_Local(void* context, char* name, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_name_main_driver_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	LIBPDEBUG("fd=%d pName=%p iCount=%d\n", local_context->fd, name, count);

	query.name = name;
	query.count = count;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NAME_MAIN_DRIVER, &query);
	if (!err)
	{
		if (query.err_no)
		{
			LIBPWARNING("ioctl(ME_QUERY_NAME_MAIN_DRIVER,...)=%d\n", query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NAME_MAIN_DRIVER,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int QuerySubdriverVersion_Local(void* context, int device, int* version, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_version_device_driver_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(version);

	LIBPDEBUG("fd=%d iDevice=%d pVersion=%p\n",
			local_context->fd, device, version);

	query.device = device;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_VERSION_DEVICE_DRIVER, &query);
	if (!err)
	{
		*version = query.version;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_VERSION_DEVICE_DRIVER,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_VERSION_DEVICE_DRIVER,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QuerySubdriverName_Local(void* context, int device, char* name, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_name_device_driver_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	LIBPDEBUG("fd=%d iDevice=%d pName=%p iCount=%d\n",
			local_context->fd, device, name, count);

	query.device = device;
	query.name = name;
	query.count = count;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NAME_DEVICE_DRIVER, &query);
	if (!err)
	{
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_NAME_DEVICE_DRIVER,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NAME_DEVICE_DRIVER,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int QueryDeviceName_Local(void* context, int device, char* name, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_name_device_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("ID:%d\n", device);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (count <= 0)
	{
		return ME_ERRNO_USER_BUFFER_SIZE;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	LIBPDEBUG("fd=%d iDevice=%d pName=%p iCount=%d\n",
			local_context->fd, device, name, count);

	query.device = device;
	query.name = name;
	query.count = count;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NAME_DEVICE, &query);
	if (!err)
	{
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_NAME_DEVICE,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NAME_DEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QueryDeviceDescription_Local(void* context, int device, char* description, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_description_device_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (count <= 0)
	{
		return ME_ERRNO_USER_BUFFER_SIZE;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(description);

	LIBPDEBUG("fd=%d iDevice=%d pDescription=%p iCount=%d\n",
			local_context->fd, device, description, count);

	query.device = device;
	query.name = description;
	query.count = count;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_DESCRIPTION_DEVICE, &query);
	if (!err)
	{
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_DESCRIPTION_DEVICE,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_DESCRIPTION_DEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QueryDevicesNumber_Local(void* context, int* no_devices, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_number_devices_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_devices);

	LIBPDEBUG("fd=%d\n", local_context->fd);

	query.err_no = ME_ERRNO_SUCCESS;
	err = ioctl(local_context->fd, ME_QUERY_NUMBER_DEVICES, &query);
	if (!err)
	{
		*no_devices = query.number;
		if (query.err_no)
		{
			LIBPWARNING("ioctl(..., ME_QUERY_NUMBER_DEVICES,...)=%d\n", query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NUMBER_DEVICES,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QueryDeviceInfo_Local(void* context, int device,
						unsigned int* vendor_id,
						unsigned int* device_id,
						unsigned int* serial_no,
						unsigned int* bus_type,
						unsigned int* bus_no,
						unsigned int* dev_no,
						unsigned int* func_no,
						int *plugged,
						int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_info_device_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(vendor_id);
	CHECK_POINTER(device_id);
	CHECK_POINTER(serial_no);
	CHECK_POINTER(bus_type);
	CHECK_POINTER(bus_no);
	CHECK_POINTER(dev_no);
	CHECK_POINTER(func_no);
	CHECK_POINTER(plugged);

	LIBPDEBUG("fd=%d iDevice=%d pVendor_id=%p pDevice_id=%p pSerial_no=%p pBus_type=%p pBus_no=%p pDev_no=%p pFunc_no=%p pPlugged=%p\n",
			local_context->fd, device, vendor_id, device_id, serial_no, bus_type, bus_no, dev_no, func_no, plugged);

	query.device = device;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_INFO_DEVICE, &query);
	if (!err)
	{
		*vendor_id = query.vendor_id;
		*device_id = query.device_id;
		*serial_no = query.serial_no;
		*bus_type  = query.bus_type;
		*bus_no    = query.bus_no;
		*dev_no    = query.dev_no;
		*func_no   = query.func_no;
		*plugged   = query.plugged;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_INFO_DEVICE,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_INFO_DEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int QuerySubdevicesNumber_Local(void* context, int device, int* no_subdevices, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_number_subdevices_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_subdevices);

	LIBPDEBUG("fd=%d iDevice=%d pNumber=%p\n",
			local_context->fd, device, no_subdevices);

	query.device = device;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NUMBER_SUBDEVICES, &query);
	if (!err)
	{
		*no_subdevices = query.number;
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_NUMBER_SUBDEVICES,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NUMBER_SUBDEVICES,...)=%d\n", local_context->fd, err);
	}

	LIBPDEBUG("ioctl(%d, ME_QUERY_NUMBER_SUBDEVICES,...)=%d returned %d subdevices\n", local_context->fd, err, *no_subdevices);
	return err;
}

int QuerySubdevicesNumberByType_Local(void* context, int device, int type, int subtype, int* no_subdevices, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_number_subdevices_by_type_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_subdevices);

	LIBPDEBUG("fd=%d iDevice=%d iType=%x iSubtype=%x pNumber=%p\n",
			local_context->fd, device, type, subtype, no_subdevices);

	query.device = device;
	query.type = type;
	query.subtype = subtype;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE, &query);
	if (!err)
	{
		*no_subdevices = query.number;
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE,...)=%d\n", local_context->fd, err);
	}

	LIBPDEBUG("ioctl(%d, ME_QUERY_NUMBER_SUBDEVICES_BY_TYPE,...)=%d returned %d subdevices\n", local_context->fd, err, *no_subdevices);
	return err;
}

int QuerySubdeviceType_Local(void* context, int device, int subdevice, int* type, int* subtype, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_subdevice_type_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(type);
	CHECK_POINTER(subtype);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d pType=%p pSubtype=%p\n",
			local_context->fd, device, subdevice, type, subtype);

	query.device = device;
	query.subdevice = subdevice;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_SUBDEVICE_TYPE, &query);
	if (!err)
	{
		*type = query.type;
		*subtype = query.subtype;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_SUBDEVICE_TYPE,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_SUBDEVICE_TYPE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QuerySubdeviceByType_Local(void* context, int device, int subdevice, int type, int subtype, int* result, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_subdevice_by_type_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(result);

	LIBPDEBUG("fd=%d iDevice=%d iStartSubdevice=%d iType=%x iSubtype=%x pSubdevice=%p\n",
			local_context->fd, device, subdevice, type, subtype, result);

	query.device = device;
	query.start_subdevice = subdevice;
	query.type = type;
	query.subtype = subtype;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_SUBDEVICE_BY_TYPE, &query);
	if (!err)
	{
		*result = query.subdevice;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_SUBDEVICE_BY_TYPE,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_SUBDEVICE_BY_TYPE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QuerySubdeviceCaps_Local(void* context, int device, int subdevice, int* caps, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_subdevice_caps_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(caps);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d pCaps=%p\n",
			local_context->fd, device, subdevice, caps);

	query.device = device;
	query.subdevice = subdevice;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_SUBDEVICE_CAPS, &query);
	if (!err)
	{
		*caps = query.caps;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_SUBDEVICE_CAPS,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_SUBDEVICE_CAPS,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QuerySubdeviceCapsArgs_Local(void* context, int device, int subdevice, int cap, int* args, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_subdevice_caps_args_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(args);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iCap=%d pArgs=%p iCount=%d\n",
			local_context->fd, device, subdevice, cap, args, count);

	query.device = device;
	query.subdevice = subdevice;
	query.cap = cap;
	query.args = args;
	query.count = count;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_SUBDEVICE_CAPS_ARGS, &query);
	if (!err)
	{
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_SUBDEVICE_CAPS_ARGS,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_SUBDEVICE_CAPS_ARGS,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QuerySubdeviceTimer_Local(void* context, int device, int subdevice,
							int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;
	me_query_timer_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags & ~ME_MEPHISTO_SCOPE_OSCILLOSCOPE_FLAG)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(min_ticks_high);
	CHECK_POINTER(min_ticks_low);
	CHECK_POINTER(max_ticks_high);
	CHECK_POINTER(max_ticks_low);
	CHECK_POINTER(base);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d Timer=0x%x piFrequency=%p piMin_Tick_Low=%p piMin_Tick_High=%p piHigh_Tick_Low=%p piHigh_Tick_High=%p\n",
			local_context->fd, device, subdevice, timer, base, min_ticks_low, min_ticks_high, max_ticks_low, max_ticks_high);

	query.device = device;
	query.subdevice = subdevice;
	query.timer = timer;
	if (iFlags)
	{
		query.timer |= ME_TIMER_OSCILLOSCOPE_FLAG;
	}
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_TIMER, &query);
	LIBPDEBUG("Device=%d, Subdevice=%d, Timer=%d, Flags=0x%x => min=%lld max=%lld\n", device, subdevice, timer, iFlags, query.min_ticks, query.max_ticks);
	if (!err)
	{
		*base = query.base_frequency;
		*min_ticks_low = query.min_ticks;
		*min_ticks_high = query.min_ticks >> 32;
		*max_ticks_low = query.max_ticks;
		*max_ticks_high = query.max_ticks >> 32;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d iTimer=0x%x), ME_QUERY_TIMER,...)=%d\n", device, subdevice, timer, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_TIMER,...)=%d\n", local_context->fd, err);
	}

	LIBPDEBUG("*base=%d min_ticks_low=%d, min_ticks_high=%d, max_ticks_low=%d, max_ticks_high=%d err=%d\n",
	*base, *min_ticks_low, *min_ticks_high, *max_ticks_low, *max_ticks_high, err);
	return err;
}


int QueryChannelsNumber_Local(void* context, int device, int subdevice, unsigned int* number, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_number_channels_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}
	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d pNumber=%p\n",
			local_context->fd, device, subdevice, number);

	CHECK_POINTER(context);
	CHECK_POINTER(number);

	query.device = device;
	query.subdevice = subdevice;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NUMBER_CHANNELS, &query);
	if (!err)
	{
		*number = query.number;
		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_QUERY_NUMBER_CHANNELS,...)=%d\n", device, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NUMBER_CHANNELS,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int QueryRangesNumber_Local(void* context, int device, int subdevice, int unit, int* no_ranges, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_number_ranges_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_ranges);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iUnit=%d pNumber=%p\n",
			local_context->fd, device, subdevice, unit, no_ranges);

	query.device = device;
	query.subdevice = subdevice;
	query.unit = unit;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_NUMBER_RANGES, &query);
	if (!err)
	{
		*no_ranges = query.number;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_NUMBER_RANGES,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_NUMBER_RANGES,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int QueryRangeInfo_Local(void* context, int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data,int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_query_range_info_t query;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(unit);
	CHECK_POINTER(min);
	CHECK_POINTER(max);
	CHECK_POINTER(max_data);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iRange=%d pUnit=%p pMin=%p pMax=%p pMaxData=%p\n",
			local_context->fd, device, subdevice, range, unit, min, max, max_data);

	query.device = device;
	query.subdevice = subdevice;
	query.range = range;
	query.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_QUERY_RANGE_INFO, &query);
	if (!err)
	{

		*unit = query.unit;

		*min = (double) query.min / 1E6;
		*max = (double) query.max / 1E6;
		*max_data = query.max_data;

		if (query.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_RANGE_INFO,...)=%d\n", device, subdevice, query.err_no);
			err = query.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_QUERY_RANGE_INFO,...)=%d\n", local_context->fd, err);
	}

	return err;
}

// int QueryRangeByMinMax_Local(void* context, int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags)
// {
// 	me_local_context_t* local_context = (me_local_context_t *)context;
// 	int err;
// 	me_query_range_by_min_max_t query;
//
// 	LIBPINFO("executed: %s\n", __FUNCTION__);
//
// 	if (iFlags != ME_QUERY_NO_FLAGS)
// 	{
// 		LIBPERROR("Invalid flag specified.\n");
// 		return ME_ERRNO_INVALID_FLAGS;
// 	}
//
// 	CHECK_POINTER(context);
// 	CHECK_POINTER(range);
// 	CHECK_POINTER(max_data);
// 	CHECK_POINTER(max);
// 	CHECK_POINTER(min);
//
// 	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iUnit=%d pMin=%p pMax=%p pMaxData=%p pRange=%p\n",
// 			local_context->fd, device, subdevice, unit, min, max, max_data, range);
//
// 	query.device = device;
// 	query.subdevice = subdevice;
// 	query.unit = unit;
// 	query.min = (int)(*min * 1E6);
// 	query.max = (int)(*max * 1E6);
// 	query.err_no = ME_ERRNO_SUCCESS;
//
// 	err = ioctl(local_context->fd, ME_QUERY_RANGE_BY_MIN_MAX, &query);
// 	if (!err)
// 	{
// 		*min = (double) query.min / 1E6;
// 		*max = (double) query.max / 1E6;
// 		*max_data = query.max_data;
// 		*range = query.range;
//
// 		if (query.err_no)
// 		{
// 			LIBPWARNING("ioctl((iDevice=%d iSubdevice=%d), ME_QUERY_RANGE_BY_MIN_MAX,...)=%d\n", device, subdevice, query.err_no);
// 			err = query.err_no;
// 		}
// 	}
// 	else
// 	{
// 		LIBPERROR("ioctl(%d, ME_QUERY_RANGE_BY_MIN_MAX,...)=%d\n", local_context->fd, err);
// 	}
//
// 	return err;
// }

static int QueryRangeByMinMax_calculate(void* context, int device, int subdevice, int unit, double min_val, double max_val, int* range, int iFlags)
{
	const float tolerance = 0.0001;	//0.1%

	unsigned int i;
	int r = -1;
	double diff = FLT_MAX;
	double range_diff;
	double range_tolerance;

	double			range_min;
	double			range_max;
	unsigned int	range_max_data;
	int				range_unit;
	int				no_ranges;

	int err;

	err = QueryRangesNumber_Local(context, device, subdevice, unit, &no_ranges, iFlags);
	if (err)
	{
		return err;
	}

	for (i = 0; i < no_ranges; ++i)
	{
		err = QueryRangeInfo_Local(context, device, subdevice, i, &range_unit, &range_min, &range_max, &range_max_data, iFlags);
		if (err)
		{
			return err;
		}

		range_tolerance = (range_max - range_min) * tolerance;
		if ((range_min - range_tolerance <= min_val) && ((range_max + range_tolerance) >= max_val))
		{
			range_diff = ((range_max - max_val) * (range_max - max_val)) + ((range_min - min_val) * (range_min - min_val));
			if (range_diff < diff)
			{
				r = i;
				diff = range_diff;
			}
			if (range_diff == 0.0)
			{// Perfect match
				break;
			}
		}
	}

	*range = r;

	return (r < 0) ? ME_ERRNO_NO_RANGE : ME_ERRNO_SUCCESS;
}

int QueryRangeByMinMax_Local(void* context, int device, int subdevice, int unit, double *min, double *max, int* max_data, int* range, int iFlags)
{
	int err;
	int range_unit;

	if (*max < *min)
	{
		LIBPERROR("Invalid minimum and maximum values specified. MIN:%f > MAX:%f\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	err = QueryRangeByMinMax_calculate(context, device, subdevice, unit, *min, *max, range, iFlags);
	if (!err)
	{
		err = QueryRangeInfo_Local(context, device, subdevice, *range, &range_unit, min, max, (unsigned int *)max_data, iFlags);
	}

	return err;
}

//Input/Output
int IrqStart_Local(void* context, int device, int subdevice, int channel, int source, int edge, int arg, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_irq_start_t enable;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iIrqSource=%d iIrqEdge=%d iIrqArg=%d iFlags=0x%x\n",
		local_context->fd ,device, subdevice, channel, source, edge, arg, iFlags);

	enable.device = device;
	enable.subdevice = subdevice;
	enable.channel = channel;
	enable.irq_source = source;
	enable.irq_edge = edge;
	enable.irq_arg = arg;
	enable.flags = iFlags;
	enable.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_IRQ_ENABLE, &enable);
	if (!err)
	{
		if (enable.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_IRQ_ENABLE,...)=%d\n", device, subdevice, enable.err_no);
			err = enable.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_IRQ_ENABLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int IrqWait_Local(void* context, int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_irq_wait_t wait;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(value);
	CHECK_POINTER(count);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iTimeOut=%d iFlags=0x%x\n",
			local_context->fd ,device, subdevice, channel, timeout, iFlags);

	wait.device = device;
	wait.subdevice = subdevice;
	wait.channel = channel;
	wait.irq_count = *count;
	wait.value = *value;
	wait.time_out = timeout;
	wait.flags = iFlags;
	wait.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_IRQ_WAIT, &wait);
	if (!err)
	{
		*count = wait.irq_count;
		*value = wait.value;

		if (wait.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_IRQ_WAIT,...)=%d\n", device, subdevice, wait.err_no);
			err = wait.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_IRQ_WAIT,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int IrqStop_Local(void* context, int device, int subdevice, int channel, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_irq_stop_t disable;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iFlags=0x%x\n",
		local_context->fd ,device, subdevice, channel, iFlags);

	disable.device = device;
	disable.subdevice = subdevice;
	disable.channel = channel;
	disable.flags = iFlags;
	disable.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_IRQ_DISABLE, &disable);
	if (!err)
	{
		if (disable.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_IRQ_DISABLE,...)=%d\n", device, subdevice, disable.err_no);
			err = disable.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_IRQ_DISABLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int IrqTest_Local(void* context, int device, int subdevice, int channel, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_irq_stop_t disable;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iFlags=0x%x\n",
		local_context->fd ,device, subdevice, channel, iFlags);

	disable.device = device;
	disable.subdevice = subdevice;
	disable.channel = channel;
	disable.flags = iFlags;
	disable.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_IRQ_CHECK, &disable);
	if (!err)
	{
		if (disable.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_IRQ_CHECK,...)=%d\n", device, subdevice, disable.err_no);
			err = disable.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_IRQ_CHECK,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int IrqSetCallback_Local(void* context, int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d subdevice=%d irq_fn=%p iFlags=0x%x", device, subdevice, irq_fn, iFlags);

	if (irq_fn)
	{	// create
		err = IrqTest_Local(context, device, subdevice, 0, iFlags);
		if (!err)
		{
			err = doCreateThread_Local(local_context, device, subdevice, irqThread_Local, irq_fn, irq_context, iFlags);
		}
	}
	else
	{	// cancel
		err = doDestroyThread_Local(local_context, device, subdevice);
	}

	return err;
}


int ResetDevice_Local(void* context, int device, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_reset_device_t reset;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iFlags=0x%x\n",
		local_context->fd, device, iFlags);

	reset.device = device;
	reset.flags = iFlags;
	reset.err_no = ME_ERRNO_SUCCESS;

	doDestroyThreads_Local(context, device);

	err = ioctl(local_context->fd, ME_IO_RESET_DEVICE, &reset);
	if (!err)
	{
		if (reset.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d), ME_IO_RESET_DEVICE,...)=%d\n", device, reset.err_no);
			err = reset.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_RESET_DEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int ResetSubdevice_Local(void* context, int device, int subdevice, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_reset_subdevice_t reset;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iFlags=0x%x\n",
		local_context->fd, device, subdevice, iFlags);

	reset.device = device;
	reset.subdevice = subdevice;
	reset.flags = iFlags;
	reset.err_no = ME_ERRNO_SUCCESS;

	doDestroyThread_Local(context, device, subdevice);

	err = ioctl(local_context->fd, ME_IO_RESET_SUBDEVICE, &reset);
	if (!err)
	{
		if (reset.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_RESET_SUBDEVICE,...)=%d\n",
			device, subdevice, reset.err_no);
			err = reset.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_RESET_SUBDEVICE,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int SingleConfig_Local(void* context, int device, int subdevice, int channel,
                     int config, int reference, int synchro,
                     int trigger, int edge,	int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_single_config_t singleconfig;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d, iDevice=%d iSubdevice=%d iChannel=%d iSingleConfig=%d iRef=%d iTrigChan=%d iTrigType=%d iTrigEdge=%d iFlags=0x%x\n",
			local_context->fd, device, subdevice, channel, config, reference, synchro, trigger, edge, iFlags);

	singleconfig.device = device;
	singleconfig.subdevice = subdevice;
	singleconfig.channel = channel;
	singleconfig.single_config = config;
	singleconfig.ref = reference;
	singleconfig.trig_chain = synchro;
	singleconfig.trig_type = trigger;
	singleconfig.trig_edge = edge;
	singleconfig.flags = iFlags;
	singleconfig.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_SINGLE_CONFIG, &singleconfig);
	if (!err)
	{
		if (singleconfig.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_SINGLE_CONFIG,...)=%d\n", device, subdevice, singleconfig.err_no);
			err = singleconfig.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_SINGLE_CONFIG,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int Single_Local(void* context, int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_single_simple_t single;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(value);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iDir=%d piValue=%p iTimeOut=%d iFlags=0x%x\n",
			local_context->fd, device, subdevice, channel, direction, value, timeout, iFlags);

	LIBPDEBUG("iValue=%d\n", *value);

	single.device = device;
	single.subdevice = subdevice;
	single.channel = channel;
	single.dir = direction;
	single.value = *value;
	single.timeout = timeout;
	single.flags = iFlags;
	single.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_SINGLE_SIMPLE, &single);
	if (!err)
	{
		*value = single.value;
		if (single.err_no)
		{
			LIBPWARNING("ioctl(iDevice=%d, iSubdevice=%d, iChannel=%d, iDir=0x%x, ME_IO_SINGLE_SIMPLE,...)=%d\n",
					single.device, single.subdevice, single.channel, single.dir, single.err_no);
			err = single.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_SINGLE_SIMPLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int SingleList_Local(void* context, meIOSingle_t* list, int count, int iFlags)
{
/** @note This is an internal call.
* It must obey this rules:
* 1: All devices on list have to be the same kind (PCI or USB).
* 2: Maximum size of whole argument can be bigger than 4KB (maximum list size is 127 entries).
*
*  This is not check here!
*/
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
#ifdef LIBMEDEBUG_DEBUG
	int i;
#endif
	me_io_single_t single;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (count <= 0)
	{
		LIBPDEBUG("No items in list.\n");
		return ME_ERRNO_SUCCESS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	LIBPDEBUG("pSingleList%p iCount=%d iFlags=0x%x\n",
			list, count, iFlags);

#ifdef LIBMEDEBUG_DEBUG
	for (i=0; i<count; i++)
	{
		LIBPDEBUG("pSingleList[%d]={iDevice=%d iSubdevice=%d iChannel=%d iDir=%d iValue=%d iTimeOut=%d iFlags=0x%x}\n",
				i, list[i].iDevice, list[i].iSubdevice, list[i].iChannel,
				list[i].iDir, list[i].iValue, list[i].iTimeOut, list[i].iFlags);
	}
#endif

	single.single_list = list;
	single.count = count;
	single.flags = iFlags;
	single.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_SINGLE, &single);
	if (!err)
	{
		if (single.err_no)
		{
#ifdef LIBMEDEBUG_DEBUG
			for (i=0; i<count; i++)
			{
				if (list[i].iErrno)
				{
					LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_SINGLE,...)=%d\n",
						list[i].iDevice, list[i].iSubdevice, list[i].iErrno);
				}
			}
#endif
			err = single.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_SINGLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int StreamConfigure_Local(void* context, int device, int subdevice,
					meIOStreamSimpleConfig_t* list, int count,
					meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_config_t config;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (count <= 0)
	{
		LIBPDEBUG("No items in list.\n");
		return ME_ERRNO_SUCCESS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(trigger);
	CHECK_POINTER(list);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d pConfigList=%p iCount=%d pTrigger=%p iFifoIrqThreshold=%d iFlags=0x%x\n",
			local_context->fd, device, subdevice, list, count, trigger, threshold, iFlags);

	config.device = device;
	config.subdevice = subdevice;
	config.config_list = list;
	config.count = count;
	config.trigger = *trigger;
	config.fifo_irq_threshold = threshold;
	config.flags = iFlags;
	config.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_CONFIG, &config);
	if (!err)
	{
		if (config.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_CONFIG,...)=%d\n", device, subdevice, config.err_no);
			err = config.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_CONFIG,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamConfig_Local(void* context, int device, int subdevice,
					meIOStreamConfig_t* list, int count,
					meIOStreamTrigger_t* trigger, int threshold, int iFlags)
{
	int err;
	meIOStreamSimpleTriggers_t	simple_triggers;
	meIOStreamSimpleConfig_t*	simple_config = NULL;
	int flags;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(trigger);
	CHECK_POINTER(list);

	simple_config = calloc(count, sizeof(meIOStreamSimpleConfig_t));
	if (!simple_config)
	{
		LIBPERROR("Can not get requestet memory for simple_config list.\n");
		return ME_ERRNO_INTERNAL;
	}

	err = me_translate_triggers_to_simple(trigger, &simple_triggers);
	if (!err)
	{
		err =  me_translate_config_to_simple(list, count, iFlags, simple_config, &flags);
	}
	if (!err)
	{
		err = StreamConfigure_Local(context, device, subdevice, simple_config, count, &simple_triggers, threshold, flags);
	}

	free (simple_config);

	return err;
}

int StreamStart_Local(void* context, int device, int subdevice, int mode, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_start_simple_t stream_start;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iStartMode=%d iTimeOut=%d iFlags=0x%x\n",
			local_context->fd, device, subdevice, mode, timeout, iFlags);

	stream_start.device = device;
	stream_start.subdevice = subdevice;
	stream_start.mode = mode;
	stream_start.timeout = timeout;
	stream_start.flags = iFlags;
	stream_start.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_START_SIMPLE, &stream_start);
	if (!err)
	{
		if (stream_start.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_START_SIMPLE,...)=%d\n", device, subdevice, stream_start.err_no);
			err = stream_start.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_START_SIMPLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamStartList_Local(void* context, meIOStreamStart_t* list, int count, int iFlags)
{
/** @note This is an internal call.
* It must obey this rules:
* 1: All devices on list have to be the same kind (PCI or USB).
* 2: Maximum size of whole argument can be bigger than 4KB (maximum list size is 127 entries).
*
*  This is not check here!
*/
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
#ifdef LIBMEDEBUG_DEBUG
	int i;
#endif
	me_io_stream_start_t start;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	LIBPDEBUG("fd=%d pStartList=%p iCount=%d iFlags=0x%x\n",
			local_context->fd, list, count, iFlags);

	if (count <= 0)
	{
		LIBPDEBUG("No items in list.");
		return ME_ERRNO_SUCCESS;
	}

#ifdef LIBMEDEBUG_DEBUG
	for (i=0; i<count; i++)
	{
		LIBPDEBUG("pStartList[%d]={iDevice=%d iSubdevice=%d iStartMode=%d iTimeOut=%d iFlags=0x%x}\n",
				i, list[i].iDevice, list[i].iSubdevice, list[i].iStartMode, list[i].iTimeOut, list[i].iFlags);
	}
#endif

	start.start_list = list;
	start.count = count;
	start.flags = iFlags;
	start.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_START, &start);
	if (!err)
	{
		if (start.err_no)
		{
#ifdef LIBMEDEBUG_DEBUG
			for (i = 0; i < count; i++)
			{
				if (list[i].iErrno)
					LIBPDEBUG("ioctl((device=%d subdevice=%d), ME_IO_STREAM_START, ...)->StartList[%d]=%d\n",
						list[i].iDevice, list[i].iSubdevice, i, list[i].iErrno);
			}
#endif
			LIBPWARNING("ioctl(..., ME_IO_STREAM_START,...)=%d\n", start.err_no);
			err = start.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_START,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamStop_Local(void* context, int device, int subdevice, int mode, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_stop_simple_t stream_stop;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iStartMode=%d iTimeout=%d iFlags=0x%x\n",
			local_context->fd, device, subdevice, mode, timeout, iFlags);

	stream_stop.device = device;
	stream_stop.subdevice = subdevice;
	stream_stop.mode = mode;
	stream_stop.time_out = timeout;
	stream_stop.flags = iFlags;
	stream_stop.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_STOP_SIMPLE, &stream_stop);
	if (!err)
	{
		if (stream_stop.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_STOP_SIMPLE,...)=%d\n", device, subdevice, stream_stop.err_no);
			err = stream_stop.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_STOP_SIMPLE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamStopList_Local(void* context, meIOStreamStop_t* list, int count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_stop_t stop;
#ifdef LIBMEDEBUG_DEBUG
	int i;
#endif

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	LIBPDEBUG("fd=%dp pStopList=%p iCount=%d iFlags=0x%x\n",
			local_context->fd, list, count, iFlags);
#ifdef LIBMEDEBUG_DEBUG
	for (i=0; i<count; i++)
	{
		LIBPDEBUG("pStopList[%d]={iDevice=%d iSubdevice=%d iStopMode=%d iFlags=0x%x}\n",
				i, list[i].iDevice, list[i].iSubdevice, list[i].iStopMode, list[i].iFlags);
	}
#endif

	stop.stop_list = list;
	stop.count = count;
	stop.flags = iFlags;
	stop.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_STOP, &stop);
	if (!err)
	{
		if (stop.err_no)
		{
#ifdef LIBMEDEBUG_DEBUG
			for (i = 0; i < count; i++)
			{
				if (list[i].iErrno)
					LIBPDEBUG("ioctl((device=%d subdevice=%d), ME_IO_STREAM_STOP, ...)->StopList[%d]=%d\n",
						list[i].iDevice, list[i].iSubdevice, i, list[i].iErrno);
			}
#endif
			LIBPWARNING("ioctl(..., ME_IO_STREAM_STOP,...)=%d\n", stop.err_no);
			err = stop.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_STOP,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamSetCallbacks_Local(void* context,
							int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	int err_ret = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d dubdevice=%d start=%p start_context=%p new_values=%p new_value_context=%p end=%p end_context=%p iFlags=0x%x",
			device, subdevice, start, start_context, new_values, new_value_context, end, end_context, iFlags);

	if (iFlags)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (new_values)
	{	// create
		err = doCreateThread_Local(local_context, device, subdevice, streamNewValuesThread_Local, new_values, new_value_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (start)
	{	// create
		err = doCreateThread_Local(local_context, device, subdevice, streamStartThread_Local, start, start_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (end)
	{	// create
		err = doCreateThread_Local(local_context, device, subdevice, streamStopThread_Local, end, end_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (!new_values && !start && !end)
	{	// cancel
		err_ret = doDestroyThread_Local(local_context, device, subdevice);
	}

	return err_ret;
}

int StreamNewValues_Local(void* context, int device, int subdevice, int timeout, int* count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_new_values_t status;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(count);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iTimeOut=%d piCount=%p iFlags=0x%x\n",
		local_context->fd, device, subdevice, timeout, count, iFlags);

	status.device = device;
	status.subdevice = subdevice;
	status.time_out = timeout;
	status.flags = iFlags;
	status.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_NEW_VALUES, &status);
	if (!err)
	{
		*count = status.count;

		if (status.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_NEW_VALUES,...)=%d\n", device, subdevice, status.err_no);
			err = status.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_NEW_VALUES,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamRead_Local(void* context,  int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_timeout_read_t read;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(values);
	CHECK_POINTER(count);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iReadMode=%d piValues=%p piCount=%p iFlags=0x%x\n",
			local_context->fd, device, subdevice, mode, values, count, iFlags);

	read.device = device;
	read.subdevice = subdevice;
	read.read_mode = mode;
	read.values = values;
	read.count = *count;
	read.timeout = timeout;
	read.flags = iFlags;
	read.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_TIMEOUT_READ, &read);
	if (!err)
	{
		*count = read.count;

		if (read.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_READ,...)=%d\n", device, subdevice, read.err_no);
			err = read.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_TIMEOUT_READ,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamWrite_Local(void* context, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_timeout_write_t write;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(values);
	CHECK_POINTER(count);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iWriteMode=%d piValues=%p piCount=%p iFlags=0x%x\n",
			local_context->fd, device, subdevice, mode, values, count, iFlags);

	LIBPDEBUG("Values count=%d",*count);

	write.device = device;
	write.subdevice = subdevice;
	write.write_mode = mode;
	write.values = values;
	write.count = *count;
	write.timeout = timeout;
	write.flags = iFlags;
	write.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_TIMEOUT_WRITE, &write);
	if (!err)
	{
		*count = write.count;

		if (write.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_WRITE,...)=%d\n", device, subdevice, write.err_no);
			err = write.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_TIMEOUT_WRITE,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamStatus_Local(void* context, int device, int subdevice, int wait, int* status, int* count, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_io_stream_status_t stream_status;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(status);
	CHECK_POINTER(count);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iWait=%d piStatus=%p piCount=%p iFlags=0x%x\n",
			local_context->fd, device, subdevice, wait, status, count, iFlags);

	stream_status.device = device;
	stream_status.subdevice = subdevice;
	stream_status.wait = wait;
	stream_status.flags = iFlags;
	stream_status.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_IO_STREAM_STATUS, &stream_status);
	if (!err)
	{
		*status = stream_status.status;
		*count = stream_status.count;

		if (stream_status.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_IO_STREAM_STATUS,...)=%d\n", device, subdevice, stream_status.err_no);
			err = stream_status.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_IO_STREAM_STATUS,...)=%d\n", local_context->fd, err);
	}

	return err;
}

int StreamTimeToTicks_Local(void* context, int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags)
{
	int err;
	int base;
	int min_ticks_low, min_ticks_high;
	double min_ticks;
	int max_ticks_low, max_ticks_high;
	double max_ticks;

	uint64_t ticks;

	err = QuerySubdeviceTimer_Local(context, device, subdevice, timer, &base, &min_ticks_low, &min_ticks_high, &max_ticks_low, &max_ticks_high, iFlags);
	if (!err)
	{
		min_ticks = (double)((uint64_t)min_ticks_low + ((uint64_t)min_ticks_high << 32));
		max_ticks = (double)((uint64_t)max_ticks_low + ((uint64_t)max_ticks_high << 32));

		if ((max_ticks != 0) && (min_ticks != 0) && (base != 0))
		{
			if (*stream_time < (min_ticks / (double)base))
			{
				*stream_time = min_ticks / (double)base;
				*ticks_low = min_ticks_low;
				*ticks_high = min_ticks_high;
			}
			else if (*stream_time > (max_ticks / (double)base))
			{
				*stream_time = max_ticks / (double)base;
				*ticks_low = max_ticks_low;
				*ticks_high = max_ticks_high;
			}
			else
			{
				ticks = *stream_time * base + 0.5;
				*ticks_low = ticks;
				*ticks_high = ticks >> 32;
				*stream_time = ticks / (double)base;
			}
		}
		else
		{
			*stream_time = HUGE_VAL;
			*ticks_low = 0;
			*ticks_high = 0;
		}
	}
	else
	{
		*stream_time = HUGE_VAL;
		*ticks_low = 0;
		*ticks_high = 0;
	}

	return err;
}

int StreamFrequencyToTicks_Local(void* context, int device, int subdevice, int timer, double* frequency, int *ticks_low, int *ticks_high, int iFlags)
{
	int err;
	int base;
	int min_ticks_low, min_ticks_high;
	double min_ticks = 0;
	int max_ticks_low, max_ticks_high;
	double max_ticks = 0;

	uint64_t ticks = 0;

	err = QuerySubdeviceTimer_Local(context, device, subdevice, timer, &base, &min_ticks_low, &min_ticks_high, &max_ticks_low, &max_ticks_high, iFlags);

	*ticks_low = 0;
	*ticks_high = 0;

	if (!err)
	{
		min_ticks = (double)min_ticks_low + (double)((uint64_t)min_ticks_high << 32);
		max_ticks = (double)max_ticks_low + (double)((uint64_t)max_ticks_high << 32);

		if ((max_ticks != 0) && (min_ticks != 0) && (base != 0))
		{
			if ((*frequency == 0) || (*frequency > ((double)base / min_ticks)))
			{
				*frequency = (double)base / min_ticks;
				*ticks_low = min_ticks_low;
				*ticks_high = min_ticks_high;
			}
			else if (*frequency < ((double)base / max_ticks))
			{
				*frequency = (double)base / max_ticks;
				*ticks_low = max_ticks_low;
				*ticks_high = max_ticks_high;
			}
			else
			{
				ticks = base / *frequency + 0.5;
				*ticks_low = ticks;
				*ticks_high = ticks >> 32;
				*frequency = (double)base / ticks;
			}
		}
		else
		{
			*frequency = HUGE_VAL;
		}
	}
	else
	{
		*frequency = 0;
	}

	return err;
}

int ParametersSet_Local(void* context, int device, me_extra_param_set_t* paramset, int flags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_extra_param_set_t configset;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(paramset);

	LIBPDEBUG("fd=%d paramset=%p\n", local_context->fd, paramset);

	configset.device = device;
	configset.flags = flags;
	configset.size = paramset->size;
	configset.arg = paramset->arg;
	configset.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_CONFIG_LOAD, &configset);
	if (!err)
	{
		if (configset.err_no)
		{
			LIBPWARNING("ioctl(ME_CONFIG_LOAD,...)=%d\n", configset.err_no);
			err = configset.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_CONFIG_LOAD,...)=%d\n", local_context->fd, err);
	}

	return err;
}


int SetOffset_Local(void* context, int device, int subdevice, int channel, int range, double* offset, int iFlags)
{
	me_local_context_t* local_context = (me_local_context_t *)context;
	int err;
	me_set_offset_t set_offset;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	LIBPDEBUG("fd=%d iDevice=%d iSubdevice=%d iChannel=%d iRange=%d pdOffset=%lf iFlags=0x%x\n",
			local_context->fd, device, subdevice, channel, range, *offset, iFlags);

	set_offset.device = device;
	set_offset.subdevice = subdevice;
	set_offset.channel = channel;
	set_offset.range = range;
	set_offset.offset = (int)(*offset * 1000000.0);
	set_offset.flags = iFlags;
	set_offset.err_no = ME_ERRNO_SUCCESS;

	err = ioctl(local_context->fd, ME_SET_OFFSET, &set_offset);
	if (!err)
	{
		*offset = ((double)set_offset.offset) / 1000000.0;

		if (set_offset.err_no)
		{
			LIBPWARNING("ioctl((iDevice=%d, iSubdevice=%d), ME_SET_OFFSET,...)=%d\n", device, subdevice, set_offset.err_no);
			err = set_offset.err_no;
		}
	}
	else
	{
		LIBPERROR("ioctl(%d, ME_SET_OFFSET,...)=%d\n", local_context->fd, err);
	}

	return err;
}


// Local threads
static int doCreateThread_Local(me_local_context_t* local_context, int device, int subdevice, void* fnThread, void* fnCB, void* contextCB, int iFlags)
{
	threadContext_t* threadArgs;
	threadsList_t* newThread;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	threadArgs = (threadContext_t *)calloc(1, sizeof(threadContext_t));
	if (!threadArgs)
	{
		LIBPERROR("Can not get requestet memory for new thread's arguments.\n");
		return -ENOMEM;
	}

	newThread = (threadsList_t *)calloc(1, sizeof(threadsList_t));
	if (!newThread)
	{
		free (threadArgs);
		LIBPERROR("Can not get requestet memory for new thread.\n");
		return -ENOMEM;
	}

	pthread_mutex_lock(&local_context->callbackContextMutex);
		newThread->device = device;
		newThread->subdevice = subdevice;
		newThread->context = local_context;
		newThread->cancel = 0;

		threadArgs->instance = newThread;
		threadArgs->fnCB = fnCB;
		threadArgs->contextCB = contextCB;
		threadArgs->flags = iFlags;

		if (pthread_create(&newThread->threadID, NULL, fnThread, threadArgs))
		{
			LIBPERROR("device[%d,%d]=>> CREATING THREAD FAILED\n", device, subdevice);
			err = ME_ERRNO_START_THREAD;
			free (newThread);
			free (threadArgs);
		}
		else
		{
			LIBPERROR("device[%d,%d]=>> THREAD CREATED (ID:%lld)\n", device, subdevice, (long long)newThread->threadID);
			pthread_detach(newThread->threadID);

			newThread->next = local_context->activeThreads;
			local_context->activeThreads = newThread;
		}
	pthread_mutex_unlock(&local_context->callbackContextMutex);

	return err;
}

static int doDestroyAllThreads_Local(me_local_context_t* local_context)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	return doDestroyThreads_Local(local_context, -1);
}

static int doDestroyThreads_Local(me_local_context_t* local_context, int device)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	return doDestroyThread_Local(local_context, device, -1);
}

static int doDestroyThread_Local(me_local_context_t* local_context, int device, int subdevice)
{
	threadsList_t**	activeThread = &local_context->activeThreads;
	threadsList_t*	deleteThread;
	pthread_t		selfID;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	selfID = pthread_self();
	pthread_mutex_lock(&local_context->callbackContextMutex);
		while (*activeThread)
		{
			if ((device < 0) || (((*activeThread)->device == device) && ((subdevice < 0) || ((*activeThread)->subdevice == subdevice))))
			{
				deleteThread = (*activeThread);

				*activeThread = deleteThread->next;
				if (pthread_equal(deleteThread->threadID, selfID))
				{	// I'm killing yourself. Lord, forgive me, please.
					deleteThread->cancel = 1;
					LIBPDEBUG("killing yourself selfID=%ld\n", selfID);
				}
				else
				{
					deleteThread->cancel = 2;
					pthread_cancel(deleteThread->threadID);
					LIBPDEBUG("killing thread=%ld selfID=%ld\n", deleteThread->threadID, selfID);
					free (deleteThread);
				}
			}
			else
			{
				activeThread = &((*activeThread)->next);
			}
		}
	pthread_mutex_unlock(&local_context->callbackContextMutex);

	return ME_ERRNO_SUCCESS;
}

static void* irqThread_Local(void* arg)
{
	me_local_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;

	int irq_count = 0;
	int value = 0;

	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.irqCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		err = IrqWait_Local(local_context, context->device, context->subdevice, 0, &irq_count, &value, 100, threadArgs.flags);
		pthread_testcancel();
		if (context->cancel)
			break;

		if (err == ME_ERRNO_TIMEOUT)
		{// Give chance to destroyer.
			continue;
		}
		/// Interrupt or STOP/RESET -> call callback function.
		if (threadArgs.irqCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
			if (threadArgs.irqCB(context->device, context->subdevice, 0, irq_count, value, threadArgs.contextCB, err))
			{
				if (context->cancel)
					break;

				if (!err)
				{/// Interrupt ONLY.
					IrqStop_Local(local_context, context->device, context->subdevice, 0, ME_IO_IRQ_STOP_NO_FLAGS);
				}
			}
			pthread_mutex_unlock(&local_context->callbackContextMutex);

			if (context->cancel)
				break;
		}
	} // while()

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);
	return NULL;
}

static void* streamStartThread_Local(void* arg)
{
	me_local_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;

	int streamStatus;
	int value = 0;

	static int old_err = ME_ERRNO_INVALID_ERROR_NUMBER;
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		err = StreamStatus_Local(local_context, context->device, context->subdevice, ME_WAIT_START, &streamStatus, &value, ME_IO_STREAM_STATUS_NO_FLAGS);
		pthread_testcancel();
		if (context->cancel)
		{
			break;
		}

		if (err && (err == old_err))
		{
			// Do not report the same error twice.
			continue;
		}

		old_err = err;

		/// Start or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				if (threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err))
				{
					if (context->cancel)
					{
						break;
					}

					if (!err)
					{/// Start ONLY.
						StreamStop_Local(local_context, context->device, context->subdevice, 0, 0, ME_IO_STREAM_STOP_NO_FLAGS);
					}
				}
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
			{
				break;
			}
		}
	} // while()

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);
	return NULL;
}

static void* streamStopThread_Local(void* arg)
{
	me_local_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;

	int streamStatus;
	int value = 0;
	static int old_err = ME_ERRNO_INVALID_ERROR_NUMBER;

	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		err = StreamStatus_Local(local_context, context->device, context->subdevice, ME_WAIT_STOP, &streamStatus, &value, ME_IO_STREAM_STATUS_NO_FLAGS);
		pthread_testcancel();
		if (context->cancel)
		{
			break;
		}

		if (err && (err == old_err))
		{
			// Do not report the same error twice.
			continue;
		}

		old_err = err;

		/// Stop or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err);
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
			{
				break;
			}
		}
	} // while()

	if (context->cancel == 1)
	{
		free (context);
	}
	pthread_exit(NULL);
	return NULL;
}

static void* streamNewValuesThread_Local(void* arg)
{
	me_local_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;

	int value = 0;

	int ret;
	int err = ME_ERRNO_SUCCESS;
	int flag;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		flag = ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG;
		if (err && (err != ME_ERRNO_TIMEOUT))
		{
			flag |= ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG;
		}

		err = StreamNewValues_Local(local_context, context->device, context->subdevice, 0, &value, flag);
		pthread_testcancel();
		if (context->cancel)
			break;

		if (!err && !value)
			continue;

		/// New values or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				ret = threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err);
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
				break;

			if (!ret)
			{
				StreamStop_Local(local_context, context->device, context->subdevice, 0, 0, ME_IO_STREAM_STOP_NO_FLAGS);
			}
		}
	} // while()

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);
	return NULL;
}
