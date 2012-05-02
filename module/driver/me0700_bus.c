/**
 * @file me0700_bus.c
 *
 * @brief Implements parallel bus protocol for ME0700.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author KH (Keith Hartley)
 */

/*
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
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


#include <linux/sched.h>
#include <linux/delay.h>

#include "me_debug.h"
#include "me_error.h"
#include "me_defines.h"

#include "me0700_bus.h"

/**
 * @brief The ME-0700 (Axon) parallel bus protocol. Read from relays module.
 */
static int me0700_read_parallel_bus(me0700_ai_subdevice_t* instance, uint8_t* val);
/**
 * @brief The ME-0700 (Axon) parallel bus protocol. Write to relays module.
 */
static int me0700_write_parallel_bus(me0700_ai_subdevice_t* instance, uint8_t val);
/**
 * @brief The ME-0700 (Axon) parallel bus protocol. Wait for idle mode to end.
 * @return 0 when success. 0x1 when +OVL. 0x2 when -OVL. -ERROR_NO on error.
 */
static int me0700_synchronize_parallel_bus(me0700_ai_subdevice_t* instance);
/**
 * @brief The ME-0700 (Axon) parallel bus protocol. Return internal address in relays module.
 */
static int me0700_set_parallel_bus_address(me0700_ai_subdevice_t* instance, uint8_t valaddr);

int me0700_clear_irq(me0700_ai_subdevice_t * instance, uint8_t val, uint8_t addr, int lock)
{
	int err;

	if (lock) { ME_SPIN_LOCK(&instance->me0700_bus_lock); }
		// Set address
		err = me0700_set_parallel_bus_address(instance, addr);
		if (err)
			goto ERROR;

		err = me0700_write_parallel_bus(instance, val);
		if (err)
			goto ERROR;

ERROR:

    if (lock) { ME_SPIN_UNLOCK(&instance->me0700_bus_lock); }
	return err;
}

int me0700_set_range_relay(me0700_ai_subdevice_t* instance, uint8_t  val, uint8_t addr, int lock)
{
	int err;

	PDEBUG("executed.\n");

	if (lock) { ME_SPIN_LOCK(&instance->me0700_bus_lock); }
		// Set address
		err = me0700_set_parallel_bus_address(instance, addr);
		if (err)
			goto ERROR;

		err = me0700_write_parallel_bus(instance, val);
		if (err)
			goto ERROR;

		err = me0700_synchronize_parallel_bus(instance);
		if (err < 0)
		{
			err = -err;
		}
		else if (err)
		{
			err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		}
ERROR:

    if (lock) { ME_SPIN_UNLOCK(&instance->me0700_bus_lock); }
	return err;
}

int me0700_update_range_relay(me0700_ai_subdevice_t* instance, uint8_t  val, uint8_t addr)
{
	uint8_t status;
	int err;

	PDEBUG("executed.\n");
	ME_SPIN_LOCK(&instance->me0700_bus_lock);
		// Set address
		err = me0700_set_parallel_bus_address(instance, addr);
		if (err)
			goto ERROR;

		err = me0700_read_parallel_bus(instance, &status);
		if (err)
			goto ERROR;
		status &= ME0700_I_STATUS_MASK;
		if ((status == ME0700_I_OVERFLOW_HIGH) || (status == ME0700_I_OVERFLOW_LOW))
		{
			err = me0700_write_parallel_bus(instance, val);
			if (err)
				goto ERROR;

			err = me0700_synchronize_parallel_bus(instance);
			if (err < 0)
			{
				err = -err;
			}
			else if (err)
			{
				err = ME_ERRNO_VALUE_OUT_OF_RANGE;
			}
		}
	ME_SPIN_UNLOCK(&instance->me0700_bus_lock);
ERROR:
	return err;
}

int me0700_get_range_relay_status(me0700_ai_subdevice_t * instance, uint8_t *  val, uint8_t addr, int lock)
{
	int err;

    if (lock) { ME_SPIN_LOCK(&instance->me0700_bus_lock); }
		// Set address
		err = me0700_set_parallel_bus_address(instance, addr);
		if (err)
			goto ERROR;

		err = me0700_read_parallel_bus(instance, val);
		if (err)
			goto ERROR;

		err = me0700_synchronize_parallel_bus(instance);
		if (err < 0)
		{
			err = -err;
		}
		else if (err)
		{
			err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		}
ERROR:

	if (lock) { ME_SPIN_UNLOCK(&instance->me0700_bus_lock); }
	return err;
}

int me0700_reset_relays(me0700_ai_subdevice_t* instance)
{
	uint8_t tmp = ME0700_WRITE_PORT_WRITE |  ME0700_WRITE_PORT_READ | ME0700_WRITE_PORT_RESET;
	int err;

	PDEBUG("executed.\n");

	ME_SPIN_LOCK(&instance->me0700_bus_lock);
		err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
		if (err)
			goto ERROR;

		tmp &= ~ME0700_WRITE_PORT_RESET;
		err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
		if (err)
			goto ERROR;

		tmp |= ME0700_WRITE_PORT_RESET;
		err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
		if (err)
			goto ERROR;

		err = me0700_synchronize_all_relays(instance);
		if (err < 0)
		{
			err = -err;
		}
		else if (err)
		{
			err = ME_ERRNO_VALUE_OUT_OF_RANGE;
		}
ERROR:
	ME_SPIN_UNLOCK(&instance->me0700_bus_lock);
	return err;
}

int me0700_reset_OF_bits(me0700_ai_subdevice_t* instance, uint8_t* mask)
{
	uint8_t OF_val;
	int err;

	if (!mask)
	{
		return ME_ERRNO_SUCCESS;
	}

	// Set address
	err = me0700_get_range_relay_status(instance, &OF_val, ME0700_WRITE_PORT_ADDRESS_IRQ_STATUS, 0);
	if (err)
	{
		OF_val = *mask;
	}

    OF_val = OF_val << 4;

	err = me0700_clear_irq(instance, OF_val, ME0700_WRITE_PORT_ADDRESS_IRQ_RESET, 0);
	if (!err)
		*mask &= OF_val;
	return err;
}

static int me0700_set_parallel_bus_address(me0700_ai_subdevice_t* instance, uint8_t valaddr)
{
	uint32_t tmp = ME0700_WRITE_PORT_WRITE |  ME0700_WRITE_PORT_READ | ME0700_WRITE_PORT_ADDDRESS | ME0700_WRITE_PORT_RESET;
	int err;

	PDEBUG("executed.\n");

	tmp |= (valaddr & 0xF0);
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;
	tmp &= ~ME0700_WRITE_PORT_WRITE;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;
	tmp |= ME0700_WRITE_PORT_WRITE;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;
	tmp &= ~ME0700_WRITE_PORT_ADDDRESS;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);

ERROR:
	return err;
}

static int me0700_read_parallel_bus(me0700_ai_subdevice_t* instance, uint8_t* val)
{
	uint32_t tmp = ME0700_WRITE_PORT_WRITE |  ME0700_WRITE_PORT_READ | ME0700_WRITE_PORT_RESET;
	int err;
	int read_val;

	PDEBUG("executed.\n");

	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;

	tmp &= ~ME0700_WRITE_PORT_READ;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;

	err = instance->data_port->base.me_subdevice_io_single_read((struct me_subdevice* )instance->data_port, NULL, 0, &read_val, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;
	*val = read_val;

	tmp |= ME0700_WRITE_PORT_READ;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
ERROR:
	return err;
}

static int me0700_write_parallel_bus(me0700_ai_subdevice_t* instance, uint8_t val)
{
	uint8_t tmp = ME0700_WRITE_PORT_WRITE |  ME0700_WRITE_PORT_READ | ME0700_WRITE_PORT_RESET;
	int err;

	PDEBUG("executed.\n");

	tmp |= (val & 0xF0);
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;

	tmp &= ~ME0700_WRITE_PORT_WRITE;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);
	if (err)
		goto ERROR;

	tmp |= ME0700_WRITE_PORT_WRITE;
	err = instance->ctrl_port->base.me_subdevice_io_single_write((struct me_subdevice* )instance->ctrl_port, NULL, 0, tmp, 0, ME_IO_SINGLE_TYPE_NO_FLAGS);

ERROR:

	return err;
}

static int me0700_synchronize_parallel_bus(me0700_ai_subdevice_t* instance)
{
	uint8_t tmp = ME0700_I_IDLE;
	int err;
	int security_counter = 0;

	do
	{
        if (in_atomic()) {
            udelay(10);
        } else {
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(1);
        }

		err = me0700_read_parallel_bus(instance, &tmp);
		if (err)
			return -err;
		if (++security_counter > HZ)
		{
			PERROR_CRITICAL("Cann't synchronize! tmp=0x%02x\n", tmp);
			return 0;
		}

		tmp &= ME0700_I_STATUS_MASK;
	}
	while (tmp == ME0700_I_IDLE);

	return tmp;
}

int me0700_synchronize_all_relays(me0700_ai_subdevice_t* instance)
{
	int err;
	int i;

	for (i=0; i<ME0700_I_CHANNELS_NUMBER; ++i)
	{
		err = me0700_set_parallel_bus_address(instance, ME0700_AI_CHANNEL_ADDR[i]);
		me0700_synchronize_parallel_bus(instance);
	}

	return err;
}

