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

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_debug.h"

# include "meids_config.h"
# include "meids_vrt.h"

int  ME_virtual_LockDriver(void* context, int lock, int iFlags)
{
	int err;

	switch (((me_dummy_context_t*)context)->context_type)
	{
		case me_context_type_local:		//PCI and USB
			err =  ((me_local_context_t*)context)->context_calls->LockDriver((me_local_context_t*)context, lock, iFlags);
			break;

		case me_context_type_remote:		//RPC
			err =  ((me_rpc_context_t*)context)->context_calls->LockDriver((me_rpc_context_t*)context, lock, iFlags);
			break;

		default:
			err = ME_ERRNO_INVALID_LOCK;
	}

 	return err;
}

int  ME_virtual_LockDevice(const me_config_t* cfg, int device, int lock, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->LockDevice(cfg_reference->context, cfg_reference->info.device_no, lock, iFlags);
	}

	return err;
}

int  ME_virtual_LockSubdevice(const me_config_t* cfg, int device, int subdevice, int lock, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->LockSubdevice(cfg_reference->context, cfg_reference->info.device_no, subdevice, lock, iFlags);
	}

	return err;
}


//Query
int  ME_virtual_QueryDriverVersion(const me_config_t* cfg, int device, int* version, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryDriverVersion(cfg_reference->context, version, iFlags);
	}

	return err;
}

int  ME_virtual_QueryDriverName(const me_config_t* cfg, int device, char* name, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryDriverName(cfg_reference->context, name, count, iFlags);
	}

	return err;
}


int  ME_virtual_QuerySubdriverVersion(const me_config_t* cfg, int device, int* version, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdriverVersion(cfg_reference->context, cfg_reference->info.device_no,version, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdriverName(const me_config_t* cfg, int device, char* name, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdriverName(cfg_reference->context, cfg_reference->info.device_no,name, count, iFlags);
	}

	return err;
}


int  ME_virtual_QueryDeviceName(const me_config_t* cfg, int device, char* name, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryDeviceName(cfg_reference->context, cfg_reference->info.device_no,name, count, iFlags);
	}
	else if (err == ME_ERRNO_DEVICE_UNPLUGGED)
	{

		strncpy(name, cfg_reference->info.device_name, (count < strlen(cfg_reference->info.device_name)) ? count : strlen(cfg_reference->info.device_name)+1);
		*(name + count - 1) = '\0';
		err = ME_ERRNO_SUCCESS;
	}

	return err;
}

int  ME_virtual_QueryDeviceDescription(const me_config_t* cfg, int device, char* description, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryDeviceDescription(cfg_reference->context, cfg_reference->info.device_no,description, count, iFlags);
	}
	else if (err == ME_ERRNO_DEVICE_UNPLUGGED)
	{

		strncpy(description, cfg_reference->info.device_description, (count < strlen(cfg_reference->info.device_description)) ? count : strlen(cfg_reference->info.device_description)+1);
		*(description + count - 1) = '\0';
		err = ME_ERRNO_SUCCESS;
	}

	return err;
}

int  ME_virtual_QueryDeviceInfo(const me_config_t* cfg, int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryDeviceInfo(cfg_reference->context, cfg_reference->info.device_no,
									vendor_id, device_id, serial_no, bus_type, bus_no, dev_no, func_no, plugged, iFlags);
	}
	else if (err == ME_ERRNO_DEVICE_UNPLUGGED)
	{
		*vendor_id	= cfg_reference->info.vendor_id;
		*device_id	= cfg_reference->info.device_id;
		*serial_no	= cfg_reference->info.serial_no;
		switch (cfg_reference->access_type)
		{
			case me_access_type_PCI:						//Local PCI and ePCI boards
				*bus_type	= ME_BUS_TYPE_PCI;
				break;

			case me_access_type_USB:						//Synapse-USB & Mephisto-Family
			case me_access_type_USB_MephistoScope:		//Mephisto-Scope
				*bus_type	= ME_BUS_TYPE_USB;
				break;

			case me_access_type_TCPIP:					//Synapse-LAN
				*bus_type	= ME_BUS_TYPE_ANY;
				break;

			default:
				*bus_type	= ME_BUS_TYPE_INVALID;

		}
		*bus_no		= -1;
		*dev_no		= -1;
		*func_no	= -1;
		*plugged	= cfg_reference->plugged;

		err = ME_ERRNO_SUCCESS;
	}

	return err;
}


int  ME_virtual_QuerySubdevicesNumber(const me_config_t* cfg, int device, int* no_subdevice, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdevicesNumber(cfg_reference->context, cfg_reference->info.device_no, no_subdevice, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdevicesNumberByType(const me_config_t* cfg, int device, int type, int subtype, int* no_subdevice, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdevicesNumberByType(cfg_reference->context, cfg_reference->info.device_no, type, subtype, no_subdevice, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdeviceType(const me_config_t* cfg, int device, int subdevice, int* type, int* subtype, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdeviceType(cfg_reference->context, cfg_reference->info.device_no, subdevice, type, subtype, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdeviceByType(const me_config_t* cfg, int device, int subdevice, int type, int subtype, int* result, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdeviceByType(cfg_reference->context, cfg_reference->info.device_no, subdevice, type, subtype, result, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdeviceCaps(const me_config_t* cfg, int device, int subdevice, int* caps, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdeviceCaps(cfg_reference->context, cfg_reference->info.device_no, subdevice, caps, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdeviceCapsArgs(const me_config_t* cfg, int device, int subdevice, int cap, int* args, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdeviceCapsArgs(cfg_reference->context, cfg_reference->info.device_no, subdevice, cap, args, count, iFlags);
	}

	return err;
}


int  ME_virtual_QueryChannelsNumber(const me_config_t* cfg, int device, int subdevice, unsigned int* number, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryChannelsNumber(cfg_reference->context, cfg_reference->info.device_no, subdevice, number, iFlags);
	}

	return err;
}


int  ME_virtual_QueryRangesNumber(const me_config_t* cfg, int device, int subdevice, int unit, int* no_ranges, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryRangesNumber(cfg_reference->context, cfg_reference->info.device_no, subdevice, unit, no_ranges, iFlags);
	}

	return err;
}

int  ME_virtual_QueryRangeInfo(const me_config_t* cfg, int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryRangeInfo(cfg_reference->context, cfg_reference->info.device_no, subdevice, range, unit, min, max, max_data, iFlags);
	}

	return err;
}

int  ME_virtual_QueryRangeByMinMax(const me_config_t* cfg, int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QueryRangeByMinMax(cfg_reference->context, cfg_reference->info.device_no, subdevice, unit, min, max, max_data, range, iFlags);
	}

	return err;
}

int  ME_virtual_QuerySubdeviceTimer(const me_config_t* cfg, int device, int subdevice, int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->QuerySubdeviceTimer(cfg_reference->context, cfg_reference->info.device_no, subdevice, timer, base, min_ticks_low, min_ticks_high, max_ticks_low, max_ticks_high, iFlags);
	}

	return err;
}


//Input/Output
int  ME_virtual_IrqStart(const me_config_t* cfg, int device, int subdevice, int channel, int source, int edge, int arg, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->IrqStart(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, source, edge, arg, iFlags);
	}

	return err;
}

int  ME_virtual_IrqWait(const me_config_t* cfg, int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->IrqWait(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, count, value, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_IrqStop(const me_config_t* cfg, int device, int subdevice, int channel, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->IrqStop(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, iFlags);
	}

	return err;
}

int  ME_virtual_IrqTest(const me_config_t* cfg, int device, int subdevice, int channel, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->IrqTest(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, iFlags);
	}

	return err;
}

int  ME_virtual_IrqSetCallback(const me_config_t* cfg, int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->IrqSetCallback(cfg_reference->context, cfg_reference->info.device_no, subdevice, irq_fn, irq_context, iFlags);
	}

	return err;
}


int  ME_virtual_ResetDevice(const me_config_t* cfg, int device, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->ResetDevice(cfg_reference->context, cfg_reference->info.device_no, iFlags);
	}

	return err;
}

int  ME_virtual_ResetSubdevice(const me_config_t* cfg, int device, int subdevice, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->ResetSubdevice(cfg_reference->context, cfg_reference->info.device_no, subdevice, iFlags);
	}

	return err;
}


int  ME_virtual_SingleConfig(const me_config_t* cfg, int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->SingleConfig(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, config, reference, synchro, trigger, edge, iFlags);
	}

	return err;
}

int  ME_virtual_Single(const me_config_t* cfg, int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->Single(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, direction, value, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_SingleList(const me_config_t* cfg, meIOSingle_t* list, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t** reference_list;
	me_cfg_device_entry_t* cfg_reference;
	meIOSingle_t* internal_list;
	meIOSingle_t* pos;
	int list_size;
	int i;

	LIBPDEBUG("pSingleList=%p iCount=%d iFlags=0x%x", list, count, iFlags);

	if (count < 1)
	{
		err = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR_0;
	}

	internal_list = calloc(count, sizeof(meIOSingle_t));
	if (!internal_list)
	{
		err = -ENOMEM;
		goto ERROR_0;
	}

	reference_list = calloc(count, sizeof(me_cfg_device_entry_t*));
	if (!reference_list)
	{
		err =  -ENOMEM;
		goto ERROR_1;
	}

	for (i=0; i<count; i++)
	{
		err = ConfigResolve(cfg, (list + i)->iDevice, reference_list + i);
		if (err)
		{
			if (!(iFlags  & ME_IO_SINGLE_NONBLOCKING))
			{
				break;
			}
		}

		cfg_reference = *(reference_list+i);
		*(internal_list + i) = *(list + i);
		(internal_list + i)->iDevice = cfg_reference->info.device_no;
		(internal_list + i)->iErrno = err;
	}

	if ((!err) || (iFlags  & ME_IO_SINGLE_NONBLOCKING))
	{
		pos = internal_list;
		list_size = 1;
		cfg_reference = *reference_list;

		for (i=1; i<count; i++)
		{
			if (cfg_reference == *(reference_list+i))
			{
				list_size++;
			}
			else
			{
				if (((me_dummy_context_t *)cfg_reference->context))
				{
					err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->SingleList(cfg_reference->context, pos, list_size, iFlags);
					if ((err) && (!(iFlags  & ME_IO_SINGLE_NONBLOCKING)))
					{
						break;
					}
				}
				pos = internal_list + i;
				list_size = 1;
				cfg_reference = *(reference_list + i);
			}
		}

		if ((!err) || (iFlags  & ME_IO_SINGLE_NONBLOCKING))
		{
			if (((me_dummy_context_t *)cfg_reference->context))
			{
				err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->SingleList(cfg_reference->context, pos, list_size, iFlags);
			}
		}
	}

	if (iFlags  & ME_IO_SINGLE_NONBLOCKING)
	{
		err = ME_ERRNO_SUCCESS;
	}

	for (i=0; i<count; i++)
	{
		(list + i)->iErrno = (internal_list + i)->iErrno;
		(list + i)->iValue = (internal_list + i)->iValue;
	}

	if (reference_list)
	{
		free(reference_list);
		reference_list = NULL;
	}
ERROR_1:
	if (internal_list)
	{
		free(internal_list);
		internal_list = NULL;
	}
ERROR_0:

	return err;
}


int  ME_virtual_StreamConfig(const me_config_t* cfg, int device, int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamConfig(cfg_reference->context, cfg_reference->info.device_no, subdevice, list, count, trigger, threshold, iFlags);
	}

	return err;
}

int  ME_virtual_StreamConfigure(const me_config_t* cfg, int device, int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamConfigure(cfg_reference->context, cfg_reference->info.device_no, subdevice, list, count, trigger, threshold, iFlags);
	}

	return err;
}

int  ME_virtual_StreamNewValues(const me_config_t* cfg, int device, int subdevice, int timeout, int* count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamNewValues(cfg_reference->context, cfg_reference->info.device_no, subdevice, timeout, count, iFlags);
	}

	return err;
}

int  ME_virtual_StreamRead(const me_config_t* cfg, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamRead(cfg_reference->context, cfg_reference->info.device_no, subdevice, mode, values, count, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_StreamWrite(const me_config_t* cfg, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamWrite(cfg_reference->context, cfg_reference->info.device_no, subdevice, mode, values, count, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_StreamStart(const me_config_t* cfg, int device, int subdevice, int mode, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStart(cfg_reference->context, cfg_reference->info.device_no, subdevice, mode, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_StreamStartList(const me_config_t* cfg, meIOStreamStart_t* list, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t** reference_list;
	me_cfg_device_entry_t* cfg_reference;
	meIOStreamStart_t* internal_list;
	meIOStreamStart_t* pos;
	int list_size;
	int i;

	LIBPDEBUG("pStartList=%p iCount=%d iFlags=0x%x", list, count, iFlags);

	if (count < 1)
	{
		err = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR_0;
	}

	internal_list = calloc(count, sizeof(meIOStreamStart_t));
	if (!internal_list)
	{
		err = -ENOMEM;
		goto ERROR_0;
	}

	reference_list = calloc(count, sizeof(me_cfg_device_entry_t*));
	if (!reference_list)
	{
		err = -ENOMEM;
		goto ERROR_1;
	}

	for (i=0; i<count; i++)
	{
		err = ConfigResolve(cfg, (list + i)->iDevice, reference_list + i);
		if (err)
		{
			if (!(iFlags  & ME_IO_STREAM_START_NONBLOCKING))
			{
				break;
			}
		}

		cfg_reference = *(reference_list+i);
		LIBPDEBUG("%d: device=%d reference=%p", i, cfg_reference->info.device_no, *(reference_list+i));
		*(internal_list + i) = *(list + i);
		(internal_list + i)->iDevice = cfg_reference->info.device_no;
		(internal_list + i)->iErrno = err;
	}

	if ((!err) || (iFlags  & ME_IO_STREAM_START_NONBLOCKING))
	{
		pos = internal_list;
		list_size = 1;
		cfg_reference = *reference_list;

		for (i=1; i<count; i++)
		{
			LIBPDEBUG("%d: device=%d reference=%p pos=%p", i, cfg_reference->info.device_no, cfg_reference, pos);
			if (cfg_reference == *(reference_list+i))
			{
				list_size++;
			}
			else
			{
				if (((me_dummy_context_t *)cfg_reference->context))
				{
					err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStartList(cfg_reference->context, pos, list_size, iFlags);
					if ((err) && (!(iFlags  & ME_IO_STREAM_START_NONBLOCKING)))
					{
						break;
					}
				}

				pos = internal_list + i;
				list_size = 1;
				cfg_reference = *(reference_list + i);
			}
		}

		if ((!err) || (iFlags  & ME_IO_STREAM_START_NONBLOCKING))
		{
			if (((me_dummy_context_t *)cfg_reference->context))
			{
				err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStartList(cfg_reference->context, pos, list_size, iFlags);
			}
		}
	}

	for (i=0; i<count; i++)
	{
		(list + i)->iErrno = (internal_list + i)->iErrno;
	}

	if (iFlags  & ME_IO_STREAM_START_NONBLOCKING)
	{
		err = ME_ERRNO_SUCCESS;
	}

	if (reference_list)
	{
		free(reference_list);
		reference_list = NULL;
	}
ERROR_1:
	if (internal_list)
	{
		free(internal_list);
		internal_list = NULL;
	}
ERROR_0:

	return err;
}

int  ME_virtual_StreamStatus(const me_config_t* cfg, int device, int subdevice, int wait, int* status, int* count, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStatus(cfg_reference->context, cfg_reference->info.device_no, subdevice, wait, status, count, iFlags);
	}

	return err;
}

int  ME_virtual_StreamStop(const me_config_t* cfg, int device, int subdevice, int mode, int timeout, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStop(cfg_reference->context, cfg_reference->info.device_no, subdevice, mode, timeout, iFlags);
	}

	return err;
}

int  ME_virtual_StreamStopList(const me_config_t* cfg, meIOStreamStop_t* list, int count, int iFlags)
{
	int err;
	me_cfg_device_entry_t** reference_list;
	me_cfg_device_entry_t* cfg_reference;
	meIOStreamStop_t* internal_list;
	meIOStreamStop_t* pos;
	int list_size;
	int i;

	LIBPDEBUG("pStopList=%p iCount=%d iFlags=0x%x", list, count, iFlags);

	if (count < 1)
	{
		err = ME_ERRNO_INVALID_CONFIG_LIST_COUNT;
		goto ERROR_0;
	}

	internal_list = calloc(count, sizeof(meIOStreamStop_t));
	if (!internal_list)
	{
		err = -ENOMEM;
		goto ERROR_0;
	}

	reference_list = calloc(count, sizeof(me_cfg_device_entry_t*));
	if (!reference_list)
	{
		err = -ENOMEM;
		goto ERROR_1;
	}

	for (i=0; i<count; i++)
	{
		err = ConfigResolve(cfg, (list + i)->iDevice, reference_list + i);
		if (err)
		{
			if (!(iFlags  & ME_IO_STREAM_STOP_NONBLOCKING))
			{
				break;
			}
		}
		cfg_reference = *(reference_list+i);
		*(internal_list + i) = *(list + i);
		(internal_list + i)->iDevice = cfg_reference->info.device_no;
		(internal_list + i)->iErrno = err;
	}

	if ((!err) || (iFlags  & ME_IO_STREAM_STOP_NONBLOCKING))
	{
		pos = internal_list;
		list_size = 1;
		cfg_reference = *reference_list;

		for (i=1; i<count; i++)
		{
			if (cfg_reference == *(reference_list+i))
			{
				list_size++;
			}
			else
			{
				if ((me_dummy_context_t *)cfg_reference->context)
				{
					err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStopList(cfg_reference->context, pos, list_size, iFlags);
					if (!(iFlags  & ME_IO_STREAM_STOP_NONBLOCKING))
					{
						break;
					}
				}
				pos = internal_list + i;
				list_size = 1;
				cfg_reference = *(reference_list + i);
			}
		}

		if ((!err) || (iFlags  & ME_IO_STREAM_STOP_NONBLOCKING))
		{
			if ((me_dummy_context_t *)cfg_reference->context)
			{
				err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamStopList(cfg_reference->context, pos, list_size, iFlags);
			}
		}
	}

	for (i=0; i<count; i++)
	{
		(list + i)->iErrno = (internal_list + i)->iErrno;
	}

	if (iFlags  & ME_IO_STREAM_START_NONBLOCKING)
	{
		err = ME_ERRNO_SUCCESS;
	}

	if (reference_list)
	{
		free(reference_list);
		reference_list = NULL;
	}
ERROR_1:
	if (internal_list)
	{
		free(internal_list);
		internal_list = NULL;
	}
ERROR_0:

	return err;
}

int  ME_virtual_StreamSetCallbacks(const me_config_t* cfg,
							int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamSetCallbacks(cfg_reference->context, cfg_reference->info.device_no, subdevice, start, start_context, new_values, new_value_context, end, end_context, iFlags);
	}

	return err;
}

int ME_virtual_StreamTimeToTicks(const me_config_t* cfg, int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamTimeToTicks(cfg_reference->context, cfg_reference->info.device_no, subdevice, timer, stream_time, ticks_low, ticks_high, iFlags);
	}

	return err;
}

int ME_virtual_StreamFrequencyToTicks(const me_config_t* cfg, int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->StreamFrequencyToTicks(cfg_reference->context, cfg_reference->info.device_no, subdevice, timer, frequency, ticks_low, ticks_high, iFlags);
	}

	return err;
}

int ME_virtual_ParametersSet(const me_config_t* cfg, me_extra_param_set_t* paramset, int flags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, paramset->device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->ParametersSet(cfg_reference->context, cfg_reference->info.device_no, paramset, flags);
	}

	return err;
}


int ME_virtual_SetOffset(const me_config_t* cfg, int device, int subdevice, int channel, int range, double* offset, int iFlags)
{
	int err;
	me_cfg_device_entry_t* cfg_reference;

	err = ConfigResolve(cfg, device, &cfg_reference);
	if (!err)
	{
		err = ((me_dummy_context_t *)cfg_reference->context)->context_calls->SetOffset(cfg_reference->context, cfg_reference->info.device_no, subdevice, channel, range, offset, iFlags);
	}

	return err;
}