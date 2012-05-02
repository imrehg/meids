/**
 * @file meseg_buf.h
 *
 * @brief Segmented circular buffer implementation.
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

#ifdef __KERNEL__

# ifndef _MESEG_BUF_H_
#  define _MESEG_BUF_H_


typedef struct
{
	uint16_t* segment;
} single_chunk_t;

typedef struct
{
	unsigned int volatile chunk;
	unsigned int volatile offset;
} chunk_addr_t;

typedef struct
{
	//buffer size in number of values
	unsigned int volatile total_size;
// 	//single chunk size in number of values
	unsigned int volatile chunk_size;

	// number of values in buffer
	unsigned int volatile values_count;

	unsigned int volatile reads_count;
	unsigned int volatile writes_count;

	// begin and end of data in buffer
	chunk_addr_t volatile head;
	chunk_addr_t volatile tail;
} me_seg_buf_header_t;

typedef struct
{
	me_seg_buf_header_t volatile header;
	//number of chunk size in number of values
	unsigned int chunks_count;
	single_chunk_t* buffers;
} me_seg_buf_t;


/// How many values is in buffer.
static unsigned int inline me_seg_buf_size(me_seg_buf_t* const buf)
{
	return buf->header.total_size;
}

/// How many values is in buffer.
static unsigned int inline me_seg_buf_values(me_seg_buf_t* const buf)
{
	return buf->header.values_count;
}

/// How many space left.
static unsigned int inline me_seg_buf_space(me_seg_buf_t* const buf)
{
	return buf->header.total_size - buf->header.values_count;
}

static void inline me_seg_buf_reset(me_seg_buf_t* const buf)
{
	buf->header.head.chunk = 0;
	buf->header.head.offset = 0;
	buf->header.tail.chunk = 0;
	buf->header.tail.offset = 0;
	buf->header.values_count = 0;

	buf->header.reads_count = 0;
	buf->header.writes_count = 0;
}


/// Create buffer
/// segment_size - size of single chunk in bytes
me_seg_buf_t* create_seg_buffer(const unsigned int number_segments, const unsigned int segment_size);
/// Destroy buffer
void destroy_seg_buffer(me_seg_buf_t** buf_ptr);

/// Remove value from buffer
int inline me_seg_buf_get(me_seg_buf_t* const buf, uint16_t* const value);
/// Add value to buffer
int inline me_seg_buf_put(me_seg_buf_t* const buf, const uint16_t value);
/// Remove last value from buffer.
int inline me_seg_buf_unget(me_seg_buf_t* const buf);
/// Wraparound. Read value and move it from head to tail
int inline me_seg_buf_rotate(me_seg_buf_t* const buf, uint16_t* const value);

int inline me_seg_buf_read(me_seg_buf_t* const buf, unsigned int pos, uint16_t* const value);

# endif	//_MESEG_BUF_H_
#endif	//__KERNEL__
