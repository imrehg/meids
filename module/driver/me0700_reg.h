/**
 * @file me0700_reg.h
 *
 * @brief ME-0700 register definitions.
 * @note Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2009 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# ifndef _ME0700_REG_H_
#  define _ME0700_REG_H_
#  include "me4600_reg.h"

#  define ME0700_IRQ_STATUS_REG				ME4600_IRQ_STATUS_REG
#  define ME0700_IRQ_STATUS_BIT_EX			ME4600_IRQ_STATUS_BIT_EX
#  define ME0700_IRQ_STATUS_BIT_LE			ME4600_IRQ_STATUS_BIT_LE
#  define ME0700_IRQ_STATUS_BIT_AI_HF		ME4600_IRQ_STATUS_BIT_AI_HF
#  define ME0700_IRQ_STATUS_BIT_AO_0_HF		ME4600_IRQ_STATUS_BIT_AO_0_HF
#  define ME0700_IRQ_STATUS_BIT_AO_1_HF		ME4600_IRQ_STATUS_BIT_AO_1_HF
#  define ME0700_IRQ_STATUS_BIT_AO_2_HF		ME4600_IRQ_STATUS_BIT_AO_2_HF
#  define ME0700_IRQ_STATUS_BIT_AO_3_HF		ME4600_IRQ_STATUS_BIT_AO_3_HF
#  define ME0700_IRQ_STATUS_BIT_SC			ME4600_IRQ_STATUS_BIT_SC
#  define ME0700_IRQ_STATUS_BIT_AI_OF		ME4600_IRQ_STATUS_BIT_AI_OF

#  define ME0700_IRQ_STATUS_BIT_AO_HF		ME4600_IRQ_STATUS_BIT_AO_HF

# endif
#endif
