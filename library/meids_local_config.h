#ifndef __KERNEL__
# ifndef _MEIDS_LOCAL_CONFIG_H_
#  define _MEIDS_LOCAL_CONFIG_H_

#  include "meids_structs.h"
#  include "meids_config.h"

/// Read config from device (context).
int  ConfigRead_Local(me_local_context_t* context, me_config_t *cfg, int flags);

# endif	//_MEIDS_LOCAL_CONFIG_H_
#endif	//__KERNEL__
