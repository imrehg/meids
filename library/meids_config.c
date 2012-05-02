#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <string.h>

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_internal.h"
# include "meids_config_structs.h"
# include "meids_debug.h"

# include "meids_config.h"

static int  append_me_cfg_device_list(const me_config_t* cfg_Source, me_config_t* cfg_Dest);
static int  copy_me_cfg_entry(const me_cfg_device_entry_t* cfg_Source, me_config_t* cfg_Dest);
static int  copy_me_cfg_desc(const me_cfg_device_info_t* cfg_Source, me_cfg_device_info_t* cfg_Dest);
static int  copy_me_cfg_subdevice_list(me_cfg_subdevice_entry_t** cfg_Source, me_cfg_subdevice_entry_t** cfg_Dest, unsigned int* count);
static int  copy_me_cfg_range_list(me_cfg_range_info_t** cfg_Source, me_cfg_range_info_t** cfg_Dest, unsigned int* count);
static int  copy_me_cfg_TCIP_remote_host(const me_cfg_tcpip_hw_info_t* cfg_Source, me_cfg_tcpip_hw_info_t* cfg_Dest);

static void clean_me_cfg_device_list(me_cfg_device_entry_t* device);
static void clean_me_cfg_subdevice_list(me_cfg_subdevice_entry_t* subdevice);

static int  find_max_index(const me_config_t* cfg);
// static int  check_index(const me_config_t* cfg, int no);
static int  find_next_index(const me_config_t* cfg, int min);

static const char* decode_type(int type);
static const char* decode_subtype(int subtype);

static const char* decode_type(int type)
{

	switch (type)
	{
		case 0x00180001:
			return "Analog Output";
		case 0x00180002:
			return "Analog Input";
		case 0x00180003:
			return "Digital Input/Output";
		case 0x00180004:
			return "Digital Output";
		case 0x00180005:
			return "Digital Input";
		case 0x00180006:
			return "Counter";
		case 0x00180007:
			return "External interrupt line";
 	}
 	return "INVALID TYPE";
}

static const char* decode_subtype(int subtype)
{

	switch (subtype)
	{
		case 0x00190001:
			return "SINGLE";
		case 0x00190002:
			return "STREAMING";
		case 0x00190003:
			return "CTR 8254";
 	}
 	return "INVALID SUBTYPE";
}

void ConfigPrint(const me_config_t* cfg)
{
	int d, s;
	me_cfg_device_entry_t** device_list;
	me_cfg_subdevice_entry_t** subdevice_list;

	device_list = cfg->device_list;
	printf("\nThere is %d devices on list.\n", cfg->device_list_count);

	for (d=0; d < cfg->device_list_count; d++)
	{// There are some devices on the list
		if ((*device_list))
		{
			printf("\nNumber: %d (%d)\n", (*device_list)->logical_device_no, (*device_list)->info.device_no);
			if ((*device_list)->access_type != me_access_type_invalid)
			{
				printf("Name: %s\n", (*device_list)->info.device_name);
				printf("Description: %s\n", (*device_list)->info.device_description);
				printf("Status: 0x%04x %s\n\n", (*device_list)->plugged, (((*device_list)->plugged == me_plugged_type_IN) || ((*device_list)->plugged == me_plugged_type_USED))? "IN":"NOT IN");

				printf("Number of subdevices: %d.\n", (*device_list)->subdevice_list_count);
				subdevice_list = (*device_list)->subdevice_list;
				for (s=0; s < (*device_list)->subdevice_list_count; s++)
				{// There are some devices on the list
					printf("  SubNumber: %d\n", s);
					printf("  Type: 0x%x:%s\n", (*subdevice_list)->info.type, decode_type((*subdevice_list)->info.type));
					printf("  Subtype: 0x%x:%s\n\n", (*subdevice_list)->info.sub_type, decode_subtype((*subdevice_list)->info.sub_type));
					subdevice_list++;
				}
			}
			else
			{
				printf("Ignored.\n");
			}
		}

		device_list++;
	}
}


int ConfigVerify(me_config_t *cfg, int flags)
{// Last step. Check if created config is correct. Mark entries as plugg_BLOCKED or plugg_USED
/// @note Function checks if logical numbers are unique.
/// @note Function mark all entries that has not assigned 'real' number as ME_PLUGGED_BLOCKED.

	int err = ME_ERRNO_SUCCESS;
	int i, e;
	int status = 0;
	me_cfg_device_entry_t** device_list;

	typedef struct v_list
	{
		struct v_list* next;
		int val;
	} v_list_t;


	struct v_list logical_numbers_head;
	struct v_list* logical_numbers_entry;
	struct v_list* logical_numbers_new_entry;

	logical_numbers_head.next = NULL;
	logical_numbers_head.val = 0;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	//Check if number is unique.
	device_list = cfg->device_list;

	for (e=0; e < cfg->device_list_count; e++, device_list++)
	{// There are some devices on the list
		logical_numbers_entry = &logical_numbers_head;

		for (i=0; i<logical_numbers_head.val; i++)
		{
			if ((*device_list)->logical_device_no == logical_numbers_entry->next->val)
			{
				status = 1;
				break;
			}
			logical_numbers_entry = logical_numbers_entry->next;
		}

		if (status)
		{// Founded!
			LIBPERROR("Number %d found twice!\n", (*device_list)->logical_device_no);
			err = ME_ERRNO_INTERNAL;
			break;
		}
		else
		{// Not found, add to the list.
			logical_numbers_entry->next = calloc(1, sizeof(v_list_t));
			logical_numbers_entry->next->next = NULL;
			logical_numbers_entry->next->val = (*device_list)->logical_device_no;
			logical_numbers_head.val++;
		}
	}

	// Clear temporary list.
	logical_numbers_entry = logical_numbers_head.next;
	for (i=0; i<logical_numbers_head.val; i++)
	{
 		logical_numbers_new_entry=logical_numbers_entry;
		logical_numbers_entry = logical_numbers_entry->next;
		if (logical_numbers_new_entry)
		{
			free(logical_numbers_new_entry);
			logical_numbers_new_entry = NULL;
		}
	}

	//Mark if device is inserted.
	device_list = cfg->device_list;
	for (e=0; e< cfg->device_list_count; e++, device_list++)
	{// There are some devices on the list
		if ((*device_list)->info.device_no < 0)
		{
			(*device_list)->plugged = me_plugged_type_BLOCKED;
		}
		else
		{
			if ((*device_list)->plugged != me_plugged_type_USED)
				(*device_list)->plugged = me_plugged_type_BLOCKED;
		}
	}

	return err;
}

int ConfigMaxNumber(const me_config_t* cfg, int* number, int flags)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	if (cfg && number)
	{
		*number = find_max_index(cfg) + 1;
	}
	else
	{
		LIBPERROR("Request has invalid pointer.\n");
		err = ME_ERRNO_INVALID_POINTER;
	}
	return err;
}

static int find_max_index(const me_config_t* cfg)
{
	int max = -1;
	int e;
	me_cfg_device_entry_t** device_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device_list = cfg->device_list;

	for (e=0; e < cfg->device_list_count; e++, device_list++)
	{// There are some devices on the list
		if (max < (*device_list)->logical_device_no)
		{
			max = (*device_list)->logical_device_no;
		}
	}

	return max;
}

/*
static int check_index(const me_config_t* cfg, int no)
{
	int e;
	me_cfg_device_entry_t** device_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device_list = cfg->device_list;

	for (e=0; e < cfg->device_list_count; e++, device_list++)
	{// There are some devices on the list
		if (no == (*device_list)->logical_device_no)
		{
			return 1;
		}
	}

	return 0;
}
*/

static int find_next_index(const me_config_t* cfg, int min)
{
	int i;
	int e;
	int max = find_max_index(cfg);
	me_cfg_device_entry_t** device_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (max < min)
		return min;

	for (i=min; i < max; i++)
	{
		device_list = cfg->device_list;

		for (e=0; e < cfg->device_list_count; e++, device_list++)
		{// There are some devices on the list
			if (i == (*device_list)->logical_device_no)
				break;
		}
		if (e == cfg->device_list_count)
			return i;
	}

	return max+1;
}

void ConfigClean(me_config_t *cfg, int flags)
{
	int i = 0;
	me_cfg_device_entry_t** device_list = cfg->device_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	if (cfg->device_list)
	{
		for (i=0; i<cfg->device_list_count; i++, device_list++)
		{
			clean_me_cfg_device_list(*device_list);
			if (*device_list)
			{
				free(*device_list);
				*device_list = NULL;
			}
		}
		free(cfg->device_list);
		cfg->device_list = NULL;
		cfg->device_list_count = 0;
	}
}

static void clean_me_cfg_device_list(me_cfg_device_entry_t* device)
{
	int i = 0;
	me_cfg_subdevice_entry_t** subdevice_list = device->subdevice_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (device->info.device_name)
	{
		free(device->info.device_name);
		device->info.device_name = NULL;
	}
	if (device->info.device_description)
	{
		free(device->info.device_description);
		device->info.device_description = NULL;
	}

	if (device->subdevice_list)
	{
		for (i=0; i<device->subdevice_list_count; i++, subdevice_list++)
		{
			clean_me_cfg_subdevice_list(*subdevice_list);
			if (*subdevice_list)
			{
				free(*subdevice_list);
				*subdevice_list = NULL;
			}
		}
		free(device->subdevice_list);
		device->subdevice_list = NULL;
	}

	if (device->access_type == me_access_type_TCPIP)
	{
		if (device->info.tcpip.remote_host)
		{
			free(device->info.tcpip.remote_host);
			device->info.tcpip.remote_host = NULL;
		}
	}
}

static void clean_me_cfg_subdevice_list(me_cfg_subdevice_entry_t* subdevice)
{
	int i = 0;
	me_cfg_range_info_t** range_list = subdevice->info.range_list;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (subdevice->info.range_list)
	{
		for (i=0; i<subdevice->info.range_list_count; i++, range_list++)
		{
			if (*range_list)
			{
				free(*range_list);
				*range_list = NULL;
			}
		}

		free(subdevice->info.range_list);
		subdevice->info.range_list = NULL;
	}
}

int ConfigEnumerate(me_config_t* cfg, int start_enum, int flags)
{
	me_cfg_device_entry_t* entry;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	if (!cfg)
	{
		LIBPERROR("cfg MUST BE INITIALIZED!\n");
		return ME_ERRNO_INTERNAL;
	}

	for (i = 0; i < cfg->device_list_count; i++)
	{
		entry = *(cfg->device_list + i);
		if (!entry)
		{
			LIBPERROR("device_list_count=%d but device_list[%d]=NULL\n", cfg->device_list_count, i);
			return ME_ERRNO_INTERNAL;
		}

		entry->logical_device_no = start_enum +i;
	}

	return ME_ERRNO_SUCCESS;
}

int ConfigDenumerate(me_config_t* cfg, int flags)
{
	me_cfg_device_entry_t* entry;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!cfg)
	{
		LIBPERROR("cfg MUST BE INITIALIZED!\n");
		return ME_ERRNO_INTERNAL;
	}

	for (i = 0; i < cfg->device_list_count; i++)
	{
		entry = *(cfg->device_list + i);
		if (!entry)
		{
			LIBPERROR("device_list_count=%d but device_list[%d]=NULL\n", cfg->device_list_count, i);
			return ME_ERRNO_INTERNAL;
		}

		entry->logical_device_no = -1;
	}

	return ME_ERRNO_SUCCESS;
}

int ConfigContinueEnumerate(me_config_t* cfg, int start_enum, int flags)
{
	me_cfg_device_entry_t* entry;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	if (!cfg)
	{
		LIBPERROR("cfg MUST BE INITIALIZED!\n");
		return ME_ERRNO_INTERNAL;
	}

	for (i = 0; i < cfg->device_list_count; i++)
	{
		entry = *(cfg->device_list + i);
		if (!entry)
		{
			LIBPERROR("device_list_count=%d but device_list[%d]=NULL\n", cfg->device_list_count, i);
			return ME_ERRNO_INTERNAL;
		}
		if (entry->logical_device_no == -1)
		{
			start_enum = find_next_index(cfg, start_enum);
			entry->logical_device_no = start_enum;
			start_enum++;
		}
	}

	return ME_ERRNO_SUCCESS;
}

int ConfigDuplicate(const me_config_t* cfg_Source, me_config_t** Dest, int flags)
{
	int err = ME_ERRNO_SUCCESS;

	me_config_t* cfg_Dest;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	//Reserve memory for new config.
	cfg_Dest = calloc(1, sizeof(me_config_t));
	if (cfg_Dest)
	{
// 		cfg_Dest->device_list_count = cfg_Source->device_list_count;
		cfg_Dest->device_list_count = 0;
		//Reserve memory for new table.
		cfg_Dest->device_list = calloc((cfg_Source->device_list_count), sizeof(me_cfg_device_entry_t *));
		if (cfg_Dest->device_list)
		{
			//Copy table.
			append_me_cfg_device_list(cfg_Source, cfg_Dest);
		}
		else
		{
			free(cfg_Dest);
			cfg_Dest = NULL;
			LIBPERROR("Can not get requestet memory for device_list table.\n");
			err = ME_ERRNO_INTERNAL;
		}
		*Dest = cfg_Dest;
	}
	else
	{
		LIBPERROR("Can not get requestet memory for new config.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

int ConfigJoin(const me_config_t* cfg_First, const me_config_t* cfg_Second, me_config_t** Dest, int flags)
{
	int err=ME_ERRNO_SUCCESS;

	me_config_t* cfg_Dest = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	//Reserve memory for new config.
	cfg_Dest = calloc(1, sizeof(me_config_t));
	if (cfg_Dest)
	{
		cfg_Dest->device_list_count = 0;
		//Reserve memory for new table.
		cfg_Dest->device_list = calloc((cfg_First->device_list_count + cfg_Second->device_list_count), sizeof(me_cfg_device_entry_t *));
		if (cfg_Dest->device_list)
		{
			//Copy first table.
			append_me_cfg_device_list(cfg_First, cfg_Dest);
			//Copy second table.
			append_me_cfg_device_list(cfg_Second, cfg_Dest);
			*Dest = cfg_Dest;
		}
		else
		{
			free(cfg_Dest);
			cfg_Dest = NULL;
			LIBPERROR("Can not get requestet memory for device_list table.\n");
			err = ME_ERRNO_INTERNAL;
		}
	}
	else
	{
		LIBPERROR("Can not get requestet memory for new config.\n");
		err = ME_ERRNO_INTERNAL;
	}
	return err;
}

int ConfigAppend(const me_cfg_device_entry_t* cfg_Source, me_config_t* cfg_Dest, int flags)
{
	int err = ME_ERRNO_SUCCESS;

	me_cfg_device_entry_t** newTable;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	//Reserve memory for new (bigger) table.
	newTable = calloc((cfg_Dest->device_list_count + 1), sizeof(me_cfg_device_entry_t *));
	if (newTable)
	{
		//Copy old table to new.
		memcpy(newTable, cfg_Dest->device_list, sizeof(me_cfg_device_entry_t *) * cfg_Dest->device_list_count);
		//Release old table
		free(cfg_Dest->device_list);
		//Put new table in place of old one.
		cfg_Dest->device_list = newTable;

		err = copy_me_cfg_entry(cfg_Source, cfg_Dest);
	}
	else
	{
		LIBPERROR("Can not get requestet memory for device_list structure.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int append_me_cfg_device_list(const me_config_t* cfg_Source, me_config_t* cfg_Dest)
{
/// @note Do not check size of destination. Caller must takes care of it.
	int err = ME_ERRNO_SUCCESS;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (i = 0; i < cfg_Source->device_list_count; i++)
	{
		err = copy_me_cfg_entry(*(cfg_Source->device_list + i), cfg_Dest);
		if (err)
			break;
	}
	return err;
}

static int copy_me_cfg_entry(const me_cfg_device_entry_t* cfg_Source, me_config_t* cfg_Dest)
{
/// @note Do not check size of destination. Caller must takes care of it.
	int err;
	me_cfg_device_entry_t* newEntry;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	// Copy added object to table. 1 device
	newEntry = calloc(1, sizeof(me_cfg_device_entry_t));
	if (newEntry)
	{
		*(cfg_Dest->device_list + cfg_Dest->device_list_count) = newEntry;
		// Copy the object structure.
		memcpy(newEntry, cfg_Source, sizeof(me_cfg_device_entry_t));
		err = copy_me_cfg_desc( &(cfg_Source->info), &(newEntry->info));
		newEntry->subdevice_list = calloc(newEntry->subdevice_list_count, sizeof(me_cfg_subdevice_entry_t*));
		if (newEntry->subdevice_list)
		{
			err = copy_me_cfg_subdevice_list(cfg_Source->subdevice_list, newEntry->subdevice_list, &(newEntry->subdevice_list_count));
			if (!err)
			{
				// Bus specific
				switch (newEntry->access_type)
				{
					case me_access_type_PCI:
						newEntry->info.vendor_id = cfg_Source->info.vendor_id;
						newEntry->info.device_id = cfg_Source->info.device_id;
						newEntry->info.serial_no = cfg_Source->info.serial_no;
						break;

					case me_access_type_TCPIP:
						if (cfg_Source->info.tcpip.remote_host)
						{
							err = copy_me_cfg_TCIP_remote_host( &(cfg_Source->info.tcpip), &(newEntry->info.tcpip));
						}
						break;

					case me_access_type_USB:
							newEntry->info.usb.root_hub_no = cfg_Source->info.usb.root_hub_no;
						break;

					default:
						LIBPERROR("ACCESS TYPE 0x%04x NOT IMPLEMENTED!\n", newEntry->access_type);
						err = ME_ERRNO_INTERNAL;
						break;
				}
			}
		}
		else
		{
			newEntry->subdevice_list_count = 0;
			LIBPERROR("Can not get requestet memory for subdevice_entry structure.\n");
			err = ME_ERRNO_INTERNAL;
		}
		cfg_Dest->device_list_count++;
	}
	else
	{
		LIBPERROR("Can not get requestet memory for device_entry structure.\n");
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int copy_me_cfg_desc(const me_cfg_device_info_t* cfg_Source, me_cfg_device_info_t* cfg_Dest)
{
	int err = ME_ERRNO_INTERNAL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	cfg_Dest->device_name = NULL;
	cfg_Dest->device_description = NULL;

	if (cfg_Source->device_name)
	{
		cfg_Dest->device_name = calloc(strlen(cfg_Source->device_name)+1, sizeof(char));
		if (cfg_Dest->device_name)
		{
			strcpy(cfg_Dest->device_name, cfg_Source->device_name);
		}
		else
		{
			printf("ERROR:%s %d Can not get requestet memory for device name.", __FUNCTION__, __LINE__);
		}
	}

	if (cfg_Source->device_description)
	{
		cfg_Dest->device_description = calloc(strlen(cfg_Source->device_description)+1, sizeof(char));
		if (cfg_Dest->device_description)
		{
			strcpy(cfg_Dest->device_description, cfg_Source->device_description);
		}
		else
		{
			printf("ERROR:%s %d Can not get requestet memory for device description.", __FUNCTION__, __LINE__);
			err = ME_ERRNO_INTERNAL;
		}
	}
	if (cfg_Dest->device_name && cfg_Dest->device_description)
	{
		err = ME_ERRNO_SUCCESS;
	}

	return err;
}

static int copy_me_cfg_TCIP_remote_host(const me_cfg_tcpip_hw_info_t* cfg_Source, me_cfg_tcpip_hw_info_t* cfg_Dest)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	cfg_Dest->remote_host = calloc(strlen(cfg_Source->remote_host)+1, sizeof(char));
	if (cfg_Dest->remote_host)
	{
		strcpy(cfg_Dest->remote_host, cfg_Source->remote_host);
	}
	else
	{
		printf("ERROR:%s %d Can not get requestet memory for remote host name.", __FUNCTION__, __LINE__);
		err = ME_ERRNO_INTERNAL;
	}

	return err;
}

static int copy_me_cfg_subdevice_list(me_cfg_subdevice_entry_t** cfg_Source, me_cfg_subdevice_entry_t** cfg_Dest, unsigned int* count)
{
	int err=ME_ERRNO_SUCCESS;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	me_cfg_subdevice_entry_t** Source = cfg_Source;
	me_cfg_subdevice_entry_t** Dest   = cfg_Dest;

	for (i=0; i < *count; i++)
	{
		// Copy added object to table. 1 subdevice
		*Dest = calloc(1, sizeof(me_cfg_subdevice_entry_t));
		if (*Dest)
		{
			memcpy(*Dest, *Source, sizeof(me_cfg_subdevice_entry_t));
			(*Dest)->info.range_list = calloc((*Dest)->info.range_list_count, sizeof(me_cfg_range_info_t*));
			if ((*Dest)->info.range_list)
			{
				err = copy_me_cfg_range_list((*Source)->info.range_list, (*Dest)->info.range_list, &((*Dest)->info.range_list_count));
			}
			else
			{
				(*Dest)->info.range_list_count = 0;
				printf("ERROR:%s %d Can not get requestet memory for subdevice_entry structure.", __FUNCTION__, __LINE__);
				err = ME_ERRNO_INTERNAL;
			}
		}
		else
		{
			*count = i;
			printf("ERROR:%s %d Can not get requestet memory for subdevice_entry structure.", __FUNCTION__, __LINE__);
			err = ME_ERRNO_INTERNAL;
			break;
		}

		Source++;
		Dest++;
	}

	return err;
}

static int copy_me_cfg_range_list(me_cfg_range_info_t** cfg_Source, me_cfg_range_info_t** cfg_Dest, unsigned int* count)
{
	int err=ME_ERRNO_SUCCESS;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	me_cfg_range_info_t** Source = cfg_Source;
	me_cfg_range_info_t** Dest   = cfg_Dest;

	for (i=0; i < *count; i++)
	{
		// Copy added object to table. range
		*Dest = calloc(1, sizeof(me_cfg_range_info_t));
		if (*Dest)
		{
			memcpy(*Dest, *Source, sizeof(me_cfg_range_info_t));
		}
		else
		{
			*count = i;
			printf("ERROR:%s %d Can not get requestet memory for subdevice_entry structure.", __FUNCTION__, __LINE__);
			err = ME_ERRNO_INTERNAL;
			break;
		}

		Source++;
		Dest++;
	}

	return err;
}

int ConfigResolve(const me_config_t* cfg, const int logical_no, me_cfg_device_entry_t** reference)
{
	int err = ME_ERRNO_DEVICE_UNPLUGGED;
	int logical_ID;
	int i;

	int max;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	*reference = (me_cfg_device_entry_t *)&global_dummy;

	if (!cfg)
	{
		return ME_ERRNO_NOT_OPEN;
	}

	LIBPDEBUG("cfg->device_list_count %d \n", cfg->device_list_count);
	max = find_max_index(cfg);
	LIBPDEBUG("max %d \n", max);


	if (logical_no < 0 || logical_no > max)
	{
		LIBPDEBUG("Wrong number specified %d \n", logical_no);
		// Wrong number. Return reference tu dummy;
		return ME_ERRNO_INVALID_DEVICE;
	}

	for (i = 0; i < cfg->device_list_count; i++)
	{
		logical_ID = (cfg->device_list[i])->logical_device_no;
		if (logical_ID == logical_no)
		{
			*reference = cfg->device_list[i];
			if (((cfg->device_list[i])->plugged == me_plugged_type_IN) || ((cfg->device_list[i])->plugged == me_plugged_type_USED))
			{
				err = ME_ERRNO_SUCCESS;
			}
			break;
		}
	}

	if (err)
		LIBPWARNING("Device number %d not in use.\n", logical_no);

	if (!err)
	{
		LIBPDEBUG("reference=%p LogicID=%d -> ID=%d\n", cfg->device_list[i], (cfg->device_list[i])->logical_device_no, (cfg->device_list[i])->info.device_no);
	}
	return err;
}

int ShortcutBuild(const me_config_t* cfg, me_config_shortcut_table_t* table)
{
	int err = ME_ERRNO_SUCCESS;
	int logical_ID;
	int i;

	int count = find_max_index(cfg);

	LIBPINFO("executed: %s\n", __FUNCTION__);

	table->device_table = calloc(count + 1, sizeof(me_config_table_t*));
	if (table->device_table)
	{
		table->device_table_count = count + 1;
		for (i = 0; i < table->device_table_count; i++)
		{//Set dummies as default.
			(table->device_table[i]).reference = (me_cfg_device_entry_t *)&global_dummy;
		}

		for (i = 0; i < cfg->device_list_count; i++)
		{//Set real values.
			logical_ID = (cfg->device_list[i])->logical_device_no;
			if (logical_ID >= 0)
			{
				(table->device_table[logical_ID]).reference = cfg->device_list[i];
			}
		}

	}
	else
	{
		table->device_table_count = 0;
		LIBPCRITICALERROR("Can not get requestet memory for device_table structure.\n");
		err = ME_ERRNO_INTERNAL;
	}
	return err;
}

int ShortcutResolve(const me_config_shortcut_table_t* table, const int logical_no, me_cfg_device_entry_t** reference)
{
	int err = ME_ERRNO_INVALID_DEVICE;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (logical_no < 0 || logical_no >= table->device_table_count)
	{
		*reference = (table->device_table[table->device_table_count-1]).reference;
		LIBPDEBUG("Wrong number specified %d.\n", logical_no);
		// Wrong number. Return reference tu dummy;
		return ME_ERRNO_INVALID_DEVICE;
	}

	*reference = (table->device_table[logical_no]).reference;

	if (*reference == (table->device_table[table->device_table_count-1]).reference)
	{
		LIBPWARNING("Device number %d not in use.\n", logical_no);
	}
	else
	{
		err = ME_ERRNO_SUCCESS;
	}

	return err;
}

void ShortcutClean(me_config_shortcut_table_t* table)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	free(table->device_table);
	table->device_table = NULL;
	table->device_table_count = 0;
}


