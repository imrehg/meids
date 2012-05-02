/**
 * @file me1000_dio.c
 *
 * @brief TheME-1000 DIO subdevice instance.
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
# include <linux/slab.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

#include "me1000_dio_reg.h"
#include "me1000_dio.h"

int me1000_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me1000_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me1000_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me1000_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me1000_dio_query_number_channels(me_subdevice_t* subdevice, int* number);
int me1000_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me1000_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);


int me1000_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me1000_dio_subdevice_t* instance;
	uint32_t tmp;

	instance = (me1000_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
				tmp &= ~(0x1 << instance->base.idx);
				me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);

			me_writel(instance->base.dev, 0x00000000, instance->port_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me1000_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me1000_dio_subdevice_t* instance;
	uint32_t mode;

	instance = (me1000_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
		case ME_IO_SINGLE_CONFIG_DIO_DWORD:
			break;

		default:
			PERROR("Invalid flags. Should be ME_IO_SINGLE_CONFIG_DIO_DWORD.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	if (trig_edge)
	{
		PERROR("Invalid trigger edge. Must be ME_TRIG_EDGE_NONE.\n");
		return ME_ERRNO_INVALID_TRIG_EDGE;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
			if (trig_chain != ME_TRIG_CHAN_NONE)
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;
		case ME_TRIG_TYPE_SW:
			if (trig_chain != ME_TRIG_CHAN_DEFAULT)
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;

		default:
			PERROR("Invalid trigger type. Should be ME_TRIG_TYPE_NONE.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	if (ref)
	{
		PERROR("Invalid reference. Must be ME_REF_NONE.\n");
		return ME_ERRNO_INVALID_REF;
	}

	switch (single_config)
	{
		case ME_SINGLE_CONFIG_DIO_INPUT:
		case ME_SINGLE_CONFIG_DIO_OUTPUT:
			break;

		default:
			PERROR("Invalid port direction.Must be ME_SINGLE_CONFIG_DIO_INPUT or ME_SINGLE_CONFIG_DIO_OUTPUT.\n");
			return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				me_readl(instance->base.dev, &mode, instance->ctrl_reg);
				switch (single_config)
				{
					case ME_SINGLE_CONFIG_DIO_INPUT:
						mode &= ~(0x1 << instance->base.idx);
						break;

					case ME_SINGLE_CONFIG_DIO_OUTPUT:
						mode |= 0x1 << instance->base.idx;
						break;
				}
				me_writel(instance->base.dev, mode, instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me1000_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me1000_dio_subdevice_t* instance;
	uint32_t tmp;

	instance = (me1000_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 32))
			{
				PERROR("Invalid bits number specified. Must be between 0 and 31.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel >= 4))
			{
				PERROR("Invalid bytes number specified. Must be between 0 and 3.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if ((channel < 0) || (channel >= 2))
			{
				PERROR("Invalid words number specified. Must be 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_DWORD:
			if (channel)
			{
				PERROR("Invalid dwords number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		me_readl(instance->base.dev, &tmp, instance->port_reg);
		switch (flags)
		{
			case ME_IO_SINGLE_TYPE_DIO_BIT:
				*value = tmp & (0x0001 << channel);
				break;

			case ME_IO_SINGLE_TYPE_DIO_BYTE:
				*value = (tmp >> (channel * 8)) & 0xFF;
				break;

			case ME_IO_SINGLE_TYPE_DIO_WORD:
				*value = (tmp >> (channel * 16)) & 0xFFFF;
				break;

			case ME_IO_SINGLE_TYPE_NO_FLAGS:
			case ME_IO_SINGLE_TYPE_DIO_DWORD:
				*value = tmp;
				break;
		}
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me1000_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me1000_dio_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint32_t mode;
	uint32_t tmp;

	instance = (me1000_dio_subdevice_t *)subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 32))
			{
				PERROR("Invalid bits number specified. Must be between 0 and 31.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel >= 4))
			{
				PERROR("Invalid bytes number specified. Must be between 0 and 3.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if ((channel < 0) || (channel >= 2))
			{
				PERROR("Invalid words number specified. Must be 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_DWORD:
			if (channel)
			{
				PERROR("Invalid dwords number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER
		ME_SUBDEVICE_LOCK;
			me_readl(instance->base.dev, &mode, instance->ctrl_reg);
			if (!(mode & (0x1 << instance->base.idx)))
			{
				PERROR("Port not in output mode.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
			}
			else
			{
				me_readl(instance->base.dev, &tmp, instance->port_reg);
				switch (flags)
				{
					case ME_IO_SINGLE_TYPE_DIO_BIT:
						tmp = value ? (tmp | (0x1 << channel)) : (tmp & ~(0x1 << channel));
						break;

					case ME_IO_SINGLE_TYPE_DIO_BYTE:
						tmp &= ~(0xFF << (channel * 8));
						tmp |= (value & 0xFF) << (channel * 8);
						break;

					case ME_IO_SINGLE_TYPE_DIO_WORD:
						tmp &= ~(0xFFFF << (channel * 16));
						tmp |= (value & 0xFFFF) << (channel * 16);
						break;

					case ME_IO_SINGLE_TYPE_NO_FLAGS:
					case ME_IO_SINGLE_TYPE_DIO_DWORD:
						tmp = value;
						break;
				}
				me_writel(instance->base.dev, tmp, instance->port_reg);
			}
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me1000_dio_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed idx=%d.\n", ((me1000_dio_subdevice_t *) subdevice)->base.idx);

	*number = ME1000_DIO_NUMBER_CHANNELS;

	return ME_ERRNO_SUCCESS;
}

int me1000_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed idx=%d.\n", ((me1000_dio_subdevice_t *) subdevice)->base.idx);

	*type = ME_TYPE_DIO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me1000_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed idx=%d.\n", ((me1000_dio_subdevice_t *) subdevice)->base.idx);

	*caps = ME_CAPS_DIO_DIR_DWORD;

	return ME_ERRNO_SUCCESS;
}

me1000_dio_subdevice_t* me1000_dio_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock)
{
	me1000_dio_subdevice_t *subdevice;

	PDEBUG("executed idx=%d.\n", idx);

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me1000_dio_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		return NULL;
	}

	// Initialize subdevice base class.
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		subdevice = NULL;
		return NULL;
	}

	// Initialize spin locks.
	subdevice->ctrl_reg_lock = ctrl_reg_lock;

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize registers.
	subdevice->ctrl_reg = reg_base + ME1000_PORT_MODE;
	subdevice->port_reg = reg_base + ME1000_PORT + (idx * ME1000_PORT_STEP);

	// Override base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me1000_dio_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me1000_dio_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me1000_dio_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me1000_dio_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me1000_dio_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me1000_dio_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me1000_dio_query_subdevice_caps;

	return subdevice;
}
