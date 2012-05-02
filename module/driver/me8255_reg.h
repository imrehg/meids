/**
 * @file me8255_reg.h
 *
 * @brief 8255 counter register definitions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

#ifdef __KERNEL__

# ifndef _ME8255_REG_H_
#  define _ME8255_REG_H_

/// Registers for 8255 A.
#  define ME1400_PORT_A_0			0x0000
#  define ME1400_PORT_A_1			0x0001
#  define ME1400_PORT_A_2			0x0002
#  define ME1400_PORT_A_CTRL		0x0003

/// Registers for 8255 B (ver A and B).
#  define ME1400AB_PORT_B_0			0x0008
#  define ME1400AB_PORT_B_1			0x0009
#  define ME1400AB_PORT_B_2			0x000A
#  define ME1400AB_PORT_B_CTRL		0x000B

/// Register sfor 8255 B (ver C and D).
#  define ME1400CD_PORT_B_0			0x0040
#  define ME1400CD_PORT_B_1			0x0041
#  define ME1400CD_PORT_B_2			0x0042
#  define ME1400CD_PORT_B_CTRL		0x0043

/// Mode: P2,P1,P0 (I-input, O-output)
/// Egz. ME8255_MODE_OIO => Port 2 = Output, Port 1 = Input,  Port 0 = Output
#  define ME8255_MODE_OOO			0x80
#  define ME8255_MODE_IOO			0x89
#  define ME8255_MODE_OIO			0x82
#  define ME8255_MODE_IIO			0x8B
#  define ME8255_MODE_OOI			0x90
#  define ME8255_MODE_IOI			0x99
#  define ME8255_MODE_OII			0x92
#  define ME8255_MODE_III			0x9B

/// If set in mirror then port X is in output mode.
#  define ME8255_PORT_0_OUTPUT		0x1
#  define ME8255_PORT_1_OUTPUT		0x2
#  define ME8255_PORT_2_OUTPUT		0x4


# endif
#endif
