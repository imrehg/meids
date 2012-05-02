/**
 * @file mehardware_access.c
 *
 * @brief Meilhaus replacement for standard port/memory access functions.
 * @note Enable access to port and memory mapped sections on PCI boards. Connected via local or external bus (USB->NET2282->PCI).
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/module.h>
# include <linux/errno.h>

# include "me_spin_lock.h"
# include "me_debug.h"
# include "me_error.h"

# if defined(ME_USB)
#  include <linux/sched.h>
#  include "NET2282_access.h"
#  include "mehardware_access.h"

#  define ME_CTRL_BIT_DMA					0x40000000
#  define DMA_BLOCK_MAX_SIZE				0x00800000
#  define DMA_BLOCK_SIZE_MASK				0x00FFFFFF
#  define DMA_REGION_SIZE					0x2000

// #  define ME_DMA_WRITE_BLOCK_SIZE			0x1000
#  define ME_DMA_WRITE_BLOCK_SIZE			0x200
// #  define ME_DMA_WRITE_BLOCK_SIZE			0x80

// #  define ME_DMA_READ_BLOCK_SIZE			0x200
// #  define ME_DMA_READ_BLOCK_SIZE			0x100
#  define ME_DMA_READ_BLOCK_SIZE			0x80

#  define ME_DMA_READ_FIFO_SIZE				0x100

// PORTS: Meilhaus versions of 'in' and 'out' commands.
static int me_outb(void* dev, uint8_t  val, volatile uint16_t addr);
static int me_outw(void* dev, uint16_t val, volatile uint16_t addr);
static int me_outl(void* dev, uint32_t val, volatile uint16_t addr);

static int me_inb(void* dev, uint8_t*  val, volatile uint16_t addr);
static int me_inw(void* dev, uint16_t* val, volatile uint16_t addr);
static int me_inl(void* dev, uint32_t* val, volatile uint16_t addr);

// MEMORY: Meilhaus versions of 'read' and 'write' commands.
static int me_stb(void* dev, uint8_t  val, volatile uint32_t addr);
static int me_stw(void* dev, uint16_t val, volatile uint32_t addr);
static int me_stl(void* dev, uint32_t val, volatile uint32_t addr);

static int me_ldb(void* dev, uint8_t*  val, volatile uint32_t addr);
static int me_ldw(void* dev, uint16_t* val, volatile uint32_t addr);
static int me_ldl(void* dev, uint32_t* val, volatile uint32_t addr);

// External PCI access functions (via USB)
void me_writeb(void* dev, uint8_t  val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		PERROR("me_writeb(0x%p : 0x%02X)=%d ACCESS BLOCKED!\n", addr, val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_outb(dev, val, (volatile uint16_t)l_addr) : me_stb(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_writeb(0x%p : 0x%02X)=%d\n", addr, val, err);
		}
		else
		{
			PDEBUG_REG("me_writeb(0x%p : 0x%02X)=%d\n", addr, val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}

void me_writew(void* dev, uint16_t val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		PERROR("me_writew(0x%p : 0x%04X)=%d ACCESS BLOCKED!\n", addr, val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_outw(dev, val, (volatile uint16_t)l_addr) : me_stw(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_writew(0x%p : 0x%04X)=%d\n", addr, val, err);
		}
		else
		{
			PDEBUG_REG("me_writew(0x%p : 0x%04X)=%d\n", addr, val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}

void me_writel(void* dev, uint32_t val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		PERROR("me_writel(0x%p : 0x%08X)=%d ACCESS BLOCKED!\n", addr, val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_outl(dev, val, (volatile uint16_t)l_addr) : me_stl(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_writel(0x%p : 0x%08X)=%d\n", addr, val, err);
		}
		else
		{
			PDEBUG_REG("me_writel(0x%p : 0x%08X)=%d\n", addr, val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}


void me_readb(void* dev, uint8_t*  val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		*val = 0;
		PERROR("me_readb(0x%p : 0x%02X)=%d ACCESS BLOCKED!\n", addr, *val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_inb(dev, val, (volatile uint16_t)l_addr) : me_ldb(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_readb(0x%p : 0x%02X)=%d\n", addr, *val, err);
		}
		else
		{
			PDEBUG_REG("me_readb(0x%p : 0x%02X)=%d\n", addr, *val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}

void me_readw(void* dev, uint16_t* val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		*val = 0;
		PERROR("me_readw(0x%p : 0x%04X)=%d ACCESS BLOCKED!\n", addr, *val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_inw(dev, val, (volatile uint16_t)l_addr) : me_ldw(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_readw(0x%p : 0x%04X)=%d\n", addr, *val, err);
		}
		else
		{
			PDEBUG_REG("me_readw(0x%p : 0x%04X)=%d\n", addr, *val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}

void me_readl(void* dev, uint32_t* val, volatile void* addr)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;

	if (atomic_read(&usb_dev->usb_transfer_status))
	{
		*val = 0;
		PERROR("me_readl(0x%p : 0x%08X)=%d ACCESS BLOCKED!\n", addr, *val, atomic_read(&usb_dev->usb_transfer_status));
		return;
	}

	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
		err = (l_addr < NET2282_MEM_BASE) ? me_inl(dev, val, (volatile uint16_t)l_addr) : me_ldl(dev, val, (volatile uint32_t)l_addr);
		if (err)
		{
			atomic_set(&usb_dev->usb_transfer_status, err);
			PERROR("me_readl(0x%p : 0x%08X)=%d\n", addr, *val, err);
		}
		else
		{
			PDEBUG_REG("me_readl(0x%p : 0x%08X)=%d\n", addr, *val, err);
		}
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);
}

// Port write
static int me_outb(void* dev, uint8_t val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_RETRY_ABORT_ENB;
	uint32_t reg = 0;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_0;
			reg = val;
			break;

		case 1:
			ctrl |= NET2282_CBE_1;
			reg = val<<8;
			break;

		case 2:
			ctrl |= NET2282_CBE_2;
			reg = val<<16;
			break;

		case 3:
			ctrl |= NET2282_CBE_3;
			reg = val<<24;
			break;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, reg, addr);

	return err;
}

static int me_outw(void* dev, uint16_t val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;


	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_1 | NET2282_CBE_0;
			reg = val;
			break;

		case 2:
			ctrl |= NET2282_CBE_3 | NET2282_CBE_2;
			reg = val<<16;
			break;

		default:
			PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
			return -EFAULT;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, reg, addr);

	return err;
}

static int me_outl(void* dev, uint32_t val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_CBE_03 | NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_RETRY_ABORT_ENB;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (addr & 0x03)
	{
		PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
		return -EFAULT;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, val, addr);

	return err;
}

// Port read
static int me_inb(void* dev, uint8_t* val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;


	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_0;
			break;

		case 1:
			ctrl |= NET2282_CBE_1;
			break;

		case 2:
			ctrl |= NET2282_CBE_2;
			break;

		case 3:
			ctrl |= NET2282_CBE_3;
			break;
	}
	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, &reg, addr);

	if (!err)
	{
		switch (addr & 0x03)
		{
			case 0:
				*val = (uint8_t)reg;
				break;

			case 1:
				*val = (uint8_t)(reg>>8);
				break;

			case 2:
				*val = (uint8_t)(reg>>16);
				break;

			case 3:
				*val = (uint8_t)(reg>>24);
				break;
		}
	}
	return err;
}

static int me_inw(void* dev, uint16_t* val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_1 | NET2282_CBE_0;
			break;

		case 2:
			ctrl |= NET2282_CBE_3 | NET2282_CBE_2;
			break;

		default:
			PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
			return -EFAULT;
	}
	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, &reg, addr);

	if (!err)
	{
		switch (addr & 0x03)
		{
			case 0:
				*val = (uint16_t)reg;
				break;

			case 2:
				*val = (uint16_t)(reg>>16);
				break;

			default:
				PERROR_CRITICAL("(addr & 0x03)=%x\n", (addr & 0x03));
				return -EFAULT;
		}
	}
	return err;
}

static int me_inl(void* dev, uint32_t* val, volatile uint16_t addr)
{
	uint32_t ctrl = NET2282_CBE_03 | NET2282_IO | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	if (addr & 0x03)
	{
		PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
		return -EFAULT;
	}

	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, val, addr);

	return err;
}


// Memmory write
static int me_stb(void* dev, uint8_t val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START;
	uint32_t reg = 0;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_0;
			reg = val;
			break;

		case 1:
			ctrl |= NET2282_CBE_1;
			reg = val<<8;
			break;

		case 2:
			ctrl |= NET2282_CBE_2;
			reg = val<<16;
			break;

		case 3:
			ctrl |= NET2282_CBE_3;
			reg = val<<24;
			break;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, reg, addr);

	return err;
}

static int me_stw(void* dev, uint16_t val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_1 | NET2282_CBE_0;
			reg = val;
			break;

		case 2:
			ctrl |= NET2282_CBE_3 | NET2282_CBE_2;
			reg = val<<16;
			break;

		default:
			PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
			return -EFAULT;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, reg, addr);

	return err;
}

static int me_stl(void* dev, uint32_t val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_CBE_03 | NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_RETRY_ABORT_ENB;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (addr & 0x03)
	{
		PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
		return -EFAULT;
	}
	err = NET2282_write_reg(usb_dev, NET2282_EP_PCI, ctrl, val, addr);

	return err;
}

// Memmory read
static int me_ldb(void* dev, uint8_t* val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_0;
			break;

		case 1:
			ctrl |= NET2282_CBE_1;
			break;

		case 2:
			ctrl |= NET2282_CBE_2;
			break;

		case 3:
			ctrl |= NET2282_CBE_3;
			break;
	}
	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, &reg, addr);

	if (!err)
	{
		switch (addr & 0x03)
		{
			case 0:
				*val = (uint16_t)reg;
				break;

			case 1:
				*val = (uint16_t)(reg>>8);
				break;

			case 2:
				*val = (uint16_t)(reg>>16);
				break;

			case 3:
				*val = (uint16_t)(reg>>24);
				break;
		}
	}
	return err;
}

static int me_ldw(void* dev, uint16_t* val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	uint32_t reg;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	switch (addr & 0x03)
	{
		case 0:
			ctrl |= NET2282_CBE_1 | NET2282_CBE_0;
			break;

		case 2:
			ctrl |= NET2282_CBE_3 | NET2282_CBE_2;
			break;

		default:
			PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
			return -EFAULT;
	}
	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, &reg, addr);

	if (!err)
	{
		switch (addr & 0x03)
		{
			case 0:
				*val = (uint16_t)reg;
				break;

			case 2:
				*val = (uint16_t)(reg>>16);
				break;

			default:
				PERROR_CRITICAL("(addr & 0x03)=%x\n", (addr & 0x03));
				return -EFAULT;
		}
	}
	return err;
}

static int me_ldl(void* dev, uint32_t* val, volatile uint32_t addr)
{
	uint32_t ctrl = NET2282_CBE_03 | NET2282_MEM | NET2282_PARK_SEL_USB | NET2282_FLOAT_ENB | NET2282_MASTER_START | NET2282_MASTER_RW | NET2282_RETRY_ABORT_ENB;
	int err;
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;

	if (!dev)
	{
		PERROR("dev=%p\n", dev);
		return -EFAULT;
	}

	if (!usb_dev->dev)
	{
		PERROR("usb_dev->dev=%p\n", usb_dev->dev);
		return -EFAULT;
	}

	if (!val)
	{
		PERROR("val=%p\n", val);
		return -EFAULT;
	}

	if (addr & 0x03)
	{
		PERROR("(addr & 0x03)=%x\n", (addr & 0x03));
		return -EFAULT;
	}
	err = NET2282_read_reg(usb_dev, NET2282_EP_PCI, ctrl, val, addr);

	return err;
}

// DMA
int access_test(void* dev)
{
	uint32_t tmp;
	int pos;
	int err;
	int ret = ME_ERRNO_SUCCESS;

	// Access test
	PINFO("ACCESS TEST\n");
	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_CHA_DMACTL);
	if (!err)
	{
		PINFO("ACCESS TEST: Reading NET2282 configuration successed. CHA_DMACTL=0x%08x.\n", tmp);
	}
	else
	{
		PINFO("ACCESS TEST: Reading NET2282 configuration failed. CHA_DMACTL err=%d\n", err);
		ret = ME_ERRNO_COMMUNICATION;
	}

	err = NET2282_NET2282_ireg_write(dev, 0xDEEDCA11, NET2282_SCRATCH_PAD);
	if (!err)
	{
		PINFO("ACCESS TEST: Writing NET2282 configuration successed.\n");
	}
	else
	{
		PINFO("ACCESS TEST: Writing NET2282 configuration failed. err=%d\n", err);
		ret = ME_ERRNO_COMMUNICATION;
	}

	err = NET2282_NET2282_reg_read(dev, &tmp, NET2282_CHA_DMASTAT);
	if (!err)
	{
		PINFO("ACCESS TEST: Reading NET2282 configuration successed. CHA_DMASTAT=0x%08x.\n", tmp);
	}
	else
	{
		PINFO("ACCESS TEST: Reading NET2282 configuration failed. CHA_DMASTAT err=%d\n", err);
		ret = ME_ERRNO_COMMUNICATION;
	}

	for(pos=0; pos<6; pos++)
	{
		err = NET2282_PLX_cfg_read(dev, &tmp, NET2282_PCIBASE0 + (pos << 2));
		if (!err)
		{
			PINFO("ACCESS TEST: Reading 9052 configuration successed. PCIBASE%d=0x%08x\n", pos, tmp);
			err = ((tmp & ~0x03) < NET2282_MEM_BASE) ? me_inl(dev, &tmp, (volatile uint16_t)(tmp & ~0x03)) : me_ldl(dev, &tmp, (volatile uint32_t)(tmp & ~0x03));
			if (!err)
			{
				PINFO("ACCESS TEST: Reading 9052 PCIBASE%d successed val=0x%08x\n", pos, tmp);
			}
			else
			{
				PINFO("ACCESS TEST: Reading PCIBASE%d failed. err=%d\n", pos, err);
			ret = ME_ERRNO_COMMUNICATION;
			}
		}
		else
		{
			PINFO("ACCESS TEST: Reading 9052 configuration failed. PCIBASE%d err=%d\n", pos, err);
			ret = ME_ERRNO_COMMUNICATION;
		}
	}

	return ret;
}

/// @note For enabling DMA transfer we need to switch whole board to special mode.
int me_DMA_lock(void* dev, volatile void* addr, uint32_t* mirror)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	uint32_t ctrl = 0;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err = -ETIMEDOUT;


	ME_DMA_LOCK(usb_dev->usb_DMA_semaphore);
	PINFO("%ld: DMA LOCK\n", jiffies);

	err = (l_addr < NET2282_MEM_BASE) ? me_inl(dev, &ctrl, (volatile uint16_t)l_addr) : me_ldl(dev, &ctrl, (volatile uint32_t)l_addr);
	if (err)
	{
		PERROR("IO >>> DMA ctrl read failed err=%d\n", err);
		PERROR("me_readl(0x%p : 0x%08X)=%d\n", addr, ctrl, err);
		*mirror = 0;
		return err;
	}
	else
	{
		*mirror = ctrl  & ~ME_CTRL_BIT_DMA;
		PDEBUG_REG("me_readl(0x%p : 0x%08X)=%d\n", addr, ctrl, err);
	}

	err = (l_addr < NET2282_MEM_BASE) ? me_outl(dev, ctrl | ME_CTRL_BIT_DMA, (volatile uint16_t)l_addr) : me_stl(dev, ctrl | ME_CTRL_BIT_DMA, (volatile uint32_t)l_addr);
	if (err)
	{
		PERROR("IO >>> DMA locking failed err=%d\n", err);
		PERROR("me_writel(0x%p : 0x%08X)=%d\n", addr, ctrl | ME_CTRL_BIT_DMA, err);
	}
	else
	{
		PDEBUG_REG("me_writel(0x%p : 0x%08X)=%d\n", addr, ctrl | ME_CTRL_BIT_DMA, err);
	}

	return err;
}

int me_DMA_unlock(void* dev, volatile void* addr, uint32_t mirror)
{
	struct NET2282_usb_device* usb_dev = (struct NET2282_usb_device *)dev;
	volatile unsigned long l_addr = (volatile unsigned long)addr;
	int err;
	int counter = MAX_REPEATS;
	uint32_t ctrl;

	for(counter = 0; counter<MAX_REPEATS; counter++)
	{
		err = (l_addr < NET2282_MEM_BASE) ? me_outl(dev, mirror, (volatile uint16_t)l_addr) : me_stl(dev, mirror, (volatile uint32_t)l_addr);
		if (err)
		{
			PERROR("me_writel(0x%p : 0x%08X)=%d\n", addr, mirror, err);
		}
		else
		{
			PDEBUG_REG("me_writel(0x%p : 0x%08X)=%d\n", addr, mirror, err);
			err = (l_addr < NET2282_MEM_BASE) ? me_inl(dev, &ctrl, (volatile uint16_t)l_addr) : me_ldl(dev, &ctrl, (volatile uint32_t)l_addr);
			if (err)
			{
				PERROR("me_readl(0x%p : 0x%08X)=%d\n", addr, ctrl, err);
			}
			else
			{
				PDEBUG_REG("me_readl(0x%p : 0x%08X)=%d\n", addr, ctrl, err);
				if (ctrl & ME_CTRL_BIT_DMA)
				{
					PERROR_CRITICAL("Cannot release DMA lock!\n");
					err = ME_ERRNO_COMMUNICATION;
				}
				else
				{
					goto END;
				}
			}
		}
	}

	if (err)
	{
		PERROR("DMA >>> IO unlocking failed err=%d\n", err);
	}

END:
	ME_DMA_UNLOCK(usb_dev->usb_DMA_semaphore);

	return err;
}

int me_DMA_write(void* dev, uint32_t* buff, int size, void* dest)
{
	int i;
	int lenght;
	int err = 0;

	PINFO("%ld: DMA WRITE BEGIN ADDR=0x%p\n", jiffies, dest);

	err = NET2282_NET2282_reg_write(dev,
								(	0x0
									| NET2282_RETRY_ABORT_ENB
// 									| NET2282_MULTI_LEVEL_ARBITER
									| NET2282_PARK_SEL_DMA_A
								),
								NET2282_PCIMSTCTL);
	if (err)
	{
		PERROR("NET2282 DMA WRITE: PCICTL error=%d.\n", err);
		goto ERROR;
	}

	err = NET2282_NET2282_reg_write(dev,
								(	0x0
									| NET2282_DMA_CTL_ADDR_HOLD
// 									| NET2282_DMA_CTL_DMA_ENABLE
// 									| NET2282_DMA_CTL_FIFO_VALIDATE
// 									| NET2282_DMA_CTL_PREEMPT_ENABLE
									| NET2282_DMA_CTL_AUTO_MODE
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
	if (err)
	{
		PERROR("NET2282 DMA WRITE: DMACTL error=%d.\n", err);
		goto ERROR;
	}

	// Write NET2282 DMA ADDRESS
	err = NET2282_NET2282_reg_write(dev, (long int)dest, NET2282_CHA_DMAADDR);
	if (err)
	{
		PERROR("NET2282 DMA WRITE: ADDRESS error=%d.\n", err);
		goto ERROR;
	}
	// Write NET2282 DMA COUNT
	err = NET2282_NET2282_reg_write(dev,
								0x0
								| (size<<2)
// 								| DMA_BLOCK_MAX_SIZE
								| (	0x0
									| NET2282_DMA_COUNT_CONTINUE
// 									| NET2282_DMA_COUNT_SCATTER_GATHER_FIFO_VALIDATE
// 									| NET2282_DMA_COUNT_END_OF_CHAIN
// 									| NET2282_DMA_COUNT_DONE_IRQ_ENABLE
// 									| NET2282_DMA_COUNT_TRANSFER_DIRECTION	//0-OUT 1-IN
// 									| NET2282_DMA_COUNT_VALID_BIT
								),
								NET2282_CHA_DMACOUNT);
	if (err)
	{
		PERROR("NET2282 DMA WRITE: COUNT error=%d.\n", err);
		goto ERROR;
	}
	err = NET2282_NET2282_reg_write(dev,
								(	0x0
									| NET2282_DMA_CTL_ADDR_HOLD
									| NET2282_DMA_CTL_DMA_ENABLE
// 									| NET2282_DMA_CTL_FIFO_VALIDATE
// 									| NET2282_DMA_CTL_PREEMPT_ENABLE
									| NET2282_DMA_CTL_AUTO_MODE
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
	if (err)
	{
		PERROR("NET2282 DMA WRITE: CTL error=%d.\n", err);
		goto ERROR;
	}

	// Transfer datas
	for (i=0; i<size; i += ME_DMA_WRITE_BLOCK_SIZE)
	{

 		lenght = ((size - i) < ME_DMA_WRITE_BLOCK_SIZE) ? (size - i) : ME_DMA_WRITE_BLOCK_SIZE;

		// 	Write FIFO
		err = NET2282_write_FIFO(dev, NET2282_EP_A, buff + i, lenght);
		if (err)
		{
			PERROR("NET2282 DMA WRITE: DATA err=%d.\n", err);
			break;
		}
		else
		{
			PINFO("NET2282 DMA WRITE: %d\n", i);
		}
		//Start DMA
		err = NET2282_NET2282_reg_write(dev,
									(	0x0
										| NET2282_DMA_STAT_TRANSFER_START
// 										| NET2282_DMA_STAT_ABORT
										| NET2282_DMA_STAT_DONE_IRQ_CLS
										| NET2282_DMA_STAT_SCATTER_GATHER_IRQ_CLS
										| NET2282_DMA_STAT_PAUSE_IRQ_CLS
										| NET2282_DMA_STAT_ABORT_IRQ_CLS
									),
									NET2282_CHA_DMASTAT);
		if (err)
		{
			PERROR("NET2282 DMA WRITE: START err=%d.\n", err);
			break;
		}
	}

ERROR:
	if (err)
	{
		PERROR("me_DMA_write(0x%p) size:%d err=%d\n", dest, size, err);
		access_test(dev);
	}
	else
	{
		PDEBUG_REG("me_DMA_write(0x%p) size:%d err=%d\n", dest, size, err);
	}
	PINFO("%ld: DMA WRITE END err=%d\n", jiffies, err);
	return err;
}

int me_DMA_read(void* dev, uint32_t* buff, int size, void* source)
{
	int i;
	int j;
	int lenght;
	int err = 0;

	PINFO("%ld: DMA READ BEGIN\n", jiffies);

	// Write NET2282 DMA ADDRESS
	err = NET2282_NET2282_reg_write(dev, (long int)source, NET2282_CHB_DMAADDR);
	if (err)
	{
		PERROR("error err=%d.\n", err);
	}

	for (i = 0; i < size; i += ME_DMA_READ_FIFO_SIZE)
	{
 		lenght = ((size - i) < ME_DMA_READ_FIFO_SIZE) ? (size - i) : ME_DMA_READ_FIFO_SIZE;
		// Write NET2282 DMA COUNT
		err = NET2282_NET2282_reg_write(dev,
									( (lenght<<2) & DMA_BLOCK_SIZE_MASK) |
										(	0x0
// 											| NET2282_DMA_COUNT_CONTINUE
// 											| NET2282_DMA_COUNT_SCATTER_GATHER_FIFO_VALIDATE
// 											| NET2282_DMA_COUNT_END_OF_CHAIN
// 											| NET2282_DMA_COUNT_DONE_IRQ_ENABLE
											| NET2282_DMA_COUNT_TRANSFER_DIRECTION	//0-OUT 1-IN
											| NET2282_DMA_COUNT_VALID_BIT
										),
									NET2282_CHB_DMACOUNT);
		if (err)
		{
			PERROR("NET2282 DMA READ: COUNT error=%d\n", err);
			break;
		}

		// Start DMA
		err = NET2282_NET2282_reg_write(dev,
									(	0x0
										| NET2282_DMA_STAT_TRANSFER_START
// 										| NET2282_DMA_STAT_ABORT
										| NET2282_DMA_STAT_DONE_IRQ_CLS
										| NET2282_DMA_STAT_SCATTER_GATHER_IRQ_CLS
										| NET2282_DMA_STAT_PAUSE_IRQ_CLS
										| NET2282_DMA_STAT_ABORT_IRQ_CLS
									),
									NET2282_CHB_DMASTAT);
		if (err)
		{
			PERROR("NET2282 DMA READ: START error=%d\n", err);
			break;
		}

		// Read FIFO
		for (j = 0; j < lenght; j += ME_DMA_READ_BLOCK_SIZE)	//80 dwords == 512 bytes => Max USB transfer block
		{
			err = NET2282_read_FIFO(dev, NET2282_EP_B, buff + i + j, ((lenght-j) < ME_DMA_READ_BLOCK_SIZE) ? ((lenght-j)<<2) : (ME_DMA_READ_BLOCK_SIZE<<2));
			if (err)
			{
				PERROR("NET2282 DMA READ: READ error=%d\n", err);
				break;
			}
		}
 	}

	if (err)
	{
		PERROR("me_DMA_read(0x%p) size:%d err=%d\n", source, size, err);
		access_test(dev);
	}
	else
	{
		PDEBUG_REG("me_DMA_read(0x%p) size:%d err=%d\n", source, size, err);
	}
	PINFO("%ld: DMA READ END\n", jiffies);
	return err;
}


# else // ME_PCI or ME_COMEDI
#  include <asm/io.h>

// Local PCI access functions

void me_writeb(void* dev, uint8_t  val, volatile void* addr)
{
	iowrite8(val, (void *)addr);
	PDEBUG_REG("me_writeb(0x%p : 0x%02X)=%d\n", addr, val, 0);
}

void me_writew(void* dev, uint16_t val, volatile void* addr)
{
	iowrite16(val, (void *)addr);
	PDEBUG_REG("me_writew(0x%p : 0x%04X)=%d\n", addr, val, 0);
}

void me_writel(void* dev, uint32_t val, volatile void* addr)
{
	iowrite32(val, (void *)addr);
	PDEBUG_REG("me_writel(0x%p : 0x%08X)=%d\n", addr, val, 0);
}


void me_readb(void* dev, uint8_t* val, volatile void* addr)
{
	*val = (uint8_t)ioread8((void *)addr);
	PDEBUG_REG("me_readb(0x%p : 0x%02X)=%d\n", addr, *val, 0);
}

void me_readw(void* dev, uint16_t* val, volatile void* addr)
{
	*val = (uint16_t)ioread16((void *)addr);
	PDEBUG_REG("me_readw(0x%p : 0x%04X)=%d\n", addr, *val, 0);
}

void me_readl(void* dev, uint32_t* val, volatile void* addr)
{
	*val = (uint32_t)ioread32((void *)addr);
	PDEBUG_REG("me_readl(0x%p : 0x%08X)=%d\n", addr, *val, 0);
}

# endif
