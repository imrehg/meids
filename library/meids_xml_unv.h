#ifndef __KERNEL__
# ifndef _MEIDS_UNV_XML_H_
#  define _MEIDS_UNV_XML_H_

#  include "meids_config_structs.h"
#  include "meids.h"

int  ME_unv_xml_OpenDriver(const char* address, me_config_t** new_driver, void** new_context, int iFlags);
int  ME_unv_xml_CloseDriver(void* context, int iFlags);
int  ME_unv_xml_LockDriver(void* context, int lock, int iFlags);

# endif	//_MEIDS_UNV_XML_H_
#endif	//__KERNEL__
