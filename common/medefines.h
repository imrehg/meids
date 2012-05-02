/**
 * @file medefines.h
 *
 * @brief Global definitions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _MEDEFINES_H_
# define _MEDEFINES_H_

# include "../osi/medefines.h"

#define ME_OPEN_PCI									0x01
#define ME_OPEN_USB									0x02
#define ME_OPEN_RPC									0x10

#define ME_OPEN_ALL									(ME_OPEN_PCI | ME_OPEN_USB | ME_OPEN_RPC)
#define ME_OPEN_LOCAL								(ME_OPEN_PCI | ME_OPEN_USB)


#define ME_WAIT_START								0x00140004
#define ME_WAIT_STOP								0x00140005

#define ME_TIMER_OSCILLOSCOPE_FLAG					0x10000000
#define ME_MEPHISTO_SCOPE_OSCILLOSCOPE_FLAG			0x01

#endif	//_MEDEFINES_H_
