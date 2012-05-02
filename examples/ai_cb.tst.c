/***************************************************************************
 *   Copyright (C) 2008 by Krzysztof Gantzke     <k.gantzke@meilhaus.de>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

#include "medriver.h"

int callbackStart(int device, int subdevice, int count, void* context, int status);
int callbackStop(int device, int subdevice, int count, void* context, int status);
int callbackNewValues(int device, int subdevice, int count, void* context, int status);


#define AIBufSize	0x20

int AIBufferCount = 0;
int AIBuffer[AIBufSize];

int main(int argc, char *argv[])
{
	int NoDev = -1;
	int NoSubDev = -1;
	int LibVer = -1;
	int DrvVer = -1;
	int i = -1;
	int ret_err = 0;

	int VendorId = -1;
	int DeviceId = -1;
	int SerialNo = -1;
	int BusType = -1;
	int BusNo = -1;
	int DevNo = -1;
	int FuncNo = -1;
	int Plugged = -1;

	double minVal = 0.0;
	double maxVal = 0.0;
	int unit = ME_UNIT_VOLT;
	int maxData = 0;
	double val = 0.0;


	char txtError[1024] = "";


	meIOStreamConfig_t ConfigList[1];
	meIOStreamTrigger_t Trigger;

	meIOStreamStart_t StartList;
	meIOStreamStop_t StopList;

	Trigger.iAcqStartTrigType = ME_TRIG_TYPE_SW;
	Trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_NONE;
	Trigger.iAcqStartTrigChan = ME_TRIG_CHAN_DEFAULT;
	Trigger.iAcqStartTicksLow = 66;
	Trigger.iAcqStartTicksHigh = 0;
	Trigger.iScanStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iScanStartTicksLow = 66;
	Trigger.iScanStartTicksHigh = 0;
	Trigger.iConvStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iConvStartTicksLow = 33000;	//1KHz
// 	Trigger.iConvStartTicksLow = 16500;	//2KHz
// 	Trigger.iConvStartTicksLow = 6600;	//5KHz
// 	Trigger.iConvStartTicksLow = 3300;	//10KHz
// 	Trigger.iConvStartTicksLow = 1650;	//20KHz
// 	Trigger.iConvStartTicksLow = 660;	//50KHz
//PCI only
// 	Trigger.iConvStartTicksLow = 330;	//100KHz
// 	Trigger.iConvStartTicksLow = 165;	//200KHz
// 	Trigger.iConvStartTicksLow = 66;	//500KHz
	Trigger.iConvStartTicksHigh = 0;
	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_COUNT;
	Trigger.iAcqStopCount = AIBufSize;
	Trigger.iFlags = ME_VALUE_NOT_USED;


	ConfigList[0].iChannel = 0;
	ConfigList[0].iStreamConfig = 0;
	ConfigList[0].iRef = ME_REF_AI_GROUND;
	ConfigList[0].iFlags = ME_VALUE_NOT_USED;


	StartList.iStartMode = ME_START_MODE_BLOCKING;
	StartList.iTimeOut = 0;
	StartList.iFlags = ME_VALUE_NOT_USED;
	StartList.iErrno = ME_VALUE_NOT_USED;

	StopList.iStopMode = ME_STOP_MODE_IMMEDIATE;
	StopList.iFlags = ME_VALUE_NOT_USED;
	StopList.iErrno = ME_VALUE_NOT_USED;

	printf("Hello, callback AI test!\n");

	if (meOpen(0))
		exit (EXIT_FAILURE);

	if (!meQueryVersionLibrary(&LibVer))
		printf("Library version: 0x%08x.\n", LibVer);

	if (!meQueryNumberDevices(&NoDev))
		printf("%d device%s in system.\n", NoDev, (NoDev>1)?"s":"");
	if (!NoDev)
		return 0;

	if (!meQueryVersionMainDriver(&DrvVer))
		printf("Main driver version: 0x%08x.\n", DrvVer);

	printf("\n");
	for (i=0; i<NoDev; ++i)
	{
		meQueryInfoDevice(i,
			&VendorId,
			&DeviceId,
			&SerialNo,
			&BusType,
			&BusNo,
			&DevNo,
			&FuncNo,
			&Plugged);

		ret_err = meQuerySubdeviceByType(i, -1, ME_TYPE_AI, ME_SUBTYPE_STREAMING, &NoSubDev);
		if (!ret_err)
		{
			printf("Device [%d,%d] is an STREAMING AI\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		ret_err = EXIT_SUCCESS;
		printf("STREAMING AI not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	StartList.iDevice = NoDev;
	StartList.iSubdevice = NoSubDev;
	StopList.iDevice = NoDev;
	StopList.iSubdevice = NoSubDev;

	meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);

	printf("Configure stream\n");
	ret_err = meIOStreamConfig(NoDev, NoSubDev, ConfigList, 1, &Trigger, ME_VALUE_NOT_USED, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Register callbacks.\n");
	ret_err = meIOStreamSetCallbacks(NoDev, NoSubDev, callbackStart, NULL, callbackNewValues, NULL, callbackStop, NULL, ME_VALUE_NOT_USED);

	printf("Start AI\n");
	ret_err = meIOStreamStart(&StartList, 1, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Work\n");
	sleep(10);

	printf("Stop AI\n");
	meIOStreamStop(&StopList, 1, ME_VALUE_NOT_USED);
	printf("AO stoped.\n");

	meQueryRangeInfo(NoDev, NoSubDev, 0, &unit, &minVal, &maxVal, &maxData);
	for (i=0; i<AIBufferCount; i++)
	{
		meUtilityDigitalToPhysical(minVal, maxVal, maxData, AIBuffer[i], 0, 0, &val);
		printf ("%d => %f\n", i, val);
	}

EXIT:
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}

	meClose(0);
	printf("Boodbye, callback AI test!\n");

	return EXIT_SUCCESS;
}

int callbackStart(int device, int subdevice, int count, void* context, int status)
{
	printf("%ld Start callback! count=%d status=%d\n", pthread_self(), count, status);

	return EXIT_SUCCESS;

}

int callbackStop(int device, int subdevice, int count, void* context, int status)
{
	printf("%ld Stop callback! count=%d status=%d\n", pthread_self(), count, status);

	return EXIT_SUCCESS;

}

int callbackNewValues(int device, int subdevice, int count, void* context, int status)
{
	int BufferCount = count;


	printf("%ld New Values callback! count=%d status=%d\n", pthread_self(), count, status);

	printf("Read stream\n");
	meIOStreamRead(device, subdevice, ME_READ_MODE_BLOCKING, AIBuffer + AIBufferCount, &BufferCount, ME_VALUE_NOT_USED);
	printf("read count = %d\n", BufferCount);
	AIBufferCount += BufferCount;

	return EXIT_SUCCESS;

}
