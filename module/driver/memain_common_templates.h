/**
 * @file memain_common_templates.h
 *
 * @brief Part of memain. Implementation of the common macros.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
 * Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define ME_IO_MULTIPLEX_TEMPLATE(NAME, TYPE, CALL, DEV_CALL, ARGS)	\
int CALL(struct file* filep, TYPE *arg){	\
	me_device_t* dev = NULL; \
	TYPE karg; \
	int err = ME_ERRNO_SUCCESS; \
	struct timespec ts_pre; \
	struct timespec ts_post; \
	struct timespec ts_exec; \
	\
	PDEBUG("executed.\n"); \
	\
	getnstimeofday(&ts_pre); \
	\
	if(copy_from_user(&karg, arg, sizeof(TYPE))){ \
		PERROR("Can't copy arguments to kernel space\n"); \
		return -EFAULT; \
	} \
	\
	ME_SPIN_LOCK(&me_lock);	\
		if ((me_filep != NULL) && (me_filep != filep))	\
		{	\
			PERROR("Driver is locked by another process.\n");	\
			karg.err_no = ME_ERRNO_LOCKED;	\
		}	\
		else	\
		{	\
			me_count++;	\
			ME_SPIN_UNLOCK(&me_lock);	\
			\
			down_read(&me_rwsem);	 \
				karg.err_no = get_medevice(karg.device, &dev);	\
				if (!karg.err_no)	\
				{	\
					karg.err_no = dev->DEV_CALL ARGS;	\
				}	\
			up_read(&me_rwsem);	\
			\
			ME_SPIN_LOCK(&me_lock);	\
			me_count--;	\
		}	\
	ME_SPIN_UNLOCK(&me_lock);	\
	\
	if(copy_to_user(arg, &karg, sizeof(TYPE))){ \
		PERROR("Can't copy arguments back to user space\n"); \
		err = -EFAULT; \
	} \
	\
	getnstimeofday(&ts_post); \
	ts_exec = timespec_sub(ts_post, ts_pre); \
	\
	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC); \
	\
	return err; \
}


#define ME_QUERY_MULTIPLEX_STR_TEMPLATE(NAME, TYPE, CALL, DEV_CALL, ARGS)	\
int CALL(struct file* filep, TYPE *arg){	\
	me_device_t* dev = NULL;	\
	char* msg = NULL;	\
	TYPE karg;	\
	int err = ME_ERRNO_SUCCESS; \
	\
	struct timespec ts_pre; \
	struct timespec ts_post; \
	struct timespec ts_exec; \
	\
	PDEBUG("executed.\n"); \
	\
	getnstimeofday(&ts_pre); \
	\
	if(copy_from_user(&karg, arg, sizeof(TYPE))){	\
		PERROR("Can't copy arguments to kernel space\n");	\
		return -EFAULT;	\
	}	\
	\
	down_read(&me_rwsem);	\
	\
		karg.err_no = get_medevice(karg.device, &dev);	\
		if (!karg.err_no)	\
		{	\
			karg.err_no = dev->DEV_CALL ARGS;	\
		}	\
		if(!karg.err_no)	\
		{	\
			if((strlen(msg) + 1) > karg.count)	\
			{	\
				PERROR("User buffer is to short.\n");	\
				karg.err_no = ME_ERRNO_USER_BUFFER_SIZE;	\
			}	\
				\
			if (copy_to_user(karg.name, msg, ((strlen(msg) + 1) > karg.count) ? karg.count : strlen(msg) + 1))	\
			{	\
				PERROR("Can't copy device name to user space\n");	\
				err = -EFAULT;	\
			}	\
		}	\
	\
 	up_read(&me_rwsem);	\
	\
	if (!err) \
	{ \
		if(copy_to_user(arg, &karg, sizeof(TYPE)))	\
		{	\
			PERROR("Can't copy query back to user space\n");	\
			err = -EFAULT;	\
		}	\
	} \
	\
	getnstimeofday(&ts_post); \
	ts_exec = timespec_sub(ts_post, ts_pre); \
	\
	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC); \
	\
	return err;	\
}



#define ME_QUERY_MULTIPLEX_TEMPLATE(NAME, TYPE, CALL, DEV_CALL, ARGS)	\
int CALL(struct file* filep, TYPE *arg){	\
	me_device_t* dev = NULL;	\
	TYPE karg;	\
	int err = ME_ERRNO_SUCCESS; \
		\
	struct timespec ts_pre; \
	struct timespec ts_post; \
	struct timespec ts_exec; \
	\
	PDEBUG("executed.\n"); \
	\
	getnstimeofday(&ts_pre); \
	 \
	if(copy_from_user(&karg, arg, sizeof(TYPE)))	\
	{	\
		PERROR("Can't copy arguments from user space\n");	\
		return -EFAULT;	\
	}	\
	\
	down_read(&me_rwsem);	\
		karg.err_no = get_medevice(karg.device, &dev);	\
		if (!karg.err_no)	\
		{	\
			karg.err_no = dev->DEV_CALL ARGS;	\
		}	\
	up_read(&me_rwsem);	\
	\
	if(copy_to_user(arg, &karg, sizeof(TYPE)))	\
	{	\
		PERROR("Can't copy arguments to user space\n");	\
		err = -EFAULT;	\
	}	\
	\
	getnstimeofday(&ts_post); \
	ts_exec = timespec_sub(ts_post, ts_pre); \
	\
	PEXECTIME("executed in %ld us\n", ts_exec.tv_nsec / NSEC_PER_USEC + ts_exec.tv_sec * USEC_PER_SEC); \
	\
	return err;	\
}
