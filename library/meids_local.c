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

# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <syslog.h>

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
# include "meids_config_structs.h"
# include "meids_structs.h"

# include "meids_local_calls.h"
# include "meids_local_config.h"

# include "meids_vrt.h"

# include "meids_local.h"


static int local_init(void);
static int local_init_calltable(meids_calls_t** context_calls);

static int local_OpenDriver(const char* address, me_config_t** new_driver, me_local_context_t** new_context, int iFlags);
static int local_CloseDriver(void* context, int iFlags);
static int local_LockDriver(me_local_context_t* context, int lock, int iFlags);

// Global variables - keep context and necessary references.
static me_context_list_t*			Loc_Context;
static me_config_t*					Loc_Config;
static me_config_t*					Loc_Config_Raw;
// static me_config_shortcut_table_t* Local_Table;

static meids_calls_t* Loc_Calls;

/// Init and exit of the shared object
void __attribute__((constructor)) meids_local_init(void);
void __attribute__((destructor)) meids_local_fini(void);

/// Private section
void meids_local_init(void)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	if(local_init())
		LIBPERROR("INIT FAILED! \n");
}

void meids_local_fini(void)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	ME_Close(ME_CLOSE_NO_FLAGS);

	free(Loc_Config);
	free(Loc_Context);
	free(Loc_Calls);
}

static int local_init(void)
{
	int err = ME_ERRNO_SUCCESS;
	int cnx_err;
	int cfg_err;
	int cfg_raw_err;
	int loc_err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	cfg_raw_err = config_list_init(&Loc_Config_Raw);
	cfg_err = config_list_init(&Loc_Config);
	cnx_err = context_list_init(&Loc_Context);
	err = context_list_init(&Loc_Context);
	loc_err = local_init_calltable(&Loc_Calls);

	if (cnx_err || cfg_raw_err  || cfg_err || loc_err)
	{
		LIBPCRITICALERROR("Can not initialize library!\n");

		if (Loc_Context)
			free(Loc_Context);

		if (Loc_Config_Raw)
			free(Loc_Config_Raw);

		if (Loc_Config)
			free(Loc_Config);

		if (Loc_Calls)
			free(Loc_Calls);

		err = -ENOMEM;
	}

	return err;
}

static int local_OpenDriver(const char* address, me_config_t** new_driver, me_local_context_t** new_context, int iFlags)
{
	me_config_t* cfg = NULL;
	me_local_context_t* context = NULL;

	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!new_driver)
		return ME_ERRNO_INVALID_POINTER;
	if (!new_context)
		return ME_ERRNO_INVALID_POINTER;

	context = calloc(1, sizeof(me_local_context_t));
	if (!context)
	{
		err = -ENOMEM;
		goto ERROR;
	}
	context->context_type = me_context_type_local;
	context->context_calls = Loc_Calls;
	context->fd = -1;

	cfg = calloc(1, sizeof(me_config_t));
	if (!cfg)
	{
		err = -ENOMEM;
		goto ERROR;
	}
	cfg->device_list = NULL;
	cfg->device_list_count = 0;

	err = Open_Local(context, address, ME_OPEN_NO_FLAGS);
	if (!err)
	{

		ConfigRead_Local(context, cfg, ME_VALUE_NOT_USED);
		ConfigEnumerate(cfg, 0, ME_VALUE_NOT_USED);

		*new_driver = cfg;
		*new_context = context;
	}
	else
	{
		*new_driver = NULL;
		*new_context = NULL;
	}

ERROR:
	if (err)
	{
		if (cfg)
		{
			free(cfg);
			cfg = NULL;
		}

		if (context)
		{
			free(context);
			context = NULL;
		}
	}

	return err;
}

static int local_CloseDriver(void* context, int iFlags)
{

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	return Close_Local(context, ME_CLOSE_NO_FLAGS);
}

static int local_LockDriver(me_local_context_t* context, int lock, int iFlags)
{
 	LIBPINFO("executed: %s\n", __FUNCTION__);

	return LockDriver_Local(context, lock, iFlags);
}


/// Protected section
int ME_local_OpenDriver(const char* address, me_config_t** new_driver, void** new_context, int iFlags)
{
	me_config_t* cfg = NULL;
	me_local_context_t* context_local;

	int err = ME_ERRNO_OPEN;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!address)
	{
		return err;
	}

	if (iFlags == ME_OPEN_NO_FLAGS)
		iFlags = ME_OPEN_ALL;

	if (iFlags & ME_OPEN_LOCAL)
	{
		err = local_OpenDriver(address, &cfg, &context_local, iFlags & ME_OPEN_LOCAL);
		if (!err)
		{
			*new_driver = cfg;
			*new_context = context_local;
		}
	}

	if (err)
	{
		*new_driver = NULL;
		*new_context = NULL;
	}

	return err;
}

int ME_local_CloseDriver(void* context, int iFlags)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	switch (((me_dummy_context_t *)context)->context_type)
	{
		case me_context_type_local:
			err = local_CloseDriver(context, iFlags);
			break;

		default:
			LIBPERROR("Wrong context type. context_type=%d\n", ((me_dummy_context_t *)context)->context_type);
			err = ME_ERRNO_CLOSE;
	}

	return err;
}

int ME_local_LockDriver(void* context, int lock, int iFlags)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	switch (((me_dummy_context_t*)context)->context_type)
	{
		case me_context_type_local:		//PCI and USB
			err =  local_LockDriver((me_local_context_t*)context, lock, iFlags);
			break;

		default:
			err = ME_ERRNO_INVALID_LOCK;
	}
 	return err;
}


/// Public section
// Access
int ME_Open(char* address, int iFlags)
{
	me_config_t* cfg;
	me_config_t* new_global_cfg;

	void* context = NULL;
	int err = ME_ERRNO_SUCCESS;
	int err_PCI = ME_ERRNO_OPEN;
	int err_USB = ME_ERRNO_OPEN;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (address)
	{
		err = ME_local_OpenDriver(address, &cfg, &context, iFlags);
		if (!err)
		{
			new_global_cfg = NULL;
			ConfigJoin(Loc_Config, cfg, &new_global_cfg, ME_VALUE_NOT_USED);
			ConfigClean(Loc_Config, ME_VALUE_NOT_USED);
			ConfigClean(cfg, ME_VALUE_NOT_USED);
			if (cfg)
			{
				free(cfg);
				cfg = NULL;
			}

			Loc_Config->device_list_count = new_global_cfg->device_list_count;
			Loc_Config->device_list = new_global_cfg->device_list;
			if (new_global_cfg)
			{
				free(new_global_cfg);
				new_global_cfg = NULL;
			}

			ConfigEnumerate(Loc_Config, 0, ME_VALUE_NOT_USED);
			ContextAppend(context, Loc_Context);
		}
		LIBPDEBUG("Open(%s)=%d.\n", address, err);
	}
	else
	{
		if (!iFlags || (iFlags & ME_OPEN_PCI))
		{
			err_PCI = ME_local_OpenDriver("/dev/medriverPCI", &cfg, &context, ME_OPEN_PCI);
			if (!err_PCI)
			{
				new_global_cfg = NULL;
				ConfigJoin(Loc_Config, cfg, &new_global_cfg, ME_VALUE_NOT_USED);
				ConfigClean(Loc_Config, ME_VALUE_NOT_USED);
				ConfigClean(cfg, ME_VALUE_NOT_USED);
				if (cfg)
				{
					free(cfg);
					cfg = NULL;
				}

				Loc_Config->device_list_count = new_global_cfg->device_list_count;
				Loc_Config->device_list = new_global_cfg->device_list;
				if (new_global_cfg)
				{
					free(new_global_cfg);
					new_global_cfg = NULL;
				}

				ConfigEnumerate(Loc_Config, 0, ME_VALUE_NOT_USED);
				ContextAppend(context, Loc_Context);
			}
			LIBPDEBUG("Open(%s)=%d.\n", "/dev/medriverPCI", err_PCI);
		}

		if (!iFlags || (iFlags & ME_OPEN_USB))
		{
			err_USB = ME_local_OpenDriver("/dev/medriverUSB", &cfg, &context, ME_OPEN_USB);
			if (!err_PCI)
			{
				new_global_cfg = NULL;
				ConfigJoin(Loc_Config, cfg, &new_global_cfg, ME_VALUE_NOT_USED);
				ConfigClean(Loc_Config, ME_VALUE_NOT_USED);
				ConfigClean(cfg, ME_VALUE_NOT_USED);
				if (cfg)
				{
					free(cfg);
					cfg = NULL;
				}

				Loc_Config->device_list_count = new_global_cfg->device_list_count;
				Loc_Config->device_list = new_global_cfg->device_list;
				if (new_global_cfg)
				{
					free(new_global_cfg);
					new_global_cfg = NULL;
				}

				ConfigEnumerate(Loc_Config, 0, ME_VALUE_NOT_USED);
				ContextAppend(context, Loc_Context);
			}
			LIBPDEBUG("Open(%s)=%d.\n", "/dev/medriverUSB", err_USB);
		}

		err = (err_PCI && err_USB) ? ME_ERRNO_NOT_OPEN : ME_ERRNO_SUCCESS;
	}

	return err;
}

int ME_Close(int iFlags)
{
	int i = 0;
	int err = ME_ERRNO_SUCCESS;

	me_cfg_device_entry_t** device_list = Loc_Config->device_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (device_list)
	{
		for (i=0; i<Loc_Config->device_list_count; i++, device_list++)
		{
			if ((*device_list)->plugged != me_plugged_type_IN)
				continue;

			switch ((*device_list)->access_type)
			{
				case me_access_type_PCI:						//Local PCI and ePCI boards
				case me_access_type_USB:						//Synapse-USB & Mephisto-Family
					err = ME_local_CloseDriver((*device_list)->context, iFlags);
					break;

				case me_access_type_USB_MephistoScope:			//Mephisto-Scope
					LIBPDEBUG("NOT IMPLEMENTED!\n");
					break;

				default:
					LIBPERROR("Wrong access type. pos:%d device_entry->access_type=%d\n", i, (*device_list)->access_type);
					err = ME_ERRNO_CLOSE;
			}
		}
	}

	ConfigClean(Loc_Config, ME_VALUE_NOT_USED);
	ContextClean(&Loc_Context);

	return err;
}

int ME_QueryDevicesNumber(int* no_devices, int iFlags)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(no_devices);

	return ConfigMaxNumber(Loc_Config, no_devices, ME_VALUE_NOT_USED);
}

int ME_QueryLibraryVersion(int* version, int iFlags)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(version);

	*version = MEIDS_VERSION_LIBRARY;

	return ME_ERRNO_SUCCESS;
}

int ME_ConfigRead(me_config_t* cfg, const char* address, int flags)
{
	me_local_context_t* context;
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(cfg);

	context = calloc(1, sizeof(me_local_context_t));
	if (!context)
	{
		err = -ENOMEM;
		goto ERROR;
	}
	context->context_type = me_context_type_local;
	context->context_calls = Loc_Calls;
	context->fd = -1;

	err = Open_Local(context, address, ME_OPEN_NO_FLAGS);
	if (!err)
	{
		err = ConfigRead_Local(context, cfg, flags);
		Close_Local(context, ME_CLOSE_NO_FLAGS);
	}

ERROR:

	if (context)
	{
		free(context);
		context = NULL;
	}

	return err;
}

//Lock
int ME_LockAll(int lock, int iFlags)
{
	me_context_list_t* context_base;
	int err = ME_ERRNO_SUCCESS;

	if (!Loc_Context->context)
		return ME_ERRNO_NOT_OPEN;

	if (!(iFlags & ME_LOCK_FORCE))
	{
		// There are more than just one driver. First checking locks.
		if (lock != ME_LOCK_CHECK)
		{
			context_base = Loc_Context;
			while (context_base->next)
			{
				err = ME_virtual_LockDriver(context_base->context, ME_LOCK_CHECK, iFlags);
				if (err)
				{
					break;
				}
				context_base = context_base->next;
			}
		}
	}

	if (!err)
	{
		context_base = Loc_Context;
		while (context_base->next)
		{
			err = ME_virtual_LockDriver(context_base->context, lock, iFlags);
			if (err)
			{
				break;
			}
			context_base = context_base->next;
		}
	}

	if (err)
	{// Nasty clash. Other task locked resources in time between CHECK and SET. Revert operation.
	/// @todo Implement it.
		if (lock == ME_LOCK_SET)
		{
			context_base = Loc_Context;
			while (context_base->next)
			{
				err = ME_virtual_LockDriver(context_base->context, ME_LOCK_RELEASE, ME_LOCK_PRESERVE);
				context_base = context_base->next;
			}
		}
	}

	return err;
}

int ME_LockDevice(int device, int lock, int iFlags)
{
	return ME_virtual_LockDevice(Loc_Config, device, lock, iFlags);
}

int ME_LockSubdevice(int device, int subdevice, int lock, int iFlags)
{
	return ME_virtual_LockSubdevice(Loc_Config, device, subdevice, lock, iFlags);
}


//Query
int ME_QueryDriverVersion(int device, int* version, int iFlags)
{
	return ME_virtual_QueryDriverVersion(Loc_Config, device, version, iFlags);
}

int ME_QueryDriverName(int device, char* name, int count, int iFlags)
{
	return ME_virtual_QueryDriverName(Loc_Config, device, name, count, iFlags);
}


int ME_QuerySubdriverVersion(int device, int* version, int iFlags)
{
	return ME_virtual_QuerySubdriverVersion(Loc_Config, device, version, iFlags);
}

int ME_QuerySubdriverName(int device, char* name, int count, int iFlags)
{
	return ME_virtual_QuerySubdriverName(Loc_Config, device, name, count, iFlags);
}


int ME_QueryDeviceName(int device, char* name, int count, int iFlags)
{
	return ME_virtual_QueryDeviceName(Loc_Config, device, name, count, iFlags);
}

int ME_QueryDeviceDescription(int device, char* description, int count, int iFlags)
{
	return ME_virtual_QueryDeviceDescription(Loc_Config, device, description, count, iFlags);
}

int ME_QueryDeviceInfo(int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags)
{
	return ME_virtual_QueryDeviceInfo(Loc_Config, device, vendor_id, device_id, serial_no, bus_type, bus_no, dev_no, func_no, plugged, iFlags);
}


int ME_QuerySubdevicesNumber(int device, int* no_subdevice, int iFlags)
{
	return ME_virtual_QuerySubdevicesNumber(Loc_Config, device, no_subdevice, iFlags);
}

int ME_QuerySubdevicesNumberByType(int device, int type, int subtype, int* no_subdevice, int iFlags)
{
	return ME_virtual_QuerySubdevicesNumberByType(Loc_Config, device, type, subtype, no_subdevice, iFlags);
}

int ME_QuerySubdeviceType(int device, int subdevice, int* type, int* subtype, int iFlags)
{
	return ME_virtual_QuerySubdeviceType(Loc_Config, device, subdevice, type, subtype, iFlags);
}

int ME_QuerySubdeviceByType(int device, int subdevice, int type, int subtype, int* result, int iFlags)
{
	return ME_virtual_QuerySubdeviceByType(Loc_Config, device, subdevice, type, subtype, result, iFlags);
}

int ME_QuerySubdeviceCaps(int device, int subdevice, int* caps, int iFlags)
{
	return ME_virtual_QuerySubdeviceCaps(Loc_Config, device, subdevice, caps, iFlags);
}

int ME_QuerySubdeviceCapsArgs(int device, int subdevice, int cap, int* args, int count, int iFlags)
{
	return ME_virtual_QuerySubdeviceCapsArgs(Loc_Config, device, subdevice, cap, args, count, iFlags);
}


int ME_QueryChannelsNumber(int device, int subdevice, unsigned int* number, int iFlags)
{
	return ME_virtual_QueryChannelsNumber(Loc_Config, device, subdevice, number, iFlags);
}


int ME_QueryRangesNumber(int device, int subdevice, int unit, int* no_ranges, int iFlags)
{
	return ME_virtual_QueryRangesNumber(Loc_Config, device, subdevice, unit, no_ranges, iFlags);
}

int ME_QueryRangeInfo(int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data, int iFlags)
{
	return ME_virtual_QueryRangeInfo(Loc_Config, device, subdevice, range, unit, min, max, max_data, iFlags);
}

int ME_QueryRangeByMinMax(int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags)
{
	return ME_virtual_QueryRangeByMinMax(Loc_Config, device, subdevice, unit, min, max, max_data, range, iFlags);
}

int ME_QuerySubdeviceTimer(int device, int subdevice,
									int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags)
{
	return ME_virtual_QuerySubdeviceTimer(Loc_Config, device, subdevice, timer, base, min_ticks_low, min_ticks_high, max_ticks_low, max_ticks_high, iFlags);
}


//Input/Output
int ME_IrqStart(int device, int subdevice, int channel, int source, int edge, int arg, int iFlags)
{
	return ME_virtual_IrqStart(Loc_Config, device, subdevice, channel, source, edge, arg, iFlags);
}

int ME_IrqWait(int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags)
{
	return ME_virtual_IrqWait(Loc_Config, device, subdevice, channel, count, value, timeout, iFlags);
}

int ME_IrqStop(int device, int subdevice, int channel, int iFlags)
{
	return ME_virtual_IrqStop(Loc_Config, device, subdevice, channel, iFlags);
}

int ME_IrqTest(int device, int subdevice, int channel, int iFlags)
{
	return ME_virtual_IrqTest(Loc_Config, device, subdevice, channel, iFlags);
}

int  ME_IrqSetCallback(int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags)
{
	return ME_virtual_IrqSetCallback(Loc_Config, device, subdevice, irq_fn, irq_context, iFlags);
}


int ME_ResetDevice(int device, int iFlags)
{
	return ME_virtual_ResetDevice(Loc_Config, device, iFlags);
}

int ME_ResetSubdevice(int device, int subdevice, int iFlags)
{
	return ME_virtual_ResetSubdevice(Loc_Config, device, subdevice, iFlags);
}


int ME_SingleConfig(int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge, int iFlags)
{
	return ME_virtual_SingleConfig(Loc_Config, device, subdevice, channel, config, reference, synchro, trigger, edge, iFlags);
}

int ME_Single(int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags)
{
	return ME_virtual_Single(Loc_Config, device, subdevice, channel, direction, value, timeout, iFlags);
}

int ME_SingleList(meIOSingle_t* list, int count, int iFlags)
{
	return ME_virtual_SingleList(Loc_Config, list, count, iFlags);
}


int ME_StreamConfig(int device, int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags)
{
	return ME_virtual_StreamConfig(Loc_Config, device, subdevice, list, count, trigger, threshold, iFlags);
}

int ME_StreamConfigure(int device, int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags)
{
	return ME_virtual_StreamConfigure(Loc_Config, device, subdevice, list, count, trigger, threshold, iFlags);
}

int ME_StreamNewValues(int device, int subdevice, int timeout, int* count, int iFlags)
{
	return ME_virtual_StreamNewValues(Loc_Config, device, subdevice, timeout, count, iFlags);
}

int ME_StreamRead(int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	return ME_virtual_StreamRead(Loc_Config, device, subdevice, mode, values, count, timeout, iFlags);
}

int ME_StreamWrite(int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	return ME_virtual_StreamWrite(Loc_Config, device, subdevice, mode, values, count, timeout, iFlags);
}

int ME_StreamStart(int device, int subdevice, int mode, int timeout, int iFlags)
{
	return ME_virtual_StreamStart(Loc_Config, device, subdevice, mode, timeout, iFlags);
}

int ME_StreamStartList(meIOStreamStart_t* list, int count, int iFlags)
{
	return ME_virtual_StreamStartList(Loc_Config, list, count, iFlags);
}

int ME_StreamStatus(int device, int subdevice, int wait, int* status, int* count, int iFlags)
{
	return ME_virtual_StreamStatus(Loc_Config, device, subdevice, wait, status, count, iFlags);
}

int ME_StreamStop(int device, int subdevice, int mode, int timeout, int iFlags)
{
	return ME_virtual_StreamStop(Loc_Config, device, subdevice, mode, timeout, iFlags);
}

int ME_StreamStopList(meIOStreamStop_t* list, int count, int iFlags)
{
	return ME_virtual_StreamStopList(Loc_Config, list, count, iFlags);
}

int  ME_StreamSetCallbacks(int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags)
{
	return ME_virtual_StreamSetCallbacks(Loc_Config,
							device, subdevice,
							start, start_context,
							new_values, new_value_context,
							end, end_context,
							iFlags);
}

int ME_StreamTimeToTicks(int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags)
{
	return ME_virtual_StreamTimeToTicks(Loc_Config, device, subdevice, timer, stream_time, ticks_low, ticks_high, iFlags);
}

int ME_StreamFrequencyToTicks(int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags)
{
	return ME_virtual_StreamFrequencyToTicks(Loc_Config, device, subdevice, timer, frequency, ticks_low, ticks_high, iFlags);
}

int  ME_ParametersSet(me_extra_param_set_t* paramset, int flags)
{
	return ME_virtual_ParametersSet(Loc_Config, paramset, flags);
}

int  ME_SetOffset(int device, int subdevice, int channel, int range, double* offset, int iFlags)
{
	return ME_virtual_SetOffset(Loc_Config, device, subdevice, channel, range, offset, iFlags);
}

void ME_ConfigPrint(void)
{
	LIBPDEBUG("Loc_Config=%p\n", Loc_Config);
	LIBPDEBUG("Loc_Config->device_list_count=%d\n", Loc_Config->device_list_count);
	LIBPDEBUG("Loc_Config->device_list=%p\n", Loc_Config->device_list);
	ConfigPrint(Loc_Config);
}

static int local_init_calltable(meids_calls_t** context_calls)
{
	if (!context_calls)
		return ME_ERRNO_INTERNAL;

	*context_calls = calloc(1, sizeof(meids_calls_t));

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!*context_calls)
		return -ENOMEM;

//Lock
	(*context_calls)->LockDriver				= LockDriver_Local;
	(*context_calls)->LockDevice				= LockDevice_Local;
	(*context_calls)->LockSubdevice				= LockSubdevice_Local;

//Query
	(*context_calls)->QueryDriverVersion		= QueryDriverVersion_Local;
	(*context_calls)->QueryDriverName			= QueryDriverName_Local;

	(*context_calls)->QuerySubdriverVersion		= QuerySubdriverVersion_Local;
	(*context_calls)->QuerySubdriverName		= QuerySubdriverName_Local;

	(*context_calls)->QueryDeviceName			= QueryDeviceName_Local;
	(*context_calls)->QueryDeviceDescription	= QueryDeviceDescription_Local;
	(*context_calls)->QueryDeviceInfo			= QueryDeviceInfo_Local;

	(*context_calls)->QuerySubdevicesNumber		= QuerySubdevicesNumber_Local;
	(*context_calls)->QuerySubdevicesNumberByType= QuerySubdevicesNumberByType_Local;
	(*context_calls)->QuerySubdeviceType		= QuerySubdeviceType_Local;
	(*context_calls)->QuerySubdeviceByType		= QuerySubdeviceByType_Local;
	(*context_calls)->QuerySubdeviceCaps		= QuerySubdeviceCaps_Local;
	(*context_calls)->QuerySubdeviceCapsArgs	= QuerySubdeviceCapsArgs_Local;

	(*context_calls)->QueryChannelsNumber		= QueryChannelsNumber_Local;

	(*context_calls)->QueryRangesNumber			= QueryRangesNumber_Local;
	(*context_calls)->QueryRangeInfo			= QueryRangeInfo_Local;
	(*context_calls)->QueryRangeByMinMax		= QueryRangeByMinMax_Local;
	(*context_calls)->QuerySubdeviceTimer		= QuerySubdeviceTimer_Local;

//Input/Output
	(*context_calls)->IrqStart					= IrqStart_Local;
	(*context_calls)->IrqWait					= IrqWait_Local;
	(*context_calls)->IrqStop					= IrqStop_Local;
	(*context_calls)->IrqTest					= IrqTest_Local;

	(*context_calls)->IrqSetCallback			= IrqSetCallback_Local;

	(*context_calls)->ResetDevice				= ResetDevice_Local;
	(*context_calls)->ResetSubdevice			= ResetSubdevice_Local;

	(*context_calls)->SingleConfig				= SingleConfig_Local;
	(*context_calls)->Single					= Single_Local;
	(*context_calls)->SingleList				= SingleList_Local;

	(*context_calls)->StreamConfig				= StreamConfig_Local;
	(*context_calls)->StreamConfigure			= StreamConfigure_Local;

	(*context_calls)->StreamNewValues			= StreamNewValues_Local;
	(*context_calls)->StreamRead				= StreamRead_Local;
	(*context_calls)->StreamWrite				= StreamWrite_Local;
	(*context_calls)->StreamStart				= StreamStart_Local;
	(*context_calls)->StreamStartList			= StreamStartList_Local;
	(*context_calls)->StreamStatus				= StreamStatus_Local;
	(*context_calls)->StreamStop				= StreamStop_Local;
	(*context_calls)->StreamStopList			= StreamStopList_Local;

	(*context_calls)->StreamSetCallbacks		= StreamSetCallbacks_Local;

	(*context_calls)->StreamTimeToTicks			= StreamTimeToTicks_Local;
	(*context_calls)->StreamFrequencyToTicks	= StreamFrequencyToTicks_Local;

	(*context_calls)->SetOffset					= SetOffset_Local;

	(*context_calls)->ParametersSet				= ParametersSet_Local;
	return 0;
}
