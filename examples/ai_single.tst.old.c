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

#define DataBlockSize	10000000
#define DataReadSize	0x1000
#define AIBufSize	0x400
int main(int argc, char *argv[])
{
#define CHANNEL	0
#define RANGE	0

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

	meIOSingle_t SingleList;

	char txtError[1024] = "";

	double val = 0.0;

	printf("Hello, AI single test!\n");

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

		ret_err = meQuerySubdeviceByType(i, -1, ME_TYPE_AI, ME_SUBTYPE_ANY, &NoSubDev);
		if (!ret_err)
		{
			printf("Device [%d,%d] is an AI\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		ret_err = EXIT_SUCCESS;
		printf("AI not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	ret_err = meQueryRangeInfo(NoDev, NoSubDev, RANGE, &unit, &minVal, &maxVal, &maxData);
	if (ret_err)
	{
		printf("EXIT %d\n", __LINE__);
		goto EXIT;
	}

	ret_err = meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		printf("EXIT %d\n", __LINE__);
		goto EXIT;
	}
	printf("Range: [%0.2f;%0.2f]\n", minVal, maxVal);

	ret_err = meIOSingleConfig(NoDev, NoSubDev, CHANNEL, RANGE, ME_REF_AI_GROUND, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, ME_TRIG_EDGE_NONE, ME_VALUE_NOT_USED);
	printf("meIOSingleConfig() = %d\n", ret_err);
	if (ret_err)
	{
		printf("EXIT %d\n", __LINE__);
		goto EXIT;
	}

	SingleList.iDevice = NoDev;
	SingleList.iSubdevice = NoSubDev;
	SingleList.iChannel = CHANNEL;
	SingleList.iDir = ME_DIR_INPUT;
	SingleList.iValue = 0;
	SingleList.iTimeOut = 0;
	SingleList.iFlags = ME_VALUE_NOT_USED;
	SingleList.iErrno = 0;
	ret_err = meIOSingle(&SingleList, 1, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		printf("EXIT %d\n", __LINE__);
		goto EXIT;
	}

	if (SingleList.iErrno)
	{
		ret_err = SingleList.iErrno;
		printf("EXIT %d\n", __LINE__);
		goto EXIT;
	}


	meUtilityDigitalToPhysical(minVal, maxVal, maxData, SingleList.iValue, 0, 0, &val);
	printf ("Value: 0x%04x %f\n", SingleList.iValue, val);

EXIT:
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}

	meClose(0);
	printf("Boodbye, AI single test!\n");

	return EXIT_SUCCESS;
}
