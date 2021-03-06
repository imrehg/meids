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
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

#include "medriver.h"

int callbackIRQ(int device, int subdevice, int channel, int count, int value, void* context, int status);
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

	char txtError[1024] = "";


	printf("Hello, IRQ callback test!\n");

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

		ret_err = meQuerySubdeviceByType(i, -1, ME_TYPE_EXT_IRQ, ME_SUBTYPE_SINGLE, &NoSubDev);
		if (!ret_err)
		{
			printf("Device [%d,%d] is an EXTERNAL IRQ\n", i, NoSubDev);
			break;
		}
	}

	if (i == NoDev)
	{
		ret_err = EXIT_SUCCESS;
		printf("EXTERNAL IRQ not found in system!\n");
		goto EXIT;
	}

	NoDev = i;

	meIOResetSubdevice(NoDev, NoSubDev, ME_VALUE_NOT_USED);

	printf("Register 2 callbacks.\n");
	ret_err = meIOIrqSetCallback(NoDev, NoSubDev, callbackIRQ, NULL, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}
	ret_err = meIOIrqSetCallback(NoDev, NoSubDev, callbackIRQ, NULL, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Start IRQ\n");
	ret_err = meIOIrqStart(NoDev, NoSubDev, 0, ME_IRQ_SOURCE_DIO_LINE, ME_IRQ_EDGE_RISING, ME_VALUE_NOT_USED, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}

	printf("Press 'ENTER' to continue.\n");
	printf("Wait 1min for interrupts.\n");
	sleep(60);

	printf("Stop IRQ\n");
	meIOIrqStop(NoDev, NoSubDev, 0, ME_VALUE_NOT_USED);

EXIT:
	printf("Deregister callbacks.\n");
	ret_err = meIOIrqSetCallback(NoDev, NoSubDev, NULL, NULL, ME_VALUE_NOT_USED);

	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}

	sleep(1);
	meClose(0);
	printf("Boodbye, IRQ callback test!\n");

	sleep(1);
	return EXIT_SUCCESS;
}

int callbackIRQ(int device, int subdevice, int channel, int count, int value, void* context, int status)
{
	printf("%ld IRQ callback! count=%d value=0x%08x status=%d\n", pthread_self(), count, value, status);

	return EXIT_SUCCESS;

}
