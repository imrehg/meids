/**
 * @file mefirmware.h
 *
 * @brief Definitions of the firmware handling functions.
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


#ifdef __KERNEL__

# ifndef _MEFIRMWARE_H_
#  define _MEFIRMWARE_H_

# include "medevice.h"

///Registry
#  define ME_XILINX_CS1_REG		0x00C8

///Flags (bits)

#  define ME_FIRMWARE_BUSY_FLAG	0x00000020
#  define ME_FIRMWARE_DONE_FLAG	0x00000004
#  define ME_FIRMWARE_CS_WRITE	0x00000100

/// @note Firmware download reserve line INT2. All other interrupts are conected to line INT1.

int me_xilinx_download(me_general_dev_t* device,
						void* register_base_control,
						void* register_base_data,
						const char* firmware_name);

#  ifdef ME_USB
int me_fimware_download_NET2282_DMA(me_general_dev_t* hw_device,
						void* register_base_control,
						void* register_base_data,
						uint32_t dma_base_data,
						const char* firmware_name);

#  endif // ME_USB
# endif //_MEFIRMWARE_H_
#endif //__KERNEL__
