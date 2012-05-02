/**
 * @file medefines.h
 *
 * @brief Global definitions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _OSI_MEDEFINES_H_
# define _OSI_MEDEFINES_H_

/*==================================================================
  General
  ================================================================*/

#define ME_VALUE_NOT_USED							0x0
#define ME_VALUE_DEFAULT							0x0
#define ME_NO_FLAGS									0x0
#define ME_VALUE_INVALID							~0x0

/*==================================================================
  Defines common to access functions
  ================================================================*/

#define ME_LOCK_RELEASE								0x00010001
#define ME_LOCK_SET									0x00010002
#define ME_LOCK_CHECK								0x00010003

/*==================================================================
  Defines meOpen function
  ================================================================*/

#define ME_OPEN_NO_FLAGS							0x00000000

/*==================================================================
  Defines meClose function
  ================================================================*/

#define ME_CLOSE_NO_FLAGS							0x00000000
#define ME_CLOSE_FORCE								0x10000000

/*==================================================================
  Defines meLockDriver function
  ================================================================*/

#define ME_LOCK_DRIVER_NO_FLAGS						0x0

/*==================================================================
  Defines meLockDevice function
  ================================================================*/

#define ME_LOCK_DEVICE_NO_FLAGS						0x0

/*==================================================================
  Defines meLockSubdevice function
  ================================================================*/

#define ME_LOCK_SUBDEVICE_NO_FLAGS					0x0


/*==================================================================
  Defines common to error functions
  ================================================================*/

#define ME_ERROR_MSG_MAX_COUNT						256

#define ME_SWITCH_DISABLE							0x00020001
#define ME_SWITCH_ENABLE							0x00020002

/*==================================================================
  Defines common to IO functions
  ================================================================*/

#define ME_REF_NONE									0x00000000
#define ME_REF_DIO_FIFO_LOW							0x00030001
#define ME_REF_DIO_FIFO_HIGH						0x00030002

#define ME_REF_CTR_PREVIOUS							0x00040001
#define ME_REF_CTR_INTERNAL_1MHZ					0x00040002
#define ME_REF_CTR_INTERNAL_10MHZ					0x00040003
#define ME_REF_CTR_EXTERNAL							0x00040004

#define ME_REF_AI_GROUND							0x00050001
#define ME_REF_AI_DIFFERENTIAL						0x00050002

#define ME_REF_AO_GROUND							0x00060001
#define ME_REF_AO_DIFFERENTIAL						0x00060002

#define ME_TRIG_CHAN_NONE							0x00000000
#define ME_TRIG_CHAN_DEFAULT						0x00070001
#define ME_TRIG_CHAN_SYNCHRONOUS					0x00070002

#define ME_TRIG_TYPE_NONE							0x00000000
#define ME_TRIG_TYPE_SW								0x00080001
#define ME_TRIG_TYPE_THRESHOLD						0x00080002
#define ME_TRIG_TYPE_WINDOW							0x00080003
#define ME_TRIG_TYPE_EDGE							0x00080004
#define ME_TRIG_TYPE_SLOPE							0x00080005
#define ME_TRIG_TYPE_EXT_DIGITAL					0x00080006
#define ME_TRIG_TYPE_EXT_ANALOG						0x00080007
#define ME_TRIG_TYPE_PATTERN						0x00080008
#define ME_TRIG_TYPE_TIMER							0x00080009
#define ME_TRIG_TYPE_COUNT							0x0008000A
#define ME_TRIG_TYPE_FOLLOW							0x0008000B

#define ME_TRIG_EDGE_NONE							0x00000000
#define ME_TRIG_EDGE_ABOVE							0x00090001
#define ME_TRIG_EDGE_BELOW							0x00090002
#define ME_TRIG_EDGE_ENTRY							0x00090003
#define ME_TRIG_EDGE_EXIT							0x00090004
#define ME_TRIG_EDGE_RISING							0x00090005
#define ME_TRIG_EDGE_FALLING						0x00090006
#define ME_TRIG_EDGE_ANY							0x00090007

#define ME_TIMER_ACQ_START							0x000A0001
#define ME_TIMER_SCAN_START							0x000A0002
#define ME_TIMER_CONV_START							0x000A0003
#define ME_TIMER_FIO_TOTAL							0x000A0004
#define ME_TIMER_FIO_FIRST_PHASE					0x000A0005


/*==================================================================
  Defines for meIOIrqStart function
  ================================================================*/

#define ME_IRQ_SOURCE_DIO_DEFAULT					0x00000000
#define ME_IRQ_SOURCE_DIO_PATTERN					0x000B0001
#define ME_IRQ_SOURCE_DIO_MASK						0x000B0002
#define ME_IRQ_SOURCE_DIO_LINE						0x000B0003
#define ME_IRQ_SOURCE_DIO_OVER_TEMP					0x000B0004
#define ME_IRQ_SOURCE_HIGH_CURRENT					0x000B0005

#define ME_IRQ_EDGE_NOT_USED						0x00000000
#define ME_IRQ_EDGE_RISING							0x000C0001
#define ME_IRQ_EDGE_FALLING							0x000C0002
#define ME_IRQ_EDGE_ANY								0x000C0003

/*==================================================================
  Defines for meIOIrqStart function
  ================================================================*/

#define ME_IO_IRQ_START_NO_FLAGS					0x000000
#define ME_IO_IRQ_START_DIO_BIT						0x000001
#define ME_IO_IRQ_START_DIO_BYTE					0x000002
#define ME_IO_IRQ_START_DIO_WORD					0x000004
#define ME_IO_IRQ_START_DIO_DWORD					0x000008
#define ME_IO_IRQ_START_PATTERN_FILTERING			0x000010
#define ME_IO_IRQ_START_EXTENDED_STATUS				0x000020
/*==================================================================
  Defines for meIOIrqWait function
  ================================================================*/

#define ME_IO_IRQ_WAIT_NO_FLAGS						0x000000
#define ME_IO_IRQ_WAIT_NORMAL_STATUS				0x000001
#define ME_IO_IRQ_WAIT_EXTENDED_STATUS				0x000002

/*==================================================================
  Defines for meIOIrqStop function
  ================================================================*/

#define ME_IO_IRQ_STOP_NO_FLAGS						0x000000

/*==================================================================
  Defines for meIOIrqSetCallback function
  ================================================================*/

#define ME_IO_IRQ_SET_CALLBACK_NO_FLAGS				0x0

/*==================================================================
  Defines for meIOResetDevice function
  ================================================================*/

#define ME_IO_RESET_DEVICE_NO_FLAGS					0x00000000
#define ME_IO_RESET_DEVICE_UNPROTECTED				0x00010000

/*==================================================================
  Defines for meIOResetSubdevice function
  ================================================================*/

#define ME_IO_RESET_SUBDEVICE_NO_FLAGS				0x0

/*==================================================================
  Defines for meIOSingleConfig function
  ================================================================*/

//Definitions for digital subdevice.
#define ME_SINGLE_CONFIG_DIO_INPUT					0x000D0001
#define ME_SINGLE_CONFIG_DIO_OUTPUT					0x000D0002
#define ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE			0x000D0003
#define ME_SINGLE_CONFIG_DIO_SINK					0x000D0004
#define ME_SINGLE_CONFIG_DIO_SOURCE					0x000D0005
#define ME_SINGLE_CONFIG_DIO_MUX32M					0x000D0006
#define ME_SINGLE_CONFIG_DIO_DEMUX32				0x000D0007
#define ME_SINGLE_CONFIG_DIO_BIT_PATTERN			0x000D0008

//Definitions for multipourpose pin on me1400
#define ME_SINGLE_CONFIG_MULTIPIN_IRQ				0x000D0009
#define ME_SINGLE_CONFIG_MULTIPIN_CLK				0x000D000a

//Definitions for frequency subdevice.
#define ME_SINGLE_CONFIG_FIO_INPUT_LINUX			0x000D0011
#define ME_SINGLE_CONFIG_FIO_OUTPUT_LINUX			0x000D0012
#define ME_SINGLE_CONFIG_FIO_INPUT					0x001E0001
#define ME_SINGLE_CONFIG_FIO_OUTPUT					0x001E0002


#define ME_SINGLE_CONFIG_CTR_8254_MODE_DISABLE		0x000E0000
#define ME_SINGLE_CONFIG_CTR_8254_MODE_0			0x000E0001
#define ME_SINGLE_CONFIG_CTR_8254_MODE_INTERRUPT_ON_TERMINAL_COUNT	ME_SINGLE_CONFIG_CTR_8254_MODE_0
#define ME_SINGLE_CONFIG_CTR_8254_MODE_1			0x000E0002
#define ME_SINGLE_CONFIG_CTR_8254_MODE_ONE_SHOT						ME_SINGLE_CONFIG_CTR_8254_MODE_1
#define ME_SINGLE_CONFIG_CTR_8254_MODE_2			0x000E0003
#define ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR				ME_SINGLE_CONFIG_CTR_8254_MODE_2
#define ME_SINGLE_CONFIG_CTR_8254_MODE_3			0x000E0004
#define ME_SINGLE_CONFIG_CTR_8254_MODE_SQUARE_WAVE					ME_SINGLE_CONFIG_CTR_8254_MODE_3
#define ME_SINGLE_CONFIG_CTR_8254_MODE_4			0x000E0005
#define ME_SINGLE_CONFIG_CTR_8254_MODE_SOFTWARE_TRIGGER				ME_SINGLE_CONFIG_CTR_8254_MODE_4
#define ME_SINGLE_CONFIG_CTR_8254_MODE_5			0x000E0006
#define ME_SINGLE_CONFIG_CTR_8254_MODE_HARDWARE_TRIGGER				ME_SINGLE_CONFIG_CTR_8254_MODE_5

#define ME_IO_SINGLE_CONFIG_NO_FLAGS				0x00
#define ME_IO_SINGLE_CONFIG_DIO_BIT					0x01
#define ME_IO_SINGLE_CONFIG_DIO_BYTE				0x02
#define ME_IO_SINGLE_CONFIG_DIO_WORD				0x04
#define ME_IO_SINGLE_CONFIG_DIO_DWORD				0x08
#define ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON			0x10
#define ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF		0x20
#define ME_IO_SINGLE_CONFIG_AI_RMS					0x40
// Flag for easy single config  for AI on me4600
#define ME_IO_SINGLE_CONFIG_CONTINUE				0x80
// Flag for multipurpose pin on me1400
#define ME_IO_SINGLE_CONFIG_MULTIPIN				0x100
/*
//REMOVED!
// Flag for extra precise range im me0700
#define ME_IO_SINGLE_CONFIG_EXTRA_SHUNT				0x200
*/
// Flag for FI single mode
#define ME_IO_SINGLE_CONFIG_FREQ_SINGLE_MODE		0x400	/* FI */
// Flag for not setting background measure configuration
#define ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND		0x800	/* AI CosateQ */
#define ME_IO_SINGLE_CONFIG_POLLING_MODE			0x1000	/* AI CosateQ */

/*==================================================================
  Defines for meIOSingle function
  ================================================================*/

#define ME_IO_SINGLE_NO_FLAGS						0x0
#define ME_IO_SINGLE_NONBLOCKING					0x20

#define ME_DIR_INPUT								0x000F0001
#define ME_DIR_OUTPUT								0x000F0002
#define ME_DIR_SET_OFFSET							0x000F0003

#define ME_IO_SINGLE_TYPE_NO_FLAGS					0x00
#define ME_IO_SINGLE_TYPE_DIO_BIT					0x01
#define ME_IO_SINGLE_TYPE_DIO_BYTE					0x02
#define ME_IO_SINGLE_TYPE_DIO_WORD					0x04
#define ME_IO_SINGLE_TYPE_DIO_DWORD					0x08
#define ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS			0x10
#define ME_IO_SINGLE_TYPE_NONBLOCKING				0x20
#define ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING			ME_IO_SINGLE_TYPE_NONBLOCKING
#define ME_IO_SINGLE_TYPE_READ_NONBLOCKING			ME_IO_SINGLE_TYPE_NONBLOCKING

#define ME_IO_SINGLE_TYPE_FREQ_DIVIDER				0x40	/* FIO*/
#define ME_IO_SINGLE_TYPE_FREQ_START_LOW			0x80	/* FO */
#define ME_IO_SINGLE_TYPE_FO_START_LOW				ME_IO_SINGLE_TYPE_FREQ_START_LOW
#define ME_IO_SINGLE_TYPE_FREQ_START_SOFT			0x100	/* FO */
#define ME_IO_SINGLE_TYPE_FO_START_SOFT				ME_IO_SINGLE_TYPE_FREQ_START_SOFT
#define ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE			0x200	/* FI */
#define ME_IO_SINGLE_TYPE_FI_LAST_VALUE				ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE
#define ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY			0x200	/* FO */
#define ME_IO_SINGLE_TYPE_FO_UPDATE_ONLY			ME_IO_SINGLE_TYPE_FREQ_UPDATE_ONLY
#define ME_IO_SINGLE_TYPE_FIO_TICKS_TOTAL			0x400	/* FIO*/
#define ME_IO_SINGLE_TYPE_FIO_TICKS_FIRST_PHASE		0x800	/* FIO*/



/*==================================================================
  Defines for meIOStreamConfig function
  ================================================================*/

#define ME_IO_STREAM_CONFIG_NO_FLAGS				0x0
#define ME_IO_STREAM_CONFIG_BIT_PATTERN				0x1
#define ME_IO_STREAM_CONFIG_WRAPAROUND				0x2
#define ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD			0x4
#define ME_IO_STREAM_CONFIG_HARDWARE_ONLY			0x8

#define ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS			0x0
/*
//REMOVED!
// Flag for extra precise range im me0700
#define ME_IO_STREAM_CONFIG_TYPE_EXTRA_SHUNT		0x100
*/

#define ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS			0x0

/*==================================================================
  Defines for meIOStreamRead function
  ================================================================*/

#define ME_READ_MODE_BLOCKING						0x00100001
#define ME_READ_MODE_NONBLOCKING					0x00100002

#define ME_IO_STREAM_READ_NO_FLAGS					0x0
#define ME_IO_STREAM_READ_FRAMES					0x1

/*==================================================================
  Defines for meIOStreamWrite function
  ================================================================*/

#define ME_WRITE_MODE_BLOCKING						0x00110001
#define ME_WRITE_MODE_NONBLOCKING					0x00110002
#define ME_WRITE_MODE_PRELOAD						0x00110003

#define ME_IO_STREAM_WRITE_NO_FLAGS					0x00000000

/*==================================================================
  Defines for meIOStreamStart function
  ================================================================*/

#define ME_IO_STREAM_START_NO_FLAGS					0x00000000
#define ME_IO_STREAM_START_NONBLOCKING				0x20

#define ME_START_MODE_BLOCKING						0x00120001
#define ME_START_MODE_NONBLOCKING					0x00120002

#define ME_IO_STREAM_START_TYPE_NO_FLAGS			0x00000000
#define ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS	0x00000010

/*==================================================================
  Defines for meIOStreamStop function
  ================================================================*/

#define ME_IO_STREAM_STOP_NO_FLAGS					0x00000000
#define ME_IO_STREAM_STOP_NONBLOCKING				0x20

#define ME_STOP_MODE_IMMEDIATE						0x00130001
#define ME_STOP_MODE_LAST_VALUE						0x00130002

#define ME_IO_STREAM_STOP_TYPE_NO_FLAGS				0x00000000
#define ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS		0x00000001

/*==================================================================
  Defines for meIOStreamStatus function
  ================================================================*/

#define ME_WAIT_NONE								0x00140001
#define ME_WAIT_IDLE								0x00140002
#define ME_WAIT_BUSY								0x00140003

#define ME_STATUS_INVALID							0x00000000
#define ME_STATUS_IDLE								0x00150001
#define ME_STATUS_BUSY								0x00150002
#define ME_STATUS_ERROR								0x00150003

#define ME_IO_STREAM_STATUS_NO_FLAGS				0x00000000

/*==================================================================
  Defines for meIOStreamSetCallbacks function
  ================================================================*/

#define ME_IO_STREAM_SET_CALLBACKS_NO_FLAGS			0x00000000

/*==================================================================
  Defines for meIOStreamNewValues function
  ================================================================*/

#define ME_IO_STREAM_NEW_VALUES_NO_FLAGS			0x00000000

/*==================================================================
  Defines for meIOTimeToTicks function
  ================================================================*/

#define ME_IO_TIME_TO_TICKS_NO_FLAGS				0x00000000
#define ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS			ME_IO_TIME_TO_TICKS_NO_FLAGS
#define ME_IO_TIME_TO_TICKS_MEPHISTO_SCOPE_OSCILLOSCOPE	0x1

/*==================================================================
  Defines for meIOFrequencyToTicks function
  ================================================================*/

#define ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS			0x00000000
#define ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS	ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS
#define ME_IO_FREQUENCY_TO_TICKS_MEPHISTO_SCOPE_OSCILLOSCOPE	0x1


/*==================================================================
  Defines for meIOSetChannelOffset function
  ================================================================*/

#define ME_IO_SET_CHANNEL_OFFSET_NO_FLAGS			0x0000000

/*==================================================================
  Defines for module types
  ================================================================*/

#define ME_MODULE_TYPE_MULTISIG_NONE				0x00000000
#define ME_MODULE_TYPE_MULTISIG_DIFF16_10V			0x00160001
#define ME_MODULE_TYPE_MULTISIG_DIFF16_20V			0x00160002
#define ME_MODULE_TYPE_MULTISIG_DIFF16_50V			0x00160003
#define ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA	0x00160004
#define ME_MODULE_TYPE_MULTISIG_RTD8_PT100			0x00160005
#define ME_MODULE_TYPE_MULTISIG_RTD8_PT500			0x00160006
#define ME_MODULE_TYPE_MULTISIG_RTD8_PT1000			0x00160007
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B			0x00160008
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E			0x00160009
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J			0x0016000A
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K			0x0016000B
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N			0x0016000C
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R			0x0016000D
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S			0x0016000E
#define ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T			0x0016000F
#define ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR		0x00160010

/*==================================================================
  Defines for meQuerySubdeviceCaps function
  ================================================================*/
/// Common definitions
#define ME_CAPS_NONE								0x00000000
#define ME_CAPS_TRIG_DIGITAL						0x00008000
#define ME_CAPS_TRIG_ANALOG							0x00010000
#define ME_CAPS_TRIG_EDGE_RISING					0x00020000
#define ME_CAPS_TRIG_EDGE_FALLING					0x00040000
#define ME_CAPS_TRIG_EDGE_ANY						0x00080000

/// Digital IO
// Configuration size
#define ME_CAPS_DIO_DIR_BIT							0x00000001
#define ME_CAPS_DIO_DIR_BYTE						0x00000002
#define ME_CAPS_DIO_DIR_WORD						0x00000004
#define ME_CAPS_DIO_DIR_DWORD						0x00000008
// Output type
#define ME_CAPS_DIO_SINK_SOURCE						0x00000010
// Input features
#define ME_CAPS_DIO_BIT_PATTERN_IRQ					0x00000020
#define ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING		0x00000040
#define ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING		0x00000080
#define ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY			0x00000100
// Output features
#define ME_CAPS_DIO_OVER_TEMP_IRQ					0x00000200
// Triggering
#define ME_CAPS_DIO_TRIG_SYNCHRONOUS				0x00004000
#define ME_CAPS_DIO_TRIG_DIGITAL					ME_CAPS_TRIG_DIGITAL
#define ME_CAPS_DIO_TRIG_ANALOG						ME_CAPS_TRIG_ANALOG
#define ME_CAPS_DIO_TRIG_EDGE_RISING				ME_CAPS_TRIG_EDGE_RISING
#define ME_CAPS_DIO_TRIG_EDGE_FALLING				ME_CAPS_TRIG_EDGE_FALLING
#define ME_CAPS_DIO_TRIG_EDGE_ANY					ME_CAPS_TRIG_EDGE_ANY

/// Counter
#define ME_CAPS_CTR_CLK_PREVIOUS					0x00000001
#define ME_CAPS_CTR_CLK_INTERNAL_1MHZ				0x00000002
#define ME_CAPS_CTR_CLK_INTERNAL_10MHZ				0x00000004
#define ME_CAPS_CTR_CLK_EXTERNAL					0x00000008

/// Analog input
#define ME_CAPS_AI_TRIG_SYNCHRONOUS					0x00000001
#define ME_CAPS_AI_TRIG_SIMULTANEOUS				ME_CAPS_AI_TRIG_SYNCHRONOUS
#define ME_CAPS_AI_FIFO								0x00000002
#define ME_CAPS_AI_FIFO_THRESHOLD					0x00000004
#define ME_CAPS_AI_SAMPLE_HOLD						0x00000008
#define ME_CAPS_AI_TRIG_DIGITAL						ME_CAPS_TRIG_DIGITAL
#define ME_CAPS_AI_TRIG_ANALOG						ME_CAPS_TRIG_ANALOG
#define ME_CAPS_AI_TRIG_EDGE_RISING					ME_CAPS_TRIG_EDGE_RISING
#define ME_CAPS_AI_TRIG_EDGE_FALLING				ME_CAPS_TRIG_EDGE_FALLING
#define ME_CAPS_AI_TRIG_EDGE_ANY					ME_CAPS_TRIG_EDGE_ANY
#define ME_CAPS_AI_DIFFERENTIAL						0x00000010

/// Analog output
#define ME_CAPS_AO_TRIG_SYNCHRONOUS					0x00000001
#define ME_CAPS_AO_TRIG_SIMULTANEOUS				ME_CAPS_AO_TRIG_SYNCHRONOUS
#define ME_CAPS_AO_FIFO								0x00000002
#define ME_CAPS_AO_FIFO_THRESHOLD					0x00000004
#define ME_CAPS_AO_TRIG_DIGITAL						ME_CAPS_TRIG_DIGITAL
#define ME_CAPS_AO_TRIG_ANALOG						ME_CAPS_TRIG_ANALOG
#define ME_CAPS_AO_TRIG_EDGE_RISING					ME_CAPS_TRIG_EDGE_RISING
#define ME_CAPS_AO_TRIG_EDGE_FALLING				ME_CAPS_TRIG_EDGE_FALLING
#define ME_CAPS_AO_TRIG_EDGE_ANY					ME_CAPS_TRIG_EDGE_ANY
#define ME_CAPS_AO_DIFFERENTIAL						0x00000010

/// External interrupt
#define ME_CAPS_EXT_IRQ_EDGE_RISING					0x00000001
#define ME_CAPS_EXT_IRQ_EDGE_FALLING				0x00000002
#define ME_CAPS_EXT_IRQ_EDGE_ANY					0x00000004

/// FPGA standart
// FPGA bus size
#define ME_CAPS_FPGA_BUSS_BIT						0x00000001
#define ME_CAPS_FPGA_BUSS_BYTE						0x00000002
#define ME_CAPS_FPGA_BUSS_WORD						0x00000004
#define ME_CAPS_FPGA_BUSS_DWORD						0x00000008
#define ME_CAPS_FPGA_IN_FIFO						0x00000002
#define ME_CAPS_FPGA_OUT_FIFO						0x00000002

/*==================================================================
  Defines for meQuerySubdeviceCapsArgs function
  ================================================================*/

#define ME_CAP_AI_FIFO_SIZE							0x001D0000
#define ME_CAP_AI_BUFFER_SIZE						0x001D0001
#define ME_CAP_AI_CHANNEL_LIST_SIZE					0x001D0002
#define ME_CAP_AI_MAX_THRESHOLD_SIZE				0x001D0003

#define ME_CAP_AO_FIFO_SIZE							0x001F0000
#define ME_CAP_AO_BUFFER_SIZE						0x001F0001
#define ME_CAP_AO_CHANNEL_LIST_SIZE					0x001F0002
#define ME_CAP_AO_MAX_THRESHOLD_SIZE				0x001F0003

#define ME_CAP_CTR_WIDTH							0x00200000

#define ME_CAP_FPGA_IN_FIFO_SIZE					0x00300000
#define ME_CAP_FPGA_IN_BUFFER_SIZE					0x00300001
#define ME_CAP_FPGA_OUT_FIFO_SIZE					0x00300002
#define ME_CAP_FPGA_OUT_BUFFER_SIZE					0x00300003

/*==================================================================
  Defines common to query functions
  ================================================================*/

#define ME_UNIT_INVALID								0x00000000
#define ME_UNIT_VOLT								0x00170001
#define ME_UNIT_AMPERE								0x00170002
#define ME_UNIT_ANY									0x00170003
#define ME_UNIT_Hz									0x00170004

#define ME_TYPE_INVALID								0x00000000
#define ME_TYPE_AO									0x00180001
#define ME_TYPE_AI									0x00180002
#define ME_TYPE_DIO									0x00180003
#define ME_TYPE_DO									0x00180004
#define ME_TYPE_DI									0x00180005
#define ME_TYPE_CTR									0x00180006
#define ME_TYPE_EXT_IRQ								0x00180007
#define ME_TYPE_FREQ_IO								0x00180008
#define ME_TYPE_FIO									ME_TYPE_FREQ_IO
#define ME_TYPE_FREQ_O								0x00180009
#define ME_TYPE_FO									ME_TYPE_FREQ_I
#define ME_TYPE_FREQ_I								0x0018000A
#define ME_TYPE_FI									ME_TYPE_FREQ_O
#define ME_TYPE_FPGA								0x0018000B



#define ME_SUBTYPE_INVALID							0x00000000
#define ME_SUBTYPE_SINGLE							0x00190001
#define ME_SUBTYPE_STREAMING						0x00190002
#define ME_SUBTYPE_CTR_8254							0x00190003
#define ME_SUBTYPE_ANY								0x00190004
#define ME_SUBTYPE_CTR								0x00190006
// #define ME_SUBTYPE_DIGITAL							0x00190007
// #define ME_SUBTYPE_ANALOG							0x00190008
// #define ME_SUBTYPE_DMA								0x00190009

#define ME_DEVICE_DRIVER_NAME_MAX_COUNT				64
#define ME_DEVICE_NAME_MAX_COUNT					64

#define ME_DEVICE_DESCRIPTION_MAX_COUNT				256

#define ME_BUS_TYPE_INVALID							0x00000000
/// @note ME_BUS_TYPE_ANY only for checking bus
#define ME_BUS_TYPE_ANY								0x001A0000
#define ME_BUS_TYPE_PCI								0x001A0001
#define ME_BUS_TYPE_USB								0x001A0002
#define ME_BUS_TYPE_LAN_PCI							0x001A0101
#define ME_BUS_TYPE_LAN_USB							0x001A0102

#define ME_PLUGGED_INVALID							0x00000000
/// @note ME_PLUGGED_ANY only for checking status
#define ME_PLUGGED_ANY								0x001B0000
#define ME_PLUGGED_IN								0x001B0001
#define ME_PLUGGED_OUT								0x001B0002

#define ME_EXTENSION_TYPE_INVALID					0x00000000
#define ME_EXTENSION_TYPE_NONE						0x001C0001
#define ME_EXTENSION_TYPE_MUX32M					0x001C0002
#define ME_EXTENSION_TYPE_DEMUX32					0x001C0003
#define ME_EXTENSION_TYPE_MUX32S					0x001C0004

#define ME_ACCESS_TYPE_INVALID						0x00000000
#define ME_ACCESS_TYPE_ANY							0x001D0000
#define ME_ACCESS_TYPE_LOCAL						0x001D0001
#define ME_ACCESS_TYPE_REMOTE						0x001D0002

/*==================================================================
  Defines for ConfigLoad
  ================================================================*/
#define ME_CONF_LOAD_CUSTOM_DRIVER					0x00001000

/*==================================================================
  Defines for FPGA boConfigLoad
  ================================================================*/
#define ME_FPGA_FIRMWARE							0xFF000001
#define ME_FPGA_SUBDEVICE							0xFF000002


/*==================================================================
  Defines for meUtilityPWM
  ================================================================*/
#define ME_PWM_START_NO_FLAGS						0x00000000
#define ME_PWM_START_CONNECT_INTERNAL				0x00000001

/*==================================================================
  Defines of flags common to query functions
  ================================================================*/
#define ME_QUERY_NO_FLAGS 							0x00000000

/*==================================================================
  Defines of flags for error handling
  ================================================================*/
#define ME_ERRNO_CLEAR_FLAGS						0x01


/*==================================================================

  Defines for meIOSingleTimeToTicks function

  ================================================================*/
#define ME_IO_SINGLE_TIME_TO_TICKS_NO_FLAGS                   0x0

/*==================================================================

  Defines for meIOSingleTicksToTime function

  ================================================================*/
#define ME_IO_SINGLE_TICKS_TO_TIME_NO_FLAGS                   0x0


#endif
