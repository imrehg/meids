# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <ctype.h>

# include <errno.h>

# include "meids_config_structs.h"
# include "meids_debug.h"
# include "me_defines.h"
# include "me_error.h"

# ifndef NO_RPC_ALLOWED
#  include "rmedriver.h"
# endif

# include "meids_internal.h"

# ifndef NO_RPC_ALLOWED
static int del_rpc_context_instance(me_rpc_context_t* instance);
# endif

static int del_local_context_instance(me_local_context_t* instance);

int ContextAppend(void* context, me_context_list_t* head)
{
	me_context_list_t* new_instance;
	me_context_list_t* place;
	int err = 0;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!head)
		return ME_ERRNO_INVALID_POINTER;

	new_instance = calloc(1, sizeof(me_context_list_t));
	if (!new_instance)
	{
		err = -ENOMEM;
		goto ERROR;
	}

	new_instance->context = NULL;
	new_instance->next = NULL;

	place = head;
	while (place->next)
	{
		place = place->next;
		if (place == place->next)
		{
			LIBPCRITICALERROR("Error in list. Entry is binded to yourself!\n");
			err = ME_ERRNO_INTERNAL;
			goto ERROR;
		}
	}

	if (place->context)
	{
		LIBPCRITICALERROR("Error in list. Last entry is not (NULL, NULL) but (%p, %p)\n", place->context, place->next);
		err = ME_ERRNO_INTERNAL;
		goto ERROR;
	}

	place->next = new_instance;
	place->context = context;

ERROR:

	if (err)
	{
		if (new_instance)
		{
			free(new_instance);
			new_instance = NULL;
		}
	}
	return err;
}

int ContextRemove(void* context, me_context_list_t* head)
{
	me_context_list_t* place = head;
	me_context_list_t* to_remove;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!head)
		return ME_ERRNO_INVALID_POINTER;

	if (!context)
		return ME_ERRNO_INVALID_POINTER;

	while (place->next)
	{
		if (place->context == context)
		{
			to_remove = place->next;
			place->context = place->next->context;
			place->next = place->next->next;

			free(to_remove);
			to_remove = NULL;

			return 0;
		}
		place = head->next;
	}

	if (place->context)
	{
		LIBPCRITICALERROR("Error in list. Last entry is not (NULL, NULL) but (%p, %p)\n", place->context, place->next);
		return ME_ERRNO_INTERNAL;
	}

	return -4;
}

int ContextClean(me_context_list_t** head)
{
	if (!head)
		return ME_ERRNO_INVALID_POINTER;
	if (!*head)
		return ME_ERRNO_INVALID_POINTER;

	if ((*head)->next)
	{
		ContextClean(&((*head)->next));
		del_context_instance((*head)->context);
		if ((*head)->context)
		{
			free((*head)->context);
			(*head)->context = NULL;
		}
	}

	free(*head);
	*head = NULL;

	return 0;
}

int del_first_context(me_context_list_t* head)
{
	me_context_list_t* place = head;
	me_context_list_t* to_remove;

	if (!place->next)
	{
		if (place)
		{
			free(place);
			place = NULL;
		}
	}
	else
	{
		to_remove = head->next;
		place->context = place->next->context;
		place->next = place->next->next;

		del_context_instance(to_remove->context);
		if (to_remove->context)
		{
			free(to_remove->context);
			to_remove->context = NULL;
		}
		if (to_remove)
		{
			free(to_remove);
			to_remove = NULL;
		}
	}

	return 0;
}

int del_last_context(me_context_list_t* head)
{
	me_context_list_t* place = head;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!place)
		return -1;

	if (!place->next)
	{
		if (place)
		{
			free(place);
			place = NULL;
		}
		return 0;
	}

	while (place->next->next)
	{
		place = place->next;
	}

	if (place->context)
	{
		free(place->context);
		place->context = NULL;
	}
	if (place->next)
	{
		free(place->next);
		place->next = NULL;
	}

	return 0;
}

int del_context_instance(me_dummy_context_t* instance)
{
	int err = ME_ERRNO_SUCCESS;
	switch (instance->context_type)
	{
		case me_context_type_local:
			err = del_local_context_instance((me_local_context_t *)instance);
			break;

# ifndef NO_RPC_ALLOWED
		case me_context_type_remote:
			err = del_rpc_context_instance((me_rpc_context_t *)instance);
			break;
# endif

		default:
			break;
	}

	return err;
}

# ifndef NO_RPC_ALLOWED
static int del_rpc_context_instance(me_rpc_context_t* instance)
{
	int flags = ME_VALUE_NOT_USED;
#  if defined RPC_USE_SUBCONTEXT
	int device;
	me_rpc_devcontext_t* device_context;
	int subdevice;
	me_rpc_subdevcontext_t* subdevice_context;
#  endif
	int err = ME_ERRNO_SUCCESS;
	int *rpc_err;

	if (instance)
	{
#  if defined RPC_USE_SUBCONTEXT
		for (device = 0; device < instance->count; ++device)
		{
			device_context = instance->device_context + device;
			if (device_context)
			{
				for (subdevice = 0; subdevice < device_context->count; ++subdevice)
				{
					subdevice_context = device_context->subdevice_context + subdevice;
					if (subdevice_context)
					{
						if (subdevice_context->fd)
						{
							rpc_err = me_close_proc_1(&flags, subdevice_context->fd);
							if (rpc_err)
							{
								free(rpc_err);
								rpc_err = NULL;
							}

							clnt_destroy(subdevice_context->fd);
						}
					}
				}
				free (device_context->subdevice_context);
				device_context->subdevice_context = NULL;

				if (device_context->fd)
				{
					rpc_err = me_close_proc_1(&flags, device_context->fd);
					if (rpc_err)
					{
						free(rpc_err);
						rpc_err = NULL;
					}

					clnt_destroy(device_context->fd);
				}

				free (device_context);
				device_context = NULL;
			}
		}
#  endif
		rpc_err = me_close_proc_1(&flags, instance->fd);
		err = (!rpc_err) ? ME_ERRNO_COMMUNICATION : *rpc_err;
		if (rpc_err)
		{
			free(rpc_err);
			rpc_err = NULL;
		}

		if (instance->fd)
		{
			clnt_destroy(instance->fd);
		}
	}

	return err;
}
# endif

static int del_local_context_instance(me_local_context_t* instance)
{
	int err = ME_ERRNO_SUCCESS;

	return err;
}

char* strip(char* raw)
{/// Strip whitspace from a string (from begin and end).
	char* p = NULL;
	char* e;

	if (raw && strlen(raw))
	{
		e = raw + strlen(raw);
		for(p=raw; p<e; ++p)
		{
			if (!isspace(*p))
				break;
		}

		for(--e; p<e; --e)
		{
			if (isspace(*e))
			{
				*e ='\0';
			}
			else
			{
				break;
			}
		}
	}
	return p;
}

int me_translate_triggers_to_simple(meIOStreamTrigger_t* long_triggers, meIOStreamSimpleTriggers_t* simple_triggers)
{
	int err = ME_ERRNO_SUCCESS;

	uint64_t acq_ticks;
	uint64_t scan_ticks;
	uint64_t conv_ticks;

	// Convert ticks to 64 bit long values
	acq_ticks = (uint64_t) long_triggers->iAcqStartTicksLow + ((uint64_t) long_triggers->iAcqStartTicksHigh << 32);
	scan_ticks = (uint64_t) long_triggers->iScanStartTicksLow + ((uint64_t) long_triggers->iScanStartTicksHigh << 32);
	conv_ticks = (uint64_t) long_triggers->iConvStartTicksLow + ((uint64_t) long_triggers->iConvStartTicksHigh << 32);

	// Aceptable settings:
	// iAcqStartTrigType		:	iScanStartTrigType			:	iConvStartTrigType

	// ME_TRIG_TYPE_SW			:	ME_TRIG_TYPE_TIMER			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_SOFTWARE
	// ME_TRIG_TYPE_SW			:	ME_TRIG_TYPE_FOLLOW			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_SOFTWARE scan_ticks=0

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_TIMER			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_ACQ_DIGITAL
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_TIMER			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_ACQ_ANALOG

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_FOLLOW			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_ACQ_DIGITAL scan_ticks=0
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_FOLLOW			:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_ACQ_ANALOG scan_ticks=0

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_LIST_DIGITAL
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_EXT_ANALOG		:	ME_TRIG_TYPE_TIMER			-> ME_TRIGGER_TYPE_LIST_ANALOG

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_EXT_DIGITAL	-> ME_TRIGGER_TYPE_CONV_DIGITAL
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_EXT_ANALOG		:	ME_TRIG_TYPE_EXT_ANALOG		-> ME_TRIGGER_TYPE_CONV_ANALOG

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_TIMER			:	ME_TRIG_TYPE_EXT_DIGITAL	-> ME_TRIGGER_TYPE_CONV_DIGITAL
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_TIMER			:	ME_TRIG_TYPE_EXT_ANALOG		-> ME_TRIGGER_TYPE_CONV_ANALOG

	// ME_TRIG_TYPE_EXT_DIGITAL	:	ME_TRIG_TYPE_FOLLOW			:	ME_TRIG_TYPE_EXT_DIGITAL	-> ME_TRIGGER_TYPE_CONV_DIGITAL scan_ticks=0
	// ME_TRIG_TYPE_EXT_ANALOG	:	ME_TRIG_TYPE_FOLLOW			:	ME_TRIG_TYPE_EXT_ANALOG		-> ME_TRIGGER_TYPE_CONV_ANALOG scan_ticks=0


	if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_SW) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (!scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_SW) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (!scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (!scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		if (!scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		if (!scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType ==ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else if ((long_triggers->iAcqStartTrigType ==ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		if (scan_ticks)
		{
			LIBPERROR("Invalid scan start argument specified. %llu\n", (long long unsigned int)scan_ticks);
			err = ME_ERRNO_INVALID_SCAN_START_ARG;
			goto ERROR;
		}
	}
	else
	{
		LIBPERROR("Invalid trigger combination specified.\n");
		err = ME_ERRNO_INVALID_ACQ_START_TRIG_TYPE;
		goto ERROR;
	}

	// Aceptable settings:
	// iScanStopTrigType		:	iAcqStopTrigType

	// ME_TRIG_TYPE_NONE		:	ME_TRIG_TYPE_NONE	-> ME_STREAM_STOP_TYPE_MANUAL
	// ME_TRIG_TYPE_NONE		:	ME_TRIG_TYPE_COUNT	-> ME_STREAM_STOP_TYPE_ACQ_LIST
	// ME_TRIG_TYPE_COUNT		:	ME_TRIG_TYPE_FOLLOW	-> ME_STREAM_STOP_TYPE_SCAN_VALUE

	if (long_triggers->iFlags)
	{
		LIBPERROR("Invalid trigger flag specified.\n");
		err = ME_ERRNO_INVALID_FLAGS;
		goto ERROR;
	}

	switch (long_triggers->iScanStopTrigType)
	{

		case ME_TRIG_TYPE_NONE:
			break;

		case ME_TRIG_TYPE_COUNT:
			if (long_triggers->iScanStopCount <= 0)
			{
				LIBPERROR("Invalid scan stop argument specified.\n");
				err = ME_ERRNO_INVALID_SCAN_STOP_ARG;
				goto ERROR;
			}
			break;

		default:
			LIBPERROR("Invalid scan stop trigger type specified.\n");
			err = ME_ERRNO_INVALID_SCAN_STOP_TRIG_TYPE;
			goto ERROR;
	}

	switch (long_triggers->iAcqStopTrigType)
	{
		case ME_TRIG_TYPE_NONE:
			if (long_triggers->iScanStopTrigType != ME_TRIG_TYPE_NONE)
			{
				LIBPERROR("Invalid acq stop trigger type or scan stop trigger type specified. Must be ME_TRIG_TYPE_NONE and ME_TRIG_TYPE_NONE. (%x %x)\n", long_triggers->iAcqStopTrigType, long_triggers->iScanStopTrigType);
				err = ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE;
				goto ERROR;
			}
			break;

		case ME_TRIG_TYPE_FOLLOW:
			if (long_triggers->iScanStopTrigType != ME_TRIG_TYPE_COUNT)
			{
				LIBPERROR("Invalid acq stop trigger type or scan stop trigger type specified. Must be ME_TRIG_TYPE_FOLLOW and ME_TRIG_TYPE_COUNT.\n");
				err = ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE;
				goto ERROR;
			}
			break;

		case ME_TRIG_TYPE_COUNT:
			if (long_triggers->iScanStopTrigType != ME_TRIG_TYPE_NONE)
			{
				LIBPERROR("Invalid acq stop trigger type or scan stop trigger type specified. Must be ME_TRIG_TYPE_COUNT and ME_TRIG_TYPE_NONE.\n");
				err = ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE;
				goto ERROR;
			}

			if (long_triggers->iAcqStopCount <= 0)
			{
				LIBPERROR("Invalid acquisition or scan stop argument specified.\n");
				err = ME_ERRNO_INVALID_ACQ_STOP_ARG;
				goto ERROR;
			}
			break;

		default:
			LIBPERROR("Invalid acq stop trigger type specified.\n");
			err = ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE;
			goto ERROR;
	}

	simple_triggers->trigger_edge = long_triggers->iAcqStartTrigEdge;
	simple_triggers->synchro = long_triggers->iAcqStartTrigChan;

	simple_triggers->acq_ticks = acq_ticks;
	simple_triggers->scan_ticks = scan_ticks;
	simple_triggers->conv_ticks = conv_ticks;

	if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_SW) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_SOFTWARE;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_SW) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_SOFTWARE;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_ACQ_DIGITAL;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_ACQ_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_ACQ_DIGITAL;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_ACQ_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_LIST_DIGITAL;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_TIMER))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_LIST_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_DIGITAL;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType == ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_TIMER) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_DIGITAL;
	}
	else if ((long_triggers->iAcqStartTrigType ==ME_TRIG_TYPE_EXT_DIGITAL) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_DIGITAL))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_ANALOG;
	}
	else if ((long_triggers->iAcqStartTrigType ==ME_TRIG_TYPE_EXT_ANALOG) && (long_triggers->iScanStartTrigType == ME_TRIG_TYPE_FOLLOW) && (long_triggers->iConvStartTrigType == ME_TRIG_TYPE_EXT_ANALOG))
	{
		simple_triggers->trigger_type = ME_TRIGGER_TYPE_CONV_DIGITAL;
	}

	if ((long_triggers->iScanStopTrigType == ME_TRIG_TYPE_NONE) && (long_triggers->iAcqStopTrigType == ME_TRIG_TYPE_NONE))
	{
		simple_triggers->stop_type = ME_STREAM_STOP_TYPE_MANUAL;
		simple_triggers->stop_count = 0;
		simple_triggers->trigger_point = 0;
	}
	else if ((long_triggers->iScanStopTrigType == ME_TRIG_TYPE_NONE) && (long_triggers->iAcqStopTrigType == ME_TRIG_TYPE_COUNT))
	{
		simple_triggers->stop_type = ME_STREAM_STOP_TYPE_ACQ_LIST;
		simple_triggers->stop_count = long_triggers->iAcqStopCount;
		simple_triggers->trigger_point = long_triggers->iAcqStopArgs[0];
	}
	else if ((long_triggers->iScanStopTrigType == ME_TRIG_TYPE_COUNT) && (long_triggers->iAcqStopTrigType == ME_TRIG_TYPE_FOLLOW))
	{
		simple_triggers->stop_type = ME_STREAM_STOP_TYPE_SCAN_VALUE;
		simple_triggers->stop_count = long_triggers->iScanStopCount;
		simple_triggers->trigger_point = long_triggers->iScanStopArgs[0];
	}

	simple_triggers->trigger_level_upper	= long_triggers->iAcqStartArgs[0];
	simple_triggers->trigger_level_lower	= long_triggers->iAcqStartArgs[1];

ERROR:
	return err;
}

int me_translate_triggers_to_unversal(meIOStreamSimpleTriggers_t* simple_triggers, meIOStreamTrigger_t* long_triggers)
{
	int err = ME_ERRNO_SUCCESS;

	switch (simple_triggers->trigger_type)
	{
		case ME_TRIGGER_TYPE_SOFTWARE:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_SW;
			long_triggers->iScanStartTrigType = (simple_triggers->scan_ticks) ? ME_TRIG_TYPE_TIMER : ME_TRIG_TYPE_FOLLOW;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_TIMER;
			break;

		case ME_TRIGGER_TYPE_ACQ_DIGITAL:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			long_triggers->iScanStartTrigType = (simple_triggers->scan_ticks) ? ME_TRIG_TYPE_TIMER : ME_TRIG_TYPE_FOLLOW;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_TIMER;
			break;

		case ME_TRIGGER_TYPE_ACQ_ANALOG:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			long_triggers->iScanStartTrigType = (simple_triggers->scan_ticks) ? ME_TRIG_TYPE_TIMER : ME_TRIG_TYPE_FOLLOW;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_TIMER;
			break;

		case ME_TRIGGER_TYPE_LIST_DIGITAL:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			long_triggers->iScanStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_TIMER;
			break;

		case ME_TRIGGER_TYPE_LIST_ANALOG:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			long_triggers->iScanStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_TIMER;
			break;

		case ME_TRIGGER_TYPE_CONV_DIGITAL:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			long_triggers->iScanStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;
			break;

		case ME_TRIGGER_TYPE_CONV_ANALOG:
			long_triggers->iAcqStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			long_triggers->iScanStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			long_triggers->iConvStartTrigType = ME_TRIG_TYPE_EXT_ANALOG;
			break;
	}

	simple_triggers->trigger_edge = long_triggers->iAcqStartTrigEdge;
	simple_triggers->synchro = long_triggers->iAcqStartTrigChan;

	// Convert ticks to 64 bit long values
	long_triggers->iAcqStartTicksLow = simple_triggers->acq_ticks;
	long_triggers->iAcqStartTicksHigh = (simple_triggers->acq_ticks >> 32);

	long_triggers->iScanStartTicksLow = simple_triggers->scan_ticks;
	long_triggers->iScanStartTicksHigh = (simple_triggers->scan_ticks >> 32);

	long_triggers->iConvStartTicksLow = simple_triggers->conv_ticks;
	long_triggers->iConvStartTicksHigh = (simple_triggers->conv_ticks >> 32);

	// Aceptable settings:
	// iScanStopTrigType		:	iAcqStopTrigType

	// ME_TRIG_TYPE_NONE		:	ME_TRIG_TYPE_NONE	-> ME_STREAM_STOP_TYPE_MANUAL
	// ME_TRIG_TYPE_NONE		:	ME_TRIG_TYPE_COUNT	-> ME_STREAM_STOP_TYPE_ACQ_LIST
	// ME_TRIG_TYPE_COUNT		:	ME_TRIG_TYPE_FOLLOW	-> ME_STREAM_STOP_TYPE_SCAN_VALUE

	switch (simple_triggers->stop_type)
	{

		case ME_STREAM_STOP_TYPE_MANUAL:
			long_triggers->iScanStopTrigType = ME_TRIG_TYPE_NONE;
			long_triggers->iAcqStopTrigType = ME_TRIG_TYPE_NONE;
			long_triggers->iAcqStopCount = 0;
			long_triggers->iScanStopCount = 0;
			break;

		case ME_STREAM_STOP_TYPE_ACQ_LIST:
			long_triggers->iScanStopTrigType = ME_TRIG_TYPE_NONE;
			long_triggers->iAcqStopTrigType = ME_TRIG_TYPE_COUNT;
			long_triggers->iAcqStopCount = simple_triggers->stop_count;
			long_triggers->iScanStopCount = 0;
			break;

		case ME_STREAM_STOP_TYPE_SCAN_VALUE:
			long_triggers->iScanStopTrigType = ME_TRIG_TYPE_COUNT;
			long_triggers->iAcqStopTrigType = ME_TRIG_TYPE_FOLLOW;
			long_triggers->iAcqStopCount = 0;
			long_triggers->iScanStopCount = simple_triggers->stop_count;
			break;

		default:
			long_triggers->iScanStopTrigType = ME_TRIG_TYPE_NONE;
			long_triggers->iAcqStopTrigType = ME_TRIG_TYPE_NONE;
			long_triggers->iAcqStopCount = 0;
			long_triggers->iScanStopCount = 0;
			LIBPERROR("Invalid scan stop trigger type specified.\n");
			err = ME_ERRNO_INVALID_SCAN_STOP_TRIG_TYPE;
			break;
	}

	long_triggers->iAcqStartArgs[0] = simple_triggers->trigger_level_upper;
	long_triggers->iAcqStartArgs[1] = simple_triggers->trigger_level_lower;
	long_triggers->iScanStopArgs[0] = simple_triggers->trigger_point;

	return err;
}

int me_translate_config_to_simple(meIOStreamConfig_t* long_config, int count, int old_flags, meIOStreamSimpleConfig_t* simple_config, int* flags)
{
	int i;

	*flags = old_flags;

	for (i = 0; i < count; i++)
	{
		switch ((long_config + i)->iRef)
		{
			case ME_REF_AIO_GROUND:
			case ME_REF_AIO_DIFFERENTIAL:
			case ME_REF_AI_GROUND:
			case ME_REF_AI_DIFFERENTIAL:
			case ME_REF_AO_GROUND:
			case ME_REF_AO_DIFFERENTIAL:
				break;

			default:
				LIBPERROR("Invalid references specified on position %d. Must be GROUND or DIFFERENTIAL.\n", i);
				return ME_ERRNO_INVALID_REF;
		}

		if ((long_config)->iRef != (long_config + i)->iRef)
		{
			LIBPERROR("Mixed references in the list. Ref[0]=0x%x Ref[%d]=0x%x\n", (long_config)->iRef, i, (long_config + i)->iRef);
			return ME_ERRNO_INVALID_REF;
		}

		if ((long_config + i)->iFlags & ~ME_IO_STREAM_CONFIG_TYPE_EXTRA_SHUNT)
		{
			LIBPERROR("Invalid meStreamConfig.iFlags specified. Should be ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS\n");
			return ME_ERRNO_INVALID_FLAGS;
		}

	}

	for (i = 0; i < count; i++)
	{
		(simple_config+ i)->iChannel = (long_config + i)->iChannel;
		(simple_config+ i)->iRange = (long_config + i)->iStreamConfig;
		if ((long_config + i)->iFlags & ME_IO_STREAM_CONFIG_TYPE_EXTRA_SHUNT)
		{
			(simple_config+ i)->iRange += ME_AI_EXTRA_RANGE;
		}
	}

	if ((long_config->iRef == ME_REF_AIO_DIFFERENTIAL) || (long_config->iRef == ME_REF_AI_DIFFERENTIAL) || (long_config->iRef == ME_REF_AO_DIFFERENTIAL))
	{
		*flags |= ME_STREAM_CONFIG_DIFFERENTIAL;
	}

	return ME_ERRNO_SUCCESS;
}

int me_translate_config_to_universal(meIOStreamSimpleConfig_t* simple_config, int count, int flags, meIOStreamConfig_t* long_config, int* iFlags)
{
	int i;

	for (i = 0; i < count; i++)
	{
		(long_config + i)->iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;
		(long_config + i)->iChannel = (simple_config+ i)->iChannel;
		if ((simple_config+ i)->iRange & ME_AI_EXTRA_RANGE)
		{
			(long_config + i)->iStreamConfig = (simple_config+ i)->iRange & ~ME_AI_EXTRA_RANGE;
			(long_config + i)->iFlags |= ME_IO_STREAM_CONFIG_TYPE_EXTRA_SHUNT;
		}
		else
		{
			(long_config + i)->iStreamConfig = (simple_config+ i)->iRange;
		}
		(long_config + i)->iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;
		if (flags & ME_STREAM_CONFIG_DIFFERENTIAL)
		{
			(long_config + i)->iRef = ME_REF_AIO_DIFFERENTIAL;
		}
		else
		{
			(long_config + i)->iRef = ME_REF_AIO_GROUND;
		}
	}

	*iFlags = flags & ~ME_STREAM_CONFIG_DIFFERENTIAL;
	return ME_ERRNO_SUCCESS;
}

int context_list_init(me_context_list_t** head)
{
	if (!head)
		return ME_ERRNO_INTERNAL;

	*head = calloc(1, sizeof(me_context_list_t));

	if (!*head)
		return -ENOMEM;

	(*head)->next = NULL;
	(*head)->context = NULL;

	return 0;
}

int config_list_init(me_config_t** head)
{
	if (!head)
		return ME_ERRNO_INTERNAL;

	*head = calloc(1, sizeof(me_config_t));
	if (!*head)
		return -ENOMEM;

	(*head)->device_list = NULL;
	(*head)->device_list_count = 0;

	return 0;
}

