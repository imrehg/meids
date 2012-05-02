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

void GetAI (int NoDev, int NoSubDev, int NoChannel);

int main(int argc, char *argv[])
{
	int LibVer;
	int DrvVer;

	int NoDev;
	int NoSubDev;
	int NoChan;

	int DevCnt;
	int SubCnt;
	int ChanCnt;
	int ret_err;

	printf("Hello, AI single test!\n");

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
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_AI, ME_SUBTYPE_ANY, &SubCnt);
			if (!ret_err)
			{
				printf("Device [%d,%d] is an AI.\n", DevCnt, SubCnt);
                GetAI (DevCnt, SubCnt, ChanCnt);
				meIOResetSubdevice(DevCnt, SubCnt, ME_VALUE_NOT_USED);

			}
			else
			{
				break;
			}
			SubCnt++;
		}
	}

	meClose(0);
	printf("Boodbye, AI single test!\n");

	return EXIT_SUCCESS;
}

void GetAI (int NoDev, int NoSubDev, int NoChannel)
{
	meIOSingle_t single;
	char txtError[1024] = "";
	int err;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	double value;

	//Using range 0
	err = meQueryRangeInfo(NoDev, NoSubDev, 7, &iUnit, &dMin, &dMax, &iMaxData);
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
		return;
	}

    printf("Range min=%f max=%f\n", dMin, dMax);

	err = meIOSingleConfig(NoDev, NoSubDev, NoChannel, 7, ME_REF_AI_GROUND, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, ME_TRIG_EDGE_NONE, ME_VALUE_NOT_USED);
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
		return;
	}

	single.iDevice=NoDev;
	single.iSubdevice=NoSubDev;
	single.iChannel=NoChannel;
	single.iDir=ME_DIR_INPUT;
	single.iValue=0x00;
	single.iTimeOut=ME_VALUE_NOT_USED;
	single.iFlags=ME_IO_SINGLE_TYPE_NO_FLAGS;
	single.iErrno=0;

	err = meIOSingle(&single, 1, ME_IO_SINGLE_NO_FLAGS);
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
	}
	else
	{
		meUtilityDigitalToPhysical(dMin, dMax, iMaxData, single.iValue, ME_MODULE_TYPE_MULTISIG_NONE, 0, &value);
		printf("Device [%d:%d/%d] GET:%f(0x%x)\n", NoDev, NoSubDev, NoChannel, value, single.iValue);
	}
}
