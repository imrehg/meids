/**
 * @file mephisto_access.h
 *
 * @brief Meilhaus access functions for MephistoScope.
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

#ifdef __KERNEL__

# ifndef _MEPHISTO_ACCESS_H_
#  define _MEPHISTO_ACCESS_H_

#  include <asm/thread_info.h>
#  include <linux/sched.h>
#  include <linux/spinlock.h>
#  include <linux/wait.h>
#  include <linux/usb.h>

#  include "mephisto_defines.h"

// 0.125s
#  define USB_TRANSFER_TIMEOUT 	(HZ>>3)
// 0.0625s
#  define USB_CLEAR_TIMEOUT 	(HZ>>4)

# define USB_FRAME_SIZE			(0x40)

# define USB_PACKET_WAIT		(0x00)
# define USB_PACKET_RECIVED		(0x01)
# define USB_PACKET_FINISH		(0x02)
# define USB_PACKET_ERROR		(0x03)

typedef struct mephisto_usb_device
{
	struct usb_device*	dev;

	union
	{
		struct
		{
			uint16_t	vendor;		/// Meilhaus vendor id.
			uint16_t	device;		/// Meilhaus device id.
		}__attribute__((packed));
		uint32_t		IDs;		/// Meilhaus vendor and device combined id.
	};

	uint8_t 			hw_revision;/// Hardware revision of the device.
	uint32_t			serial_no;	/// Serial number of the device.

	// Hardware access
	struct semaphore* 	usb_semaphore;
}mephisto_usb_device_t;

typedef struct //usb_context_struct
{
	wait_queue_head_t usb_queue;
	volatile int usb_status;
} usb_context_struct_t;


int mephisto_cmd(mephisto_usb_device_t* dev, mephisto_cmd_e cmd, MEPHISTO_modes_tu* arg_send, unsigned int arg_send_size, MEPHISTO_modes_tu* arg_recive, unsigned int arg_recive_size);
int mephisto_get_packet(mephisto_usb_device_t* dev, struct urb *urb, uint16_t* buf, unsigned int* count);
void mephisto_endpoints_reset(mephisto_usb_device_t* dev);

int read_from_GPIO(mephisto_usb_device_t* dev, GPIO_arg_t* recive);
int write_to_GPIO(mephisto_usb_device_t* dev, GPIO_arg_t send, GPIO_arg_t* recive);
int set_offset(mephisto_usb_device_t* dev, const int channel, const MEPHISTO_modes_tu amplitude, MEPHISTO_modes_tu* value);

MEPHISTO_modes_tu int_to_float(const int);
MEPHISTO_modes_tu float_to_int(const MEPHISTO_modes_tu);

MEPHISTO_modes_tu uvolts_to_float(const int);
MEPHISTO_modes_tu float_to_uvolts(const MEPHISTO_modes_tu);

# endif	//_MEPHISTO_ACCESS_H_
#endif	//__KERNEL__
