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

#include "medriver.h"

#define AIBufSize	0x10
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


	char txtError[1024] = "";

	double val = 0.0;

	int AIBufferCount = AIBufSize;
	int AIBuffer[AIBufSize];
	meIOStreamConfig_t ConfigList[2];
	meIOStreamTrigger_t Trigger;

	meIOStreamStart_t StartList;

	Trigger.iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
	Trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_RISING;
	Trigger.iAcqStartTrigChan = ME_TRIG_CHAN_DEFAULT;
	Trigger.iAcqStartTicksLow = 66;
	Trigger.iAcqStartTicksHigh = 0;
	Trigger.iScanStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
	Trigger.iScanStartTicksLow = 0;
	Trigger.iScanStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iScanStartTicksLow = 66;
	Trigger.iScanStartTicksHigh = 0;
	Trigger.iConvStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iConvStartTicksLow = 33000;	//1KHz
// // 	Trigger.iConvStartTicksLow = 3300;	//10KHz
// // 	Trigger.iConvStartTicksLow = 1650;	//20KHz
// // 	Trigger.iConvStartTicksLow = 660;	//50KHz
// // 	Trigger.iConvStartTicksLow = 330;	//100KHz
// // 	Trigger.iConvStartTicksLow = 66;	//500KHz
	Trigger.iConvStartTicksHigh = 0;
	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_COUNT;
	Trigger.iAcqStopCount = AIBufSize/2;
	Trigger.iFlags = ME_VALUE_NOT_USED;

	ConfigList[0].iChannel = 0;
	ConfigList[0].iStreamConfig = 0;
	ConfigList[0].iRef = ME_REF_AI_GROUND;
	ConfigList[0].iFlags = ME_VALUE_NOT_USED;

	ConfigList[1].iChannel = 1;
	ConfigList[1].iStreamConfig = 0;
	ConfigList[1].iRef = ME_REF_AI_GROUND;
	ConfigList[1].iFlags = ME_VALUE_NOT_USED;

	StartList.iStartMode = ME_START_MODE_BLOCKING;
	StartList.iTimeOut = 0;
	StartList.iFlags = ME_VALUE_NOT_USED;
	StartList.iErrno = ME_VALUE_NOT_USED;

	printf("Hello, AI externally triggered test!\n");

	if (meOpen(0))
		exit (EXIT_FAILURE);

	if (!meQueryNumberDevices(&NoDev))
		printf("%d device%s in system.\n", NoDev, (NoDev>1)?"s":"");

	if (!meQueryVersionLibrary(&LibVer))
		printf("Library version: 0x%08x.\n", LibVer);

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

	ret_err = meQueryRangeInfo(NoDev, NoSubDev, 0, &unit, &minVal, &maxVal, &maxData);
	if (ret_err)
	{
		goto EXIT;
	}
	ret_err = meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Configure stream\n");
	ret_err = meIOStreamConfig(NoDev, NoSubDev, ConfigList, 2, &Trigger, 0, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Start AI\n");
	ret_err = meIOStreamStart(&StartList, 1, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}
	printf("Work\n");

	ret_err = meIOStreamRead(NoDev, NoSubDev, ME_READ_MODE_BLOCKING, AIBuffer, &AIBufferCount, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf ("Samples: %d\n", AIBufferCount);
	for (i=0; i<AIBufferCount; i++)
	{
		meUtilityDigitalToPhysical(minVal, maxVal, maxData, AIBuffer[i], 0, 0, &val);
		printf ("Sample %d[%d] => %f\n", i/2, i%2, val);
	}

	printf("Stop AI\n");

EXIT:
	meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}

	meClose(0);
	printf("Boodbye, AI externally triggered test!\n");

	return EXIT_SUCCESS;
}
