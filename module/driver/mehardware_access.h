/**
 * @file mehardware_access.h
 *
 * @brief Provide replacement for standard port/memory access functions.
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

# ifndef _MEHARDWARE_ACCESS_H_
#  define _MEHARDWARE_ACCESS_H_

void me_writeb(void* dev, uint8_t  val, volatile void* addr);
void me_writew(void* dev, uint16_t val, volatile void* addr);
void me_writel(void* dev, uint32_t val, volatile void* addr);

void me_readb(void* dev, uint8_t*  val, volatile void* addr);
void me_readw(void* dev, uint16_t* val, volatile void* addr);
void me_readl(void* dev, uint32_t* val, volatile void* addr);

#  ifdef ME_USB

#   define MAX_REPEATS 4

	int me_DMA_lock(void* dev, volatile void* addr, uint32_t* mirror);
	int me_DMA_unlock(void* dev, volatile void* addr, uint32_t mirror);
	int me_DMA_write(void* dev, uint32_t* buff, int size, void* dest);
	int me_DMA_read(void* dev, uint32_t* buff, int size, void* dest);

	int access_test(void* dev);

#  endif

# endif	//_MEHARDWARE_ACCESS_H_
#endif	//__KERNEL__
