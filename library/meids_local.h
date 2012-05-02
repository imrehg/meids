#ifndef __KERNEL__
# ifndef _MEIDS_LOCAL_H_
#  define _MEIDS_LOCAL_H_

#  include "meids_config_structs.h"
#  include "meids.h"

int  ME_local_OpenDriver(const char* address, me_config_t** new_driver, void** new_context, int iFlags);
int  ME_local_CloseDriver(void* context, int iFlags);
int  ME_local_LockDriver(void* context, int lock, int iFlags);

# endif	//_MEIDS_LOCAL_H_
#else
# error KERNEL???
#endif	//__KERNEL__
