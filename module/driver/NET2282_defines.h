/**
 * @file NET2282_defines.h
 *
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @brief NET2282 registers' definitions.
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

#ifndef _NET2282_DEFINES_H_
# define _NET2282_DEFINES_H_

#include <linux/pci_regs.h>

#define ME_IRQ_INTERVAL										1

///Default endpoints
#define NET2282_EP_CFG										0x0d
#define NET2282_EP_PCI										0x0e
#define NET2282_EP_IRQ										0x0f

#define NET2282_EP_A										0x01
#define NET2282_EP_B										0x02
#define NET2282_EP_C										0x03
#define NET2282_EP_D										0x04
#define NET2282_EP_E										0x05
#define NET2282_EP_F										0x06

///Access to NET2282 'OWN' PCI sections
//Use with NET2282_NET2282_CFG_xxx()
// Access to PCI configuration registers.
#define NET2282_PCI_CFG_REG									0x0000
// Access to memory mapped config registers.
#define NET2282_PCI_MEM_REG									0x0010
// Access to 8051 program RAM.
#define NET2282_8051_RAM									0x0020

// Chapter 11.7 in 'NET2282 PCI USB 2.0 High-Speed Peripheral Controller'
// PCI Master Byte Enables Determines which PCI Byte Enables (CBE[3:0]#) are asserted during a Master cycle Data phase.

// Chapter 7.6.5.1 in 'NET2282 PCI USB 2.0 High-Speed Peripheral Controller'
// The Byte Enables determine which of the four bytes in the selected Configuration register are accessed (Byte Enable 0 corresponds to bits [7:0], and so forth).

// Chapter 2.7 in 'NET2282 PCI USB 2.0 High-Speed Peripheral Controller'
// CBE[3:0]# contain the Byte Enables during the Data phase.
// CBE0# corresponds to byte 0 (AD[7:0])
// CBE1# corresponds to byte 1 (AD[15:8])
// CBE2# corresponds to byte 2 (AD[23:16])
// CBE3# corresponds to byte 3 (AD[31:24])

/// PCI registers
#define NET2282_PCIVENDID									PCI_VENDOR_ID
#define NET2282_PCIDEVID									PCI_DEVICE_ID
#define NET2282_PCICMD										PCI_COMMAND
#define NET2282_PCISTAT										PCI_STATUS
#define NET2282_PCIDEVREV									PCI_CLASS_REVISION
#define NET2282_PCICLASS									PCI_CLASS_PROG
#define NET2282_PCICACHESIZE								PCI_CACHE_LINE_SIZE
#define NET2282_PCILATENCY									PCI_LATENCY_TIMER
#define NET2282_PCIHEADER									PCI_HEADER_TYPE
#define NET2282_PCIBIST										PCI_BIST
#define NET2282_PCIBASE0									PCI_BASE_ADDRESS_0
#define NET2282_PCIBASE1									PCI_BASE_ADDRESS_1
#define NET2282_PCIBASE2									PCI_BASE_ADDRESS_2
#define NET2282_PCIBASE3									PCI_BASE_ADDRESS_3
#define NET2282_PCIBASE4									PCI_BASE_ADDRESS_4
#define NET2282_PCIBASE5									PCI_BASE_ADDRESS_5
#define NET2282_PCICARDBUS									PCI_CARDBUS_CIS
#define NET2282_PCISUBVID									PCI_SUBSYSTEM_VENDOR_ID
#define NET2282_PCISUBID									PCI_SUBSYSTEM_ID
#define NET2282_PCIROMBASE									PCI_ROM_ADDRESS
#define NET2282_PCICAPPTR									PCI_CAPABILITY_LIST
#define NET2282_PCIINTLINE									PCI_INTERRUPT_LINE
#define NET2282_PCIINTPIN									PCI_INTERRUPT_PIN
#define NET2282_PCIMINGNT									PCI_MIN_GNT
#define NET2282_PCIMAXLAT									PCI_MAX_LAT
#define NET2282_PWRMNGID									0x40
#define NET2282_PWRMNGNEXT									0x41
#define NET2282_PWRMNGCAP									0x42
#define NET2282_PWRMNGCSR									0x44
#define NET2282_PWRMNGBRIDGE								0x46
#define NET2282_PWRMNGDATA									0x47

/// Main control registers
#define NET2282_DEVINIT										0x00
#define NET2282_EECTL										0x04
#define NET2282_EECTLFREQ									0x08
#define NET2282_PCICTL										0x0C
#define NET2282_PCIIRQENB0									0x10
#define NET2282_PCIIRQENB1									0x14
#define NET2282_CPUIRQENB0									0x18
#define NET2282_CPUIRQENB1									0x1C
#define NET2282_USBIRQENB1									0x24
#define NET2282_IRQSTAT0									0x28
#define NET2282_IRQSTAT1									0x2C
#define NET2282_IDXADDR										0x30
#define NET2282_IDXDATA										0x34
#define NET2282_FIFOCTL										0x38
#define NET2282_MEMADDR										0x40
#define NET2282_MEMDATA0									0x44
#define NET2282_MEMDATA1									0x48
#define NET2282_GPIOCTL										0x50
#define NET2282_GPIOSTAT									0x54
#define NET2282_STDRSP										0x80
#define NET2282_PRODVENDID									0x84
#define NET2282_RELNUM										0x88
#define NET2282_USBCTL										0x8C
#define NET2282_USBSTAT										0x90
#define NET2282_XCVRDIAG									0x94
#define NET2282_SETUP0123									0x98
#define NET2282_SETUP4567									0x9C
#define NET2282_OURADDR										0xA4
#define NET2282_OURCONFIG									0xA8
#define NET2282_EP_EMPTY									0xAC
#define NET2282_VIRT_EP										0xB0
#define NET2282_VIRT_EP_ENB									0xB4
#define NET2282_EP_ENABLE									0xB8
#define NET2282_EP_DISABLE									0xBC

/// PCI
#define NET2282_PCIMSTCTL									0x100
#define NET2282_PCIMSTADDR									0x104
#define NET2282_PCIMSTDATA									0x108
#define NET2282_PCIMSTSTAT									0x10C

/// DMA
#define NET2282_CHA_DMACTL									0x180
#define NET2282_CHA_DMASTAT									0x184
#define NET2282_CHA_DMACOUNT								0x190
#define NET2282_CHA_DMAADDR									0x194
#define NET2282_CHA_DMADESC									0x198

#define NET2282_CHB_DMACTL									0x1A0
#define NET2282_CHB_DMASTAT									0x1A4
#define NET2282_CHB_DMACOUNT								0x1B0
#define NET2282_CHB_DMAADDR									0x1B4
#define NET2282_CHB_DMADESC									0x1B8

#define NET2282_CHC_DMACTL									0x1C0
#define NET2282_CHC_DMASTAT									0x1C4
#define NET2282_CHC_DMACOUNT								0x1D0
#define NET2282_CHC_DMAADDR									0x1D4
#define NET2282_CHC_DMADESC									0x1D8

#define NET2282_CHD_DMACTL									0x1E0
#define NET2282_CHD_DMASTAT									0x1E4
#define NET2282_CHD_DMACOUNT								0x1F0
#define NET2282_CHD_DMAADDR									0x1F4
#define NET2282_CHD_DMADESC									0x1F8

/// Endpoints
#define NET2282_CFGOUT_CFG									0x200
#define NET2282_CFGOUT_RSP									0x204
#define NET2282_CFGIN_CFG									0x210
#define NET2282_CFGIN_RSP									0x214
#define NET2282_PCIOUT_CFG									0x220
#define NET2282_PCIOUT_RSP									0x224
#define NET2282_PCIIN_CFG									0x230
#define NET2282_PCIIN_RSP									0x234
#define NET2282_STATIN_CFG									0x240
#define NET2282_STATIN_RSP									0x244

#define NET2282_EP0_CFG										0x300
#define NET2282_EP0_RSP										0x304
#define NET2282_EP0_IRQENB									0x308
#define NET2282_EP0_STAT									0x30C
#define NET2282_EP0_AVAIL									0x310
#define NET2282_EP0_DATA									0x314
#define NET2282_EP0_DATA1									0x318

#define NET2282_EPA_CFG										0x320
#define NET2282_EPA_RSP										0x324
#define NET2282_EPA_IRQENB									0x328
#define NET2282_EPA_STAT									0x32C
#define NET2282_EPA_AVAIL									0x330
#define NET2282_EPA_DATA									0x334
#define NET2282_EPA_DATA1									0x338

#define NET2282_EPB_CFG										0x340
#define NET2282_EPB_RSP										0x344
#define NET2282_EPB_IRQENB									0x348
#define NET2282_EPB_STAT									0x34C
#define NET2282_EPB_AVAIL									0x350
#define NET2282_EPB_DATA									0x354
#define NET2282_EPB_DATA1									0x358

#define NET2282_EPC_CFG										0x360
#define NET2282_EPC_RSP										0x364
#define NET2282_EPC_IRQENB									0x368
#define NET2282_EPC_STAT									0x36C
#define NET2282_EPC_AVAIL									0x370
#define NET2282_EPC_DATA									0x374
#define NET2282_EPC_DATA1									0x378

#define NET2282_EPD_CFG										0x380
#define NET2282_EPD_RSP										0x384
#define NET2282_EPD_IRQENB									0x388
#define NET2282_EPD_STAT									0x38C
#define NET2282_EPD_AVAIL									0x390
#define NET2282_EPD_DATA									0x394
#define NET2282_EPD_DATA1									0x398

#define NET2282_EPE_CFG										0x3A0
#define NET2282_EPE_RSP										0x3A4
#define NET2282_EPE_IRQENB									0x3A8
#define NET2282_EPE_STAT									0x3AC
#define NET2282_EPE_AVAIL									0x3B0
#define NET2282_EPE_DATA									0x3B4
#define NET2282_EPE_DATA1									0x3B8

#define NET2282_EPF_CFG										0x3C0
#define NET2282_EPF_RSP										0x3C4
#define NET2282_EPF_IRQENB									0x3C8
#define NET2282_EPF_STAT									0x3CC
#define NET2282_EPF_AVAIL									0x3D0
#define NET2282_EPF_DATA									0x3D4
#define NET2282_EPF_DATA1									0x3D8


/// NET2282_DEVINIT - bits' definitions
// Chapter 11.5 in 'NET2282 PCI USB 2.0 High-Speed Peripheral Controller'

// b0
// 8051 Reset
// When clear, the 8051 is enabled to execute firmware. When set, the 8051 is held in a Reset state.
// If a valid serial EEPROM with 8051 firmware is detected, automatically cleared when the serial EEPROM read completes.
// Not cleared if no valid serial EEPROM is detected.
#define NET2282_8051_RESET									(0x1<<0)

// b1
// USB Soft Reset
// When set, the USB control section of the NET2282 is reset.
// Also, the OURADDR and OURCONFIG registers are cleared. Reading this bit always returns a 0.
#define NET2282_USB_SOFT_RESET								(0x1<<1)

// b2
// PCI Soft Reset
// When set, the PCI Configuration registers and PCI control section of the device are reset.
// If operating in PCI Host mode, the PCI RST# pin is driven for 2 to 3 ms.
// Reading this bit returns the value of the PCI RST# output.
#define NET2282_PCI_SOFT_RESET								(0x1<<2)

// b3
// Configuration Soft Reset
// When set, all NET2282 Configuration registers are reset.
// Reading this bit always returns a 0.
#define NET2282_CONF_SOFT_RESET							(0x1<<3)

// b4
// FIFO Soft Reset
// When set, all Endpoint FIFOs are flushed.
// Reading this bit always returns a 0.
#define NET2282_FIFO_SOFT_RESET							(0x1<<4)

// b5
// PCI Enable
// When clear, all PCI accesses to the NET2282 result in a Target Retry response.
// When set, the NET2282 normally responds to PCI accesses.
// Automatically set when no valid serial EEPROM is detected and PCI Adapter mode is selected.
#define NET2282_PCI_ENABLE									(0x1<<5)

// b6
// PCI ID
// When clear, the normal PCI Device and Vendor ID are returned to the PCI host when the PCIVENDID and PCIDEVID Configuration registers are accessed.
// When set, the Subsystem Device and Vendor ID values are returned instead.
#define NET2282_PCI_ID										(0x1<<6)

// b7
// Force PCI RST
// When clear, the PCI RST# pin is driven when RESET# asserts, the PCI Soft Reset pin is set, or a Power Management Soft reset occurs.
// When set, the PCI RST# pin is continuously driven.
// Valid only when the NET2282 is in PCI Host mode.
#define NET2282_FORCE_PCI_RST								(0x1<<7)

// b8:b11 Local clock
// Controls the LCLKO pin frequency.
// When set to 0, the clock is stopped.
// Non-zero values represent divisors of the 30-MHz clock.
// The default value is 8h, representing a frequency of 3.75 MHz.
#define LOCAL_CLOCK											0x00000F00
#define LOCAL_CLOCK_FREQ_SHIFT								8
#define NET2282_CLK_STOP									(0 << LOCAL_CLOCK_FREQ_SHIFT)
#define NET2282_CLK_30Mhz									(1 << LOCAL_CLOCK_FREQ_SHIFT)
#define NET2282_CLK_15Mhz									(2 << LOCAL_CLOCK_FREQ_SHIFT)
#define NET2282_CLK_7_5Mhz									(4 << LOCAL_CLOCK_FREQ_SHIFT)
#define NET2282_CLK_3_75Mhz								(8 << LOCAL_CLOCK_FREQ_SHIFT)

// b12:15 PCI Expansion ROM Range
// Reserved.

// b16:20 PCI Expansion ROM Range
// Determines the PCI Expansion ROM Base Address register range, in increments of 2 KB.
// The default corresponds to a 2 KB range and the maximum range is 64 KB.
// Bit 16 of this register corresponds to address bit 11, and bit 20 corresponds to address bit 15.
// Starting with bit 16 of the register, as each successive bit is changed to a 0, the range doubles.
// The PCI Expansion ROM Base Address register must be a multiple of the range.
#define NET2282_EXP_ROM_RANGE								0x001F0000

/// NET2282_PCIMSTCTL - bits' definitions
// Chapter 11.7 in 'NET2282 PCI USB 2.0 High-Speed Peripheral Controller'

// b3:b0
// PCI Master Byte Enables
// Determines which PCI Byte Enables (CBE[3:0]#) are asserted during a Master cycle Data phase.
// CBE0# corresponds to lines AD[7:0]
// CBE1# corresponds to lines AD[15:8]
// CBE2# corresponds to lines AD[23:16]
// CBE3# corresponds to lines AD[31:24]
#define NET2282_CBE_0										(0x1<<0)
#define NET2282_CBE_1										(0x1<<1)
#define NET2282_CBE_2										(0x1<<2)
#define NET2282_CBE_3										(0x1<<3)
#define NET2282_CBE_03										(NET2282_CBE_3 | NET2282_CBE_2 | NET2282_CBE_1 | NET2282_CBE_0)
// b5:b4
// PCI Master Read/Write
// PCI Master Start
// This bits are handling automaticly by NET2282 PCIIN/PCIOUT endpoints.
#define NET2282_MASTER_RW									(0x1<<4)
#define NET2282_MASTER_START								(0x1<<5)

// b7:b6
// PCI Master Command Select. When the NET2280 performs PCI transactions initiated by
// the 8051 or the USB interface, this field determines the PCI command codes that are issued.
// Value       Read Cmd                 Write Cmd
// 00          0110 (Mem Read)          0111 (Mem Write)
// 01          0010 (I/O Read)          0011 (I/O Write)
// 10          1010 (Cfg Read)          1011 (Cfg Write)
// 11          Reserved
#define MASTER_COMAND_SELECTOR_SHIFT						6
#define	NET2282_MEM											(0x0 << MASTER_COMAND_SELECTOR_SHIFT)
#define	NET2282_IO											(0x1 << MASTER_COMAND_SELECTOR_SHIFT)
#define	NET2282_CFG											(0x2 << MASTER_COMAND_SELECTOR_SHIFT)

// b8
// DMA Read Line Enable
// When clear, the NET2282 DMA controller issues a Memory Read command for transactions that do not start on a Cache Line boundary.
// When set, the DMA controller issues a Read Line command if a transaction is not aligned to a Cache Line boundary, and retains FIFO space for at least one cache line of data.
// The PCI burst is stopped at the cache line boundary if there is insufficient space in a FIFO for at least one cache line of data or if a Read Multiple command is started.
#define	NET2282_DMA_READ_LINE_ENB							(0x1<<8)

// b9
// DMA Read Multiple Enable
// When clear, the NET2282 DMA controller issues a Memory Read command for transactions that start on a Cache Line boundary.
// When set, the DMA controller issues a Read Multiple command if a transaction is aligned to a Cache Line boundary, and retains FIFO space for at least one cache line of data.
// The PCI burst continues if there is space in the FIFO for another line.
#define	NET2282_DMA_READ_MULTI_ENB							(0x1<<9)

// b10
// DMA Memory Write and Invalidate Enable
// When clear, the NET2282 DMA controller issues a Memory Write command for all Write transactions.
// When set, the DMA controller issues a Memory Write and Invalidate command if a transaction is aligned to a Cache Line boundary, and retains at least one cache line of data in the Endpoint FIFO.
// The PCI burst continues if there are more lines of data in the FIFO. Effective only if the PCICMD register Memory Write and Invalidate bit is set.
#define	NET2282_DMA_WRITE_ENB								(0x1<<10)

// b11
// PCI Retry Abort Enable
// When clear, the NET2282 indefinitely attempts to access a target.
// When set, the NET2282 aborts the transaction after 256 consecutive Retries, and sets an interrupt.
// For a DMA transaction, the DMA operation is aborted.
#define	NET2282_RETRY_ABORT_ENB								(0x1<<11)

// b12
// PCI Multi-Level Arbiter
// When clear, all PCI requesters are placed into a single-level Round-Robin Arbiter, each with equal access to the PCI Bus.
// When set, a three-level Arbiter is selected.
#define	NET2282_MULTI_LEVEL_ARBITER							(0x1<<12)

// b15:b13
// Internal PCI Arbiter Park Select
// Determines which PCI Master controller is granted the bus when there are no pending requests.
// Values:
// 000b = Last Grantee
// 001b = 8051/USB (default)
// 010b = External Requester
// 011b = Reserved
// 100b = DMA Channel A
// 101b = DMA Channel B
// 110b = DMA Channel C
// 111b = DMA Channel D
#define PARK_SEL_SHIFT										13
#define	NET2282_PARK_SEL_LAST								(0x0 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_USB								(0x1 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_EXT								(0x2 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_DMA_A								(0x4 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_DMA_B								(0x5 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_DMA_C								(0x6 << PARK_SEL_SHIFT)
#define	NET2282_PARK_SEL_DMA_D								(0x7 << PARK_SEL_SHIFT)

// b16
// PCI Float Enable
// When clear, RST#, REQ#, GNT#, and INTA# can be driven during the Low-Power Suspend state.
// When set,   RST#, REQ#, GNT#, and INTA# are three-stated during the Low-Power Suspend state.
#define	NET2282_FLOAT_ENB									(0x1<<16)

/// NET2282_USBIRQENB1 - bits' definitions
// Enable these interrupts in NET2280 USBIRQENB1 register:
// USB Interrupt Enable (bit 31) must be set to get *any* interrupt (plus at least one other bit)
#define	NET2282_SOF_INTERRUPT_ENABLE						(0x1<<0)
#define	NET2282_RESUME_INTERRUPT_ENABLE						(0x1<<1)
#define	NET2282_SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE		(0x1<<2)
#define	NET2282_SUSPEND_REQUEST_INTERRUPT_ENABLE			(0x1<<3)
#define	NET2282_ROOT_PORT_RESET_INTERRUPT_ENABLE			(0x1<<4)
#define	NET2282_CONTROL_STATUS_INTERRUPT_ENABLE				(0x1<<6)
#define	NET2282_VBUS_INTERRUPT_ENABLE						(0x1<<7)
#define	NET2282_EEPROM_DONE_INTERRUPT_ENABLE				(0x1<<8)
#define	NET2282_DMA_A_INTERRUPT_ENABLE						(0x1<<9)
#define	NET2282_DMA_B_INTERRUPT_ENABLE						(0x1<<10)
#define	NET2282_DMA_C_INTERRUPT_ENABLE						(0x1<<11)
#define	NET2282_DMA_D_INTERRUPT_ENABLE						(0x1<<12)
#define	NET2282_GPIO_INTERRUPT_ENABLE						(0x1<<13)
#define	NET2282_SOF_DOENCOUNT_INTERRUPT_ENABLE				(0x1<<14)
#define	NET2282_VIRTUALIZED_ENDPOINT_INTERRUPT_ENABLE		(0x1<<15)
#define	NET2282_PCI_MASTER_CYCLE_DONE_INTERRUPT_ENABLE		(0x1<<16)
#define	NET2282_PCI_RETRY_ABORT_INTERRUPT_ENABLE			(0x1<<17)
#define	NET2282_PCI_TARGET_ABORT_RECEIVED_INTERRUPT_ENABLE	(0x1<<19)
#define	NET2282_PCI_MASTER_ABORT_RECEIVED_INTERRUPT_ENABLE	(0x1<<20)
#define	NET2282_PCI_PERR_INTERRUPT_ENABLE					(0x1<<21)
#define	NET2282_PCI_SERR_INTERRUPT_ENABLE					(0x1<<22)
#define	NET2282_PCI_PME_INTERRUPT_ENABLE					(0x1<<23)
#define	NET2282_PCI_INTA_INTERRUPT_ENABLE					(0x1<<24)
#define	NET2282_PCI_PARITY_ERROR_INTERRUPT_ENABLE			(0x1<<25)
#define	NET2282_PCI_ARBITER_TIMEOUT_INTERRUPT_ENABLE		(0x1<<26)
#define	NET2282_POWER_STATE_CHANGE_INTERRUPT_ENABLE			(0x1<<27)
#define	NET2282_USB_INTERRUPT_ENABLE						(0x1<<31)

/// NET2282_PCICMD - default settings. When set PCI bus is enabled and operative. Nothing more to say :-).
# define	NET2282_PCICMD_SLAVE							0xF1000013
# define	NET2282_PCICMD_MASTER							0xF3800006


/// NET2282_EP_RSP - bits' definitions
#define NET2282_EP_RSP_CLEAR_ENDPOINT_HALT					(0x1<<0)
#define NET2282_EP_RSP_CLEAR_ENDPOINT_TOGGLE				(0x1<<1)
#define NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS_MODE 			(0x1<<2)
#define NET2282_EP_RSP_CLEAR_CONTROL_STATUS_PHASE_HANDSHAKE	(0x1<<3)
#define NET2282_EP_RSP_CLEAR_INTERRUPT_MODE					(0x1<<4)
#define NET2282_EP_RSP_CLEAR_EP_FORCE_CRC_ERROR				(0x1<<5)
#define NET2282_EP_RSP_CLEAR_EP_HIDE_STATUS_PHASE			(0x1<<6)
#define NET2282_EP_RSP_CLEAR_NAK_OUT_PACKETS				(0x1<<7)
#define NET2282_EP_RSP_SET_ENDPOINT_HALT					(0x1<<8)
#define NET2282_EP_RSP_SET_ENDPOINT_TOGGLE					(0x1<<9)
#define NET2282_EP_RSP_SET_NAK_OUT_PACKETS_MODE				(0x1<<10)
#define NET2282_EP_RSP_SET_CONTROL_STATUS_PHASE_HANDSHAKE	(0x1<<11)
#define NET2282_EP_RSP_SET_INTERRUPT_MODE					(0x1<<12)
#define NET2282_EP_RSP_SET_EP_FORCE_CRC_ERROR				(0x1<<13)
#define NET2282_EP_RSP_SET_EP_HIDE_STATUS_PHASE				(0x1<<14)
#define NET2282_EP_RSP_SET_NAK_OUT_PACKETS					(0x1<<15)

/// NET2282 DMA_CTL - bits' definitions
// b0
// DMA Address Hold
// When clear, the DMA address increments after each DMA Data transfer. When set, the DMA address does not increment during the DMA Data transfer.
#define	NET2282_DMA_CTL_ADDR_HOLD							(0x1<<0)

// b1
// DMA Enable
// When set, the DMA controller is enabled to transfer data and process Scatter/Gather descriptors.
// When cleared, the DMA controller is disabled.
// If a DMA Data transfer is in progress when this bit is cleared, the Data transfer and descriptor processing are paused.
// Automatically cleared when the DMA Abort bit is set or a PCI Master or Target Abort occurs during a DMA transaction.
// When low, the value read back is high until the DMA channel completes the Pause sequence.
#define	NET2282_DMA_CTL_DMA_ENABLE							(0x1<<1)

// b2
// DMA FIFO Validate
// When clear, the last Short packet is not automatically validated at the end of a DMA Data transfer.
// Auto-validation can be enabled during Scatter/Gather DMA Data transfers by way of the DMACOUNT register DMA Scatter/Gather FIFO Validate bit.
// When set, the last Short packet is automatically validated at the end of a DMA Data transfer.
// Valid for single or Scatter/Gather DMA Data transfers.
#define	NET2282_DMA_CTL_FIFO_VALIDATE						(0x1<<2)

// b2
// DMA Preempt Enable
// When set, DMA controller can be preempted if the 8051 or USB requires PCI Bus access.
#define	NET2282_DMA_CTL_PREEMPT_ENABLE						(0x1<<3)

// b2
// DMA OUT Auto Start Enable
// When set, DMA controller automatically starts when an OUT packet is received.
#define	NET2282_DMA_CTL_AUTO_MODE							(0x1<<4)

// b5:b15
// Reserved

// b16
// DMA Scatter/Gather Enable
// When clear, a single DMA operation is performed when the DMA Start bit is set.
// The DMACOUNT and DMAADDR registers define the transfer.
// When set, the DMA controller traverses a linked list of descriptors, each of which defines a DMA operation.
// The list of descriptors is located in PCI system memory.
#define	NET2282_DMA_CTL_SCATTER_GATHER_MODE						(0x1<<16)

// b17
// DMA Valid Bit Enable
// When clear, DMA descriptors are processed without regard to the Valid bit (DMACOUNT, bit 31).
// When set, DMA descriptors are processed only when the Valid bit is set.
// If a valid descriptor with a 0-Byte Count is encountered for an OUT endpoint,
// the DMA Scatter/Gather controller proceeds to the next descriptor.
// If a Valid descriptor with a 0-Byte Count is encountered for an IN endpoint,
// and the DMA FIFO Validate bit is not set, the DMA Scatter/Gather controller proceeds to the next descriptor.
// If a valid descriptor with a 0-Byte Count is encountered for an IN endpoint,
// and the DMA FIFO Validate bit is set,
// then a Zero-Length packet is written to the Endpoint FIFO before the DMA Scatter/Gather controller proceeds to the next descriptor.
#define	NET2282_DMA_CTL_DMA_VALIDATE						(0x1<<17)

// b18
// DMA Valid Bit Polling Enable
// When clear, the DMA Scatter/Gather controller stops polling for valid descriptors if the controller encounters a Valid bit that is not set.
// When set, the DMA Scatter/Gather controller continues polling the same descriptor if the controller encounters a Valid bit that is not set.
// Valid only if the DMA Valid Bit Enable and DMA Scatter/Gather Enable bits are set.
#define	NET2282_DMA_CTL_POLLING_MODE						(0x1<<18)

// b19:b20
// Descriptor Polling Rate
// Selects the polling rate when a cleared valid bit is detected.
// Values:
// 00b = Continuous
// 01b = 1 µs (default)
// 10b = 100 µs
// 11b = 1 ms
#define	NET2282_DMA_CTL_POLLING_CONT						(0x00<<19)
#define	NET2282_DMA_CTL_POLLING_1us							(0x01<<19)
#define	NET2282_DMA_CTL_POLLING_100us						(0x10<<19)
#define	NET2282_DMA_CTL_POLLING_1ms							(0x11<<19)

// b21
// DMA Clear Count Enable
// When clear, the DMA descriptor Byte Count field is not changed at the end of a transfer.
// When set, the Byte Count field and Valid bit of a DMA descriptor are set to 0 at the end of a transfer.
#define	NET2282_DMA_CTL_COUNT_CLEAR							(0x1<<21)

// b22:b24
// Reserved

// b25
// DMA Scatter/Gather Done Interrupt Enable
// When set, an interrupt is generated when the last descriptor in the Scatter/Gather Linked list completes its transfer.
// Valid only if NET2282_DMA_COUNT_END_OF_CHAIN is set.
#define	NET2282_DMA_CTL_SCATTER_GATHER_IRQ_ENABLE			(0x1<<25)

// b26
// DMA Pause Done Interrupt Enable
// When set, an interrupt is generated when the DMA Channel Pause sequence completes.
#define	NET2282_DMA_CTL_PAUSE_IRQ_ENABLE					(0x1<<26)

// b27
// DMA Abort Done Interrupt Enable
// When set, an interrupt is generated when the DMA Channel Abort sequence completes.
#define	NET2282_DMA_CTL_ABORT_IRQ_ENABLE					(0x1<<27)

// b28:31
// Reserved

/// NET2282 DMA_COUNT - bits' definitions
// b0:b23
// DMA Byte Count
// Determines the number of bytes to transfer.
// The maximum DMA Data transfer size is 16 MB.
// This register decrements as data transfers.
#define	NET2282_DMA_COUNT_SIZE_MASK							0x00FFFFFF

// b24
// DMA OUT Continue
// When clear and a Short OUT packet is received, the EP_STAT register NAK Packets bit is set
// if the EP_RSP register NAK OUT Packets Mode bit is set.
// Also, the DMA Scatter/Gather controller stops if the DMA Byte Count did not reach 0.
// When clear and a Short OUT packet is received,
// the NAK Packets bit is set and then automatically cleared when the DMA controller finishes transferring the Short packet to the PCI Bus.
// Also, the DMA Scatter/Gather controller is then enabled to read the next descriptor.
#define	NET2282_DMA_COUNT_CONTINUE							(0x1<<24)

// b25:b26
// Reserved (read ONLY!)
// These bits are loaded from the DMACOUNT word in a DMA descriptor to the EP_n_HS_MAXPKT registers Additional Transaction Opportunities field.

// b27
// DMA Scatter/Gather FIFO Validate
// When clear, the Last Short packet is not automatically validated at the end of a DMA Data transfer.
// When set, the Last Short packet is automatically validated at the end of a DMA Data transfer.
// The DMACTL register DMA FIFO Validate bit takes precedence over this bit.
// If the DMA FIFO Validate bit is set, then all USB Short packets are automatically validated at the end of each Scatter/Gather DMA Data transfer.
#define	NET2282_DMA_COUNT_SCATTER_GATHER_FIFO_VALIDATE		(0x1<<27)

// b28
// End of Chain
// When clear, there are additional DMA descriptors following the current descriptor being processed.
// When set, this descriptor is the last descriptor in the Scatter/Gather chain.
#define	NET2282_DMA_COUNT_END_OF_CHAIN						(0x1<<28)

// b29
// DMA Done Interrupt Enable
// When set, an interrupt is generated when the DMA Byte Count for this descriptor reaches 0.
#define	NET2282_DMA_COUNT_DONE_IRQ_ENABLE					(0x1<<29)

// b30
// DMA Direction
// When clear, the DMA channel transfers data from the USB host to PCI Bus for OUT endpoints.
// When set, the DMA channel transfers data from the PCI Bus to the USB host for IN endpoints.
#define	NET2282_DMA_COUNT_TRANSFER_DIRECTION				(0x1<<30)

// b31
// Valid Bit
// Indicates the validity of the current DMA descriptor.
#define	NET2282_DMA_COUNT_VALID_BIT							(0x1<<31)

/// NET2282 DMA_STAT - bits' definitions
// b0
// DMA Start
// Writing a 1 causes the DMA controller to start. This bit is self-clearing, and always returns a 0.
#define	NET2282_DMA_STAT_TRANSFER_START						(0x1<<0)

// b1
// DMA Abort
// Writing a 1 causes the DMA Data transfer to abort.
// When read, this bit remains high until the DMA Abort sequence completes.
// Note: If a DMA Abort occurs while an unaligned DMA Data transfer is paused, the abort is delayed until the DMA Data transfer is re-enabled.
#define	NET2282_DMA_STAT_ABORT								(0x1<<1)

// b2:b23
// Reserved

// b24
// DMA Transaction Done Interrupt
// Set when the DMA controller completes its current Data transfer and the DMACOUNT register DMA Done Interrupt Enable bit is set.
// Writing a 1 clears this status bit.
#define	NET2282_DMA_STAT_DONE_IRQ_CLS						(0x1<<24)

// b25
// DMA Scatter/Gather Done Interrupt
// Set when the last descriptor in the Scatter/Gather Linked list completes its transfer.
// Writing a 1 clears this status bit.
#define	NET2282_DMA_STAT_SCATTER_GATHER_IRQ_CLS				(0x1<<25)

// b26
// DMA Pause Done Interrupt
// Set when the DMA Channel Pause sequence completes.
// The Pause sequence is initiated by setting the DMA Enable bit low.
// Writing a 1 clears this status bit.
#define	NET2282_DMA_STAT_PAUSE_IRQ_CLS						(0x1<<26)

// b27
// DMA Abort Done Interrupt
// Set when the DMA Channel Abort sequence completes.
// The Abort sequence is initiated by setting the DMA Abort bit high.
// Writing a 1 clears this status bit.
#define	NET2282_DMA_STAT_ABORT_IRQ_CLS						(0x1<<27)

// b28:b31
// Reserved

/// NET2282 EECTL - bits' definitions
// b0:b7
// Serial EEPROM Write Data
// Determines the byte written to the serial EEPROM when the Serial EEPROM Byte Write Start bit is set.
// This field can represent an opcode, address, or data being written to the serial EEPROM.
#define	NET2282_EEPROM_WRITE_DATA_SHIFT						0
#define	NET2282_EEPROM_WRITE_DATA_MASK						0xFF

// b8:b15
// Serial EEPROM Read Data
// Determines the byte read from the serial EEPROM when the Serial EEPROM Byte Read Start bit is set.
#define	NET2282_EEPROM_READ_DATA_SHIFT						8
#define	NET2282_EEPROM_READ_DATA_MASK						(0xFF<<8)

// b16
// Serial EEPROM Byte Write Start
// When set, the value in the Serial EEPROM Write Data field is written to the serial EEPROM.
// This bit is automatically cleared when the Write operation completes.
#define	NET2282_EEPROM_WRITE_DATA_START						(0x1<<16)

// b17
// Serial EEPROM Byte Read Start
// When set, a byte is read from the serial EEPROM, and is accessed using the Serial EEPROM Read Data field.
// Automatically cleared when the Read operation completes.
#define NET2282_EEPROM_READ_DATA_START						(0x1<<17)

// b18
// Serial EEPROM Chip Select Enable
// When set, the Serial EEPROM Chip Select is enabled.
#define NET2282_EEPROM_CS_ENABLE							(0x1<<18)

// b19
// Serial EEPROM Busy
// When set, the Serial EEPROM controller is busy performing a Byte Read or Write operation.
// An interrupt is generated when this bit goes false.
#define NET2282_EEPROM_BUSY									(0x1<<19)

// b20
// Serial EEPROM Valid
// Non-blank serial EEPROM with 5Ah in the first byte is detected.
#define NET2282_EEPROM_VALID								(0x1<<20)

// b21
// Serial EEPROM Present
// Set when the Serial EEPROM controller determines that a serial EEPROM is connected to the NET2282.
#define NET2282_EEPROM_PRESENT								(0x1<<21)

// b22
// Serial EEPROM Chip Select Active
// Set when the EECS# Chip Select pin is active.
// The Chip Select is active across multiple byte operations.
#define NET2282_EEPROM_CS_ACTIVE							(0x1<<22)

// b23:b24
// Serial EEPROM Address Width
// Reports the addressing width of the installed serial EEPROM.
// If the addressing width cannot be determined, 00b is returned.
// A non-zero value is reported only if the validation signature (5Ah) is successfully read from the first serial EEPROM location.
// Values:
// 00b = Undetermined
// 01b = 1 byte
// 10b = 2 bytes
// 11b = 3 bytes
#define NET2282_EEPROM_ADDR_WIDTH_SHIFT						23
#define NET2282_EEPROM_ADDR_WIDTH_MASK						(0x3<<23)

// b25:b31
// Reserved

#define NET2282_EEPROM_OPCODE_WRITE_EN						6
#define NET2282_EEPROM_OPCODE_STATUS						5
#define NET2282_EEPROM_OPCODE_WRITE							2
#define NET2282_EEPROM_OPCODE_READ							3

/// NET2282 USBCTL - bits' definitions
#define NET2282_TIMED_DISCONNECT							(0x1<<9)

/// NET2282 FIFOCTL - bits' definitions
// b0:b1
// FIFO Configuration Select
// Selects the FIFO configuration for Endpoints A, B, C, and D. Values:
// 00b = Endpoints A, B, C, D each consist of a 1-KB FIFO
// 01b = Endpoints A and B each consist of a 2-KB FIFO. Endpoints C and D are unavailable.
// 10b = Endpoint A consist of a 2-KB FIFO, and Endpoints B and C each consist of a 1-KB FIFO. Endpoint D is unavailable.
// 11b = Not Used
#define NET2282_FIFO_CONF_MASK								0x3
#define NET2282_FIFO_CONF_4x1KB								0x00
#define NET2282_FIFO_CONF_2x2KB								0x01
#define NET2282_FIFO_CONF_2KB_2x1KB							0x10

// b2
// PCI BASE2 Select
// When clear, Endpoint FIFOs A, B, C, and D are each assigned one quarter of the PCI BASE2 space (A = 1st, B = 2nd, C = 3rd, D = 4th).
// When set, PCI writes to the lower half of the PCI BASE2 space are directed to the Endpoint A FIFO, reads from the lower half are directed to the Endpoint B FIFO,
// writes to the upper half are directed to the Endpoint C FIFO, and reads from the upper half are directed to the Endpoint D FIFO.
#define NET2282_FIFO_BASE2_BIDIRECTIONAL					(0x1<<2)

// b3
// Ignore FIFO Availability
// When clear, PCI accesses to empty and/or full FIFOs result in a Retry termination.
// When set, PCI accesses to empty and/or full FIFOs result in a normal termination; however, Read data is undefined and Write data is ignored.
#define NET2282_FIFO_NOT_VALIDATE							(0x1<<3)

// b4:b15
// Reserved

// b16:b31
// PCI BASE2 Range
// Determines the PCI Base Address 2 register range, in increments of 64 KB.
// The default corresponds to a 64-KB range.
// Starting with bit 16, as each successive bit is changed to a 0, the range doubles.
// Value of 0 corresponds to a 4 GB range, thereby causing the PCI Base Address 2 register to be disabled.
// The PCI Base Address 2 register must be a multiple of the range.
#define NET2282_FIFO_BASE2_RANGE_MASK						(0xFFFF<<16)
#define NET2282_FIFO_BASE2_RANGE_64KB						(0xFFFF<<16)
#define NET2282_FIFO_BASE2_RANGE_128KB						(0xFFFE<<16)

/// Indexed registers
#define	NET2282_DIAG										0x00
# define	NET2282_FORCE_CPU_INERRUPT						(0x1<<8)
# define	NET2282_FORCE_USB_INERRUPT						(0x1<<9)
# define	NET2282_FORCE_PCI_INERRUPT						(0x1<<10)
# define	NET2282_FORCE_PCISERR_INERRUPT					(0x1<<11)

// High-Speed Interrupt Polling Rate
// Specifies the interrupt polling rate in terms of microframes (125 µs).
// Returned as the last byte of all Interrupt Endpoint descriptors when the Get Configuration descriptor is set to Auto-Enumerate mode, and the NET2282 is operating in High-Speed mode.
#define	NET2282_USB_HS_INTPOLL_RATE							0x08

// Full-Speed Interrupt Polling Rate
// Specifies the interrupt polling rate, in milliseconds.
// Returned as the last byte of all Interrupt Endpoint descriptors when the Get Configuration descriptor is set to Auto-Enumerate mode, and the NET2282 is operating in Full-Speed mode.
#define	NET2282_USB_FS_INTPOLL_RATE							0x09

// STATIN High-Speed Interrupt Polling Rate
// Specifies the interrupt polling rate in terms of microframes (125 µs).
// Returned as the last byte of the STATIN Interrupt Endpoint descriptor when the Get Configuration descriptor is set to Auto-Enumerate mode, and the NET2282 is operating in High-speed mode.
#define	 NET2282_STATIN_HS_INTPOLL_RATE 					0x84

// STATIN Full-Speed Interrupt Polling Rate
// Specifies the interrupt polling rate in milliseconds.
// Returned as the last byte of the STATIN Interrupt Endpoint descriptor when the Get Configuration descriptor is set to Auto-Enumerate mode, and the NET2282 is operating in Full-Speed mode.
#define	 NET2282_STATIN_FS_INTPOLL_RATE 					0x85

#define	 NET2282_EPA_HS_MAXPTK 								0x20
#define	 NET2282_EPB_HS_MAXPTK 								0x30
#define	 NET2282_EPC_HS_MAXPTK 								0x40
#define	 NET2282_EPD_HS_MAXPTK 								0x50
#define	 NET2282_EPE_HS_MAXPTK 								0x60
#define	 NET2282_EPF_HS_MAXPTK 								0x70

#define	 NET2282_EPA_FS_MAXPTK 								0x21
#define	 NET2282_EPB_FS_MAXPTK 								0x31
#define	 NET2282_EPC_FS_MAXPTK 								0x41
#define	 NET2282_EPD_FS_MAXPTK 								0x51
#define	 NET2282_EPE_FS_MAXPTK 								0x61
#define	 NET2282_EPF_FS_MAXPTK 								0x71

#define	 NET2282_SCRATCH_PAD								0x0B

#endif	//_NET2282_DEFINES_H_
