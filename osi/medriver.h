/**
 * @file medriver.h
 *
 * @brief API headers.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _OSI_MEDRIVER_H_
# define _OSI_MEDRIVER_H_

#include "metypes.h"
#include "meerror.h"
#include "medefines.h"
#include "medatatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	/*===========================================================================
	  Functions to access the driver system
	  =========================================================================*/

	int meOpen(int iFlags);
	int meClose(int iFlags);

	int meLockDriver(int iLock, int iFlags);
	int meLockDevice(int iDevice, int iLock, int iFlags);
	int meLockSubdevice(int iDevice, int iSubdevice, int iLock, int iFlags);

	/*===========================================================================
	  Error handling functions
	  =========================================================================*/

	int meErrorGetLast(int* piErrorCode, int iFlags);
	int meErrorGetLastMessage(char *pcErrorMsg, int iCount);
	int meErrorGetMessage(int iErrorCode, char *pcErrorMsg, int iCount);
	int meErrorSetDefaultProc(int iSwitch);
	int meErrorSetUserProc(meErrorCB_t pErrorProc);


	/*===========================================================================
	  Functions to perform I/O on a device
	  =========================================================================*/

	int meIOIrqSetCallback(
			int iDevice,
			int iSubdevice,
			meIOIrqCB_t pCallback,
			void *pCallbackContext,
			int iFlags);
	int meIOIrqStart(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iIrqSource,
			int iIrqEdge,
			int iIrqArg,
			int iFlags);
	int meIOIrqStop(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iFlags);
	int meIOIrqWait(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int *piIrqCount,
			int *piValue,
			int iTimeOut,
			int iFlags);

	int meIOResetDevice(int iDevice, int iFlags);
	int meIOResetSubdevice(int iDevice, int iSubdevice, int iFlags);

	int meIOSingleConfig(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iSingleConfig,
			int iRef,
			int iTrigChan,
			int iTrigType,
			int iTrigEdge,
			int iFlags);
	int meIOSingle(meIOSingle_t *pSingleList, int iCount, int iFlags);

	int meIOStreamConfig(
			int iDevice,
			int iSubdevice,
			meIOStreamConfig_t *pConfigList,
			int iCount,
			meIOStreamTrigger_t *pTrigger,
			int iFifoIrqThreshold,
			int iFlags);
	int meIOStreamNewValues(
			int iDevice,
			int iSubdevice,
			int iTimeOut,
			int *piCount,
			int iFlags);
	int meIOStreamRead(
			int iDevice,
			int iSubdevice,
			int iReadMode,
			int *piValues,
			int *piCount,
			int iFlags);
	int meIOStreamWrite(
			int iDevice,
			int iSubdevice,
			int iWriteMode,
			int *piValues,
			int *piCount,
			int iFlags);
	int meIOStreamStart(meIOStreamStart_t *pStartList, int iCount, int iFlags);
	int meIOStreamStop(meIOStreamStop_t *pStopList, int iCount, int iFlags);
	int meIOStreamStatus(
			int iDevice,
			int iSubdevice,
			int iWait,
			int *piStatus,
			int *piCount,
			int iFlags);
	int meIOStreamSetCallbacks(
			int iDevice,
			int iSubdevice,
			meIOStreamCB_t pStartCB,
			void *pStartCBContext,
			meIOStreamCB_t pNewValuesCB,
			void *pNewValuesCBContext,
			meIOStreamCB_t pEndCB,
			void *pEndCBContext,
			int iFlags);
	int meIOStreamTimeToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			double *pdTime,
			int *piTicksLow,
			int *piTicksHigh,
			int iFlags);
	int meIOStreamFrequencyToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			double *pdFrequency,
			int *piTicksLow,
			int *piTicksHigh,
			int iFlags);

	int meIOSetChannelOffset(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iRange,
			double *pdOffset,
			int iFlags);

	int meIOSingleTimeToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			double *pdTime,
			int *piTicksLow,
			int *piTicksHigh,
			int iFlags);
	int meIOSingleTicksToTime(
			int iDevice,
			int iSubdevice,
			int iTimer,
			int iTicksLow,
			int iTicksHigh,
			double *pdTime,
			int iFlags);

	/*===========================================================================
	  Functions to query the driver system
	  =========================================================================*/

	int meQueryDescriptionDevice(int iDevice, char *pcDescription, int iCount);

	int meQueryInfoDevice(
			int iDevice,
			int *piVendorId,
			int *piDeviceId,
			int *piSerialNo,
			int *piBusType,
			int *piBusNo,
			int *piDevNo,
			int *piFuncNo,
			int *piPlugged);

	int meQueryNameDevice(int iDevice, char *pcName, int iCount);
	int meQueryNameDeviceDriver(int iDevice, char *pcName, int iCount);

	int meQueryNumberDevices(int *piNumber);
	int meQueryNumberSubdevices(int iDevice, int *piNumber);
	int meQueryNumberChannels(int iDevice, int iSubdevice, int *piNumber);
	int meQueryNumberRanges(
			int iDevice,
			int iSubdevice,
			int iUnit,
			int *piNumber);

	int meQueryRangeByMinMax(
			int iDevice,
			int iSubdevice,
			int iUnit,
			double *pdMin,
			double *pdMax,
			int *piMaxData,
			int *piRange);
	int meQueryRangeInfo(
			int iDevice,
			int iSubdevice,
			int iRange,
			int *piUnit,
			double *pdMin,
			double *pdMax,
			int *piMaxData);

	int meQuerySubdeviceByType(
			int iDevice,
			int iStartSubdevice,
			int iType,
			int iSubtype,
			int *piSubdevice);
	int meQuerySubdeviceType(
			int iDevice,
			int iSubdevice,
			int *piType,
			int *piSubtype);
	int meQuerySubdeviceCaps(
			int iDevice,
			int iSubdevice,
			int *piCaps);
	int meQuerySubdeviceCapsArgs(
			int iDevice,
			int iSubdevice,
			int iCap,
			int *piArgs,
			int iCount);

	int meQueryVersionLibrary(int *piVersion);
	int meQueryVersionMainDriver(int *piVersion);
	int meQueryVersionDeviceDriver(int iDevice, int *piVersion);


	/*===========================================================================
	  Common utility functions
	  =========================================================================*/

	int meUtilityExtractValues(
			int iChannel,
			int *piAIBuffer,
			int iAIBufferCount,
			meIOStreamConfig_t *pConfigList,
			int iConfigListCount,
			int *piChanBuffer,
			int *piChanBufferCount);
	int meUtilityDigitalToPhysical(
			double dMin,
			double dMax,
			int iMaxData,
			int iData,
			int iModuleType,
			double dRefValue,
			double *pdPhysical);
	int meUtilityDigitalToPhysicalV(
			double dMin,
			double dMax,
			int iMaxData,
			int *piDataBuffer,
			int iCount,
			int iModuleType,
			double dRefValue,
			double *pdPhysicalBuffer);
	int meUtilityPhysicalToDigital(
			double dMin,
			double dMax,
			int iMaxData,
			double dPhysical,
			int *piData);
	int meUtilityPhysicalToDigitalV(
			double dMin,
			double dMax,
			int iMaxData,
			double* pdPhysicalBuffer,
			int iCount,
			int* piDataBuffer);
	int meUtilityPWMStart(
			int iDevice,
			int iSubdevice1,
			int iSubdevice2,
			int iSubdevice3,
			int iRef,
			int iPrescaler,
			int iDutyCycle,
			int iFlag);
	int meUtilityPWMStop(int iDevice,
			int iSubdevice1);
	int meUtilityPWMRestart(
			int iDevice,
			int iSubdevice1,
			int iRef,
			int iPrescaler);

	/*===========================================================================
	  FIO related utility functions
	  =========================================================================*/
	int meUtilityPeriodToTicks(int iBaseFreq, double dPeriod, unsigned int* piTicks);
	int meUtilityTicksToPeriod(int iBaseFreq, unsigned int iTicks, double* pdPeriod);
	int meUtilityFrequencyToTicks(int iBaseFreq, double dFrequency, unsigned int* piTicks);
	int meUtilityTicksToFrequency(int iBaseFreq, unsigned int iTicks, double* pdFrequency);
	int meUtilityCodeDivider(double dDivider, unsigned int* piDivider);
	int meUtilityDecodeDivider(unsigned int iDivider, double* pdDivider);

	/*===========================================================================
	  Load configuration from file into driver system
	  =========================================================================*/

	int meConfigLoad(char *);

#ifdef __cplusplus
}
#endif

#endif
