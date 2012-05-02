/* XML serialization for Meilhaus driver system.
 * ==========================================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
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
 *  Author:	Guenter Gebhardt
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <libxml/parser.h>
# include <libxml/tree.h>

# include "me_defines.h"
# include "me_error.h"

# include "meids_ioctl.h"
# include "meids_internal.h"
# include "meids_debug.h"
# include "meids_config.h"

# include "meids_xml.h"

int determine_node_count(xmlNode* root_node, char* list, char* entry);
int determine_node_count_type(xmlNode* root_node, char* list, char* entry, me_XML_bus_type_t* type);

int determine_node_count(xmlNode* root_node, char* list, char* entry)
{
	xmlNode* cur_node = NULL;
	xmlNode* cur_subnode = NULL;
	int no_devices = 0;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	// Determine the count of entries
	for (cur_node = root_node->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, list))
		{
			for (cur_subnode = cur_node->children; cur_subnode; cur_subnode = cur_subnode->next)
			{
				if (!strcmp((char *) cur_subnode->name, entry))
				{
					no_devices++;
				}
			}
		}
	}

	return no_devices;
}

int determine_node_count_type(xmlNode* root_node, char* list, char* entry, me_XML_bus_type_t* type)
{
	xmlNode* cur_node = NULL;
	xmlNode* cur_subnode = NULL;
	int no_devices = 0;

	char* marker = NULL;
	int marker_val;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	// Determine the count of entries
	for (cur_node = root_node->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, list))
		{
			for (cur_subnode = cur_node->children; cur_subnode; cur_subnode = cur_subnode->next)
			{
				if (!strcmp((char *) cur_subnode->name, entry))
				{
					if (type->location != ME_ACCESS_TYPE_ANY)
					{
						marker = (char *) xmlGetProp(cur_subnode, (xmlChar *) "access");
						marker_val = strtol(marker, NULL, 10);
						xmlFree(marker);
						if (marker_val != type->location)
						{
							continue;
						}
					}

					if (type->bus != ME_BUS_TYPE_ANY)
					{
						marker = (char *) xmlGetProp(cur_subnode, (xmlChar *) "bus");
						marker_val = strtol(marker, NULL, 10);
						xmlFree(marker);
						if (marker_val != type->bus)
						{
							continue;
						}
					}

					no_devices++;
				}
			}
		}
	}

	return no_devices;
}

int ConfigLoad_XML_ALL(me_config_t* cfg_XML, char* address , int flags)
{
	me_XML_bus_type_t bus = {
							.location = ME_ACCESS_TYPE_ANY,
							.bus = ME_BUS_TYPE_ANY
							};

	LIBPINFO("executed: %s\n", __FUNCTION__);

	return ConfigLoad_XML(cfg_XML, address , &bus, flags);
}

int ConfigLoad_XML_PCI(me_config_t* cfg_XML , char* address , int flags)
{
	me_XML_bus_type_t bus = {
							.location = ME_ACCESS_TYPE_LOCAL,
							.bus = ME_BUS_TYPE_PCI
							};

	LIBPINFO("executed: %s\n", __FUNCTION__);

	return ConfigLoad_XML(cfg_XML, address , &bus, flags);
}

int ConfigLoad_XML_USB(me_config_t* cfg_XML , char* address , int flags)
{//SynapseUSB
	me_XML_bus_type_t bus = {
							.location = ME_ACCESS_TYPE_LOCAL,
							.bus = ME_BUS_TYPE_USB
							};

	LIBPINFO("executed: %s\n", __FUNCTION__);

	return ConfigLoad_XML(cfg_XML, address , &bus, flags);
}

int ConfigLoad_XML_TCPIP(me_config_t* cfg_XML , char* address , int flags)
{//SynapseLAN
	me_XML_bus_type_t bus = {
							.location = ME_ACCESS_TYPE_REMOTE,
							.bus = ME_BUS_TYPE_ANY
							};

	LIBPINFO("executed: %s\n", __FUNCTION__);

	return ConfigLoad_XML(cfg_XML, address , &bus, flags);
}

int ConfigLoad_XML(me_config_t* cfg_XML , char* address , me_XML_bus_type_t* location, int flags)
{
	xmlDocPtr doc;
	xmlNode* root_node = NULL;
	xmlNode* cur_node = NULL;

	int no_devices = 0;
	int err = ME_ERRNO_SUCCESS;

	LIBXML_TEST_VERSION;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	cfg_XML->device_list_count = 0;
	cfg_XML->device_list = NULL;

	// Parse the xml file
	doc = xmlReadFile(address , NULL, XML_PARSE_NOWARNING | XML_PARSE_NOERROR | XML_PARSE_NOBLANKS);
	if (doc != NULL)
	{
		// Get the root element node.
		root_node = xmlDocGetRootElement(doc);
		if (root_node != NULL)
		{
			no_devices = determine_node_count_type(root_node, "device_list", "device_entry", location);
			if (no_devices)
			{
				// Config has 'device_list' and 'device_entry' nodes. Reserve memory for device structure.
				cfg_XML->device_list = calloc(no_devices, sizeof(me_cfg_device_entry_t *));
				if (cfg_XML->device_list)
				{
					// Get 'device_list' element node
					for (cur_node = root_node->children; cur_node; cur_node = cur_node->next)
					{
						if (!strcmp((char *) cur_node->name, "device_list"))
						{
							// Build the structure.
							err = build_me_cfg_device_list(doc, cur_node, cfg_XML->device_list, &cfg_XML->device_list_count, no_devices, location);
							break; // There is only one device list.
						}
					}
				}
				else
				{
					LIBPERROR("Can not get requestet memory for device_list structure.\n");
					err = ME_ERRNO_INTERNAL;
				}
			}
			else
			{
				err = ME_ERRNO_INTERNAL;
				LIBPERROR("Structure in file %s hasn't got 'device_entry' node.\n", address );
			}

		}
		else
		{
			err = ME_ERRNO_INTERNAL;
			LIBPERROR("Can not get root element node. %s hasn't correct XML structure.\n", address );
		}


		xmlFreeDoc(doc);
	}
	else
	{
		err = ME_ERRNO_INTERNAL;
		LIBPERROR("Can not read %s file.\n", address );
	}
	xmlCleanupParser();
	return err;
}

int build_me_cfg_device_list(xmlDoc* doc, xmlNode* device_list_node, me_cfg_device_entry_t** device_list, unsigned int *count, int max_dev, me_XML_bus_type_t* location)
{
	int err = ME_ERRNO_SUCCESS;
	unsigned int cnt = 0;
	xmlNode* cur_node = NULL;
	me_cfg_device_entry_t* cur_device;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = device_list_node->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "device_entry"))
		{
			if (cnt>=max_dev)
			{
				LIBPERROR("Too many devices!\n");
				cnt = max_dev;
				err = ME_ERRNO_INTERNAL;
				break;
			}

			cur_device = calloc(1, sizeof(me_cfg_device_entry_t));
			if (cur_device)
			{
				*device_list = cur_device;
				device_list++;

				err = build_me_cfg_device_entry(doc, cur_node, cur_device, location);
				if (err == MEiDS_ERRNO_XML_ENTRY_NOT_FOUND)
				{
					device_list--;
					free (cur_device);
					err = ME_ERRNO_SUCCESS;
				}
				else
				{
					cnt++;
				}
			}
			else
			{
				LIBPERROR("Can not get requestet memory for device_entry.");
				err = ME_ERRNO_INTERNAL;
			}

			if (err)
				break;
		}
	}
	*count = cnt;
	return err;
}

int build_me_cfg_device_entry(xmlDoc* doc, xmlNode* device_entry, me_cfg_device_entry_t* device, me_XML_bus_type_t* location)
{
	int err = MEiDS_ERRNO_XML_ENTRY_NOT_FOUND;
	int no_subdevices = 0;
	int device_info_access = 0;
	int subdevice_list_access = 0;

	int loc;
	int bus;

	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device->access_type = me_access_type_invalid;
	device->subdevice_list_count = 0;
	device->subdevice_list = NULL;

	marker = (char *) xmlGetProp(device_entry, (xmlChar *) "access");
	loc = strtol(marker, NULL, 10);
	xmlFree(marker);

	if ((location->location == ME_ACCESS_TYPE_ANY) || (loc == location->location))
	{
		marker = (char *) xmlGetProp(device_entry, (xmlChar *) "bus");
		bus = strtol(marker, NULL, 10);
		xmlFree(marker);

		if ((location->bus == ME_BUS_TYPE_ANY) || (bus == location->bus))
		{
			switch (loc)
			{
				case ME_ACCESS_TYPE_LOCAL:
					switch (bus)
					{
						case ME_BUS_TYPE_PCI:
							device->access_type = me_access_type_PCI;
							break;

						case ME_BUS_TYPE_USB:
							device->access_type = me_access_type_USB;
							break;

						default:
							LIBPXML("Wrong XML entry: Location:Local, Bus:0x%x(%d) not recognized", bus, bus);
							goto EXIT;
					}
					break;

				case ME_ACCESS_TYPE_REMOTE:
					switch (bus)
					{
						case 0:
							/// @note Work around for existing config files
						case ME_BUS_TYPE_PCI:
							//cPCI card on SynapseLAN
							device->access_type = me_access_type_TCPIP;
							break;

						case ME_BUS_TYPE_USB:
							//SynapseUSB via SynapseLAN
							device->access_type = me_access_type_TCPIP;
							break;

						default:
							LIBPXML("Wrong XML entry: Location:Remote, Bus:0x%x(%d) not recognized", bus, bus);
							goto EXIT;
					}
					break;

				default:
					LIBPXML("Wrong XML entry: Location:0x%x(%d) not recognized, Bus:0x%x(%d)", loc, loc, bus, bus);
					goto EXIT;
			}

			marker = (char *) xmlGetProp(device_entry, (xmlChar *) "device_number");
			device->logical_device_no = strtol(marker, NULL, 10);
			xmlFree(marker);

			marker = (char *) xmlGetProp(device_entry, (xmlChar *) "device_plugged");
			device->plugged = strtol(marker, NULL, 10);
			xmlFree(marker);

			no_subdevices = determine_node_count(device_entry, "subdevice_list", "subdevice_entry");
			if (no_subdevices)
			{
				device->subdevice_list = calloc(no_subdevices, sizeof(me_cfg_subdevice_entry_t *));
				if (device->subdevice_list)
				{
					err = ME_ERRNO_SUCCESS;
					device->info.device_no = -1;	//No device assigned, yet.

					/// @note Only 1 instance acceptable for 'device_info' and 'subdevice_list'. Others are omitted.
					for (cur_node = device_entry->children; cur_node; cur_node = cur_node->next)
					{//Interesting only in 'device_info' and 'subdevice_list' branches
						if (!strcmp((char *) cur_node->name, "device_info"))
						{
							if (!device_info_access)
							{
								err = build_me_cfg_device_info(doc, cur_node, &device->info);
								if (!err)
								{
									if (device->access_type == me_access_type_PCI)
									{
										err = build_me_cfg_pci_info(doc, cur_node, &device->info);
									}
									else if (device->access_type == me_access_type_USB)
									{
										err = build_me_cfg_usb_info(doc, cur_node, &device->info);
									}
								}
								device_info_access = 1;
							}
							else
							{
								LIBPXML("Wrong structure of XML file. Section 'device_info' repeated.\n");
							}
						}
						else if (!strcmp((char *) cur_node->name, "subdevice_list"))
						{
							if (!subdevice_list_access)
							{
								err = build_me_cfg_subdevice_list(doc, cur_node, device->subdevice_list, &device->subdevice_list_count, no_subdevices);
								subdevice_list_access = 1;
							}
							else
							{
								LIBPXML("Wrong structure of XML file. Section 'subdevice_list' repeated.\n");
							}
						}
						else if ((device->access_type == me_access_type_TCPIP) && (!strcmp((char *) cur_node->name, "tcpip")))
						{
							if (!device->info.tcpip.remote_host)
							{
								err = build_me_cfg_tcpip_info(doc, cur_node, &device->info);
							}
							else
							{
								LIBPXML("Wrong structure of XML file. Section 'tcpip' repeated.\n");
							}
						}


						if (err)
							break;
					}
				}
				else
				{
					LIBPERROR("Can not get requestet memory for subdevice_list structure.");
					err = ME_ERRNO_INTERNAL;
				}
			}

		}
	}

EXIT:
	return err;
}

int build_me_cfg_pci_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info)
{
	int err = ME_ERRNO_SUCCESS;
	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	device_info->pci.bus_no = ME_VALUE_INVALID;
	device_info->pci.device_no = ME_VALUE_INVALID;
	device_info->pci.function_no = ME_VALUE_INVALID;

	for (cur_node = info_node->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *)xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "pci_bus_no"))
		{
			device_info->pci.bus_no = strtol(strip(marker), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "pci_dev_no"))
		{
			device_info->pci.device_no = strtol(strip(marker), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "pci_func_no"))
		{
			device_info->pci.function_no = strtol(strip(marker), NULL, 10);
		}
		xmlFree(marker);
	}
	return err;
}

int build_me_cfg_usb_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info)
{
	int err = ME_ERRNO_SUCCESS;
	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = info_node->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *)xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "usb_root_hub_no"))
		{
			device_info->usb.root_hub_no = strtol(strip(marker), NULL, 10);
		}
		xmlFree(marker);
	}
	return err;
}

int build_me_cfg_tcpip_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info)
{
	int err = ME_ERRNO_SUCCESS;
	xmlNode* cur_node = NULL;
	char* marker = NULL;
	unsigned int lenght;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = info_node->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *)xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "remote_host"))
		{
			lenght = strlen(strip(marker));
			if (lenght)
			{
				device_info->tcpip.remote_host = calloc(lenght+1, sizeof(char));
				if (device_info->tcpip.remote_host)
				{
					strcpy(device_info->tcpip.remote_host, strip(marker));
				}
				else
				{
					LIBPERROR("Can not get requestet memory for remote host.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
/// @note This tag is not necessary any more. LAN device will be checked and valid ID will be used.
//		else if (!strcmp((char *) cur_node->name, "remote_device_number"))
//		{
//			device_info->device_no = strtol(strip(marker), NULL, 10);
//		}
		xmlFree(marker);
	}
	return err;
}

int build_me_cfg_device_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info)
{
	int err = ME_ERRNO_SUCCESS;
	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

/// Vendor ID is for future. For now only Meilhaus id is supported 0x1402 by MEiDS.
	device_info->vendor_id = ME_VALUE_INVALID;
	device_info->device_id = ME_VALUE_INVALID;
	device_info->serial_no = ME_VALUE_INVALID;

	device_info->device_name = NULL;
	device_info->device_description = NULL;
	unsigned int lenght;

	for (cur_node = info_node->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *)xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "vendor_id"))
		{
			device_info->vendor_id = strtol(strip(marker), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "device_id"))
		{
			device_info->device_id = strtol(strip(marker), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "serial_no"))
		{
			device_info->serial_no = strtol(strip(marker), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "device_name"))
		{
			lenght = strlen(strip(marker));
			if (lenght)
			{
				device_info->device_name = calloc(lenght+1, sizeof(char));
				if (device_info->device_name)
				{
					strcpy(device_info->device_name, strip(marker));
				}
				else
				{
					LIBPERROR("Can not get requestet memory for device_name.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
		else if (!strcmp((char *) cur_node->name, "device_description"))
		{
			lenght = strlen(strip(marker));
			if (lenght)
			{
				device_info->device_description = calloc(lenght+1, sizeof(char));
				if (device_info->device_description)
				{
					strcpy(device_info->device_description, strip(marker));
				}
				else
				{
					LIBPERROR("Can not get requestet memory for device_description.");
					err = ME_ERRNO_INTERNAL;
				}
			}
		}
		xmlFree(marker);
	}
	return err;
}

int build_me_cfg_subdevice_list(xmlDoc* doc, xmlNode* subdevice_list_node, me_cfg_subdevice_entry_t** subdevice_list, unsigned int *count, int max_subdev)
{
	int err = ME_ERRNO_SUCCESS;
	unsigned int cnt = 0;
	xmlNode* cur_node = NULL;
	me_cfg_subdevice_entry_t* cur_subdevice;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = subdevice_list_node->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "subdevice_entry"))
		{
			if (cnt>=max_subdev)
			{
				LIBPERROR("Too many subdevices!\n");
				cnt = max_subdev;
				err = ME_ERRNO_INTERNAL;
				break;
			}

			cur_subdevice = calloc(1, sizeof(me_cfg_subdevice_entry_t));
			if (cur_subdevice)
			{
				cnt++;
				*subdevice_list = cur_subdevice;
				subdevice_list++;

				err = build_me_cfg_subdevice_entry(doc, cur_node, cur_subdevice);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for subdevice entry.");
				err = ME_ERRNO_INTERNAL;
			}

			if (err)
				break;
		}
	}

	*count = cnt;
	return ME_ERRNO_SUCCESS;
	}

int build_me_cfg_subdevice_entry(xmlDoc* doc, xmlNode* subdevice_entry, me_cfg_subdevice_entry_t* subdevice)
{
	int err = ME_ERRNO_SUCCESS;
	xmlNode* cur_node = NULL;

#ifdef XML_SUBDEVICE_INFO
# ifdef XML_RANGE_INFO
	int no_ranges;
# endif
#endif

	char* ext = NULL;
	char* lock = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	subdevice->info.range_list_count = 0;
	subdevice->info.range_list = NULL;

	subdevice->info.type = ME_TYPE_INVALID;
	subdevice->info.sub_type = ME_SUBTYPE_INVALID;
	subdevice->info.channels = 0;

	ext = (char *) xmlGetProp(subdevice_entry, (xmlChar *) "subdevice_extension");
	subdevice->extention.type = strtol(ext, NULL, 10);
	xmlFree(ext);

	lock = (char *) xmlGetProp(subdevice_entry, (xmlChar *) "subdevice_lock");
	subdevice->locked = strtol(lock , NULL, 10);
	xmlFree(lock);

#ifdef XML_SUBDEVICE_INFO
# ifdef XML_RANGE_INFO
	no_ranges = determine_node_count(subdevice_entry, "range_list", "range_entry");
	if (no_ranges)
	{
		subdevice->info.range_list = calloc(no_ranges, sizeof(me_cfg_range_info_t*));
		if (!subdevice->info.range_list)
		{
			LIBPERROR("Can not get requestet memory for range list.");
			return ME_ERRNO_INTERNAL;
		}
	}
# endif
#endif

	for (cur_node = subdevice_entry->children; cur_node; cur_node = cur_node->next)
	{
#ifdef XML_SUBDEVICE_INFO
		if (!strcmp((char *) cur_node->name, "subdevice_info"))
		{
			build_me_cfg_subdevice_info(doc, cur_node, &subdevice->info);
		}
		else
# ifdef XML_RANGE_INFO
		if (!strcmp((char *) cur_node->name, "range_list"))
		{
			if (no_ranges)
			{
				err = build_me_cfg_range_list(doc, cur_node, subdevice->info.range_list, &subdevice->info.range_list_count, no_ranges);
				if (err)
					break;
			}
			else
			{
				// Shouldn't be any!
				LIBPDEBUG("'range_list' exist but there is no 'range_entry' detected\n");
			}
		}
		else
# endif
#endif
		if (!strcmp((char *) cur_node->name, "mux32m"))
		{
			build_me_cfg_mux32m(doc, cur_node, &subdevice->extention);
		}
		else if (!strcmp((char *) cur_node->name, "demux32"))
		{
			build_me_cfg_demux32(doc, cur_node, &subdevice->extention);
		}
	}
	return err;
}

#ifdef XML_SUBDEVICE_INFO
void build_me_cfg_subdevice_info(xmlDoc* doc, xmlNode* info_node, me_cfg_subdevice_info_t *info)
{
	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = info_node->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *) xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "subdevice_type"))
		{
			info->type = strtol(marker, NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "subdevice_sub_type"))
		{
			info->sub_type = strtol(marker, NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "subdevice_number_channels"))
		{
			info->channels = strtol(marker, NULL, 10);
		}
		xmlFree(marker);
	}
}

# ifdef XML_RANGE_INFO
int build_me_cfg_range_list(xmlDoc* doc, xmlNode* range_list_node, me_cfg_range_info_t** range_list, unsigned int *count, int max_ranges)
{
	int err = ME_ERRNO_SUCCESS;
	int cnt = 0;
	xmlNode* cur_node = NULL;
	me_cfg_range_info_t* cur_ranges;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	for (cur_node = range_list_node->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "range_entry"))
		{
			cnt++;
			if (cnt>max_ranges)
			{
				LIBPERROR("Too many ranges!\n");
				cnt = max_ranges;
				err = ME_ERRNO_INTERNAL;
				break;
			}

			cur_ranges = calloc(1, sizeof(me_cfg_range_info_t));
			if (cur_ranges)
			{
				*range_list = cur_ranges;
				range_list++;

				build_me_cfg_range_entry(doc, cur_node, cur_ranges);
			}
			else
			{
				LIBPERROR("Can not get requestet memory for range_entry.");
				err = ME_ERRNO_INTERNAL;
			}

			if (err)
				break;
		}
	}
	*count = cnt;
	return err;
}

void build_me_cfg_range_entry(xmlDoc* doc, xmlNode* range_entry, me_cfg_range_info_t *range)
{
	xmlNode* cur_node = NULL;
	char* marker = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	range->unit = me_units_type_invalid;
	range->min = 0.0;
	range->max = 0.0;
	range->max_data = 0;

	for (cur_node = range_entry->children; cur_node; cur_node = cur_node->next)
	{
		marker = (char *) xmlNodeListGetString(doc, cur_node->children, 0);
		if (!strcmp((char *) cur_node->name, "range_unit"))
		{
			range->unit = (enum me_units_type) strtol(marker, NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "range_min"))
		{
			range->min = strtod(marker, NULL);
		}
		else if (!strcmp((char *) cur_node->name, "range_max"))
		{
			range->max = strtod(marker, NULL);
		}
		else if (!strcmp((char *) cur_node->name, "range_max_data"))
		{
			range->max_data = strtol(marker, NULL, 10);
		}
		xmlFree(marker);
	}
}
# endif //XML_RANGE_INFO

#endif //XML_SUBDEVICE_INFO

void build_me_cfg_mux32m(xmlDoc* doc, xmlNode* mux32m, me_cfg_extention_t* extention)
{
	xmlNode* cur_node = NULL;
	char* timed = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	extention->type = me_cfg_extention_type_mux32;

	timed = (char *) xmlGetProp(mux32m, (xmlChar *) "timed");

	if (strtol(timed, NULL, 10))
	{
		extention->mux32.timed = 1;
	}
	else
	{
		extention->mux32.timed = 0;
	}

	xmlFree(timed);

	extention->mux32.ai_channel = ME_VALUE_INVALID;
	extention->mux32.dio_device = ME_VALUE_INVALID;
	extention->mux32.dio_subdevice = ME_VALUE_INVALID;
	extention->mux32.timer_subdevice = 0;
	extention->mux32.timer_subdevice = 0;
	extention->mux32.mux32s_count = 0;


	for (cur_node = mux32m->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "mux32m_ai_channel"))
		{
			extention->mux32.ai_channel = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "mux32m_dio_device"))
		{
			extention->mux32.dio_device = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "mux32m_dio_subdevice"))
		{
			extention->mux32.dio_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "mux32m_timer_device"))
		{
			extention->mux32.timer_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "mux32m_timer_subdevice"))
		{
			extention->mux32.timer_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "mux32s_list"))
		{
			get_me_cfg_mux32s_count(doc, cur_node, &extention->mux32.mux32s_count);
		}
	}
}

void build_me_cfg_demux32(xmlDoc* doc, xmlNode* demux32, me_cfg_extention_t* extention)
{
	xmlNode* cur_node = NULL;
	char* timed = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	extention->type = me_cfg_extention_type_demux32;

	timed = (char *) xmlGetProp(demux32, (xmlChar *) "timed");

	if (strtol(timed, NULL, 10))
	{
		extention->demux32.timed = 1;
	}
	else
	{
		extention->demux32.timed = 0;
	}

	xmlFree(timed);

	extention->demux32.ao_channel = ME_VALUE_INVALID;
	extention->demux32.dio_device = ME_VALUE_INVALID;
	extention->demux32.dio_subdevice = ME_VALUE_INVALID;
	extention->demux32.timer_subdevice = 0;
	extention->demux32.timer_subdevice = 0;

	for (cur_node = demux32->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "demux32_ao_channel"))
		{
			extention->demux32.ao_channel = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "demux32_dio_device"))
		{
			extention->demux32.dio_device = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "demux32_dio_subdevice"))
		{
			extention->demux32.dio_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "demux32_timer_device"))
		{
			extention->demux32.timer_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
		else if (!strcmp((char *) cur_node->name, "demux32_timer_subdevice"))
		{
			extention->demux32.timer_subdevice = strtol((char *) xmlNodeListGetString(doc, cur_node->children, 0), NULL, 10);
		}
	}
}

void get_me_cfg_mux32s_count(xmlDoc* doc, xmlNode* mux32s_list, unsigned int *count)
{
	int cnt = 0;
	xmlNode* cur_node = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	// Determine the count of mux32s entries
	cnt = 0;

	for (cur_node = mux32s_list->children; cur_node; cur_node = cur_node->next)
	{
		if (!strcmp((char *) cur_node->name, "mux32s_entry"))
		{
			cnt++;
		}
	}

	*count = cnt;
}

int ConfigBind(const me_config_t* cfg_XML, const me_config_t* cfg_Source, me_config_t* cfg_Dest, int flags)
{
	int err = 0;
 	me_cfg_device_entry_t* Reference;
	int i;

	LIBPINFO("executed: %s\n", __FUNCTION__);


	if (flags)
	{
		LIBPWARNING("Flags are not supported, yet.\n");
	}

	//First fixed entries
	for (i = 0; i < cfg_XML->device_list_count; i++)
	{
		Reference = *(cfg_XML->device_list + i);
		if ((Reference->plugged == me_plugged_type_IN) || (Reference->plugged == me_plugged_type_OUT))
		{// This is fixed one. Add to device list.
			bind_to_dedicated(Reference, cfg_Source);
			err = ConfigAppend(Reference, cfg_Dest, 0);
			if (err)
				break;
		}
	}

	if (!err)
	{
		// Rest of the boards
		for (i = 0; i < cfg_Source->device_list_count; i++)
		{
			Reference = *(cfg_Source->device_list + i);
			if (Reference->plugged == me_plugged_type_IN)
			{// This is free device.
				bind_to_first_available(Reference, cfg_XML, cfg_Dest);
				err = ConfigAppend(Reference, cfg_Dest, 0);
				if (err)
					break;
			}
		}
	}

	return err;
}

int bind_to_first_available(me_cfg_device_entry_t* Entry, const me_config_t* XML, const me_config_t* Config)
{
	/// Assign logical number to hardware instance.
	int err=ME_ERRNO_SUCCESS;
	int freeID = 0;
	int i;
	int plugged;
	int status = 0;
	me_cfg_device_entry_t* XML_Entry;

	do
	{
		status = 0;
		for (i=0; i<Config->device_list_count; i++)
		{
			if (freeID == (*(Config->device_list + i))->logical_device_no)
			{// It is already taken. Try next one.
				freeID++;
				i=0;
			}
		}

		for (i=0; i<XML->device_list_count; i++)
		{
			XML_Entry = (*(XML->device_list + i));
			if (XML_Entry->logical_device_no == freeID)
			{
				plugged = XML_Entry->plugged;

				if (plugged > me_plugged_type_CONTROL_min && plugged < me_plugged_type_max)
				{
					//Check if this device can NOT be inserted here. (MUST BE)
					if (plugged & me_plugged_type_VENDOR)
					{
						if (XML_Entry->info.vendor_id != Entry->info.vendor_id)
						{
							status = 1;
							break;
						}
					}
					if (plugged & me_plugged_type_TYPE)
					{
						if (
							(XML_Entry->info.vendor_id != Entry->info.vendor_id)
							||
							(XML_Entry->info.device_id != Entry->info.device_id)
							)
						{
							status = 1;
							break;
						}
					}
					if (plugged & me_plugged_type_SERIALNUMBER)
					{
						if (
							(XML_Entry->info.vendor_id != Entry->info.vendor_id)
							||
							(XML_Entry->info.device_id != Entry->info.device_id)
							||
							(XML_Entry->info.serial_no != Entry->info.serial_no)
							)
						{
							status = 1;
							break;
						}
					}
					if (plugged & me_plugged_type_BUS_TYPE)
					{
						if (XML_Entry->access_type != Entry->access_type)
						{
							status = 1;
							break;
						}
					}

					if (plugged & me_plugged_type_ADDR_SLOT)
					{
						if (Entry->access_type != XML_Entry->access_type)
						{
							status = 1;
							break;
						}
						else if (Entry->info.device_no != XML_Entry->info.device_no)
						{
							status = 1;
							break;
						}
						else
						{
							if (Entry->access_type == me_access_type_PCI)
							{
								// No more conditions
							}
							else if (Entry->access_type == me_access_type_TCPIP)
							{
								/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
								if (strcmp(Entry->info.tcpip.remote_host, XML_Entry->info.tcpip.remote_host))
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_USB)
							{
								if (Entry->info.usb.root_hub_no != XML_Entry->info.usb.root_hub_no)
								{
									status = 1;
									break;
								}
							}
						}
					}

					if (plugged & me_plugged_type_ADDR_MAJOR)
					{
						if (Entry->access_type != XML_Entry->access_type)
						{
							status = 1;
							break;
						}
						else
						{
							if (Entry->access_type == me_access_type_PCI)
							{
								if (Entry->info.pci.bus_no != XML_Entry->info.pci.bus_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_TCPIP)
							{
								/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
								if (strcmp(Entry->info.tcpip.remote_host, XML_Entry->info.tcpip.remote_host))
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_USB)
							{
								if (Entry->info.usb.root_hub_no != XML_Entry->info.usb.root_hub_no)
								{
									status = 1;
									break;
								}
							}
						}
					}

					if (plugged & me_plugged_type_ADDR_MINOR)
					{
						if (Entry->access_type != XML_Entry->access_type)
						{
							status = 1;
							break;
						}
						else
						{
							if (Entry->access_type == me_access_type_PCI)
							{
								if (Entry->info.pci.device_no != XML_Entry->info.pci.device_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_TCPIP)
							{
								/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
								if (Entry->info.device_no != XML_Entry->info.device_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_USB)
							{
								if (Entry->info.device_no != XML_Entry->info.device_no)
								{
									status = 1;
									break;
								}
							}
						}
					}

					//Check if this device can NOT be inserted here. (MUSTN'T BE)
					if (plugged & (me_plugged_type_VENDOR << 7))
					{
						if (XML_Entry->info.vendor_id == Entry->info.vendor_id)
						{
							status = 1;
							break;
						}
					}
					if (plugged & (me_plugged_type_TYPE << 7))
					{
						if (
							(XML_Entry->info.vendor_id == Entry->info.vendor_id)
							&&
							(XML_Entry->info.device_id == Entry->info.device_id)
							)
						{
							status = 1;
							break;
						}
					}
					if (plugged & (me_plugged_type_SERIALNUMBER << 7))
					{
						if (
							(XML_Entry->info.vendor_id == Entry->info.vendor_id)
							&&
							(XML_Entry->info.device_id == Entry->info.device_id)
							&&
							(XML_Entry->info.serial_no == Entry->info.serial_no)
							)
						{
							status = 1;
							break;
						}
					}
					if (plugged & (me_plugged_type_BUS_TYPE << 7))
					{
						if (XML_Entry->access_type == Entry->access_type)
						{
							status = 1;
							break;
						}
					}
					if (plugged & (me_plugged_type_ADDR_SLOT << 7))
					{
						if (Entry->access_type == XML_Entry->access_type)
						{
							if (Entry->info.device_no == XML_Entry->info.device_no)
							{
								if (Entry->access_type == me_access_type_PCI)
								{
									// No more conitions
									status = 1;
									break;
								}
								else if (Entry->access_type == me_access_type_TCPIP)
								{
									/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
									if (!strcmp(Entry->info.tcpip.remote_host, XML_Entry->info.tcpip.remote_host))
									{
										status = 1;
										break;
									}
								}
								else if (Entry->access_type == me_access_type_USB)
								{
									if (Entry->info.usb.root_hub_no == XML_Entry->info.usb.root_hub_no)
									{
										status = 1;
										break;
									}
								}
							}
						}
					}
					if (plugged & (me_plugged_type_ADDR_MAJOR << 7))
					{
						if (Entry->access_type == XML_Entry->access_type)
						{
							if (Entry->access_type == me_access_type_PCI)
							{
								if (Entry->info.pci.bus_no == XML_Entry->info.pci.bus_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_TCPIP)
							{
								/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
								if (!strcmp(Entry->info.tcpip.remote_host, XML_Entry->info.tcpip.remote_host))
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_USB)
							{
								if (Entry->info.usb.root_hub_no == XML_Entry->info.usb.root_hub_no)
								{
									status = 1;
									break;
								}
							}
						}
					}
					if (plugged & (me_plugged_type_ADDR_MINOR << 7))
					{
						if (Entry->access_type == XML_Entry->access_type)
						{
							if (Entry->access_type == me_access_type_PCI)
							{
								if (Entry->info.pci.device_no == XML_Entry->info.pci.device_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_TCPIP)
							{
								/// ONLY PCI for now. TCP only in draft: string comparation. Must be re-writed to cover general nameing conventions.
								if (Entry->info.device_no == XML_Entry->info.device_no)
								{
									status = 1;
									break;
								}
							}
							else if (Entry->access_type == me_access_type_USB)
							{
								if (Entry->info.device_no == XML_Entry->info.device_no)
								{
									status = 1;
									break;
								}
							}
						}
					}
				}
				else
				{// Error!
					if (plugged == me_plugged_type_IN || plugged == me_plugged_type_OUT)
					{
						LIBPERROR("bind_to_dedicated() for device_number=%d device_plugged=0x%04x. FAILED !!", freeID, plugged);
					}
					else
					{
						LIBPERROR("XML struct for device_number=%d device_plugged=0x%04x. UNRECOGNIZE", freeID, plugged);
					}
					err = ME_ERRNO_INTERNAL;
					status = 1;
					break;
				}
			}
		}
	}
	while (status);

	if (!err)
	{
		Entry->logical_device_no = freeID;
		Entry->plugged = me_plugged_type_USED;
	}
	else
	{
		Entry->logical_device_no = -1;
		Entry->plugged = me_plugged_type_BLOCKED;
	}
	return err;
}

int bind_to_dedicated(me_cfg_device_entry_t* Entry, const me_config_t* HardwareSources)
{
	/// Assign hardware instance to logical number (entry)).
	int i;
	int status = 0;
	int err=ME_ERRNO_SUCCESS;

	me_cfg_device_entry_t* HW_Entry;

	Entry->info.device_no = -1;
	for (i = 0; i < HardwareSources->device_list_count; ++i)
	{
		HW_Entry = *(HardwareSources->device_list + i);
		if (HW_Entry->plugged == me_plugged_type_IN)
		{
			if (
				(HW_Entry->access_type == Entry->access_type)
				&&
				(HW_Entry->info.vendor_id == Entry->info.vendor_id)
				&&
				(HW_Entry->info.device_id == Entry->info.device_id)
				&&
				(HW_Entry->info.serial_no == Entry->info.serial_no)
				)
			{
				switch (HW_Entry->access_type)
				{
					case me_access_type_PCI:
						if (
							(HW_Entry->info.pci.bus_no == Entry->info.pci.bus_no)
							&&
							(HW_Entry->info.pci.device_no == Entry->info.pci.device_no)
							&&
							(HW_Entry->info.pci.function_no == Entry->info.pci.function_no)
							)
						{
							status = 1;
						}
						break;

					case me_access_type_TCPIP:
						if (!strcmp(HW_Entry->info.tcpip.remote_host, Entry->info.tcpip.remote_host))
						{
							status = 1;
						}
						break;

					case me_access_type_USB:
						if (HW_Entry->info.usb.root_hub_no == Entry->info.usb.root_hub_no)
						{
							status = 1;
						}
						break;

					default:
						/// Checking not implemented.
						LIBPERROR("ACCESS TYPE 0x%04x NOT IMPLEMENTED!\n", HW_Entry->access_type);
						err = ME_ERRNO_INTERNAL;
						status = 1;
				}
				if (status)
				{
					break;
				}
			}
		}
	}

	if (status)
	{
		if (Entry->plugged == me_plugged_type_IN)
		{
			HW_Entry->plugged = me_plugged_type_USED;
		}
		else if (Entry->plugged == me_plugged_type_OUT)
		{
			HW_Entry->plugged = me_plugged_type_BLOCKED;
		}

		Entry->info.device_no = HW_Entry->info.device_no;
		Entry->context = HW_Entry->context;
	}
	else
	{
		Entry->plugged = me_plugged_type_BLOCKED;
	}
	return err;
}
