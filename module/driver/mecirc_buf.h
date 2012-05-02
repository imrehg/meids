/**
 * @file mecirc_buf.h
 *
 * @brief Circular buffer implementation.
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

# ifndef _MECIRC_BUF_H_
#  define _MECIRC_BUF_H_

/// @note buf->mask = buf->count-1 = ME4600_AI_CIRC_BUF_COUNT-1

#   ifdef _CBUFF_32b_t
	// 32 bit
	typedef struct //me_circ_buf_32b
	{
		int volatile head;
		int volatile tail;
		unsigned int mask;	//buffor size-1 must be 2^n-1 to work
		uint32_t* buf;
	} me_circ_buf_t;
#   else
	// 16 bit
	typedef struct //me_circ_buf_16b
	{
		int volatile head;
		int volatile tail;
		unsigned int mask;	//buffor size-1 must be 2^n-1 to work
		uint16_t* buf;
	} me_circ_buf_t;
#   endif	//_CBUFF_32b_t

/// How many values is in buffer.
static int inline me_circ_buf_values(me_circ_buf_t *buf)
{
	return ((buf->head - buf->tail) & (buf->mask));
}

/// How many space left.
static int inline me_circ_buf_space(me_circ_buf_t *buf)
{
	return ((buf->tail - (buf->head + 1)) & (buf->mask));
}

/// How many values can be read from buffor in one chunck.
static int inline me_circ_buf_values_to_end(me_circ_buf_t *buf)
{
	return (buf->tail <= buf->head) ? (buf->head - buf->tail) : (buf->mask - buf->tail + 1);
}

/// How many values can be write to buffer in one chunck.
static int inline me_circ_buf_space_to_end(me_circ_buf_t *buf)
{
	return (buf->tail <= buf->head) ? (buf->mask - buf->head + 1) : (buf->tail - buf->head - 1);
}

# endif	//_MECIRC_BUF_H_
#endif	//__KERNEL__
