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

#include "meids_local_RQuery.h"

/// Functions to query a remote driver system. Not in use for local devices.

int meRQueryDescriptionDevice(
		char *location,
		int iDevice,
		char *pcDescription,
		int iCount)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryInfoDevice(
		char *location,
		int iDevice,
		int *piVendorId,
		int *piDeviceId,
		int *piSerialNo,
		int *piBusType,
		int *piBusNo,
		int *piDevNo,
		int *piFuncNo,
		int *piPlugged)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryNameDevice(
		char *location,
		int iDevice,
		char *pcName,
		int iCount)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryNumberDevices(char *location, int *piNumber)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryNumberSubdevices(char *location, int iDevice, int *piNumber)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryNumberChannels(
		char *location,
		int iDevice,
		int iSubdevice,
		int *piNumber)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryNumberRanges(
		char *location,
		int iDevice,
		int iSubdevice,
		int iUnit,
		int *piNumber)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQueryRangeInfo(
		char *location,
		int iDevice,
		int iSubdevice,
		int iRange,
		int *piUnit,
		double *pdMin,
		double *pdMax,
		int *piMaxData)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

int meRQuerySubdeviceType(
		char *location,
		int iDevice,
		int iSubdevice,
		int *piType,
		int *piSubtype)
{
	return ME_ERRNO_NOT_SUPPORTED;
}
