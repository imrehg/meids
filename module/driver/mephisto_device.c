/**
 * @file mephisto_device.c
 *
 * @brief MephistoScope device class implementation.
 * @note Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <asm/uaccess.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_common.h"
# include "me_internal.h"

# include "mesubdevice.h"
# include "mephisto_dio.h"
# include "mephisto_ai.h"

# include "mephisto_device.h"

# define ME_MODULE_NAME		MEPHISTO_NAME
# define ME_MODULE_VERSION	MEPHISTO_VERSION

static void mephisto_get_device_info(mephisto_device_t* device, uint16_t device_id);
static int mephisto_reset_device(me_device_t* device, struct file* filep, int flags);
static int mephisto_device_postinit(me_device_t* device, struct file* filep);

me_device_t* mephisto_constr(mephisto_usb_device_t* device, me_device_t* instance)
{

	mephisto_device_t* mephisto_device;
	me_subdevice_t* subdevice;

	PDEBUG("executed.\n");

	if (!instance)
	{
		// Allocate structure for device instance.
		mephisto_device = kzalloc(sizeof(mephisto_device_t), GFP_KERNEL);
		if (!mephisto_device)
		{
			PERROR("Cannot get memory for MephistoScope device instance.\n");
			return NULL;
		}

		// Initialize spin locks.
		init_MUTEX(&mephisto_device->device_semaphore);

		// Set constans.
		mephisto_get_device_info(mephisto_device, device->device);

		// Initialize base class structure.
		if (me_device_init(&mephisto_device->base, device))
		{
			PERROR("Cannot initialize device base class.\n");
			goto ERROR;
		}

		/// Create subdevice instances.
		// DIO
		subdevice = (me_subdevice_t *) mephisto_dio_constr(0, &mephisto_device->AI_status, &mephisto_device->device_semaphore);
		if (!subdevice)
		{
			PERROR("Cannot get memory for DIO0 subdevice.\n");
			goto ERROR;
		}

		me_slist_add(&mephisto_device->base.slist, (void *)&mephisto_device->base.bus.local_dev, subdevice);

		// AI
		subdevice = (me_subdevice_t *) mephisto_ai_constr(0, &mephisto_device->AI_status, &mephisto_device->device_semaphore);
		if (!subdevice)
		{
			PERROR("Cannot get memory for AI%d subdevice.\n", 0);
			goto ERROR;
		}

		me_slist_add(&mephisto_device->base.slist, (void *)&mephisto_device->base.bus.local_dev, subdevice);
	}
	else
	{
		mephisto_device = (mephisto_device_t *)instance;
		me_device_reinit(instance, device);
	}

	mephisto_device->base.me_device_io_reset_device = mephisto_reset_device;
	mephisto_device->base.me_device_postinit = mephisto_device_postinit;

	return (me_device_t *) mephisto_device;

ERROR:
	PERROR_CRITICAL("Can not create instance of %s\n", ME_MODULE_NAME);
	if(mephisto_device)
	{
		me_device_disconnect((me_device_t *)mephisto_device);
		if (mephisto_device->base.me_device_destructor)
		{
			mephisto_device->base.me_device_destructor((me_device_t *)mephisto_device);
		}
		kfree(mephisto_device);
		mephisto_device = NULL;
	}
	return (me_device_t *) mephisto_device;
}

static void mephisto_get_device_info(mephisto_device_t* device, uint16_t device_id)
{
	device->base.info.device_version =		ME_MODULE_VERSION;
	device->base.info.driver_name =			ME_MODULE_NAME;

	device->base.info.device_name = 		MEPHISTO_NAME;
	device->base.info.device_description =	MEPHISTO_DESCRIPTION;
}

static int mephisto_reset_device(me_device_t* device, struct file* filep, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	me_subdevice_t* s;
	int i;
	MEPHISTO_modes_tu ret_status;

	PDEBUG("executed.\n");

	// Enter device.
	err = me_dlock_enter(&device->dlock, filep);
	if (err)
	{
		PERROR("Cannot enter device.\n");
		return err;
	}

	// Check subdevice locks.
	if (!(flags & ME_IO_RESET_DEVICE_UNPROTECTED))
	{
		err = me_dlock_lock(&device->dlock, &device->slist, filep, ME_LOCK_CHECK, ME_NO_FLAGS);
		if(err)
		{
			PERROR("Cannot reset device. Something is locked.\n");
			return err;
		}
	}

	// Reset device
	err = mephisto_cmd(&device->bus.local_dev, MEPHISTO_CMD_Restart, NULL, 0, &ret_status, 1);
	if (!err)
	{
		// Reset every subdevice in list.
		for (i = 0; i < me_slist_get_number_subdevices(&device->slist); i++)
		{
			s = me_slist_get_subdevice(&device->slist, i);
			err = s->me_subdevice_io_reset_subdevice(s, filep, flags & ~ME_IO_RESET_DEVICE_UNPROTECTED);

			if (err && (err != ME_ERRNO_LOCKED))
			{
				PERROR("Cannot reset %d subdevice.\n", i);
			}
		}

		// Reset apply only for not blocked subdevices. err == ME_ERRNO_LOCKED is not an error.
		if (err == ME_ERRNO_LOCKED)
			err = ME_ERRNO_SUCCESS;
	}

	// Exit device.
	me_dlock_exit(&device->dlock, filep);

	return err;
}

static int mephisto_device_postinit(me_device_t* device, struct file* filep)
{
	unsigned char productID[36];
	MEPHISTO_modes_tu send_arg;

	memset(productID, 0, 36);
	memset(&send_arg, '?', 4);

	if (!mephisto_cmd(&device->bus.local_dev, MEPHISTO_CMD_Inquiry, &send_arg, 1, (MEPHISTO_modes_tu*)productID, 8))
	{
		PLOG("MephistoScope product ID: %s\n", productID);
	}
	else
	{
		PERROR("Inquiry can not be read.\n");
	}

	return mephisto_reset_device(device, filep, ME_NO_FLAGS);
}

MODULE_VERSION(__stringify(ME_MODULE_VERSION));
