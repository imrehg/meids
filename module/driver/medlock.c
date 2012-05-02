/**
 * @file medlock.c
 *
 * @brief Implements the device lock class.
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


#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/fs.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_spin_lock.h"

# include "mesubdevice.h"
# include "meslist.h"
# include "medlock.h"
# include "meslock.h"

/**
 * @brief Initializate device lock instance
 * @param dlock Pointer to device lock instance.
 * @return ME_ERRNO_SUCCESS
 */
void me_dlock_init(me_dlock_t* dlock)
{
	dlock->filep = NULL;
	dlock->count = 0;
	ME_INIT_LOCK(&dlock->dlock);
}

/**
 * @brief Deinitializate device lock instance
 * @param dlock Pointer to device lock instance.
 */
void me_dlock_deinit(me_dlock_t* dlock)
{
	dlock->filep = NULL;
	dlock->count = 0;
}

/**
 * @brief Init device entry spinlock.
 * @param dlock Pointer to device lock instance.
 * @param filep Process unique identification.
 * @return On success ME_ERRNO_SUCCESS.
 * @return When device locked by another thread ME_ERRNO_LOCKED.
 */
int me_dlock_enter(me_dlock_t* dlock, struct file* filep)
{
	int err = ME_ERRNO_SUCCESS;

	ME_LOCK(&dlock->dlock);
		if ((dlock->filep != NULL) && (dlock->filep != filep))
		{
			PERROR("Device is locked by another process.\n");
			err = ME_ERRNO_LOCKED;
		}
		else
		{
			dlock->count++;
		}
	ME_UNLOCK(&dlock->dlock);

	return err;
}

/**
 * @brief Deinit device entry spinlock.
 * @param dlock Pointer to device lock instance.
 * @param filep NOT IN USE.
 * @return ME_ERRNO_SUCCESS.
 */
int me_dlock_exit(me_dlock_t* dlock, struct file* filep)
{
	ME_LOCK(&dlock->dlock);
		dlock->count--;
	ME_UNLOCK(&dlock->dlock);

	if (dlock->count < 0)
	{
		PERROR_CRITICAL("me_dlock_exit() called WITOUT previous me_dlock_enter()\n");
		dlock->count = 0;
	}

	return ME_ERRNO_SUCCESS;
}

/**
 * @brief Lock subdevice.
 * @param dlock Pointer to device lock instance.
 * @param slist The subdevice list of the device.
 * @param filep Process unique identification.
 * @param lock	Action: SET, RELEASE or CHECK
 * @return On success ME_ERRNO_SUCCESS.
 * @return When device in use ME_ERRNO_USED.
 * @return When device locked by another thread ME_ERRNO_LOCKED.
 * @return When wrong action code ME_ERRNO_INVALID_LOCK.
 */
int me_dlock_lock(me_dlock_t* dlock, me_slist_t* slist, struct file* filep, int lock, int flags)
{
	struct list_head* pos;
	me_subdevice_t* subdevice;
	int err = ME_ERRNO_SUCCESS;

	ME_LOCK(&dlock->dlock);
		if (!(flags & ME_LOCK_FORCE) || (lock == ME_LOCK_CHECK))
		{
			if ((dlock->filep != NULL) && (dlock->filep != filep))
			{
				PERROR("Device is locked by another process.\n");
				err = ME_ERRNO_LOCKED;
			}
		}

		if (!err)
		{
			switch (lock)
			{
				case ME_LOCK_RELEASE:
					if (dlock->count)
					{
						PERROR("Device is used by another process.\n");
						err = ME_ERRNO_USED;
						break;
					}

					if (!(flags & ME_LOCK_FORCE))
					{
						// Check subdevices.
						list_for_each(pos, &slist->head)
						{
							subdevice = list_entry(pos, me_subdevice_t, list);
							err = me_slock_lock(&subdevice->lock, filep, ME_LOCK_CHECK, ME_NO_FLAGS);
							if (err)
								break;
						}
						if (err)
							break;
					}

					dlock->filep = NULL;

					// Unlock also subdevices
					if (!(flags & ME_LOCK_PRESERVE))
					{
						list_for_each(pos, &slist->head)
						{
							subdevice = list_entry(pos, me_subdevice_t, list);
							err = me_slock_lock(&subdevice->lock, filep, ME_LOCK_RELEASE, flags);
							if (err)
								break;
						}
					}
					break;

				case ME_LOCK_SET:
					if (dlock->count)
					{
						PERROR("Device is used by another process.\n");
						err = ME_ERRNO_USED;
						break;
					}

					if (!(flags & ME_LOCK_FORCE))
					{
						// Check subdevices.
						list_for_each(pos, &slist->head)
						{
							subdevice = list_entry(pos, me_subdevice_t, list);
							err = me_slock_lock(&subdevice->lock, filep, ME_LOCK_CHECK, ME_NO_FLAGS);
							if (err)
								break;
						}
						if (err)
							break;
					}

					dlock->filep = filep;
					break;

				case ME_LOCK_CHECK:
					if (!(flags & ME_LOCK_FORCE))
					{
						// Check subdevices.
						list_for_each(pos, &slist->head)
						{
							subdevice = list_entry(pos, me_subdevice_t, list);
							err = me_slock_lock(&subdevice->lock, filep, ME_LOCK_CHECK, ME_NO_FLAGS);
							if (err)
								break;
						}
						if (err)
							break;
					}
					break;

				default:
					PERROR("Invalid lock.\n");
					err = ME_ERRNO_INVALID_LOCK;
			}
		}
	ME_UNLOCK(&dlock->dlock);

	return err;
}
