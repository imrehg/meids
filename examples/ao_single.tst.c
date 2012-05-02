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

void SetAO (int NoDev, int NoSubDev, int NoChannel);
void GetAO (int NoDev, int NoSubDev, int NoChannel);

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

	printf("Hello, AO single test!\n");

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
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_AO, ME_SUBTYPE_ANY, &SubCnt);
			if (!ret_err)
			{
				printf("Device [%d,%d] is an AO.\n", DevCnt, SubCnt);
				if (meQueryNumberChannels(DevCnt, SubCnt, &NoChan))
				{
					NoChan = 0;
				}

				for (ChanCnt=0; ChanCnt<NoChan; ++ChanCnt)
				{
					SetAO (DevCnt, SubCnt, ChanCnt);
				}

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
	printf("Boodbye, AO single test!\n");

	return EXIT_SUCCESS;
}

void SetAO (int NoDev, int NoSubDev, int NoChannel)
{
#define STEPS	9
	meIOSingle_t single;
	char txtError[1024] = "";
	int err;
	int i;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	double step;

	//Using range 0
	err = meQueryRangeInfo(NoDev, NoSubDev, 0, &iUnit, &dMin, &dMax, &iMaxData);
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
		return;
	}
	step = (dMax-dMin)/(STEPS-1);

	err = meIOSingleConfig(NoDev, NoSubDev, NoChannel, 0, ME_REF_AO_GROUND, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, ME_TRIG_EDGE_NONE, ME_VALUE_NOT_USED);
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
		return;
	}

	single.iDevice=NoDev;
	single.iSubdevice=NoSubDev;
	single.iChannel= NoChannel;
	single.iDir=ME_DIR_OUTPUT;
	single.iTimeOut=ME_VALUE_NOT_USED;
	single.iFlags=ME_IO_SINGLE_TYPE_NO_FLAGS;
	single.iErrno=0;

	for (i = 0; i < STEPS; ++i)
	{
		meUtilityPhysicalToDigital(dMin, dMax, iMaxData, dMin + (step * i), &single.iValue);
		err = meIOSingle(&single, 1, ME_IO_SINGLE_NO_FLAGS);
		if (err)
		{
			meErrorGetMessage(err, txtError, 1024);
			printf ("Error: %d => %s\n", err, txtError);
			continue;
		}
		else
		{
			printf("Device [%d:%d/%d] SET:%f(0x%x) ", NoDev, NoSubDev, NoChannel, dMin + (step * i), single.iValue);
			GetAO (NoDev, NoSubDev, NoChannel);
		}
		sleep (1);
	}
}

void GetAO (int NoDev, int NoSubDev, int NoChannel)
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
	err = meQueryRangeInfo(NoDev, NoSubDev, 0, &iUnit, &dMin, &dMax, &iMaxData);
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
		printf("GET:%f(0x%x)\n", value, single.iValue);
	}
}
