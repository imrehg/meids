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

#include "medriver.h"
#include "meutility.h"

#define MAXDataSize 256
#define RANGE 0
/// LINUX ONLY!
int main(int argc, char *argv[])
{
	int NoDev = -1;
	int NoSubDev = -1;
	int LibVer = -1;
	int DrvVer = -1;
	int i = -1;
	int err = 0;

	int VendorId = -1;
	int DeviceId = -1;
	int SerialNo = -1;
	int BusType = -1;
	int BusNo = -1;
	int DevNo = -1;
	int FuncNo = -1;
	int Plugged = -1;

	char txtError[1024] = "";

	int DataSize = MAXDataSize;

	double AIBuffer[MAXDataSize];

	printf("Hello, AI single scan test!\n");

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

		err = meQuerySubdeviceByType(i, -1, ME_TYPE_AI, ME_SUBTYPE_STREAMING, &NoSubDev);
		if (!err)
		{
			printf("Device [%d,%d] is an STREAMING AI\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		err = EXIT_SUCCESS;
		printf("STREAMING AI not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	meIOResetSubdevice(NoDev, NoSubDev, ME_IO_RESET_SUBDEVICE_NO_FLAGS);

	printf("Scan AI lines.\n");
	err = meSingleScanRead(NoDev, NoSubDev, AIBuffer, &DataSize, RANGE, ME_TRIG_TYPE_SW, ME_TRIG_EDGE_NONE, 0, ME_NO_FLAGS);
	if (!err)
	{
		for (i=0; i<DataSize; i++)
		{
			printf("%d: %fV\n", i, AIBuffer[i]);
		}
	}

EXIT:
	if (err)
	{
		meErrorGetMessage(err, txtError, 1024);
		printf ("Error: %d => %s\n", err, txtError);
	}

	meClose(0);
	printf("Boodbye, AI single scan test!\n");

	return EXIT_SUCCESS;
}
