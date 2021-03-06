/**
 * @file me4600_dio.c
 * @brief The ME-4600 digital input/output subdevice instance.
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

#include "me4600_dio_reg.h"
#include "me4600_dio.h"

int me4600_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me4600_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me4600_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me4600_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me4600_dio_query_number_channels(me_subdevice_t* subdevice, int* number);
int me4600_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me4600_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);


int me4600_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4600_dio_subdevice_t* instance;
	uint32_t tmp;

	instance = (me4600_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);
	PINFO("Port type: %s\n", (instance->port_type == ME_TYPE_DO) ? "DO" : ((instance->port_type == ME_TYPE_DI) ? "DI": ((instance->port_type == ME_TYPE_DIO) ? "DIO": "UNKNOWN")));

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
				// Default settings: clear all bits -> INPUT mode
				tmp &= ~(ME4600_DIO_CTRL_BIT_MODE_MASK << (instance->base.idx * ME4600_DIO_MODE_SHIFT));
				tmp &= ~(ME4600_DIO_CTRL_BIT_FIFO_HIGH << instance->base.idx);
				tmp &= ~ME4600_DIO_CTRL_BIT_FUNCTION_MASK;
				if (instance->port_type == ME_TYPE_DO)
				{
					tmp |= ME4600_DIO_CTRL_BIT_MODE_OUTPUT << (instance->base.idx * ME4600_DIO_MODE_SHIFT);
				}
				me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
			me_writel(instance->base.dev, 0, instance->port_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me4600_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4600_dio_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint32_t tmp;

	instance = (me4600_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);
	PINFO("Port type: %s\n", (instance->port_type == ME_TYPE_DO) ? "DO" : ((instance->port_type == ME_TYPE_DI) ? "DI": ((instance->port_type == ME_TYPE_DIO) ? "DIO": "UNKNOWN")));

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
		case ME_IO_SINGLE_CONFIG_DIO_BYTE:
			break;

		default:
			PERROR("Invalid flags specified. Should be ME_IO_SINGLE_CONFIG_DIO_BYTE.\n");
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

	if (channel)
	{
		PERROR("Invalid channel number.\n");
		err = ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER
		ME_SUBDEVICE_LOCK;
			switch (ref)
			{
				case ME_REF_NONE:
					break;

				case ME_REF_DIO_FIFO_LOW:
				case ME_REF_DIO_FIFO_HIGH:
					if (instance->port_type == ME_TYPE_DI)
					{
						PERROR("Invalid port reference specified. Must be ME_REF_NONE.\n");
						err = ME_ERRNO_INVALID_REF;
						goto ERROR;
					}
					break;

				default:
					PERROR("Invalid port reference specified.\n");
					err = ME_ERRNO_INVALID_REF;
					goto ERROR;
			}

			switch (single_config)
			{
				case ME_SINGLE_CONFIG_DIO_INPUT:
					if (instance->port_type == ME_TYPE_DO)
					{
						PERROR("Invalid configuration specified. Must be ME_SINGLE_CONFIG_DIO_OUTPUT.\n");
						err = ME_ERRNO_INVALID_SINGLE_CONFIG;
						goto ERROR;
					}
					if (ref != ME_REF_NONE)
					{
						PERROR("Invalid port reference specified. Must be ME_REF_NONE.\n");
						err = ME_ERRNO_INVALID_REF;
						goto ERROR;
					}
					break;

				case ME_SINGLE_CONFIG_DIO_OUTPUT:
					if (instance->port_type == ME_TYPE_DI)
					{
						PERROR("Invalid configuration specified. Must be ME_SINGLE_CONFIG_DIO_INPUT.\n");
						err = ME_ERRNO_INVALID_SINGLE_CONFIG;
						goto ERROR;
					}
					if (ref != ME_REF_NONE)
					{
						PERROR("Invalid port reference specified. Must be ME_REF_NONE.\n");
						err = ME_ERRNO_INVALID_REF;
						goto ERROR;
					}
					break;

				case ME_SINGLE_CONFIG_DIO_BIT_PATTERN:
				case ME_SINGLE_CONFIG_DIO_MUX32M:
				case ME_SINGLE_CONFIG_DIO_DEMUX32:
					if (instance->port_type == ME_TYPE_DI)
					{
						PERROR("Invalid configuration specified. Must be ME_SINGLE_CONFIG_DIO_INPUT.\n");
						err = ME_ERRNO_INVALID_SINGLE_CONFIG;
						goto ERROR;
					}
					if (ref == ME_REF_NONE)
					{
						PERROR("Invalid port reference specified. Must be ME_REF_DIO_FIFO_LOW or ME_REF_DIO_FIFO_HIGH.\n");
						err = ME_ERRNO_INVALID_REF;
						goto ERROR;
					}
					break;

				default:
					PERROR("Invalid port configuration specified. Should be ME_SINGLE_CONFIG_DIO_INPUT, ME_SINGLE_CONFIG_DIO_OUTPUT or ME_SINGLE_CONFIG_DIO_BIT_PATTERN.\n");
					err = ME_ERRNO_INVALID_REF;
					goto ERROR;
			}

			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
				// INPUT mode
				tmp &= ~(ME4600_DIO_CTRL_BIT_MODE_MASK << (instance->base.idx * ME4600_DIO_MODE_SHIFT));

				switch (single_config)
				{
					case ME_SINGLE_CONFIG_DIO_INPUT:
						break;

					case ME_SINGLE_CONFIG_DIO_OUTPUT:
						tmp |= ME4600_DIO_CTRL_BIT_MODE_OUTPUT << (instance->base.idx * ME4600_DIO_MODE_SHIFT);
						break;

					case ME_SINGLE_CONFIG_DIO_BIT_PATTERN:
						PINFO("Mode: BP.\n");
						if (tmp & (ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT)))
						{
							if ((tmp & ME4600_DIO_CTRL_BIT_FUNCTION_MASK) != ME4600_DIO_CTRL_BIT_FUNCTION_BP)
							{
								PERROR("Can not set BP mode. Ports are configured for MUX or DEMUX.\n");
								err = ME_ERRNO_USED;
								goto ERROR;
							}
						}
						tmp |= ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT);
						tmp &= ~ME4600_DIO_CTRL_BIT_FUNCTION_MASK;
						break;

					case ME_SINGLE_CONFIG_DIO_MUX32M:
						PINFO("Mode: MUX.\n");
						if (tmp & (ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT)))
						{
							if ((tmp & ME4600_DIO_CTRL_BIT_FUNCTION_MASK) != ME4600_DIO_CTRL_BIT_FUNCTION_MUX)
							{
								PERROR("Can not set MUX mode. Ports are configured for BP or DEMUX.\n");
								err = ME_ERRNO_USED;
								goto ERROR;
							}
						}
						tmp |= ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT);
						tmp &= ~ME4600_DIO_CTRL_BIT_FUNCTION_MASK;
						tmp |= ME4600_DIO_CTRL_BIT_FUNCTION_MUX;
						break;

					case ME_SINGLE_CONFIG_DIO_DEMUX32:
						PINFO("Mode: DEMUX.\n");
						if (tmp & (ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT)))
						{
							if ((tmp & ME4600_DIO_CTRL_BIT_FUNCTION_MASK) != ME4600_DIO_CTRL_BIT_FUNCTION_DEMUX)
							{
								PERROR("Can not set DEMUX mode. Ports are configured for MUX or BP.\n");
								err = ME_ERRNO_USED;
								goto ERROR;
							}
						}
						tmp |= ME4600_DIO_CTRL_BIT_MODE_FIFO << (instance->base.idx * ME4600_DIO_MODE_SHIFT);
						tmp &= ~ME4600_DIO_CTRL_BIT_FUNCTION_MASK;
						tmp |= ME4600_DIO_CTRL_BIT_FUNCTION_DEMUX;
						break;
				}

				if (ref == ME_REF_DIO_FIFO_HIGH)
				{
					tmp |= ME4600_DIO_CTRL_BIT_FIFO_HIGH << instance->base.idx;
				}
				else
				{
					tmp &= ~(ME4600_DIO_CTRL_BIT_FIFO_HIGH << instance->base.idx);
				}

				me_writel(instance->base.dev, tmp, instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}


int me4600_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me4600_dio_subdevice_t* instance;
	uint32_t tmp;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_dio_subdevice_t *)subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);
	PINFO("Port type: %s\n", (instance->port_type == ME_TYPE_DO) ? "DO" : ((instance->port_type == ME_TYPE_DI) ? "DI": ((instance->port_type == ME_TYPE_DIO) ? "DIO": "UNKNOWN")));

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
			me_readl(instance->base.dev, &tmp, instance->port_reg);
			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					*value = tmp & (0x1 << channel);
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_BYTE:
				default:
					*value = tmp & 0xFF;
			}

			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp >>= instance->base.idx * ME4600_DIO_MODE_SHIFT;
			tmp &= ME4600_DIO_CTRL_BIT_MODE_MASK;
			if ((tmp != ME4600_DIO_CTRL_BIT_MODE_OUTPUT) && (tmp != ME4600_DIO_CTRL_BIT_MODE_INPUT))
			{
				PERROR("Port not in output nor input mode. MODE: %x\n", tmp);
				err = ME_ERRNO_PREVIOUS_CONFIG;
			}
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}


int me4600_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me4600_dio_subdevice_t* instance;
	uint32_t tmp;
	int err = ME_ERRNO_SUCCESS;

	instance = (me4600_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);
	PINFO("Port type: %s\n", (instance->port_type == ME_TYPE_DO) ? "DO" : ((instance->port_type == ME_TYPE_DI) ? "DI": ((instance->port_type == ME_TYPE_DIO) ? "DIO": "UNKNOWN")));

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
			me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
			tmp >>= (instance->base.idx * ME4600_DIO_MODE_SHIFT);
			tmp &= ME4600_DIO_CTRL_BIT_MODE_MASK;
			if (tmp != ME4600_DIO_CTRL_BIT_MODE_OUTPUT)
			{
				PERROR("Port not in output mode. MODE: %x\n", tmp);
				err = ME_ERRNO_PREVIOUS_CONFIG;
				goto ERROR;
			}

			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					me_readl(instance->base.dev, &tmp, instance->port_reg);
					if (value) tmp |= 0x1 << channel;
					else tmp &= ~(0x1 << channel);
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_BYTE:
				default:
					tmp = value & 0xFF;
			}
			me_writel(instance->base.dev, tmp, instance->port_reg);
ERROR:
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me4600_dio_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed idx=%d.\n", ((me4600_dio_subdevice_t *) subdevice)->base.idx);

	*number = 8;

	return ME_ERRNO_SUCCESS;
}

int me4600_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	me4600_dio_subdevice_t* instance;
	instance = (me4600_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	*type = instance->port_type;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me4600_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed idx=%d.\n", ((me4600_dio_subdevice_t *) subdevice)->base.idx);

	*caps = me4600_DIO_CAPS;

	return ME_ERRNO_SUCCESS;
}

me4600_dio_subdevice_t* me4600_dio_constr(void* reg_base, unsigned int idx, me_lock_t* ctrl_reg_lock, int port_type)
{
	me4600_dio_subdevice_t* subdevice;

	PDEBUG("executed idx=%d.\n", idx);
	PINFO("Port type: %s\n", (port_type == ME_TYPE_DO) ? "DO" : ((port_type == ME_TYPE_DI) ? "DI": ((port_type == ME_TYPE_DIO) ? "DIO": "UNKNOWN")));

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4600_dio_subdevice_t), GFP_KERNEL);
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

	// Save digital i/o index.
	subdevice->base.idx = idx;
	subdevice->port_type = port_type;

	// Set the subdevice ports' registers.
	subdevice->ctrl_reg = reg_base + ME4600_DIO_CTRL_REG;
	subdevice->port_reg = reg_base + ME4600_DIO_PORT_REG + (idx  << 2);

	// Override base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me4600_dio_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me4600_dio_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me4600_dio_io_single_read;
	if (port_type != ME_TYPE_DI)
	{
		subdevice->base.me_subdevice_io_single_write = me4600_dio_io_single_write;
	}
	subdevice->base.me_subdevice_query_number_channels = me4600_dio_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4600_dio_query_subdevice_type;
	if (port_type == ME_TYPE_DIO)
	{
		subdevice->base.me_subdevice_query_subdevice_caps = me4600_dio_query_subdevice_caps;
	}

	return subdevice;
}
