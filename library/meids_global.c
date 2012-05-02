/* Shared library for Meilhaus driver system.
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
 *  Author:	Guenter Gebhardt
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */
#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <syslog.h>
# include <unistd.h>
# include <time.h>

# include <float.h>
# include <math.h>

# include "me_error.h"
# include "me_types.h"
# include "me_defines.h"
# include "me_structs.h"
# include "me_ioctl.h"

# include "meids_common.h"
# include "meids_internal.h"
# include "meids_pthread.h"
# include "meids_debug.h"
# include "meids_config_structs.h"
# include "meids_structs.h"
# include "meids_error.c"	/* This file contains only table with errors' descriptions. */

# include "meids.h"
# include "meids_config.h"

/// Standard header file for library.
# include "medriver.h"

# include "pt_100.h"
# include "te_type_b.h"
# include "te_type_e.h"
# include "te_type_j.h"
# include "te_type_k.h"
# include "te_type_n.h"
# include "te_type_r.h"
# include "te_type_s.h"
# include "te_type_t.h"

# include "meids_init.h"

/* Hardware context. Default values. */
#ifndef LIBMEDRIVER_MAX_DEVICES
# define LIBMEDRIVER_MAX_DEVICES	32
#endif

#ifndef LIBMEDRIVER_MAX_SUBDEVICES
# define LIBMEDRIVER_MAX_SUBDEVICES	32
#endif

static int   doSingle(meIOSingle_t* pSingleList, int iCount, int iFlags);

static double meCalcPT100Temp(double resistance, int iIMeasured);
static double meCalcTCTemp(double dVoltage, double dTeGain,
							double* pdTable, unsigned long ulTableSize,
							double dMinTemp, double dTempOffset);

typedef struct globalContext
{
	// Protects the file descriptor and the client handles in the device context.
	pthread_mutex_t globalContextMutex;
}
globalContext_t;

static globalContext_t globalContext;

static int open_count;
static pthread_mutex_t open_count_mutex;


int test_RPC_timeout = 0;


// Error handling stuff

// Global error - last error's number
static int  meErrno = ME_ERRNO_SUCCESS;
static void SetErrno(int err);
static int  GetErrno(void);
static void meErrorProc(char* text, int err);

static int meErrorDefaultProcFlag = 0; // Flag for default error handler
static void meErrorDefaultProc(char* pcFunction, int iErrorCode);	//Default error handling function
static meErrorCB_t meErrorUserProc = NULL; // User defined error function

///Init and exit of the shared object
void __attribute__((constructor)) meids_global_init(void);
void __attribute__((destructor)) meids_global_fini(void);

///Library initialization and shutdown
void meids_global_init(void)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	pthread_mutex_init(&open_count_mutex, NULL);
	open_count = 0;

	pthread_mutex_init(&globalContext.globalContextMutex, NULL);

	openlog(NULL, 0, LOG_USER);
// 	openlog(NULL, 0 | LOG_CONS, LOG_USER);
}

void meids_global_fini(void)
{
	LIBPINFO("executed: %s\n", __FUNCTION__);

	if (open_count > 0)
	{
		open_count = 1;
		meClose(ME_CLOSE_NO_FLAGS);
	}

	closelog();
}


///Functions to access the driver system

int meOpen(int iFlags)
{
	addr_list_t* config = NULL;
	addr_list_t* nconf;
	int err;
	int err_ret = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	pthread_mutex_lock(&open_count_mutex);
		if (open_count < 0)
			open_count = 0;

		if (!open_count)
		{
			if (!CreateInit(MEIDS_CONF_FILE, &config) && config)
			{
				nconf = config;

				while (nconf)
				{
					err = ME_Open(nconf->addr, nconf->flag);
					if (!err)
						err_ret = err;

					nconf = nconf->next;
				}
				DestroyInit(&config);
			}
		}
		open_count++;
	pthread_mutex_unlock(&open_count_mutex);

	meErrorProc("meOpen()", err_ret);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err_ret;
}

int meClose(int iFlags)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	pthread_mutex_lock(&open_count_mutex);
		if (open_count < 0)
			open_count = 0;

		if (iFlags & ME_CLOSE_FORCE)
		{// Ignore reference counter and close library.
			open_count = 0;
			iFlags &= ~ME_CLOSE_FORCE;
		}
		else
		{
			--open_count;
		}

		if (!open_count)
		{
			err = ME_Close(iFlags);
		}
	pthread_mutex_unlock(&open_count_mutex);

	meErrorProc("meClose()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meLockDriver(int iLock, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("iLock=%d iFlags=0x%x", iLock, iFlags);

	err = ME_LockAll(iLock, iFlags);

	meErrorProc("meLockDriver()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meLockDevice(int iDevice, int iLock, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("device=%d lock=%d flags=0x%x", iDevice,  iLock , iFlags);

	err = ME_LockDevice(iDevice, iLock, iFlags);

	meErrorProc("meLockDevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meLockSubdevice(int iDevice, int iSubdevice, int iLock, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("iDevice=%d iSubdevice=%d iLock=%d iFlags=0x%x", iDevice, iSubdevice, iLock, iFlags);

	err = ME_LockSubdevice(iDevice, iSubdevice, iLock, iFlags);

	meErrorProc("meLockSubdevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

///Error handling functions

static void SetErrno(int err)
{
	if(err)
		meErrno = err;
}

static int GetErrno(void)
{
	return meErrno;
}

int meErrorGetLast(int* piErrorCode, int iFlags)
{

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("iErrorCode=%d iFlags=%d", *piErrorCode, iFlags);

	if (iFlags & ~ME_ERRNO_CLEAR_FLAGS)
	{
		meErrorProc("meErrorGetLast()", ME_ERRNO_INVALID_FLAGS);
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (!piErrorCode)
	{
		meErrorProc("meErrorGetLast()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	*piErrorCode =  GetErrno();
	if (iFlags)
	{
		meErrno = ME_ERRNO_SUCCESS;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meErrorGetLastMessage(char* pcErrorMsg, int iCount)
{
	return meErrorGetMessage(GetErrno(), pcErrorMsg, iCount);
}

int meErrorGetMessage(int iErrorCode, char* pcErrorMsg, int iCount)
{
	int ret = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("iErrorCode=%d iCount=%d", iErrorCode, iCount);

	if (!pcErrorMsg)
	{
		meErrorProc("meErrorGetMessage()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	if (iCount <= 0)
	{
		meErrorProc("meErrorGetMessage()", ME_ERRNO_INVALID_ERROR_MSG_COUNT);
		return ME_ERRNO_INVALID_ERROR_MSG_COUNT;
	}


	if (iErrorCode < ME_ERRNO_SUCCESS)
	{
		strerror_r(-iErrorCode, pcErrorMsg, iCount);
	}
	else if (iErrorCode > ME_ERRNO_INVALID_ERROR_NUMBER)
	{
		LIBPWARNING("ME_ERRNO_INVALID_ERROR_NUMBER");
		strncpy(pcErrorMsg, meErrorMsgTable[ME_ERRNO_INVALID_ERROR_NUMBER], iCount-1);
		ret = ME_ERRNO_INVALID_ERROR_NUMBER;
	}
	else if (strlen(meErrorMsgTable[iErrorCode]) > iCount - 1)
	{
		strncpy(pcErrorMsg, meErrorMsgTable[iErrorCode], iCount - 1);
		ret = ME_ERRNO_INVALID_ERROR_MSG_COUNT;
	}
	else
	{
		strcpy(pcErrorMsg, meErrorMsgTable[iErrorCode]);
	}

	// This is pure paranoia, but to be sure...
	pcErrorMsg[iCount - 1] = '\0';

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ret;
}

int meErrorSetDefaultProc(int iSwitch)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("iSwitch=%d", iSwitch);

	if (iSwitch == ME_SWITCH_ENABLE)
	{
		meErrorDefaultProcFlag = 1;
	}
	else if (iSwitch == ME_SWITCH_DISABLE)
	{
		meErrorDefaultProcFlag = 0;
	}
	else
	{
		LIBPWARNING("ME_ERRNO_INVALID_SWITCH");
		err = ME_ERRNO_INVALID_SWITCH;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meErrorSetUserProc(meErrorCB_t pErrorProc)
{
	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	meErrorUserProc = pErrorProc;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

void meErrorDefaultProc(char* pcFunction, int iErrorCode)
{
	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	char msg[ME_ERROR_MSG_MAX_COUNT] = {0};
	meErrorGetMessage(iErrorCode, msg, sizeof(msg));
	fprintf(stderr, "In function %s: %s (%d)\n", pcFunction, msg, iErrorCode);

	LIBPDEBUG("Error (%d) in function %s: %s \n", iErrorCode, pcFunction, msg);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);
}

///Functions to perform I/O on a device

int meIOIrqStart(int iDevice, int iSubdevice, int iChannel, int iIrqSource, int iIrqEdge, int iIrqArg, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_IrqStart(iDevice, iSubdevice, iChannel, iIrqSource, iIrqEdge, iIrqArg, iFlags);

	meErrorProc("meIOIrqStart()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOIrqStop(int iDevice, int iSubdevice, int iChannel, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_IrqStop(iDevice, iSubdevice, iChannel, iFlags);

	meErrorProc("meIOIrqStop()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOIrqWait(int iDevice, int iSubdevice, int iChannel, int* piIrqCount, int* piValue, int iTimeOut, int iFlags)
{
	int err;

	int* piIrqCount_local;
	int* piValue_local;
	int IrqCount_local = 0;
	int Value_local = 0;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	piIrqCount_local = (piIrqCount) ? piIrqCount: &IrqCount_local;
	piValue_local = (piValue) ? piValue : &Value_local;

	if (!(iFlags & ME_IO_IRQ_WAIT_PRESERVE))
	{
		iFlags &= ~ME_IO_IRQ_WAIT_PRESERVE;
	}
	else
	{
		*piIrqCount_local = 0;
	}

	err = ME_IrqWait(iDevice, iSubdevice, iChannel, piIrqCount_local, piValue_local, iTimeOut, iFlags);

	meErrorProc("meIOIrqWait()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOIrqSetCallback(int iDevice, int iSubdevice, meIOIrqCB_t pIrqCB, void* pContext, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_IrqSetCallback(iDevice, iSubdevice, pIrqCB, pContext, iFlags);

	meErrorProc("meIOIrqSetCallback()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

/// @todo Check if 'RESET' should remove callbacks. If not add flag for it.
int meIOResetDevice(int iDevice, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_ResetDevice(iDevice, iFlags);

	meErrorProc("meIOResetDevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOResetSubdevice(int iDevice, int iSubdevice, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_ResetSubdevice(iDevice, iSubdevice, iFlags);

	meErrorProc("meIOResetSubdevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

static int doSingle(meIOSingle_t* pSingleList, int iCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPDEBUG("pSingleList=%p iCount=%d iFlags=0x%x", pSingleList, iCount, iFlags);

	if (iCount <= 0)
	{
		LIBPDEBUG("No items in list.");
		return ME_ERRNO_INVALID_SINGLE_LIST;
	}

	if (iCount == 1)
	{
		if (iFlags)
		{
			LIBPDEBUG("Invalid flags specified.");
			return ME_ERRNO_INVALID_FLAGS;
		}

		err = ME_Single(pSingleList[0].iDevice, pSingleList[0].iSubdevice, pSingleList[0].iChannel,
				pSingleList[0].iDir, &pSingleList[0].iValue, pSingleList[0].iTimeOut, pSingleList[0].iFlags);
	}
	else
	{
		err = ME_SingleList(pSingleList, iCount, iFlags);
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOSingle(meIOSingle_t* pSingleList, int iCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!pSingleList)
		return ME_ERRNO_INVALID_POINTER;

	err = doSingle(pSingleList, iCount, iFlags);

	meErrorProc("meIOSingle()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOSingleConfig(int iDevice, int iSubdevice, int iChannel, int iSingleConfig, int iRef, int iTrigChan, int iTrigType, int iTrigEdge, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_SingleConfig(iDevice, iSubdevice, iChannel, iSingleConfig, iRef, iTrigChan, iTrigType, iTrigEdge, iFlags);

	meErrorProc("meIOSingleConfig()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamConfig(int iDevice, int iSubdevice, meIOStreamConfig_t* pConfigList, int iCount, meIOStreamTrigger_t* pTrigger, int iFifoIrqThreshold, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamConfig(iDevice, iSubdevice, pConfigList, iCount, pTrigger, iFifoIrqThreshold, iFlags);

	meErrorProc("meIOStreamConfig()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamNewValues(int iDevice, int iSubdevice, int iTimeOut, int* piCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamNewValues(iDevice, iSubdevice, iTimeOut, piCount, iFlags);

	meErrorProc("meIOStreamConfig()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamRead(int iDevice, int iSubdevice, int iReadMode, int* piValues, int* piCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamRead(iDevice, iSubdevice, iReadMode, piValues, piCount, 0, iFlags);

	meErrorProc("meIOStreamRead()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamWrite(int iDevice, int iSubdevice, int iWriteMode, int* piValues, int* piCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamWrite(iDevice, iSubdevice, iWriteMode, piValues, piCount, 0, iFlags);

	meErrorProc("meIOStreamWrite()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamSetCallbacks(int iDevice, int iSubdevice,
							meIOStreamCB_t pStartCB, void* pStartCBContext,
							meIOStreamCB_t pNewValuesCB, void* pNewValuesCBContext,
							meIOStreamCB_t pEndCB, void* pEndCBContext,
							int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamSetCallbacks(iDevice, iSubdevice,
								pStartCB, pStartCBContext,
								pNewValuesCB, pNewValuesCBContext,
								pEndCB, pEndCBContext,
								iFlags);

	meErrorProc("meIOStreamSetCallbacks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamStart(meIOStreamStart_t* pStartList, int iCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamStartList(pStartList, iCount, iFlags);

	meErrorProc("meIOStreamStart()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamStatus(int iDevice, int iSubdevice, int iWait, int* piStatus, int* piCount, int iFlags)
{
	int err;

	int* piStatus_local;
	int* piCount_local;
	int Status_local;
	int Count_local;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	piStatus_local = (piStatus) ? piStatus : &Status_local;
	piCount_local = (piCount) ? piCount : &Count_local;

	err = ME_StreamStatus(iDevice, iSubdevice, iWait, piStatus_local, piCount_local, iFlags);

	meErrorProc("meIOStreamStatus()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamStop(meIOStreamStop_t* pStopList, int iCount, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamStopList(pStopList, iCount, iFlags);

	meErrorProc("meIOStreamStop()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamTimeToTicks(int iDevice, int iSubdevice, int iTimer, double* pdTime, int* piTicksLow, int* piTicksHigh, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamTimeToTicks(iDevice, iSubdevice, iTimer, pdTime, piTicksLow, piTicksHigh, iFlags);

	meErrorProc("meIOStreamTimeToTicks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOSingleTimeToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			double *pdTime,
			int *piTicksLow,
			int *piTicksHigh,
			int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamTimeToTicks(iDevice, iSubdevice, iTimer, pdTime, piTicksLow, piTicksHigh, iFlags);

	meErrorProc("meIOSingleTimeToTicks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOStreamFrequencyToTicks(int iDevice, int iSubdevice, int iTimer, double* pdFrequency, int* piTicksLow, int* piTicksHigh, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_StreamFrequencyToTicks(iDevice, iSubdevice, iTimer, pdFrequency, piTicksLow, piTicksHigh, iFlags);

	meErrorProc("meIOStreamFrequencyToTicks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meIOSingleTicksToTime(
			int iDevice,
			int iSubdevice,
			int iTimer,
			int iTicksLow,
			int iTicksHigh,
			double *pdTime,
			int iFlags)
{
	int err = ME_ERRNO_SUCCESS;
	double nominal_freq = 1.0;
	int low;
	int high;

	long long unsigned int single_second_ticks;
	long long unsigned int ticks;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	*pdTime =  0.0;
	if (!(iTicksLow < 0 || iTicksLow < 0))
	{
		err = ME_StreamTimeToTicks(iDevice, iSubdevice, iTimer, &nominal_freq, &low, &high, iFlags);
		if (!err)
		{
			single_second_ticks = low + ((long long int)high << 32);
			if (single_second_ticks > 0)
			{
				ticks = iTicksLow + ((long long int)iTicksHigh << 32);
				*pdTime = (double)ticks / (double)single_second_ticks;
			}
		}
	}

	meErrorProc("meIOSingleTicksToTime()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}


int meIOSetChannelOffset(int iDevice, int iSubdevice, int iChannel, int iRange, double *pdOffset, int iFlags)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_SetOffset(iDevice, iSubdevice, iChannel, iRange, pdOffset, iFlags);

	meErrorProc("meIOSetChannelOffset()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

/// Functions to query the driver system

int meQueryVersionLibrary(int* piVersion)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryLibraryVersion(piVersion, ME_QUERY_NO_FLAGS);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}


int meQueryDescriptionDevice(int iDevice, char* pcDescription, int iCount)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryDeviceDescription(iDevice, pcDescription, iCount, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryDescriptionDevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNameDevice(int iDevice, char* pcName, int iCount)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	LIBPINFO("ID:%d\n", iDevice);

	err = ME_QueryDeviceName(iDevice, pcName, iCount, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryNameDevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNameDeviceDriver(int iDevice, char* pcName, int iCount)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdriverName(iDevice, pcName, iCount, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryNameDeviceDriver()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryInfoDevice(int iDevice,
						int* piVendorId, int* piDeviceId, int* piSerialNo,
						int* piBusType, int* piBusNo, int* piDevNo, int* piFuncNo,
						int* piPlugged)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryDeviceInfo(iDevice,
							(unsigned int *)piVendorId, (unsigned int *)piDeviceId, (unsigned int *)piSerialNo,
							(unsigned int *)piBusType, (unsigned int *)piBusNo, (unsigned int *)piDevNo, (unsigned int *)piFuncNo,
							piPlugged,
							ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryInfoDevice()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryVersionDeviceDriver(int iDevice, int* piVersion)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdriverVersion(iDevice, piVersion, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryVersionDeviceDriver()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryVersionMainDriver(int* piVersion)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryDriverVersion(0, piVersion, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryVersionMainDriver()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNumberDevices(int* piNumber)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryDevicesNumber(piNumber, ME_QUERY_NO_FLAGS);
	LIBPINFO("Detected %d devices\n", *piNumber);

	meErrorProc("meQueryNumberDevices()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNumberSubdevices(int iDevice, int* piNumber)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdevicesNumber(iDevice, piNumber, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryNumberSubdevices()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNumberChannels(int iDevice, int iSubdevice, int* piNumber)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryChannelsNumber(iDevice, iSubdevice, (unsigned int *)piNumber, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryNumberChannels()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryNumberRanges(int iDevice, int iSubdevice, int iUnit, int* piNumber)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryRangesNumber(iDevice, iSubdevice, iUnit, piNumber, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryNumberRanges()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryRangeByMinMax(int iDevice, int iSubdevice, int iUnit, double* pdMin, double* pdMax, int* piMaxData, int* piRange)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryRangeByMinMax(iDevice, iSubdevice, iUnit, pdMin, pdMax, piMaxData, piRange, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryRangeByMinMax()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQueryRangeInfo(int iDevice, int iSubdevice, int iRange, int* piUnit, double* pdMin, double* pdMax, int* piMaxData)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QueryRangeInfo(iDevice, iSubdevice, iRange, piUnit, pdMin, pdMax, (unsigned int *)piMaxData, ME_QUERY_NO_FLAGS);

	meErrorProc("meQueryRangeInfo()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQuerySubdeviceByType(int iDevice, int iStartSubdevice, int iType, int iSubtype, int* piSubdevice)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdeviceByType(iDevice, (iStartSubdevice < 0) ? 0 : iStartSubdevice, iType, iSubtype, piSubdevice, ME_QUERY_NO_FLAGS);

	meErrorProc("meQuerySubdeviceByType()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQuerySubdeviceType(int iDevice, int iSubdevice, int* piType, int* piSubtype)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdeviceType(iDevice, iSubdevice, piType, piSubtype, ME_QUERY_NO_FLAGS);

	meErrorProc("meQuerySubdeviceType()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQuerySubdeviceCaps(int iDevice, int iSubdevice, int* piCaps)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdeviceCaps(iDevice, iSubdevice, piCaps, ME_QUERY_NO_FLAGS);

	meErrorProc("meQuerySubdeviceCaps()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meQuerySubdeviceCapsArgs(int iDevice, int iSubdevice, int iCap, int* piArgs, int iCount)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_QuerySubdeviceCapsArgs(iDevice, iSubdevice, iCap, piArgs, iCount, ME_QUERY_NO_FLAGS);

	meErrorProc("meQuerySubdeviceCapsArgs()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}


/// Common utility functions

int meUtilityExtractValues(	int iChannel,
							int* piAIBuffer, int iAIBufferCount,
							meIOStreamConfig_t* pConfigList, int iConfigListCount,
							int* piChanBuffer, int* piChanBufferCount)
{
	int i = 0;
	int j = 0;
	int k = 0;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!piAIBuffer || !piChanBufferCount || !piChanBuffer)
	{
		meErrorProc("meUtilityExtractValues()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	while (iConfigListCount > 0)
	{
		for (j = 0; j < iConfigListCount; j++)
		{
			if ((k >= *piChanBufferCount) || (i * iConfigListCount + j >= iAIBufferCount))
			{
				*piChanBufferCount = k;
				return ME_ERRNO_SUCCESS;
			}

			if (pConfigList[j].iChannel == iChannel)
			{
				piChanBuffer[k] = piAIBuffer[i * iConfigListCount + j];
				k++;
			}
			else
			{
				continue;
			}
		}

		i++;
	}

	*piChanBufferCount = 0;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meUtilityPhysicalToDigital(double dMin, double dMax, int iMaxData, double dPhysical, int* piData)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!piData)
	{
		err = ME_ERRNO_INVALID_POINTER;
		meErrorProc("meUtilityPhysicalToDigital()", err);
		return err;
	}

	if (iMaxData < 0)
	{
		err = ME_ERRNO_INVALID_MIN_MAX;
		meErrorProc("meUtilityPhysicalToDigital()", err);
		return err;
	}

	if (iMaxData == 0)
	{
		*piData = 0;
		return err;
	}


	*piData = (dPhysical - dMin) / (dMax - dMin) * iMaxData + 0.5;

	if (*piData < 0)
	{
		*piData = 0;
		err  = ME_ERRNO_VALUE_OUT_OF_RANGE;
	}
	else if (*piData > iMaxData)
	{
		*piData = iMaxData;

		if (dPhysical > dMax)	// This is for covering some possible inaccuracy.
		{
			err  = ME_ERRNO_VALUE_OUT_OF_RANGE;
		}
	}

	meErrorProc("meUtilityPhysicalToDigital()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityDigitalToPhysical(double dMin, double dMax, int iMaxData, int iData, int iModuleType, double dRefValue, double* pdPhysical)
{
	int err = ME_ERRNO_SUCCESS;
	double dResistance;
	double dVoltage;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!pdPhysical)
		return ME_ERRNO_INVALID_POINTER;

	if (iMaxData <= 0)
	{
		err = ME_ERRNO_INVALID_MIN_MAX;
		*pdPhysical = 0;
	}

	if (iData < 0)
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*pdPhysical = dMin;
	}
	else if (iData > iMaxData)
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*pdPhysical = dMax;
	}

	if (!err)
	{

		dVoltage = dMax - dMin;
		dVoltage *= iData;
		dVoltage /= iMaxData;
		dVoltage += dMin;

		if (dRefValue == 0)
		{
			switch (iModuleType)
			{
				case ME_MODULE_TYPE_MULTISIG_RTD8_PT100:
				case ME_MODULE_TYPE_MULTISIG_RTD8_PT500:
				case ME_MODULE_TYPE_MULTISIG_RTD8_PT1000:
					dRefValue = 0.0005;
			}
		}

		switch (iModuleType)
		{

			case ME_MODULE_TYPE_MULTISIG_NONE:

			case ME_MODULE_TYPE_MULTISIG_DIFF16_10V:
				*pdPhysical = dVoltage;

				break;

			case ME_MODULE_TYPE_MULTISIG_DIFF16_20V:
				*pdPhysical = dVoltage * 2;

				break;

			case ME_MODULE_TYPE_MULTISIG_DIFF16_50V:
				*pdPhysical = dVoltage * 5;

				break;

			case ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA:
				*pdPhysical = 20E-3 / 10 * dVoltage;

				break;

			case ME_MODULE_TYPE_MULTISIG_RTD8_PT100:
				dResistance = dVoltage / 40 / dRefValue;

				*pdPhysical = meCalcPT100Temp(dResistance, dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_RTD8_PT500:
				dResistance = dVoltage / 8 / dRefValue;

				*pdPhysical = meCalcPT100Temp(dResistance, dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_RTD8_PT1000:
				dResistance = dVoltage / 4 / dRefValue;

				*pdPhysical = meCalcPT100Temp(dResistance, dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_B_GAIN,
								te_type_b,
								sizeof(te_type_b) / sizeof(double),
								ME_TE_TYPE_B_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_E_GAIN,
								te_type_e,
								sizeof(te_type_e) / sizeof(double),
								ME_TE_TYPE_E_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_J_GAIN,
								te_type_j,
								sizeof(te_type_j) / sizeof(double),
								ME_TE_TYPE_J_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_K_GAIN,
								te_type_k,
								sizeof(te_type_k) / sizeof(double),
								ME_TE_TYPE_K_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_N_GAIN,
								te_type_n,
								sizeof(te_type_n) / sizeof(double),
								ME_TE_TYPE_N_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_R_GAIN,
								te_type_r,
								sizeof(te_type_r) / sizeof(double),
								ME_TE_TYPE_R_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_S_GAIN,
								te_type_s,
								sizeof(te_type_s) / sizeof(double),
								ME_TE_TYPE_S_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T:
				*pdPhysical = meCalcTCTemp(
								dVoltage,
								ME_TE_TYPE_T_GAIN,
								te_type_t,
								sizeof(te_type_t) / sizeof(double),
								ME_TE_TYPE_T_MIN_TEMP,
								dRefValue);

				break;

			case ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR:
				*pdPhysical = (dVoltage / 4 - 500E-3) / 10E-3;

				break;

			default:
				err = ME_ERRNO_INVALID_MODULE_TYPE;
		}

		meErrorProc("meUtilityDigitalToPhysical()", err);
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityPhysicalToDigitalV(double dMin, double dMax, int iMaxData,
								double* pdPhysicalBuffer, int iCount,
								int* piDataBuffer)
{
	int err = ME_ERRNO_SUCCESS;
	int i;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!pdPhysicalBuffer || !piDataBuffer)
	{
		return ME_ERRNO_INVALID_POINTER;
	}

	for (i=0; i<iCount; i++)
	{
		meUtilityPhysicalToDigital(dMin, dMax, iMaxData, pdPhysicalBuffer[i], &piDataBuffer[i]);
	}

	meErrorProc("meUtilityDigitalToPhysical()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityDigitalToPhysicalV(double dMin, double dMax, int iMaxData,
								int* piDataBuffer, int iCount,
								int iModuleType, double dRefValue, double* pdPhysicalBuffer)
{
	int err = ME_ERRNO_SUCCESS;
	int i;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!pdPhysicalBuffer || !piDataBuffer)
	{
		return ME_ERRNO_INVALID_POINTER;
	}

	switch (iModuleType)
	{
		case ME_MODULE_TYPE_MULTISIG_NONE:
		case ME_MODULE_TYPE_MULTISIG_DIFF16_10V:
		case ME_MODULE_TYPE_MULTISIG_DIFF16_20V:
		case ME_MODULE_TYPE_MULTISIG_DIFF16_50V:
		case ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA:
		case ME_MODULE_TYPE_MULTISIG_RTD8_PT100:
		case ME_MODULE_TYPE_MULTISIG_RTD8_PT500:
		case ME_MODULE_TYPE_MULTISIG_RTD8_PT1000:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S:
		case ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T:
		case ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR:
			break;

		default:
			err = ME_ERRNO_INVALID_MODULE_TYPE;
			iModuleType = ME_MODULE_TYPE_MULTISIG_NONE;
	}

	for (i = 0; i < iCount; i++)
	{
		meUtilityDigitalToPhysical(dMin, dMax, iMaxData, piDataBuffer[i], iModuleType, dRefValue, &pdPhysicalBuffer[i]);
	}

	meErrorProc("meUtilityDigitalToPhysical()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

static double meCalcPT100Temp(double resistance, int iIMeasured)
{
	int i = 0;
	double grad = ME_PT_100_TEMP_OFFSET;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (resistance < pt_100[i])
	{
		grad = ME_PT_100_TEMP_OFFSET;
		return grad;
	}

	for (i = 0; i < ME_PT_100_TABLE_COUNT; i++)
	{
		if (resistance > pt_100[i])
		{
			continue;
		}
		else if (resistance == pt_100[i])
		{
			grad = i + ME_PT_100_TEMP_OFFSET;
			break;
		}
		else if (resistance < pt_100[i])
		{
			grad = (i - 1) + (1 / (pt_100[i] - pt_100[i - 1])) * (resistance - pt_100[i - 1]) + ME_PT_100_TEMP_OFFSET;
			break;
		}
	}

	if (i == ME_PT_100_TABLE_COUNT)
	{
		grad = ME_PT_100_TABLE_COUNT - 1 + ME_PT_100_TEMP_OFFSET;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return grad;
}

static double meCalcTCTemp(double dVoltage, double dTeGain,
							double* pdTable, unsigned long ulTableSize,
							double dMinTemp, double dTempOffset)
{
	int i;
	double tevoltage;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	tevoltage = dVoltage / dTeGain * 1E6; // The table holds the voltage in uV

	if (tevoltage <= pdTable[0])
	{
		return pdTable[0];
	}

	if (tevoltage >= pdTable[ulTableSize - 1])
	{
		return pdTable[ulTableSize - 1];
	}

	for (i = 0; i < ulTableSize; i++)
	{
		if (tevoltage == pdTable[i])
		{
			return dMinTemp + i + dTempOffset;
		}
		else if (tevoltage > pdTable[i])
		{
			continue;
		}
		else
		{
			return (((dMinTemp + i) + (tevoltage - pdTable[i]) / (pdTable[i + 1] - pdTable[i])) + dTempOffset);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return 0.0;
}

int meUtilityPWMStart(int iDevice, int iSubdevice1, int iSubdevice2, int iSubdevice3, int iRef, int iPrescaler, int iDutyCycle, int iFlag)
{
	int err;
	int type;
	int subtype;
	int i;
	int iRef_C2;
	meIOSingle_t list[3];
	int caps;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	// Check if all of them are counters.
	err = meQuerySubdeviceType(iDevice, iSubdevice1, &type, &subtype);
	if (err)
		return err;

	if (type != ME_TYPE_CTR)
	{
		err = ME_ERRNO_NOT_SUPPORTED;
		meErrorProc("meUtilityPWMStart()", err);
		return err;
	}

	err = meQuerySubdeviceType(iDevice, iSubdevice2, &type, &subtype);
	if (err)
		return err;

	if (type != ME_TYPE_CTR)
	{
		err = ME_ERRNO_NOT_SUPPORTED;
		meErrorProc("meUtilityPWMStart()", err);
		return err;
	}

	err = meQuerySubdeviceType(iDevice, iSubdevice3, &type, &subtype);
	if (err)
		return err;

	if (type != ME_TYPE_CTR)
	{
		err = ME_ERRNO_NOT_SUPPORTED;
		meErrorProc("meUtilityPWMStart()", err);
		return err;
	}


	if (iFlag == ME_PWM_START_CONNECT_INTERNAL)
	{
		// Internally connected: Out_Counter1 >> Gate_Counter2
		// Check if counters are in correct order

		if (iSubdevice1+1 != iSubdevice2)
		{
			err = ME_ERRNO_NOT_SUPPORTED;
			meErrorProc("meUtilityPWMStart()", err);
			return err;
		}

		// Check if internal connections supported by hardware
		// Counter1 >> Counter2
		err = meQuerySubdeviceCaps(iDevice, iSubdevice2, &caps);
		if (err)
			return err;

		if ((caps & ME_CAPS_CTR_CLK_PREVIOUS) != ME_CAPS_CTR_CLK_PREVIOUS)
		{
			err = ME_ERRNO_NOT_SUPPORTED;
			meErrorProc("meUtilityPWMStart()", err);
			return err;
		}

		iRef_C2 = ME_REF_CTR_PREVIOUS;
	}
	else if (iFlag == ME_VALUE_NOT_USED)
	{
		// Extrnal conections - no needs to check anything here
		iRef_C2 = ME_REF_CTR_EXTERNAL;
	}
	else
	{
		/* Wrong flag value */
		err = ME_ERRNO_INVALID_FLAGS;

		meErrorProc("meUtilityPWMStart()", err);
		return err;
	}

	if (iRef != ME_REF_CTR_EXTERNAL)
	{
		// Check if choosen clock source is supported
		err = meQuerySubdeviceCaps(iDevice, iSubdevice1, &caps);
		if (err)
			return err;

		if (iRef == ME_REF_CTR_INTERNAL_1MHZ)
		{
			if ((caps & ME_CAPS_CTR_CLK_INTERNAL_1MHZ) != ME_CAPS_CTR_CLK_INTERNAL_1MHZ)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMStart()", err);
				return err;
			}
		}
		else if (iRef == ME_REF_CTR_INTERNAL_10MHZ)
		{
			if ((caps & ME_CAPS_CTR_CLK_INTERNAL_10MHZ) != ME_CAPS_CTR_CLK_INTERNAL_10MHZ)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMStart()", err);
				return err;
			}
		}
		else if (iRef == ME_REF_CTR_PREVIOUS)
		{
			//This is for me1400D. All counters can be cascaded -> signal's source can be an output from previous counter.

			if ((caps & ME_CAPS_CTR_CLK_PREVIOUS) != ME_CAPS_CTR_CLK_PREVIOUS)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMStart()", err);
				return err;
			}
		}
		else
		{
			//error - no more internal signal's sources!
			err = ME_ERRNO_INVALID_REF;
			meErrorProc("meUtilityPWMStart()", err);
			return err;
		}
	}

	if ((iDutyCycle > 99) || (iDutyCycle < 1))
	{
		err = ME_ERRNO_INVALID_DUTY_CYCLE;
		meErrorProc("meUtilityPWMStart()", err);
		return err;
	}

	err = meIOSingleConfig(iDevice, iSubdevice1, 0, ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR, iRef, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, 0, ME_IO_SINGLE_CONFIG_NO_FLAGS);
	if (err)
		return err;

	err = meIOSingleConfig( iDevice, iSubdevice2, 0, ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR, iRef_C2, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, 0, ME_IO_SINGLE_CONFIG_NO_FLAGS);
	if (err)
		return err;

	err = meIOSingleConfig(iDevice, iSubdevice3, 0, ME_SINGLE_CONFIG_CTR_8254_MODE_INTERRUPT_ON_TERMINAL_COUNT, ME_REF_CTR_EXTERNAL, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, 0, ME_IO_SINGLE_CONFIG_NO_FLAGS);
	if (err)
		return err;

	for (i = 0; i < 3; i++)
	{
		list[i].iDevice = iDevice;
		list[i].iChannel = 0;
		list[i].iDir = ME_DIR_OUTPUT;
		list[i].iFlags = ME_IO_SINGLE_TYPE_NO_FLAGS;
		list[i].iErrno = ME_ERRNO_SUCCESS;
	}

	list[0].iSubdevice = iSubdevice1;

	list[1].iSubdevice = iSubdevice2;
	list[2].iSubdevice = iSubdevice3;

	list[0].iValue = iPrescaler;
	list[1].iValue = 100;
	list[2].iValue = iDutyCycle;

	err = meIOSingle(list, 3, ME_IO_SINGLE_NO_FLAGS);
	if (err)
		return err;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meUtilityPWMStop(int iDevice, int iSubdevice1)
{
	int err;
	int type;
	int subtype;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = meQuerySubdeviceType(iDevice, iSubdevice1, &type, &subtype);
	if (err)
		return err;

	if ((type != ME_TYPE_CTR))
	{
		err = ME_ERRNO_NOT_SUPPORTED;
		meErrorProc("meUtilityPWMStop()", err);
		return err;
	}

	//Reset prescaler -> block it
	err = meIOResetSubdevice(iDevice, iSubdevice1,ME_IO_RESET_SUBDEVICE_NO_FLAGS);
	if (err)
		return err;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meUtilityPWMRestart(int iDevice, int iSubdevice1, int iRef, int iPrescaler)
{
	int err;
	int type;
	int subtype;
	int caps;
	meIOSingle_t list;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	/* Check if all of them are counters */
	err = meQuerySubdeviceType(iDevice, iSubdevice1, &type, &subtype);
	if (err)
		return err;

	if (type != ME_TYPE_CTR)
	{
		err = ME_ERRNO_NOT_SUPPORTED;
		meErrorProc("meUtilityPWMRestart()", err);
		return err;
	}

	if (iRef != ME_REF_CTR_EXTERNAL)
	{
		// Check if choosen clock source is supported
		err = meQuerySubdeviceCaps(iDevice, iSubdevice1, &caps);
		if (err)
			return err;

		if (iRef == ME_REF_CTR_INTERNAL_1MHZ)
		{
			if ((caps & ME_CAPS_CTR_CLK_INTERNAL_1MHZ) != ME_CAPS_CTR_CLK_INTERNAL_1MHZ)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMRestart()", err);
				return err;
			}
		}
		else if (iRef == ME_REF_CTR_INTERNAL_10MHZ)
		{
			if ((caps & ME_CAPS_CTR_CLK_INTERNAL_10MHZ) != ME_CAPS_CTR_CLK_INTERNAL_10MHZ)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMRestart()", err);
				return err;
			}
		}
		else if (iRef == ME_REF_CTR_PREVIOUS)
		{
			//This is for me1400D. All counters can be cascaded -> signal's source can be an output from previous counter.

			if ((caps & ME_CAPS_CTR_CLK_PREVIOUS) != ME_CAPS_CTR_CLK_PREVIOUS)
			{
				err = ME_ERRNO_INVALID_REF;
				meErrorProc("meUtilityPWMRestart()", err);
				return err;
			}
		}
		else
		{
			//error - no more internal signal's sources!
			err = ME_ERRNO_INVALID_REF;
			meErrorProc("meUtilityPWMRestart()", err);
			return err;
		}
	}

	//Set mode 2
	err = meIOSingleConfig(iDevice, iSubdevice1, 0, ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR, iRef, ME_TRIG_CHAN_DEFAULT, ME_TRIG_TYPE_SW, 0, ME_IO_SINGLE_CONFIG_NO_FLAGS);
	if (err)
		return err;

	//Start prescaler
	list.iDevice = iDevice;
	list.iChannel = 0;
	list.iDir = ME_DIR_OUTPUT;
	list.iFlags = ME_IO_SINGLE_TYPE_NO_FLAGS;
	list.iErrno = ME_ERRNO_SUCCESS;
	list.iSubdevice = iSubdevice1;
	list.iValue = iPrescaler;

	err = meIOSingle(&list, 1, ME_IO_SINGLE_NO_FLAGS);
	if (err)
		return err;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meConfigLoad(char* pParamSet)
{
	int err;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	err = ME_ParametersSet((me_extra_param_set_t *)pParamSet, ((me_extra_param_set_t *)pParamSet)->flags);

	meErrorProc("meConfigLoad()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}


static void meErrorProc(char* text, int err)
{
	if (err)
	{
		LIBPERROR("Error (%d) in function %s. \n", err, text);
		if (meErrorDefaultProcFlag)
			meErrorDefaultProc(text, err);
		if (meErrorUserProc)
			meErrorUserProc(text, err);
	}

	SetErrno(err);
}

void ME_SetErrno(char* text, int err)
{
	if (text)
	{
		meErrorProc(text, err);
	}
	else
	{
		SetErrno(err);
	}
}

int meUtilityPeriodToTicks(int iBaseFreq, double dPeriod, unsigned int* piTicks)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if ((iBaseFreq <= 0) || (dPeriod < 0))
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*piTicks = 0;
	}
	else
	{
		*piTicks = dPeriod * (double)iBaseFreq;
	}

	meErrorProc("meUtilityPeriodToTicks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityTicksToPeriod(int iBaseFreq, unsigned int iTicks, double* pdPeriod)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (iBaseFreq <= 0)
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*pdPeriod = 0;
	}
	else
	{
		*pdPeriod = (double)iTicks / (double)iBaseFreq;
	}

	meErrorProc("meUtilityTicksToPeriod()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityFrequencyToTicks(int iBaseFreq, double dFrequency, unsigned int* piTicks)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if ((dFrequency <= 0) || (iBaseFreq <= 0))
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*piTicks =  0;
	}
	else
	{
		*piTicks =  (double)iBaseFreq / dFrequency;
	}

	meErrorProc("meUtilityFrequencyToTicks()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityTicksToFrequency(int iBaseFreq, unsigned int iTicks, double* pdFrequency)
{
	int err = ME_ERRNO_SUCCESS;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);

	if (iBaseFreq <= 0)
	{
		err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		*pdFrequency =  0;
	}
	else
	{
		*pdFrequency = (double)iBaseFreq / (double)iTicks;
	}

	meErrorProc("meUtilityTicksToFrequency()", err);

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return err;
}

int meUtilityCodeDivider(double dDivider, unsigned int* piDivider)
{
	double tmp;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!piDivider)
	{
		meErrorProc("meUtilityCodeDivider()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	if (dDivider < 0.0)
	{
		tmp = 0.0;
	}
	else if (dDivider > 1.0)
	{
		tmp = 1.0;
	}
	else
	{
		tmp = dDivider;
	}

	tmp *= 0x10000;
	tmp *= 0x10000;
	*piDivider = tmp;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}

int meUtilityDecodeDivider(unsigned int iDivider, double* pdDivider)
{
	double tmp = (unsigned int)iDivider;

	struct timespec ts_pre;
	struct timespec ts_post;

	LIBPINFO("executed: %s\n", __FUNCTION__);

	clock_gettime(CLOCK_MONOTONIC, &ts_pre);


	if (!pdDivider)
	{
		meErrorProc("meUtilityDecodeDivider()", ME_ERRNO_INVALID_POINTER);
		return ME_ERRNO_INVALID_POINTER;
	}

	tmp /= 0x10000;
	tmp /= 0x10000;
	*pdDivider = (tmp > 0) ? tmp : 1 + tmp;

	clock_gettime(CLOCK_MONOTONIC, &ts_post);
	LIBPEXECTIME("executed in %ld us\n", (ts_post.tv_nsec - ts_pre.tv_nsec) / 1000 + (ts_post.tv_sec - ts_pre.tv_sec) * 1000000);

	return ME_ERRNO_SUCCESS;
}
