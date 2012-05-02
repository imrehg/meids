/**
 * @file meslist.c
 *
 * @brief Implements the subdevice list class.
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

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"

# include <linux/slab.h>

# include "meslist.h"

unsigned int me_slist_get_number_subdevices(me_slist_t* slist)
{
	PDEBUG_LIST("executed.\n");
	PDEBUG_LIST("List(?): %d.\n", slist->n);
	return slist->n;
}

int me_slist_query_number_subdevices(me_slist_t* slist, int* number)
{
	PDEBUG_LIST("executed.\n");
	*number = me_slist_get_number_subdevices(slist);
	return ME_ERRNO_SUCCESS;
}

int me_slist_query_number_subdevices_by_type(me_slist_t* slist, int type, int subtype, int* number)
{
	me_subdevice_t *pos;
	int s_type, s_subtype;
	int subdevice_count = 0;

	PDEBUG_LIST("executed.\n");

	list_for_each_entry(pos, &slist->head, list)
	{
		pos->me_subdevice_query_subdevice_type(
		    pos,
		    &s_type,
		    &s_subtype);

		if ((ME_SUBTYPE_ANY == subtype) || (s_subtype == subtype))
		{
			if (s_type == type)
			{
				subdevice_count++;
			}
		}
	}

	*number = subdevice_count;
	return ME_ERRNO_SUCCESS;
}

me_subdevice_t* me_slist_get_subdevice(me_slist_t* slist, unsigned int index)
{
	struct list_head *pos;
	me_subdevice_t* subdevice = NULL;
	unsigned int i = 0;

	PDEBUG_LIST("executed.\n");

	if (index >= slist->n)
	{
		PERROR("Index out of range.\n");
		return NULL;
	}

	list_for_each(pos, &slist->head)
	{
		if (i == index)
		{
			subdevice = list_entry(pos, me_subdevice_t, list);
			break;
		}

		++i;
	}

	return subdevice;
}

int me_slist_get_subdevice_by_type(me_slist_t* slist, unsigned int start_subdevice, int type, int subtype, int* subdevice)
{
	me_subdevice_t *pos;
	int s_type, s_subtype;
	unsigned int index = 0;

	PDEBUG_LIST("executed.\n");

	if (start_subdevice >= slist->n)
	{
		PERROR("size %d ask for %d.\n", slist->n, start_subdevice);
		PERROR("Start index out of range.\n");
		return ME_ERRNO_NOMORE_SUBDEVICE_TYPE;
	}

	list_for_each_entry(pos, &slist->head, list)
	{
		if (index < start_subdevice)
		{ // Go forward to start subdevice.
			++index;
			continue;
		}

		pos->me_subdevice_query_subdevice_type(
		    pos,
		    &s_type,
		    &s_subtype);

		if ((ME_SUBTYPE_ANY == subtype) || (s_subtype == subtype))
		{
			if (s_type == type)
				break;
		}
		++index;
	}

	if (index >= slist->n)
	{
		return ME_ERRNO_NOMORE_SUBDEVICE_TYPE;
	}

	*subdevice = index;

	return ME_ERRNO_SUCCESS;
}

void me_slist_add(me_slist_t* slist, void* local_dev, me_subdevice_t* subdevice)
{
			PDEBUG_LIST("executed.\n");
			subdevice->dev = local_dev;
			me_slist_add_subdevice_tail(slist, subdevice);
}

void me_slist_add_subdevice_tail(me_slist_t* slist, me_subdevice_t* subdevice)
{
	PDEBUG_LIST("executed.\n");

	list_add_tail(&subdevice->list, &slist->head);
	++slist->n;
	PDEBUG_LIST("List(+): %d.\n", slist->n);
}

me_subdevice_t* me_slist_del_subdevice_tail(me_slist_t* slist)
{

	struct list_head *last;
	me_subdevice_t* subdevice;

	PDEBUG_LIST("executed.\n");

	if (list_empty(&slist->head)) return NULL;

	last = slist->head.prev;

	subdevice = list_entry(last, me_subdevice_t, list);

	list_del(last);

	--slist->n;

	PDEBUG_LIST("List(-): %d.\n", slist->n);
	return subdevice;
}

void me_slist_init(me_slist_t* slist)
{
	PDEBUG_LIST("executed.\n");

	INIT_LIST_HEAD(&slist->head);
	slist->n = 0;
	PDEBUG_LIST("List(init): %d.\n", slist->n);
}

void me_slist_deinit(me_slist_t* slist)
{
	struct list_head *s;
	me_subdevice_t* subdevice;

	PDEBUG_LIST("executed.\n");
	PDEBUG_LIST("slist=%p.\n", slist);
	PDEBUG_LIST("&slist->head=%p.\n", &slist->head);

	while (!list_empty(&slist->head))
	{
		s = slist->head.next;
		PDEBUG_LIST("s=%p.\n", s);
		subdevice = list_entry(s, me_subdevice_t, list);
		PDEBUG_LIST("subdevice=%p.\n", subdevice);
		if (subdevice)
		{
			subdevice->me_subdevice_destructor(subdevice);
		}
		list_del(s);
		if (subdevice)
		{
			kfree(subdevice);
			subdevice = NULL;
		}
	}

	slist->n = 0;
	PDEBUG_LIST("List(deinit): %d.\n", slist->n);
}
