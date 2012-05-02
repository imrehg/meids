/*
 * Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * Source File : query.tst.c
 *
 * Author      : GG (Guenter Gebhardt)
 * Author      : KG (Krzysztof Gantzke)     <k.gantzke@meilhaus.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <medriver.h>
char* meDevTypes[] =
{
	"INVALID",
	"ANALOG OUTPUT",
	"ANALOG INPUT",
	"DIGITAL INPUT/OUTPUT",
	"DIGITAL OUTPUT",
	"DIGITAL INPUT",
	"COUNTER",
	"EXTERNAL IRQ",
	"FREQUENCY INPUT/OUTPUT",
	"FREQUENCY OUTPUT",
	"FREQUENCY INPUT"
};

char* meSubDevTypes[] =
{
	"INVALID",
	"SINGLE",
	"STREAM",
	"8254",
	"ANY"
};

char* typetostr(int number);
char* subtypetostr(int number);

int main(int argc, char *argv[]){
	int err;
	int i, j, k;
	char err_msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	char description[ME_DEVICE_DESCRIPTION_MAX_COUNT] = {0};
	char name_device[ME_DEVICE_NAME_MAX_COUNT] = {0};
	char name_driver[ME_DEVICE_DRIVER_NAME_MAX_COUNT] = {0};
	int d_version;
	int l_version;
	int n_devices;
	int n_subdevices;
	int n_channels;
	int n_ranges;
	int unit;
	double min;
	double max;
	int max_data;
	int type;
	int subtype;

	//Initialization of ME-iDS
	err = meOpen(ME_OPEN_NO_FLAGS);
	if (err){
		meErrorGetMessage(err, err_msg, sizeof(err_msg));
		fprintf(stderr, "In meOpen(): %s\n", err_msg);
		return 1;
	}
	//Get library version
	err = meQueryVersionLibrary(&l_version);
	if (err){
		meErrorGetMessage(err, err_msg, sizeof(err_msg));
		fprintf(stderr, "In meQueryLibraryVersion(): %s\n", err_msg);
		return 1;
	}
	printf("Library version is 0x%X\n", l_version);
	//Get number of registered devices
	err = meQueryNumberDevices(&n_devices);
	if (err){
		meErrorGetMessage(err, err_msg, sizeof(err_msg));
		fprintf(stderr, "In meQueryNumberDevices(): %s\n", err_msg);
		return 1;
	}

	if (n_devices >0)
	{
		//Get main driver version
		err = meQueryVersionMainDriver(&d_version);
		if (err){
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryDriverVersion(): %s\n", err_msg);
			return 1;
		}

		printf("Main driver version is 0x%X\n", d_version);
	}

	printf("%d devices detected by driver system\n", n_devices);

	for(i = 0; i < n_devices; i++)
	{
// 		For  each device get:
// 		 - device name
// 		 - device description
// 		 - driver name
// 		 - driver version
// 		 - number sub-devices

		printf("\n");
		printf("Device %d:\n", i);
		printf("=========\n");
		err = meQueryNameDevice(i, name_device, sizeof(name_device));
		if (err)
		{
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryDeviceName(): %s\n", err_msg);
			return 1;
		}
		printf("Device name is %s\n", name_device);

		err = meQueryNameDeviceDriver(i, name_driver, sizeof(name_driver));
		if (err)
		{
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryNameDeviceDriver(): %s\n", err_msg);
			return 1;
		}
		printf("Driver name is %s\n", name_driver);

		err = meQueryDescriptionDevice(i, description, sizeof(description));
		if (err)
		{
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryDescriptionDevice(): %s\n", err_msg);
			return 1;
		}
		printf("Device description: %s\n", description);

		err = meQueryVersionDeviceDriver(i, &d_version);
		if (err)
		{
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryDriverName(): %s\n", err_msg);
			return 1;
		}
		printf("Device driver version is 0x%X\n", d_version);

		err = meQueryNumberSubdevices(i, &n_subdevices);
		if (err)
		{
			meErrorGetMessage(err, err_msg, sizeof(err_msg));
			fprintf(stderr, "In meQueryNumberSubdevices(): %s\n", err_msg);
			return 1;
		}
		printf("%d subdevices available:\n", n_subdevices);

		for(j = 0; j < n_subdevices; j++)
		{
// 			For each sub-device get:
// 			 - type
// 			 - sub-type

			err = meQuerySubdeviceType(i, j, &type, &subtype);
			if (err)
			{
				meErrorGetMessage(err, err_msg, sizeof(err_msg));
				fprintf(stderr, "In meQuerySubdeviceType(): %s\n", err_msg);
				return 1;
			}
			printf("\tSubdevice %d is of type %s (0x%X) and subtype %s (0x%X)\n", j, typetostr(type), type , subtypetostr(subtype), subtype);

			err = meQueryNumberChannels(i, j, &n_channels);
			if (err)
			{
				meErrorGetMessage(err, err_msg, sizeof(err_msg));
				fprintf(stderr, "In meQueryNumberChannels(): %s\n", err_msg);
				return 1;
			}
			printf("\t\tSubdevice %d has %d channels\n", j, n_channels);

			if (type == ME_TYPE_AI || type == ME_TYPE_AO)
			{
				//For  AI and AO sub-device get number ranges
				err = meQueryNumberRanges(i, j, ME_UNIT_ANY, &n_ranges);
				if (err)
				{
					meErrorGetMessage(err, err_msg, sizeof(err_msg));
					fprintf(stderr, "In meQueryNumberRanges(): %s\n", err_msg);
					return 1;
				}
				printf("\t\t\tSubdevice %d has %d ranges:\n", j, n_ranges);

				for(k = 0; k < n_ranges; k++)
				{
					err = meQueryRangeInfo(
							i,
							j,
							k,
							&unit,
							&min,
							&max,
							&max_data);
					if (err)
					{
						meErrorGetMessage(err, err_msg, sizeof(err_msg));
						fprintf(stderr, "In meQueryNumberRanges(): %s\n", err_msg);
						return 1;
					}
					printf("\t\t\t\tRange %d: Unit = 0x%X, Min = %lf, Max = %lf, Max Data = %d\n", k, unit, min, max, max_data);
				}
			}
		}
	}

	//Close ME-iDS
	err = meClose(ME_CLOSE_NO_FLAGS);
	if (err)
	{
		meErrorGetMessage(err, err_msg, sizeof(err_msg));
		fprintf(stderr, "In meClose(): %s\n", err_msg);
		return 1;
	}

	return 0;
}

//Helpers
char* typetostr(int number)
{
	if ((number<ME_TYPE_AO) || (number>ME_TYPE_FREQ_I))
	{
		return meDevTypes[0];
	}

	return meDevTypes[number - ME_TYPE_AO + 1];
}

char* subtypetostr(int number)
{
	if ((number<ME_SUBTYPE_SINGLE) || (number>ME_SUBTYPE_ANY))
	{
		return meSubDevTypes[0];
	}
	return meSubDevTypes[number - ME_SUBTYPE_SINGLE + 1];

}
