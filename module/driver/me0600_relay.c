/**
 * @file me0600_relay.c
 *
 * @brief ME-630 relay subdevice instance.
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

# include "me0600_relay_reg.h"
# include "me0600_relay.h"

int me0600_relay_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me0600_relay_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
											int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me0600_relay_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me0600_relay_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me0600_relay_query_number_channels(me_subdevice_t* subdevice, int* number);
int me0600_relay_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me0600_relay_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);


int me0600_relay_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me0600_relay_subdevice_t* instance;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (me0600_relay_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			me_writeb(instance->base.dev, 0x0, instance->port_0_reg);
			me_writeb(instance->base.dev, 0x0, instance->port_1_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me0600_relay_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
											int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me0600_relay_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me0600_relay_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
		case ME_IO_SINGLE_CONFIG_DIO_WORD:
			break;

		default:
			PERROR("Invalid flags specified. Should be ME_IO_SINGLE_CONFIG_DIO_WORD.\n");
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
		PERROR("Invalid reference. Must be ME_REF_NONE. Must be 0.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if (single_config != ME_SINGLE_CONFIG_DIO_OUTPUT)
	{
		PERROR("Invalid word direction specified. Must be ME_SINGLE_CONFIG_DIO_OUTPUT.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel specified.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me0600_relay_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me0600_relay_subdevice_t* instance;
	uint8_t tmp;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me0600_relay_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 16))
			{
				PERROR("Invalid bit number specified. Must be between 0 and 15.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel >= 2))
			{
				PERROR("Invalid bit number specified. Must be 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
		case ME_IO_SINGLE_TYPE_NO_FLAGS:
			if (channel)
			{
				PERROR("Invalid byte number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					if ((channel >= 0) && (channel < 8))
					{
						me_readb(instance->base.dev, &tmp, instance->port_0_reg);
						*value = tmp & (0x1 << channel);
					}
					else //if ((channel >= 8) && (channel < 16))
					{
						me_readb(instance->base.dev, &tmp, instance->port_1_reg);
						*value = tmp & (0x1 << (channel - 8));
					}
					break;

				case ME_IO_SINGLE_TYPE_DIO_BYTE:
					if (channel == 0)
					{
						me_readb(instance->base.dev, &tmp, instance->port_0_reg);
						*value = tmp;
					}
					else //if (channel == 1)
					{
						me_readb(instance->base.dev, &tmp, instance->port_1_reg);
						*value = tmp;
					}
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_WORD:
					me_readb(instance->base.dev, &tmp, instance->port_1_reg);
					*value = (int)tmp << 8;
					me_readb(instance->base.dev, &tmp, instance->port_0_reg);
					*value |= (int)tmp;
					break;
			}
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0600_relay_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me0600_relay_subdevice_t* instance;
	uint8_t tmp;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me0600_relay_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 16))
			{
				PERROR("Invalid bit number specified. Must be between 0 and 15.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel >= 2))
			{
				PERROR("Invalid bit number specified. Must be 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
		case ME_IO_SINGLE_TYPE_NO_FLAGS:
			if (channel)
			{
				PERROR("Invalid byte number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					if ((channel >= 0) && (channel < 8))
					{
						me_readb(instance->base.dev, &tmp, instance->port_0_reg);
						tmp = value ? (tmp | (0x1 << channel)) : (tmp & ~(0x1 << channel));
						me_writeb(instance->base.dev, tmp, instance->port_0_reg);
					}
					else //if ((channel >= 8) && (channel < 16))
					{
						me_readb(instance->base.dev, &tmp, instance->port_1_reg);
						tmp = value ? (tmp | (0x1 << (channel - 8))) : (tmp & ~(0x1 << (channel - 8)));
						me_writeb(instance->base.dev, tmp, instance->port_1_reg);
					}
					break;

				case ME_IO_SINGLE_TYPE_DIO_BYTE:
					if (channel == 0)
					{
						me_writeb(instance->base.dev, value, instance->port_0_reg);
					}
					else //if (channel == 1)
					{
						me_writeb(instance->base.dev, value, instance->port_1_reg);
					}
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_WORD:
					me_writeb(instance->base.dev, value & 0xFF, instance->port_0_reg);
					me_writeb(instance->base.dev, value >> 8, instance->port_1_reg);
					break;
			}
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0600_relay_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed.\n");

	*number = 16;

	return ME_ERRNO_SUCCESS;
}

int me0600_relay_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me0600_relay_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = 0;

	return ME_ERRNO_SUCCESS;
}

me0600_relay_subdevice_t* me0600_relay_constr(void* reg_base, unsigned int idx)
{
	me0600_relay_subdevice_t* subdevice;
	int err;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me0600_relay_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		return NULL;
	}

	// Initialize subdevice base class.
	err = me_subdevice_init(&subdevice->base);
	if (err)
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		return NULL;
	}

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize registers.
	subdevice->port_0_reg = reg_base + ME0600_RELAIS_0_REG;
	subdevice->port_1_reg = reg_base + ME0600_RELAIS_1_REG;

	// Overload base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me0600_relay_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me0600_relay_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me0600_relay_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me0600_relay_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me0600_relay_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me0600_relay_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me0600_relay_query_subdevice_caps;

	return subdevice;
}
