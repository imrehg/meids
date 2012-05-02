/**
 * @file meeeprom.h
 *
 * @brief PLX 9052/9050/9030 EEPROM access.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
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

#ifndef _MEEEPROM_H_
# define _MEEEPROM_H_

# include "medevice.h"
# include "me_plx9052_reg.h"

# define EEPROM_DELAY				0x00
/// @note: OP_CODE are 3 bits long
# define EEPROM_READ_OP_CODE		0x06		/* read instruction operating code */
# define EEPROM_WRITE_OP_CODE		0x05		/* write instruction operating code */
# define EEPROM_WEN_OP_CODE			0x04		/* write enable instruction operating code */
# define EEPROM_WDS_OP_CODE			0x04		/* write disable instruction operating code */

/// @note: 2 highes bits of addres
# define EEPROM_WEN_ADDR			0x03		/* write enable instruction address */
# define EEPROM_WDS_ADDR			0x00		/* write disable instruction address */

/// @note: data size in read/write is 16 bits
int me_plx_eeprom_check(me_general_dev_t* device, void* register_base_control);
int me_plx_eeprom_address_lenght_check(me_general_dev_t* device, void* register_base_control, int* address_lenght);
int me_plx_eeprom_read(me_general_dev_t* device, void* register_base_control, int eeprom_address_lenght, uint8_t address, uint16_t* values, int lenght);
int me_plx_eeprom_write(me_general_dev_t* device, void* register_base_control, int eeprom_address_lenght, uint8_t address, uint16_t* values, int lenght);
#endif
