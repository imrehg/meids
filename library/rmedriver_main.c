/* RPC server demon for ME-iDS
 * ===========================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author:	Guenter Gebhardt
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifdef __KERNEL__
# error This is user space demon!
#endif	//__KERNEL__

#include <rpc/rpc_msg.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include "rmedriver.h"

#include "meids_debug.h"

#define RMEDRIVER_SVC_PORT		65000
#define RMEDRIVER_LISTEN_QUEUE	10


/*===========================================================================
  Read and write data from or to a tcp connection respectively
  =========================================================================*/

static int readtcp(char *ctx, caddr_t buf, register int len)
{
	/// This is done to remove warning.
	register long int sock = (long int) ctx;
	fd_set mask;
	fd_set readfds;

	FD_ZERO(&mask);
	FD_SET(sock, &mask);

	do
	{

		readfds = mask;

		if (select(_rpc_dtablesize(), &readfds, NULL, NULL, NULL) < 0)
		{
			if (errno == EINTR)
				continue;

			goto fatal_err;
		}

	}
	while (!FD_ISSET(sock, &readfds));

	if ((len = read(sock, buf, len)) > 0)
		return len;

fatal_err:
	return -1;
}


static int writetcp(char *ctx, caddr_t buf, int len)
{
	/// This is done to remove warning.
	register long int sock = (long int) ctx;
	register int i, cnt;

	for (cnt = len; cnt > 0; cnt -= i, buf += i)
	{
		if ((i = write(sock, buf, cnt)) < 0)
			return -1;
	}

	return len;
}


/*===========================================================================
  Signal handler to catch all zombies
  =========================================================================*/

static void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{}

	return;
}


/*===========================================================================
  The dispatcher
  =========================================================================*/

static int dispatch(XDR *xdrs)
{
	bool_t retStatus = TRUE;

	struct rpc_msg msg;
	union {
		int me_close_proc_1_arg;
		int me_open_proc_1_arg;
		me_lock_driver_params me_lock_driver_proc_1_arg;
		me_lock_device_params me_lock_device_proc_1_arg;
		me_lock_subdevice_params me_lock_subdevice_proc_1_arg;
		me_io_irq_stop_params me_io_irq_stop_proc_1_arg;
		me_io_irq_start_params me_io_irq_start_proc_1_arg;
		me_io_irq_wait_params me_io_irq_wait_proc_1_arg;
		me_io_reset_device_params me_io_reset_device_proc_1_arg;
		me_io_reset_subdevice_params me_io_reset_subdevice_proc_1_arg;
		me_io_single_config_params me_io_single_config_proc_1_arg;
		me_io_single_params me_io_single_proc_1_arg;
		me_io_stream_config_params me_io_stream_config_proc_1_arg;
		me_io_stream_read_params me_io_stream_read_proc_1_arg;
		me_io_stream_write_params me_io_stream_write_proc_1_arg;
		me_io_stream_start_params me_io_stream_start_proc_1_arg;
		me_io_stream_stop_params me_io_stream_stop_proc_1_arg;
		me_io_stream_status_params me_io_stream_status_proc_1_arg;
		me_io_stream_frequency_to_ticks_params me_io_stream_frequency_to_ticks_proc_1_arg;
		me_io_stream_time_to_ticks_params me_io_stream_time_to_ticks_proc_1_arg;
		me_io_stream_new_values_params me_io_stream_new_values_proc_1_arg;
		int me_query_description_device_proc_1_arg;
		int me_query_info_device_proc_1_arg;
		int me_query_name_device_proc_1_arg;
		int me_query_name_device_driver_proc_1_arg;
		int me_query_number_subdevices_proc_1_arg;
		me_query_number_channels_params me_query_number_channels_proc_1_arg;
		me_query_number_ranges_params me_query_number_ranges_proc_1_arg;
		me_query_range_by_min_max_params me_query_range_by_min_max_proc_1_arg;
		me_query_range_info_params me_query_range_info_proc_1_arg;
		me_query_subdevice_by_type_params me_query_subdevice_by_type_proc_1_arg;
		me_query_subdevice_type_params me_query_subdevice_type_proc_1_arg;
		me_query_subdevice_caps_params me_query_subdevice_caps_proc_1_arg;
		me_query_subdevice_caps_args_params me_query_subdevice_caps_args_proc_1_arg;
		int me_query_version_device_driver_proc_1_arg;
	} argument;

// 	char *result;
	void* result = NULL;

	xdrproc_t _xdr_argument, _xdr_result;

	char *(*local)(char *, struct svc_req *);

	xdrs->x_op = XDR_DECODE;

	if (!xdrrec_skiprecord(xdrs))
		return 1;

	if (!xdr_callmsg(xdrs, &msg))
		return 1;

	if (msg.ru.RM_cmb.cb_prog != RMEDRIVER_PROG)
	{
		LIBPERROR("Invalid program number.");
		msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
		msg.rm_direction = REPLY;
		msg.ru.RM_rmb.rp_stat = MSG_ACCEPTED;
		msg.ru.RM_rmb.ru.RP_ar.ar_stat = PROG_UNAVAIL;

		xdrs->x_op = XDR_ENCODE;
		xdr_replymsg(xdrs, &msg);
		xdrrec_endofrecord(xdrs, 1);

		return 0;
	}

	if (msg.ru.RM_cmb.cb_vers != RMEDRIVER_VERS)
	{
		LIBPERROR("Invalid program version number.");
		msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
		msg.rm_direction = REPLY;
		msg.ru.RM_rmb.rp_stat = MSG_ACCEPTED;
		msg.ru.RM_rmb.ru.RP_ar.ar_stat = PROG_MISMATCH;
		msg.ru.RM_rmb.ru.RP_ar.ru.AR_versions.low = 1;
		msg.ru.RM_rmb.ru.RP_ar.ru.AR_versions.high = 1;

		xdrs->x_op = XDR_ENCODE;
		xdr_replymsg(xdrs, &msg);
		xdrrec_endofrecord(xdrs, 1);

		return 0;
	}

	switch (msg.ru.RM_cmb.cb_proc)
	{

		case NULLPROC:
			msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
			msg.rm_direction = REPLY;
			msg.ru.RM_rmb.rp_stat = MSG_ACCEPTED;
			msg.ru.RM_rmb.ru.RP_ar.ar_stat = SUCCESS;
			msg.acpted_rply.ar_results.where = NULL;
			msg.acpted_rply.ar_results.proc = (xdrproc_t) xdr_void;

			xdrs->x_op = XDR_ENCODE;
			xdr_replymsg(xdrs, &msg);
			xdrrec_endofrecord(xdrs, 1);

			return 0;

		case ME_CLOSE_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_close_proc_1_svc;

			break;

		case ME_OPEN_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_open_proc_1_svc;

			break;

		case ME_LOCK_DRIVER_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_lock_driver_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_lock_driver_proc_1_svc;

			break;

		case ME_LOCK_DEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_lock_device_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_lock_device_proc_1_svc;

			break;

		case ME_LOCK_SUBDEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_lock_subdevice_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_lock_subdevice_proc_1_svc;

			break;

		case ME_IO_IRQ_STOP_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_irq_stop_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_irq_stop_proc_1_svc;

			break;

		case ME_IO_IRQ_START_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_irq_start_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_irq_start_proc_1_svc;

			break;

		case ME_IO_IRQ_WAIT_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_irq_wait_params;
			_xdr_result = (xdrproc_t) xdr_me_io_irq_wait_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_irq_wait_proc_1_svc;

			break;

		case ME_IO_RESET_DEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_reset_device_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_reset_device_proc_1_svc;

			break;

		case ME_IO_RESET_SUBDEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_reset_subdevice_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_reset_subdevice_proc_1_svc;

			break;

		case ME_IO_SINGLE_CONFIG_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_single_config_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_single_config_proc_1_svc;

			break;

		case ME_IO_SINGLE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_single_params;
			_xdr_result = (xdrproc_t) xdr_me_io_single_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_single_proc_1_svc;

			break;

		case ME_IO_STREAM_CONFIG_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_config_params;
			_xdr_result = (xdrproc_t) xdr_int;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_config_proc_1_svc;

			break;

		case ME_IO_STREAM_READ_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_read_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_read_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_read_proc_1_svc;

			break;

		case ME_IO_STREAM_WRITE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_write_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_write_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_write_proc_1_svc;

			break;

		case ME_IO_STREAM_START_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_start_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_start_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_start_proc_1_svc;

			break;

		case ME_IO_STREAM_STOP_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_stop_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_stop_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_stop_proc_1_svc;

			break;

		case ME_IO_STREAM_STATUS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_status_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_status_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_status_proc_1_svc;

			break;

		case ME_IO_STREAM_FREQUENCY_TO_TICKS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_frequency_to_ticks_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_frequency_to_ticks_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_frequency_to_ticks_proc_1_svc;

			break;

		case ME_IO_STREAM_TIME_TO_TICKS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_time_to_ticks_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_time_to_ticks_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_time_to_ticks_proc_1_svc;

			break;

		case ME_IO_STREAM_NEW_VALUES_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_io_stream_new_values_params;
			_xdr_result = (xdrproc_t) xdr_me_io_stream_new_values_res;

			local = (char * (*)(char *, struct svc_req *)) me_io_stream_new_values_proc_1_svc;

			break;

		case ME_QUERY_DESCRIPTION_DEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_description_device_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_description_device_proc_1_svc;

			break;

		case ME_QUERY_INFO_DEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_info_device_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_info_device_proc_1_svc;

			break;

		case ME_QUERY_NAME_DEVICE_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_name_device_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_name_device_proc_1_svc;

			break;

		case ME_QUERY_NAME_DEVICE_DRIVER_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_name_device_driver_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_name_device_driver_proc_1_svc;

			break;

		case ME_QUERY_NUMBER_DEVICES_PROC:
			_xdr_argument = (xdrproc_t) xdr_void;
			_xdr_result = (xdrproc_t) xdr_me_query_number_devices_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_number_devices_proc_1_svc;

			break;

		case ME_QUERY_NUMBER_SUBDEVICES_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_number_subdevices_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_number_subdevices_proc_1_svc;

			break;

		case ME_QUERY_NUMBER_CHANNELS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_number_channels_params;
			_xdr_result = (xdrproc_t) xdr_me_query_number_channels_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_number_channels_proc_1_svc;

			break;

		case ME_QUERY_NUMBER_RANGES_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_number_ranges_params;
			_xdr_result = (xdrproc_t) xdr_me_query_number_ranges_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_number_ranges_proc_1_svc;

			break;

		case ME_QUERY_RANGE_BY_MIN_MAX_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_range_by_min_max_params;
			_xdr_result = (xdrproc_t) xdr_me_query_range_by_min_max_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_range_by_min_max_proc_1_svc;

			break;

		case ME_QUERY_RANGE_INFO_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_range_info_params;
			_xdr_result = (xdrproc_t) xdr_me_query_range_info_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_range_info_proc_1_svc;

			break;

		case ME_QUERY_SUBDEVICE_BY_TYPE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_subdevice_by_type_params;
			_xdr_result = (xdrproc_t) xdr_me_query_subdevice_by_type_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_subdevice_by_type_proc_1_svc;

			break;

		case ME_QUERY_SUBDEVICE_TYPE_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_subdevice_type_params;
			_xdr_result = (xdrproc_t) xdr_me_query_subdevice_type_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_subdevice_type_proc_1_svc;

			break;

		case ME_QUERY_SUBDEVICE_CAPS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_subdevice_caps_params;
			_xdr_result = (xdrproc_t) xdr_me_query_subdevice_caps_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_subdevice_caps_proc_1_svc;

			break;

		case ME_QUERY_SUBDEVICE_CAPS_ARGS_PROC:
			_xdr_argument = (xdrproc_t) xdr_me_query_subdevice_caps_args_params;
			_xdr_result = (xdrproc_t) xdr_me_query_subdevice_caps_args_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_subdevice_caps_args_proc_1_svc;

			break;

		case ME_QUERY_VERSION_LIBRARY_PROC:
			_xdr_argument = (xdrproc_t) xdr_void;
			_xdr_result = (xdrproc_t) xdr_me_query_version_library_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_version_library_proc_1_svc;

			break;

		case ME_QUERY_VERSION_MAIN_DRIVER_PROC:
			_xdr_argument = (xdrproc_t) xdr_void;
			_xdr_result = (xdrproc_t) xdr_me_query_version_main_driver_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_version_main_driver_proc_1_svc;

			break;

		case ME_QUERY_VERSION_DEVICE_DRIVER_PROC:
			_xdr_argument = (xdrproc_t) xdr_int;
			_xdr_result = (xdrproc_t) xdr_me_query_version_device_driver_res;

			local = (char * (*)(char *, struct svc_req *)) me_query_version_device_driver_proc_1_svc;

			break;

		default:
			LIBPERROR("Invalid procedure number.\n");

			msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
			msg.rm_direction = REPLY;
			msg.ru.RM_rmb.rp_stat = MSG_ACCEPTED;
			msg.ru.RM_rmb.ru.RP_ar.ar_stat = PROC_UNAVAIL;

			xdrs->x_op = XDR_ENCODE;
			xdr_replymsg(xdrs, &msg);

			xdrrec_endofrecord(xdrs, 1);

			return 0;
	}

	memset((char *) &argument, 0, sizeof(argument));

	xdrs->x_op = XDR_DECODE;
	if (!_xdr_argument(xdrs, (caddr_t) &argument))
	{
		LIBPERROR("Cannot read arguments.\n");
		msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
		msg.rm_direction = REPLY;
		msg.ru.RM_rmb.rp_stat = MSG_ACCEPTED;
		msg.ru.RM_rmb.ru.RP_ar.ar_stat = GARBAGE_ARGS;

		xdrs->x_op = XDR_ENCODE;
		xdr_replymsg(xdrs, &msg);
		xdrrec_endofrecord(xdrs, 1);

		return 0;
	}

	/* Call local procedure */
	result = (*local)((char *) &argument, NULL);
	if (!result)
	{
		LIBPERROR("Error while calling procedure. Return pointer equal NULL\n");
		retStatus = FALSE;
	}
	else
	{
		msg.ru.RM_rmb.ru.RP_ar.ar_verf = msg.ru.RM_cmb.cb_verf;
		msg.rm_direction = REPLY;
		msg.rm_reply.rp_stat = MSG_ACCEPTED;
		msg.acpted_rply.ar_stat = SUCCESS;
		msg.acpted_rply.ar_results.where = result;
		msg.acpted_rply.ar_results.proc = (xdrproc_t) _xdr_result;

		xdrs->x_op = XDR_ENCODE;

		if (!xdr_replymsg(xdrs, &msg))
		{
			LIBPERROR("Can't send reply.\n");
			retStatus = FALSE;
		}

		xdrrec_endofrecord(xdrs, 1);
	}

	xdrs->x_op = XDR_FREE;
	if (!_xdr_argument(xdrs, (caddr_t) &argument))
	{
		LIBPERROR("Cannot free arguments.\n");
		retStatus = FALSE;
	}

	if (result)
	{
		free(result);
	}

	return (retStatus) ? 0 : 1;
}


/*===========================================================================
  Main function
  =========================================================================*/

int main(int argc, char **argv)
{
	/// This is done to remove warning.
	long int connfd;
	int listenfd;
	socklen_t clilen;

	struct sockaddr_in cliaddr, servaddr;
	pid_t childpid;
	int err;
	XDR xdrs;

	err = daemon(0, 0);

	if (err)
	{

		LIBPERROR("Error in daemon() %d:%s", errno, strerror(errno));
		return 1;
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd < 0)
	{
		LIBPERROR("Error in socket() %d:%s", errno, strerror(errno));
		return 1;
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(RMEDRIVER_SVC_PORT);

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
	{
		LIBPERROR("Error in bind() %d:%s", errno, strerror(errno));
		return 1;
	}

	if (listen(listenfd, RMEDRIVER_LISTEN_QUEUE))
	{
		LIBPERROR("Error in listen() %d:%s", errno, strerror(errno));
		return 1;
	}

	if (signal(SIGCHLD, sig_chld) == SIG_ERR)
	{
		LIBPERROR("Error in signal() %d:%s", errno, strerror(errno));
		return 1;
	}

	pmap_unset(RMEDRIVER_PROG, RMEDRIVER_VERS);

	if (!pmap_set(RMEDRIVER_PROG, RMEDRIVER_VERS, IPPROTO_TCP, RMEDRIVER_SVC_PORT))
	{
		LIBPERROR("Error in pmap_set() %d:%s", errno, strerror(errno));
		return 1;
	}

	while (1)
	{
		clilen = sizeof(cliaddr);

		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

		if (connfd < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				LIBPERROR("Error in accept() %d:%s", errno, strerror(errno));
				return 1;
			}
		}

		childpid = fork();

		if (childpid < 0)
		{
			LIBPERROR("Error in fork() %d:%s", errno, strerror(errno));
			return 1;
		}
		else if (childpid == 0)
		{
			if (close(listenfd))
			{
				LIBPERROR("Error in close(listenfd) %d:%s", errno, strerror(errno));
				return 1;
			}

			xdrrec_create(&xdrs, 0, 0, (char *)connfd, readtcp, writetcp);

			while (1)
			{ // Serve incoming requests
				err = dispatch(&xdrs);

				if (err)
					break;
			}

			return 1;
		}

		if (close(connfd))
		{ // Release the connection
			LIBPERROR("Error in close(connfd) %d:%s", errno, strerror(errno));
			return 1;
		}
	}
}
