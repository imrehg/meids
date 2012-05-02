/***************************************************************************
 *   Copyright (C) 2009 by Krzysztof Gantzke     <k.gantzke@meilhaus.de>   *
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
#include <errno.h>

#include "medriver.h"

#define NoBlocks	2

int main(int argc, char *argv[]);
void GetAIPacket (int device, int subdevice, int range);

int main(int argc, char *argv[])
{
	int NoDev = -1;
	int NoSubDev = -1;
	int NoRange;

	int MaxRange;

	int LibVer = -1;
	int DrvVer = -1;
	int i, l;
	int ret_err = 0;


	int VendorId = -1;
	int DeviceId = -1;
	int SerialNo = -1;
	int BusType = -1;
	int BusNo = -1;
	int DevNo = -1;
	int FuncNo = -1;
	int Plugged = -1;

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

		meQueryNumberSubdevices(i, &NoSubDev);
		for (l=0; l<NoSubDev; l++)
		{
			ret_err = meQueryNumberRanges(i, l, ME_UNIT_ANY, &MaxRange);
			if (!ret_err)
			{
				ret_err = meQuerySubdeviceByType(i, l, ME_TYPE_AI, ME_SUBTYPE_STREAMING, &l);
				if (!ret_err)
				{
					for (NoRange=0; NoRange<MaxRange; NoRange++)
					{
						GetAIPacket (i, l, NoRange);
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	meClose(0);

	return EXIT_SUCCESS;
}

void GetAIPacket (int NoDev, int NoSubDev, int NoRange)
{
	int i,l;
	int ret_err = 0;

	char txtError[1024] = "";

	double minVal = 0.0;
	double maxVal = 0.0;
	int unit = ME_UNIT_VOLT;
	int maxData = 0;

	double val = 0.0;

	int NoChannels;
	int AIBufferCount;
	int* AIBuffer = NULL;
	meIOStreamConfig_t* ConfigList = NULL;
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
	Trigger.iConvStartTicksLow = 330000;	//100Hz
	Trigger.iConvStartTicksHigh = 0;

	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iAcqStopCount = 0;
	Trigger.iFlags = ME_VALUE_NOT_USED;

	StartList.iStartMode = ME_START_MODE_BLOCKING;
	StartList.iTimeOut = 0;
	StartList.iFlags = ME_VALUE_NOT_USED;
	StartList.iErrno = ME_VALUE_NOT_USED;

	StopList.iStopMode = ME_STOP_MODE_IMMEDIATE;
	StopList.iFlags = ME_VALUE_NOT_USED;
	StopList.iErrno = ME_VALUE_NOT_USED;

	StartList.iDevice = NoDev;
	StartList.iSubdevice = NoSubDev;

	StopList.iDevice = NoDev;
	StopList.iSubdevice = NoSubDev;

	printf("Configure AI device [%d:%d]\n", NoDev, NoSubDev);
	ret_err = meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	ret_err = meQueryNumberChannels(NoDev, NoSubDev, &NoChannels);
	if (ret_err)
	{
		goto EXIT;
	}
	if (!NoChannels)
	{
		printf("No channels!\n");
		goto EXIT;
	}

	AIBuffer = (int *)calloc(NoChannels, sizeof(int));
	if (!AIBuffer)
	{
		ret_err = ENOMEM;
		goto EXIT;
	}
	ConfigList = (meIOStreamConfig_t *)calloc(NoChannels, sizeof(meIOStreamConfig_t));
	if (!ConfigList)
	{
		ret_err = ENOMEM;
		goto EXIT;
	}
	for (i=0; i<NoChannels; ++i)
	{
		ConfigList[i].iChannel = i;
		ConfigList[i].iStreamConfig = NoRange;
		ConfigList[i].iRef = ME_REF_AI_GROUND;
		ConfigList[i].iFlags = ME_VALUE_NOT_USED;
	}

	ret_err = meQueryRangeInfo(NoDev, NoSubDev, NoRange, &unit, &minVal, &maxVal, &maxData);
	if (ret_err)
	{
		goto EXIT;
	}
	printf("Range %d: %+0.2FV - %+0.2FV\n", NoRange, minVal, maxVal);

	ret_err = meIOStreamConfig(NoDev, NoSubDev, ConfigList, NoChannels, &Trigger, NoChannels, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	ret_err = meIOStreamStart(&StartList, 1, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}
	printf("Working...\n");

	for (l=0; l<NoBlocks; ++l)
	{
		AIBufferCount=NoChannels;

		ret_err = meIOStreamRead(NoDev, NoSubDev, ME_READ_MODE_BLOCKING, AIBuffer, &AIBufferCount, ME_VALUE_NOT_USED);
		if (ret_err)
		{
			goto EXIT;
		}

		for (i=0; i<AIBufferCount; i++)
		{
			meUtilityDigitalToPhysical(minVal, maxVal, maxData, AIBuffer[i], 0, 0, &val);
			printf ("Sample: %d Channel: %d => %0.2fV\n", l, i, val);
		}
	}

	meIOStreamStop(&StopList, 1, ME_VALUE_NOT_USED);

EXIT:
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}
	else
	{
		printf("\n");
	}
	if (ConfigList)
		free (ConfigList);
	if (AIBuffer)
		free (AIBuffer);
}
