/**
 * @file medlock.h
 *
 * @brief Provides the device lock class.
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

#ifdef __KERNEL__

# ifndef _MEDLOCK_H_
#  define _MEDLOCK_H_

#  include <linux/fs.h>

#  include "melock_defines.h"

/**
 * @brief The device lock class.
 */
typedef struct //me_dlock
{
	struct file* filep;		/**< Pointer to file structure holding the device. */
	int count;				/**< Number of tasks which are inside the device. */
	me_lock_t dlock;	/**< Spin lock protecting the attributes from concurrent access. */
} me_dlock_t;

/**
 * @brief Tries to enter a device.
 *
 * @param dlock The device lock instance.
 * @param filep The file structure identifying the calling process.
 *
 * @return 0 on success.
 */
int me_dlock_enter(me_dlock_t* dlock, struct file* filep);

/**
 * @brief Exits a device.
 *
 * @param dlock The device lock instance.
 * @param filep The file structure identifying the calling process.
 *
 * @return 0 on success.
 */
int me_dlock_exit(me_dlock_t* dlock, struct file* filep);

/**
 * @brief Tries to perform a locking action on a device.
 *
 * @param dlock The device lock instance.
 * @param slist The subdevice list of the device.
 * @param filep The file structure identifying the calling process.
 * @param The action to be done.
 * @param flags Flags from user space.
 *
 * @return 0 on success.
 */
int me_dlock_lock(me_dlock_t* dlock, me_slist_t* slist, struct file* filep, int lock, int flags);

/**
 * @brief Initializes a lock structure.
 *
 * @param dlock The lock structure to initialize.
 */
void me_dlock_init(me_dlock_t* dlock);

/**
 * @brief Deinitializes a lock structure.
 *
 * @param dlock The lock structure to deinitialize.
 * @return 0 on success.
 */
void me_dlock_deinit(me_dlock_t* dlock);

# endif
#endif
