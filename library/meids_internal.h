#ifndef __KERNEL__
# ifndef _MEIDS_INTERNAL_H_
#  define _MEIDS_INTERNAL_H_

#include "meids_config_structs.h"
#include "me_types.h"

char *strip(char* s);

int context_list_init(me_context_list_t** head);
int config_list_init(me_config_t** head);

int me_translate_triggers_to_simple(meIOStreamTrigger_t* long_triggers, meIOStreamSimpleTriggers_t* simple_triggers);
int me_translate_triggers_to_unversal(meIOStreamSimpleTriggers_t* simple_triggers, meIOStreamTrigger_t* long_triggers);

int me_translate_config_to_simple(meIOStreamConfig_t* long_config, int count, int old_flags, meIOStreamSimpleConfig_t* simple_config, int* flags);
int me_translate_config_to_universal(meIOStreamSimpleConfig_t* simple_config, int count, int flags, meIOStreamConfig_t* long_config, int* iFlags);

int ContextAppend(void* context, me_context_list_t* head);
int ContextRemove(void* context, me_context_list_t* head);
int ContextClean(me_context_list_t** head);

int del_first_context(me_context_list_t* head);
int del_last_context(me_context_list_t* head);
int del_context_instance(me_dummy_context_t* instance);


# endif	//_MEIDS_INTERNAL_H_
#endif	//__KERNEL__
