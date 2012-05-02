#ifndef __KERNEL__
# ifndef _MEIDS_XML_H_
#  define _MEIDS_XML_H_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "meids_ioctl.h"

///Test only!
# ifndef XML_SUBDEVICE_INFO
#  define XML_SUBDEVICE_INFO
# endif
# ifndef XML_RANGE_INFO
#  define XML_RANGE_INFO
# endif

int  ConfigLoad_XML_ALL  (me_config_t *cfg_XML, char *pcConfigFile, int flags);
int  ConfigLoad_XML_PCI  (me_config_t *cfg_XML, char *pcConfigFile, int flags);
int  ConfigLoad_XML_USB  (me_config_t *cfg_XML, char *pcConfigFile, int flags);
int  ConfigLoad_XML_TCPIP(me_config_t *cfg_XML, char *pcConfigFile, int flags);
int  ConfigLoad_XML      (me_config_t *cfg_XML, char *pcConfigFile, me_XML_bus_type_t* location, int flags);

int  build_me_cfg_device_list(xmlDoc *doc, xmlNode *device_list_node, me_cfg_device_entry_t** device_list, unsigned int *count, int max_dev, me_XML_bus_type_t* location);
int  build_me_cfg_device_entry(xmlDoc *doc, xmlNode *device_entry, me_cfg_device_entry_t* device, me_XML_bus_type_t* location);

int  build_me_cfg_pci_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info);
int  build_me_cfg_usb_info(xmlDoc* doc, xmlNode* info_node, me_cfg_device_info_t* device_info);
int  build_me_cfg_tcpip_info(xmlDoc *doc, xmlNode *info_node, me_cfg_device_info_t *device_info);
int  build_me_cfg_device_info(xmlDoc *doc, xmlNode *info_node, me_cfg_device_info_t *device_info);

int  build_me_cfg_subdevice_list(xmlDoc *doc, xmlNode *subdevice_list_node, me_cfg_subdevice_entry_t** subdevice_list, unsigned int *count, int max_subdev);
int  build_me_cfg_subdevice_entry(xmlDoc *doc, xmlNode *subdevice_entry, me_cfg_subdevice_entry_t* subdevice);
#ifdef XML_SUBDEVICE_INFO
void build_me_cfg_subdevice_info(xmlDoc *doc, xmlNode *info_node, me_cfg_subdevice_info_t *info);
# ifdef XML_RANGE_INFO
int  build_me_cfg_range_list(xmlDoc *doc, xmlNode *range_list_node, me_cfg_range_info_t** range_list, unsigned int *count, int max_ranges);
void build_me_cfg_range_entry(xmlDoc *doc, xmlNode *range_entry, me_cfg_range_info_t *range);
# endif //XML_RANGE_INFO
#endif //XML_SUBDEVICE_INFO

void build_me_cfg_mux32m(xmlDoc *doc, xmlNode *mux32m, me_cfg_extention_t* extention);
void build_me_cfg_demux32(xmlDoc *doc, xmlNode *demux32, me_cfg_extention_t* extention);
void get_me_cfg_mux32s_count(xmlDoc *doc, xmlNode *mux32s_list, unsigned int *count);

int  ConfigBind(const me_config_t* cfg_XML, const me_config_t* cfg_Source, me_config_t *cfg_Dest, int flags);
int  bind_to_dedicated(me_cfg_device_entry_t* Entry, const me_config_t* HardwareSources);
int  bind_to_first_available(me_cfg_device_entry_t* Entry, const me_config_t* XML, const me_config_t* Config);

# endif	//_MEIDS_XML_H_
#endif	//__KERNEL__
