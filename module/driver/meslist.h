/**
 * @file me_slist.h
 *
 * @brief Provides the subdevice list class.
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

# ifndef _ME_SLIST_H_
#  define _ME_SLIST_H_

#  include <linux/list.h>

#  include "mesubdevice.h"

/**
 * @brief The subdevice list container.
 */
typedef struct //me_slist
{
	struct list_head head;		/**< The head of the internal list. */
	unsigned int n;				/**< The number of subdevices in the list. */
} me_slist_t;

/**
 * @brief Queries the number of subdevices currently inside the list.
 *
 * @param slist The subdevice list to query.
 * @param[out] number The number of subdevices on the device.
 *
 * @return ME-iDS error code.
 */
int me_slist_query_number_subdevices(me_slist_t* slist, int *number);

/**
 * @brief Queries the number of subdevices of provided type currently inside the list.
 *
 * @param slist The subdevice list to query.
 * @param type The type of the subdevice to query.
 * @param subtype The subtype of the subdevice to query.
 * @param[out] number The number of subdevices on the device.
 *
 * @return ME-iDS error code.
 */
int me_slist_query_number_subdevices_by_type(me_slist_t* slist, int type, int subtype, int *number);

/**
 * @brief Returns the number of subdevices currently inside the list.
 *
 * @param slist The subdevice list to query.
 *
 * @return The number of subdevices in the list.
 */
unsigned int me_slist_get_number_subdevices(me_slist_t* slist);

/**
 * @brief Get a subdevice by index.
 *
 * @param slist The subdevice list to query.
 * @param index The index of the subdevice to get in the list.
 *
 * @return The subdevice at index if available.\n
 *         NULL if the index is out of range.
 */
me_subdevice_t* me_slist_get_subdevice(me_slist_t* slist, unsigned int index);

/**
 * @brief Get a subdevice index by type and subtype.
 *
 * @param slist The subdevice list to query.
 * @param start_subdevice The subdevice index at which the start shall begin.
 * @param type The type of the subdevice to query.
 * @param subtype The subtype of the subdevice to query.
 * @param[out] subdevice On success this parameter returns the index of the subdevice matching the requested type.
 *
 * @return ME_ERRNO_SUCCESS on success.
 */
int me_slist_get_subdevice_by_type(me_slist_t* slist, unsigned int start_subdevice, int type, int subtype, int* subdevice);

/**
 * @brief Adds a subdevice to the list.
 *
 * @param me_device The device context.
 * @param subdevice The subdevice to add to the list.
 */
void me_slist_add(me_slist_t* slist, void* local_dev, me_subdevice_t* subdevice);

/**
 * @brief Adds a subdevice to the tail of the list.
 *
 * @param slist The subdevice list to add a subdevice to.
 * @param subdevice The subdevice to add to the list.
 */
void me_slist_add_subdevice_tail(me_slist_t* slist, me_subdevice_t* subdevice);

/**
 * @brief Removes a subdevice from the tail of the list.
 *
 * @param slist The subdevice list.
 *
 * @return Pointer to the removed subdeivce.\n
 *         NULL in cases where the list was empty.
 */
me_subdevice_t* me_slist_del_subdevice_tail(me_slist_t* slist);

/**
 * @brief Initializes a subdevice list structure.
 *
 * @param lock The subdevice list structure to initialize.
 */
void me_slist_init(me_slist_t* slist);

/**
 * @brief Deinitializes a subdevice list structure and destructs every subdevice in it.
 *
 * @param slist The subdevice list structure to deinitialize.
 * @return 0 on success.
 */
void me_slist_deinit(me_slist_t* slist);

// #  if defined(ME_USB)
// int me_slist_check_active_irq_urb(me_slist_t* slist);
// #  endif

# endif
#endif
