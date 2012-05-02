#ifndef __KERNEL__
# ifndef _MEIDS_CONFIG_H_
#  define _MEIDS_CONFIG_H_

#  include "meids_structs.h"

/// Add 1 entry (device) to existing config.
int  ConfigAppend(const me_cfg_device_entry_t* cfg_Source, me_config_t* cfg_Dest, int flags);

/// Set logical numbers.
int  ConfigEnumerate(me_config_t* cfg, int start_enum, int flags);
int  ConfigContinueEnumerate(me_config_t* cfg, int start_enum, int flags);

/// Reset logical numbers.
int  ConfigDenumerate(me_config_t* cfg, int flags);

/// Make copy of it.
int  ConfigDuplicate(const me_config_t* cfg_Source, me_config_t** Dest, int flags);

/// Make new instance combining Firs and Second source.
int  ConfigJoin(const me_config_t* cfg_First, const me_config_t* cfg_Second, me_config_t** Dest, int flags);

/// Check it
int  ConfigVerify(me_config_t *cfg, int flags);

/// Clean it.
void ConfigClean(me_config_t *cfg, int flags);

/// Return 'real' device assigned to logical number.
int  ConfigResolve(const me_config_t* cfg, const int logical_nono, me_cfg_device_entry_t** reference);

/// Print config.
void ConfigPrint(const me_config_t* cfg);

/// Create reference table.
int  ShortcutBuild(const me_config_t* cfg, me_config_shortcut_table_t* table);
/// Return 'real' device assigned to logical number.
int  ShortcutResolve(const me_config_shortcut_table_t* table, const int logical_no, me_cfg_device_entry_t** reference);
/// Clear it
void ShortcutClean(me_config_shortcut_table_t* table);

int  ConfigMaxNumber(const me_config_t* cfg, int* number, int flags);

# endif	//_MEIDS_CONFIG_H_
#endif	//__KERNEL__
