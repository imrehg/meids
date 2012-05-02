/**
 * @file me_defines.h
 *
 * @brief Internal definitions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _ME_DEFINES_H_
# define _ME_DEFINES_H_

# include "../common/medefines.h"

/* Flags for SingleConfig channels configure */
# define ME_SINGLE_CHANNEL_NOT_CONFIGURED			0x0000
# define ME_SINGLE_CHANNEL_CONFIGURED				0x0001
# define ME_SINGLE_CHANNEL_CONFIG_MASK				0x0001

/* Define driver type */
# define ME_DRIVER_UNKNOWN							0x0000
# define ME_DRIVER_PCI								0x0001
# define ME_DRIVER_USB								0x0002

# define ME_IO_IRQ_WAIT_PRESERVE					0x1000

/// Report new sytuation only when ISM read/write data from/to buffer. User operations are screened.
# define ME_IO_STREAM_NEW_VALUES_SCREEN_FLAG		0x0001
# define ME_IO_STREAM_NEW_VALUES_ERROR_REPORT_FLAG	0x0002

#define ME_LOCK_PRESERVE							0x01
#define ME_LOCK_FORCE								0x10

#define ME_STREAM_CONFIG_DIFFERENTIAL				0x00100000
#define ME_REF_AIO_NONE								0x70000000
#define ME_REF_AIO_DIFFERENTIAL						0x70000001
#define ME_REF_AIO_GROUND							0x70000002

#define ME_STREAM_STOP_TYPE_NONE					0x00000000
#define ME_STREAM_STOP_TYPE_MANUAL					ME_STREAM_STOP_TYPE_NONE
#define ME_STREAM_STOP_TYPE_ACQ_LIST				0x000A1001
#define ME_STREAM_STOP_TYPE_SCAN_VALUE				0x000A1002

#define ME_QUERY_REFRESH_FLAGS			1		// 'ME_QUERY_DEEP_FORCE_FLAGS' plus saving results in library contects.
#define ME_QUERY_FORCE_FLAGS 			0x100	// For lockal devices read information from driver. For remote ones read information from copy in remote library contects.
#define ME_QUERY_DEEP_FORCE_FLAGS 		0x300	// Always read information from driver (slow).

/*
//REMOVED!
#define ME_AI_EXTRA_RANGE				0x10000000
*/
#endif
