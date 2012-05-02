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

	meIOSingle_t SingleList[4];

	printf("Hello, AO single synchronous start test!\n");

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

		ret_err = meQuerySubdeviceByType(i, -1, ME_TYPE_AO, ME_SUBTYPE_ANY, &NoSubDev);
		if (!ret_err)
		{
			printf("Device [%d,%d] is an AO\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		ret_err = EXIT_SUCCESS;
		printf("AO not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	for (i=0; i<4; ++i)
	{
		meIOResetSubdevice(NoDev, NoSubDev + i, ME_VALUE_NOT_USED);
	}
	printf("Configure outputs\n");
	for (i=0; i<4; ++i)
	{
		meIOSingleConfig(NoDev, NoSubDev + i, 0, 0, ME_REF_AO_GROUND, ME_TRIG_CHAN_SYNCHRONOUS, ME_TRIG_TYPE_SW, ME_TRIG_EDGE_NONE, ME_VALUE_NOT_USED);

		SingleList[i].iDevice = NoDev;
		SingleList[i].iSubdevice = NoSubDev + i;
		SingleList[i].iChannel = 0;
		SingleList[i].iDir = ME_DIR_OUTPUT;
		SingleList[i].iTimeOut = 0;
		SingleList[i].iFlags = ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING;
	}
	SingleList[3].iFlags = ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS;

	printf("Press ENTER to start.\n");
	getchar();

	printf("Write outputs\n");
	for (i=0; i<4; ++i)
	{
		SingleList[i].iValue = 0x0000;
	}
	meIOSingle(SingleList, 4, ME_VALUE_NOT_USED);

	for (i=0; i<4; ++i)
	{
		SingleList[i].iValue = 0xFFFF;
	}
	meIOSingle(SingleList, 4, ME_VALUE_NOT_USED);

	for (i=0; i<4; ++i)
	{
		SingleList[i].iValue = 0x8000;
	}
	meIOSingle(SingleList, 4, ME_VALUE_NOT_USED);

EXIT:
	meClose(0);
	printf("Boodbye, AO single synchronous start test!\n");

	return EXIT_SUCCESS;
}
