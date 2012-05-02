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

void test_meIOStreamFrequencyToTicks(int dev);
void GetStreamAI (int dev);

int main(int argc, char *argv[]){
	int err;
	int i;
	char err_msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	char description[ME_DEVICE_DESCRIPTION_MAX_COUNT] = {0};
	char name_device[ME_DEVICE_NAME_MAX_COUNT] = {0};
	char name_driver[ME_DEVICE_DRIVER_NAME_MAX_COUNT] = {0};
	int d_version;
	int l_version;
	int n_devices;

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
		/*For  each device get:
		 - device name
		 - device description
		 - driver name
		 - driver version
		 - number sub-devices
		 */
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

		test_meIOStreamFrequencyToTicks(i);
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


void test_meIOStreamFrequencyToTicks(int dev)
{
	int i;
	int err;
	char err_msg[ME_ERROR_MSG_MAX_COUNT] = {0};

	double pdFrequency;
	int piTicksLow;
	int piTicksHigh;

	meIOResetSubdevice(dev, 4, ME_VALUE_NOT_USED);
	printf("%d meIOStreamFrequencyToTicks:\n", dev);
	for (i=0; i<10; ++i)
	{
		pdFrequency = 500000;

		err = meIOStreamFrequencyToTicks(dev, 4, ME_TIMER_SCAN_START, &pdFrequency, &piTicksLow, &piTicksHigh, ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS);
		meErrorGetMessage(err, err_msg, sizeof(err_msg));
		printf("    %d: err=%d -> %s\n", i, err, err_msg);
		GetStreamAI (dev);
	}
}

#define DataBlockSize	0x100
#define DataReadSize	0x100
#define AIBufSize		0x100

void GetStreamAI (int dev)
{
	int ret_err = 0;
	char txtError[1024] = "";

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
	Trigger.iConvStartTicksLow = 66;	//1kHz
	Trigger.iConvStartTicksHigh = 0;
	Trigger.iScanStopTrigType = ME_TRIG_TYPE_NONE;
	Trigger.iScanStopCount = 0;
	Trigger.iAcqStopTrigType = ME_TRIG_TYPE_COUNT;
	Trigger.iAcqStopCount = DataBlockSize;
	Trigger.iFlags = ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS;

//Prepare config
	ConfigList[0].iChannel = 0;
	ConfigList[0].iStreamConfig = 0;
	ConfigList[0].iRef = ME_REF_AI_GROUND;
	ConfigList[0].iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;

//Prepare start
	StartList.iDevice = dev;
	StartList.iSubdevice = 4;
	StartList.iStartMode = ME_START_MODE_BLOCKING;
	StartList.iTimeOut = 0;
	StartList.iFlags = ME_IO_STREAM_START_TYPE_NO_FLAGS;
	StartList.iErrno = ME_VALUE_NOT_USED;

//Prepare stop
	StopList.iDevice = dev;
	StopList.iSubdevice = 4;
	StopList.iStopMode = ME_STOP_MODE_IMMEDIATE;
	StopList.iFlags = ME_IO_STREAM_STOP_TYPE_NO_FLAGS;
	StopList.iErrno = ME_VALUE_NOT_USED;

	ret_err = meIOStreamConfig(dev, 4, ConfigList, 1, &Trigger, AIBufSize, ME_IO_STREAM_CONFIG_NO_FLAGS);
	if (ret_err)
	{
		goto EXIT;
	}

	ret_err = meIOStreamStart(&StartList, 1, ME_VALUE_NOT_USED);
	if (ret_err)
	{
		goto EXIT;
	}
	printf("Stream\n");

	while (totalCount<DataBlockSize)
	{
		AIBufferCount=AIBufSize;

		ret_err = meIOStreamRead(dev, 4, ME_READ_MODE_BLOCKING, AIBuffer, &AIBufferCount, ME_IO_STREAM_READ_NO_FLAGS);
		if (ret_err)
		{
			goto EXIT;
		}
		totalCount += AIBufferCount;
	}

// 	meIOStreamStop(&StopList, 1, ME_IO_STREAM_STOP_NO_FLAGS);
// 	printf("AI stoped.\n");

EXIT:
	if (ret_err)
	{
		meErrorGetMessage(ret_err, txtError, 1024);
		printf ("Error: %d => %s\n", ret_err, txtError);
	}
}
