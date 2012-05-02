/***************************************************************************
 *   Copyright (C) 2005 by Guenter Gebhardt                                *
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
#include <string.h>

#include "medriver.h"


void usage(void)
{
    printf("cnt - Example program for counters\n\n");
    printf("Usage: cnt [arguments]\n\n");
    printf("Arguments:\n\n");
    printf("-h                     Print this help and exit.\n");
    printf("-d <device number>     Use <device number> device (Default is 0).\n");
    printf("-s <subdevice number>  Use <subdevice number> subdevice (Default is the first counter subdevice).\n");
    printf("-m <mode number>       Configure counter with mode <mode number> (Default is 3).\n");
    printf("-r <reference>         Configure counter with reference <reference>.\n");
	printf("                           Possible values are: 'Previous', '1MHz', '10MHz' and 'External' (Default).\n");
    printf("-v <value>             Use <value> for counter (Default is 16).\n");
}



int main(int argc, char *argv[])
{
	int err = ME_ERRNO_SUCCESS;
	int c;
	char msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	meIOSingle_t *list;
	int device = 0;
	int subdevice = -1;
	int value = 0x10;
	int single_config = ME_SINGLE_CONFIG_CTR_8254_MODE_3;
	int ref = ME_REF_CTR_EXTERNAL;

	/* Parse the command line arguments */
	while((c = getopt(argc, argv, "hd:c:s:c:m:r:v:")) != -1)
	{
		switch(c)
		{
			case 'h':
				usage();
				exit(0);
			case 'd':
				device = atoi(optarg);
				break;
			case 's':
				subdevice = atoi(optarg);
				break;
			case 'm':
				single_config = atoi(optarg);
				break;
			case 'r':
				if(!strcmp(optarg, "Previous"))
				{
					ref = ME_REF_CTR_PREVIOUS;
				}
				else if(!strcmp(optarg, "1MHz"))
				{
					ref = ME_REF_CTR_INTERNAL_1MHZ;
				}
				else if(!strcmp(optarg, "10MHz"))
				{
					ref = ME_REF_CTR_INTERNAL_10MHZ;
				}
				else if(!strcmp(optarg, "External"))
				{
					ref = ME_REF_CTR_EXTERNAL;
				}
				else
				{
					usage();
					exit(2);
				}
				break;
			case 'v':
				value = atoi(optarg);
				break;
			default:
				usage();
				exit(2);
		}
	}

	if(optind != argc)
	{
		fprintf(stderr, "%s: No non option arguments are supported.\n", argv[0]);
		usage();
		exit(2);
	}

	err = meOpen(0);
	if(err)
	{
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meOpen(): %s\n", msg);
		return 1;
	}

	if(subdevice == -1)
	{
		err = meQuerySubdeviceByType(
				device,
				0,
				ME_TYPE_CTR,
				ME_SUBTYPE_ANY,
				&subdevice);
		if(err)
		{
			meErrorGetMessage(err, msg, sizeof(msg));
			fprintf(stderr, "In meQuerySubdeviceByType(): %s\n", msg);
			goto ERROR;
		}
	}

	list = (meIOSingle_t *) malloc(sizeof(meIOSingle_t));
	if(!list)
	{
		perror("Cannot get buffer for single list");
		goto ERROR;
	}

	err = meIOSingleConfig(device, subdevice, 0, single_config, ref, ME_TRIG_CHAN_NONE, ME_TRIG_TYPE_NONE, ME_VALUE_NOT_USED, ME_VALUE_NOT_USED);
	if(err)
	{
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingleConfig(): %s\n", msg);
		goto ERROR_FREE;
	}

	list->iDevice = device;
	list->iSubdevice = subdevice;
	list->iChannel = 0;
	list->iDir = ME_DIR_OUTPUT;
	list->iValue = value;
	list->iFlags = 0;
	list->iErrno = 0;

	err = meIOSingle(list, 1, 0);
	if(err)
	{
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingle(): %s\n", msg);
		goto ERROR_FREE;
	}

	printf ("Counter started. Value=%d\n", value);
	printf ("Press 'enter' to end.\n");
	getchar();

	list->iValue = 0;
	list->iDir = ME_DIR_INPUT;

	err = meIOSingle(list, 1, 0);
	if(err)
	{
		meErrorGetMessage(err, msg, sizeof(msg));
		fprintf(stderr, "In meIOSingle(): %s\n", msg);
	}
	else
	{
		printf("Value = 0x%08X\n", list->iValue);
	}

ERROR_FREE:
	free (list);

ERROR:
	meClose(0);

	return 0;
}
