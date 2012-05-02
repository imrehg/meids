/*
 * Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * Source File : aoSingle.c
 * Author      : GG (Guenter Gebhardt)  <g.gebhardt@meilhaus.de>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <medriver.h>


void usage(void){
	printf("aoSingle - Example program for analog output\n\n");
	printf("Usage: aoSingle [arguments]\n\n");
	printf("Arguments:\n\n");
	printf("-h                     Print this help and exit.\n");
	printf("-d <device number>     Use <device number> device (Default is 0).\n");
	printf("-s <subdevice number>  Use <subdevice number> subdevice (Default is the first analog output subdevice).\n");
	printf("-c <channel number>    Use <channel number> channel (Default is 0).\n");
	printf("-r <range number>      Use <range number> range (Default is the first fitting for value).\n");
	printf("-u <unit>              Possible values are 'Any', 'Volt' (Default) and 'Ampere'.\n");
	printf("-v <value>             Write pysical value <value> to analog output channel.\n");
	printf("-t <trigger type>      Possible values are 'SW' (Default) and 'HWD'.\n");
	printf("-e <trigger edge>      Configure trigger edge.\n");
	printf("                       Valid only for software trigger.\n");
	printf("                       Possible Values are 'Rising' (Default), 'Falling'and 'Any'.\n");
	printf("-o <time out>          Set time out to <time out> ms (Default is 10000 ms).\n");
}


int main(int argc, char *argv[]){
	int err;
	int c;
	char msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	meIOSingle_t *list;
	int device = 0;
	int subdevice = -1;
	int channel = 0;
	double value = 5.0;
	int single_config = -1;
	int unit = ME_UNIT_VOLT;
	double min;
	double max;
	int max_data;
	int trigger_type = ME_TRIG_TYPE_SW;
	int trigger_edge = ME_TRIG_EDGE_NONE;
	int time_out = 10000;

	printf("\n");
	/* Parse the command line arguments */
	while((c = getopt(argc, argv, "hd:s:c:r:v:u:t:e:o:")) != -1){
		switch(c){
			case 'h':
				usage();
				exit(0);
			case 'd':
				device = atoi(optarg);
				break;
			case 's':
				subdevice = atoi(optarg);
				break;
			case 'c':
				channel = atoi(optarg);
				break;
			case 'r':
				single_config = atoi(optarg);
				break;
			case 'u':
				if(!strcmp(optarg, "Any")){
					unit = ME_UNIT_ANY;
				}
				else if(!strcmp(optarg, "Ampere")){
					unit = ME_UNIT_AMPERE;
				}
				else if(!strcmp(optarg, "Volt")){
					unit = ME_UNIT_VOLT;
				}
				else{
					usage();
					exit(2);
				}
				break;
			case 'v':
				value = atof(optarg);
				break;
			case 't':
				if(!strcmp(optarg, "SW")){
					trigger_type = ME_TRIG_TYPE_SW;
				}
				else if(!strcmp(optarg, "HWD")){
					trigger_type = ME_TRIG_TYPE_EXT_DIGITAL;
					if (trigger_edge == ME_TRIG_EDGE_NONE){
						trigger_edge = ME_TRIG_EDGE_RISING;
					}	
				}
				else{
					usage();
					exit(2);
				}
				break;
			case 'e':
				if(!strcmp(optarg, "Rising")){
					trigger_edge = ME_TRIG_EDGE_RISING;
				}
				else if(!strcmp(optarg, "Falling")){
					trigger_edge = ME_TRIG_EDGE_FALLING;
				}
				else if(!strcmp(optarg, "Any")){
					trigger_edge = ME_TRIG_EDGE_ANY;
				}
				else{
					usage();
					exit(2);
				}
				break;
			case 'o':
				time_out = atof(optarg);
				break;
			default:
				usage();
				exit(2);
		}
	}

	if(optind != argc){
		fprintf(stderr, "%s: No non option arguments are supported.\n", argv[0]);
		usage();
		exit(2);
	}

	printf("Openning MEiDS.\n");
	err = meOpen(0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meOpen(): %s\n", msg);	
		return 1;
	}

	if(subdevice == -1){
		err = meQuerySubdeviceByType(
				device,
				0,
				ME_TYPE_AO,
				ME_SUBTYPE_ANY,
				&subdevice);
		if(err){
			meErrorGetMessage(err, msg, sizeof(msg));
			fprintf(stderr, "In meQuerySubdeviceByType(): %s\n", msg);	
			goto ERROR;
		}
	}

	if(single_config == -1){
		min = value;
		max = value;
		err = meQueryRangeByMinMax(
				device,
				subdevice,
				unit,
				&min,
				&max,
				&max_data,
				&single_config);
		if(err){
			meErrorGetMessage(err, msg, sizeof(msg));
			fprintf(stderr, "In meQueryRangeByMinMax(): %s\n", msg);	
			goto ERROR;
		}
	}
	else{
		err = meQueryRangeInfo(
				device,
				subdevice,
				single_config,
				&unit,
				&min,
				&max,
				&max_data);
		if(err){
			meErrorGetMessage(err, msg, sizeof(msg));
			fprintf(stderr, "In meQueryRangeInfo(): %s\n", msg);	
			goto ERROR;
		}
	}


	list = (meIOSingle_t *) malloc(sizeof(meIOSingle_t));
	if(!list){
		perror("Cannot get buffer for single list");
		exit(1);
	}

	err = meUtilityPhysicalToDigital(
			min,
			max,
			max_data,
			value,
			&list->iValue);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meQuerySubdeviceByType(): %s\n", msg);	
		goto ERROR;
	}

	list->iDevice = device;
	list->iSubdevice = subdevice;
	list->iChannel = channel;
	list->iDir = ME_DIR_OUTPUT;
	list->iTimeOut = time_out;
	list->iFlags = 0;
	list->iErrno = 0;

	err = meIOResetSubdevice(device, subdevice, 0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOResetSubdevice(): %s\n", msg);	
		goto ERROR;
	}

	err = meIOSingleConfig(
			device,
		   	subdevice,
		   	channel,
		   	single_config,
		   	ME_REF_AO_GROUND,
		   	ME_TRIG_CHAN_DEFAULT,
		   	trigger_type,
		   	trigger_edge,
		   	0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingleConfig(): %s\n", msg);	
		goto ERROR;
	}

	printf("Writting %f to AO.\n", value);
	err = meIOSingle(list, 1, 0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingle(): %s\n", msg);	
		goto ERROR;
	}

	printf("Press \"ENTER\" to continue \n");
	getchar();

ERROR:
	printf("Closing MEiDS.\n");
	meClose(0);

	return err;
}
