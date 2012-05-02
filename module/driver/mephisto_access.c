/**
 * @file mephisto_access.c
 *
 * @brief Meilhaus access functions to MephistoScope.
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

# ifndef ME_MEPHISTO
# error MEPHISTO only!
# endif

# ifndef MODULE
#  define MODULE
# endif

# ifndef KBUILD_MODNAME
#  define KBUILD_MODNAME KBUILD_STR(mephisto_access)
# endif

# include <linux/module.h>
# include <linux/errno.h>

# include "me_spin_lock.h"
# include "me_debug.h"

# include "mephisto_access.h"

# define USB_WAIT1				0x0031
# define USB_WAIT2				0x6031
# define USB_MAX_RESPONSE_TIME	(HZ << 4)


static void usb_complete(struct urb* urb, struct pt_regs* regs);

const char* MEPHISTO_commands[] = {
	"*SWr",
	"*SRd",
	"*SAm",
	"*SOf",
	"*STm",
	"*SMe",
	"*STr",
	"*SMd",
	"*SRt",
	"*RUN",
	"ZZZZ",
	"*IOW",
	"*IOR",
	"*IDN",	/*as argument use "????" */
	"*RST",
	"*UPD",
	"*CAL",
	NULL
};


int mephisto_cmd(mephisto_usb_device_t* dev, mephisto_cmd_e cmd, MEPHISTO_modes_tu* arg_send, unsigned int arg_send_size, MEPHISTO_modes_tu* arg_recive, unsigned int arg_recive_size)
{
	int err = ME_ERRNO_SUCCESS;
	MEPHISTO_modes_tu* buf_write = NULL;
	MEPHISTO_modes_tu* buf_write_arg = NULL;

	void* buf_read = NULL;
	uint16_t* buf_read_status;
	MEPHISTO_modes_tu* buf_read_arg = NULL;

	usb_context_struct_t* context = NULL;
	int i;

	unsigned int send_size;
	unsigned long int start_read_jiffies;

	struct urb *urb = NULL;

	PDEBUG("executed.\n");

	if (!dev)
	{
		return ME_ERRNO_INTERNAL;
	}

	send_size = (arg_send_size + 1) * sizeof(MEPHISTO_modes_tu);
	buf_write = kmalloc(send_size, GFP_KERNEL);
	if (!buf_write)
	{
		PERROR("Cann't allocate buf_write.\n");
		err = -ENOMEM;
		goto ERROR;
	}
	buf_write_arg = buf_write + 1;
	memcpy(buf_write, MEPHISTO_commands[cmd], sizeof(MEPHISTO_modes_tu));

	for (i=0; i<arg_send_size; ++i)
	{
		buf_write_arg[i].value = cpu_to_le32(arg_send[i].value);
	}

	buf_read = kmalloc(USB_FRAME_SIZE, GFP_KERNEL);
	if (!buf_read)
	{
		PERROR("Cann't allocate in_data_usb structure.\n");
		err = -ENOMEM;
		goto ERROR;
	}
	buf_read_status = buf_read;
	buf_read_arg = buf_read + 2;

	context = kmalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_context structure.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	// Initialize local wait queue ans status.
	init_waitqueue_head(&context->usb_queue);

	urb=usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
	{
		PERROR("Cann't allocate urb structure.\n");
		err = -ENOMEM;
	}
	else
	{
		down(dev->usb_semaphore);
			usb_fill_bulk_urb(	urb,
								dev->dev,
								usb_sndbulkpipe(dev->dev, MEPHISTO_EP_OUT),
								buf_write,
								send_size,
								(usb_complete_t)usb_complete,
								(void *)context);

			context->usb_status = -EINPROGRESS;
			err = usb_submit_urb(urb, GFP_KERNEL);
			if(err)
			{
				PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
			}

			if (wait_event_interruptible_timeout(context->usb_queue, context->usb_status != -EINPROGRESS, USB_TRANSFER_TIMEOUT) <= 0)
			{
				err = -ETIMEDOUT;
				PERROR("Wait for ACK timed out.\n");
				usb_unlink_urb(urb);
			}
			else
			{
				err = context->usb_status;
				PDEBUG_TRANS("send size=%d\n", urb->actual_length);
				PDEBUG_TRANS("cmd: '%c%c%c%c'\n", buf_write->text[3], buf_write->text[2], buf_write->text[1], buf_write->text[0]);
				for (i=0; i<arg_send_size; ++i)
				{
					PDEBUG_TRANS("s%d: %08x >> '%c%c%c%c'\n",
							i,
							buf_write_arg[i].value,
							buf_write_arg[i].text[3], buf_write_arg[i].text[2], buf_write_arg[i].text[1], buf_write_arg[i].text[0]);
					PDEBUG_TRANS_FLOAT("s%d: %d.%06ld\n",
						i,
						(int)buf_write_arg[i].fvalue.value, abs(((int)(1000000 * buf_write_arg[i].fvalue.value)) % 1000000));
				}
			}

			if (context->usb_status == -EPIPE)
			{
				err = -EPIPE;
				PERROR("Broken pipe.\n");
				usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, MEPHISTO_EP_OUT));
			}

			if (signal_pending(current))
			{
				err = -ECANCELED;
				PDEBUG("Aborted by signal.\n");
				usb_kill_urb(urb);
			}

			if (arg_recive && (arg_recive_size > 0))
			{
				start_read_jiffies = jiffies;
				while (!err)
				{
					usb_fill_bulk_urb(	urb,
										dev->dev,
										usb_rcvbulkpipe(dev->dev, MEPHISTO_EP_IN),
										buf_read,
										USB_FRAME_SIZE,
										(usb_complete_t)usb_complete,
										(void *)context);

					context->usb_status = -EINPROGRESS;
					err = usb_submit_urb(urb, GFP_KERNEL);
					if(err)
					{
						PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
					}

					if (wait_event_interruptible_timeout(context->usb_queue, context->usb_status != -EINPROGRESS, USB_TRANSFER_TIMEOUT) <= 0)
					{
						err = -ETIMEDOUT;
						PERROR("Wait for ACK timed out.\n");
						usb_unlink_urb(urb);
					}
					else
					{
						err = context->usb_status;
						if (err)
						{
							PERROR("Error in transmition: %d\n", err);
						}

						if (!err && (urb->actual_length  == (arg_recive_size * sizeof(MEPHISTO_modes_tu)) + 2))
						{
							for (i=0; i<arg_recive_size; ++i)
							{
								arg_recive[i].value = buf_read_arg[i].value;
							}
						}
					}

					if (signal_pending(current))
					{
						err = -ECANCELED;
						PDEBUG("Aborted by signal.\n");
						usb_kill_urb(urb);
						break;
					}

					for (i=0; i<((urb->actual_length - 2) >> 2); ++i)
					{
						arg_recive[i].value = buf_read_arg[i].value;
						PDEBUG_TRANS("r%d: %08x >> '%c%c%c%c'\n",
							i,
							buf_read_arg[i].value,
							buf_read_arg[i].text[3], buf_read_arg[i].text[2], buf_read_arg[i].text[1], buf_read_arg[i].text[0]);
						PDEBUG_TRANS_FLOAT("r%d: %d.%06ld\n",
							i,
							(int)buf_read_arg[i].fvalue.value, abs(((int)(1000000 * buf_read_arg[i].fvalue.value)) % 1000000));
					}

					if (!err && (urb->actual_length  == (arg_recive_size * sizeof(MEPHISTO_modes_tu)) + 2))
					{
						for (i=0; i<arg_recive_size; ++i)
						{
							arg_recive[i].value = buf_read_arg[i].value;
						}

						break;
					}

					if (context->usb_status == -EPIPE)
					{
						err = -EPIPE;
						PERROR("Broken pipe.\n");
						usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, MEPHISTO_EP_IN));
						break;
					}

					if (jiffies - start_read_jiffies > USB_MAX_RESPONSE_TIME)
					{
						err = -ETIMEDOUT;
						PERROR("Read back timed out.\n");
						usb_unlink_urb(urb);
						usb_kill_urb(urb);
						break;
					}
				}
			}

			usb_free_urb(urb);
		up(dev->usb_semaphore);
	}

ERROR:
	if (context)
	{
		kfree(context);
		context = NULL;
	}

	if (buf_write)
	{
		kfree(buf_write);
		buf_write = NULL;
	}

	if (buf_read)
	{
		kfree(buf_read);
		buf_read = NULL;
	}

	return err;
}

void mephisto_endpoints_reset(mephisto_usb_device_t* dev)
{
	PDEBUG("executed.\n");

	down(dev->usb_semaphore);
		usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, MEPHISTO_EP_IN));
		usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, MEPHISTO_EP_OUT));
	up(dev->usb_semaphore);

}

static void usb_complete(struct urb* urb, struct pt_regs* regs)
{
	usb_context_struct_t* context = (usb_context_struct_t *)urb->context;

	if (!context)
	{
		return;
	}

	switch (urb->status)
	{
		case -ESHUTDOWN:
			context->usb_status = urb->status;
		case -ENOENT:
		case -ECONNRESET:
			PDEBUG("USB call canceled. Status=%d\n", -urb->status);
			break;

		case -EINPROGRESS:
			PDEBUG("ERROR! STILL IN PROGRESS! %d\n", -urb->status);
			break;

		case -EOVERFLOW:
			PDEBUG("USB recived too much data (%d). Status=%d\n", urb->actual_length, -urb->status);
		case 0:
			context->usb_status = 0;
			wake_up_interruptible_all(&context->usb_queue);
			break;

		default:
			PERROR("ERROR IN TRANSMISION! %d\n", -urb->status);
			context->usb_status = urb->status;
			wake_up_interruptible(&context->usb_queue);
	}
}

#define MephistoScope_PACKET_SIZE (0x40)
int mephisto_get_packet(mephisto_usb_device_t* dev, struct urb *urb, uint16_t* buf, unsigned int* count)
{
	int err = USB_PACKET_ERROR;

	usb_context_struct_t* context = NULL;

// 	PDEBUG_TRANS("executed.\n");

	if (!dev)
	{
		return USB_PACKET_ERROR;
	}

	context = kmalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_context structure.\n");
		err = USB_PACKET_ERROR;
		goto EXIT;
	}

	// Initialize local wait queue and status.
	init_waitqueue_head(&context->usb_queue);

	down(dev->usb_semaphore);
		usb_fill_bulk_urb(	urb,
							dev->dev,
							usb_rcvbulkpipe(dev->dev, MEPHISTO_EP_IN),
							buf,
							*count,
							(usb_complete_t)usb_complete,
							(void *)context);
		*count = 0;

		context->usb_status = -EINPROGRESS;
		err = usb_submit_urb(urb, GFP_KERNEL);
		if(err)
		{
			PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
			err = USB_PACKET_ERROR;
		}
		else
		{
			if (wait_event_interruptible_timeout(context->usb_queue, context->usb_status != -EINPROGRESS, HZ<<8) <= 0)
			{
				PERROR("Wait for ACK timed out.\n");
				usb_kill_urb(urb);
				schedule_timeout(HZ);
				err = USB_PACKET_ERROR;
			}
			else
			{
				if (signal_pending(current))
				{
					PDEBUG("Aborted by signal.\n");
					usb_kill_urb(urb);
					err = USB_PACKET_FINISH;
				}
				else
				{
					err = USB_PACKET_ERROR;
					if (context->usb_status == -EPIPE)
					{
						PERROR("Broken pipe.\n");
						usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, MEPHISTO_EP_IN));
					}
					else if (context->usb_status == -ETIMEDOUT)
					{
						PERROR("Transmision timeout.\n");
						mephisto_endpoints_reset(dev);
					}
					else if (context->usb_status)
					{
						PERROR("Readback error.\n");
					}
					else
					{
						if (urb->actual_length > 2)
						{
							*count = urb->actual_length >> 1;
							err = USB_PACKET_RECIVED;
						}
						else
						{
							err = USB_PACKET_WAIT;
						}
					}
				}
			}
		}
	up(dev->usb_semaphore);

EXIT:
	if (context)
	{
		kfree(context);
		context = NULL;
	}

	return err;
}


int read_from_GPIO(mephisto_usb_device_t* dev, GPIO_arg_t* recive)
{// This cannot be executet when stream runs!
	SetMode_arg_t		mode_send;
	SetMode_arg_t		mode_recive;
	Setup_arg_recive_t	setup_recive;

	PDEBUG("executed.\n");

	memcpy(&mode_send, "1DMV", sizeof(SetMode_arg_t));
	if (mephisto_cmd(dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
	{
		return ME_ERRNO_COMMUNICATION;
	}

	if (mephisto_cmd(dev, MEPHISTO_CMD_SetupRead, NULL, 0, (void *)&setup_recive, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Reading GPIO failed.\n");
		recive->data_GPIO.value = 0;
		recive->dir_GPIO.value = 0;
		return ME_ERRNO_COMMUNICATION;
	}

	recive->data_GPIO.value = setup_recive.data_GPIO.value;
	recive->dir_GPIO.value = setup_recive.dir_GPIO.value;

	return ME_ERRNO_SUCCESS;
}

int write_to_GPIO(mephisto_usb_device_t* dev, GPIO_arg_t send, GPIO_arg_t* recive)
{// This cannot be executet when stream runs!
	SetMode_arg_t		mode_send;
	SetMode_arg_t		mode_recive;
	Setup_arg_send_t	setup_send;
	Setup_arg_recive_t	setup_send_return;

	Setup_arg_recive_t	setup_recive;

	PDEBUG("executed.\n");

	memcpy(&mode_send, "1DMV", sizeof(SetMode_arg_t));
	if (mephisto_cmd(dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
	{
		return ME_ERRNO_COMMUNICATION;
	}
	mephisto_cmd(dev, MEPHISTO_CMD_SetupRead, NULL, 0, (void *)&setup_recive, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu));

	setup_send.trigger_channel.value = 0;
	memset(&setup_send.trigger_type, 'M', 4);
	setup_send.amplitude_0 = uvolts_to_float(20000000);
	setup_send.amplitude_1 = uvolts_to_float(20000000);
	setup_send.offset_0.value = 0;
	setup_send.offset_1.value = 0;
	setup_send.time_base = uvolts_to_float(10);
	setup_send.memory_depth = uvolts_to_float(1000000);
	setup_send.trigger_point.value = 0;
	setup_send.upper_trigger_level.value = 0;
	setup_send.lower_trigger_level.value = 0;

	setup_send.data_GPIO = send.data_GPIO;
	setup_send.dir_GPIO = send.dir_GPIO;

	if (mephisto_cmd(dev, MEPHISTO_CMD_SetupWrite, (void *)&setup_send, sizeof(Setup_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&setup_send_return, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Reading GPIO failed.\n");
		if (recive)
		{
			recive->data_GPIO.value = 0;
			recive->dir_GPIO.value = 0;
		}
		return ME_ERRNO_COMMUNICATION;
	}

	if (recive)
	{
		recive->data_GPIO.value = setup_send_return.data_GPIO.value;
		recive->dir_GPIO.value = setup_send_return.dir_GPIO.value;
	}

	return ME_ERRNO_SUCCESS;
}

MEPHISTO_modes_tu uvolts_to_float(const int value)
{
	MEPHISTO_modes_tu ret_value;
	uint64_t fraction;
	int exponent = 118;

	ret_value.value = 0;
	if (value > 0)
	{
		ret_value.fvalue.value_sign = 0;

		fraction = value;
	}
	else if (value < 0)
	{
		ret_value.fvalue.value_sign = 1;

		fraction = -value;
	}

	else
	{
		return ret_value;
	}

	fraction <<= 32;
	(void) do_div(fraction, 1000000);

	while (fraction & ~0x0FFFFFF)
	{
		fraction >>= 1;
		exponent++;
	}
	while (!(fraction & (1 << 23)))
	{
		fraction <<= 1;
		exponent--;
	}

	ret_value.fvalue.value_fraction =  fraction & 0x07FFFFF;
	ret_value.fvalue.value_exponent = exponent;

	return ret_value;
}

MEPHISTO_modes_tu float_to_uvolts(const MEPHISTO_modes_tu value)
{
	MEPHISTO_modes_tu ret_value;

	uint64_t fraction;
	int exponent;

	fraction = value.fvalue.value_fraction | 0x800000;
	exponent = value.fvalue.value_exponent - 127;

	fraction *= 1000000;
	if (exponent > 0)
	{
		fraction <<= exponent;
	}
	else if (exponent < 0)
	{
		fraction >>= -exponent;
	}

	if (fraction & (1 << 22))
	{
		fraction >>= 23;
		fraction += 1;
	}
	else
	{
		fraction >>= 23;
	}

	ret_value.value =  fraction & 0xFFFFFFFF;

	if (value.fvalue.value_sign == 1)
	{
		ret_value.value = -ret_value.value;
	}

	return ret_value;
}

MEPHISTO_modes_tu int_to_float(const int value)
{
	MEPHISTO_modes_tu ret_value;
	uint64_t fraction;
	int exponent = 127;

	ret_value.value = 0;
	if (value > 0)
	{
		ret_value.fvalue.value_sign = 0;

		fraction = value;
	}
	else if (value < 0)
	{
		ret_value.fvalue.value_sign = 1;

		fraction = -value;
	}

	else
	{
		return ret_value;
	}

	fraction <<= 23;

	while (fraction & ~0x0FFFFFF)
	{
		fraction >>= 1;
		exponent++;
	}
	while (!(fraction & (1 << 23)))
	{
		fraction <<= 1;
		exponent--;
	}

	ret_value.fvalue.value_fraction =  fraction & 0x07FFFFF;
	ret_value.fvalue.value_exponent = exponent;

	return ret_value;
}

MEPHISTO_modes_tu float_to_int(const MEPHISTO_modes_tu value)
{
	MEPHISTO_modes_tu ret_value;

	uint64_t fraction;
	int exponent;

	fraction = value.fvalue.value_fraction | 0x800000;
	exponent = value.fvalue.value_exponent - 127;

	if (exponent > 0)
	{
		fraction <<= exponent;
	}
	else if (exponent < 0)
	{
		fraction >>= -exponent;
	}

	if (fraction & (1 << 22))
	{
		fraction >>= 23;
		fraction += 1;
	}
	else
	{
		fraction >>= 23;
	}

	ret_value.value =  fraction & 0xFFFFFFFF;

	if (value.fvalue.value_sign == 1)
	{
		ret_value.value = -ret_value.value;
	}

	return ret_value;
}

int set_offset(mephisto_usb_device_t* dev, const int channel, const MEPHISTO_modes_tu amplitude, MEPHISTO_modes_tu* value)
{// This cannot be executet when stream runs!
	SetMode_arg_t		mode_send;
	SetMode_arg_t		mode_recive;
	Setup_arg_send_t	setup_send;
	Setup_arg_recive_t	setup_send_return;

	Setup_arg_recive_t	setup_recive;

	PDEBUG("executed.\n");

	memcpy(&mode_send, "0ASO", sizeof(SetMode_arg_t));
	if (mephisto_cmd(dev, MEPHISTO_CMD_SetMode, (void *)&mode_send, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu), (void *)&mode_recive, sizeof(SetMode_arg_t) / sizeof(MEPHISTO_modes_tu)))
	{
		return ME_ERRNO_COMMUNICATION;
	}
	mephisto_cmd(dev, MEPHISTO_CMD_SetupRead, NULL, 0, (void *)&setup_recive, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu));

	setup_send.trigger_channel.value = 0;
	memset(&setup_send.trigger_type, 'M', 4);
	setup_send.amplitude_0 = (channel == 0) ? uvolts_to_float(amplitude.value) : setup_recive.amplitude_0;
	setup_send.amplitude_1 = (channel == 1) ? uvolts_to_float(amplitude.value) : setup_recive.amplitude_1;
	setup_send.offset_0 = (channel == 0) ? uvolts_to_float(value->value) : setup_recive.offset_0;
	setup_send.offset_1 = (channel == 1) ? uvolts_to_float(value->value) : setup_recive.offset_1;
	setup_send.time_base = setup_recive.time_base;
	setup_send.memory_depth = setup_recive.memory_depth;
	setup_send.trigger_point = setup_recive.trigger_point;
	setup_send.upper_trigger_level.value = 0;
	setup_send.lower_trigger_level.value = 0;

	setup_send.data_GPIO = setup_recive.data_GPIO;
	setup_send.dir_GPIO = setup_recive.dir_GPIO;

	if (mephisto_cmd(dev, MEPHISTO_CMD_SetupWrite, (void *)&setup_send, sizeof(Setup_arg_send_t) / sizeof(MEPHISTO_modes_tu), (void *)&setup_send_return, sizeof(Setup_arg_recive_t) / sizeof(MEPHISTO_modes_tu)))
	{
		PERROR("Reading configuration failed.\n");
		if (value)
		{
			value->value = 0;
		}
		return ME_ERRNO_COMMUNICATION;
	}

	if (value)
	{
		value->value = float_to_uvolts((channel == 0) ? setup_send_return.offset_0 : setup_send_return.offset_1).value;
	}

	return ME_ERRNO_SUCCESS;
}

