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
void GetDI (int NoDev, int NoSubDev, const char* Name);

int main(int argc, char *argv[])
{
	int NoDev;
	int NoSubDev;

	int LibVer;
	int DrvVer;
	int DevCnt;
	int SubCnt;
	int ret_err = 0;

	printf("Hello, DI test!\n");

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
			ret_err = meQuerySubdeviceByType(DevCnt, SubCnt, ME_TYPE_DI, ME_SUBTYPE_ANY, &SubCnt);
			if (!ret_err)
			{
				GetDI (DevCnt, SubCnt, "DI");
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
				GetDI (DevCnt, SubCnt, "DIO");
			}
			else
			{
				break;
			}
			SubCnt++;
		}
	}

	meClose(0);
	printf("Boodbye, DI test!\n");

	return EXIT_SUCCESS;
}

void GetDI (int NoDev, int NoSubDev, const char* Name)
{
	meIOSingle_t single;
	char txtError[1024] = "";
	int err;

	err = meIOSingleConfig(NoDev, NoSubDev, 0, ME_SINGLE_CONFIG_DIO_INPUT, ME_REF_NONE, ME_TRIG_CHAN_NONE, ME_TRIG_TYPE_NONE, ME_TRIG_EDGE_NONE, ME_IO_SINGLE_NO_FLAGS);
	if (err)
	{
		goto EXIT;
	}

	single.iDevice=NoDev;
	single.iSubdevice=NoSubDev;
	single.iChannel=0;
	single.iDir=ME_DIR_INPUT;
	single.iValue=0x00;
	single.iTimeOut=ME_VALUE_NOT_USED;
	single.iFlags=ME_IO_SINGLE_TYPE_NO_FLAGS;
	single.iErrno=0;

	err = meIOSingle(&single, 1, ME_IO_SINGLE_NO_FLAGS);

EXIT:
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