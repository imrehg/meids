/* Configuration parser for Meilhaus driver system.
 * ==========================================
 *
 *  Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */
#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <stdio.h>
# include <string.h>
# include <stdlib.h>

# include "me_error.h"
# include "me_defines.h"

# include "meids_debug.h"
# include "meids_config.h"
# include "meids_common.h"
# include "meids_xml.h"

# include "meids_init.h"

static int AddXMLInit(addr_list_t** conf);

int CreateInit(char* addr, addr_list_t** conf)
{
	FILE* fd = NULL;
	char line[256];
	char command[256];
	int flag = ME_OPEN_NO_FLAGS;
	int switcher = 0;
	addr_list_t** nconf;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	/// Must point to NULL'ed pointer for list.
	if (!conf || *conf)
	{
		LIBPERROR("Wrong pointer for address list passed!\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	nconf = conf;

	if (addr)
	{
		fd = fopen(addr, "r");
	}

	if (!fd)
		return ME_ERRNO_SUCCESS;


	while (!feof(fd))
	{
		line[0] ='\0';
		command[0] ='\0';
		// get line
		fgets(line, 256, fd);
		//get command
		sscanf(line,"%s",command);
		if (strlen(command) && command[0] != '#')
		{
			if (!strncmp("local:", command, strlen("local:")))
			{
				flag = ME_OPEN_LOCAL;
				switcher = 0;
			}
			else if (!strncmp("pci:", command, strlen("pci:")))
			{
				flag = ME_OPEN_PCI;
				switcher = 0;
			}
			else if (!strncmp("usb:", command, strlen("usb:")))
			{
				flag = ME_OPEN_USB;
				switcher = 0;
			}
			else if (!strncmp("rpc:", command, strlen("rpc:")))
			{
				flag = ME_OPEN_RPC;
				switcher = 0;
			}
			else if (!strncmp("detect:", command, strlen("detect:")))
			{
				flag = ME_OPEN_NO_FLAGS;
				switcher = 1;
			}
			else
			{
				if (switcher)
				{
					CreateInit(command, nconf);
					while (*nconf)
					{
						nconf = &((*nconf)->next);
					}
				}
				else
				{
					*nconf = calloc(1, sizeof(addr_list_t));
					if (*nconf)
					{
						(*nconf)->addr = calloc(strlen(command)+1, sizeof(char));
						if ((*nconf)->addr)
						{
							strcpy((*nconf)->addr, command);
						}
						else
						{
							LIBPERROR("Can not get requestet memory for address.");
							err = ME_ERRNO_INTERNAL;
						}
						(*nconf)->flag = flag;
						(*nconf)->next = NULL;
						nconf = &((*nconf)->next);
					}
					else
					{
						LIBPERROR("Can not get requestet memory for config.");
						err = ME_ERRNO_INTERNAL;
					}
				}
			}
		}
		if (err)
			break;
	}

	fclose(fd);

	if (!err)
	{
		err = AddXMLInit(nconf);
	}

	if (!err)
	{
		err = CleanInit(conf);
	}

	return err;
}

void DestroyInit(addr_list_t** conf)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!conf)
		return;

	if (*conf)
	{
		if ((*conf)->next)
			DestroyInit(&((*conf)->next));

		if ((*conf)->addr)
		{
			free((*conf)->addr);
			(*conf)->addr = NULL;
		}

		free(*conf);
		*conf = NULL;
	}
}

static int AddXMLInit(addr_list_t** conf)
{
	me_config_t cfg_XML;
	addr_list_t** nconf = conf;
	int i;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	ConfigLoad_XML_TCPIP(&cfg_XML, MEIDS_XML_FILE, ME_VALUE_NOT_USED);

	for (i=0; i<cfg_XML.device_list_count; i++)
	{
		if (cfg_XML.device_list[i] && cfg_XML.device_list[i]->access_type == me_access_type_TCPIP)
		{
			if (cfg_XML.device_list[i]->info.tcpip.remote_host)
			{
				*nconf = calloc(1, sizeof(addr_list_t));
				if (*nconf)
				{
					(*nconf)->addr = calloc(strlen(cfg_XML.device_list[i]->info.tcpip.remote_host)+1, sizeof(char));
					if ((*nconf)->addr)
					{
						strcpy((*nconf)->addr, cfg_XML.device_list[i]->info.tcpip.remote_host);
					}
					else
					{
						LIBPERROR("Can not get requestet memory for remote address.");
						err = ME_ERRNO_INTERNAL;
					}
					(*nconf)->flag = ME_OPEN_RPC;
					(*nconf)->next = NULL;
					nconf = &((*nconf)->next);
				}
				else
				{
					LIBPERROR("Can not get requestet memory for config.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
		if (err)
			break;
	}

	ConfigClean(&cfg_XML, ME_VALUE_NOT_USED);

	return err;
}

int CleanInit(addr_list_t** conf)
{
	addr_list_t** nconf = conf;
	addr_list_t* newconf = NULL;
	addr_list_t** nnewconf = &newconf;
	int err = ME_ERRNO_SUCCESS;

	while (*nconf)
	{
		if (!SearchInit((*nconf)->addr, newconf))
		{	// Entry not in structure. Adding it.
			*nnewconf = calloc(1, sizeof(addr_list_t));
			if (nnewconf)
			{
				(*nnewconf)->addr = calloc(strlen((*nconf)->addr)+1, sizeof(char));
				if ((*nnewconf)->addr)
				{
					strcpy((*nnewconf)->addr, (*nconf)->addr);
				}
				else
				{
					LIBPERROR("Can not get requestet memory for remote address.");
					err = ME_ERRNO_INTERNAL;
				}
				(*nnewconf)->flag = (*nconf)->flag;
				(*nnewconf)->next = NULL;
				nnewconf = &((*nnewconf)->next);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for config.");
				err = ME_ERRNO_INTERNAL;
			}
		}

		if (err)
			break;

		nconf = &((*nconf)->next);
	}

	DestroyInit(conf);
	*conf = newconf;

	return err;
}

int SearchInit(char* addr, addr_list_t* conf)
{
	int ret;

	if (!conf)
		return 0;

	if (!addr || !strlen(addr))
		return 0;

	if ((conf->addr))
	{
		if (!strcmp(conf->addr, addr))
		{	// Found it!
			return 1;
		}
	}
	else
	{
		LIBPERROR("Corruption in structure! Config entry exist, but no address provided!\n");
	}

	ret = SearchInit(addr, conf->next);

	return (ret>0) ? ret+1: ret;
}
