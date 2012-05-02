/* Shared library for Meilhaus driver system (RPC).
 * ==========================================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <fcntl.h>
# include <errno.h>

# include <rpc/rpc.h>
# include <float.h>
# include <math.h>

# include "rmedriver.h"

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_internal.h"
# include "meids_debug.h"
# include "meids_rpc_calls.h"

static int   doCreateThread_RPC(me_rpc_context_t* context, int device, int subdevice, void* fnThread, void* fnCB, void* contextCB, int iFlags);
static int   doDestroyAllThreads_RPC(me_rpc_context_t* context);
static int   doDestroyThreads_RPC(me_rpc_context_t* context, int device);
static int   doDestroyThread_RPC(me_rpc_context_t* context, int device, int subdevice);

static void* irqThread_RPC(void* arg);
static void* streamStartThread_RPC(void* arg);
static void* streamStopThread_RPC(void* arg);
static void* streamNewValuesThread_RPC(void* arg);
static int   checkRPC(me_rpc_context_t* rpc_context);

// Open synchronization
static pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond;
pthread_t Test_RPC_thread;
void Test_RPC(const char* address);

extern int test_RPC_timeout;

static int Open_Context(me_rpc_context_t *context, const char* address, int iFlags);
static int Close_Context(me_rpc_context_t* context, int iFlags);

#if defined RPC_USE_SUBCONTEXT
static int Open_DevContext(me_rpc_devcontext_t* context, char* address, int device, int iFlags);
static int Open_SubDevContext(me_rpc_subdevcontext_t* context, char* address, int iFlags);

static int Close_DevContext(me_rpc_devcontext_t* context, int iFlags);
static int Close_SubDevContext(me_rpc_subdevcontext_t* context, int iFlags);
#endif

int Open_RPC(me_rpc_context_t *context, const char* address, int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	if (iFlags != ME_OPEN_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (!context->fd)
	{
		err = Open_Context(context, (address) ? address : "192.168.20.228", iFlags);
	}
	else
	{// Already open
		LIBPDEBUG("Already open!\n");
	}

	if (err)
	{
		LIBPERROR("Open failed!\n");
		Close_Context(context, 0);
	}

	return err;
}

static int Open_Context(me_rpc_context_t* context, const char* address, int iFlags)
{
	int* RPC_open_res ;
#if defined RPC_USE_SUBCONTEXT
	me_query_number_devices_res* RPC_res = NULL;
	int i;
#endif
	int err = ME_ERRNO_SUCCESS;

	struct timeval base_time;
	struct timezone base_time_tz;
	struct timespec timeout_tspec;
	int thread_status;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	context->access_point_addr = calloc(strlen(address)+1, sizeof(char));
	if (!context->access_point_addr)
	{
		LIBPERROR("Can not get requestet memory for addres.\n");
		return ME_ERRNO_INTERNAL;
	}
	strncpy(context->access_point_addr, address, strlen(address));

	if (test_RPC_timeout)
	{
		pthread_mutex_lock(&condition_mutex);
			pthread_cond_init(&condition_cond, NULL);

			gettimeofday(&base_time, &base_time_tz);
			timeout_tspec.tv_nsec = base_time.tv_usec + (test_RPC_timeout * 1000);
			while (timeout_tspec.tv_nsec >= 1000000)
			{
				timeout_tspec.tv_nsec -= 1000000;
				timeout_tspec.tv_sec += 1;
			}
			timeout_tspec.tv_nsec *= 1000;

			pthread_create(&Test_RPC_thread, NULL, (void *)&Test_RPC, (void *)address);
			thread_status = pthread_cond_timedwait(&condition_cond, &condition_mutex, &timeout_tspec);

			pthread_cond_destroy(&condition_cond);
		pthread_mutex_unlock(&condition_mutex);

		if (thread_status)
		{
			LIBPERROR("Connection not possible: Test_RPC_thread(%s)=ME_ERRNO_OPEN\n", address);
			err = ME_ERRNO_OPEN;
			goto ERROR;
		}
	}

	context->fd = clnt_create(strip(context->access_point_addr), RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!context->fd)
	{
		LIBPERROR("Connection not possible: clnt_create(%s)=ME_ERRNO_OPEN\n", address);
		err = ME_ERRNO_OPEN;

		free(context->access_point_addr);
		context->access_point_addr = NULL;

		goto ERROR;
	}

	// Init created instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		RPC_open_res = me_open_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (!RPC_open_res)
	{
		LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_open_res)
	{
		err = *RPC_open_res ;
		LIBPERROR("me_open_proc_1()=%d\n", err);
	}

	if (RPC_open_res)
	{
		free(RPC_open_res);
		RPC_open_res = NULL;
	}

#if defined RPC_USE_SUBCONTEXT
	if (!err)
	{
		pthread_mutex_lock(&(context->rpc_mutex));
			RPC_res = me_query_number_devices_proc_1(NULL, context->fd);
		pthread_mutex_unlock(&(context->rpc_mutex));

		if (!RPC_res)
		{
			LIBPERROR("me_query_number_devices_proc_1()=ME_ERRNO_COMMUNICATION\n");
			err = ME_ERRNO_COMMUNICATION;
		}
		else if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_devices_proc_1()=%d\n", err);
		}
		else if (RPC_res->number > 0)
		{
			context->device_context = calloc(RPC_res->number, sizeof(me_rpc_devcontext_t));
			if (context->device_context)
			{
				context->count = RPC_res->number;
			}
			else
			{
				LIBPERROR("Can not get requestet memory for device_context structure.");
				err = ME_ERRNO_INTERNAL;
			}
		}

		if (RPC_res)
		{
			free(RPC_res);
			RPC_res = NULL;
		}
	}

	if (!err)
	{// Create sub-context
		for (i=0; i<context->count; ++i)
		{
			pthread_mutex_init(&((context->device_context + i)->rpc_mutex), NULL);
			err = Open_DevContext(context->device_context + i, context->access_point_addr, i, iFlags);
			if (err)
				break;
		}
	}
#endif	//RPC_USE_SUBCONTEXT

ERROR:
	return err;
}

void Test_RPC(const char* address)
{
	CLIENT* clnt = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clnt = clnt_create(address, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (clnt)
	{
		pthread_mutex_lock(&condition_mutex);
			LIBPINFO("Test_RPC_thread() successed.\n");
			pthread_cond_signal(&condition_cond);
		pthread_mutex_unlock(&condition_mutex);
		clnt_destroy(clnt);
	}
	else
	{
		LIBPINFO("Test_RPC_thread() failed.\n");
	}
}


#if defined RPC_USE_SUBCONTEXT

static int Open_DevContext(me_rpc_devcontext_t* context, char* address, int device, int iFlags)
{
	int* RPC_open_res ;
	me_query_number_subdevices_res* RPC_res = NULL;
	int i;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	context->fd = clnt_create(address, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!context->fd)
	{
		LIBPERROR("clnt_create()=ME_ERRNO_OPEN\n");
		return ME_ERRNO_OPEN;
	}

	pthread_mutex_init(&context->rpc_mutex, NULL);

	// Init created instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		RPC_open_res = me_open_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (!RPC_open_res)
	{
		LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_open_res)
	{
		err = *RPC_open_res ;
		LIBPERROR("me_open_proc_1()=%d\n", err);
	}

	if (RPC_open_res)
	{
		free(RPC_open_res);
		RPC_open_res = NULL;
	}

	if (!err)
	{
		pthread_mutex_lock(&(context->rpc_mutex));
			RPC_res = me_query_number_subdevices_proc_1(&device, context->fd);
		pthread_mutex_unlock(&(context->rpc_mutex));

		if (!RPC_res)
		{
			LIBPERROR("me_query_number_subdevices_proc_1()=ME_ERRNO_COMMUNICATION\n");
			err = ME_ERRNO_COMMUNICATION;
		}
		else if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_subdevices_proc_1()=%d\n", err);
		}
		else if (RPC_res->number > 0)
		{
			context->subdevice_context = calloc(RPC_res->number, sizeof(me_rpc_subdevcontext_t));
			if (context->subdevice_context)
			{
				context->count = RPC_res->number;
			}
			else
			{
				LIBPERROR("Can not get requestet memory for device_context structure.");
				err = ME_ERRNO_INTERNAL;
			}
		}
	}
	if (!err)
	{
		for (i = 0; i < context->count; i++)
		{
			pthread_mutex_init(&((context->subdevice_context + i)->rpc_mutex), NULL);
			err = Open_SubDevContext(context->subdevice_context + i, address, iFlags);
			if (err)
				break;
		}
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

static int Open_SubDevContext(me_rpc_subdevcontext_t* context, char* address, int iFlags)
{
	int* RPC_open_res ;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);


	context->fd = clnt_create(address, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
	if (!context->fd)
	{
		LIBPERROR("clnt_create()=ME_ERRNO_OPEN\n");
		return ME_ERRNO_OPEN;
	}

	pthread_mutex_init(&context->rpc_mutex, NULL);

	// Init created instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		RPC_open_res = me_open_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (!RPC_open_res)
	{
		LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_open_res)
	{
		err = *RPC_open_res ;
		LIBPERROR("me_open_proc_1()=%d\n", err);
	}

	if (RPC_open_res)
	{
		free(RPC_open_res);
		RPC_open_res = NULL;
	}

	return err;
}
#endif	//RPC_USE_SUBCONTEXT

int Close_RPC(me_rpc_context_t *context, int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	if (iFlags != ME_CLOSE_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		err = ME_ERRNO_INVALID_FLAGS;
	}
	else
	{
		if (!context->fd)
		{// Already closed. This is not always an error. Standard routine call this function for every device on list.

			LIBPDEBUG("Already close!");
		}
		else
		{
			// Destroy context.
			Close_Context(context, iFlags);
		}
	}
	return err;
}

static int Close_Context(me_rpc_context_t* context, int iFlags)
{
#if defined RPC_USE_SUBCONTEXT
	int i;
#endif
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	doDestroyAllThreads_RPC(context);

	if (!context->fd)
		return err;

#if defined RPC_USE_SUBCONTEXT
	for (i=0; i<context->count; i++)
	{
		Close_DevContext(context->device_context+i, iFlags);
	}

	if (context->device_context)
	{
		free (context->device_context);
		context->device_context = NULL;
	}
	context->count = 0;
#endif

	// Close instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		me_close_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (context->fd)
		clnt_destroy(context->fd);
	context->fd = NULL;

	return err;
}

#if defined RPC_USE_SUBCONTEXT
static int Close_DevContext(me_rpc_devcontext_t* context, int iFlags)
{
	int i;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);

	if (!context->fd)
		return err;

	for (i=0; i<context->count; i++)
	{
		Close_SubDevContext(context->subdevice_context+i, iFlags);
	}

	if (context->subdevice_context)
	{
		free (context->subdevice_context);
		context->subdevice_context = NULL;
	}
	context->count = 0;

	// Close instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		me_close_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (context->fd)
		clnt_destroy(context->fd);
	context->fd = NULL;

	return err;
}

static int Close_SubDevContext(me_rpc_subdevcontext_t* context, int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	// Close instance of library.
	pthread_mutex_lock(&(context->rpc_mutex));
		me_close_proc_1(&iFlags, context->fd);
	pthread_mutex_unlock(&(context->rpc_mutex));

	if (context->fd)
		clnt_destroy(context->fd);
	context->fd = NULL;

	return err;
}
#endif


// Locks
int LockDriver_RPC(void* context, int lock, int iFlags)
{
	int* RPC_res = NULL;
	me_lock_driver_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("lock=%d iFlags=0x%x", lock, iFlags);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	params.lock = lock ;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_lock_driver_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_lock_driver_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_lock_driver_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int LockDevice_RPC(void* context, int device, int lock, int iFlags)
{
	int* RPC_res = NULL;
	me_lock_device_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d lock=%d flags=0x%x", device, lock, iFlags);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	params.device = device;
	params.lock = lock ;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_lock_device_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_lock_device_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_lock_device_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int LockSubdevice_RPC(void* context, int device, int subdevice, int lock, int iFlags)
{
	int* RPC_res = NULL;
	me_lock_subdevice_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d lock=%d flags=0x%x", device, lock, iFlags);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	params.device = device;
	params.subdevice = subdevice;
	params.lock = lock ;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_lock_subdevice_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_lock_subdevice_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_lock_subdevice_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

//Query
int  QueryDriverVersion_RPC(void* context, int* version, int iFlags)
{
	me_query_version_main_driver_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(version);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_version_main_driver_proc_1(NULL, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_version_main_driver_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*version = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_version_main_driver_proc_1()=%d\n", err);
		}
		*version = RPC_res->ver;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryDriverName_RPC(void* context, char* name, int count, int iFlags)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	strncpy(name, "MEiDS RPC", count);
	name[count - 1] = '\0';

	return ME_ERRNO_SUCCESS;
}


int  QuerySubdriverVersion_RPC(void* context, int device, int* version, int iFlags)
{
	me_query_version_device_driver_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(version);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_version_device_driver_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_version_device_driver_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*version = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_version_device_driver_proc_1()=%d\n", err);
		}
		*version = RPC_res->ver;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QuerySubdriverName_RPC(void* context, int device, char* name, int count, int iFlags)
{
	me_query_name_device_driver_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_name_device_driver_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_name_device_driver_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*name = '\0';
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_name_device_driver_proc_1()=%d\n", err);
		}
		strncpy(name, RPC_res->name, count);
		name[count - 1] = '\0';
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_query_name_device_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  QueryDeviceName_RPC(void* context, int device, char* name, int count, int iFlags)
{
	me_query_name_device_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(name);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_name_device_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_name_device_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*name = '\0';
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_name_device_proc_1()=%d\n", err);
		}
		strncpy(name, RPC_res->name, count);
		name[count - 1] = '\0';
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_query_name_device_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryDeviceDescription_RPC(void* context, int device, char* description, int count, int iFlags)
{
	me_query_description_device_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(description);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_description_device_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_description_device_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*description = '\0';
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_description_device_proc_1()=%d\n", err);
		}
		strncpy(description, RPC_res->description, count);
		description[count - 1] = '\0';
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_query_description_device_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryDevicesNumber_RPC(void* context, int* no_devices, int iFlags)
{
	me_query_number_devices_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_devices);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_number_devices_proc_1(NULL, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_number_devices_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*no_devices = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_devices_proc_1()=%d\n", err);
		}
		*no_devices = RPC_res->number;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryDeviceInfo_RPC(void* context, int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags)
{
	me_query_info_device_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(vendor_id);
	CHECK_POINTER(device_id);
	CHECK_POINTER(serial_no);
	CHECK_POINTER(bus_type);
	CHECK_POINTER(bus_no);
	CHECK_POINTER(dev_no);
	CHECK_POINTER(func_no);
	CHECK_POINTER(plugged);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_info_device_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_info_device_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*vendor_id = 0;
		*device_id = 0;
		*serial_no = 0;
		*bus_type = 0;
		*bus_no = 0;
		*dev_no = 0;
		*func_no = 0;
		*plugged = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_info_device_proc_1()=%d\n", err);
		}
		*vendor_id = RPC_res->vendor_id;
		*device_id = RPC_res->device_id;
		*serial_no = RPC_res->serial_no;
		*bus_type = RPC_res->bus_type | 0x100;
		*bus_no = RPC_res->bus_no;
		*dev_no = RPC_res->dev_no;
		*func_no = RPC_res->func_no;
		*plugged = RPC_res->plugged;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  QuerySubdevicesNumber_RPC(void* context, int device, int* no_subdevices, int iFlags)
{
	me_query_number_subdevices_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_subdevices);

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_number_subdevices_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_number_subdevices_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*no_subdevices = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_subdevices_proc_1()=%d\n", err);
		}
		*no_subdevices = RPC_res->number;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int QuerySubdevicesNumberByType_RPC(void* context, int device, int type, int subtype, int* no_subdevices, int iFlags)
{
// 	me_query_number_subdevices_res* RPC_res = NULL;
// 	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

// 	err = checkRPC(rpc_context);
// 	if (err)
// 		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_subdevices);
/*
	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_number_subdevices_proc_1(&device, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_number_subdevices_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*no_subdevices = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_subdevices_proc_1()=%d\n", err);
		}
		*no_subdevices = RPC_res->number;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

*/
/// @todo Implement this function for RPC
	err = ME_ERRNO_NOT_SUPPORTED;
	return err;
}

int  QuerySubdeviceType_RPC(void* context, int device, int subdevice, int* type, int* subtype, int iFlags)
{
	me_query_subdevice_type_params params;
	me_query_subdevice_type_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(type);
	CHECK_POINTER(subtype);

	params.device = device;
	params.subdevice = subdevice;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_subdevice_type_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_subdevice_type_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*type = 0;
		*subtype = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_subdevice_type_proc_1()=%d\n", err);
		}
		*type = RPC_res->type;
		*subtype = RPC_res->subtype;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QuerySubdeviceByType_RPC(void* context, int device, int subdevice, int type, int subtype, int* result, int iFlags)
{
	me_query_subdevice_by_type_params params;
	me_query_subdevice_by_type_res* RPC_res = NULL;;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(result);

	params.device = device;
	params.start_subdevice = subdevice;
	params.type = type;
	params.subtype = subtype;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_subdevice_by_type_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_subdevice_by_type_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*result = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_subdevice_by_type_proc_1()=%d\n", err);
		}
		*result = RPC_res->subdevice;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QuerySubdeviceCaps_RPC(void* context, int device, int subdevice, int* caps, int iFlags)
{
	me_query_subdevice_caps_params params;
	me_query_subdevice_caps_res* RPC_res = NULL;;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(caps);

	params.device = device;
	params.subdevice = subdevice;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_subdevice_caps_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_subdevice_caps_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*caps = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_subdevice_caps_proc_1()=%d\n", err);
		}
		*caps = RPC_res->caps;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QuerySubdeviceCapsArgs_RPC(void* context, int device, int subdevice, int cap, int* args, int count, int iFlags)
{
	me_query_subdevice_caps_args_params params;
	me_query_subdevice_caps_args_res* RPC_res = NULL;;
	int i;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(args);

	params.device = device;
	params.subdevice = subdevice;
	params.cap = cap;
	params.count = count;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_subdevice_caps_args_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_subdevice_caps_args_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*args = 0;
		for (i=0; i<count; i++)
		{
			args[i] = 0;
		}
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_subdevice_caps_args_proc_1()=%d\n", err);
		}

		// Copy only if space is available and returned values are.
		for (i=0; (i < count) && (i < RPC_res->args.args_len); i++)
		{
			args[i] = RPC_res->args.args_val[i];
		}
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_query_subdevice_caps_args_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QuerySubdeviceTimer_RPC(void* context, int device, int subdevice, int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags)
{
	me_io_stream_time_to_ticks_res* RPC_res = NULL;
	me_io_stream_time_to_ticks_params params;

	me_io_stream_frequency_to_ticks_res* RPC_res_base = NULL;
	me_io_stream_frequency_to_ticks_params params_base;

	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags & ~ME_MEPHISTO_SCOPE_OSCILLOSCOPE_FLAG)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(min_ticks_high);
	CHECK_POINTER(min_ticks_low);
	CHECK_POINTER(max_ticks_high);
	CHECK_POINTER(max_ticks_low);
	CHECK_POINTER(base);

	*base = 0;
	*min_ticks_low = 0;
	*min_ticks_high = 0;
	*max_ticks_low = 0;
	*max_ticks_high = 0;

	params.device = device;
	params.subdevice = subdevice;
	params.timer = timer;
	params.flags = iFlags;
	params.time = -HUGE_VAL;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_time_to_ticks_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_time_to_ticks_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_time_to_ticks_proc_1()=%d\n", err);
		}
		*min_ticks_low = RPC_res->ticks_low;
		*min_ticks_high = RPC_res->ticks_high;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	if (!err)
	{
		params.device = device;
		params.subdevice = subdevice;
		params.timer = timer;
		params.flags = iFlags;
		params.time = HUGE_VAL;

		pthread_mutex_lock(&rpc_context->rpc_mutex);
			RPC_res = me_io_stream_time_to_ticks_proc_1(&params, rpc_context->fd);
		pthread_mutex_unlock(&rpc_context->rpc_mutex);

		if (!RPC_res)
		{
			LIBPERROR("me_io_stream_time_to_ticks_proc_1()=ME_ERRNO_COMMUNICATION\n");
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			if (RPC_res->error)
			{
				err = RPC_res->error;
				LIBPERROR("me_io_stream_time_to_ticks_proc_1()=%d\n", err);
			}
			*max_ticks_low = RPC_res->ticks_low;
			*max_ticks_high = RPC_res->ticks_high;
		}

		if (RPC_res)
		{
			free(RPC_res);
			RPC_res = NULL;
		}
	}

	if (!err)
	{
		params_base.device = device;
		params_base.subdevice = subdevice;
		params_base.timer = timer;
		params_base.flags = iFlags;
		params_base.frequency = 1;

		pthread_mutex_lock(&rpc_context->rpc_mutex);
			RPC_res_base = me_io_stream_frequency_to_ticks_proc_1(&params_base, rpc_context->fd);
		pthread_mutex_unlock(&rpc_context->rpc_mutex);

		if (!RPC_res_base)
		{
			LIBPERROR("me_io_stream_frequency_to_ticks_proc_1()=ME_ERRNO_COMMUNICATION\n");
			err = ME_ERRNO_COMMUNICATION;
		}
		else
		{
			if (RPC_res_base->error)
			{
				err = RPC_res_base->error;
				LIBPERROR("me_io_stream_frequency_to_ticks_proc_1()=%d\n", err);
			}
			*base = RPC_res_base->ticks_low;
		}

		if (RPC_res_base)
		{
			free(RPC_res_base);
			RPC_res_base = NULL;
		}
	}

	return err;
}


int  QueryChannelsNumber_RPC(void* context, int device, int subdevice, unsigned int* number, int iFlags)
{
	me_query_number_channels_params params;
	me_query_number_channels_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(number);

	params.device = device;
	params.subdevice = subdevice;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_number_channels_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_number_channels_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*number = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_channels_proc_1()=%d\n", err);
		}
		*number = RPC_res->number;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  QueryRangesNumber_RPC(void* context, int device, int subdevice, int unit, int* no_ranges, int iFlags)
{
	me_query_number_ranges_params params;
	me_query_number_ranges_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(no_ranges);

	params.device = device;
	params.subdevice = subdevice;
	params.unit = unit;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_number_ranges_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_number_ranges_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*no_ranges = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_number_ranges_proc_1()=%d\n", err);
		}
		*no_ranges = RPC_res->number;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryRangeInfo_RPC(void* context, int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data, int iFlags)
{
	me_query_range_info_params params;
	me_query_range_info_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(unit);
	CHECK_POINTER(min);
	CHECK_POINTER(max);
	CHECK_POINTER(max_data);

	params.device = device;
	params.subdevice = subdevice;
	params.range = range;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_range_info_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_range_info_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*unit = 0;
		*min = 0;
		*max = 0;
		*max_data = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_range_info_proc_1()=%d\n", err);
		}
		*unit = RPC_res->unit;
		*min = RPC_res->min;
		*max = RPC_res->max;
		*max_data = RPC_res->max_data;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  QueryRangeByMinMax_RPC(void* context, int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags)
{
	me_query_range_by_min_max_params params;
	me_query_range_by_min_max_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	if (iFlags != ME_QUERY_NO_FLAGS)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	CHECK_POINTER(context);
	CHECK_POINTER(range);
	CHECK_POINTER(max_data);
	CHECK_POINTER(max);
	CHECK_POINTER(min);

	params.device = device;
	params.subdevice = subdevice;
	params.unit = unit;
	params.min = *min;
	params.max = *max;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_query_range_by_min_max_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_query_range_by_min_max_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*min = 0;
		*max = 0;
		*max_data = 0;
		*range = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_query_range_by_min_max_proc_1()=%d\n", err);
		}
		*min = RPC_res->min;
		*max = RPC_res->max;
		*max_data = RPC_res->max_data;
		*range = RPC_res->range;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


//Input/Output
int  IrqStart_RPC(void* context, int device, int subdevice, int channel, int source, int edge, int arg, int iFlags)
{
	me_io_irq_start_params params;
	int* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);

	params.device = device;
	params.subdevice = subdevice;
	params.channel = channel;
	params.irq_source = source;
	params.irq_edge = edge;
	params.irq_arg = arg;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_irq_start_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_irq_start_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_irq_start_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  IrqWait_RPC(void* context, int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags)
{
	me_io_irq_wait_params params;
	me_io_irq_wait_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(value);
	CHECK_POINTER(count);

	params.device = device;
	params.subdevice = subdevice;
	params.channel = channel;
	params.time_out = timeout;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_irq_wait_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_irq_wait_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*value = 0;
		*count = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_irq_wait_proc_1()=%d\n", err);
		}
		*value = RPC_res->value;
		*count = RPC_res->irq_count;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  IrqStop_RPC(void* context, int device, int subdevice, int channel, int iFlags)
{
	me_io_irq_stop_params params;
	int* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);

	params.device = device;
	params.subdevice = subdevice;
	params.channel = channel;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_irq_stop_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_irq_stop_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_irq_stop_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  IrqTest_RPC(void* context, int device, int subdevice, int channel, int iFlags)
{
	me_io_irq_wait_params params;
	me_io_irq_wait_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	params.device = device;
	params.subdevice = subdevice;
	params.channel = channel;
	params.time_out = 1;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_irq_wait_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_irq_wait_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else
	{
		if ((RPC_res->error) && (RPC_res->error != ME_ERRNO_TIMEOUT))
		{
			err = RPC_res->error;
			LIBPERROR("me_io_irq_wait_proc_1()=%d\n", err);
		}
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int IrqSetCallback_RPC(void* context, int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d subdevice=%d irq_fn=%p iFlags=0x%x", device, subdevice, irq_fn, iFlags);

	if (irq_fn)
	{	// create
		err = IrqTest_RPC(context, device, subdevice, 0, iFlags);
		if (!err)
		{
			err = doCreateThread_RPC(context, device, subdevice, irqThread_RPC, irq_fn, irq_context, iFlags);
		}
	}
	else
	{	// cancel
		err = doDestroyThread_RPC(context, device, subdevice);
	}

	return err;
}

int  ResetDevice_RPC(void* context, int device, int iFlags)
{
	me_io_reset_device_params params;
	int* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);

	doDestroyThreads_RPC(context, device);

	params.device = device;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_reset_device_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_reset_device_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_reset_device_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  ResetSubdevice_RPC(void* context, int device, int subdevice, int iFlags)
{
	me_io_reset_subdevice_params params;
	int* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);

	doDestroyThread_RPC(context, device, subdevice);

	params.device = device;
	params.subdevice = subdevice;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_reset_subdevice_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_reset_subdevice_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_reset_subdevice_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  SingleConfig_RPC(void* context, int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge, int iFlags)
{
	me_io_single_config_params params;
	int* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);

	params.device = device;
	params.subdevice = subdevice;
	params.channel = channel;
	params.single_config = config;
	params.ref = reference;
	params.trig_chain = synchro;
	params.trig_type = trigger;
	params.trig_edge = edge;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_single_config_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_single_config_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_single_config_proc_1()=%d\n", err);
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  Single_RPC(void* context, int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags)
{
	meIOSingle_t list;
	int err = ME_ERRNO_SUCCESS;

	list.iDevice = device;
	list.iSubdevice = subdevice;
	list.iChannel = channel;
	list.iDir = direction;
	list.iValue =  *value;
	list.iTimeOut = timeout;
	list.iFlags = iFlags;

	err = SingleList_RPC(context, &list, 1, ME_VALUE_NOT_USED);

	*value = list.iValue;

	return err;
}

int  SingleList_RPC(void* context, meIOSingle_t* list, int count, int iFlags)
{
/** @note This is an external call.
* It must obey this rules:
* 1: All devices on list have to be from one SynapseLAN.
* 2: Maximum size of whole argument can be bigger than 4KB (maximum list size is 127 entries).
*
*  This is not check here!
*/
	me_io_single_params params;
	me_io_single_res* RPC_res = NULL;
	int i;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	params.single_list.single_list_val = calloc(count, sizeof(me_io_single_entry_params));
	if (!params.single_list.single_list_val)
	{
		LIBPCRITICALERROR("Cannot get memory for single list.\n");
		return ME_ERRNO_INTERNAL;
	}

	params.single_list.single_list_len = count;
	params.flags = iFlags;

	for (i = 0; i < count; i++)
	{
		params.single_list.single_list_val[i].device = list[i].iDevice ;
		params.single_list.single_list_val[i].subdevice = list[i].iSubdevice;
		params.single_list.single_list_val[i].channel = list[i].iChannel;
		params.single_list.single_list_val[i].dir = list[i].iDir;
		params.single_list.single_list_val[i].value = list[i].iValue;
		params.single_list.single_list_val[i].time_out = list[i].iTimeOut;
		params.single_list.single_list_val[i].flags = list[i].iFlags;
	}

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_single_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPCRITICALERROR("me_io_single_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		for (i = 0; i < count; i++)
		{
			list[i].iValue = 0;
			list[i].iErrno = ME_ERRNO_COMMUNICATION;
		}
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_single_proc_1()=%d\n", err);
		}

		if (RPC_res->single_list.single_list_len != count)
		{
			err = ME_ERRNO_INTERNAL;
			LIBPCRITICALERROR("Wrong size of return array in me_io_single_proc_1() %d=>%d\n", count, RPC_res->single_list.single_list_len);
		}
		else
		{
			for (i = 0; i < count; i++)
			{
				list[i].iValue = RPC_res->single_list.single_list_val[i].value;
				list[i].iErrno = RPC_res->single_list.single_list_val[i].error;
			}
		}
	}

	if (params.single_list.single_list_val)
	{
		free(params.single_list.single_list_val);
		params.single_list.single_list_val = NULL;
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_io_single_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  StreamConfigure_RPC(void* context, int device, int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags)
{
	meIOStreamTrigger_t universal_triggers;
	meIOStreamConfig_t*	universal_config = NULL;
	int flags;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	CHECK_POINTER(context);
	CHECK_POINTER(list);
	CHECK_POINTER(trigger);

	universal_config = calloc(count, sizeof(meIOStreamConfig_t));
	if (!universal_config)
	{
		LIBPERROR("Can not get requestet memory for stream_config list.\n");
		return ME_ERRNO_INTERNAL;
	}

	err = me_translate_triggers_to_unversal(trigger, &universal_triggers);
	if (!err)
	{
		err = me_translate_config_to_universal(list, count, iFlags, universal_config, &flags);
	}
	if (!err)
	{
		err = StreamConfig_RPC(context, device, subdevice, universal_config, count, &universal_triggers, threshold, flags);
	}

	free (universal_config);

	return err;
}

int  StreamConfig_RPC(void* context, int device,int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags)
{
	me_io_stream_config_params params;
	int* RPC_res = NULL;
	int i;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(list);
	CHECK_POINTER(trigger);

	params.config_list.config_list_len = count;
	params.config_list.config_list_val = calloc(count, sizeof(me_io_stream_config_entry_params));
	if (!params.config_list.config_list_val)
	{
		LIBPERROR("Cannot get memory for stream list.");
		return ME_ERRNO_INTERNAL;
	}

	params.device = device;
	params.subdevice = subdevice;
	params.fifo_irq_threshold = threshold;
	params.flags = iFlags;

	for (i = 0; i < count; i++)
	{
		params.config_list.config_list_val[i].channel = list[i].iChannel;
		params.config_list.config_list_val[i].stream_config = list[i].iStreamConfig;
		params.config_list.config_list_val[i].ref = list[i].iRef;
		params.config_list.config_list_val[i].flags = list[i].iFlags;
	}

	params.trigger.acq_start_trig_type = trigger->iAcqStartTrigType;
	params.trigger.acq_start_trig_edge = trigger->iAcqStartTrigEdge;
	params.trigger.acq_start_trig_chain = trigger->iAcqStartTrigChan;
	params.trigger.acq_start_ticks_low = trigger->iAcqStartTicksLow;
	params.trigger.acq_start_ticks_high = trigger->iAcqStartTicksHigh;
	params.trigger.acq_start_args.acq_start_args_len = 10;
	params.trigger.acq_start_args.acq_start_args_val = trigger->iAcqStartArgs;

	params.trigger.scan_start_trig_type = trigger->iScanStartTrigType;
	params.trigger.scan_start_ticks_low = trigger->iScanStartTicksLow;
	params.trigger.scan_start_ticks_high = trigger->iScanStartTicksHigh;
	params.trigger.scan_start_args.scan_start_args_len = 10;
	params.trigger.scan_start_args.scan_start_args_val = trigger->iScanStartArgs;

	params.trigger.conv_start_trig_type = trigger->iConvStartTrigType;
	params.trigger.conv_start_ticks_low = trigger->iConvStartTicksLow;
	params.trigger.conv_start_ticks_high = trigger->iConvStartTicksHigh;
	params.trigger.conv_start_args.conv_start_args_len = 10;
	params.trigger.conv_start_args.conv_start_args_val = trigger->iConvStartArgs;

	params.trigger.scan_stop_trig_type = trigger->iScanStopTrigType;
	params.trigger.scan_stop_count = trigger->iScanStopCount;
	params.trigger.scan_stop_args.scan_stop_args_len = 10;
	params.trigger.scan_stop_args.scan_stop_args_val = trigger->iScanStopArgs;

	params.trigger.acq_stop_trig_type = trigger->iAcqStopTrigType;
	params.trigger.acq_stop_count = trigger->iAcqStopCount;
	params.trigger.acq_stop_args.acq_stop_args_len = 10;
	params.trigger.acq_stop_args.acq_stop_args_val = trigger->iAcqStopArgs;

	params.trigger.flags = trigger->iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_config_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_config_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
	}
	else if (*RPC_res)
	{
		err = *RPC_res ;
		LIBPERROR("me_io_stream_config_proc_1()=%d\n", err);
	}


	if (params.config_list.config_list_val)
	{
		free(params.config_list.config_list_val);
		params.config_list.config_list_val = NULL;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamStart_RPC(void* context, int device, int subdevice, int mode, int timeout, int iFlags)
{
	meIOStreamStart_t list;
	int err = ME_ERRNO_SUCCESS;

	list.iDevice = device;
	list.iSubdevice = subdevice;
	list.iStartMode = mode;
	list.iTimeOut = timeout;
	list.iFlags = iFlags;

	err = StreamStartList_RPC(context, &list, 1, ME_VALUE_NOT_USED);

	return err;
}

int  StreamStartList_RPC(void* context, meIOStreamStart_t* list, int count, int iFlags)
{
/** @note This is an external call.
* It must obey this rules:
* 1: All devices on list have to be from one SynapseLAN.
* 2: Maximum size of whole argument can be bigger than 4KB (maximum list size is 127 entries).
*
*  This is not check here!
*/
	me_io_stream_start_params params;
	me_io_stream_start_res* RPC_res = NULL;
	int i;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	params.start_list.start_list_val = calloc(count, sizeof(me_io_stream_start_entry_params));
	if (!params.start_list.start_list_val)
	{
		LIBPERROR("Cannot get memory for stream start list.");
		return ME_ERRNO_INTERNAL;
	}

	params.start_list.start_list_len = count;
	params.flags = iFlags;

	for (i = 0; i < count; i++)
	{
		params.start_list.start_list_val[i].device = list[i].iDevice ;
		params.start_list.start_list_val[i].subdevice = list[i].iSubdevice;
		params.start_list.start_list_val[i].start_mode = list[i].iStartMode;
		params.start_list.start_list_val[i].time_out = list[i].iTimeOut;
		params.start_list.start_list_val[i].flags = list[i].iFlags;
	}

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_start_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_start_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		for (i = 0; i < count; i++)
		{
			list[i].iErrno = ME_ERRNO_COMMUNICATION;
		}
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_start_proc_1()=%d\n", err);
		}

		for (i = 0; i < count; i++)
		{
			list[i].iErrno = RPC_res->start_list.start_list_val[i];
		}
	}

	if (params.start_list.start_list_val)
	{
		free(params.start_list.start_list_val);
		params.start_list.start_list_val = NULL;
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_io_stream_start_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamStop_RPC(void* context, int device, int subdevice, int mode, int timeout, int iFlags)
{
/// @note In this mode "timeout" is ignored.
	meIOStreamStop_t list;
	int err = ME_ERRNO_SUCCESS;


	list.iDevice = device;
	list.iSubdevice = subdevice;
	list.iStopMode = mode;
	list.iFlags = iFlags;

	err = StreamStopList_RPC(context, &list, 1, ME_VALUE_NOT_USED);

	return err;
}

int  StreamStopList_RPC(void* context, meIOStreamStop_t* list, int count, int iFlags)
{
/** @note This is an external call.
* It must obey this rules:
* 1: All devices on list have to be from one SynapseLAN.
* 2: Maximum size of whole argument can be bigger than 4KB (maximum list size is 127 entries).
*
*  This is not check here!
*/
	me_io_stream_stop_params params;
	me_io_stream_stop_res* RPC_res = NULL;
	int i;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(list);

	params.stop_list.stop_list_val = calloc(count, sizeof(me_io_stream_stop_entry_params));
	if (!params.stop_list.stop_list_val)
	{
		LIBPERROR("Cannot get memory for stream stop list.");
		return ME_ERRNO_INTERNAL;
	}

	params.stop_list.stop_list_len = count;
	params.flags = iFlags;

	for (i = 0; i < count; i++)
	{
		params.stop_list.stop_list_val[i].device = list[i].iDevice ;
		params.stop_list.stop_list_val[i].subdevice = list[i].iSubdevice;
		params.stop_list.stop_list_val[i].stop_mode = list[i].iStopMode;
		params.stop_list.stop_list_val[i].flags = list[i].iFlags;
	}

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_stop_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_stop_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		for (i = 0; i < count; i++)
		{
			list[i].iErrno = ME_ERRNO_COMMUNICATION;
		}
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_stop_proc_1()=%d\n", err);
		}

		for (i = 0; i < count; i++)
		{
			list[i].iErrno = RPC_res->stop_list.stop_list_val[i];
		}
	}

	if (params.stop_list.stop_list_val)
	{
		free(params.stop_list.stop_list_val);
		params.stop_list.stop_list_val = NULL;
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_io_stream_stop_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamNewValues_RPC(void* context, int device, int subdevice, int timeout, int* count, int iFlags)
{
	me_io_stream_new_values_params params;
	me_io_stream_new_values_res* RPC_res = NULL;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(count);

	params.device = device;
	params.subdevice = subdevice;
	params.time_out = timeout;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_new_values_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_new_values_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*count = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_new_values_proc_1()=%d\n", err);
		}
		*count = RPC_res->count;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamRead_RPC(void* context, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	me_io_stream_read_res* RPC_res = NULL;
	me_io_stream_read_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(values);
	CHECK_POINTER(count);

	params.device = device;
	params.subdevice = subdevice;
	params.read_mode = mode;
	params.count = *count;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_read_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_read_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*count = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_read_proc_1()=%d\n", err);
		}
		if (*count > RPC_res->values.values_len)
		{
			*count = RPC_res->values.values_len;
		}
		memcpy(values, RPC_res->values.values_val, *count * sizeof(int));
	}

	if (RPC_res)
	{
		xdr_free((xdrproc_t) xdr_me_io_stream_read_res, (char *)RPC_res);
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamWrite_RPC(void* context, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags)
{
	me_io_stream_write_res* RPC_res = NULL;
	me_io_stream_write_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(values);
	CHECK_POINTER(count);

	params.device = device;
	params.subdevice = subdevice;
	params.write_mode = mode;
	params.values.values_val = values;
	params.values.values_len = *count;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_write_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_write_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*count = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_write_proc_1()=%d\n", err);
		}
		*count = RPC_res->count;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int StreamSetCallbacks_RPC(void* context,
							int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags)
{
	int err;
	int err_ret = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);
	LIBPDEBUG("device=%d dubdevice=%d start=%p start_context=%p new_values=%p new_value_context=%p end=%p end_context=%p iFlags=0x%x",
			device, subdevice, start, start_context, new_values, new_value_context, end, end_context, iFlags);

	if (iFlags)
	{
		LIBPERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (new_values)
	{	// create
		err = doCreateThread_RPC(context, device, subdevice, streamNewValuesThread_RPC, new_values, new_value_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (start)
	{	// create
		err = doCreateThread_RPC(context, device, subdevice, streamStartThread_RPC, start, start_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (end)
	{	// create
		err = doCreateThread_RPC(context, device, subdevice, streamStopThread_RPC, end, end_context, ME_NO_FLAGS);
		if (!err_ret)
			err_ret = err;
	}

	if (!new_values && !start && !end)
	{	// cancel
		err = doDestroyThread_RPC(context, device, subdevice);
	}

	return err_ret;
}

int  StreamStatus_RPC(void* context, int device, int subdevice, int wait, int* status, int* count, int iFlags)
{
	me_io_stream_status_res* RPC_res = NULL;
	me_io_stream_status_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(status);
	CHECK_POINTER(count);

	params.device = device;
	params.subdevice = subdevice;
	params.wait = wait;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_status_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_status_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*count = 0;
		*status = ME_STATUS_INVALID;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_status_proc_1()=%d\n", err);
		}
		*count = RPC_res->count;
		*status = RPC_res->status;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamTimeToTicks_RPC(void* context, int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags)
{
	me_io_stream_time_to_ticks_res* RPC_res = NULL;
	me_io_stream_time_to_ticks_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(ticks_high);
	CHECK_POINTER(ticks_low);
	CHECK_POINTER(stream_time);

	params.device = device;
	params.subdevice = subdevice;
	params.timer = timer;
	params.time = *stream_time;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_time_to_ticks_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_time_to_ticks_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*stream_time = 0;
		*ticks_low = 0;
		*ticks_high = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_time_to_ticks_proc_1()=%d\n", err);
		}
		*stream_time = RPC_res->time;
		*ticks_low = RPC_res->ticks_low;
		*ticks_high = RPC_res->ticks_high;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}

int  StreamFrequencyToTicks_RPC(void* context, int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags)
{
	me_io_stream_frequency_to_ticks_res* RPC_res = NULL;
	me_io_stream_frequency_to_ticks_params params;
	me_rpc_context_t* rpc_context = (me_rpc_context_t *)context;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	err = checkRPC(rpc_context);
	if (err)
		return err;

	CHECK_POINTER(context);
	CHECK_POINTER(ticks_high);
	CHECK_POINTER(ticks_low);
	CHECK_POINTER(frequency);

	params.device = device;
	params.subdevice = subdevice;
	params.timer = timer;
	params.frequency = *frequency;
	params.flags = iFlags;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		RPC_res = me_io_stream_frequency_to_ticks_proc_1(&params, rpc_context->fd);
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	if (!RPC_res)
	{
		LIBPERROR("me_io_stream_frequency_to_ticks_proc_1()=ME_ERRNO_COMMUNICATION\n");
		err = ME_ERRNO_COMMUNICATION;
		*frequency = 0;
		*ticks_low = 0;
		*ticks_high = 0;
	}
	else
	{
		if (RPC_res->error)
		{
			err = RPC_res->error;
			LIBPERROR("me_io_stream_frequency_to_ticks_proc_1()=%d\n", err);
		}
		*frequency = RPC_res->frequency;
		*ticks_low = RPC_res->ticks_low;
		*ticks_high = RPC_res->ticks_high;
	}

	if (RPC_res)
	{
		free(RPC_res);
		RPC_res = NULL;
	}

	return err;
}


int  ParametersSet_RPC(void* context, int device, me_extra_param_set_t* paramset, int flags)
{
	return ME_ERRNO_NOT_SUPPORTED;
}


int SetOffset_RPC(void* context, int device, int subdevice, int channel, int range, double* offset, int iFlags)
{
	return ME_ERRNO_NOT_SUPPORTED;
}

// RPC threads
static int doCreateThread_RPC(me_rpc_context_t* local_context, int device, int subdevice, void* fnThread, void* fnCB, void* contextCB, int iFlags)
{
	threadContext_t* threadArgs;
	threadsList_t* newThread;
	int err = ME_ERRNO_SUCCESS;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	threadArgs = (threadContext_t *)calloc(1, sizeof(threadContext_t));
	if (!threadArgs)
	{
		LIBPERROR("Can not get requestet memory for new thread's arguments.\n");
		return -ENOMEM;
	}

	newThread = (threadsList_t *)calloc(1, sizeof(threadsList_t));
	if (!newThread)
	{
		free (threadArgs);
		LIBPERROR("Can not get requestet memory for new thread.\n");
		return -ENOMEM;
	}

	pthread_mutex_lock(&local_context->callbackContextMutex);
		newThread->device = device;
		newThread->subdevice = subdevice;
		newThread->context = local_context;
		newThread->cancel = 0;

		threadArgs->instance = newThread;
		threadArgs->fnCB = fnCB;
		threadArgs->contextCB = contextCB;
		threadArgs->flags = iFlags;

		if (pthread_create(&newThread->threadID, NULL, fnThread, threadArgs))
		{
			LIBPERROR("device[%d,%d]=>> CREATING THREAD FAILED\n", device, subdevice);
			err = ME_ERRNO_START_THREAD;
			free (newThread);
			free (threadArgs);
		}
		else
		{
			LIBPERROR("device[%d,%d]=>> THREAD CREATED (ID:%lld)\n", device, subdevice, (long long)newThread->threadID);
			pthread_detach(newThread->threadID);

			newThread->next = local_context->activeThreads;
			local_context->activeThreads = newThread;
		}
	pthread_mutex_unlock(&local_context->callbackContextMutex);

	return err;
}

static int doDestroyAllThreads_RPC(me_rpc_context_t* local_context)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	return doDestroyThreads_RPC(local_context, -1);
}

static int doDestroyThreads_RPC(me_rpc_context_t* local_context, int device)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	return doDestroyThread_RPC(local_context, device, -1);
}

static int doDestroyThread_RPC(me_rpc_context_t* local_context, int device, int subdevice)
{
	threadsList_t**	activeThread = &local_context->activeThreads;
	threadsList_t*	deleteThread;
	pthread_t		selfID;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	selfID = pthread_self();
	pthread_mutex_lock(&local_context->callbackContextMutex);
		while (*activeThread)
		{
			if ((device < 0) || (((*activeThread)->device == device) && ((subdevice < 0) || ((*activeThread)->subdevice == subdevice))))
			{
				deleteThread = (*activeThread);

				*activeThread = deleteThread->next;
				if (pthread_equal(deleteThread->threadID, selfID))
				{	// I'm killing yourself. Lord, forgive me, please.
					deleteThread->cancel = 1;
					LIBPDEBUG("killing yourself selfID=%ld\n", selfID);
				}
				else
				{
					deleteThread->cancel = 2;
					pthread_cancel(deleteThread->threadID);
					LIBPDEBUG("killing thread=%ld selfID=%ld\n", deleteThread->threadID, selfID);
					free (deleteThread);
				}
			}
			else
			{
				activeThread = &((*activeThread)->next);
			}
		}
	pthread_mutex_unlock(&local_context->callbackContextMutex);

	return ME_ERRNO_SUCCESS;
}

static void* irqThread_RPC(void* arg)
{
	me_rpc_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;
	int* RPC_open_res;
	int iFlags = 0;

	int irq_count = 0;
	int value = 0;

	int err = ME_ERRNO_SUCCESS;

	CLIENT* clnt = NULL;
	me_io_irq_wait_params params;
	me_io_irq_wait_res* RPC_res = NULL;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.irqCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		if (!clnt)
		{
			clnt = clnt_create(local_context->access_point_addr, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
			if (clnt)
			{
				RPC_open_res = me_open_proc_1(&iFlags, clnt);
				if (!RPC_open_res)
				{
					LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
					err = ME_ERRNO_COMMUNICATION;
					break;
				}
				else if (*RPC_open_res)
				{
					err = *RPC_open_res ;
					LIBPERROR("me_open_proc_1()=%d\n", err);
					break;
				}

				if (RPC_open_res)
				{
					free(RPC_open_res);
					RPC_open_res = NULL;
				}
			}
		}

		if (clnt)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

			params.device = context->device;
			params.subdevice = context->subdevice;
			params.channel = 0;
			params.time_out = 100;
			params.flags = threadArgs.flags;

			RPC_res = me_io_irq_wait_proc_1(&params, clnt);
			if (!RPC_res)
			{
				LIBPERROR("me_io_irq_wait_proc_1()=ME_ERRNO_COMMUNICATION\n");
				err = ME_ERRNO_COMMUNICATION;
				value = 0;
				irq_count = 0;

				clnt_destroy(clnt);
				clnt = NULL;
			}
			else
			{
				if (RPC_res->error)
				{
					err = RPC_res->error;
					LIBPERROR("me_io_irq_wait_proc_1()=%d\n", err);
				}
				else
				{
					err = ME_ERRNO_SUCCESS;
				}
				value = RPC_res->value;
				irq_count = RPC_res->irq_count;

				free(RPC_res);
				RPC_res = NULL;
			}

			pthread_testcancel();
			if (context->cancel)
				break;

			if (err == ME_ERRNO_TIMEOUT)
			{// Give chance to destroyer.
				continue;
			}
		}
		else
		{
			err = ME_ERRNO_COMMUNICATION;
		}

		/// Interrupt or STOP/RESET -> call callback function.
		if (threadArgs.irqCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
			if (threadArgs.irqCB(context->device, context->subdevice, 0, irq_count, value, threadArgs.contextCB, err))
			{
				if (context->cancel)
					break;

				if (!err)
				{/// Interrupt ONLY.
					IrqStop_RPC(local_context, context->device, context->subdevice, 0, ME_IO_IRQ_STOP_NO_FLAGS);
				}
			}
			pthread_mutex_unlock(&local_context->callbackContextMutex);

			if (context->cancel)
				break;
		} // while()
	}

	if (clnt)
	{
		clnt_destroy(clnt);
	}

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);

	return NULL;
}

static void* streamStartThread_RPC(void* arg)
{
	me_rpc_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;
	int* RPC_open_res ;
	int iFlags = 0;

	int streamStatus;
	int value = 0;

	static int old_err = ME_ERRNO_INVALID_ERROR_NUMBER;
	int err;

	CLIENT* clnt = NULL;
	me_io_stream_status_res* status_res = NULL;
	me_io_stream_status_params status_params;

	me_io_stream_stop_params stop_params;
	me_io_stream_stop_res* stop_res = NULL;
	me_io_stream_stop_entry_params stop_entry;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		if (!clnt)
		{
			clnt = clnt_create(local_context->access_point_addr, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
			if (clnt)
			{
				RPC_open_res = me_open_proc_1(&iFlags, clnt);
				if (!RPC_open_res)
				{
					LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
					err = ME_ERRNO_COMMUNICATION;
					break;
				}
				else if (*RPC_open_res)
				{
					err = *RPC_open_res ;
					LIBPERROR("me_open_proc_1()=%d\n", err);
					break;
				}

				if (RPC_open_res)
				{
					free(RPC_open_res);
					RPC_open_res = NULL;
				}
			}
		}

		if (clnt)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

			status_params.device = context->device;
			status_params.subdevice = context->subdevice;
			status_params.wait = ME_WAIT_START;
			status_params.flags = ME_IO_STREAM_STATUS_NO_FLAGS;

			status_res = me_io_stream_status_proc_1(&status_params, clnt);

			if (!status_res)
			{
				LIBPERROR("me_io_stream_status_proc_1()=ME_ERRNO_COMMUNICATION\n");
				err = ME_ERRNO_COMMUNICATION;
				value = 0;
				streamStatus = ME_STATUS_INVALID;

				clnt_destroy(clnt);
				clnt = NULL;
			}
			else
			{
				if (status_res->error)
				{
					err = status_res->error;
					LIBPERROR("me_io_stream_status_proc_1()=%d\n", err);
				}
				else
				{
					err = ME_ERRNO_SUCCESS;
				}

				value = status_res->count;
				streamStatus = status_res->status;

				free(status_res);
				status_res = NULL;
			}

			pthread_testcancel();
			if (context->cancel)
			{
				break;
			}

			if (err && (err == old_err))
			{
				// Do not report the same error twice.
				continue;
			}
		}
		else
		{
			err = ME_ERRNO_COMMUNICATION;
		}

		old_err = err;

		/// Start or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				if (threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err))
				{
					if (context->cancel)
					{
						break;
					}

					if (!err)
					{/// Start ONLY.
						if (clnt)
						{
							stop_params.stop_list.stop_list_len = 1;
							stop_params.stop_list.stop_list_val = &stop_entry;
							stop_params.stop_list.stop_list_val->device = context->device;
							stop_params.stop_list.stop_list_val->subdevice = context->subdevice;
							stop_params.stop_list.stop_list_val->stop_mode = ME_STOP_MODE_IMMEDIATE;
							stop_params.stop_list.stop_list_val->flags = ME_IO_STREAM_STOP_NO_FLAGS;
							stop_params.flags = ME_IO_STREAM_STOP_NO_FLAGS;
							stop_res = me_io_stream_stop_proc_1(&stop_params, clnt);
							if (stop_res)
							{
								free(stop_res);
							}
						}
					}
				}
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
			{
				break;
			}
		}
	} // while()

	if (clnt)
	{
		clnt_destroy(clnt);
	}

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);

	return NULL;
}

static void* streamStopThread_RPC(void* arg)
{
	me_rpc_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;
	int* RPC_open_res ;
	int iFlags = 0;

	int streamStatus;
	int value = 0;
	static int old_err = ME_ERRNO_INVALID_ERROR_NUMBER;

	int err;

	CLIENT* clnt = NULL;
	me_io_stream_status_res* status_res = NULL;
	me_io_stream_status_params status_params;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		if (!clnt)
		{
			clnt = clnt_create(local_context->access_point_addr, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
			if (clnt)
			{
				RPC_open_res = me_open_proc_1(&iFlags, clnt);
				if (!RPC_open_res)
				{
					LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
					err = ME_ERRNO_COMMUNICATION;
					break;
				}
				else if (*RPC_open_res)
				{
					err = *RPC_open_res ;
					LIBPERROR("me_open_proc_1()=%d\n", err);
					break;
				}

				if (RPC_open_res)
				{
					free(RPC_open_res);
					RPC_open_res = NULL;
				}
			}
		}

		if (clnt)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);


			status_params.device = context->device;
			status_params.subdevice = context->subdevice;
			status_params.wait = ME_WAIT_STOP;
			status_params.flags = ME_IO_STREAM_STATUS_NO_FLAGS;

			status_res = me_io_stream_status_proc_1(&status_params, clnt);

			if (!status_res)
			{
				LIBPERROR("me_io_stream_status_proc_1()=ME_ERRNO_COMMUNICATION\n");
				err = ME_ERRNO_COMMUNICATION;
				value = 0;
				streamStatus = ME_STATUS_INVALID;

				clnt_destroy(clnt);
				clnt = NULL;
			}
			else
			{
				if (status_res->error)
				{
					err = status_res->error;
					LIBPERROR("me_io_stream_status_proc_1()=%d\n", err);
				}
				else
				{
					err = ME_ERRNO_SUCCESS;
				}

				value = status_res->count;
				streamStatus = status_res->status;

				free(status_res);
				status_res = NULL;
			}

// 			err = StreamStatus_RPC(local_context, context->device, context->subdevice, ME_WAIT_STOP, &streamStatus, &value, ME_IO_STREAM_STATUS_NO_FLAGS);
			pthread_testcancel();
			if (context->cancel)
			{
				break;
			}

			if (err && (err == old_err))
			{
				// Do not report the same error twice.
				continue;
			}
		}
		else
		{
			err = ME_ERRNO_COMMUNICATION;
		}

		old_err = err;

		/// Stop or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err);
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
			{
				break;
			}
		}
	} // while()

	if (context->cancel == 1)
	{
		free (context);
	}
	pthread_exit(NULL);

	return NULL;
}

static void* streamNewValuesThread_RPC(void* arg)
{
	me_rpc_context_t* local_context;
	threadContext_t	threadArgs;
	threadsList_t*	context;
	int* RPC_open_res ;
	int iFlags = 0;

	int value = 0;

	int ret;
	int err = ME_ERRNO_SUCCESS;
	int flag;

	CLIENT* clnt = NULL;
	me_io_stream_new_values_params nval_params;
	me_io_stream_new_values_res* nval_res = NULL;

	me_io_stream_stop_params stop_params;
	me_io_stream_stop_res* stop_res = NULL;
	me_io_stream_stop_entry_params stop_entry;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (!arg)
	{
		LIBPCRITICALERROR("No thread context provided!\n");
		pthread_exit (NULL);
		return NULL;
	}

	memcpy(&threadArgs, arg, sizeof(threadContext_t));
	free (arg);
	context = threadArgs.instance;
	local_context = context->context;

	LIBPDEBUG("iDevice=%d iSubdevice=%d\n", context->device, context->subdevice);

	if (!threadArgs.streamCB)
	{
		LIBPERROR("device=[%d,%d] ThreadID=%lld =>> No callback registred!\n", context->device, context->subdevice, (long long)context->threadID);
	}

	while (1)
	{
		if (!clnt)
		{
			clnt = clnt_create(local_context->access_point_addr, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
			if (clnt)
			{
				RPC_open_res = me_open_proc_1(&iFlags, clnt);
				if (!RPC_open_res)
				{
					LIBPERROR("me_open_proc_1()=ME_ERRNO_COMMUNICATION\n");
					err = ME_ERRNO_COMMUNICATION;
					break;
				}
				else if (*RPC_open_res)
				{
					err = *RPC_open_res ;
					LIBPERROR("me_open_proc_1()=%d\n", err);
					break;
				}

				if (RPC_open_res)
				{
					free(RPC_open_res);
					RPC_open_res = NULL;
				}
			}
		}

		if (clnt)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			flag = ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG;
			if (err && (err != ME_ERRNO_TIMEOUT))
			{
				flag |= ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG;
			}


			nval_params.device = context->device;
			nval_params.subdevice = context->subdevice;
			nval_params.time_out = 0;
			nval_params.flags = flag;

			nval_res = me_io_stream_new_values_proc_1(&nval_params, clnt);

			if (!nval_res)
			{
				LIBPERROR("me_io_stream_new_values_proc_1()=ME_ERRNO_COMMUNICATION\n");
				err = ME_ERRNO_COMMUNICATION;
				value = 0;

				clnt_destroy(clnt);
				clnt = NULL;
			}
			else
			{
				if (nval_res->error)
				{
					err = nval_res->error;
					LIBPERROR("me_io_stream_new_values_proc_1()=%d\n", err);
				}
				else
				{
					err = ME_ERRNO_SUCCESS;
				}

				value = nval_res->count;

				free(nval_res);
				nval_res = NULL;
			}

			pthread_testcancel();
			if (context->cancel)
			{
				break;
			}

			if (!err && !value)
			{
				continue;
			}
		}
		else
		{
			err = ME_ERRNO_COMMUNICATION;
		}

		/// New values or RESET -> call callback function.
		if (threadArgs.streamCB)
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			LIBPDEBUG("device[%d,%d] =>> CALLBACK\n", context->device, context->subdevice);
			pthread_mutex_lock(&local_context->callbackContextMutex);
				ret = threadArgs.streamCB(context->device, context->subdevice, value, threadArgs.contextCB, err);
			pthread_mutex_unlock(&local_context->callbackContextMutex);
			if (context->cancel)
				break;

			if (!ret)
			{
				if (clnt)
				{
					stop_params.stop_list.stop_list_len = 1;
					stop_params.stop_list.stop_list_val = &stop_entry;
					stop_params.stop_list.stop_list_val->device = context->device;
					stop_params.stop_list.stop_list_val->subdevice = context->subdevice;
					stop_params.stop_list.stop_list_val->stop_mode = ME_STOP_MODE_IMMEDIATE;
					stop_params.stop_list.stop_list_val->flags = ME_IO_STREAM_STOP_NO_FLAGS;
					stop_params.flags = ME_IO_STREAM_STOP_NO_FLAGS;
					stop_res = me_io_stream_stop_proc_1(&stop_params, clnt);
					if (stop_res)
					{
						free(stop_res);
					}
				}
			}
		}
	} // while()

	if (context->cancel == 1)
		free (context);
	pthread_exit(NULL);

	return NULL;
}

static int checkRPC(me_rpc_context_t* rpc_context)
{
	pid_t pid = getpid();
	int* RPC_open_res ;
	int iFlags = 0;

	if (rpc_context->pid == pid)
	{
		return ME_ERRNO_SUCCESS;
	}

	LIBPDEBUG("New thread detected! PID: %d\n", pid);
	rpc_context->pid = pid;

	pthread_mutex_lock(&rpc_context->rpc_mutex);
		rpc_context->fd = clnt_create(rpc_context->access_point_addr, RMEDRIVER_PROG, RMEDRIVER_VERS, "tcp");
		if (rpc_context->fd)
		{
			RPC_open_res = me_open_proc_1(&iFlags, rpc_context->fd);
			if (RPC_open_res)
			{
				free(RPC_open_res);
			}
		}
	pthread_mutex_unlock(&rpc_context->rpc_mutex);

	return (rpc_context->fd) ? ME_ERRNO_SUCCESS :ME_ERRNO_COMMUNICATION;
}