/**
 * @file meslock.h
 *
 * @brief Provides the subdevice lock class.
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

# ifndef _MESLOCK_H_
#  define _MESLOCK_H_

# include <linux/fs.h>

# include "melock_defines.h"

/**
 * @brief The subdevice lock class.
 */
typedef struct //me_slock
{
	struct file* filep;		/**< Pointer to file structure holding the subdevice. */
	int count;				/**< Number of tasks which are inside the subdevice. */
	me_lock_t slock;	/**< Spin lock protecting the attributes from concurrent access. */
} me_slock_t;

/**
 * @brief Tries to enter a subdevice.
 *
 * @param slock The subdevice lock instance.
 * @param filep The file structure identifying the calling process.
 *
 * @return 0 on success.
 */
int me_slock_enter(me_slock_t* slock, struct file* filep);

/**
 * @brief Exits a subdevice.
 *
 * @param slock The subdevice lock instance.
 * @param filep The file structure identifying the calling process.
 *
 * @return 0 on success.
 */
int me_slock_exit(me_slock_t* slock, struct file* filep);

/**
 * @brief Tries to perform a locking action on a subdevice.
 *
 * @param slock The subdevice lock instance.
 * @param filep The file structure identifying the calling process.
 * @param lock  The action to be done.
 * @param flag  Flag for conditional beheviour.
 *
 * @return 0 on success.
 */
int me_slock_lock(me_slock_t* slock, struct file* filep, int lock, int flag);

/**
 * @brief Initializes a lock structure.
 *
 * @param slock The lock structure to initialize.
 * @return 0 on success.
 */
int me_slock_init(me_slock_t* slock);

/**
 * @brief Deinitializes a lock structure.
 *
 * @param slock The lock structure to deinitialize.
 * @return 0 on success.
 */
void me_slock_deinit(me_slock_t* slock);


#endif
#endif
