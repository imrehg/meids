/*
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * Header file: meids_utility.h
 * Author:	KG (Krzysztof Gantzke)	<k.gantzke@meilhaus.de>
 */

#ifndef _MEUTILITY_H_
# define _MEUTILITY_H_

# ifdef __cplusplus
extern "C" {
# endif

#  define ME_RANGE_UNIPOLAR_10V		0xEA540001
#  define ME_RANGE_BIPOLAR_10V		0xEA540002
#  define ME_RANGE_UNIPOLAR_2_5V	0xEA540003
#  define ME_RANGE_BIPOLAR_2_5V		0xEA540004
#  define ME_RANGE_0_20mA			0xEA540005
#  define ME_RANGE_4_20mA			0xEA540006
#  define ME_RANGE_UNIPOLAR_50V		0xEA540007

typedef struct meTypeList
{
	unsigned int me_di  : 1;
	unsigned int me_do  : 1;
	unsigned int me_dio : 1;
	unsigned int me_ai  : 1;
	unsigned int me_ao  : 1;
	unsigned int me_cnt : 1;
	unsigned int me_ext : 1;
	unsigned int dummy  : 17;
}  __attribute__((packed)) meTypeList_t;

/**
	@brief Get dynamic range number assigned to pre-deffined ones.
*/
int meGetPredefinedRange(int iDevice, int iSubdevice, int iRequed_Range, int* piRange);
/**
	@brief Get dynamic range number that has provided parameters.
*/
int meGetStrictRange(int iDevice, int iSubdevice, int iUnit, double dMin, double dMax, int* piRange);

/**
	@brief Simple version of meIOSingle() command. Single value is read/write.
*/
int meSingle(int iDevice, int iSubdevice, int iChannel, int Direction, int* piValue, int iTimeout, int iFlags);
/**
	@brief Simple version of meIOSingle() command. Single value is read.
*/
int meSingleRead(int iDevice, int iSubdevice, int iChannel, int* piValue, int iTimeout, int iFlags);
/**
	@brief Simple version of meIOSingle() command. Single value is write.
*/
int meSingleWrite(int iDevice, int iSubdevice, int iChannel, int iValue, int iTimeout, int iFlags);

/**
	@brief Read single value using provided range.
*/
int meValueRead(int iDevice, int iSubdevice, int iChannel, double* pdValue, int iRange, int iTimeout, int iFlags);
/**
	@brief Write single value using provided range.
*/
int meValueWrite(int iDevice, int iSubdevice, int iChannel, double dValue, int iRange, int iTimeout, int iFlags);

/**
	@brief Read current values from all channels.
*/
int meSingleScanRead(int iDevice, int iSubdevice, double* pdValue, int* count, int iRange, int iTriggerType, int iTriggerEdge, int iTimeout, int iFlags);

/**
	@brief Extended version of meIOStreamRead(). Timeout added.
*/
int meStreamRead(int iDevice, int iSubdevice, int iReadMode, int* piValues, int* piCount, int iTimeout, int iFlags);
/**
	@brief Extended version of meIOStreamWrite(). Timeout added.
*/
int meStreamWrite(int iDevice, int iSubdevice, int iWriteMode, int* piValues, int* piCount, int iTimeout, int iFlags);

/**
	@brief Get board serial number.
*/
int meQuerySerialNumber(int iDevice, int* piDeviceId, int* piSerialNo);
/**
	@brief Get number of boards of provided type.
*/
int meQueryNumberSubdevicesByType(int iDevice, int iType, int iSubtype, int* piNumber);

/**
	@brief Number of devices with corresponding ID.
*/
int meQueryNumberDevicesById(int iID, int iIDMask);

/**
	@brief Number of devices that have at least one subdevice of type marked on list.
*/
int meQueryNumberDevicesByTypeList(meTypeList_t TypesList);
/**
	@brief Number of devices that have at least one subdevice of type marked on list. Subdevice has to also support provided caps.
*/
int meQueryNumberDevicesByTypeListAndCaps(meTypeList_t TypesList, int iCaps);

/**
	@brief Number of sub-devices that have at least one of types marked on list.
*/
int meQueryNumberSubdevicesByTypeList(int iDevice, meTypeList_t TypesList);
/**
	@brief Number of sub-devices that have at least one of types marked on list. Subdevice has to also support provided caps.
*/
int meQueryNumberSubdevicesByTypeListAndCaps(int iDevice, meTypeList_t TypesList, int iCaps);

/**
	@brief Test if subdevice type is marked on list.
*/
int CheckMeSubdeviceTypeList(int iDevice, int iSubdevice, meTypeList_t TypesList);
/**
	@brief Test if subdevice supports provided caps.
*/
int CheckMeSubdeviceCaps(int iDevice, int iSubdevice, int iCaps);
/**
	@brief Test if subdevice type is marked on list and also support provided caps.
*/
int CheckMeSubdeviceTypeListAndCaps(int iDevice, int iSubdevice, meTypeList_t TypesList, int iCaps);

/**
	@brief Set default timeout for connecting to remote devices (in ms).
*/
void SetMeRemoteDetectionTimeout(int timeout);

# ifdef __cplusplus
}
# endif	//__cplusplus
#endif	//_MEUTILITY_H_
