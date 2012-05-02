/**
 * @file me8255.c
 *
 * @brief The 8255 DIO subdevice instance.
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
# include "me_internal.h"

#include "me8255_reg.h"
#include "me8255.h"

int me8255_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me8255_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me8255_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8255_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me8255_query_number_channels(me_subdevice_t* subdevice, int* number);
int me8255_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me8255_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);

static uint8_t get_mode_from_mirror(uint32_t mirror);
static void me8255_reconfigure_chip(me8255_subdevice_t* instance);


int me8255_io_reset_subdevice(struct me_subdevice* subdevice, struct file* filep, int flags)
{
	me8255_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8255_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				*instance->ctrl_reg_mirror &= ~(ME8255_PORT_0_OUTPUT << instance->base.idx);
				me8255_reconfigure_chip(instance);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);

		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8255_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me8255_subdevice_t* instance;
	int tmp;

	PDEBUG("executed.\n");

	instance = (me8255_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
		case ME_IO_SINGLE_CONFIG_DIO_BYTE:
			break;

		default:
			PERROR("Invalid flags. Should be ME_IO_SINGLE_CONFIG_DIO_BYTE.\n");
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
			PERROR("Invalid port configuration specified. Must be ME_SINGLE_CONFIG_DIO_INPUT or ME_SINGLE_CONFIG_DIO_OUTPUT.\n");
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
					case ME_SINGLE_CONFIG_DIO_INPUT:
						tmp = *instance->ctrl_reg_mirror & ~(ME8255_PORT_0_OUTPUT << instance->base.idx);
						break;

					case ME_SINGLE_CONFIG_DIO_OUTPUT:
						tmp = *instance->ctrl_reg_mirror | (ME8255_PORT_0_OUTPUT << instance->base.idx);
						break;

					default:
						tmp = *instance->ctrl_reg_mirror;
				}
				if (*instance->ctrl_reg_mirror != tmp)
				{
					*instance->ctrl_reg_mirror = tmp;
					me8255_reconfigure_chip(instance);
				}
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8255_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8255_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8255_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 8))
			{
				PERROR("Invalid bit number specified. Must be between 0 and 7.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if (channel)
			{
				PERROR("Invalid bit number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		me_readb(instance->base.dev, &tmp, instance->port_base_reg + instance->base.idx);
		switch (flags)
		{
			case ME_IO_SINGLE_TYPE_DIO_BIT:
				*value = (tmp & (0x1 << channel)) ? 1 : 0;
				break;

			case ME_IO_SINGLE_TYPE_NO_FLAGS:
			case ME_IO_SINGLE_TYPE_DIO_BYTE:
				*value = tmp;
				break;
		}
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8255_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me8255_subdevice_t* instance;
	uint8_t tmp;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me8255_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 8))
			{
				PERROR("Invalid bit number specified. Must be between 0 and 7.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if (channel)
			{
				PERROR("Invalid bit number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				if (!(*instance->ctrl_reg_mirror & (ME8255_PORT_0_OUTPUT << instance->base.idx)))
				{
							PERROR("Port not in output mode.\n");
							err = ME_ERRNO_PREVIOUS_CONFIG;
				}
				else
				{
					if (flags & ME_IO_SINGLE_TYPE_DIO_BIT)
					{
						me_readb(instance->base.dev, &tmp, instance->port_base_reg);
						if (value)
						{
							tmp |= 0x1 << channel;
						}
						else
						{
							tmp &= ~(0x1 << channel);
						}
					}
					else
					{
						tmp = value;
					}
					*instance->port_regs_mirror &= ~(0x00FF << (8 * instance->base.idx));
					*instance->port_regs_mirror |= (tmp << (8 * instance->base.idx));
					me_writeb(instance->base.dev, tmp, instance->port_base_reg + instance->base.idx);
				}
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8255_query_number_channels(struct me_subdevice* subdevice, int *number)
{
	PDEBUG("executed.\n");

	*number = 8;

	return ME_ERRNO_SUCCESS;
}

int me8255_query_subdevice_type(struct me_subdevice* subdevice, int *type, int *subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DIO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me8255_query_subdevice_caps(struct me_subdevice* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = ME_CAPS_DIO_DIR_BYTE;

	return ME_ERRNO_SUCCESS;
}

static uint8_t get_mode_from_mirror(uint32_t mirror)
{
	PDEBUG("executed.\n");

	if (mirror & ME8255_PORT_0_OUTPUT)
	{
		if (mirror & ME8255_PORT_1_OUTPUT)
		{
			if (mirror & ME8255_PORT_2_OUTPUT)
			{
				return ME8255_MODE_OOO;
			}
			else
			{
				return ME8255_MODE_IOO;
			}
		}
		else
		{
			if (mirror & ME8255_PORT_2_OUTPUT)
			{
				return ME8255_MODE_OIO;
			}
			else
			{
				return ME8255_MODE_IIO;
			}
		}
	}
	else
	{
		if (mirror & ME8255_PORT_1_OUTPUT)
		{
			if (mirror & ME8255_PORT_2_OUTPUT)
			{
				return ME8255_MODE_OOI;
			}
			else
			{
				return ME8255_MODE_IOI;
			}
		}
		else
		{
			if (mirror & ME8255_PORT_2_OUTPUT)
			{
				return ME8255_MODE_OII;
			}
			else
			{
				return ME8255_MODE_III;
			}
		}
	}
}

static void me8255_reconfigure_chip(me8255_subdevice_t* instance)
{
	int idx;
	uint8_t tmp;

	*instance->port_regs_mirror &= ~(0x000000FF << (8 * instance->base.idx));
	me_writeb(instance->base.dev, get_mode_from_mirror(*instance->ctrl_reg_mirror), instance->ctrl_reg);

	for (idx=0; idx<3; ++idx)
	{
		tmp = *instance->port_regs_mirror >> (8 * idx);
		if (tmp)
		{
			me_writeb(instance->base.dev, tmp, instance->port_base_reg + idx);
		}
	}
}

me8255_subdevice_t* me8255_constr(uint16_t device_id, void* reg_base,
										unsigned int me8255_idx, unsigned int idx,
										uint32_t* port_reg_mirror,
										int* ctrl_reg_mirror, me_lock_t* ctrl_reg_lock)
{
	me8255_subdevice_t *subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me8255_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for 8255 instance.\n");
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

	// Save the pointer to global port settings.
	subdevice->ctrl_reg_mirror = ctrl_reg_mirror;

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Save the pointer to global port values.
	subdevice->port_regs_mirror = port_reg_mirror;
	// Do device specific initialization.
	if (me8255_idx == 0)
	{
		subdevice->ctrl_reg = reg_base + ME1400_PORT_A_CTRL;
		subdevice->port_base_reg = reg_base + ME1400_PORT_A_0;
	}
	else if (me8255_idx == 1)
	{
		switch (device_id)
		{
			case PCI_DEVICE_ID_MEILHAUS_ME140C:
			case PCI_DEVICE_ID_MEILHAUS_ME140D:
				subdevice->ctrl_reg = reg_base + ME1400CD_PORT_B_CTRL;
				subdevice->port_base_reg = reg_base + ME1400CD_PORT_B_0;
				break;

			default:
				subdevice->ctrl_reg = reg_base + ME1400AB_PORT_B_CTRL;
				subdevice->port_base_reg = reg_base + ME1400AB_PORT_B_0;
				break;
		}
	}
	else
	{
		PERROR("8255 index is out of range. Must be 0 or 1.\n");
		kfree(subdevice);
		return NULL;
	}

	// Overload subdevice base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8255_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me8255_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8255_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me8255_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me8255_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8255_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8255_query_subdevice_caps;

	return subdevice;
}
