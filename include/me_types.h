/**
 * @file me_types.h
 *
 * @brief Internal APIs type definitions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _ME_TYPES_H_
# define _ME_TYPES_H_

# include "../common/metypes.h"

# include <linux/types.h>
# ifndef __KERNEL__
#  include <stdint.h>
# endif	// __KERNEL__

#define ME_TRIGGER_TYPE_SOFTWARE		0x00

#define ME_TRIGGER_TYPE_DIGITAL			0x00
#define ME_TRIGGER_TYPE_ANALOG			0x10

#define ME_TRIGGER_TYPE_THRESHOLD		0x20
#define ME_TRIGGER_TYPE_WINDOW			0x40
#define ME_TRIGGER_TYPE_EDGE			0x80
#define ME_TRIGGER_TYPE_SLOPE			0x100
#define ME_TRIGGER_TYPE_PATTERN			0x200

#define ME_TRIGGER_TYPE_ACQ				0x01
#define ME_TRIGGER_TYPE_LIST			0x02
#define ME_TRIGGER_TYPE_CONV			0x04

#define ME_TRIGGER_TYPE_ACQ_DIGITAL		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_DIGITAL)
#define ME_TRIGGER_TYPE_LIST_DIGITAL	(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_DIGITAL)
#define ME_TRIGGER_TYPE_CONV_DIGITAL	(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_DIGITAL)

#define ME_TRIGGER_TYPE_ACQ_ANALOG		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_ANALOG)
#define ME_TRIGGER_TYPE_LIST_ANALOG		(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_ANALOG)
#define ME_TRIGGER_TYPE_CONV_ANALOG		(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_ANALOG)

#define ME_TRIGGER_TYPE_ACQ_THRESHOLD	(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_THRESHOLD)
#define ME_TRIGGER_TYPE_LIST_THRESHOLD	(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_THRESHOLD)
#define ME_TRIGGER_TYPE_CONV_THRESHOLD	(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_THRESHOLD)

#define ME_TRIGGER_TYPE_ACQ_WINDOW		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_WINDOW)
#define ME_TRIGGER_TYPE_LIST_WINDOW		(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_WINDOW)
#define ME_TRIGGER_TYPE_CONV_WINDOW		(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_WINDOW)

#define ME_TRIGGER_TYPE_ACQ_EDGE		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_EDGE)
#define ME_TRIGGER_TYPE_LIST_EDGE		(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_EDGE)
#define ME_TRIGGER_TYPE_CONV_EDGE		(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_EDGE)

#define ME_TRIGGER_TYPE_ACQ_SLOPE		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_SLOPE)
#define ME_TRIGGER_TYPE_LIST_SLOPE		(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_SLOPE)
#define ME_TRIGGER_TYPE_CONV_SLOPE		(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_SLOPE)

#define ME_TRIGGER_TYPE_ACQ_PATTERN		(ME_TRIGGER_TYPE_ACQ  | ME_TRIGGER_TYPE_PATTERN)
#define ME_TRIGGER_TYPE_LIST_PATTERN	(ME_TRIGGER_TYPE_LIST | ME_TRIGGER_TYPE_PATTERN)
#define ME_TRIGGER_TYPE_CONV_PATTERN	(ME_TRIGGER_TYPE_CONV | ME_TRIGGER_TYPE_PATTERN)

typedef struct //meIOStreamSimpleTriggers
{
	int trigger_type;

	int trigger_edge;
	int trigger_level_lower;
	int trigger_level_upper;
	int trigger_point;


	uint64_t acq_ticks;
	uint64_t scan_ticks;
	uint64_t conv_ticks;

	int synchro;

	int stop_type;
	int stop_count;
}  meIOStreamSimpleTriggers_t;

typedef struct //meIOStreamSimpleConfig
{
	int iChannel;
	int iRange;
} meIOStreamSimpleConfig_t;

#endif