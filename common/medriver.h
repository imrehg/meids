/**
 * @file medriver.h
 *
 * @brief Common internal API headers.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _MEDRIVER_H_
# define _MEDRIVER_H_

# include "../osi/medriver.h"

	/*===========================================================================
	  Functions to query a remote driver system
	  =========================================================================*/

	int meRQueryDescriptionDevice(
			char *location,
			int iDevice,
			char *pcDescription,
			int iCount);

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
			int *piPlugged);

	int meRQueryNameDevice(
			char *location,
			int iDevice,
			char *pcName,
			int iCount);

	int meRQueryNumberDevices(char *location, int *piNumber);
	int meRQueryNumberSubdevices(char *location, int iDevice, int *piNumber);
	int meRQueryNumberChannels(
			char *location,
			int iDevice,
			int iSubdevice,
			int *piNumber);
	int meRQueryNumberRanges(
			char *location,
			int iDevice,
			int iSubdevice,
			int iUnit,
			int *piNumber);

	int meRQueryRangeInfo(
			char *location,
			int iDevice,
			int iSubdevice,
			int iRange,
			int *piUnit,
			double *pdMin,
			double *pdMax,
			int *piMaxData);

	int meRQuerySubdeviceType(
			char *location,
			int iDevice,
			int iSubdevice,
			int *piType,
			int *piSubtype);


#endif
