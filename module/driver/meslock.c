/**
 * @file meslock.c
 *
 * @brief Implements the subdevice lock class.
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

# include "meslock.h"

/**
 * @brief Initializate subdevice lock instance
 * @param slock Pointer to subdevice lock instance.
 * @return ME_ERRNO_SUCCESS
 */
int me_slock_init(me_slock_t* slock)
{
	slock->filep = NULL;
	slock->count = 0;
	ME_INIT_LOCK(&slock->slock);

	return ME_ERRNO_SUCCESS;
}

/**
 * @brief Deinitializate subdevice lock instance
 * @param slock Pointer to subdevice lock instance.
 */
void me_slock_deinit(me_slock_t* slock)
{
	slock->filep = NULL;
	slock->count = 0;
}

/**
 * @brief Init subdevice entry spinlock.
 * @param slock Pointer to subdevice lock instance.
 * @param filep Process unique identification.
 * @return On success ME_ERRNO_SUCCESS.
 * @return When subdevice locked by another thread ME_ERRNO_LOCKED.
 */
int me_slock_enter(me_slock_t* slock, struct file* filep)
{
	int err = ME_ERRNO_SUCCESS;

	ME_LOCK(&slock->slock);
		if ((slock->filep != NULL) && (slock->filep != filep))
		{
			PERROR("Subdevice is locked by another process.\n");
			err = ME_ERRNO_LOCKED;
		}
		else
		{
			slock->count++;
		}
	ME_UNLOCK(&slock->slock);

	return err;
}

/**
 * @brief Deinit subdevice entry spinlock.
 * @param slock Pointer to subdevice lock instance.
 * @param filep NOT IN USE.
 * @return ME_ERRNO_SUCCESS.
 */
int me_slock_exit(me_slock_t* slock, struct file* filep)
{
	ME_LOCK(&slock->slock);
		slock->count--;

		if (slock->count < 0)
		{
			PERROR_CRITICAL("me_slock_exit() called WITHOUT previous me_slock_enter()\n");
			slock->count = 0;
		}
	ME_UNLOCK(&slock->slock);

	return ME_ERRNO_SUCCESS;
}

/**
 * @brief Lock subdevice.
 * @param slock Pointer to subdevice lock instance.
 * @param filep Process unique identification.
 * @param lock	Action: SET, RELEASE or CHECK
 * @return On success ME_ERRNO_SUCCESS.
 * @return When subdevice in use ME_ERRNO_USED.
 * @return When subdevice locked by another thread ME_ERRNO_LOCKED.
 * @return When wrong action code ME_ERRNO_INVALID_LOCK.
 */
int me_slock_lock(me_slock_t* slock, struct file* filep, int lock, int flag)
{
	int err = ME_ERRNO_SUCCESS;

	ME_LOCK(&slock->slock);
		if (!(flag & ME_LOCK_FORCE) || (lock == ME_LOCK_CHECK))
		{
			if ((slock->filep != NULL) && (slock->filep != filep))
			{
				PERROR("Subdevice is locked by another process.\n");
				err = ME_ERRNO_LOCKED;
			}
		}

		if (!err)
		{
			switch (lock)
			{
				case ME_LOCK_RELEASE:
					if (slock->count)
					{
						PERROR("Subdevice is used by another process.\n");
						err = ME_ERRNO_USED;
					}
					else
					{
						slock->filep = NULL;
					}
					break;

				case ME_LOCK_SET:
					if (slock->count)
					{
						PERROR("Subdevice is used by another process.\n");
						err = ME_ERRNO_USED;
					}
					else
					{
						slock->filep = filep;
					}
					break;

				case ME_LOCK_CHECK:
					break;

				default:
					err = ME_ERRNO_INVALID_LOCK;
					break;
			}
		}
	ME_UNLOCK(&slock->slock);

	return err;
}
