/**
 * @file NET2282_access.h
 *
 * @brief Meilhaus access functions for NET2282 USB-PCI controler header file.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/***************************************************************************
 *   Copyright (C) 2008 by Krzysztof Gantzke                               *
 *   k.gantzke@meilhaus.de                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef __KERNEL__

# ifndef _NET2282_ACCESS_H_
#  define _NET2282_ACCESS_H_

#  include <asm/thread_info.h>
#  include <linux/sched.h>
#  include <linux/spinlock.h>
#  include <linux/wait.h>

#  include "NET2282_defines.h"

/// @note NET2282_MEM_BASE must be biger than 0x00010000 AND NET2282_IO_BASE.
#  define NET2282_MEM_BASE				0x01000000
/// @note NET2282_IO_BASE must be lower than 0x00010000. NET2282_IO_BASE MUSTN'T be set to 0x0. 0x0 is reserved for unused regions.
#  define NET2282_IO_BASE				0x00000100

#  define USB_CONTEXT_READ_ADDR 		0x0AA0
#  define USB_CONTEXT_READ_DATA 		0x0BB0
#  define USB_CONTEXT_WRITE 			0x0CC0

// 0.25s
#  define USB_DMA_TRANSFER_TIMEOUT		(HZ>>2)
// 0.125s
#  define USB_TRANSFER_TIMEOUT 			(HZ>>3)

	typedef struct //out_usb_struct
	{
		uint16_t PCIMSTCTL;
		uint32_t PCIMSTADDR;
		uint32_t PCIMSTDATA;
	}__attribute__((packed)) out_usb_struct_t;

	typedef struct //in_addr_usb_struct
	{
		uint16_t PCIMSTCTL;
		uint32_t PCIMSTADDR;
	}__attribute__ ((packed)) in_addr_usb_struct_t;

	typedef struct //in_data_usb_struct
	{
		uint32_t PCIMSTDATA;
	}__attribute__((packed)) in_data_usb_struct_t;

	typedef struct //usb_context_struct
	{
		wait_queue_head_t usb_queue;
		volatile int status;
	} usb_context_struct_t;

	/// IRQ
#   define USB_GHOST_INTERRUPT		0xCA112282
#   define USB_CONTEXT_INTERRUPT	0xCA110000
#   define USB_CANCEL_INTERRUPT		0xCA11DEAD
#   define USB_ENDED_INTERRUPT		0xDEADDEAD

	typedef struct //interrupt_usb_struct
	{
		uint32_t IRQSTAT;
	}__attribute__((packed)) interrupt_usb_struct_t;

	typedef struct //usb_IRQ_context_struct
	{
		struct urb* irq_urb;

		volatile int status;
		wait_queue_head_t usb_queue;

		long long int counter;
	} usb_IRQ_context_struct_t;

	struct NET2282_usb_device
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

		struct semaphore* 	usb_DMA_semaphore;	/// DMA needs exlusive access to NET2282 <- global lock
		struct semaphore 	usb_IRQ_semaphore;	/// IRQ handlers shouldn't be interrupted by control task <- device context lock

		atomic_t usb_transfer_status;
	};

	int NET2282_NET2282_cfg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr);
	int NET2282_NET2282_cfg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr);

	// Access to NET2282 control configuration registers (32b, memory mapped [0x000 - 0x3FF]).
	int NET2282_NET2282_reg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr);
	int NET2282_NET2282_reg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr);

	// Access to NET2282 indexed control configuration registers (32b, indirect access via NET2282_IDXADDR and NET2282_IDXDATA).
	int NET2282_NET2282_ireg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr);
	int NET2282_NET2282_ireg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr);

	/// Slave section. Resources accessed via NET2282
	/// @note Access to port/memomry mapped resorces implemented in mehardware_access.c. Here only access to configuration space.
	int NET2282_PLX_cfg_write(struct NET2282_usb_device* dev, uint32_t val, uint32_t addr);
	int NET2282_PLX_cfg_read(struct NET2282_usb_device* dev, uint32_t* val, uint32_t addr);

	/// Common section. Access to NET2282 via USB
	int NET2282_write_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t val, uint32_t addr);
	int NET2282_read_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t* val, uint32_t addr);
	/// @note Maximum count is 512 bytes (512 * uint8_t == 128 * uint32_t). USB specification.
	int NET2282_write_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count);
	int NET2282_read_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count);
	int NET2282_hardware_init(struct NET2282_usb_device* dev);
	int NET2282_DMA_init(struct NET2282_usb_device* dev);

	void NET2282_ENDPOINTS_reset(struct NET2282_usb_device* dev);

# endif	//_NET2282_ACCESS_H_
#endif	//__KERNEL__
