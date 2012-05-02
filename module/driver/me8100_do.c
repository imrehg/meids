/**
 * @file me8100_do.c
 *
 * @brief The ME-8100 digital output subdevice instance.
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

# include <linux/sched.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "me8100_reg.h"
# include "me8100_do_reg.h"
# include "me8100_do.h"

int me8100_do_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me8100_do_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me8100_do_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8100_do_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me8100_do_query_number_channels(me_subdevice_t* subdevice, int* number);
int me8100_do_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me8100_do_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);


int me8100_do_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me8100_do_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8100_do_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				// Reset all DO related bits(INTB_X are assigned to DI).
				*(instance->ctrl_reg_copy) &= ME8100_DIO_CTRL_BIT_INTB_0 | ME8100_DIO_CTRL_BIT_INTB_1;
				me_writew(instance->base.dev, *(instance->ctrl_reg_copy), instance->ctrl_reg);

			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
			me_writew(instance->base.dev, 0, instance->port_reg);
			instance->port_reg_mirror = 0;
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_do_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me8100_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me8100_do_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
		case ME_IO_SINGLE_CONFIG_DIO_WORD:
			break;

		default:
			PERROR("Invalid flags. Should be ME_IO_SINGLE_CONFIG_DIO_WORD.\n");
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
		case ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE:
		case ME_SINGLE_CONFIG_DIO_SINK:
		case ME_SINGLE_CONFIG_DIO_SOURCE:
			break;

		default:
			PERROR("Invalid port configuration specified. Must be ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE, ME_SINGLE_CONFIG_DIO_SINK or ME_SINGLE_CONFIG_DIO_SOURCE.\n");
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
				switch (single_config)
				{
					case ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE:
						*(instance->ctrl_reg_copy) &= ~(ME8100_DIO_CTRL_BIT_ENABLE_DIO);
						break;

					case ME_SINGLE_CONFIG_DIO_SINK:
						*(instance->ctrl_reg_copy) |= ME8100_DIO_CTRL_BIT_ENABLE_DIO;
						*(instance->ctrl_reg_copy) &= ~ME8100_DIO_CTRL_BIT_SOURCE;
						break;

					case ME_SINGLE_CONFIG_DIO_SOURCE:
						*(instance->ctrl_reg_copy) |= ME8100_DIO_CTRL_BIT_ENABLE_DIO | ME8100_DIO_CTRL_BIT_SOURCE;
						break;
				}
				me_writew(instance->base.dev, *(instance->ctrl_reg_copy), instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8100_do_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8100_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me8100_do_subdevice_t *) subdevice;

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

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if (channel)
			{
				PERROR("Invalid word number specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			err = ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		switch (flags)
		{
			case ME_IO_SINGLE_TYPE_DIO_BIT:
				*value = instance->port_reg_mirror & (0x1 << channel);
				break;

			case ME_IO_SINGLE_TYPE_DIO_BYTE:
				*value = (instance->port_reg_mirror >> (8 * channel)) & 0xFF;
				break;

			case ME_IO_SINGLE_TYPE_NO_FLAGS:
			case ME_IO_SINGLE_TYPE_DIO_WORD:
				*value = instance->port_reg_mirror;
				break;
		}
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8100_do_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me8100_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me8100_do_subdevice_t *) subdevice;

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

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if (channel)
			{
				PERROR("Invalid word number specified. Must be 0.\n");
				err = ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			err = ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					instance->port_reg_mirror = value ? (instance->port_reg_mirror | (0x1 << channel)) : (instance->port_reg_mirror & ~(0x1 << channel));
					break;

				case ME_IO_SINGLE_TYPE_DIO_BYTE:
					instance->port_reg_mirror &= ~(0x00FF << (8 * channel));
					instance->port_reg_mirror |= ((value & 0x00FF) << (8 * channel));
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_WORD:
					instance->port_reg_mirror = value;
					break;
			}

			me_writew(instance->base.dev, value, instance->port_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8100_do_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed.\n");

	*number = 16;

	return ME_ERRNO_SUCCESS;
}

int me8100_do_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me8100_do_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = me8100_DO_CAPS;	//ME_CAPS_DIO_SINK_SOURCE;

	return ME_ERRNO_SUCCESS;
}

me8100_do_subdevice_t* me8100_do_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, uint16_t* ctrl_reg_copy)
{
	me8100_do_subdevice_t* subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kmalloc(sizeof(me8100_do_subdevice_t), GFP_KERNEL);
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
		return NULL;
	}

	// Initialize spin locks.
	subdevice->ctrl_reg_lock = ctrl_reg_lock;
	subdevice->ctrl_reg_copy = ctrl_reg_copy;

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize registers.
	subdevice->port_reg = reg_base + ME8100_DO_REG_A + idx * ME8100_DO_REG_OFFSET;
	subdevice->ctrl_reg = reg_base + ME8100_CTRL_REG_A + idx * ME8100_DO_REG_OFFSET;

	// Overload base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8100_do_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me8100_do_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8100_do_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me8100_do_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me8100_do_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8100_do_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8100_do_query_subdevice_caps;

	return subdevice;
}
