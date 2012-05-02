/**
 * @file mefirmware.c
 *
 * @brief Implements the firmware handling (loading).
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

# ifndef KBUILD_MODNAME
#  define KBUILD_MODNAME KBUILD_STR(mefirmware)
# endif

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_plx9052_reg.h"
# include "me_plx9056_reg.h"

# include <linux/delay.h>
# include <linux/firmware.h>

# include "medevice.h"
# include "mefirmware.h"

# define STD_FW_DIR	"meids/"

# ifndef KERNEL_RELEASE
#  define KERNEL_RELEASE "default/"
# endif

static const struct firmware* me_get_firmware(const char* firmware_name, struct device* dev);

static int  me46xx_fimware_init(me_general_dev_t* hw_device, void* register_base_control, void* register_base_data);
static void me46xx_fimware_deinit(me_general_dev_t* hw_device, void* register_base_control);
static int  me46xx_fimware_test_DONE(me_general_dev_t* hw_device, void* register_base_control);
static int  me46xx_fimware_write(me_general_dev_t* hw_device, void* register_base_control, void* register_base_data, const struct firmware* fw);

int me_xilinx_download(me_general_dev_t* hw_device,
						void* register_base_control,
						void* register_base_data,
						const char* firmware_name)
{
	int ret = ME_ERRNO_SUCCESS;
	const struct firmware* fw;
#ifdef MEDEBUG_INFO
	long unsigned int load_start_time, load_stop_time;
#endif

	PDEBUG("executed.\n");

	fw = me_get_firmware(firmware_name, &((hw_device->dev)->dev));
	if (!fw)
	{
		PERROR("Request for firmware failed.\n");
		return ME_FIRMWARE_ERROR;
	}

	ret = me46xx_fimware_init(hw_device, register_base_control, register_base_data);
	if (ret == ME_FIRMWARE_DOWNLOAD_DISABLE)
	{
		release_firmware(fw);
		return ME_ERRNO_SUCCESS;
	}

#ifdef MEDEBUG_INFO
	load_start_time = jiffies;
#endif

	// Download Xilinx firmware
	me46xx_fimware_write(hw_device, register_base_control, register_base_data, fw);

	PDEBUG("Firmware %s download finished. %ld bytes written to PLX.\n", firmware_name, (long int)fw->size);
#ifdef MEDEBUG_INFO
	load_stop_time = jiffies;
	load_stop_time -= load_start_time;
	load_stop_time *= 1000;
	load_stop_time /= HZ;

	PINFO("Download time: %ldms (%ldns/value)\n", load_stop_time, (load_stop_time*1000000)/fw->size);
#endif

	ret = me46xx_fimware_test_DONE(hw_device, register_base_control);

	me46xx_fimware_deinit(hw_device, register_base_control);
	release_firmware(fw);

	return ret;
}

static const struct firmware* me_get_firmware(const char* firmware_name, struct device* dev)
{
	char* fw_name;
	const struct firmware* fw;
	int err = 0;

	PINFO("executed.\n");

	if (!firmware_name)
	{
		PERROR("Request for firmware failed. No name provided. \n");
		return NULL;
	}

	fw_name = kzalloc(strlen(KERNEL_RELEASE) + strlen(STD_FW_DIR) + strlen(firmware_name) + 1, GFP_KERNEL);
	if (!fw_name)
	{
		PERROR("Request for memory failed.\n");
		return NULL;
	}

	strncpy(fw_name, KERNEL_RELEASE, strlen(KERNEL_RELEASE));
	strncat(fw_name, STD_FW_DIR, strlen(STD_FW_DIR));
	strncat(fw_name, firmware_name, strlen(firmware_name));

	PINFO("Request '%s' firmware.\n", firmware_name);
	if (request_firmware(&fw, fw_name, dev))
	{
		if (request_firmware(&fw, fw_name + strlen(KERNEL_RELEASE), dev))
		{
			err = request_firmware(&fw, firmware_name, dev);
			if (err)
			{
				PERROR("Request for firmware failed.\n");
			}
		}
	}

	kfree(fw_name);
	return (err) ? NULL : fw;
}

static int me46xx_fimware_init(me_general_dev_t* hw_device, void* register_base_control, void* register_base_data)
{
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	// Set PLX local interrupt 2 polarity to high.
	// Interrupt is thrown by init pin of xilinx.
	me_readl(hw_device, &tmp, register_base_control + PLX9052_INTCSR);
	tmp |= PLX9052_INTCSR_LOCAL_INT2_POL;
	me_writel(hw_device, tmp, register_base_control + PLX9052_INTCSR);

	// Set /CS and /WRITE of the Xilinx
	me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
	tmp |= ME_FIRMWARE_CS_WRITE;
	me_writel(hw_device, tmp, register_base_control + PLX9052_ICR);

	// Init Xilinx with CS1
	me_readl(hw_device, &tmp, register_base_data + ME_XILINX_CS1_REG);

	// Wait for init to complete
	udelay(20);

	me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
	if (tmp & ME_FIRMWARE_DONE_FLAG)
	{
		PDEBUG("Firmware in PROM detected.\n");
		return ME_FIRMWARE_DOWNLOAD_DISABLE;
	}

	// Check /INIT pin
	me_readl(hw_device, &tmp, register_base_control + PLX9052_INTCSR);
	if (!(tmp & PLX9052_INTCSR_LOCAL_INT2_STATE))
	{
		PERROR("Can't init Xilinx.\n");
		return ME_FIRMWARE_INIT_ERROR;
	}
	else
	{
		// Reset /CS and /WRITE of the Xilinx
		me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
		tmp &= ~ME_FIRMWARE_CS_WRITE;
		me_writel(hw_device, tmp, register_base_control + PLX9052_ICR);
		udelay(10);
	}

	return ME_ERRNO_SUCCESS;
}

static void me46xx_fimware_deinit(me_general_dev_t* hw_device, void* register_base_control)
{
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	// Set /CS and /WRITE
	me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
	tmp |= ME_FIRMWARE_CS_WRITE;
	me_writel(hw_device, tmp, register_base_control + PLX9052_ICR);

	me_writel(hw_device, ME_PLX9052_PCI_INTS_BLOCKED, register_base_control + PLX9052_INTCSR);
}

static int me46xx_fimware_test_DONE(me_general_dev_t* hw_device, void* register_base_control)
{
	uint32_t tmp;

	PDEBUG("executed.\n");

	me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
	if (!(tmp & ME_FIRMWARE_DONE_FLAG))
	{
		PERROR("FAILURE. DONE flag is not set. (0x%x)\n", tmp);
		return ME_FIRMWARE_DONE_ERROR;
	}
	return ME_ERRNO_SUCCESS;
}

static int me46xx_fimware_write(me_general_dev_t* hw_device, void* register_base_control, void* register_base_data, const struct firmware* fw)
{
	int idx;
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	// Download Xilinx firmware
	for (idx = 0; idx < fw->size; idx++)
	{
		me_writel(hw_device, fw->data[idx], register_base_data);
#ifndef ME6000_v2_4
///	This checking is for me6000 board's version 2.4 only
		// Check if BUSY flag is set (low = ready, high = busy)
		me_readl(hw_device, &tmp, register_base_control + PLX9052_ICR);
		if (tmp & ME_FIRMWARE_BUSY_FLAG)
		{
			PERROR("Xilinx is still busy (after writing %d bytes)\n", idx);
			return ME_FIRMWARE_DONE_ERROR;
		}
#endif	//ME6000_v2_4
	}
	return ME_ERRNO_SUCCESS;
}


# ifdef ME_USB
// #  define ME_DMA_DOWNLOAD_BLOCK_SIZE			0x1000
#  define ME_DMA_DOWNLOAD_BLOCK_SIZE			0x100

static void me_NET2282_FIFO_format(me_general_dev_t* hw_device);

static void me_NET2282_FIFO_format(me_general_dev_t* hw_device)
{/// Workaround for NET2282 bug. - When using FIFO bigger than 2KB first packet is not ACK.
	uint32_t* dma_buffer;
	int err;

	PDEBUG("executed.\n");

	dma_buffer = (uint32_t *)kzalloc(ME_DMA_DOWNLOAD_BLOCK_SIZE * sizeof(uint32_t), GFP_KERNEL);
	if (dma_buffer)
	{
		err = NET2282_write_FIFO(hw_device, NET2282_EP_A, dma_buffer, ME_DMA_DOWNLOAD_BLOCK_SIZE);
		if (err == -ETIMEDOUT)
		{

			PERROR("NET2282 bug. This write can finish with timeout.\n");
		}

		//Start DMA
		NET2282_NET2282_reg_write(hw_device,
								(	0x0
									| NET2282_DMA_STAT_TRANSFER_START
// 									| NET2282_DMA_STAT_ABORT
									| NET2282_DMA_STAT_DONE_IRQ_CLS
									| NET2282_DMA_STAT_SCATTER_GATHER_IRQ_CLS
									| NET2282_DMA_STAT_PAUSE_IRQ_CLS
									| NET2282_DMA_STAT_ABORT_IRQ_CLS
								),
								NET2282_CHA_DMASTAT);

		kfree(dma_buffer);
	}

}

int me_fimware_download_NET2282_DMA(me_general_dev_t* hw_device,
						void* register_base_control,
						void* register_base_data,
						uint32_t dma_base_data,
						const char* firmware_name)
{
	int idx = 0;
	int ret = ME_ERRNO_SUCCESS;

	int i;
	int sum = 0;
	uint32_t* dma_buffer;

#ifdef MEDEBUG_INFO
	long unsigned int load_start_time, load_stop_time;
#endif

	const struct firmware* fw;

	PDEBUG("executed.\n");

	fw = me_get_firmware(firmware_name, &((hw_device->dev)->dev));
	if (!fw)
	{
		PERROR("Request for firmware failed.\n");
		return ME_FIRMWARE_ERROR;
	}

	dma_buffer = (uint32_t *)kzalloc(ME_DMA_DOWNLOAD_BLOCK_SIZE * sizeof(uint32_t), GFP_KERNEL);
	if (!dma_buffer)
	{
		PERROR("Can't get requestet memmory.\n");
		release_firmware(fw);
		return -ENOMEM;
	}

	// Write NET2282 DMA ADDRESS
	NET2282_NET2282_reg_write(hw_device, dma_base_data, NET2282_CHA_DMAADDR);
	// Write NET2282 DMA COUNT
	NET2282_NET2282_reg_write(hw_device,
								0x00800000 |
								(	0x0
//									| NET2282_DMA_COUNT_CONTINUE
// 									| NET2282_DMA_COUNT_SCATTER_GATHER_FIFO_VALIDATE
// 									| NET2282_DMA_COUNT_END_OF_CHAIN
// 									| NET2282_DMA_COUNT_DONE_IRQ_ENABLE
// 									| NET2282_DMA_COUNT_TRANSFER_DIRECTION	//0-OUT 1-IN
									| NET2282_DMA_COUNT_VALID_BIT
								),
								NET2282_CHA_DMACOUNT);

	// This workaround is necessary when transfered buffer is bigger than 2KB.
	me_NET2282_FIFO_format(hw_device);

	ret = me46xx_fimware_init(hw_device, register_base_control, register_base_data);
	if (ret == ME_FIRMWARE_DOWNLOAD_DISABLE)
	{
		release_firmware(fw);
		return ME_ERRNO_SUCCESS;
	}

#ifdef MEDEBUG_INFO
	load_start_time = jiffies;
#endif

	PDEBUG("Download starteded.\n");
	// Download Xilinx firmware
	for (idx = 0; idx < fw->size; idx += ME_DMA_DOWNLOAD_BLOCK_SIZE)
	{
		for (i=0; i < ME_DMA_DOWNLOAD_BLOCK_SIZE; i++)
		{
			if (fw->size > idx+ i)
				*(dma_buffer + i) = (uint32_t)fw->data[i+ idx];
			else
				break;
		}
			// 	Write FIFO
		NET2282_write_FIFO(hw_device, NET2282_EP_A, dma_buffer, i);
		sum += i;
		//Start DMA
		NET2282_NET2282_reg_write(hw_device,
									(	0x0
										| NET2282_DMA_STAT_TRANSFER_START
// 										| NET2282_DMA_STAT_ABORT
										| NET2282_DMA_STAT_DONE_IRQ_CLS
										| NET2282_DMA_STAT_SCATTER_GATHER_IRQ_CLS
										| NET2282_DMA_STAT_PAUSE_IRQ_CLS
										| NET2282_DMA_STAT_ABORT_IRQ_CLS
									),
									NET2282_CHA_DMASTAT);
	}

	PDEBUG("Firmware %s download finished. %ld bytes written to PLX.\n", firmware_name, (long int)fw->size);
#ifdef MEDEBUG_INFO
	load_stop_time = jiffies;
	load_stop_time -= load_start_time;
	load_stop_time *= 1000;
	load_stop_time /= HZ;

	PINFO("Download time: %ldms (%ldns/value)\n", load_stop_time, (load_stop_time*1000000)/fw->size);
#endif

	ret = me46xx_fimware_test_DONE(hw_device, register_base_control);

	me46xx_fimware_deinit(hw_device, register_base_control);

	kfree(dma_buffer);
	NET2282_DMA_init(hw_device);
	release_firmware(fw);

	return ret;
}
# endif	//ME_USB
