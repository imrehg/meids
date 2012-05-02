/**
 * @file mephisto_dio.c
 * @brief The MephistoScope digital input/output subdevice instance.
 * @note Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# include "mephisto_access.h"
# include "mephisto_dio.h"

int mephisto_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);
int mephisto_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
int mephisto_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int mephisto_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags);
int mephisto_dio_query_number_channels(me_subdevice_t* subdevice, int* number);
int mephisto_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
int mephisto_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);

int mephisto_dio_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	mephisto_dio_subdevice_t* instance;

	instance = (mephisto_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	return mephisto_dio_io_single_config(subdevice, filep, 0, ME_SINGLE_CONFIG_DIO_INPUT, ME_REF_NONE, ME_TRIG_CHAN_NONE, ME_TRIG_TYPE_NONE, ME_TRIG_EDGE_NONE, ME_IO_SINGLE_CONFIG_DIO_DWORD);
}

int mephisto_dio_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	mephisto_dio_subdevice_t* instance;
	//Config
	GPIO_arg_t send;
	GPIO_arg_t recive;

	int err = ME_ERRNO_SUCCESS;

	instance = (mephisto_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_CONFIG_DIO_BIT:
			if (channel<0 || channel>23)
			{
				PERROR("Invalid channel specified. Must be between 0 and 23.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_CONFIG_DIO_BYTE:
			if (channel<0 || channel>2)
			{
				PERROR("Invalid channel specified. Must be between 0 and 2.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_CONFIG_NO_FLAGS:
			if (channel<0 || channel>1)
			{
				PERROR("Invalid channel specified. Must be between 0 and 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_CONFIG_DIO_DWORD:
			if (channel!=0)
			{
				PERROR("Invalid channel specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified. Should be ME_IO_SINGLE_CONFIG_DIO_BIT, ME_IO_SINGLE_CONFIG_DIO_BYTE,  ME_IO_SINGLE_CONFIG_DIO_WORD or ME_IO_SINGLE_CONFIG_DIO_DWORD.\n");
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

	ME_SUBDEVICE_ENTER
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				send.data_GPIO.value = instance->data_copy;
				send.dir_GPIO.value = instance->cfg_copy;

				switch (flags)
				{
					case ME_IO_SINGLE_CONFIG_DIO_BIT:
						send.data_GPIO.value |= (0x01 << channel);
						if (single_config == ME_SINGLE_CONFIG_DIO_OUTPUT)
						{
							send.dir_GPIO.value |= (0x01 << channel);
						}
						else
						{
							send.dir_GPIO.value &= ~(0x01 << channel);
						}
						break;

					case ME_IO_SINGLE_CONFIG_DIO_BYTE:
						send.data_GPIO.value |= (0x00FF << (channel << 3));
						if (single_config == ME_SINGLE_CONFIG_DIO_OUTPUT)
						{
							send.dir_GPIO.value |= (0x00FF << (channel << 3));
						}
						else
						{
							send.dir_GPIO.value &= ~(0x00FF << (channel << 3));
						}
						break;

					case ME_IO_SINGLE_CONFIG_DIO_WORD:
						send.data_GPIO.value |= (0x00FFFF << (channel << 4));
						if (single_config == ME_SINGLE_CONFIG_DIO_OUTPUT)
						{
							send.dir_GPIO.value |= (0x00FFFF << (channel << 4));
						}
						else
						{
							send.dir_GPIO.value &= ~(0x00FFFF << (channel << 4));
						}
						break;

					case ME_IO_SINGLE_CONFIG_DIO_DWORD:
					default:
						send.data_GPIO.value = 0x00FFFFFF;
						if (single_config == ME_SINGLE_CONFIG_DIO_OUTPUT)
						{
							send.dir_GPIO.value = 0x00FFFFFF;
						}
						else
						{
							send.dir_GPIO.value = 0x00;
						}
				}

				if (write_to_GPIO(instance->base.dev, send, &recive))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}
				if (send.dir_GPIO.value != recive.dir_GPIO.value)
				{
					PERROR("DIO can not be configured. Conflict with AI subdevice.\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
				}

				instance->cfg_copy = send.dir_GPIO.value;
				instance->data_copy = (send.data_GPIO.value | ~(instance->cfg_copy)) & 0x00FFFFFF;
			}
			else
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
			}
ERROR:
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_dio_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	mephisto_dio_subdevice_t* instance;
	GPIO_arg_t recive;
	int err = ME_ERRNO_SUCCESS;

	instance = (mephisto_dio_subdevice_t *)subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel > 23))
			{
				PERROR("Invalid channel number specified. Must be between 0 and 23.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel > 2))
			{
				PERROR("Invalid channel number specified. Must be between 0 and 2.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if ((channel < 0) || (channel > 1))
			{
				PERROR("Invalid channel number specified. Must be between 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_DWORD:
			if (channel != 0)
			{
				PERROR("Invalid channel number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				if (read_from_GPIO(instance->base.dev, &recive))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}

				switch (flags)
				{
					case ME_IO_SINGLE_TYPE_DIO_BIT:
						*value = recive.data_GPIO.value & (0x0001 << channel);
						break;

					case ME_IO_SINGLE_TYPE_DIO_BYTE:
						*value = (recive.data_GPIO.value >> (channel << 3)) & 0xFF;
						break;

					case ME_IO_SINGLE_TYPE_DIO_WORD:
						*value = (recive.data_GPIO.value >> (channel << 4)) & 0xFFFF;
						break;

					case ME_IO_SINGLE_TYPE_NO_FLAGS:
					case ME_IO_SINGLE_TYPE_DIO_DWORD:
						*value = recive.data_GPIO.value & 0x00FFFFFF;
						break;
				}
			}
			else
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
			}
ERROR:
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_dio_io_single_write(me_subdevice_t* subdevice, struct file* filep, int channel, int value, int time_out, int flags)
{
	mephisto_dio_subdevice_t* instance;
	GPIO_arg_t send;
	int err = ME_ERRNO_SUCCESS;

	instance = (mephisto_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	switch (flags)
	{
		case ME_IO_SINGLE_TYPE_DIO_BIT:
			if ((channel < 0) || (channel > 23))
			{
				PERROR("Invalid channel number specified. Must be between 0 and 23.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_BYTE:
			if ((channel < 0) || (channel > 2))
			{
				PERROR("Invalid channel number specified. Must be between 0 and 2.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_DIO_WORD:
			if ((channel < 0) || (channel > 1))
			{
				PERROR("Invalid channel number specified. Must be between 0 or 1.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		case ME_IO_SINGLE_TYPE_NO_FLAGS:
		case ME_IO_SINGLE_TYPE_DIO_DWORD:
			if (channel != 0)
			{
				PERROR("Invalid channel number specified. Must be 0.\n");
				return ME_ERRNO_INVALID_CHANNEL;
			}
			break;

		default:
			PERROR("Invalid flags specified.\n");
			return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		down(instance->device_semaphore);
			if (*instance->status <= MEPHISTO_AI_STATUS_configured)
			{
				send.data_GPIO.value = instance->data_copy;
				send.dir_GPIO.value = instance->cfg_copy;

				switch (flags)
				{
					case ME_IO_SINGLE_TYPE_DIO_BIT:
						send.data_GPIO.value = value ? (send.data_GPIO.value | (0x1 << channel)) : (send.data_GPIO.value & ~(0x1 << channel));
						break;

					case ME_IO_SINGLE_TYPE_DIO_BYTE:
						send.data_GPIO.value &= ~(0x00FF << (channel << 3));
						send.data_GPIO.value |= (value & 0x00FF) << (channel << 3);
						break;

					case ME_IO_SINGLE_TYPE_DIO_WORD:
						send.data_GPIO.value &= ~(0x00FFFF << (channel << 4));
						send.data_GPIO.value |= (value & 0x00FFFF) << (channel << 4);
						break;

					case ME_IO_SINGLE_TYPE_NO_FLAGS:
					case ME_IO_SINGLE_TYPE_DIO_DWORD:
						send.data_GPIO.value &= ~0x00FFFFFF;
						send.data_GPIO.value |= value & 0x00FFFFFF;
						break;
				}

				send.data_GPIO.value &= 0x00FFFFFF;
				if (write_to_GPIO(instance->base.dev, send, NULL))
				{
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
				}
			}
			else
			{
				err = ME_ERRNO_SUBDEVICE_BUSY;
			}
ERROR:
		up(instance->device_semaphore);
	ME_SUBDEVICE_EXIT;

	return err;
}

int mephisto_dio_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	PDEBUG("executed idx=%d.\n", ((mephisto_dio_subdevice_t *) subdevice)->base.idx);

	*number = 24;

	return ME_ERRNO_SUCCESS;
}

int mephisto_dio_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	mephisto_dio_subdevice_t* instance;
	instance = (mephisto_dio_subdevice_t *) subdevice;

	PDEBUG("executed idx=%d.\n", instance->base.idx);

	*type = ME_TYPE_DIO;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

int mephisto_dio_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	PDEBUG("executed idx=%d.\n", ((mephisto_dio_subdevice_t *) subdevice)->base.idx);

	*caps = MEPHISTO_DIO_CAPS;

	return ME_ERRNO_SUCCESS;
}

mephisto_dio_subdevice_t* mephisto_dio_constr(unsigned int idx, mephisto_AI_status_e* status, struct semaphore* device_semaphore)
{
	mephisto_dio_subdevice_t* subdevice;

	PDEBUG("executed idx=%d.\n", idx);

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(mephisto_dio_subdevice_t), GFP_KERNEL);
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

	subdevice->status = status;

	// Initialize spin locks.
	subdevice->device_semaphore = device_semaphore;

	// Save digital i/o index.
	subdevice->base.idx = idx;

	// Override base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = mephisto_dio_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = mephisto_dio_io_single_config;
	subdevice->base.me_subdevice_io_single_read = mephisto_dio_io_single_read;
	subdevice->base.me_subdevice_io_single_write = mephisto_dio_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = mephisto_dio_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = mephisto_dio_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = mephisto_dio_query_subdevice_caps;

	return subdevice;
}
