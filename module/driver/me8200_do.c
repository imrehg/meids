/**
 * @file me8200_do.c
 *
 * @brief The ME-8200 digital output subdevice instance.
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
# include "me8200_do_reg.h"
# include "me8200_do.h"

/// Defines
int me8200_do_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
int me8200_do_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags);
int me8200_do_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8200_do_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me8200_do_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me8200_do_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int me8200_do_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8200_do_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int me8200_do_query_number_channels(me_subdevice_t* subdevice, int* number);
int me8200_do_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me8200_do_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
int me8200_do_query_version_firmware(me_subdevice_t* subdevice, int* version);

static void me8200_do_check_version(me8200_do_subdevice_t* instance, me_general_dev_t* device, void* addr);


int me8200_do_io_reset_subdevice(struct me_subdevice* subdevice, struct file* filep, int flags)
{
	me8200_do_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (me8200_do_subdevice_t *)subdevice;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			me_writeb(instance->base.dev, 0x00, instance->port_reg);
			ME_SPIN_LOCK(instance->irq_mode_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_mode_reg);
				tmp &= ~(ME8200_IRQ_MODE_BIT_ENABLE_POWER << (ME8200_IRQ_MODE_POWER_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_mode_reg);
			ME_SPIN_UNLOCK(instance->irq_mode_lock);
			instance->rised = -1;
			instance->count = 0;
		ME_UNLOCK_PROTECTOR;
	wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{
	me8200_do_subdevice_t* instance;
	uint8_t tmp;

	if (flags & ~ME_IO_IRQ_START_DIO_BYTE)
	{
		PERROR("Invalid flag specified.\n");
			PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_DIO_BYTE.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (irq_arg)
	{
		PERROR("Invalid irq argument specified. Must be 0.\n");
		return ME_ERRNO_INVALID_IRQ_EDGE;
	}

	if (irq_edge != ME_IRQ_EDGE_NOT_USED)
	{
		PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_NOT_USED.\n");
		return ME_ERRNO_INVALID_IRQ_EDGE;
	}

	if (irq_source && (irq_source != ME_IRQ_SOURCE_DIO_OVER_TEMP))
	{
		PERROR("Invalid interrupt source specified. Should be ME_IRQ_SOURCE_DIO_OVER_TEMP.\n");
		return ME_ERRNO_INVALID_IRQ_SOURCE;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK(instance->irq_mode_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_mode_reg);
				tmp |= ME8200_IRQ_MODE_BIT_ENABLE_POWER << (ME8200_IRQ_MODE_POWER_SHIFT * instance->base.idx);
				me_writeb(instance->base.dev, tmp, instance->irq_mode_reg);
			ME_SPIN_UNLOCK(instance->irq_mode_lock);
			instance->rised = 0;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags)
{
	me8200_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	unsigned long int delay = LONG_MAX - 2;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_WAIT_NO_FLAGS\n");
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

			if (wait_event_interruptible_timeout(instance->wait_queue, (instance->rised != 0), delay) <= 0)
			{
				PERROR("Wait on external interrupt timed out.\n");
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

		instance->rised = 0;
		*irq_count = instance->count;
		*value = 0;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8200_do_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8200_do_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *) subdevice;

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
			ME_SPIN_LOCK(instance->irq_mode_lock);
				me_readb(instance->base.dev, &tmp, instance->irq_mode_reg);
				tmp &= ~(ME8200_IRQ_MODE_BIT_ENABLE_POWER << (ME8200_IRQ_MODE_POWER_SHIFT * instance->base.idx));
				me_writeb(instance->base.dev, tmp, instance->irq_mode_reg);
			ME_SPIN_UNLOCK(instance->irq_mode_lock);
			instance->rised = 0;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me8200_do_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *) subdevice;

	if (flags)
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

int me8200_do_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me8200_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *)subdevice;


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

	if (single_config != ME_SINGLE_CONFIG_DIO_OUTPUT)
	{
		PERROR("Invalid port direction specified. Must be ME_SINGLE_CONFIG_DIO_OUTPUT.\n");
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8200_do_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8200_do_subdevice_t* instance;
	uint8_t tmp;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *)subdevice;

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
				PERROR("Invalid bits number specified. Must be 0.\n");
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

int me8200_do_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	me8200_do_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	uint8_t tmp = 0;

	PDEBUG("executed.\n");

	instance = (me8200_do_subdevice_t *) subdevice;

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
				PERROR("Invalid bits number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			me_readb(instance->base.dev, &tmp, instance->port_reg);
			switch (flags)
			{
				case ME_IO_SINGLE_TYPE_DIO_BIT:
					tmp = value ? (tmp | (0x1 << channel)) : (tmp & ~(0x1 << channel));
					break;

				case ME_IO_SINGLE_TYPE_NO_FLAGS:
				case ME_IO_SINGLE_TYPE_DIO_BYTE:
					tmp = value;
					break;
			}
			me_writeb(instance->base.dev, tmp, instance->port_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8200_do_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed.\n");

	*number = 8;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_DO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = ME_CAPS_DIO_OVER_TEMP_IRQ;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_query_version_firmware(me_subdevice_t* subdevice, int* version)
{
	me8200_do_subdevice_t *instance = (me8200_do_subdevice_t *) subdevice;
	PDEBUG("executed.\n");

	*version = instance->version;

	return ME_ERRNO_SUCCESS;
}

int me8200_do_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{
	me8200_do_subdevice_t* instance;
	uint8_t ctrl;

	instance = (me8200_do_subdevice_t *) subdevice;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed.\n");

		instance->rised = 1;
		instance->count++;

		me_readb(instance->base.dev, &ctrl, instance->irq_mode_reg);
		ctrl |= ME8200_IRQ_MODE_BIT_CLEAR_POWER << instance->base.idx;
		me_writeb(instance->base.dev, ctrl, instance->irq_mode_reg);
		ctrl &= ~(ME8200_IRQ_MODE_BIT_CLEAR_POWER << instance->base.idx);
		me_writeb(instance->base.dev, ctrl, instance->irq_mode_reg);
	ME_FREE_HANDLER_PROTECTOR;

	wake_up_interruptible_all(&instance->wait_queue);

	return ME_ERRNO_SUCCESS;
}

static void me8200_do_check_version(me8200_do_subdevice_t* instance, me_general_dev_t* device, void* addr)
{
	uint8_t tmp;

	PDEBUG("executed.\n");
	me_readb(device, &tmp, addr);
	instance->version = 0x000000FF & tmp;
	PINFO("me8200 firmware version: %d\n", instance->version);

	/// @note Fix for wrong values in this registry.
	if ((instance->version<0x7) || (instance->version>0x1F))
		instance->version = 0x0;
}

me8200_do_subdevice_t* me8200_do_constr(me_general_dev_t* device, void* reg_base, unsigned int idx, me_lock_t* irq_mode_lock)
{
	me8200_do_subdevice_t *subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me8200_do_subdevice_t), GFP_KERNEL);
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

	subdevice->irq_mode_lock = irq_mode_lock;

	// Check firmware version.
	me8200_do_check_version(subdevice, device, reg_base + ME8200_FIRMWARE_VERSION_REG);

	// Initialize the wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Save the index of the digital output.
	subdevice->base.idx = idx;

	// Initialize the registers.
	if (idx == 0)
	{
		subdevice->port_reg = reg_base + ME8200_DO_PORT_0_REG;
	}
	else if (idx == 1)
	{
		subdevice->port_reg = reg_base + ME8200_DO_PORT_1_REG;
	}
	else
	{
		PERROR("Wrong subdevice idx=%d.\n", idx);
		kfree(subdevice);
		return NULL;
	}

	subdevice->irq_mode_reg = reg_base + ME8200_IRQ_MODE_REG;
	subdevice->irq_status_reg = reg_base + ME8200_DO_IRQ_STATUS_REG;

	// Overload base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8200_do_io_reset_subdevice;
	subdevice->base.me_subdevice_io_irq_start = me8200_do_io_irq_start;
	subdevice->base.me_subdevice_io_irq_wait = me8200_do_io_irq_wait;
	subdevice->base.me_subdevice_io_irq_stop = me8200_do_io_irq_stop;
	subdevice->base.me_subdevice_io_irq_test = me8200_do_io_irq_test;
	subdevice->base.me_subdevice_io_single_config = me8200_do_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8200_do_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me8200_do_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me8200_do_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8200_do_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8200_do_query_subdevice_caps;
	subdevice->base.me_subdevice_query_version_firmware = me8200_do_query_version_firmware;

	subdevice->base.me_subdevice_irq_handle = me8200_do_irq_handle;

	subdevice->rised = -1;
	subdevice->count = 0;

	return subdevice;
}
