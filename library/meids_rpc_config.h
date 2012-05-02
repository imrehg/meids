#ifndef __KERNEL__
# ifndef _MEIDS_RPC_CONFIG_H_
#  define _MEIDS_RPC_CONFIG_H_

#  include "meids_structs.h"
#  include "meids_config.h"

/// Read config from device (context).
int  ConfigRead_RPC(me_rpc_context_t* context, me_config_t *cfg, const char* address, int flags);

# endif	//_MEIDS_RPC_CONFIG_H_
#endif	//__KERNEL__
