/**
 * @file me_dlist.h
 *
 * @brief Provides the device list class.
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

# ifndef _ME_DLIST_H_
#  define _ME_DLIST_H_

#  include <linux/list.h>

#  include "medevice.h"

/**
 * @brief The device list container.
 */
typedef struct //me_dlist
{
	struct list_head head;		/**< The head of the internal list. */
	unsigned int n;				/**< The number of devices in the list. */
} me_dlist_t;


/**
 * @brief Queries the number of devices currently inside the list.
 *
 * @param dlist The device list to query.
 * @param[out] number The number of devices.
 *
 * @return ME-iDS error code.
 */
// int me_dlist_query_number_devices(struct me_dlist *dlist, int *number);
int me_dlist_query_number_devices(me_dlist_t *dlist, int *number);


/**
 * @brief Returns the number of devices currently inside the list.
 *
 * @param dlist The device list to query.
 *
 * @return The number of devices in the list.
 */
// unsigned int me_dlist_get_number_devices(struct me_dlist *dlist);
unsigned int me_dlist_get_number_devices(me_dlist_t *dlist);


/**
 * @brief Get a device by index.
 *
 * @param dlist The device list to query.
 * @param index The index of the device to get in the list.
 *
 * @return The device at index if available.\n
 *         NULL if the index is out of range.
 */
// me_device_t *me_dlist_get_device(struct me_dlist *dlist, unsigned int index);
me_device_t *me_dlist_get_device(me_dlist_t *dlist, unsigned int index);


/**
 * @brief Adds a device to the tail of the list.
 *
 * @param dlist The device list to add a device to.
 * @param device The device to add to the list.
 */
// void me_dlist_add_device_tail(struct me_dlist *dlist, me_device_t *device);
void me_dlist_add_device_tail(me_dlist_t *dlist, me_device_t *device);


/**
 * @brief Removes a device from the tail of the list.
 *
 * @param dlist The device list.
 *
 * @return Pointer to the removed subdeivce.\n
 *         NULL in cases where the list was empty.
 */
// me_device_t *me_dlist_del_device_tail(struct me_dlist *dlist);
me_device_t *me_dlist_del_device_tail(me_dlist_t *dlist);


/**
 * @brief Initializes a device list structure.
 *
 * @param lock The device list structure to initialize.
 * @return 0 on success.
 */
// int me_dlist_init(me_dlist_t *dlist);
int me_dlist_init(me_dlist_t *dlist);


/**
 * @brief Deinitializes a device list structure and destructs every device in it.
 *
 * @param dlist The device list structure to deinitialize.
 * @return 0 on success.
 */
// void me_dlist_deinit(me_dlist_t *dlist);
void me_dlist_deinit(me_dlist_t *dlist);


# endif
#endif
