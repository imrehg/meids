/**
 * @file memain_pci.h
 *
 * @brief Header for loader module for Meilhaus Driver System (PCI bus).
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
# ifndef _MEMAIN_PCI_H_
#  define _MEMAIN_PCI_H_

#  include "me_internal.h"
#  include "medevice.h"

	static struct pci_device_id me_pci_table[] __devinitdata =
	{
	{MEILHAUS_PCI_VENDOR_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0 }
	};

	MODULE_DEVICE_TABLE(pci, me_pci_table);

# endif	//_MEMAIN_PCI_H_
#endif	//__KERNEL__
