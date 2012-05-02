#ifndef __KERNEL__
# ifndef _MEIDS_UNV_H_
#  define _MEIDS_UNV_H_

#  include "meids_config_structs.h"
#  include "meids.h"

int  ME_unv_OpenDriver(const char* address, me_config_t** new_driver, void** new_context, int iFlags);
int  ME_unv_CloseDriver(void* context, int iFlags);
int  ME_unv_LockDriver(void* context, int lock, int iFlags);

# endif	//_MEIDS_UNV_H_
#else
# error KERNEL???
#endif	//__KERNEL__
