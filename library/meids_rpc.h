#ifndef __KERNEL__
# ifndef _MEIDS_RPC_H_
#  define _MEIDS_RPC_H_

#  include "meids_config_structs.h"
#  include "meids.h"

int  ME_rpc_OpenDriver(const char* address, me_config_t** new_driver, void** new_context, int iFlags);
int  ME_rpc_CloseDriver(void* context, int iFlags);
int  ME_rpc_LockDriver(me_rpc_context_t* context, int lock, int iFlags);

# endif	//_MEIDS_RPC_H_
#else
# error KERNEL???
#endif	//__KERNEL__
