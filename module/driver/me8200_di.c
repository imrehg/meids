/**
 * @file me8200_di.c
 *
 * @brief The ME-8200 digital input subdevice instance.
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

# include "me8200_reg.h"
# include "me8200_di_reg.h"
# include "me8200_di.h"

#define PATERN_FILTERING	(0x1	<<	0)
#define EDGES_RAISING		(0x1	<<	4)
#define EDGES_FALLING		(0x1	<<	5)
#define PATERN_MASK			(PATERN_FILTERING)
#define EDGES_MASK			(EDGES_RAISING | EDGES_FALLING)

/// Defines
int me8200_di_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
int me8200_di_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags);
int me8200_di_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8200_di_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8200_di_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me8200_di_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8200_di_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me8200_di_query_number_channels(me_subdevice_t* subdevice, int* number);
int me8200_di_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me8200_di_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int me8200_di_query_version_firmware(me_subdevice_t* subdevice, int* version);

static int me8200_di_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);
static int me8200_di_irq_handle_EX(me_subdevice_t* subdevice, uint32_t irq_status);
static void me8200_di_check_version(me8200_di_subdevice_t* instance, me_general_dev_t* device, void* addr);


///Functions
int me8200_di_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me8200_di_subdevice_t* instance = (me8200_di_subdevice_t *) subdevice;
	uint8_t tmp;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_ctrl_reg);
				tmp |= (ME8200_DI_IRQ_CTRL_BIT_ENABLE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
				tmp &= ~(ME8200_DI_IRQ_CTRL_BIT_ENABLE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				tmp |= (ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			instance->rised = -1;
			instance->status_value = 0;
			instance->status_value_edges = 0;
			instance->filtering_flag = 0x00;

			instance->count = 0;
		ME_UNLOCK_PROTECTOR;
	wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{
	me8200_di_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

	if (irq_source == ME_IRQ_SOURCE_DIO_DEFAULT)
	{
		irq_source = ME_IRQ_SOURCE_DIO_MASK;
	}

	if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
	{
		if (flags & ~(ME_IO_IRQ_START_PATTERN_FILTERING | ME_IO_IRQ_START_DIO_BYTE))
		{
			PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_DIO_BYTE or ME_IO_IRQ_START_PATTERN_FILTERING.\n");
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
		if (flags & ~(ME_IO_IRQ_START_EXTENDED_STATUS | ME_IO_IRQ_START_DIO_BYTE))
		{
			PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_DIO_BYTE or ME_IO_IRQ_START_EXTENDED_STATUS.\n");
			return ME_ERRNO_INVALID_FLAGS;
		}

		if ((irq_edge != ME_IRQ_EDGE_RISING) && (irq_edge != ME_IRQ_EDGE_FALLING) && (irq_edge != ME_IRQ_EDGE_ANY))
		{
			PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_ANY.\n");
			return ME_ERRNO_INVALID_IRQ_EDGE;
		}

		if (!(irq_arg & 0xFF))
		{
			PERROR("No mask specified.\n");
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
			instance->filtering_flag = 0x00;
			if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
			{
					me_writeb(instance->base.dev, irq_arg, instance->compare_reg);
					me_writeb(instance->base.dev, 0xFF, instance->mask_reg);
					instance->compare_value = irq_arg;
					if (flags & ME_IO_IRQ_START_PATTERN_FILTERING)
					{
						instance->filtering_flag |=  PATERN_FILTERING;
					}
			}
			if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
			{
					me_writeb(instance->base.dev, irq_arg, instance->mask_reg);
			}

			ME_SPIN_LOCK(instance->irq_mode_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_mode_reg);
				tmp &= ~(ME8200_IRQ_MODE_MASK << (ME8200_IRQ_MODE_DI_SHIFT * instance->base.idx));
				if (irq_source == ME_IRQ_SOURCE_DIO_PATTERN)
				{
					tmp |= ME8200_IRQ_MODE_MASK_COMPARE << (ME8200_IRQ_MODE_DI_SHIFT * instance->base.idx);
				}

				if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
				{
					tmp |= ME8200_IRQ_MODE_MASK_MASK << (ME8200_IRQ_MODE_DI_SHIFT * instance->base.idx);
				}
				me_writeb(instance->base.dev, tmp, instance->irq_mode_reg);
			ME_SPIN_UNLOCK(instance->irq_mode_lock);

			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_ctrl_reg);
				tmp |= (ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				tmp |= ME8200_DI_IRQ_CTRL_BIT_ENABLE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx);

				if (irq_source == ME_IRQ_SOURCE_DIO_MASK)
				{
					tmp &= ~(ME8200_DI_IRQ_CTRL_MASK_EDGE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
					if (irq_edge == ME_IRQ_EDGE_RISING)
					{
						tmp |= ME8200_DI_IRQ_CTRL_MASK_EDGE_RISING << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx);
						instance->filtering_flag |= EDGES_RAISING;
					}
					else if (irq_edge == ME_IRQ_EDGE_FALLING)
					{
						tmp |= ME8200_DI_IRQ_CTRL_MASK_EDGE_FALLING << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx);
						instance->filtering_flag |= EDGES_FALLING;
					}
					else if (irq_edge == ME_IRQ_EDGE_ANY)
					{
						tmp |= ME8200_DI_IRQ_CTRL_MASK_EDGE_ANY << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx);
						instance->filtering_flag |= EDGES_FALLING | EDGES_RAISING;
					}
				}
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
				tmp &= ~(ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			me_readb(instance->base.dev, &tmp, instance->port_reg);
			instance->line_value = tmp;

			instance->rised = 0;
			instance->status_value = 0;
			instance->status_value_edges = 0;
			instance->status_flag = flags & ME_IO_IRQ_START_EXTENDED_STATUS;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8200_di_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int *irq_count, int* value, int time_out, int flags)
{
	me8200_di_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	unsigned long int delay = LONG_MAX -2;
	int count;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

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
			count = instance->count;

			if (wait_event_interruptible_timeout(instance->wait_queue, ((count != instance->count) || (instance->rised<0)), delay) <= 0)
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

		*irq_count = instance->count;
		if (!err)
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

int me8200_di_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8200_di_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

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

	ME_SUBDEVICE_ENTER
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK(instance->irq_ctrl_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_ctrl_reg);
				tmp |= (ME8200_DI_IRQ_CTRL_BIT_ENABLE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
				tmp &= ~(ME8200_DI_IRQ_CTRL_BIT_ENABLE << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				tmp |= (ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_ctrl_reg);
			ME_SPIN_UNLOCK(instance->irq_ctrl_lock);

			instance->rised = -1;
			instance->status_value = 0;
			instance->status_value_edges = 0;
			instance->filtering_flag = 0x00;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8200_di_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

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

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me8200_di_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

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

	if (ref)
	{
		PERROR("Invalid reference. Must be ME_REF_NONE.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if (single_config != ME_SINGLE_CONFIG_DIO_INPUT)
	{
		PERROR("Invalid port direction specified. Must be ME_SINGLE_CONFIG_DIO_INPUT.\n");
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

int me8200_di_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8200_di_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8200_di_subdevice_t *) subdevice;

	switch (flags)
	{

		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel >= 8))
			{
				PERROR("Invalid bits number specified. Must be between 0 and 7.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if (channel)
			{
				PERROR("Invalid bytes number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		me_readb(instance->base.dev, &tmp, instance->port_reg);
		switch (flags)
		{
			case ME_IO_SINGLE_TYPE_DIO_BIT:
				*value = tmp & (0x1 << channel);
				break;

			case ME_IO_SINGLE_TYPE_NO_FLAGS:
			case ME_IO_SINGLE_TYPE_DIO_BYTE:
					*value = tmp;
				break;
		}
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_query_number_channels(me_subdevice_t* subdevice, int *number)
{
	PDEBUG("executed.\n");

	*number = 8;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_query_subdevice_type(me_subdevice_t* subdevice, int *type, int *subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DI;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps =
	    ME_CAPS_DIO_BIT_PATTERN_IRQ |
	    ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING |
	    ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING |
	    ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY;

	return ME_ERRNO_SUCCESS;
}

int me8200_di_query_version_firmware(me_subdevice_t* subdevice, int* version)
{
	me8200_di_subdevice_t *instance = (me8200_di_subdevice_t *) subdevice;
	PDEBUG("executed.\n");

	*version = instance->version;

	return ME_ERRNO_SUCCESS;
}

static int me8200_di_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{
	me8200_di_subdevice_t* instance;
	uint8_t ctrl;
	uint8_t irq_value;
	uint8_t line_value;

	unsigned int status_val;
	unsigned int status_val_EX;
	int err = ME_ERRNO_SUCCESS;

	instance = (me8200_di_subdevice_t *) subdevice;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed.\n");

		me_readb(instance->base.dev, &irq_value, instance->irq_status_reg);
		if (!irq_value)
		{
			PINFO("%ld Shared interrupt. %s(): idx=%d irq_status_reg=0x%04X\n", jiffies, __FUNCTION__, instance->base.idx, irq_value);
			err = ME_ERRNO_INVALID_IRQ_SOURCE;
		}
		else
		{
			me_readb(instance->base.dev, &ctrl, instance->irq_ctrl_reg);
			ctrl |= ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx);
			me_writeb(instance->base.dev, ctrl, instance->irq_ctrl_reg);
			ctrl &= ~(ME8200_DI_IRQ_CTRL_BIT_CLEAR << (ME8200_DI_IRQ_CTRL_SHIFT * instance->base.idx));
			me_writeb(instance->base.dev, ctrl, instance->irq_ctrl_reg);

			me_readb(instance->base.dev, &line_value, instance->port_reg);

			// Make extended information.

			switch (instance->filtering_flag & EDGES_MASK)
			{
				case EDGES_RAISING:
					status_val    = (unsigned int)irq_value | (~instance->line_value & (unsigned int)line_value);
					status_val_EX = status_val << 16;
					break;

				case EDGES_FALLING:
					status_val    = (unsigned int)irq_value | (instance->line_value & ~(unsigned int)line_value);
					status_val_EX = status_val;
					break;

				default:
					status_val    = instance->line_value ^ (unsigned int)line_value;
					status_val_EX = ((~instance->line_value & (unsigned int)line_value) << 16) | (instance->line_value & ~(unsigned int)line_value);
			}

			instance->line_value = (unsigned int)line_value;

			if (instance->rised <= 0)
			{
				instance->status_value = status_val;
				instance->status_value_edges = status_val_EX;
			}
			else
			{
				instance->status_value |= status_val;
				instance->status_value_edges |= status_val_EX;
			}

			if (instance->filtering_flag & PATERN_FILTERING)
			{// For compare mode only.
				if (instance->compare_value == line_value)
				{
					instance->rised = 1;
					instance->count++;
				}
			}
			else
			{
				instance->rised = 1;
				instance->count++;
			}
		}
	ME_FREE_HANDLER_PROTECTOR;

	if (!err)
	{
		wake_up_interruptible_all(&instance->wait_queue);
	}

	return err;
}

static int me8200_di_irq_handle_EX(me_subdevice_t* subdevice, uint32_t irq_status)
{
	me8200_di_subdevice_t* instance;
	uint8_t tmp;
	uint8_t irq_status_val = 0;
	uint16_t irq_status_EX = 0;
	uint32_t status_val = 0;
	int i, j;
	int err = ME_ERRNO_SUCCESS;

	instance = (me8200_di_subdevice_t *) subdevice;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed.\n");

		//Reset latches. Copy status to extended registers.
		me_readb(instance->base.dev, &irq_status_val, instance->irq_status_reg);
		if (!irq_status_val)
		{
			PINFO("%ld Shared interrupt. %s(): idx=%d irq_status_reg=0x%04X\n", jiffies, __FUNCTION__, instance->base.idx, irq_status_val);
			err = ME_ERRNO_INVALID_IRQ_SOURCE;
		}
		else
		{
			me_readb(instance->base.dev, &tmp, instance->irq_status_low_reg);
			irq_status_EX = tmp;
			me_readb(instance->base.dev, &tmp, instance->irq_status_high_reg);
			irq_status_EX |= (((uint16_t)tmp)<<8);

			// Format extended information.
			for (i=0, j=0; i<8; i++, j+=2)
			{
				status_val |= ((0x01 << j) & irq_status_EX) >> (j-i);			//Fall
				status_val |= ((0x01 << (j+1)) & irq_status_EX) << (15-j+i);	//Raise
			}

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

			if (instance->filtering_flag & PATERN_FILTERING)
			{// For compare mode only.
				me_readb(instance->base.dev, &tmp, instance->port_reg);
				instance->line_value = tmp;

				if (instance->compare_value == instance->line_value)
				{
					instance->rised = 1;
					instance->count++;
				}
			}
			else
			{
				instance->rised = 1;
				instance->count++;
			}
		}
	ME_FREE_HANDLER_PROTECTOR;

	if (!err)
	{
		wake_up_interruptible_all(&instance->wait_queue);
	}

	return err;
}

static void me8200_di_check_version(me8200_di_subdevice_t* instance, me_general_dev_t* device, void* addr)
{
	uint8_t tmp;

	PDEBUG("executed.\n");
	me_readb(device, &tmp, addr);
	instance->version = 0x000000FF & tmp;
	PINFO("me8200 firmware version: %d\n", instance->version);

	/// @note Fix for wrong values in this registry.
	if ((instance->version<0x7) || (instance->version>0x1F))
	{
		instance->version = 0x0;
	}
}

me8200_di_subdevice_t* me8200_di_constr(me_general_dev_t* device, void* reg_base, unsigned int idx, me_lock_t* irq_ctrl_lock, me_lock_t* irq_mode_lock)
{
	me8200_di_subdevice_t* subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me8200_di_subdevice_t), GFP_KERNEL);
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

	subdevice->irq_ctrl_lock = irq_ctrl_lock;
	subdevice->irq_mode_lock = irq_mode_lock;

	// Check firmware version.
	me8200_di_check_version(subdevice, device, reg_base + ME8200_FIRMWARE_VERSION_REG);

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Save the subdevice index.
	subdevice->base.idx = idx;

	//Initialize registers.
	if (idx == 0)
	{
		subdevice->port_reg = reg_base + ME8200_DI_PORT_0_REG;
		subdevice->mask_reg = reg_base + ME8200_DI_MASK_0_REG;
		subdevice->compare_reg = reg_base + ME8200_DI_COMPARE_0_REG;
		subdevice->irq_status_reg = reg_base + ME8200_DI_CHANGE_0_REG;

		subdevice->irq_status_low_reg = reg_base + ME8200_DI_EXTEND_CHANGE_0_LOW_REG;
		subdevice->irq_status_high_reg = reg_base + ME8200_DI_EXTEND_CHANGE_0_HIGH_REG;
	}
	else if (idx == 1)
	{
		subdevice->port_reg = reg_base + ME8200_DI_PORT_1_REG;
		subdevice->mask_reg = reg_base + ME8200_DI_MASK_1_REG;
		subdevice->compare_reg = reg_base + ME8200_DI_COMPARE_1_REG;
		subdevice->irq_status_reg = reg_base + ME8200_DI_CHANGE_1_REG;

		subdevice->irq_status_low_reg = reg_base + ME8200_DI_EXTEND_CHANGE_1_LOW_REG;
		subdevice->irq_status_high_reg = reg_base + ME8200_DI_EXTEND_CHANGE_1_HIGH_REG;
	}
	else
	{
		PERROR("Wrong subdevice idx=%d.\n", idx);
		kfree(subdevice);
		return NULL;
	}

	subdevice->irq_ctrl_reg = reg_base + ME8200_DI_IRQ_CTRL_REG;
	subdevice->irq_mode_reg = reg_base + ME8200_IRQ_MODE_REG;


	// Overload base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8200_di_io_reset_subdevice;
	subdevice->base.me_subdevice_io_irq_start = me8200_di_io_irq_start;
	subdevice->base.me_subdevice_io_irq_wait = me8200_di_io_irq_wait;
	subdevice->base.me_subdevice_io_irq_stop = me8200_di_io_irq_stop;
	subdevice->base.me_subdevice_io_irq_test = me8200_di_io_irq_test;
	subdevice->base.me_subdevice_io_single_config = me8200_di_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8200_di_io_single_read;
	subdevice->base.me_subdevice_query_number_channels = me8200_di_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8200_di_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8200_di_query_subdevice_caps;
	subdevice->base.me_subdevice_query_version_firmware = me8200_di_query_version_firmware;

	subdevice->base.me_subdevice_irq_handle = (subdevice->version == 0) ? me8200_di_irq_handle : me8200_di_irq_handle_EX;

	subdevice->rised = 0;
	subdevice->count = 0;

	return subdevice;
}
