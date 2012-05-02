/* Utility functions for Meilhaus driver system.
 * ==========================================
 *
 *  Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# error This is user space ONLY library!
#endif	//__KERNEL__

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <syslog.h>
# include <unistd.h>

# include <float.h>
# include <math.h>

# include "me_error.h"
# include "me_defines.h"
# include "meids.h"
# include "meids_debug.h"

# include "meids_utility.h"

# include "medriver.h"

extern int test_RPC_timeout;

int meGetPredefinedRange(int iDevice, int iSubdevice, int iRequed_Range, int* piRange)
{
	double	min_value;
	double	max_value;
	int		units;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iRequed_Range=0x%x, piRange=0x%p\n", iDevice, iSubdevice, iRequed_Range, piRange);

	if (!piRange)
	{
		ME_SetErrno("meUtilityGetPredefinedRange()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	*piRange = -1;

	switch (iRequed_Range)
	{
		case ME_RANGE_UNIPOLAR_10V:
			min_value = 0.0;
			max_value = 10.0;
			units = ME_UNIT_VOLT;
			break;

		case ME_RANGE_BIPOLAR_10V:
			min_value = -10.0;
			max_value = 10.0;
			units = ME_UNIT_VOLT;
			break;

		case ME_RANGE_UNIPOLAR_2_5V:
			min_value = 0.0;
			max_value = 2.5;
			units = ME_UNIT_VOLT;
			break;

		case ME_RANGE_BIPOLAR_2_5V:
			min_value = -2.5;
			max_value = 2.5;
			units = ME_UNIT_VOLT;
			break;

		case ME_RANGE_0_20mA:
			min_value = 0.0;
			max_value = 20.0;
			units = ME_UNIT_AMPERE;
			break;

		case ME_RANGE_4_20mA:
			min_value = 4.0;
			max_value = 20.0;
			units = ME_UNIT_AMPERE;
			break;

		case ME_RANGE_UNIPOLAR_50V:
			min_value = 0.0;
			max_value = 50.0;
			units = ME_UNIT_VOLT;
			break;

		default:
			ME_SetErrno("meUtilityGetPredefinedRange()", ME_ERRNO_INVALID_RANGE);
			return ME_ERRNO_INVALID_RANGE;
	}

	return meGetStrictRange(iDevice, iSubdevice, units, min_value, max_value, piRange);
}

int meGetStrictRange(int iDevice, int iSubdevice, int iUnit, double dMin, double dMax, int* piRange)
{
	int err = ME_ERRNO_NO_RANGE;
	int no_ranges = 0;
	int idx;

	int				unit;
	double			min_value;
	double			max_value;
	unsigned int	max_data;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iUnit=0x%x dMin=%f, dMax=%f, piRange=0x%p\n", iDevice, iSubdevice, iUnit, dMin, dMax, piRange);

	if (!piRange)
	{
		ME_SetErrno("meUtilityGetStrictRange()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	*piRange = -1;

	if (!ME_QueryRangesNumber(iDevice, iSubdevice, iUnit, &no_ranges, ME_QUERY_NO_FLAGS))
	{
		for (idx=0; idx<no_ranges; idx++)
		{
			unit = iUnit;
			if (!ME_QueryRangeInfo(iDevice, iSubdevice, idx, &unit, &min_value, &max_value, &max_data, ME_QUERY_NO_FLAGS))
			{
				if ((min_value == dMin) && (max_value >= (0.99 * dMax)) && (max_value <= dMax))
				{
					*piRange = idx;
					err = ME_ERRNO_SUCCESS;
					break;
				}
			}
		}
	}

	ME_SetErrno("meUtilityGetStrictRange()", err);
	return err;
}

int meSingle(int iDevice, int iSubdevice, int iChannel, int Direction, int* piValue, int iTimeout, int iFlags)
{
	int	err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iChannel=%d Direction=0x%x piValue=0x%p iTimeout=%d iFlags=0x%x\n", iDevice, iSubdevice, iChannel, Direction, piValue, iTimeout, iFlags);

	if (!piValue)
	{
		ME_SetErrno("meSingle()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	err = ME_Single(iDevice, iSubdevice, iChannel, Direction, piValue, iTimeout, iFlags);

	ME_SetErrno("meSingle()", err);
	return err;
}

int meSingleRead(int iDevice, int iSubdevice, int iChannel, int* piValue, int iTimeout, int iFlags)
{
	int	err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iChannel=%d piValue=0x%p iTimeout=%d iFlags=0x%x\n", iDevice, iSubdevice, iChannel, piValue, iTimeout, iFlags);

	if (!piValue)
	{
		ME_SetErrno("meSingleRead()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	err = ME_Single(iDevice, iSubdevice, iChannel, ME_DIR_INPUT, piValue, iTimeout, iFlags);

	ME_SetErrno("meSingleRead()", err);
	return err;
}

int meSingleWrite(int iDevice, int iSubdevice, int iChannel, int iValue, int iTimeout, int iFlags)
{
	int	err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iChannel=%d iValue=%d iTimeout=%d iFlags=0x%x\n", iDevice, iSubdevice, iChannel, iValue, iTimeout, iFlags);

	err = ME_Single(iDevice, iSubdevice, iChannel, ME_DIR_OUTPUT, &iValue, iTimeout, iFlags);

	ME_SetErrno("meSingleWrite()", err);
	return err;
}

int meValueRead(int iDevice, int iSubdevice, int iChannel, double* pdValue, int iRange, int iTimeout, int iFlags)
{
	int				iValue;
	int				unit;
	double			min_value;
	double			max_value;
	unsigned int	max_data;
	int				err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iChannel=%d pdValue=0x%p iRange=%d iTimeout=%d iFlags=0x%x\n", iDevice, iSubdevice, iChannel, pdValue, iRange, iTimeout, iFlags);

	if (!pdValue)
	{
		ME_SetErrno("meValueRead()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	err = ME_Single(iDevice, iSubdevice, iChannel, ME_DIR_INPUT, &iValue, iTimeout, iFlags);

	err = ME_QueryRangeInfo(iDevice, iSubdevice, iRange, &unit, &min_value, &max_value, &max_data, ME_QUERY_NO_FLAGS);
	err = meUtilityDigitalToPhysical(min_value, max_value, max_data, iValue, ME_MODULE_TYPE_MULTISIG_NONE, 0.00, pdValue);

	ME_SetErrno("meValueRead()", err);
	return err;
}

int meValueWrite(int iDevice, int iSubdevice, int iChannel, double dValue, int iRange, int iTimeout, int iFlags)
{
	int				iValue;
	int				unit;
	double			min_value;
	double			max_value;
	unsigned int	max_data;
	int				err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d iSubdevice=%d iChannel=%d dValue=%f iRange=%d iTimeout=%d iFlags=0x%x\n", iDevice, iSubdevice, iChannel, dValue, iRange, iTimeout, iFlags);

	err = ME_QueryRangeInfo(iDevice, iSubdevice, iRange, &unit, &min_value, &max_value, &max_data, ME_QUERY_NO_FLAGS);
	err = meUtilityPhysicalToDigital(min_value, max_value, max_data, dValue, &iValue);

	err = ME_Single(iDevice, iSubdevice, iChannel, ME_DIR_OUTPUT, &iValue, iTimeout, iFlags);

	ME_SetErrno("meValueWrite()", err);
	return err;
}

int meQuerySerialNumber(int iDevice, int* piDeviceId, int* piSerialNo)
{
	unsigned int	iVendorId;
	unsigned int	iBusType;
	unsigned int	iBusNo;
	unsigned int	iDevNo;
	unsigned int	iFuncNo;
	int				iPlugged;
	int				err;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("iDevice=%d piDeviceId=0x%p piSerialNo=0x%p\n", iDevice, piDeviceId, piSerialNo);

	if (!piSerialNo || !piDeviceId)
	{
		ME_SetErrno("meQuerySerialNumber()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	err = ME_QueryDeviceInfo(iDevice,
							&iVendorId, (unsigned int *) piDeviceId, (unsigned int *)piSerialNo,
							&iBusType, &iBusNo, &iDevNo, &iFuncNo,
							&iPlugged,
							ME_QUERY_NO_FLAGS);

	ME_SetErrno("meQuerySerialNumber()", err);

	return err;

}

int meQueryNumberSubdevicesByType(int iDevice, int iType, int iSubtype, int* piNumber)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = ME_QuerySubdevicesNumberByType(iDevice, iType, iSubtype, piNumber, ME_QUERY_NO_FLAGS);

	ME_SetErrno("meQueryNumberSubdevicesByType()", err);

	return err;
}


int meSingleScanRead(int iDevice, int iSubdevice, double* pdValue, int* count, int iRange, int iTriggerType, int iTriggerEdge, int iTimeout, int iFlags)
{/// @note  Do one read from all channels. Works only with AI streaming sub-device.
	int err = ME_ERRNO_SUCCESS;
	int tmp;
	int* buff = NULL;
	meIOStreamSimpleConfig_t* config_list = NULL;
	meIOStreamSimpleTriggers_t trigger;

	int iUnit;
	double dMin;
	double dMax;
	unsigned int iMaxData;

	if (!pdValue || !pdValue)
	{
		ME_SetErrno("meSingleScanRead()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	if (*count <= 0)
	{
		// Asked for nothing.
		return ME_ERRNO_SUCCESS;
	}

	err = ME_QueryChannelsNumber(iDevice, iSubdevice, (unsigned int *)&tmp, ME_QUERY_NO_FLAGS);
	if (err)
	{
		goto EXIT;
	}

	if (iFlags & ME_STREAM_CONFIG_DIFFERENTIAL)
	{
// When useing differential mode number of channels must be divided be 2. Support for differential mode is checked in driver.
		if (*count > tmp / 2)
		{
			*count = tmp / 2;
		}
	}
	else
	{
		if (*count > tmp)
		{
			*count = tmp;
		}
	}

	buff = calloc(*count, sizeof(int));
	config_list = calloc(*count, sizeof(meIOStreamSimpleConfig_t));
	if (!buff || !config_list)
	{
		err = -ENOMEM;
		goto EXIT;
	}

	for (tmp = 0; tmp < *count; tmp++)
	{
		(config_list + tmp)->iRange = iRange;
		(config_list + tmp)->iChannel = tmp;
	}

	switch (iTriggerType)
	{
		case ME_TRIG_TYPE_NONE:
		case ME_TRIG_TYPE_SW:
			trigger.trigger_type = ME_TRIGGER_TYPE_SOFTWARE;
			break;

		case ME_TRIG_TYPE_EXT_DIGITAL:
			trigger.trigger_type = ME_TRIGGER_TYPE_ACQ_DIGITAL;
			break;

		case ME_TRIG_TYPE_EXT_ANALOG:
			trigger.trigger_type = ME_TRIGGER_TYPE_ACQ_ANALOG;
			break;

		default:
		err = ME_ERRNO_INVALID_TRIG_TYPE;
		goto EXIT;
	}

	trigger.trigger_edge = iTriggerEdge;
	trigger.acq_ticks = 0;	// Maximum speed.
	trigger.scan_ticks = 0;	// Maximum speed.
	trigger.conv_ticks = 0;	// Maximum speed.
	trigger.synchro = ME_TRIG_CHAN_NONE;
	trigger.stop_type = ME_STREAM_STOP_TYPE_SCAN_VALUE;
	trigger.stop_count = *count;

	tmp=*count;
	err = ME_StreamConfigure(iDevice, iSubdevice, config_list, tmp, &trigger, ME_VALUE_NOT_USED, iFlags);
	if (err)
	{
		goto EXIT;
	}
	err = ME_StreamStart(iDevice, iSubdevice, ME_START_MODE_BLOCKING, iTimeout, ME_IO_STREAM_START_NO_FLAGS);
	if (err)
	{
		goto EXIT;
	}
	err = ME_StreamRead(iDevice, iSubdevice, ME_READ_MODE_BLOCKING, buff, &tmp, *count, ME_IO_STREAM_READ_NO_FLAGS);	// timout: 1ms per value (?)
	if (err)
	{
		goto EXIT;
	}
	err = ME_QueryRangeInfo(iDevice, iSubdevice, iRange, &iUnit, &dMin, &dMax, &iMaxData, ME_QUERY_NO_FLAGS);
	if (err)
	{
		goto EXIT;
	}

	err = meUtilityDigitalToPhysicalV(dMin, dMax, iMaxData, buff, tmp, ME_MODULE_TYPE_MULTISIG_NONE, 0, pdValue);

EXIT:
	if (err)
	{
		ME_SetErrno("meSingleScanRead()", err);
		*count = 0;
	}
	free (config_list);
	free (buff);
	return err;
}

int meStreamRead(int iDevice, int iSubdevice, int iReadMode, int* piValues, int* piCount, int iTimeout, int iFlags)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = ME_StreamRead(iDevice, iSubdevice, iReadMode, piValues, piCount, iTimeout, iFlags);

	ME_SetErrno("meStreamRead()", ME_ERRNO_INVALID_POINTER);

	return err;
}

int meStreamWrite(int iDevice, int iSubdevice, int iWriteMode, int* piValues, int* piCount, int iTimeout, int iFlags)
{
	int err;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = ME_StreamWrite(iDevice, iSubdevice, iWriteMode, piValues, piCount, iTimeout, iFlags);

	ME_SetErrno("meStreamWrite()", ME_ERRNO_INVALID_POINTER);

	return err;
}

/// Count number of boards recognized in system.
int meQueryNumberDevicesById(int iID, int iIDMask)
{
	int index_counter = 0;
	int boards_counter = 0;
	int boards_number = 0;
	int devices_number = 0;

	int VendorId;
	int DeviceId;
	int SerialNo;
	int BusType;
	int BusNo;
	int DevNo;
	int FuncNo;
	int Plugged;

	if (!meQueryNumberDevices(&devices_number))
	{
		do
		{
			if (!meQueryInfoDevice(index_counter, &VendorId, &DeviceId, &SerialNo, &BusType, &BusNo, &DevNo, &FuncNo, &Plugged))
			{
				if ((Plugged == ME_PLUGGED_IN) && ((DeviceId & iIDMask) == (iID & iIDMask)))
				{
					boards_number++;
				}
				boards_counter++;
			}
			index_counter++;
		}
		while (boards_counter < devices_number);
	}

	return boards_number;
}

int meQueryNumberDevicesByTypeList(meTypeList_t TypesList)
{
	int devices_number;
	int index_counter = 0;
	int boards_counter = 0;
	int boards_number = 0;

	int VendorId;
	int DeviceId;
	int SerialNo;
	int BusType;
	int BusNo;
	int DevNo;
	int FuncNo;
	int Plugged;

	if (!meQueryNumberDevices(&devices_number))
	{
		do
		{
			if (!meQueryInfoDevice(index_counter, &VendorId, &DeviceId, &SerialNo, &BusType, &BusNo, &DevNo, &FuncNo, &Plugged))
			{
				if ((Plugged == ME_PLUGGED_IN) && (meQueryNumberSubdevicesByTypeList(index_counter, TypesList) > 0))
				{
					boards_number++;
				}
				boards_counter++;
			}
			index_counter++;
		}
		while (boards_counter < devices_number);
	}

	return boards_number;
}

int meQueryNumberDevicesByTypeListAndCaps(meTypeList_t TypesList, int iCaps)
{
	int index_counter = 0;
	int boards_counter = 0;
	int boards_number = 0;
	int devices_number = 0;

	int VendorId;
	int DeviceId;
	int SerialNo;
	int BusType;
	int BusNo;
	int DevNo;
	int FuncNo;
	int Plugged;

	if (!meQueryNumberDevices(&devices_number))
	{
		do
		{
			if (!meQueryInfoDevice(index_counter, &VendorId, &DeviceId, &SerialNo, &BusType, &BusNo, &DevNo, &FuncNo, &Plugged))
			{
				if ((Plugged = ME_PLUGGED_IN) && (meQueryNumberSubdevicesByTypeListAndCaps(index_counter, TypesList, iCaps) > 0))
				{
					boards_number++;
				}
				boards_counter++;
			}
			index_counter++;
		}
		while (boards_counter < devices_number);
	}

	return boards_number;
}


int meQueryNumberSubdevicesByTypeList(int iDevice, meTypeList_t TypesList)
{
	int subdevices_number;
	int subindex_counter;
	int subdevice_counter = 0;

	if (!meQueryNumberSubdevices(iDevice, &subdevices_number))
	{
		for (subindex_counter = 0;  subindex_counter < subdevices_number; ++subindex_counter)
		{
			if (!CheckMeSubdeviceTypeList(iDevice, subindex_counter, TypesList))
			{
				subdevice_counter++;
			}
		}
	}
	return subdevice_counter;
}

int meQueryNumberSubdevicesByTypeListAndCaps(int iDevice, meTypeList_t TypesList, int iCaps)
{
	int subdevices_number;
	int subindex_counter;
	int subdevice_counter = 0;

	if (!meQueryNumberSubdevices(iDevice, &subdevices_number))
	{
		for (subindex_counter = 0; subindex_counter < subdevices_number; ++subindex_counter)
		{
			if (!CheckMeSubdeviceTypeListAndCaps(iDevice, subindex_counter, TypesList, iCaps))
			{
				subdevice_counter++;
			}
		}
	}

	return subdevice_counter;
}


int CheckMeSubdeviceTypeList(int iDevice, int iSubdevice, meTypeList_t TypesList)
{
	int subdevice_type;
	int subdevice_subtype;

	meQuerySubdeviceType(iDevice, iSubdevice, &subdevice_type, &subdevice_subtype);
	switch (subdevice_type)
	{
		case ME_TYPE_AO:
			if (TypesList.me_ao)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_AI:
			if (TypesList.me_ai)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_DIO:
			if (TypesList.me_dio)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_DO:
			if (TypesList.me_do)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_DI:
			if (TypesList.me_di)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_CTR:
			if (TypesList.me_cnt)
				return ME_ERRNO_SUCCESS;
			break;
		case ME_TYPE_EXT_IRQ:
			if (TypesList.me_ext)
				return ME_ERRNO_SUCCESS;
			break;
	}

	return ME_ERRNO_INVALID_SUBDEVICE;
}

int CheckMeSubdeviceCaps(int iDevice, int iSubdevice, int iCaps)
{
	int subdevice_caps;

	if (!meQuerySubdeviceCaps(iDevice, iSubdevice, &subdevice_caps))
	{
		if ((iCaps & subdevice_caps) == iCaps)
		{
			return ME_ERRNO_SUCCESS;
		}
	}

	return ME_ERRNO_INVALID_CAP;
}

int CheckMeSubdeviceTypeListAndCaps(int iDevice, int iSubdevice, meTypeList_t TypesList, int iCaps)
{
	return CheckMeSubdeviceTypeList(iDevice, iSubdevice, TypesList) || CheckMeSubdeviceCaps(iDevice, iSubdevice, iCaps);
}

void SetMeRemoteDetectionTimeout(int timeout)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	test_RPC_timeout = timeout;
}
