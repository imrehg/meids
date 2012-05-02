/**
 * @file NET2282_access.c
 *
 * @brief Meilhaus access functions for NET2282 USB-PCI controler.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @note Enable access to PCI board via NET2282 bridge.
 * @note Support only 32bit PCI.
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef ME_USB
# error USB only!
# endif

# ifndef MODULE
#  define MODULE
# endif

# ifndef KBUILD_MODNAME
#  define KBUILD_MODNAME KBUILD_STR(NET2282_access)
# endif

# include <linux/module.h>
# include <linux/usb.h>
# include <linux/errno.h>
# include <linux/pci_regs.h>


# include "me_spin_lock.h"
# include "me_debug.h"

# include "NET2282_access.h"

# define USB_MAX_REPEAT 4

static int NET2282_PLX_set_PCI_BASEs(struct NET2282_usb_device* dev, uint32_t io_base, uint32_t mem_base);
static void usb_complete(struct urb* urb, struct pt_regs* regs);
static uint32_t region_size(uint32_t base);

static int _NET2282_write_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t val, uint32_t addr);
static int _NET2282_read_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t* val, uint32_t addr);
static int _NET2282_write_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count);
static int _NET2282_read_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count);

/// Master section. NET2282 resources.
// Access to NET2282 PCI configuration registers.
int NET2282_NET2282_cfg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr)
{
	int err;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	if (addr & 0x03)
		return -EFAULT;

		err = NET2282_write_reg(dev,
								NET2282_EP_CFG,
								NET2282_CBE_03 | NET2282_PCI_CFG_REG,
								val,
								addr);
	return err;
}
int NET2282_NET2282_cfg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr)
{
	int err;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	if (!val)
		return -EFAULT;

	if (addr & 0x03)
		return -EFAULT;

		err = NET2282_read_reg(dev,
								NET2282_EP_CFG,
								NET2282_CBE_03 | NET2282_PCI_CFG_REG,
								val,
								addr);
	return err;
}

// Access to NET2282 control configuration registers (32b, memory mapped [0x000 - 0x3FF]).
int NET2282_NET2282_reg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr)
{
	int err;

	if (addr & 0x03)
		return -EFAULT;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

		err = NET2282_write_reg(dev,
								NET2282_EP_CFG,
								NET2282_CBE_03 | NET2282_PCI_MEM_REG,
								val,
								addr);
	return err;
}
int NET2282_NET2282_reg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr)
{
	int err;

	if (addr & 0x03)
		return -EFAULT;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	if (!val)
		return -EFAULT;

		err = NET2282_read_reg(dev,
								NET2282_EP_CFG,
								NET2282_CBE_03 | NET2282_PCI_MEM_REG,
								val,
								addr);
	return err;
}

// Access to NET2282 indexed control configuration registers (32b, indirect access via NET2282_IDXADDR and NET2282_IDXDATA).
int NET2282_NET2282_ireg_write(struct NET2282_usb_device* dev, uint32_t val, uint16_t addr)
{
	int err;

	err = NET2282_NET2282_reg_write(dev, addr, NET2282_IDXADDR);
	if (!err)
	{
		err = NET2282_NET2282_reg_write(dev, val, NET2282_IDXDATA);
	}

	return err;
}
int NET2282_NET2282_ireg_read(struct NET2282_usb_device* dev, uint32_t* val, uint16_t addr)
{
	int err;

	err = NET2282_NET2282_reg_write(dev, addr, NET2282_IDXADDR);
	if (!err)
	{
		err = NET2282_NET2282_reg_read(dev, val, NET2282_IDXDATA);
	}

	return err;
}

/// Slave section. Resources accessed via NET2282
/// @note Access to port/memomry mapped resorces implemented in mehardware_access.c. Here only access to configuration space.
int NET2282_PLX_cfg_write(struct NET2282_usb_device* dev, uint32_t val, uint32_t addr)
{
	int err;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

		err = NET2282_write_reg(dev,
								NET2282_EP_PCI,
								(0x0
								| NET2282_CFG
								| NET2282_CBE_03
								| NET2282_PARK_SEL_USB
								| NET2282_MASTER_START
								| NET2282_RETRY_ABORT_ENB
								),
								val,
								addr);
	return err;
}

int NET2282_PLX_cfg_read(struct NET2282_usb_device* dev, uint32_t* val, uint32_t addr)
{
	int err;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	if (!val)
		return -EFAULT;

		err = NET2282_read_reg(dev,
								NET2282_EP_PCI,
								(0x0
								| NET2282_CFG
								| NET2282_CBE_03
								| NET2282_PARK_SEL_USB
								| NET2282_MASTER_START
								| NET2282_MASTER_RW
								| NET2282_RETRY_ABORT_ENB
								),
								val,
								addr);
	return err;
}


/// Common section. Access to NET2282 via USB
int NET2282_write_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t val, uint32_t addr)
{
	int err;
	int fails = 0;
	do
	{
		err = _NET2282_write_reg(dev, endpoint, ctrl, val, addr);
	}
	while ((err == -EPIPE) && (++fails < USB_MAX_REPEAT));

	return err;
}
static int _NET2282_write_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t val, uint32_t addr)
{
	int err = 0;
 	out_usb_struct_t* out_usb;
 	usb_context_struct_t* context;

    struct urb *urb;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	out_usb = kmalloc(sizeof(out_usb_struct_t), GFP_KERNEL);
	if (!out_usb)
	{
		PERROR("Cann't allocate out_usb structure.\n");
		return -ENOMEM;
	}

	context = kmalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_write_context structure.\n");
		return -ENOMEM;
	}

	out_usb->PCIMSTCTL = cpu_to_le16(ctrl);
	out_usb->PCIMSTADDR = cpu_to_le32(addr);
	out_usb->PCIMSTDATA  = cpu_to_le32(val);

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
		usb_fill_bulk_urb(	urb,
							dev->dev,
							usb_sndbulkpipe(dev->dev, endpoint),
							out_usb,
							sizeof(out_usb_struct_t),
							(usb_complete_t)usb_complete,
							(void *)context);

		context->status = USB_CONTEXT_WRITE;
		err = usb_submit_urb(urb, GFP_KERNEL);
		if(err)
		{
			PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
		}

		if (wait_event_interruptible_timeout(context->usb_queue, context->status != USB_CONTEXT_WRITE, USB_TRANSFER_TIMEOUT) <= 0)
		{
			err = -ETIMEDOUT;
			PERROR("Wait for ACK timed out.\n");
			PERROR("ctrl=0x%08x addr=0x%08x val=0x%08x.\n", ctrl, addr, val);
			usb_unlink_urb(urb);
		}
		else
		{
			err = context->status;
		}

		if (context->status == -EPIPE)
		{
			usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, endpoint));
		}

		if (err == USB_CONTEXT_WRITE)
		{
			PERROR("Write data timed out?\n");
			err = -ETIMEDOUT;
		}

		if (signal_pending(current))
		{
			err = -ECANCELED;
			PDEBUG("Aborted by signal.\n");
			usb_kill_urb(urb);
		}

		usb_free_urb(urb);
	}

	if (context)
	{
		kfree(context);
		context = NULL;
	}

	if (!err)
		PDEBUG_TRANS("WRITE: ENDPOINT:0x%02x PCIMSTCTL:0x%04x PCIMSTADDR:0x%04x PCIMSTDATA:0x%04x\n",
					endpoint, out_usb->PCIMSTCTL, out_usb->PCIMSTADDR, out_usb->PCIMSTDATA);

	kfree(out_usb);

	return err;
}

int NET2282_read_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t* val, uint32_t addr)
{
	int err;
	int fails = 0;
	do
	{
		err = _NET2282_read_reg(dev, endpoint, ctrl, val, addr);
	}
	while ((err == -EPIPE) && (++fails < USB_MAX_REPEAT));

	return err;
}
static int _NET2282_read_reg(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t ctrl, uint32_t* val, uint32_t addr)
{
	int err = 0;
 	in_addr_usb_struct_t* in_addr_usb = NULL;
 	in_data_usb_struct_t* in_data_usb = NULL;
 	usb_context_struct_t* context = NULL;

	struct urb *urb = NULL;

	in_addr_usb = kmalloc(sizeof(in_addr_usb_struct_t), GFP_KERNEL);
	if (!in_addr_usb)
	{
		PERROR("Cann't allocate in_addr_usb structure.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	in_data_usb = kmalloc(sizeof(in_data_usb_struct_t), GFP_KERNEL);
	if (!in_data_usb)
	{
		PERROR("Cann't allocate in_data_usb structure.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	context = kmalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_context structure.\n");
		err = -ENOMEM;
		goto ERROR;
	}

	in_addr_usb->PCIMSTCTL = cpu_to_le16(ctrl);
	in_addr_usb->PCIMSTADDR = cpu_to_le32(addr);

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
		usb_fill_bulk_urb(	urb,
							dev->dev,
							usb_sndbulkpipe(dev->dev, endpoint),
							in_addr_usb,
							sizeof(in_addr_usb_struct_t),
							(usb_complete_t)usb_complete,
							(void *)context);

		context->status = USB_CONTEXT_READ_ADDR;
		err = usb_submit_urb(urb, GFP_KERNEL);
		if(err)
		{
			PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
		}

		if (wait_event_interruptible_timeout(context->usb_queue, context->status != USB_CONTEXT_READ_ADDR, USB_TRANSFER_TIMEOUT) <= 0)
		{
			err = -ETIMEDOUT;
			PERROR("Wait for ACK timed out.\n");
			PERROR("ctrl=0x%08x addr=0x%08x.\n", ctrl, addr);
			usb_unlink_urb(urb);
		}
		else
		{
			err = context->status;
		}

		if (context->status == -EPIPE)
		{
			usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, endpoint));
		}

		if (err == USB_CONTEXT_READ_ADDR)
		{
			PERROR("Read data timed out?\n");
			err = -ETIMEDOUT;
		}

		if (signal_pending(current))
		{
			err = -ECANCELED;
			PDEBUG("Aborted by signal.\n");
			usb_kill_urb(urb);
		}

		if (!err)
		{
			usb_fill_bulk_urb(	urb,
								dev->dev,
								usb_rcvbulkpipe(dev->dev, endpoint),
								in_data_usb,
								sizeof(in_data_usb_struct_t),
								(usb_complete_t)usb_complete,
								(void *)context);

			context->status = USB_CONTEXT_READ_DATA;
			err = usb_submit_urb(urb, GFP_KERNEL);
			if(err)
			{
				PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
			}

			if (wait_event_interruptible_timeout(context->usb_queue, context->status != USB_CONTEXT_READ_DATA, USB_TRANSFER_TIMEOUT) <= 0)
			{
				err = -ETIMEDOUT;
				PERROR("Wait timed out.\n");
				usb_unlink_urb(urb);
			}
			else
			{
				err = context->status;
			}

			if (context->status == -EPIPE)
			{
				usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, endpoint));
			}

			if (err == USB_CONTEXT_READ_DATA)
			{
				PERROR("Read data timed out?\n");
				err = -ETIMEDOUT;
			}

			if (signal_pending(current))
			{
				err = -ECANCELED;
				PDEBUG("Aborted by signal.\n");
				usb_kill_urb(urb);
			}

			if (!err)
			{
				*val = le32_to_cpu(in_data_usb->PCIMSTDATA);
			}
		}

		usb_free_urb(urb);
	}

ERROR:
	if (context)
	{
		kfree(context);
		context = NULL;
	}

	if (!err)
		PDEBUG_TRANS("READ: ENDPOINT:0x%02x PCIMSTCTL:0x%04x PCIMSTADDR:0x%04x PCIMSTDATA:0x%04x\n",
					endpoint, in_addr_usb->PCIMSTCTL, in_addr_usb->PCIMSTADDR, in_data_usb->PCIMSTDATA);

	kfree(in_addr_usb);
	kfree(in_data_usb);

	return err;
}

static void usb_complete(struct urb* urb, struct pt_regs* regs)
{
	usb_context_struct_t* context = (usb_context_struct_t *)urb->context;

	switch (urb->status)
	{
		case -ESHUTDOWN:
		case -ENOENT:
		case -ECONNRESET:
			context->status = urb->status;
			PDEBUG("USB call canceled. Status=%d\n", -urb->status);
			break;

		case -EINPROGRESS:
			PDEBUG("ERROR! STILL IN PROGRESS! %d\n", -urb->status);
			break;

		case 0:
			context->status = 0;
			wake_up_interruptible_all(&context->usb_queue);
			break;

		default:
			PERROR("ERROR IN TRANSMISION! %d\n", -urb->status);
			context->status = urb->status;
			wake_up_interruptible(&context->usb_queue);
	}

	if (urb->status)
	{
		PDEVELOP("urb->status = %d\n", -urb->status);
	}

}

int NET2282_write_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count)
{
	int err;
	int fails = 0;
	do
	{
		err = _NET2282_write_FIFO(dev, endpoint, val, count);
	}
	while ((err == -EPIPE) && (++fails < USB_MAX_REPEAT));

	return err;
}
static int _NET2282_write_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count)
{
	int err = 0;
	usb_context_struct_t* context;

    struct urb *urb;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	context = kzalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_write_context structure.\n");
		return -ENOMEM;
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
		usb_fill_bulk_urb(	urb,
							dev->dev,
							usb_sndbulkpipe(dev->dev, endpoint),
							val,
							count<<2,
							(usb_complete_t)usb_complete,
							(void *)context);

		context->status = USB_CONTEXT_WRITE;
		err = usb_submit_urb(urb, GFP_KERNEL);
		if(err)
		{
			PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
		}

		if (wait_event_interruptible_timeout(context->usb_queue, context->status != USB_CONTEXT_WRITE, USB_DMA_TRANSFER_TIMEOUT) <= 0)
		{
			err = -ETIMEDOUT;
			PERROR("Wait for ACK timed out (Write FIFO).\n");
			usb_unlink_urb(urb);
			//usb_kill_urb(urb);
		}
		else
		{
			err = context->status;
		}

		if (context->status == -EPIPE)
		{
				usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, endpoint));
		}

		if (err == USB_CONTEXT_WRITE)
		{
			PERROR("Write data timed out?\n");
			err = -ETIMEDOUT;
		}

		if (signal_pending(current))
		{
			err = -ECANCELED;
			PDEBUG("Aborted by signal.\n");
			usb_kill_urb(urb);
		}

		usb_free_urb(urb);
	}

	kfree(context);

	if (!err)
	{
		PDEBUG_TRANS("WRITE FIFO: ENDPOINT:0x%02x\n", endpoint);
	}
	else
	{
		PERROR("WRITE FIFO: ENDPOINT:0x%02x error=%d\n", endpoint, err);
	}

	return err;
}

int NET2282_read_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count)
{
	int err;
	int fails = 0;
	do
	{
		err = _NET2282_read_FIFO(dev, endpoint, val, count);
	}
	while ((err == -EPIPE) && (++fails < USB_MAX_REPEAT));

	return err;
}
static int _NET2282_read_FIFO(struct NET2282_usb_device* dev, uint8_t endpoint, uint32_t* val, int count)
{
	int err = 0;
	usb_context_struct_t* context;

    struct urb *urb;

	if (!dev)
		return -EFAULT;

	if (!dev->dev)
		return -EFAULT;

	context = kmalloc(sizeof(usb_context_struct_t), GFP_KERNEL);
	if (!context)
	{
		PERROR("Cann't allocate usb_write_context structure.\n");
		return -ENOMEM;
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
		usb_fill_bulk_urb(	urb,
							dev->dev,
							usb_rcvbulkpipe(dev->dev, endpoint),
							val,
							count,
							(usb_complete_t)usb_complete,
							(void *)context);

		context->status = USB_CONTEXT_WRITE;
		err = usb_submit_urb(urb, GFP_KERNEL);
		if(err)
		{
			PERROR_CRITICAL("Couldn't submit URB: %d\n", err);
		}

		if (wait_event_interruptible_timeout(context->usb_queue, context->status != USB_CONTEXT_WRITE, USB_DMA_TRANSFER_TIMEOUT)<= 0)
		{
			err = -ETIMEDOUT;
			PERROR("Wait for ACK timed out. (Read FIFO)\n");
			usb_unlink_urb(urb);
			//usb_kill_urb(urb);
		}
		else
		{
			err = context->status;
		}

		if (context->status == -EPIPE)
		{
			usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, endpoint));
		}

		if (err == USB_CONTEXT_WRITE)
		{
			PERROR("Read data timed out?\n");
			err = -ETIMEDOUT;
		}

		if (signal_pending(current))
		{
			err = -ECANCELED;
			PDEBUG("Aborted by signal.\n");
			usb_kill_urb(urb);
		}

		usb_free_urb(urb);
	}

	kfree(context);

	if (!err)
	{
		PDEBUG_TRANS("READ FIFO: ENDPOINT:0x%02x\n", endpoint);
	}
	else
	{
		PDEBUG_TRANS("READ FIFO: ENDPOINT:0x%02x error=%d\n", endpoint, err);
	}

	return err;
}


///Setup section
// Init NET2282
static int NET2282_PLX_set_PCI_BASEs(struct NET2282_usb_device* dev, uint32_t io_base, uint32_t mem_base)
{
	int err;
	unsigned int pos;
	uint32_t region;
	uint32_t base_mask;
	uint32_t base_size;
	uint32_t base_gap;
	uint32_t tmp;

	for(pos=0; pos<6; pos++)
	{
		region = NET2282_PCIBASE0 + (pos << 2);

		err = NET2282_PLX_cfg_write(dev, ~0, region);
		if (err)
			break;
		err = NET2282_PLX_cfg_read(dev, &base_mask, region);
		if (err)
			break;

        if (!base_mask || base_mask == 0xFFFFFFFF)
        {
        	/// @note Unused regions MUST be set to 0 otherwise some boards don't work!
			err = NET2282_PLX_cfg_write(dev, 0, region);
			if (err)
				break;
        	continue;
        }


		if (base_mask & PCI_BASE_ADDRESS_SPACE_IO)
		{ // IO
			base_size = region_size(base_mask);
			if (base_size)
			{
#ifdef ME_NET2282_EXTRA_SPACE
				base_gap = (base_size & ~0xFF) + 0x100;
#else
				base_gap = base_size;
#endif
				err = NET2282_PLX_cfg_write(dev, io_base, region);
				if (err)
					break;
				err = NET2282_PLX_cfg_read(dev, &tmp, region);
				if (err)
					break;
				tmp &= ~0x1;
				if (io_base != tmp)
				{
					PDEBUG("IO Base%d ADDRESS BUGFIX! (%x != %x)\n", pos, io_base, tmp);
					io_base = (io_base & ~0xFF) + 0x100;
				}
				err = NET2282_PLX_cfg_write(dev, io_base, region);

				PINFO("IO Base%d base=0x%08x size=%d reserved=%d err=%d\n", pos, io_base, base_size, base_gap, err);
				io_base += base_gap;
			}
		}
		else
		{ // MEM
			base_size = region_size(base_mask);
			if (base_size)
			{
#ifdef ME_NET2282_EXTRA_SPACE
				base_gap = (base_size & ~0xFFFF) + 0x10000;
#else
				base_gap = base_size;
#endif
				err = NET2282_PLX_cfg_write(dev, mem_base, region);
				if (err)
					break;
				err = NET2282_PLX_cfg_read(dev, &tmp, region);
				if (err)
					break;
				tmp &= ~0x1;
				if (mem_base != tmp)
				{
					PDEBUG("MEM Base%d ADDRESS BUGFIX!(%x != %x)\n", pos, mem_base, tmp);
					mem_base = (mem_base & ~0xFFFF) + 0x10000;
				}
				err = NET2282_PLX_cfg_write(dev, mem_base, region);

				PINFO("IO Base%d base=0x%08x size=%d reserved=%d err=%d\n", pos, mem_base, base_size, base_gap, err);
				mem_base += base_gap;
			}
		}
		if (err)
			break;
	}
	return err;
}

static uint32_t region_size(uint32_t base)
{
	uint32_t size;
	uint32_t mask = 0;

	if (base & PCI_BASE_ADDRESS_SPACE_IO)
	{ // IO
		mask = PCI_BASE_ADDRESS_IO_MASK & 0xffff;
	}
	else
	{ // MEM
		mask = (uint32_t)PCI_BASE_ADDRESS_MEM_MASK;
	}

	size = mask & base;
	if (!size)
		return 0;
	return (size & ~(size-1));
}

int NET2282_DMA_init(struct NET2282_usb_device* dev)
{
	uint32_t tmp;
	int err;

	PDEBUG("executed.\n");

	// Disable and configure A and B endpoints
	// OUTPUT QUEUE: PC ->> SYNAPSE
	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPA_CFG);
	// Disable
	tmp &= ~(0x1<<10);
	// Bulk endpoint
	tmp &= ~(0x3<<8);
	tmp |= 0x2<<8;
	// Direction: OUTPUT
	tmp &= ~(0x1<<7);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPA_CFG);

	// INPUT QUEUE: PC <<- SYNAPSE
	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPB_CFG);
	// Disable
	tmp &= ~(0x1<<10);
	// Bulk endpoint
	tmp &= ~(0x3<<8);
	tmp |= 0x2<<8;
	// Direction: INPUT
	tmp |= (0x1<<7);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPB_CFG);

	// Just disable all other endpoints
	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPC_CFG);
	tmp &= ~(0x1<<10);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPC_CFG);

	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPD_CFG);
	tmp &= ~(0x1<<10);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPD_CFG);

	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPE_CFG);
	tmp &= ~(0x1<<10);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPE_CFG);

	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPF_CFG);
	tmp &= ~(0x1<<10);
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPF_CFG);

	/// Configure FIFO - 2x2KB
	err = NET2282_NET2282_reg_write(dev,
										(0xFFFE0000
										| NET2282_FIFO_CONF_2x2KB
// 										| NET2282_FIFO_BASE2_BIDIRECTIONAL
// 										| NET2282_FIFO_NOT_VALIDATE
										),
										NET2282_FIFOCTL);

// 	err = NET2282_NET2282_ireg_write(dev, 0x7FD, NET2282_EPA_HS_MAXPTK);

	/// Enable endpoints A and B
	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPA_CFG);
	tmp |= 0x1<<10;
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPA_CFG);

	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_EPB_CFG);
	tmp |= 0x1<<10;
	err = NET2282_NET2282_reg_write(dev, tmp, NET2282_EPB_CFG);

	/// Flush FIFOs
	err = NET2282_NET2282_reg_write(dev, 0x003f1200, NET2282_EPA_STAT);
	err = NET2282_NET2282_reg_write(dev, 0x003f1200, NET2282_EPB_STAT);

	// Write NET2282 DMA CONTROL
	NET2282_NET2282_reg_write(dev,
								(	0x0
									| NET2282_DMA_CTL_ADDR_HOLD
									| NET2282_DMA_CTL_DMA_ENABLE
// 									| NET2282_DMA_CTL_FIFO_VALIDATE
// 									| NET2282_DMA_CTL_PREEMPT_ENABLE
// 									| NET2282_DMA_CTL_AUTO_MODE
// 									| NET2282_DMA_CTL_SCATTER_GATHER_MODE
// 									| NET2282_DMA_CTL_DMA_VALIDATE
// 									| NET2282_DMA_CTL_POLLING_MODE
// 									| NET2282_DMA_CTL_POLLING_1us
// 									| NET2282_DMA_CTL_COUNT_CLEAR
// 									| NET2282_DMA_CTL_SCATTER_GATHER_IRQ_ENABLE
// 									| NET2282_DMA_CTL_PAUSE_IRQ_ENABLE
// 									| NET2282_DMA_CTL_ABORT_IRQ_ENABLE
								),
								NET2282_CHA_DMACTL);

	NET2282_NET2282_reg_write(dev,
								(	0x0
									| NET2282_DMA_CTL_ADDR_HOLD
									| NET2282_DMA_CTL_DMA_ENABLE
// 									| NET2282_DMA_CTL_FIFO_VALIDATE
// 									| NET2282_DMA_CTL_PREEMPT_ENABLE
// 									| NET2282_DMA_CTL_AUTO_MODE
// 									| NET2282_DMA_CTL_SCATTER_GATHER_MODE
// 									| NET2282_DMA_CTL_DMA_VALIDATE
// 									| NET2282_DMA_CTL_POLLING_MODE
// 									| NET2282_DMA_CTL_POLLING_1us
// 									| NET2282_DMA_CTL_COUNT_CLEAR
// 									| NET2282_DMA_CTL_SCATTER_GATHER_IRQ_ENABLE
// 									| NET2282_DMA_CTL_PAUSE_IRQ_ENABLE
// 									| NET2282_DMA_CTL_ABORT_IRQ_ENABLE
								),
								NET2282_CHB_DMACTL);
	return err;

}

int NET2282_hardware_init(struct NET2282_usb_device* dev)
{// This function only does the initialization needed to communicate with the PCI device behind the bridge
	int err;

	PDEBUG("executed.\n");
	//Enable PCI
		err = NET2282_write_reg(dev,
								NET2282_EP_PCI,
								NET2282_CFG | NET2282_CBE_03 | NET2282_PARK_SEL_USB,
								NET2282_PCICMD_MASTER,
								NET2282_PCICMD);

	if (!err)
		err = NET2282_PLX_cfg_write(dev, NET2282_PCICMD_SLAVE, NET2282_PCICMD);

	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_8051_RESET | NET2282_CLK_30Mhz | NET2282_PCI_ENABLE, NET2282_DEVINIT);

	if (!err)
		err = NET2282_NET2282_reg_write(dev, 0, NET2282_USBIRQENB1);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, 0, NET2282_PCIIRQENB0);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, 0, NET2282_PCIIRQENB1);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, ~0, NET2282_IRQSTAT0);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, ~NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE, NET2282_IRQSTAT1);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_USB_INTERRUPT_ENABLE | NET2282_PCI_INTA_INTERRUPT_ENABLE, NET2282_USBIRQENB1);

		PINFO("#INTA line %sabled\n", (!err)?"en":"dis");

	// Set Base2 (MUST BE BEFORE DMA config!)
	if (!err)
		err = NET2282_NET2282_cfg_write(dev, 0x80000000, PCI_BASE_ADDRESS_2);
	if (!err)
		err = NET2282_NET2282_cfg_write(dev, 0xC0000000, PCI_BASE_ADDRESS_1);

	if (!err)
		err = NET2282_DMA_init(dev);

	// From Windows code. I've got no idea why I should do it.
	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS_MODE, NET2282_EPA_RSP);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS_MODE, NET2282_EPB_RSP);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS_MODE, NET2282_EPC_RSP);
	if (!err)
		err = NET2282_NET2282_reg_write(dev, NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS_MODE, NET2282_EPD_RSP);

	if (!err)
		err = NET2282_PLX_set_PCI_BASEs(dev, NET2282_IO_BASE, NET2282_MEM_BASE);

	if (!err)
		PDEBUG("Enabling bridge: DONE\n");
	else
		PERROR("Enabling bridge: FAILED\n");

	return err;
}

void NET2282_ENDPOINTS_reset(struct NET2282_usb_device* dev)
{
	PDEBUG("executed.\n");

		usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, NET2282_EP_CFG));
		usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, NET2282_EP_CFG));
		usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, NET2282_EP_PCI));
		usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, NET2282_EP_PCI));
		usb_clear_halt(dev->dev, usb_rcvintpipe(dev->dev, NET2282_EP_IRQ));
		usb_clear_halt(dev->dev, usb_sndintpipe(dev->dev, NET2282_EP_IRQ));

		usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, NET2282_EP_A));
		usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, NET2282_EP_A));
		usb_clear_halt(dev->dev, usb_rcvbulkpipe(dev->dev, NET2282_EP_B));
		usb_clear_halt(dev->dev, usb_sndbulkpipe(dev->dev, NET2282_EP_B));

}
