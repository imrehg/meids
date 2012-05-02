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

#define DataBlockSize	0x10000
#define DataReadSize	0x1000
#define AIBufSize		0x400

int
irq_cb(int iDevice,
       int iSubdevice,
       int iChannel,
       int iIrqCount,
       int iValue,
       void * pvContext,
       int iErrorCode)
{
    char errTxt[1024];
    int err;

    printf("%s called: iDevice=%i iSubdevice=%i iChannel=%i iIrqCount=%i iValue=%i pvContext=%p iErrorCode=%i.\n",
           __FUNCTION__,
           iDevice,
           iSubdevice,
           iChannel,
           iIrqCount,
           iValue,
           pvContext,
           iErrorCode);

#if 0
    printf("Clear callback.\n");
	err = meIOIrqSetCallback(iDevice,
                             iSubdevice,
                             NULL,
                             NULL,
                             ME_IO_IRQ_WAIT_NORMAL_STATUS);
    if (err) {
        meErrorGetMessage(err, errTxt, sizeof(errTxt));
        printf("In meIOIrqSetCallback(): %s.", errTxt);
        return err;
    }

    printf("Stop external IRQ.\n");
    err = meIOIrqStop(iDevice,
                      iSubdevice,
                      0,
                      ME_IO_IRQ_STOP_NO_FLAGS);
    if (err) {
        meErrorGetMessage(err, errTxt, sizeof(errTxt));
        printf("In meIOIrqStop(): %s.", errTxt);
        return err;
    }
#endif

    return 0;
}


int
prepareOverLoad(int noDev, int noSubDev)
{
    char errTxt[1024];
    int err;

    printf("%s entered: noDev=%i noSubDev=%i.\n", __FUNCTION__, noDev, noSubDev);

	err = meIOIrqStart(noDev,
			           noSubDev,
			           0,
			           ME_IRQ_SOURCE_HIGH_CURRENT,
                       ME_IRQ_EDGE_NOT_USED,
                       ME_VALUE_NOT_USED,
                       ME_IO_IRQ_START_NO_FLAGS);
    if (err) {
        meErrorGetMessage(err, errTxt, sizeof(errTxt));
        printf("In meIOIrqStart(): %s.", errTxt);
        return err;
    }

	err = meIOIrqSetCallback(noDev,
                             noSubDev,
                             irq_cb,
                             NULL,
                             ME_IO_IRQ_WAIT_NORMAL_STATUS);
    if (err) {
        meErrorGetMessage(err, errTxt, sizeof(errTxt));
        printf("In meIOIrqSetCallback(): %s.", errTxt);
        return err;
    }

    return 0;
}


void
GetStreamAI(int NoDev, int NoSubDev)
{
	int ret_err = 0;

	double minVal = 0.0;
	double maxVal = 0.0;
	int unit = ME_UNIT_VOLT;
	int maxData = 0;

	char txtError[1024] = "";

	double val = 0.0;

	int totalCount = 0;
	int AIBufferCount = AIBufSize;
	int AIBuffer[DataReadSize];
	meIOStreamConfig_t ConfigList[1];
	meIOStreamTrigger_t Trigger;

	meIOStreamStart_t StartList;
	meIOStreamStop_t StopList;

//Prepare triggers
	Trigger.iAcqStartTrigType = ME_TRIG_TYPE_SW;
	Trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_NONE;
	Trigger.iAcqStartTrigChan = ME_TRIG_CHAN_DEFAULT;
	Trigger.iAcqStartTicksLow = 66;//2us
	Trigger.iAcqStartTicksHigh = 0;
	Trigger.iScanStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iScanStartTicksLow = 66;//2us
	Trigger.iScanStartTicksHigh = 0;
	Trigger.iConvStartTrigType = ME_TRIG_TYPE_TIMER;
	Trigger.iConvStartTicksLow = 33000;	//1kHz
// 	Trigger.iConvStartTicksLow = 16500;	//2kHz
// 	Trigger.iConvStartTicksLow = 6600;	//5kHz
// 	Trigger.iConvStartTicksLow = 3300;	//10kHz
// 	Trigger.iConvStartTicksLow = 1650;	//20kHz
// 	Trigger.iConvStartTicksLow = 660;	//50kHz
//PCI only
// 	Trigger.iConvStartTicksLow = 330;	//100kHz
// 	Trigger.iConvStartTicksLow = 165;	//200kHz
// 	Trigger.iConvStartTicksLow = 66;	//500kHz
	Trigger.iConvStartTicksHigh = 0;
	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_COUNT;
	Trigger.iAcqStopCount = DataBlockSize;
	Trigger.iFlags = ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS;

//Prepare config
	ConfigList[0].iChannel = 0;
	ConfigList[0].iStreamConfig = 7;
	ConfigList[0].iRef = ME_REF_AI_GROUND;
	ConfigList[0].iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;

//Prepare start
	StartList.iDevice = NoDev;
	StartList.iSubdevice = NoSubDev;
	StartList.iStartMode = ME_START_MODE_BLOCKING;
	StartList.iTimeOut = 0;
	StartList.iFlags = ME_IO_STREAM_START_TYPE_NO_FLAGS;
	StartList.iErrno = ME_VALUE_NOT_USED;

//Prepare stop
	StopList.iDevice = NoDev;
	StopList.iSubdevice = NoSubDev;
	StopList.iStopMode = ME_STOP_MODE_IMMEDIATE;
	StopList.iFlags = ME_IO_STREAM_STOP_TYPE_NO_FLAGS;
	StopList.iErrno = ME_VALUE_NOT_USED;

	meQueryRangeInfo(NoDev, NoSubDev, 7, &unit, &minVal, &maxVal, &maxData);
    printf("Range: min=%f max=%f.\n", minVal, maxVal);

	printf("Configure AI\n");
	ret_err = meIOStreamConfig(NoDev, NoSubDev, ConfigList, 1, &Trigger, AIBufSize, ME_IO_STREAM_CONFIG_NO_FLAGS);
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

	while (totalCount<DataBlockSize)
	{
		AIBufferCount=AIBufSize;

		ret_err = meIOStreamRead(NoDev, NoSubDev, ME_READ_MODE_BLOCKING, AIBuffer, &AIBufferCount, ME_IO_STREAM_READ_NO_FLAGS);
		if (ret_err)
		{
			goto EXIT;
		}
		meUtilityDigitalToPhysical(minVal, maxVal, maxData, *AIBuffer, 0, 0, &val);
		printf ("\tSample 0x%x => %f\n", totalCount, val);
		totalCount += AIBufferCount;
	}

	printf("Stop AI\n");
	meIOStreamStop(&StopList, 1, ME_IO_STREAM_STOP_NO_FLAGS);
	printf("AI stoped.\n");

EXIT:
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}
}


int
main(int argc, char *argv[])
{
	int LibVer;
	int DrvVer;

	int NoDev;
	int NoSubDev;

	int DevCnt;
	int SubCnt;
	int ret_err;

	printf("Hello, AI stream test!\n");

	if (meOpen(0))
		exit (EXIT_FAILURE);

	if (!meQueryVersionLibrary(&LibVer))
		printf("Library version: 0x%08x.\n", LibVer);

	if (!meQueryVersionMainDriver(&DrvVer))
		printf("Main driver version: 0x%08x.\n", DrvVer);

	if (!meQueryNumberDevices(&NoDev))
		printf("%d device%s in system.\n", NoDev, (NoDev>1)?"s":"");

	if (!NoDev)
		return 0;

	printf("\n");
	for (DevCnt=0; DevCnt<NoDev; ++DevCnt)
	{
		if (meQueryNumberSubdevices(DevCnt, &NoSubDev))
		{
			break;
		}

		SubCnt=0;
		while (SubCnt < NoSubDev)
		{
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_AI, ME_SUBTYPE_STREAMING, &SubCnt);
			if (!ret_err)
			{
                int err;

				printf("Device [%d,%d] is an AI.\n", DevCnt, SubCnt);
                meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);

                err = prepareOverLoad(DevCnt, SubCnt);
                if (err) {
                    printf("In prepareOverLoad(): %i\n", err);
                    return 1;
                } else {
                    GetStreamAI(DevCnt, SubCnt);
                }

                printf("Reset subdevice.\n");
				meIOResetSubdevice(DevCnt, SubCnt, ME_VALUE_NOT_USED);
			}
			else
			{
				break;
			}
			SubCnt++;
		}
	}

    printf("Close driver system.\n");
	meClose(0);
	printf("Boodbye, AI stream test!\n");

	return EXIT_SUCCESS;
}



