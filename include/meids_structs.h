#ifndef _MEIDS_STRUCTS_H_
# define _MEIDS_STRUCTS_H_

#include "meids_config_structs.h"

typedef struct ME_Host
{
	uint lenght;
	char* addr;
} me_host_t;

typedef struct ME_Host_List
{
	struct ME_Host_List* next;
	me_host_t host;
}me_host_list_t;

typedef struct ME_Addr
{
	const char* XML_Conf_File;
	const char* PCI;
	const char* USB;
	const me_host_list_t* Host_List;
}me_addr_t;

//Vendor
#define me_plugged_type_VENDOR			0x001

//Board type egz.: me4680IS
#define me_plugged_type_TYPE			0x002

//Serial number
#define me_plugged_type_SERIALNUMBER	0x004

//Type of connection egz.: PCI, TCP/IP
#define me_plugged_type_BUS_TYPE		0x008

//Address (Use only with 'me_plugged_type_BUS_TYPE'!)
 //'Real' ID
# define me_plugged_type_ADDR_SLOT		0x010
 //Location
# define me_plugged_type_ADDR_MAJOR		0x020
# define me_plugged_type_ADDR_MINOR		0x040

//General
#define me_plugged_type_ANY				0x3F




// Top layer

typedef struct me_config_table
{// Static config structure for fast access.
	me_cfg_device_entry_t* reference;
}
me_config_table_t;

static const me_cfg_device_entry_t global_dummy =
{//This static instance for dummies in library.
	NULL,
	me_access_type_invalid,
	-1,
	{
		0, 0, 0,
		"ME-DUMMY",
		"Empty slot.",
		-1,
		{
			{0, 0, 0}
		}
	},
	me_plugged_type_BLOCKED,
	NULL,
	0
};

typedef struct me_config_shortcut_table
{// Static config structure
	unsigned int device_table_count;
	me_config_table_t* device_table;
}
me_config_shortcut_table_t;

typedef struct me_XML_bus_type
{
	int location;
	int bus;
}me_XML_bus_type_t;

#endif	//_MEIDS_STRUCTS_H_
