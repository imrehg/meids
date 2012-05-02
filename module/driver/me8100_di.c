/**
 * @file me8100_di.c
 *
 * @brief The ME-8100 digital input subdevice instance.
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
# include "me8100_di_reg.h"
# include "me8100_di.h"

/// Defines
int me8100_di_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
int me8100_di_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags);
int me8100_di_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8100_di_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8100_di_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me8100_di_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8100_di_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me8100_di_query_number_channels(me_subdevice_t* subdevice, int* number);
int me8100_di_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me8100_di_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);

static int me8100_di_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);


///Functions

int me8100_di_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me8100_di_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				// Clear interrupts control bits. - No interrupts.
				*(instance->ctrl_reg_copy) &= ~(ME8100_DIO_CTRL_BIT_INTB_0 | ME8100_DIO_CTRL_BIT_INTB_1);
				me_writew(instance->base.dev, *(instance->ctrl_reg_copy), instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			me_writew(instance->base.dev, 0, instance->mask_reg);
			me_writew(instance->base.dev, 0, instance->pattern_reg);
			instance->rised = -1;
			instance->irq_count = 0;
			instance->filtering_flag = 0;
		ME_UNLOCK_PROTECTOR;

		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{
	me8100_di_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint16_t tmp;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	if (irq_source == ME_IRQ_SOURCE_DIO_DEFAULT)
	{
		irq_source = ME_IRQ_SOURCE_DIO_MASK;
	}

	if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
	{
		if (flags & ~(ME_IO_IRQ_START_PATTERN_FILTERING | ME_IO_IRQ_START_DIO_WORD))
		{
			PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_DIO_WORD or ME_IO_IRQ_START_PATTERN_FILTERING.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}

		if (irq_edge != ME_IRQ_EDGE_NOT_USED)
		{
			PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_NOT_USED.\n");
			return ME_ERRNO_INVALID_IRQ_EDGE;
		}
	}
	else if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
	{
		if (flags & ~(ME_IO_IRQ_START_EXTENDED_STATUS | ME_IO_IRQ_START_DIO_WORD))
		{
			PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_DIO_WORD or ME_IO_IRQ_START_EXTENDED_STATUS.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}

		if (irq_edge != ME_IRQ_EDGE_ANY)
		{
			PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_ANY.\n");
			return ME_ERRNO_INVALID_IRQ_EDGE;
		}

		if (!(irq_arg & 0xFFFF))
		{
			PERROR("No mask specified. Mustn't be 0.\n");
			return ME_ERRNO_INVALID_IRQ_ARG;
		}
	}
	else
	{
		PERROR("Invalid irq source specified. Should be ME_IRQ_SOURCE_DIO_MASK or ME_IRQ_SOURCE_DIO_PATTERN.\n");
		return ME_ERRNO_INVALID_IRQ_SOURCE;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
			{
				me_writew(instance->base.dev, irq_arg, instance->pattern_reg);
				instance->compare_value = irq_arg;
				instance->filtering_flag = (flags & ME_IO_IRQ_START_PATTERN_FILTERING) ? 1 : 0;
			}
			if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
			{
				me_writew(instance->base.dev, irq_arg, instance->mask_reg);
			}

			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				*(instance->ctrl_reg_copy) |= ME8100_DIO_CTRL_BIT_INTB_0;
				if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
				{
					*(instance->ctrl_reg_copy) &= ~ME8100_DIO_CTRL_BIT_INTB_1;
				}

				if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
				{
					*(instance->ctrl_reg_copy) |= ME8100_DIO_CTRL_BIT_INTB_1;
				}
				me_writew(instance->base.dev, *(instance->ctrl_reg_copy), instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			instance->rised = 0;
			instance->status_value = 0;
			instance->status_value_edges = 0;

			// me_readw(instance->base.dev, &tmp, instance->din_int_reg);
			me_readw(instance->base.dev, &tmp, instance->port_reg);
			instance->line_value = tmp;
			instance->status_flag = flags & ME_IO_IRQ_START_EXTENDED_STATUS;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8100_di_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags)
{
	me8100_di_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	unsigned long int delay = LONG_MAX - 2;
	int count;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	if (flags & ~(ME_IO_IRQ_WAIT_NORMAL_STATUS | ME_IO_IRQ_WAIT_EXTENDED_STATUS))
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_WAIT_NO_FLAGS, ME_IO_IRQ_WAIT_NORMAL_STATUS or ME_IO_IRQ_WAIT_EXTENDED_STATUS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if (time_out)
	{
		delay = (time_out * HZ) / 1000;
		if (!delay)
			delay = 1;
		if (delay>LONG_MAX - 2)
			delay = LONG_MAX - 2;
	}

	ME_SUBDEVICE_ENTER;
		if (instance->rised <= 0)
		{
			instance->rised = 0;
			count = instance->irq_count;

			if (wait_event_interruptible_timeout(instance->wait_queue, ((count != instance->irq_count) || (instance->rised<0)), delay) <= 0)
			{
				PERROR("Wait on interrupt timed out.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (instance->rised < 0)
			{
				PDEBUG("Aborted by user.\n");
				err = ME_ERRNO_CANCELLED;
			}
		}

		if (signal_pending(current))
		{
			PDEBUG("Aborted by signal.\n");
			err = ME_ERRNO_SIGNAL;
		}

		*irq_count = instance->irq_count;
		if(!err)
		{
			if (flags & ME_IO_IRQ_WAIT_NORMAL_STATUS)
			{
				*value = instance->status_value;
			}
			else if (flags & ME_IO_IRQ_WAIT_EXTENDED_STATUS)
			{
				*value = instance->status_value_edges;
			}
			else
			{// Use default
				if (!instance->status_flag)
				{
					*value = instance->status_value;
				}
				else
				{
					*value = instance->status_value_edges;
				}
			}
			instance->rised = 0;
		}
		else
		{
			*value = 0;
		}
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8100_di_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8100_di_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_STOP_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				*(instance->ctrl_reg_copy) &= ~(ME8100_DIO_CTRL_BIT_INTB_1 | ME8100_DIO_CTRL_BIT_INTB_0);
				me_writew(instance->base.dev, *(instance->ctrl_reg_copy), instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			instance->rised = 0;
			instance->status_value = 0;
			instance->status_value_edges = 0;
			instance->filtering_flag = 0;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8100_di_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	if (flags & ~(ME_IO_IRQ_WAIT_NORMAL_STATUS | ME_IO_IRQ_WAIT_EXTENDED_STATUS))
	{
		PERROR("Invalid flag specified.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain,  int trig_type, int trig_edge, int flags)
{
	me8100_di_subdevice_t* instance;

	instance = (me8100_di_subdevice_t *) subdevice;

	PDEBUG("executed.\n");

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

	if (single_config != ME_SINGLE_CONFIG_DIO_INPUT)
	{
		PERROR("Invalid port configuration specified. Must be ME_SINGLE_CONFIG_DIO_INPUT.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8100_di_subdevice_t* instance;
	uint16_t tmp;

	PDEBUG("executed.\n");

	instance = (me8100_di_subdevice_t *) subdevice;

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 16))
			{
				PERROR("Invalid bits number specified. Must be between 0 and 15.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel >= 2))
			{
				PERROR("Invalid bytes number specified. Must be 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if (channel)
			{
				PERROR("Invalid words number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		me_readw(instance->base.dev, &tmp, instance->port_reg);
		switch (flags)
		{
			case ME_IO_SINGLE_TYPE_DIO_BIT:
				*value =  tmp & (0x1 << channel);
				break;

			case ME_IO_SINGLE_TYPE_DIO_BYTE:
				tmp >>= (channel * 8);
				*value =  tmp & 0xFF;
				break;

			case ME_IO_SINGLE_TYPE_NO_FLAGS:
			case ME_IO_SINGLE_TYPE_DIO_WORD:
				*value = tmp;
		}
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_query_number_channels(me_subdevice_t* subdevice, int *number)
{
	PDEBUG("executed.\n");

	*number = 16;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DI;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me8100_di_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = me8100_DI_CAPS;	//ME_CAPS_DIO_BIT_PATTERN_IRQ | ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY;

	return ME_ERRNO_SUCCESS;
}

static int me8100_di_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{
	me8100_di_subdevice_t* instance = (me8100_di_subdevice_t *) subdevice;
	uint16_t	tmp;
	uint16_t irq_status_val;
	uint16_t line_value = 0;
	uint32_t status_val = 0;
	int err = ME_ERRNO_SUCCESS;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed.\n");

		me_readw(instance->base.dev, &tmp, instance->irq_reset_reg);
		me_readw(instance->base.dev, &line_value, instance->port_reg);
		irq_status_val = instance->line_value ^ line_value;

		// Make extended information.
		status_val |= (0x00FF & (~(uint16_t)instance->line_value & line_value)) << 16;	//Raise
		status_val |= (0x00FF & ((uint16_t)instance->line_value & ~line_value));		//Fall

		instance->line_value = line_value;

		if (instance->rised <= 0)
		{
			instance->status_value = irq_status_val;
			instance->status_value_edges = status_val;
		}
		else
		{
			instance->status_value |= irq_status_val;
			instance->status_value_edges |= status_val;
		}

		if (instance->filtering_flag)
		{// For compare mode only.
			if (instance->compare_value == instance->line_value)
			{
				instance->rised = 1;
				instance->irq_count++;
			}
		}
		else
		{
			instance->rised = 1;
			instance->irq_count++;
		}
	ME_FREE_HANDLER_PROTECTOR;

	wake_up_interruptible_all(&instance->wait_queue);

	return err;
}

me8100_di_subdevice_t* me8100_di_constr(void* reg_base, unsigned int idx, me_lock_t* irq_ctrl_lock, uint16_t* ctrl_reg_copy)
{
	me8100_di_subdevice_t *subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me8100_di_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		return NULL;
	}

	//Initialize subdevice base class.
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		return NULL;
	}

	// Initialize spin locks.
	subdevice->irq_ctrl_lock = irq_ctrl_lock;
	subdevice->ctrl_reg_copy = ctrl_reg_copy;

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize the registers.
// 	subdevice->irq_status_reg = plx_reg_base + PLX_INTCSR;
	subdevice->ctrl_reg = reg_base + ME8100_CTRL_REG_A + idx * ME8100_DI_REG_OFFSET ;
	subdevice->port_reg = reg_base + ME8100_DI_REG_A + idx * ME8100_DI_REG_OFFSET ;
	subdevice->mask_reg = reg_base + ME8100_MASK_REG_A + idx * ME8100_DI_REG_OFFSET ;
	subdevice->pattern_reg = reg_base + ME8100_PATTERN_REG_A + idx * ME8100_DI_REG_OFFSET ;
	subdevice->din_int_reg = reg_base + ME8100_INT_DI_REG_A + idx * ME8100_DI_REG_OFFSET ;
	subdevice->irq_reset_reg = reg_base + ME8100_RES_INT_REG_A + idx * ME8100_DI_REG_OFFSET ;

	// Overload base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8100_di_io_reset_subdevice;
	subdevice->base.me_subdevice_io_irq_start = me8100_di_io_irq_start;
	subdevice->base.me_subdevice_io_irq_wait = me8100_di_io_irq_wait;
	subdevice->base.me_subdevice_io_irq_stop = me8100_di_io_irq_stop;
	subdevice->base.me_subdevice_io_irq_test = me8100_di_io_irq_test;
	subdevice->base.me_subdevice_io_single_config = me8100_di_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8100_di_io_single_read;
	subdevice->base.me_subdevice_query_number_channels = me8100_di_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8100_di_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8100_di_query_subdevice_caps;
	subdevice->base.me_subdevice_irq_handle = me8100_di_irq_handle;

	subdevice->rised = 0;
	subdevice->irq_count = 0;

	return subdevice;
}
