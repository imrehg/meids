/**
 * @file meseg_buf.c
 *
 * @brief Segmented circular buffer.
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/fs.h>
# include <linux/slab.h>


# include <linux/delay.h>
# include <linux/sched.h>
# include <linux/workqueue.h>
# include <asm/uaccess.h>
# include <asm/msr.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"
# include "meeeprom.h"


# include "meseg_buf.h"

me_seg_buf_t* create_seg_buffer(const unsigned int number_segments, const unsigned int segment_size)
{
	unsigned int idx;
	me_seg_buf_t* buf;
	unsigned int err  = ME_ERRNO_SUCCESS;

	PDEBUG_BUF("executed.\n");

	buf = kzalloc(sizeof(me_seg_buf_t), GFP_KERNEL);
	PDEBUG_BUF("Creating buffer structure %u segments of %u bytes (%p/%lu)\n", number_segments, segment_size, buf, sizeof(me_seg_buf_t));
	if (buf)
	{
		buf->header.chunk_size = segment_size >> 1;
		buf->header.total_size = buf->header.chunk_size * number_segments;
		PDEBUG_BUF("Buffer size: %d values\n", buf->header.total_size);

		buf->buffers = kzalloc(sizeof(single_chunk_t) * number_segments, GFP_KERNEL);
		PDEBUG_BUF("Creating buffer %u segments of %u bytes (%p/%lu)\n", number_segments, segment_size, buf->buffers, sizeof(single_chunk_t) * number_segments);
		if (buf->buffers)
		{
			buf->chunks_count = number_segments;
			for (idx=0; idx<number_segments; ++idx)
			{
				buf->buffers[idx].segment = kzalloc((segment_size + 0x03) & (~0x03), GFP_KERNEL);
				PDEBUG_BUF("Creating segment %u (%p/%u)\n", idx, buf->buffers[idx].segment, (segment_size + 0x03) & (~0x03));
				if (!buf->buffers[idx].segment)
				{
					PERROR("Cann't get memmory for chunk %d.\n", idx);
					buf->chunks_count = idx;
					err = ME_ERRNO_INTERNAL;
					break;
				}
			}
		}
		else
		{
			err = ME_ERRNO_INTERNAL;
		}
	}
	else
	{
		PERROR("Cann't get memmory for  buffer table.\n");
	}

	if (err)
	{
		PERROR("Creation failed. Clearing all.\n");
		destroy_seg_buffer(&buf);
	}

	return buf;
}


void destroy_seg_buffer(me_seg_buf_t** buf_ptr)
{
	unsigned int idx;
	me_seg_buf_t* buf = *buf_ptr;

	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		return;
	}

	if ((buf->chunks_count) && (buf->buffers))
	{
		for (idx=0; idx<buf->chunks_count; ++idx)
		{
			if (buf->buffers[idx].segment)
			{
				PDEBUG_BUF("Removing segment %u (%p)\n", idx, buf->buffers[idx].segment);
				kfree(buf->buffers[idx].segment);
			}
			else
			{
				break;
			}
		}
		PDEBUG_BUF("Removing buffer structure (%p)\n", buf->buffers);
		kfree(buf->buffers);
	}

	PDEBUG_BUF("Removing buffer (%p)\n", buf);
	kfree(buf);
	*buf_ptr = NULL;
}

int inline me_seg_buf_get(me_seg_buf_t* const buf, uint16_t* const value)
{
	uint16_t* volatile addr;

	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		PERROR("buf == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!value)
	{
		PERROR("value == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!buf->header.values_count)
	{
		PERROR("ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW\n");
		*value = 0x0000;
		return ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
	}

	addr = buf->buffers[buf->header.tail.chunk].segment + buf->header.tail.offset;
	*value = *addr;
	PDEBUG_BUF("GET segment: %u(%p) offset: %u => 0x%04x\n", buf->header.tail.chunk, buf->buffers[buf->header.tail.chunk].segment, buf->header.tail.offset, *addr);
	++buf->header.tail.offset;
	if (buf->header.tail.offset == buf->header.chunk_size)
	{
		buf->header.tail.offset = 0;
		++buf->header.tail.chunk;
		if (buf->header.tail.chunk == buf->chunks_count)
		{
			buf->header.tail.chunk = 0;
		}
	}

	--buf->header.values_count;

	++buf->header.reads_count;
	return ME_ERRNO_SUCCESS;
}

int inline me_seg_buf_put(me_seg_buf_t* const buf, const uint16_t value)
{
	uint16_t* addr;

	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		PERROR("buf == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (buf->header.values_count == buf->header.total_size)
	{
		PERROR("ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW (%d values in buffer)\n", buf->header.values_count);
		return ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
	}

	PDEBUG_BUF("PUT segment: %u(%p) offset: %u <= 0x%04x\n", buf->header.head.chunk, buf->buffers[buf->header.head.chunk].segment, buf->header.head.offset, value);
	addr = buf->buffers[buf->header.head.chunk].segment + buf->header.head.offset;
	*addr = value;
	++buf->header.head.offset;
	if (buf->header.head.offset == buf->header.chunk_size)
	{
		buf->header.head.offset = 0;
		++buf->header.head.chunk;
		if (buf->header.head.chunk == buf->chunks_count)
		{
			buf->header.head.chunk = 0;
		}
	}

	++buf->header.values_count;

	++buf->header.writes_count;
	return ME_ERRNO_SUCCESS;
}

int inline me_seg_buf_unget(me_seg_buf_t* const buf)
{
	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		PERROR("buf == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (buf->header.values_count == 0)
	{
		PERROR("Empty buffer\n");
		return ME_ERRNO_INTERNAL;
	}

	if (buf->header.head.offset)
	{
		--buf->header.head.offset;
	}
	else
	{
		buf->header.head.offset = buf->header.chunk_size - 1;
		if (buf->header.head.chunk)
		{
			--buf->header.head.chunk;
		}
		else
		{
			buf->header.head.chunk = buf->chunks_count - 1;
		}
	}
	PDEBUG_BUF("UNGET segment: %u(%p) offset: %u <= 0x%04x\n", buf->header.head.chunk, buf->buffers[buf->header.head.chunk].segment, buf->header.head.offset, value);

	--buf->header.values_count;

	--buf->header.writes_count;

	return ME_ERRNO_SUCCESS;
}

int inline me_seg_buf_rotate(me_seg_buf_t* const buf, uint16_t* const value)
{
	uint16_t* addr;

	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		PERROR("buf == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!value)
	{
		PERROR("value == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!buf->header.values_count)
	{
		PERROR("ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW\n");
		*value = 0x0000;
		return ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
	}

	PDEBUG_BUF("ROTATE segment: %u(%p) offset: %u ->\n", buf->header.tail.chunk, buf->buffers[buf->header.tail.chunk].segment, buf->header.tail.offset);
	addr = buf->buffers[buf->header.tail.chunk].segment + buf->header.tail.offset;
	*value = *addr;
	++buf->header.tail.offset;
	if (buf->header.tail.offset == buf->header.chunk_size)
	{
		buf->header.tail.offset = 0;
		++buf->header.tail.chunk;
		if (buf->header.tail.chunk == buf->chunks_count)
		{
			buf->header.tail.chunk = 0;
		}
	}

	PDEBUG_BUF("segment: %u(%p) offset: %u <=> 0x%04x\n", buf->header.head.chunk, buf->buffers[buf->header.tail.chunk].segment, buf->header.head.offset, *value);
	addr = buf->buffers[buf->header.head.chunk].segment + buf->header.head.offset;
	*addr = *value;
	++buf->header.head.offset;
	if (buf->header.head.offset == buf->header.chunk_size)
	{
		buf->header.head.offset = 0;
		++buf->header.head.chunk;
		if (buf->header.head.chunk == buf->chunks_count)
		{
			buf->header.head.chunk = 0;
		}
	}

	return ME_ERRNO_SUCCESS;

}

int inline me_seg_buf_read(me_seg_buf_t* const buf, unsigned int pos, uint16_t* const value)
{
	uint32_t offset;
	uint64_t chunk;
	uint32_t chunk_size;
	uint16_t* addr;

	PDEBUG_BUF("executed.\n");

	if (!buf)
	{
		PERROR("buf == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!value)
	{
		PERROR("value == NULL\n");
		return ME_ERRNO_INVALID_POINTER;
	}

	if (!buf->header.values_count)
	{
		*value = 0x0000;
		PERROR("ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW\n");
		return ME_ERRNO_SOFTWARE_BUFFER_UNDERFLOW;
	}
	if (pos >= buf->header.total_size)
	{
		*value = 0x0000;
		PERROR("ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW\n");
		return ME_ERRNO_SOFTWARE_BUFFER_OVERFLOW;
	}

	chunk = pos;
	chunk_size = buf->header.chunk_size;
	offset = do_div(chunk, chunk_size);

	addr = buf->buffers[chunk].segment + offset;
	*value = *addr;
	PDEBUG_BUF("READ segment: %u(%p) offset: %u => 0x%04x\n", (unsigned int)chunk, buf->buffers[chunk].segment, offset, *addr);

	return ME_ERRNO_SUCCESS;
}
