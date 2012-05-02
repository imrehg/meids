/*
 * Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * Source File : aiSingle.c                                              
 * Author      : GG (Guenter Gebhardt)  <g.gebhardt@meilhaus.de>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <medriver.h>


void usage(void){
    printf("aiSingle - Example program for analog input\n\n");
    printf("Usage: aiSingle [arguments]\n\n");
    printf("Arguments:\n\n");
    printf("-h                     Print this help and exit.\n");
    printf("-d <device number>     Use <device number> device (Default is 0).\n");
    printf("-s <subdevice number>  Use <subdevice number> subdevice (Default is the first analog output subdevice).\n");
    printf("-c <channel number>    Use <channel number> channel (Default is 0).\n");
    printf("-r <range number>      Use <range number> range (Default is the first fitting for value).\n");
    printf("-t <trigger type>      Configure trigger type with <trigger type>.\n");
	printf("                       Possible Values are 'SW' (Default), 'HWD'and 'HWA'.\n");
    printf("-e <trigger edge>      Configure trigger edge with <trigger edge>.\n");
	printf("                       Valid only for software trigger.\n");
	printf("                       Possible Values are 'Rising' (Default), 'Falling'and 'Any'.\n");
    printf("-a <analog ref>        Configure analog input with reference <analog ref>.\n");
	printf("                       Possible Values are 'Ground' (Default) and 'Diff'.\n");
    printf("-m <min value>         The minimum physical value to read (Default is -10.0).\n");
    printf("-x <max value>         The maximum physical value to read (Default is 10.0).\n");
	printf("-o <time out>          Use time out <time out> ms when external trigger is selected (Default is 10000 ms).\n");
}


int main(int argc, char *argv[]){
	int err;
	int c;
	char msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	meIOSingle_t *list;
	int device = 0;
	int subdevice = -1;
	int channel = 0;
	double value = 0.0;
	int single_config = -1;
	int unit = ME_UNIT_VOLT;
	double min = -10.0;
	double max = 9.9;
	int max_data;
	int trigger_type = ME_TRIG_TYPE_SW;
	int trigger_edge = ME_TRIG_EDGE_NONE;
	int ref = ME_REF_AI_GROUND;
	int time_out = 10000;

	printf("\n");
	/* Parse the command line arguments */
	while((c = getopt(argc, argv, "hd:s:c:r:t:e:a:m:x:o:")) != -1){
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
				else if(!strcmp(optarg, "HWA")){
					trigger_type = ME_TRIG_TYPE_EXT_ANALOG;
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
					trigger_type = ME_TRIG_EDGE_RISING;
				}
				else if(!strcmp(optarg, "Falling")){
					trigger_type = ME_TRIG_EDGE_FALLING;
				}
				else if(!strcmp(optarg, "Any")){
					trigger_type = ME_TRIG_EDGE_ANY;
				}
				else{
					usage();
					exit(2);
				}
				break;
			case 'a':
				if(!strcmp(optarg, "Ground")){
					ref = ME_REF_AI_GROUND;
				}
				else if(!strcmp(optarg, "Diff")){
					ref = ME_REF_AI_DIFFERENTIAL;
				}
				else{
					usage();
					exit(2);
				}
				break;
			case 'm':
				min = atof(optarg);
				break;
			case 'x':
				max = atof(optarg);
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
				ME_TYPE_AI,
				ME_SUBTYPE_ANY,
				&subdevice);
		if(err){
			meErrorGetMessage(err, msg, sizeof(msg));
			fprintf(stderr, "In meQuerySubdeviceByType(): %s\n", msg);	
		goto ERROR;
		}
	}

	if(single_config == -1){
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

	err = meIOSingleConfig(
			device,
		   	subdevice,
		   	channel,
		   	single_config,
		   	ME_REF_AI_GROUND,
		   	ME_TRIG_CHAN_DEFAULT,
		   	trigger_type,
		   	trigger_edge,
		   	0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingleConfig(): %s\n", msg);	
		goto ERROR;
	}

	list = (meIOSingle_t *) malloc(sizeof(meIOSingle_t));
	if(!list){
		perror("Cannot get buffer for single list");
		goto ERROR;
	}

	list->iDevice = device;
	list->iSubdevice = subdevice;
	list->iChannel = channel;
	list->iDir = ME_DIR_INPUT;
	list->iTimeOut = time_out;
	list->iFlags = 0;
	list->iErrno = 0;

	err = meIOSingle(list, 1, 0);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingle(): %s\n", msg);	
		goto ERROR;
	}

	err = meUtilityDigitalToPhysical(
			min,
			max,
			max_data,
			list->iValue,
			ME_MODULE_TYPE_MULTISIG_NONE,
			0,
			&value);
	if(err){
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meUtilityToPhysical(): %s\n", msg);	
		goto ERROR;
	}

	printf("Physical value = %lf\n", value);

ERROR:
	printf("Closing MEiDS.\n");
	meClose(0);

	return 0;
}
