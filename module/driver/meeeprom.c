/**
 * @file meeeprom.c
 *
 * @brief Implements the PLX 9052/9050/9030 EEPROM access.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 20078 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# include "me_internal.h"

# include <linux/delay.h>

# include "medevice.h"
# include "meeeprom.h"

static void me_plx_eeprom_clk(me_general_dev_t* device, void* register_base_control, uint32_t* val);
static void me_plx_eeprom_enable(me_general_dev_t* device, void* register_base_control, uint32_t* val);
static void me_plx_eeprom_disable(me_general_dev_t* device, void* register_base_control, uint32_t* val);
static void me_plx_eeprom_cmd(me_general_dev_t* device, void* register_base_control, uint8_t op_code, uint8_t cmd, int address_lenght, uint32_t* val);
static void me_plx_eeprom_address(me_general_dev_t* device, void* register_base_control, uint8_t address, int address_lenght, uint32_t* val);

static void me_plx_eeprom_clk(me_general_dev_t* device, void* register_base_control, uint32_t* val)
{
    // Clock low
	*val |= PLX9052_ICR_BIT_EEPROM_CLOCK_SET;
	me_writel(device, *val, register_base_control + PLX9052_ICR);
	udelay(EEPROM_DELAY);

    // Clock high
	*val &= ~PLX9052_ICR_BIT_EEPROM_CLOCK_SET;
	me_writel(device, *val, register_base_control + PLX9052_ICR);
	udelay(EEPROM_DELAY);

}

static void me_plx_eeprom_enable(me_general_dev_t* device, void* register_base_control, uint32_t* val)
{
	/// @note: Rising edge of CS starts command.

	// Clear instruction and clock bits
	*val &= ~(PLX9052_ICR_BIT_EEPROM_WRITE | PLX9052_ICR_BIT_EEPROM_CLOCK_SET);
	// Chip select FALSE
	*val &= ~PLX9052_ICR_BIT_EEPROM_CHIP_SELECT;
	me_writel(device, *val, register_base_control + PLX9052_ICR);
	udelay(EEPROM_DELAY);

	// Chip select TRUE
	*val |= PLX9052_ICR_BIT_EEPROM_CHIP_SELECT;
	me_writel(device, *val, register_base_control + PLX9052_ICR);
}

static void me_plx_eeprom_disable(me_general_dev_t* device, void* register_base_control, uint32_t* val)
{
	// Chip select FALSE. Clear instruction and clock bits
	*val &= ~(PLX9052_ICR_BIT_EEPROM_CHIP_SELECT | PLX9052_ICR_BIT_EEPROM_WRITE | PLX9052_ICR_BIT_EEPROM_CLOCK_SET);
	me_writel(device, *val, register_base_control + PLX9052_ICR);
}

static void me_plx_eeprom_cmd(me_general_dev_t* device, void* register_base_control, uint8_t op_code, uint8_t cmd, int address_lenght, uint32_t* val)
{
	int index;

	// OP_CODE: 3 bits
	for  (index=2; index>=0; index--)
	{
		*val = (op_code & (0x1 << index)) ? *val | PLX9052_ICR_BIT_EEPROM_WRITE : *val & ~PLX9052_ICR_BIT_EEPROM_WRITE;
		me_writel(device, *val, register_base_control + PLX9052_ICR);
		me_plx_eeprom_clk(device, register_base_control, val);
	}

	// Command: 2 active bits
	for  (index=1; index>=0; index--)
	{
		*val = (cmd & (0x1 << index)) ? *val | PLX9052_ICR_BIT_EEPROM_WRITE : *val & ~PLX9052_ICR_BIT_EEPROM_WRITE;
		me_writel(device, *val, register_base_control + PLX9052_ICR);
		me_plx_eeprom_clk(device, register_base_control, val);
	}

	// Command: nonactive bits
	for  (index=address_lenght-1-2; index>=0; index--)
	{
		me_plx_eeprom_clk(device, register_base_control, val);
	}
}

static void me_plx_eeprom_OP_CODE(me_general_dev_t* device, void* register_base_control, uint8_t op_code, uint32_t* val)
{
	int index;

	// OP_CODE 3 bits
	for  (index=2; index>=0; index--)
	{
		*val = (op_code & (0x1 << index)) ? *val | PLX9052_ICR_BIT_EEPROM_WRITE : *val & ~PLX9052_ICR_BIT_EEPROM_WRITE;
		me_writel(device, *val, register_base_control + PLX9052_ICR);
		me_plx_eeprom_clk(device, register_base_control, val);
	}

}

static void me_plx_eeprom_address(me_general_dev_t* device, void* register_base_control, uint8_t address, int address_lenght, uint32_t* val)
{
	int index;

	for  (index=address_lenght-1; index>=0; index--)
	{
		*val = (address & (0x1 << index)) ? *val | PLX9052_ICR_BIT_EEPROM_WRITE : *val & ~PLX9052_ICR_BIT_EEPROM_WRITE;
		me_writel(device, *val, register_base_control + PLX9052_ICR);
		me_plx_eeprom_clk(device, register_base_control, val);
	}

}

int me_plx_eeprom_check(me_general_dev_t* device, void* register_base_control)
{
	uint32_t tmp;
	me_readl(device, &tmp, register_base_control + PLX9052_ICR);
	if (tmp & PLX9052_ICR_BIT_EEPROM_VALID)
	{
		return ME_ERRNO_SUCCESS;
	}

	PERROR("No valid PCI bridge eeprom detected!\n");
	return ME_ERRNO_LACK_OF_RESOURCES;
}

int me_plx_eeprom_address_lenght_check(me_general_dev_t* device, void* register_base_control, int* address_lenght)
{
	uint32_t tmp;
	int index;
	uint16_t val = 0;


	me_readl(device, &tmp, register_base_control + PLX9052_ICR);

	me_plx_eeprom_enable(device, register_base_control, &tmp);

	// OP_CODE:READ 3 bits
	me_plx_eeprom_OP_CODE(device, register_base_control, EEPROM_READ_OP_CODE, &tmp);

	// Set out line to permanent low.
	tmp &= ~PLX9052_ICR_BIT_EEPROM_WRITE;

	// Generate CLKs (address + devID + venID)
	for  (index=1; index<48; index++)
	{
		me_plx_eeprom_clk(device, register_base_control, &tmp);
		me_readl(device, &tmp, register_base_control + PLX9052_ICR);

		val <<= 1;
		if (tmp & PLX9052_ICR_BIT_EEPROM_READ)
		{
			val |= 0x1;
		}

		// Bits b16:b31 are used for storing vendor ID
		if ((index>32) && (val == PCI_VENDOR_ID_MEILHAUS))
			break;
	}

	me_plx_eeprom_disable(device, register_base_control, &tmp);

	// Calculate address lenght.
	*address_lenght = index-32;
	PINFO("PLX EEPROM address lenght=%d\n", *address_lenght);

	return ME_ERRNO_SUCCESS;
}

int me_plx_eeprom_read(me_general_dev_t* device, void* register_base_control, int eeprom_address_lenght, uint8_t address, uint16_t* values, int lenght)
{
	uint32_t tmp;
	int index;
	int field_index;
	uint16_t val = 0;

	me_readl(device, &tmp, register_base_control + PLX9052_ICR);

	me_plx_eeprom_enable(device, register_base_control, &tmp);

	// OP_CODE 3 bits
	me_plx_eeprom_OP_CODE(device, register_base_control, EEPROM_READ_OP_CODE, &tmp);

	// Address
	me_plx_eeprom_address(device, register_base_control, address, eeprom_address_lenght, &tmp);

	// Input line must be low. Check it
	me_readl(device, &tmp, register_base_control + PLX9052_ICR);
	if (tmp & PLX9052_ICR_BIT_EEPROM_READ)
	{
		PERROR("READ from EEPROM can not be done!\n");
		return ME_ERRNO_INTERNAL;
	}

	// Set out line to permanent low.
	tmp &= ~PLX9052_ICR_BIT_EEPROM_WRITE;

	for  (field_index=0; field_index<lenght; field_index++)
	{
		for  (index=0; index<16; index++)
		{
			me_plx_eeprom_clk(device, register_base_control, &tmp);
			me_readl(device, &tmp, register_base_control + PLX9052_ICR);

			val <<= 1;
			if (tmp & PLX9052_ICR_BIT_EEPROM_READ)
			{
				val |= 0x1;
			}
		}
		*(values + field_index) = val;
	}

	me_plx_eeprom_disable(device, register_base_control, &tmp);

	return ME_ERRNO_SUCCESS;
}

int me_plx_eeprom_write(me_general_dev_t* device, void* register_base_control, int eeprom_address_lenght, uint8_t address, uint16_t* values, int lenght)
{
	uint32_t tmp;
	int index;
	int field_index;
	uint16_t val = 0;

	me_readl(device, &tmp, register_base_control + PLX9052_ICR);

	me_plx_eeprom_enable(device, register_base_control, &tmp);

	me_plx_eeprom_cmd(device, register_base_control, EEPROM_WEN_OP_CODE, EEPROM_WEN_ADDR, eeprom_address_lenght, &tmp);

	for  (field_index=0; field_index<lenght; field_index++)
	{
		val = *(values + field_index);
		for  (index=15; index>=0; index--)
		{
			tmp = (val & (0x1 << index)) ? tmp | PLX9052_ICR_BIT_EEPROM_WRITE : tmp & ~PLX9052_ICR_BIT_EEPROM_WRITE;
			me_writel(device, tmp, register_base_control + PLX9052_ICR);
			me_plx_eeprom_clk(device, register_base_control, &tmp);
		}
	}

	me_plx_eeprom_cmd(device, register_base_control, EEPROM_WDS_OP_CODE, EEPROM_WDS_ADDR, eeprom_address_lenght, &tmp);

	me_plx_eeprom_disable(device, register_base_control, &tmp);

	return ME_ERRNO_SUCCESS;
}
