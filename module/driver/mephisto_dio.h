/**
 * @file mephisto_dio.h
 *
 * @brief MephistoScope digital input/output subdevice class.
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

# ifndef _MEPHISTO_DIO_H_
#  define _MEPHISTO_DIO_H_

#  include "mesubdevice.h"

#  include "mephisto_defines.h"

#  define MEPHISTO_DIO_CAPS				(ME_CAPS_DIO_DIR_WORD | ME_CAPS_DIO_DIR_DWORD)

/**
 * @brief The MephistoScope digital input/output subdevice class.
 */
typedef struct //mephisto_dio_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	struct semaphore* 	device_semaphore;
	volatile mephisto_AI_status_e* status;

	uint32_t cfg_copy;
	uint32_t data_copy;

	// Attributes
} mephisto_dio_subdevice_t;


/**
 * @brief The constructor to generate a MephistoScope digital input/ouput subdevice instance.
 *
 * @param idx The index of the digital i/o port on the device.
 *
 * @return Pointer to new instance on success.\n
 * NULL on error.
 */
mephisto_dio_subdevice_t* mephisto_dio_constr(unsigned int idx, mephisto_AI_status_e* status, struct semaphore* device_semaphore);

# endif
#endif
