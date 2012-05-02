/**
 * @file me_dlist.c
 *
 * @brief Implements the device list class.
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


#include "me_debug.h"
#include "me_error.h"
#include "me_defines.h"

#include "medlist.h"


// int me_dlist_query_number_devices(struct me_dlist *dlist, int *number)
int me_dlist_query_number_devices(me_dlist_t *dlist, int *number)
{
	PDEBUG_LIST("executed.\n");
	*number = dlist->n;
	return ME_ERRNO_SUCCESS;
}


// unsigned int me_dlist_get_number_devices(struct me_dlist *dlist)
unsigned int me_dlist_get_number_devices(me_dlist_t *dlist)
{
	PDEBUG_LIST("executed.\n");
	return dlist->n;
}


// me_device_t *me_dlist_get_device(struct me_dlist *dlist, unsigned int index)
me_device_t *me_dlist_get_device(me_dlist_t *dlist, unsigned int index)
{

	struct list_head *pos;
	me_device_t *device = NULL;
	unsigned int i = 0;

	PDEBUG_LIST("executed.\n");

	if (index >= dlist->n)
	{
		PERROR("Index out of range.\n");
		return NULL;
	}

	list_for_each(pos, &dlist->head)
	{
		if (i == index)
		{
			device = list_entry(pos, me_device_t, list);
			break;
		}

		++i;
	}

	return device;
}


// void me_dlist_add_device_tail(struct me_dlist *dlist, me_device_t *device)
void me_dlist_add_device_tail(me_dlist_t *dlist, me_device_t *device)
{
	PDEBUG_LIST("executed.\n");

	list_add_tail(&device->list, &dlist->head);
	++dlist->n;
}


// me_device_t *me_dlist_del_device_tail(struct me_dlist *dlist)
me_device_t *me_dlist_del_device_tail(me_dlist_t *dlist)
{

	struct list_head *last;
	me_device_t *device;

	PDEBUG_LIST("executed.\n");

	if (list_empty(&dlist->head)) return NULL;

	last = dlist->head.prev;

	device = list_entry(last, me_device_t, list);

	list_del(last);

	--dlist->n;

	return device;
}


// int me_dlist_init(me_dlist_t *dlist)
int me_dlist_init(me_dlist_t *dlist)
{
	PDEBUG_LIST("executed.\n");

	INIT_LIST_HEAD(&dlist->head);
	dlist->n = 0;
	return 0;
}


// void me_dlist_deinit(me_dlist_t *dlist)
void me_dlist_deinit(me_dlist_t *dlist)
{

	struct list_head *s;
	me_device_t *device;

	PDEBUG_LIST("executed.\n");

	while (!list_empty(&dlist->head))
	{
		s = dlist->head.next;
		device = list_entry(s, me_device_t, list);
		device->me_device_destructor(device);

		list_del(s);
		if (device)
			kfree(device);
		device = NULL;

		--dlist->n;
	}

	PDEBUG_LIST("dlist->n=%d.\n", dlist->n);
	dlist->n = 0;
}
