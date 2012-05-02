/*******************************************************************************
 * Copyright c PLX Technology, Inc.
 * Copyright C 2010 Meilhaus Electronic GmbH support@meilhaus.de
 *
 * PLX Technology Inc. licenses this source file under the GNU Lesser General Public
 * License LGPL version 2.  This source file may be modified or redistributed
 * under the terms of the LGPL and without express permission from PLX Technology.
 *
 * PLX Technology, Inc. provides this software AS IS, WITHOUT ANY WARRANTY,
 * EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  PLX makes no guarantee
 * or representations regarding the use of, or the results of the use of,
 * the software and documentation in terms of correctness, accuracy,
 * reliability, currentness, or otherwise; and you rely on the software,
 * documentation and results solely at your own risk.
 *
 * IN NO EVENT SHALL PLX BE LIABLE FOR ANY LOSS OF USE, LOSS OF BUSINESS,
 * LOSS OF PROFITS, INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES
 * OF ANY KIND.
 *
 ******************************************************************************/

/******************************************************************************
 *
 * File Name:
 *
 *      me_plx9056_reg.h -> Based on Reg9056.h from PLX SDK v5.00
 *
 * Description:
 *
 *      This file defines the PLX 9056 chip registers.
 *
 ******************************************************************************/

#ifndef _ME_PLX9056_REG_H_
# define _ME_PLX9056_REG_H_

// PCI Configuration Registers
# define PLX9056_PM_CAP_ID            0x040
# define PLX9056_PM_CSR               0x044
# define PLX9056_HS_CAP_ID            0x048
#  define PLX9056_HS_CSR              0x04A
# define PLX9056_VPD_CAP_ID           0x04c
# define PLX9056_VPD_DATA             0x050

// Additional defintions
# define PLX9056_MAX_REG_OFFSET       0x108
# define PLX9056_EEPROM_SIZE          0x064          // EEPROM size in bytes used by PLX Chip
# define PLX9056_DMA_CHANNELS         2              // Number of DMA channels supported by PLX Chip

// Local Configuration Registers
# define PLX9056_SPACE0_RANGE         0x000
# define PLX9056_SPACE0_REMAP         0x004
# define PLX9056_LOCAL_DMA_ARBIT      0x008
# define PLX9056_ENDIAN_DESC          0x00c
# define PLX9056_LATENCY_TIMER        0x00d
# define PLX9056_EXP_ROM_RANGE        0x010
# define PLX9056_EXP_ROM_REMAP        0x014
# define PLX9056_SPACE0_ROM_DESC      0x018
# define PLX9056_DM_RANGE             0x01c
# define PLX9056_DM_MEM_BASE          0x020
# define PLX9056_DM_IO_BASE           0x024
# define PLX9056_DM_PCI_MEM_REMAP     0x028
# define PLX9056_DM_PCI_IO_CONFIG     0x02c
# define PLX9056_SPACE1_RANGE         0x0f0
# define PLX9056_SPACE1_REMAP         0x0f4
# define PLX9056_SPACE1_DESC          0x0f8
# define PLX9056_DM_DAC               0x0fc
# define PLX9056_ARBITER_CTRL         0x100
# define PLX9056_ABORT_ADDRESS        0x104


# define PLX9056_MAILBOX0             0x040
# define PLX9056_MAILBOX1             0x044
# define PLX9056_MAILBOX2             0x048
# define PLX9056_MAILBOX3             0x04c
# define PLX9056_MAILBOX4             0x050
# define PLX9056_MAILBOX5             0x054
# define PLX9056_MAILBOX6             0x058
# define PLX9056_MAILBOX7             0x05c
# define PLX9056_LOCAL_DOORBELL       0x060
# define PLX9056_PCI_DOORBELL         0x064
# define PLX9056_INT_CTRL_STAT        0x068
# define PLX9056_EEPROM_CTRL_STAT     0x06c
# define PLX9056_PERM_VENDOR_ID       0x070
# define PLX9056_REVISION_ID          0x074


// DMA Registers
# define PLX9056_DMA0_MODE				0x080
# define PLX9056_DMA0_PCI_ADDR			0x084
# define PLX9056_DMA0_LOCAL_ADDR		0x088
# define PLX9056_DMA0_COUNT				0x08c
#  define PLX9056_DMA0_COUNT_RING		0x084
#  define PLX9056_DMA0_PCI_ADDR_RING	0x088
#  define PLX9056_DMA0_LOCAL_ADDR_RING	0x08c
# define PLX9056_DMA0_DESC_PTR			0x090
# define PLX9056_DMA1_MODE				0x094
# define PLX9056_DMA1_PCI_ADDR			0x098
# define PLX9056_DMA1_LOCAL_ADDR		0x09c
# define PLX9056_DMA1_COUNT				0x0a0
#  define PLX9056_DMA1_COUNT_RING		0x0a0
#  define PLX9056_DMA1_PCI_ADDR_RING	0x098
#  define PLX9056_DMA1_LOCAL_ADDR_RING	0x09c
# define PLX9056_DMA1_DESC_PTR			0x0a4
# define PLX9056_DMA_COMMAND_STAT		0x0a8
# define PLX9056_DMA0_STAT				0x0a8
# define PLX9056_DMA1_STAT				0x0a9
# define PLX9056_DMA_ARBIT				0x0ac
# define PLX9056_DMA_THRESHOLD			0x0b0
# define PLX9056_DMA0_PCI_DAC 			0x0b4
# define PLX9056_DMA1_PCI_DAC			0x0b8


// Messaging Unit Registers
# define PLX9056_OUTPOST_INT_STAT		0x030
# define PLX9056_OUTPOST_INT_MASK		0x034
# define PLX9056_MU_CONFIG				0x0c0
# define PLX9056_FIFO_BASE_ADDR			0x0c4
# define PLX9056_INFREE_HEAD_PTR		0x0c8
# define PLX9056_INFREE_TAIL_PTR		0x0cc
# define PLX9056_INPOST_HEAD_PTR		0x0d0
# define PLX9056_INPOST_TAIL_PTR		0x0d4
# define PLX9056_OUTFREE_HEAD_PTR		0x0d8
# define PLX9056_OUTFREE_TAIL_PTR		0x0dc
# define PLX9056_OUTPOST_HEAD_PTR		0x0e0
# define PLX9056_OUTPOST_TAIL_PTR		0x0e4
# define PLX9056_FIFO_CTRL_STAT			0x0e8

#endif	//_ME_PLX9056_REG_H_
