/***************************************************************************
 *   Copyright (C) 2009 by Krzysztof Gantzke       k.gantzke@meilhaus.de   *
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
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "medriver.h"

int main(int argc, char *argv[]);
void GetDO (int NoDev, int NoSubDev, const char* Name);
void SetDO (int NoDev, int NoSubDev, const char* Name);
int GetOutputType(int NoDev, int NoSubDev, const char* Name);

int main(int argc, char *argv[])
{
	int NoDev;
	int NoSubDev;

	int LibVer;
	int DrvVer;
	int DevCnt;
	int SubCnt;
	int ret_err = 0;

	printf("Hello, DO test!\n");

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
		meQueryNumberSubdevices(DevCnt, &NoSubDev);
		SubCnt=0;
		while (SubCnt < NoSubDev)
		{
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_DO, ME_SUBTYPE_ANY, &SubCnt);
			if (!ret_err)
			{
				SetDO (DevCnt, SubCnt, "DO");
			}
			else
			{
				break;
			}
			SubCnt++;
		}

		SubCnt=0;
		while (SubCnt < NoSubDev)
		{
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_DIO, ME_SUBTYPE_ANY, &SubCnt);
			if (!ret_err)
			{
				SetDO (DevCnt, SubCnt, "DIO");
			}
			else
			{
				break;
			}
			SubCnt++;
		}
	}

	meClose(0);
	printf("Boodbye, DO test!\n");

	return EXIT_SUCCESS;
}

void SetDO (int NoDev, int NoSubDev, const char* Name)
{
	meIOSingle_t single;
	int NoChannels;
	int Caps;
	int DIO_Output_Type;
	char txtError[1024] = "";
	int err;
	int i;

	err = meQueryNumberChannels(NoDev, NoSubDev, &NoChannels);
	if (err)
	{
		goto EXIT;
	}

	err = meQuerySubdeviceCaps(NoDev, NoSubDev, &Caps);
	if (err)
	{
		goto EXIT;
	}

	if (Caps & ME_CAPS_DIO_SINK_SOURCE)
	{
		DIO_Output_Type = GetOutputType(NoDev, NoSubDev, Name);
	}
	else
	{
		DIO_Output_Type = ME_SINGLE_CONFIG_DIO_OUTPUT;
	}

	err = meIOSingleConfig(NoDev, NoSubDev, 0, DIO_Output_Type, ME_REF_NONE, ME_TRIG_CHAN_NONE, ME_TRIG_TYPE_NONE, ME_TRIG_EDGE_NONE, ME_IO_SINGLE_NO_FLAGS);
	if (err)
	{
		goto EXIT;
	}

	single.iDevice=NoDev;
	single.iSubdevice=NoSubDev;
	single.iChannel=0;
	single.iDir=ME_DIR_OUTPUT;
	single.iTimeOut=ME_VALUE_NOT_USED;
	single.iFlags=ME_IO_SINGLE_TYPE_NO_FLAGS;
	single.iErrno=0;

	for (i=0; i<NoChannels; ++i)
	{
		single.iValue = 0x01<<i;
		err = meIOSingle(&single, 1, ME_IO_SINGLE_NO_FLAGS);
		if (err)
		{
			goto EXIT;
		}
		else
		{
			GetDO (NoDev, NoSubDev, Name);
		}
		sleep (1);
	}

EXIT:
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
	}
	printf("\n");
}

void GetDO (int NoDev, int NoSubDev, const char* Name)
{
	meIOSingle_t single;
	char txtError[1024] = "";
	int err;

	single.iDevice=NoDev;
	single.iSubdevice=NoSubDev;
	single.iChannel=0;
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
		printf("Device %d: %s%d value=0x%x\n", NoDev, Name, NoSubDev, single.iValue);
	}
}

int GetOutputType(int NoDev, int NoSubDev, const char* Name)
{
	char DO_type;
	int ret = ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE;
	printf("Device %d: %s%d has SINK/SOURCE output driver.\n", NoDev, Name, NoSubDev);
	printf("Type '1' for SINK\n");
	printf("Type '2' for SOURCE\n");

	DO_type = getchar();
 	while ((getchar()) != 10);

	switch (DO_type)
	{
		case '1':
			ret = ME_SINGLE_CONFIG_DIO_SINK;
			printf ("SINK output.\n");
			break;

		case '2':
			ret = ME_SINGLE_CONFIG_DIO_SOURCE;
			printf ("SOURCE output.\n");
			break;

		default:
			printf ("High impedance output.\n");
	}

	return ret;
}