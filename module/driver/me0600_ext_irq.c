/**
 * @file me0600_ext_irq.c
 *
 * @brief The ME-0600(ME-630) external interrupt subdevice instance.
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
# include "me_plx9052_reg.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

# include "me0600_ext_irq_reg.h"
# include "me0600_ext_irq.h"

int me0600_ext_irq_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int me0600_ext_irq_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags);
int me0600_ext_irq_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags);
int me0600_ext_irq_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me0600_ext_irq_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags);
int me0600_ext_irq_query_number_channels(me_subdevice_t* subdevice, int* number);
int me0600_ext_irq_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int me0600_ext_irq_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);


static int me0600_ext_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);

int me0600_ext_irq_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me0600_ext_irq_subdevice_t* instance;
	uint32_t tmp;
	uint8_t dummy;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (me0600_ext_irq_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			ME_SPIN_LOCK (instance->intcsr_lock);
				me_readl(instance->base.dev, &tmp, instance->intcsr);
				switch (instance->base.idx)
				{
					case 0:
						tmp |= PLX9052_INTCSR_LOCAL_INT1_POL | PLX9052_INTCSR_PCI_INT_EN;
						tmp &= ~PLX9052_INTCSR_LOCAL_INT1_EN;
						break;
					case 1:
						tmp |= PLX9052_INTCSR_LOCAL_INT2_POL | PLX9052_INTCSR_PCI_INT_EN;
						tmp &= ~PLX9052_INTCSR_LOCAL_INT2_EN;
						break;
				}
				me_writel(instance->base.dev, tmp, instance->intcsr);
			ME_SPIN_UNLOCK (instance->intcsr_lock);

			//Reset line.
			me_readb(instance->base.dev, &dummy, instance->reset_reg);

			instance->status = irq_status_none;
			instance->count = 0;
			instance->reset_count++;
		ME_UNLOCK_PROTECTOR;
		wake_up_interruptible_all(&instance->wait_queue);
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me0600_ext_irq_io_irq_start(me_subdevice_t* subdevice, struct file* filep, int channel, int irq_source, int irq_edge, int irq_arg, int flags)
{
	me0600_ext_irq_subdevice_t* instance;
	uint32_t tmp;
	uint8_t dummy;

	PDEBUG("executed.\n");

	instance = (me0600_ext_irq_subdevice_t *)subdevice;

	if (flags & ~ME_IO_IRQ_START_DIO_BIT)
	{
		PERROR("Invalid flag specified. Should be ME_IO_IRQ_START_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (irq_arg)
	{
		PERROR("Invalid irq argument specified. Must be 0.\n");
		return ME_ERRNO_INVALID_IRQ_ARG;
	}

	if (irq_edge != ME_IRQ_EDGE_RISING)
	{
		PERROR("Invalid irq edge specified. Must be ME_IRQ_EDGE_RISING.\n");
		return ME_ERRNO_INVALID_IRQ_EDGE;
	}

	if (irq_source && (irq_source != ME_IRQ_SOURCE_DIO_LINE))
	{
		PERROR("Invalid irq source specified. Should be ME_IRQ_SOURCE_DIO_LINE.\n");
		return ME_ERRNO_INVALID_IRQ_SOURCE;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			//Reset line.
			me_readb(instance->base.dev, &dummy, instance->reset_reg);
			ME_SPIN_LOCK(instance->intcsr_lock);
				// Enable IRQ
				me_readl(instance->base.dev, &tmp, instance->intcsr);
				switch (instance->base.idx)
				{
					case 0:
						tmp |= PLX9052_INTCSR_LOCAL_INT1_EN | PLX9052_INTCSR_LOCAL_INT1_POL | PLX9052_INTCSR_PCI_INT_EN;
						break;
					case 1:
						tmp |= PLX9052_INTCSR_LOCAL_INT2_EN | PLX9052_INTCSR_LOCAL_INT2_POL | PLX9052_INTCSR_PCI_INT_EN;
						break;
				}
				me_writel(instance->base.dev, tmp, instance->intcsr);
			ME_SPIN_UNLOCK(instance->intcsr_lock);
			instance->status = irq_status_run;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me0600_ext_irq_io_irq_wait(me_subdevice_t* subdevice, struct file* filep, int channel, int* irq_count, int* value, int time_out, int flags)
{
	me0600_ext_irq_subdevice_t* instance;
	unsigned long int delay = LONG_MAX-2;
	int old_count;
	int old_reset_count;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	instance = (me0600_ext_irq_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_IRQ_WAIT_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel specified. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		if (time_out)
		{
			delay = (time_out * HZ) / 1000;

			if (!delay)
				delay = 1;
			if (delay>LONG_MAX - 2)
				delay = LONG_MAX - 2;
		}

		old_count = (*irq_count) ? *irq_count : instance->count;
		old_reset_count = instance->reset_count;

		if (old_count == instance->count)
		{
			if (wait_event_interruptible_timeout(instance->wait_queue, ((old_count != instance->count) || (old_reset_count != instance->reset_count)), delay) <= 0)
			{
				PERROR("Wait on external interrupt timed out.\n");
				err = ME_ERRNO_TIMEOUT;
			}

			if (instance->status == irq_status_none)
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
		*value = 1;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me0600_ext_irq_io_irq_stop(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me0600_ext_irq_subdevice_t* instance;
	uint32_t tmp;

	PDEBUG("executed.\n");

	instance = (me0600_ext_irq_subdevice_t *) subdevice;

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
			ME_SPIN_LOCK(instance->intcsr_lock);
				me_readl(instance->base.dev, &tmp, instance->intcsr);
				switch (instance->base.idx)
				{
					case 0:
						tmp &= ~PLX9052_INTCSR_LOCAL_INT1_EN;
						break;

					case 1:
						tmp &= ~PLX9052_INTCSR_LOCAL_INT2_EN;
						break;
				}
				me_writel(instance->base.dev, tmp, instance->intcsr);
			ME_SPIN_UNLOCK(instance->intcsr_lock);
			instance->status = irq_status_stop;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me0600_ext_irq_io_irq_test(me_subdevice_t* subdevice, struct file* filep, int channel, int flags)
{
	me0600_ext_irq_subdevice_t* instance;

	PDEBUG("executed.\n");

	instance = (me0600_ext_irq_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_NO_FLAGS.\n");
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

int me0600_ext_irq_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed.\n");

	*number = 1;

	return ME_ERRNO_SUCCESS;
}

int me0600_ext_irq_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_EXT_IRQ;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int me0600_ext_irq_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed.\n");

	*caps = me0600_EXT_IRQ_CAPS;	//ME_CAPS_EXT_IRQ_EDGE_RISING;

	return ME_ERRNO_SUCCESS;
}

static int me0600_ext_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{//Dedicated IRQ handling point.

	uint8_t	tmp;

	me0600_ext_irq_subdevice_t* instance = (me0600_ext_irq_subdevice_t *)subdevice;

	ME_HANDLER_PROTECTOR;
		PDEBUG("executed.\n");

		instance->count++;
		//Reset line.
		me_readb(instance->base.dev, &tmp, instance->reset_reg);
	ME_FREE_HANDLER_PROTECTOR;

	if (instance->status == irq_status_run)
	{
		wake_up_interruptible_all(&instance->wait_queue);
	}

	return ME_ERRNO_SUCCESS;
}

me0600_ext_irq_subdevice_t* me0600_ext_irq_constr(void* plx_reg_base, void* reg_base, unsigned int idx, me_lock_t* intcsr_lock)
{
	me0600_ext_irq_subdevice_t* subdevice;

	PDEBUG("executed.\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me0600_ext_irq_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for 630_ext_irq instance.\n");
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
	subdevice->intcsr_lock = intcsr_lock;

	// Initialize wait queue.
	init_waitqueue_head(&subdevice->wait_queue);

	// Save the subdevice index.
	subdevice->base.idx = idx;

	// Initialize registers.
	subdevice->intcsr = plx_reg_base + PLX9052_INTCSR;
	subdevice->reset_reg = reg_base + ME0600_INT_0_RESET_REG + idx;

	// Initialize the subdevice methods.
	subdevice->base.me_subdevice_io_irq_start = me0600_ext_irq_io_irq_start;
	subdevice->base.me_subdevice_io_irq_wait = me0600_ext_irq_io_irq_wait;
	subdevice->base.me_subdevice_io_irq_stop = me0600_ext_irq_io_irq_stop;
	subdevice->base.me_subdevice_io_irq_test = me0600_ext_irq_io_irq_test;
	subdevice->base.me_subdevice_io_reset_subdevice = me0600_ext_irq_io_reset_subdevice;
	subdevice->base.me_subdevice_query_number_channels = me0600_ext_irq_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me0600_ext_irq_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me0600_ext_irq_query_subdevice_caps;

	subdevice->base.me_subdevice_irq_handle = me0600_ext_irq_handle;

	subdevice->status = irq_status_none;
	subdevice->count = 0;
	subdevice->reset_count = 0;

	return subdevice;
}
