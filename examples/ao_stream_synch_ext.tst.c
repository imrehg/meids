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

int main(int argc, char *argv[])
{
	int NoDev = -1;
	int NoSubDev = -1;
	int LibVer = -1;
	int DrvVer = -1;
	int i = -1;
	int ret_err;

	int VendorId = -1;
	int DeviceId = -1;
	int SerialNo = -1;
	int BusType = -1;
	int BusNo = -1;
	int DevNo = -1;
	int FuncNo = -1;
	int Plugged = -1;

#define AOBufSize	0x10
	int AOBufferCount = AOBufSize;
	int AOBuffer[AOBufSize];
	meIOStreamConfig_t ConfigList;
	meIOStreamTrigger_t Trigger;

	meIOStreamStart_t StartList[4];

	Trigger.iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
	Trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_RISING;
	Trigger.iAcqStartTrigChan = ME_TRIG_CHAN_SYNCHRONOUS;
	Trigger.iAcqStartTicksLow = 0;
	Trigger.iAcqStartTicksHigh = 0;
	Trigger.iScanStartTrigType = ME_TRIG_TYPE_FOLLOW;
	Trigger.iScanStartTicksLow = 0;
	Trigger.iScanStartTicksHigh = 0;
	Trigger.iConvStartTrigType = ME_TRIG_TYPE_TIMER;
// 	Trigger.iConvStartTicksLow = 33000;	//1KHz
// 	Trigger.iConvStartTicksLow = 16500;	//2KHz
// 	Trigger.iConvStartTicksLow = 6600;	//5KHz
// 	Trigger.iConvStartTicksLow = 3300;	//10KHz
// 	Trigger.iConvStartTicksLow = 1650;	//20KHz
// 	Trigger.iConvStartTicksLow = 660;	//50KHz
// 	Trigger.iConvStartTicksLow = 330;	//100KHz
// 	Trigger.iConvStartTicksLow = 165;	//200KHz
	Trigger.iConvStartTicksLow = 66;	//500KHz
	Trigger.iConvStartTicksHigh = 0;
	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iAcqStopCount = 0;
	Trigger.iFlags = ME_VALUE_NOT_USED;

	ConfigList.iChannel = 0;
	ConfigList.iStreamConfig = 0;
	ConfigList.iRef = ME_REF_AO_GROUND;
	ConfigList.iFlags = ME_VALUE_NOT_USED;

	for (i=0; i<AOBufSize; i++)
	{
		AOBuffer[i] = (i & 0x1) ? 0 : ~0;
	}

	printf("Hello, AO stream synchronous start externally triggered test!\n");

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

		ret_err = meQuerySubdeviceByType(i, -1, ME_TYPE_AO, ME_SUBTYPE_STREAMING, &NoSubDev);
		if (!ret_err)
		{
			printf("Device [%d,%d] is an STREAMING AO\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		ret_err = EXIT_SUCCESS;
		printf("STREAMING AO not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	for (i=0; i<4; ++i)
	{
		meIOResetSubdevice(NoDev, NoSubDev + i, ME_VALUE_NOT_USED);
	}

	printf("Configure streams\n");
	for (i=0; i<4; ++i)
	{
		if(meIOStreamConfig(NoDev, NoSubDev + i, &ConfigList, 1, &Trigger, ME_VALUE_NOT_USED, ME_VALUE_NOT_USED))
		{
			printf("meIOStreamConfig() failed!\n");
			goto EXIT;
		}
	}

	printf("Write datas\n");
	for (i=0; i<4; ++i)
	{
		AOBufferCount = AOBufSize;
		meIOStreamWrite(NoDev, NoSubDev + i, ME_WRITE_MODE_PRELOAD, AOBuffer, &AOBufferCount, ME_VALUE_NOT_USED);
	}

	for (i=0; i<4; ++i)
	{
		StartList[i].iDevice = NoDev;
		StartList[i].iSubdevice = NoSubDev + i;
		StartList[i].iStartMode = ME_START_MODE_NONBLOCKING;
		StartList[i].iTimeOut = 0;
		StartList[i].iFlags = ME_VALUE_NOT_USED;
		StartList[i].iErrno = ME_VALUE_NOT_USED;
	}

	printf("Press ENTER to start.\n");
	getchar();

	printf("Start AO (synchronical).\n");
	meIOStreamStart(StartList, 4, ME_VALUE_NOT_USED);

	printf("Waiting for external trigger.\n");

	printf("Press ENTER to quite.\n");
	getchar();

	printf("Stop AO (asynchronical).\n");
	for (i=0; i<4; ++i)
	{
		meIOResetSubdevice(NoDev, NoSubDev + i, ME_VALUE_NOT_USED);
	}


EXIT:
	meClose(0);
	printf("Boodbye, AO stream synchronous start externally triggered test!\n");

	return EXIT_SUCCESS;
}
