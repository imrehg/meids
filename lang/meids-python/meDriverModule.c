/* Python extension module with API for Meilhaus Driver system.
 * ============================================================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author:	Guenter Gebhardt	<g.gebhardt@meilhaus.de>
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#include <Python.h>
#include <Numeric/arrayobject.h>
#include <medriver.h>

#ifdef ME_WINDOWS
	typedef __int64 int64_t;
#else
#ifndef __int8_t_defined
	typedef long long int64_t;
#endif //__int8_t_defined
#endif


void initmeDriver(void);

/* Module specific error exception */
static PyObject *meError;


/*===========================================================================
  Library constants definitions
  =========================================================================*/

/* Possible constant types */
#define ME_PY_NONE    0
#define ME_PY_INT     1
#define ME_PY_FLOAT   2

/* Constant information structure */
typedef struct me_const_info {
	int type;
	char *name;
	long lvalue;
	double dvalue;
} me_const_info_t;

/* Constants from medefines.h */
static me_const_info_t me_const_table[] = {
	{ ME_PY_INT, "ME_VALUE_NOT_USED", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_VALUE_DEFAULT", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_NO_FLAGS", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_QUERY_NO_FLAGS", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_VALUE_INVALID", (long) ~ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_LOCK_RELEASE", (long) ME_LOCK_RELEASE, 0 },
	{ ME_PY_INT, "ME_LOCK_SET", (long) ME_LOCK_SET, 0 },
	{ ME_PY_INT, "ME_LOCK_CHECK", (long) ME_LOCK_CHECK, 0 },
	{ ME_PY_INT, "ME_OPEN_NO_FLAGS", (long) ME_OPEN_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_CLOSE_NO_FLAGS", (long) ME_CLOSE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_LOCK_DRIVER_NO_FLAGS", (long) ME_LOCK_DRIVER_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_LOCK_DEVICE_NO_FLAGS", (long) ME_LOCK_DEVICE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_LOCK_SUBDEVICE_NO_FLAGS", (long) ME_LOCK_SUBDEVICE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_ERROR_MSG_MAX_COUNT", (long) ME_ERROR_MSG_MAX_COUNT, 0 },
	{ ME_PY_INT, "ME_SWITCH_DISABLE", (long) ME_SWITCH_DISABLE, 0 },
	{ ME_PY_INT, "ME_SWITCH_ENABLE", (long) ME_SWITCH_ENABLE, 0 },
	{ ME_PY_INT, "ME_REF_DIO_FIFO_LOW", (long) ME_REF_DIO_FIFO_LOW, 0 },
	{ ME_PY_INT, "ME_REF_DIO_FIFO_HIGH", (long) ME_REF_DIO_FIFO_HIGH, 0 },
	{ ME_PY_INT, "ME_REF_CTR_PREVIOUS", (long) ME_REF_CTR_PREVIOUS, 0 },
	{ ME_PY_INT, "ME_REF_CTR_INTERNAL_1MHZ", (long) ME_REF_CTR_INTERNAL_1MHZ, 0 },
	{ ME_PY_INT, "ME_REF_CTR_INTERNAL_10MHZ", (long) ME_REF_CTR_INTERNAL_10MHZ, 0 },
	{ ME_PY_INT, "ME_REF_CTR_EXTERNAL", (long) ME_REF_CTR_EXTERNAL, 0 },
	{ ME_PY_INT, "ME_REF_AI_GROUND", (long) ME_REF_AI_GROUND, 0 },
	{ ME_PY_INT, "ME_REF_AI_DIFFERENTIAL", (long) ME_REF_AI_DIFFERENTIAL, 0 },
	{ ME_PY_INT, "ME_REF_AO_GROUND", (long) ME_REF_AO_GROUND, 0 },
	{ ME_PY_INT, "ME_REF_NONE", (long) ME_REF_NONE, 0 },
	{ ME_PY_INT, "ME_TRIG_CHAN_NONE", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_TRIG_CHAN_DEFAULT", (long) ME_TRIG_CHAN_DEFAULT, 0 },
	{ ME_PY_INT, "ME_TRIG_CHAN_SYNCHRONOUS", (long) ME_TRIG_CHAN_SYNCHRONOUS, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_NONE", (long) ME_TRIG_TYPE_NONE, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_SW", (long) ME_TRIG_TYPE_SW, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_THRESHOLD", (long) ME_TRIG_TYPE_THRESHOLD, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_WINDOW", (long) ME_TRIG_TYPE_WINDOW, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_EDGE", (long) ME_TRIG_TYPE_EDGE, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_SLOPE", (long) ME_TRIG_TYPE_SLOPE, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_EXT_DIGITAL", (long) ME_TRIG_TYPE_EXT_DIGITAL, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_EXT_ANALOG", (long) ME_TRIG_TYPE_EXT_ANALOG, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_PATTERN", (long) ME_TRIG_TYPE_PATTERN, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_TIMER", (long) ME_TRIG_TYPE_TIMER, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_COUNT", (long) ME_TRIG_TYPE_COUNT, 0 },
	{ ME_PY_INT, "ME_TRIG_TYPE_FOLLOW", (long) ME_TRIG_TYPE_FOLLOW, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_NONE", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_ABOVE", (long) ME_TRIG_EDGE_ABOVE, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_BELOW", (long) ME_TRIG_EDGE_BELOW, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_ENTRY", (long) ME_TRIG_EDGE_ENTRY, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_EXIT", (long) ME_TRIG_EDGE_EXIT, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_RISING", (long) ME_TRIG_EDGE_RISING, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_FALLING", (long) ME_TRIG_EDGE_FALLING, 0 },
	{ ME_PY_INT, "ME_TRIG_EDGE_ANY", (long) ME_TRIG_EDGE_ANY, 0 },
	{ ME_PY_INT, "ME_TIMER_ACQ_START", (long) ME_TIMER_ACQ_START, 0 },
	{ ME_PY_INT, "ME_TIMER_SCAN_START", (long) ME_TIMER_SCAN_START, 0 },
	{ ME_PY_INT, "ME_TIMER_CONV_START", (long) ME_TIMER_CONV_START, 0 },
#ifdef ME_TIMER_FREQ
	{ ME_PY_INT, "ME_TIMER_FREQ", (long) ME_TIMER_FREQ, 0 },
#endif
	{ ME_PY_INT, "ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS", (long) ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IRQ_SOURCE_DIO_PATTERN", (long) ME_IRQ_SOURCE_DIO_PATTERN, 0 },
	{ ME_PY_INT, "ME_IRQ_SOURCE_DIO_MASK", (long) ME_IRQ_SOURCE_DIO_MASK, 0 },
	{ ME_PY_INT, "ME_IRQ_SOURCE_DIO_LINE", (long) ME_IRQ_SOURCE_DIO_LINE, 0 },
	{ ME_PY_INT, "ME_IRQ_SOURCE_DIO_OVER_TEMP", (long) ME_IRQ_SOURCE_DIO_OVER_TEMP, 0 },
	{ ME_PY_INT, "ME_IRQ_EDGE_NOT_USED", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_IRQ_EDGE_RISING", (long) ME_IRQ_EDGE_RISING, 0 },
	{ ME_PY_INT, "ME_IRQ_EDGE_FALLING", (long) ME_IRQ_EDGE_FALLING, 0 },
	{ ME_PY_INT, "ME_IRQ_EDGE_ANY", (long) ME_IRQ_EDGE_ANY, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_NO_FLAGS", (long) ME_IO_IRQ_START_NO_FLAGS, 0 },
#ifdef ME_IO_STREAM_START_NONBLOCKING
	{ ME_PY_INT, "ME_IO_STREAM_START_NONBLOCKING", (long) ME_IO_STREAM_START_NONBLOCKING, 0 },
#endif
	{ ME_PY_INT, "ME_IO_IRQ_START_DIO_BIT", (long) ME_IO_IRQ_START_DIO_BIT, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_DIO_BYTE", (long) ME_IO_IRQ_START_DIO_BYTE, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_DIO_WORD", (long) ME_IO_IRQ_START_DIO_WORD, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_DIO_DWORD", (long) ME_IO_IRQ_START_DIO_DWORD, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_PATTERN_FILTERING", (long) ME_IO_IRQ_START_PATTERN_FILTERING, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_START_EXTENDED_STATUS", (long) ME_IO_IRQ_START_EXTENDED_STATUS, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_STOP_NO_FLAGS", (long) ME_IO_IRQ_STOP_NO_FLAGS, 0 },
#ifdef ME_IO_STREAM_STOP_NONBLOCKING
	{ ME_PY_INT, "ME_IO_STREAM_STOP_NONBLOCKING", (long) ME_IO_STREAM_STOP_NONBLOCKING, 0 },
#endif
	{ ME_PY_INT, "ME_IO_IRQ_WAIT_NO_FLAGS", (long) ME_IO_IRQ_WAIT_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_IRQ_SET_CALLBACK_NO_FLAGS", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_IO_RESET_DEVICE_NO_FLAGS", (long) ME_IO_RESET_DEVICE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_RESET_SUBDEVICE_NO_FLAGS", (long) ME_IO_RESET_SUBDEVICE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_INPUT", (long) ME_SINGLE_CONFIG_DIO_INPUT, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_OUTPUT", (long) ME_SINGLE_CONFIG_DIO_OUTPUT, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE", (long) ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_SINK", (long) ME_SINGLE_CONFIG_DIO_SINK, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_SOURCE", (long) ME_SINGLE_CONFIG_DIO_SOURCE, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_MUX32M", (long) ME_SINGLE_CONFIG_DIO_MUX32M, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_DEMUX32", (long) ME_SINGLE_CONFIG_DIO_DEMUX32, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_DIO_BIT_PATTERN", (long) ME_SINGLE_CONFIG_DIO_BIT_PATTERN, 0 },
#ifdef ME_SINGLE_CONFIG_MULTIPIN_IRQ
	{ ME_PY_INT, "ME_SINGLE_CONFIG_MULTIPIN_IRQ", (long) ME_SINGLE_CONFIG_MULTIPIN_IRQ, 0 },
#endif
#ifdef ME_SINGLE_CONFIG_MULTIPIN_CLK
	{ ME_PY_INT, "ME_SINGLE_CONFIG_MULTIPIN_CLK", (long) ME_SINGLE_CONFIG_MULTIPIN_CLK, 0 },
#endif
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_0", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_0, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_1", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_1, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_2", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_2, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_3", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_3, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_4", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_4, 0 },
	{ ME_PY_INT, "ME_SINGLE_CONFIG_CTR_8254_MODE_5", (long) ME_SINGLE_CONFIG_CTR_8254_MODE_5, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_NO_FLAGS", (long) ME_IO_SINGLE_CONFIG_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_DIO_BIT", (long) ME_IO_SINGLE_CONFIG_DIO_BIT, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_DIO_BYTE", (long) ME_IO_SINGLE_CONFIG_DIO_BYTE, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_DIO_WORD", (long) ME_IO_SINGLE_CONFIG_DIO_WORD, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_DIO_DWORD", (long) ME_IO_SINGLE_CONFIG_DIO_DWORD, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON", (long) ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF", (long) ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_AI_RMS", (long) ME_IO_SINGLE_CONFIG_AI_RMS, 0 },
#ifdef ME_IO_SINGLE_CONFIG_CONTINUE
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_CONTINUE", (long) ME_IO_SINGLE_CONFIG_CONTINUE, 0 },
#endif
#ifdef ME_IO_SINGLE_CONFIG_MULTIPIN
	{ ME_PY_INT, "ME_IO_SINGLE_CONFIG_MULTIPIN", (long) ME_IO_SINGLE_CONFIG_MULTIPIN, 0 },
#endif
	{ ME_PY_INT, "ME_IO_SINGLE_NO_FLAGS", (long) ME_IO_SINGLE_NO_FLAGS, 0 },
#ifdef ME_IO_SINGLE_NONBLOCKING
	{ ME_PY_INT, "ME_IO_SINGLE_NONBLOCKING", (long) ME_IO_SINGLE_NONBLOCKING, 0 },
#endif
	{ ME_PY_INT, "ME_DIR_INPUT", (long) ME_DIR_INPUT, 0 },
	{ ME_PY_INT, "ME_DIR_OUTPUT", (long) ME_DIR_OUTPUT, 0 },
#ifdef ME_DIR_SET_OFFSET
	{ ME_PY_INT, "ME_DIR_SET_OFFSET", (long) ME_DIR_SET_OFFSET, 0 },
#endif
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_NO_FLAGS", (long) ME_IO_SINGLE_TYPE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_DIO_BIT", (long) ME_IO_SINGLE_TYPE_DIO_BIT, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_DIO_BYTE", (long) ME_IO_SINGLE_TYPE_DIO_BYTE, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_DIO_WORD", (long) ME_IO_SINGLE_TYPE_DIO_WORD, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_DIO_DWORD", (long) ME_IO_SINGLE_TYPE_DIO_DWORD, 0 },
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS", (long) ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS, 0 },
#ifdef ME_IO_SINGLE_TYPE_NONBLOCKING
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_NONBLOCKING", (long) ME_IO_SINGLE_TYPE_NONBLOCKING, 0 },
#endif
#ifdef ME_IO_SINGLE_TYPE_FREQ_DIVIDER
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_FREQ_DIVIDER", (long) ME_IO_SINGLE_TYPE_FREQ_DIVIDER, 0 },
#endif
#ifdef ME_IO_SINGLE_TYPE_FREQ_START_LOW
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_FREQ_START_LOW", (long) ME_IO_SINGLE_TYPE_FREQ_START_LOW, 0 },
#endif
#ifdef ME_IO_SINGLE_TYPE_FREQ_START_SOFT
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_FREQ_START_SOFT", (long) ME_IO_SINGLE_TYPE_FREQ_START_SOFT, 0 },
#endif
#ifdef ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE", (long) ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE, 0 },
#endif
#ifdef ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING
	{ ME_PY_INT, "ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING", (long) ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING, 0 },
#endif
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_NO_FLAGS", (long) ME_IO_STREAM_CONFIG_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_BIT_PATTERN", (long) ME_IO_STREAM_CONFIG_BIT_PATTERN, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_WRAPAROUND", (long) ME_IO_STREAM_CONFIG_WRAPAROUND, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD", (long) ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD, 0 },
#ifdef ME_IO_STREAM_CONFIG_HARDWARE_ONLY
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_HARDWARE_ONLY", (long) ME_IO_STREAM_CONFIG_HARDWARE_ONLY, 0 },
#endif
	{ ME_PY_INT, "ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS", (long) ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS", (long) ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_READ_MODE_BLOCKING", (long) ME_READ_MODE_BLOCKING, 0 },
	{ ME_PY_INT, "ME_READ_MODE_NONBLOCKING", (long) ME_READ_MODE_NONBLOCKING, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_READ_FRAMES", (long) ME_IO_STREAM_READ_FRAMES, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_READ_NO_FLAGS", (long) ME_IO_STREAM_READ_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_WRITE_MODE_BLOCKING", (long) ME_WRITE_MODE_BLOCKING, 0 },
	{ ME_PY_INT, "ME_WRITE_MODE_NONBLOCKING", (long) ME_WRITE_MODE_NONBLOCKING, 0 },
	{ ME_PY_INT, "ME_WRITE_MODE_PRELOAD", (long) ME_WRITE_MODE_PRELOAD, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_WRITE_NO_FLAGS", (long) ME_IO_STREAM_WRITE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_START_NO_FLAGS", (long) ME_IO_STREAM_START_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_START_MODE_BLOCKING", (long) ME_START_MODE_BLOCKING, 0 },
	{ ME_PY_INT, "ME_START_MODE_NONBLOCKING", (long) ME_START_MODE_NONBLOCKING, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_START_TYPE_NO_FLAGS", (long) ME_IO_STREAM_START_TYPE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS", (long) ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_STOP_NO_FLAGS", (long) ME_IO_STREAM_STOP_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_STOP_MODE_IMMEDIATE", (long) ME_STOP_MODE_IMMEDIATE, 0 },
	{ ME_PY_INT, "ME_STOP_MODE_LAST_VALUE", (long) ME_STOP_MODE_LAST_VALUE, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_STOP_TYPE_NO_FLAGS", (long) ME_IO_STREAM_STOP_TYPE_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS", (long) ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS, 0 },
	{ ME_PY_INT, "ME_WAIT_NONE", (long) ME_WAIT_NONE, 0 },
	{ ME_PY_INT, "ME_WAIT_IDLE", (long) ME_WAIT_IDLE, 0 },
#ifdef ME_WAIT_BUSY
	{ ME_PY_INT, "ME_WAIT_BUSY", (long) ME_WAIT_BUSY, 0 },
#endif
	{ ME_PY_INT, "ME_STATUS_INVALID", (long) ME_STATUS_INVALID, 0 },
	{ ME_PY_INT, "ME_STATUS_IDLE", (long) ME_STATUS_IDLE, 0 },
	{ ME_PY_INT, "ME_STATUS_BUSY", (long) ME_STATUS_BUSY, 0 },
	{ ME_PY_INT, "ME_STATUS_ERROR", (long) ME_STATUS_ERROR, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_STATUS_NO_FLAGS", (long) ME_IO_STREAM_STATUS_NO_FLAGS, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_SET_CALLBACKS_NO_FLAGS", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_IO_STREAM_NEW_VALUES_NO_FLAGS", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_IO_TIME_TO_TICKS_NO_FLAGS", (long) ME_IO_TIME_TO_TICKS_NO_FLAGS, 0 },
#ifdef ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS
	{ ME_PY_INT, "ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS", (long) ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS, 0 },
#endif
#ifdef ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS
	{ ME_PY_INT, "ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS", (long) ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS, 0 },
#endif
#ifdef ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS
	{ ME_PY_INT, "ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS", (long) ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS, 0 },
#endif
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_NONE", (long) ME_MODULE_TYPE_MULTISIG_NONE, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_DIFF16_10V", (long) ME_MODULE_TYPE_MULTISIG_DIFF16_10V, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_DIFF16_20V", (long) ME_MODULE_TYPE_MULTISIG_DIFF16_20V, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_DIFF16_50V", (long) ME_MODULE_TYPE_MULTISIG_DIFF16_50V, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA", (long) ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_RTD8_PT100", (long) ME_MODULE_TYPE_MULTISIG_RTD8_PT100, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_RTD8_PT500", (long) ME_MODULE_TYPE_MULTISIG_RTD8_PT500, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_RTD8_PT1000", (long) ME_MODULE_TYPE_MULTISIG_RTD8_PT1000, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T", (long) ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T, 0 },
	{ ME_PY_INT, "ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR", (long) ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR, 0 },
	{ ME_PY_INT, "ME_CAPS_NONE", (long) ME_VALUE_NOT_USED, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_BIT", (long) ME_CAPS_DIO_DIR_BIT, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_BYTE", (long) ME_CAPS_DIO_DIR_BYTE, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_WORD", (long) ME_CAPS_DIO_DIR_WORD, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_DWORD", (long) ME_CAPS_DIO_DIR_DWORD, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_SINK_SOURCE", (long) ME_CAPS_DIO_SINK_SOURCE, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_BIT_PATTERN_IRQ", (long) ME_CAPS_DIO_BIT_PATTERN_IRQ, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING", (long) ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING", (long) ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY", (long) ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_OVER_TEMP_IRQ", (long) ME_CAPS_DIO_OVER_TEMP_IRQ, 0 },
	{ ME_PY_INT, "ME_CAPS_CTR_CLK_PREVIOUS", (long) ME_CAPS_CTR_CLK_PREVIOUS, 0 },
	{ ME_PY_INT, "ME_CAPS_CTR_CLK_INTERNAL_1MHZ", (long) ME_CAPS_CTR_CLK_INTERNAL_1MHZ, 0 },
	{ ME_PY_INT, "ME_CAPS_CTR_CLK_INTERNAL_10MHZ", (long) ME_CAPS_CTR_CLK_INTERNAL_10MHZ, 0 },
	{ ME_PY_INT, "ME_CAPS_CTR_CLK_EXTERNAL", (long) ME_CAPS_CTR_CLK_EXTERNAL, 0 },
#ifdef ME_CAPS_AI_TRIG_SIMULTANEOUS
	{ ME_PY_INT, "ME_CAPS_AI_TRIG_SIMULTANEOUS", (long) ME_CAPS_AI_TRIG_SIMULTANEOUS, 0 },
	{ ME_PY_INT, "ME_CAPS_AI_TRIG_SYNCHRONOUS", (long) ME_CAPS_AI_TRIG_SIMULTANEOUS, 0 },
#else
# ifdef ME_CAPS_AI_TRIG_SYNCHRONOUS
	{ ME_PY_INT, "ME_CAPS_AI_TRIG_SIMULTANEOUS", (long) ME_CAPS_AI_TRIG_SYNCHRONOUS, 0 },
	{ ME_PY_INT, "ME_CAPS_AI_TRIG_SYNCHRONOUS", (long) ME_CAPS_AI_TRIG_SYNCHRONOUS, 0 },
# endif
#endif
	{ ME_PY_INT, "ME_CAPS_AI_FIFO", (long) ME_CAPS_AI_FIFO, 0 },
	{ ME_PY_INT, "ME_CAPS_AI_FIFO_THRESHOLD", (long) ME_CAPS_AI_FIFO_THRESHOLD, 0 },
#ifdef ME_CAPS_AO_TRIG_SIMULTANEOUS
	{ ME_PY_INT, "ME_CAPS_AO_TRIG_SIMULTANEOUS", (long) ME_CAPS_AO_TRIG_SIMULTANEOUS, 0 },
	{ ME_PY_INT, "ME_CAPS_AO_TRIG_SYNCHRONOUS", (long) ME_CAPS_AO_TRIG_SIMULTANEOUS, 0 },
#else
# ifdef ME_CAPS_AO_TRIG_SYNCHRONOUS
	{ ME_PY_INT, "ME_CAPS_AO_TRIG_SIMULTANEOUS", (long) ME_CAPS_AO_TRIG_SYNCHRONOUS, 0 },
	{ ME_PY_INT, "ME_CAPS_AO_TRIG_SYNCHRONOUS", (long) ME_CAPS_AO_TRIG_SYNCHRONOUS, 0 },
# endif
#endif
	{ ME_PY_INT, "ME_CAPS_AO_FIFO", (long) ME_CAPS_AO_FIFO, 0 },
	{ ME_PY_INT, "ME_CAPS_AO_FIFO_THRESHOLD", (long) ME_CAPS_AO_FIFO_THRESHOLD, 0 },
	{ ME_PY_INT, "ME_CAPS_EXT_IRQ_EDGE_RISING", (long) ME_CAPS_EXT_IRQ_EDGE_RISING, 0 },
	{ ME_PY_INT, "ME_CAPS_EXT_IRQ_EDGE_FALLING", (long) ME_CAPS_EXT_IRQ_EDGE_FALLING, 0 },
	{ ME_PY_INT, "ME_CAPS_EXT_IRQ_EDGE_ANY", (long) ME_CAPS_EXT_IRQ_EDGE_ANY, 0 },
	{ ME_PY_INT, "ME_CAP_AI_FIFO_SIZE", (long) ME_CAP_AI_FIFO_SIZE, 0 },
#ifdef ME_CAP_AI_BUFFER_SIZE
	{ ME_PY_INT, "ME_CAP_AI_BUFFER_SIZE", (long) ME_CAP_AI_BUFFER_SIZE, 0 },
#endif
	{ ME_PY_INT, "ME_CAP_AO_FIFO_SIZE", (long) ME_CAP_AO_FIFO_SIZE, 0 },
#ifdef ME_CAP_AO_BUFFER_SIZE
	{ ME_PY_INT, "ME_CAP_AO_BUFFER_SIZE", (long) ME_CAP_AO_BUFFER_SIZE, 0 },
#endif
	{ ME_PY_INT, "ME_CAP_CTR_WIDTH", (long) ME_CAP_CTR_WIDTH, 0 },
	{ ME_PY_INT, "ME_UNIT_INVALID", (long) ME_UNIT_INVALID, 0 },
	{ ME_PY_INT, "ME_UNIT_VOLT", (long) ME_UNIT_VOLT, 0 },
	{ ME_PY_INT, "ME_UNIT_AMPERE", (long) ME_UNIT_AMPERE, 0 },
	{ ME_PY_INT, "ME_UNIT_ANY", (long) ME_UNIT_ANY, 0 },
	{ ME_PY_INT, "ME_TYPE_INVALID", (long) ME_TYPE_INVALID, 0 },
	{ ME_PY_INT, "ME_TYPE_AO", (long) ME_TYPE_AO, 0 },
	{ ME_PY_INT, "ME_TYPE_AI", (long) ME_TYPE_AI, 0 },
	{ ME_PY_INT, "ME_TYPE_DIO", (long) ME_TYPE_DIO, 0 },
	{ ME_PY_INT, "ME_TYPE_DO", (long) ME_TYPE_DO, 0 },
	{ ME_PY_INT, "ME_TYPE_DI", (long) ME_TYPE_DI, 0 },
	{ ME_PY_INT, "ME_TYPE_CTR", (long) ME_TYPE_CTR, 0 },
	{ ME_PY_INT, "ME_TYPE_EXT_IRQ", (long) ME_TYPE_EXT_IRQ, 0 },
#ifdef ME_TYPE_FREQ_IO
	{ ME_PY_INT, "ME_TYPE_FREQ_IO", (long) ME_TYPE_FREQ_IO, 0 },
#endif
#ifdef ME_TYPE_FREQ_I
	{ ME_PY_INT, "ME_TYPE_FREQ_I", (long) ME_TYPE_FREQ_I, 0 },
#endif
#ifdef ME_TYPE_FREQ_O
	{ ME_PY_INT, "ME_TYPE_FREQ_O", (long) ME_TYPE_FREQ_O, 0 },
#endif
	{ ME_PY_INT, "ME_SUBTYPE_INVALID", (long) ME_SUBTYPE_INVALID, 0 },
	{ ME_PY_INT, "ME_SUBTYPE_SINGLE", (long) ME_SUBTYPE_SINGLE, 0 },
	{ ME_PY_INT, "ME_SUBTYPE_STREAMING", (long) ME_SUBTYPE_STREAMING, 0 },
	{ ME_PY_INT, "ME_SUBTYPE_CTR_8254", (long) ME_SUBTYPE_CTR_8254, 0 },
	{ ME_PY_INT, "ME_SUBTYPE_ANY", (long) ME_SUBTYPE_ANY, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_BIT", (long) ME_CAPS_DIO_DIR_BIT, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_BYTE", (long) ME_CAPS_DIO_DIR_BYTE, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_WORD", (long) ME_CAPS_DIO_DIR_WORD, 0 },
	{ ME_PY_INT, "ME_CAPS_DIO_DIR_DWORD", (long) ME_CAPS_DIO_DIR_DWORD, 0 },
	{ ME_PY_INT, "ME_CAPS_AI_SAMPLE_HOLD", (long) ME_CAPS_AI_SAMPLE_HOLD, 0 },
	{ ME_PY_INT, "ME_DEVICE_DRIVER_NAME_MAX_COUNT", (long) ME_DEVICE_DRIVER_NAME_MAX_COUNT, 0 },
	{ ME_PY_INT, "ME_DEVICE_NAME_MAX_COUNT", (long) ME_DEVICE_NAME_MAX_COUNT, 0 },
	{ ME_PY_INT, "ME_DEVICE_DESCRIPTION_MAX_COUNT", (long) ME_DEVICE_DESCRIPTION_MAX_COUNT, 0 },
	{ ME_PY_INT, "ME_BUS_TYPE_INVALID", (long) ME_BUS_TYPE_INVALID, 0 },
#ifdef ME_BUS_TYPE_ANY
	{ ME_PY_INT, "ME_BUS_TYPE_ANY", (long) ME_BUS_TYPE_ANY, 0 },
#endif
	{ ME_PY_INT, "ME_BUS_TYPE_PCI", (long) ME_BUS_TYPE_PCI, 0 },
	{ ME_PY_INT, "ME_BUS_TYPE_USB", (long) ME_BUS_TYPE_USB, 0 },
#ifdef ME_BUS_TYPE_LAN_PCI
	{ ME_PY_INT, "ME_BUS_TYPE_LAN_PCI", (long) ME_BUS_TYPE_LAN_PCI, 0 },
#endif
#ifdef ME_BUS_TYPE_LAN_USB
	{ ME_PY_INT, "ME_BUS_TYPE_LAN_USB", (long) ME_BUS_TYPE_LAN_USB, 0 },
#endif
	{ ME_PY_INT, "ME_PLUGGED_INVALID", (long) ME_PLUGGED_INVALID, 0 },
	{ ME_PY_INT, "ME_PLUGGED_IN", (long) ME_PLUGGED_IN, 0 },
	{ ME_PY_INT, "ME_PLUGGED_OUT", (long) ME_PLUGGED_OUT, 0 },
	{ ME_PY_INT, "ME_EXTENSION_TYPE_INVALID", (long) ME_EXTENSION_TYPE_INVALID, 0 },
	{ ME_PY_INT, "ME_EXTENSION_TYPE_NONE", (long) ME_EXTENSION_TYPE_NONE, 0 },
	{ ME_PY_INT, "ME_EXTENSION_TYPE_MUX32M", (long) ME_EXTENSION_TYPE_MUX32M, 0 },
	{ ME_PY_INT, "ME_EXTENSION_TYPE_DEMUX32", (long) ME_EXTENSION_TYPE_DEMUX32, 0 },
	{ ME_PY_INT, "ME_EXTENSION_TYPE_MUX32S", (long) ME_EXTENSION_TYPE_MUX32S, 0 },
	{ ME_PY_INT, "ME_ACCESS_TYPE_INVALID", (long) ME_ACCESS_TYPE_INVALID, 0 },
#ifdef ME_ACCESS_TYPE_ANY
	{ ME_PY_INT, "ME_ACCESS_TYPE_ANY", (long) ME_ACCESS_TYPE_ANY, 0 },
#endif
	{ ME_PY_INT, "ME_ACCESS_TYPE_LOCAL", (long) ME_ACCESS_TYPE_LOCAL, 0 },
	{ ME_PY_INT, "ME_ACCESS_TYPE_REMOTE", (long) ME_ACCESS_TYPE_REMOTE, 0 },
	{ ME_PY_INT, "ME_CONF_LOAD_CUSTOM_DRIVER", (long) ME_CONF_LOAD_CUSTOM_DRIVER, 0 },
#ifdef ME_PWM_START_NO_FLAGS
	{ ME_PY_INT, "ME_PWM_START_NO_FLAGS", (long) ME_PWM_START_NO_FLAGS, 0 },
#endif
	{ ME_PY_INT, "ME_PWM_START_CONNECT_INTERNAL", (long) ME_PWM_START_CONNECT_INTERNAL, 0 },
#ifdef ME_QUERY_NO_FLAGS
	{ ME_PY_INT, "ME_QUERY_NO_FLAGS", (long) ME_QUERY_NO_FLAGS, 0 },
#endif
#ifdef ME_ERRNO_CLEAR_FLAGS
	{ ME_PY_INT, "ME_ERRNO_CLEAR_FLAGS", (long) ME_ERRNO_CLEAR_FLAGS, 0 },
#endif
	{ 0 }
};

static PyObject * me_int_CreateError(int error){

	PyObject* objResult;		//error tuplet
	PyObject* objErrorMessage;	//Message
	PyObject* objErrorValue;	//Value

	char pcError[ME_ERROR_MSG_MAX_COUNT];

	objResult = NULL;
	if(error)
	{//Error code returned
		//Get message
		if(ME_ERRNO_SUCCESS != meErrorGetMessage(error, pcError, sizeof(pcError)))
		{//wrong error code
			error = ME_ERRNO_INVALID_ERROR_NUMBER;
		}

		//Create object
		objResult = PyTuple_New(2);
		if(objResult)
		{
			//get value
			objErrorValue = PyInt_FromLong(error);
			if(PyTuple_SetItem(objResult, 0, objErrorValue))
			{
				Py_DECREF(objResult);
				Py_DECREF(objErrorValue);
				return NULL;
			}

			//get corresponding message
			objErrorMessage = PyString_FromString(pcError);
			if(PyTuple_SetItem(objResult, 1, objErrorMessage))
			{
				Py_DECREF(objResult);
				Py_DECREF(objErrorValue);
				Py_DECREF(objErrorMessage);
				return NULL;
			}
		}
	}
	return objResult;
}

static void me_InstallConstants(PyObject *d, me_const_info_t *constants) {
	int i;
	PyObject *obj;

	for(i = 0; constants[i].type; i++){
		switch(constants[i].type){
			case ME_PY_INT:
				obj = PyInt_FromLong(constants[i].lvalue);
				break;
			case ME_PY_FLOAT:
				obj = PyFloat_FromDouble(constants[i].dvalue);
				break;
			default:
				obj = 0;
				break;
		}

		if(obj){
			PyDict_SetItemString(d, constants[i].name, obj);
			Py_DECREF(obj);
		}
	}
}


/*===========================================================================
  Functions to access the driver system
  =========================================================================*/

static PyObject *_wrap_meOpen(PyObject *self, PyObject *args) {
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"i:meOpen", &iFlags)) return NULL;

	iResult = meOpen(iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meClose(PyObject *self, PyObject *args) {
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"i:meClose", &iFlags)) return NULL;

	iResult = meClose(iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meLockDriver(PyObject *self, PyObject *args) {
	int iLock;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"ii:meLockDriver", &iLock, &iFlags)) return NULL;

	iResult = meLockDriver(iLock, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meLockDevice(PyObject *self, PyObject *args) {
	int iDevice;
	int iLock;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"iii:meLockDevice", &iDevice, &iLock, &iFlags)) return NULL;

	iResult = meLockDevice(iDevice, iLock, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meLockSubdevice(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iLock;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"iiii:meLockSubdevice", &iDevice, &iSubdevice, &iLock, &iFlags)) return NULL;

	iResult = meLockSubdevice(iDevice, iSubdevice, iLock, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


/*===========================================================================
  Functions to perform I/O on a device
  =========================================================================*/

static PyObject *_wrap_meIOIrqStart(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iChannel;
	int iIrqSource;
	int iIrqEdge;
	int iIrqArg;
	PyObject *objIrqArg;
	int iFlags;
	int iResult;
	char pcError[ME_ERROR_MSG_MAX_COUNT];

	if(!PyArg_ParseTuple(
				args,
				"iiiiiOi:meIOIrqStart",
			   	&iDevice,
			   	&iSubdevice,
			   	&iChannel,
			   	&iIrqSource,
				&iIrqEdge,
				&objIrqArg,
			   	&iFlags)) return NULL;

	if(PyInt_Check(objIrqArg)){
		iIrqArg = PyInt_AsLong(objIrqArg);
	}
	else if(PyLong_Check(objIrqArg)){
		iIrqArg = PyLong_AsUnsignedLongMask(objIrqArg);
	}
	else{
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 6",
				"Python (long) integer", objIrqArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iResult = meIOIrqStart(
			iDevice,
			iSubdevice,
			iChannel,
			iIrqSource,
			iIrqEdge,
			iIrqArg,
		   	iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOIrqStop(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iChannel;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(
				args,
				"iiii:meIOIrqStop",
			   	&iDevice,
			   	&iSubdevice,
			   	&iChannel,
			   	&iFlags)) return NULL;

	iResult = meIOIrqStop(
			iDevice,
			iSubdevice,
			iChannel,
		   	iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOIrqWait(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iChannel;
	int iIrqCount;
	int iValue;
	int iTimeOut;
	int iFlags;
	int iResult;

	PyObject *objIrqCount;
	PyObject *objValue;
	PyObject *objResult;

	if(!PyArg_ParseTuple(
				args,
				"iiiii:meIOIrqWait",
			   	&iDevice,
			   	&iSubdevice,
			   	&iChannel,
				&iTimeOut,
			   	&iFlags)) return NULL;

	iResult = meIOIrqWait(
			iDevice,
			iSubdevice,
			iChannel,
			&iIrqCount,
			&iValue,
			iTimeOut,
		   	iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	if(!(objIrqCount = PyInt_FromLong(iIrqCount))){
		return NULL;
	}

	if(iValue < 0){
		if(!(objValue = PyLong_FromUnsignedLong(iValue))){
			Py_DECREF(objIrqCount);
			return NULL;
		}
	}
	else{
		if(!(objValue = PyInt_FromLong(iValue))){
			Py_DECREF(objIrqCount);
			return NULL;
		}
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objIrqCount);
		Py_DECREF(objValue);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objIrqCount)){
		Py_DECREF(objResult);
		Py_DECREF(objIrqCount);
		Py_DECREF(objValue);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objValue)){
		Py_DECREF(objResult);
		Py_DECREF(objIrqCount);
		Py_DECREF(objValue);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meIOResetDevice(PyObject *self, PyObject *args) {
	int iDevice;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"ii:meIOResetDevice", &iDevice, &iFlags)) return NULL;

	iResult = meIOResetDevice(iDevice, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOResetSubdevice(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"iii:meIOResetSubdevice", &iDevice, &iSubdevice, &iFlags)) return NULL;

	iResult = meIOResetSubdevice(iDevice, iSubdevice, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOSingleConfig(PyObject *self, PyObject *args){
	int iDevice;
	int iSubdevice;
	int iChannel;
	int iSingleConfig;
	int iRef;
	int iTrigChan;
	int iTrigType;
	int iTrigEdge;
	int iFlags;
	int iResult;

	if(!PyArg_ParseTuple(args,"iiiiiiiii:meIOSingleConfig",
			   	&iDevice,
			   	&iSubdevice,
			   	&iChannel,
			   	&iSingleConfig,
			   	&iRef,
			   	&iTrigChan,
			   	&iTrigType,
			   	&iTrigEdge,
			   	&iFlags))
	   	return NULL;

	iResult = meIOSingleConfig(
			iDevice,
		   	iSubdevice,
		   	iChannel,
		   	iSingleConfig,
		   	iRef,
		   	iTrigChan,
		   	iTrigType,
		   	iTrigEdge,
		   	iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOSingle(PyObject *self, PyObject *args) {
	meIOSingle_t *singleList;
	int iCount;
	int iFlags;
	int iResult;
	PyObject *objSingleList;
	PyObject *objSingleEntry;
	PyObject *objSingleArg;
	char pcError[ME_ERROR_MSG_MAX_COUNT];
	int i;

	if(!PyArg_ParseTuple(args, "Oi:meIOSingle", &objSingleList, &iFlags)) return NULL;

	if(!PyList_Check(objSingleList)){
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 1", "Python list", objSingleList->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iCount = PyList_GET_SIZE(objSingleList);
	singleList = PyMem_Malloc(sizeof(meIOSingle_t) * iCount);
	if(!singleList)
		return PyErr_NoMemory();

	for(i = 0; i < iCount; i++){
		if(!(objSingleEntry = PyList_GetItem(objSingleList, i))){
			PyMem_Free(singleList);
			return NULL;
		}
		if(!PyDict_Check(objSingleEntry)){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as list item in argument 1",
					"Python dictionary",
				   	objSingleEntry->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Device */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Device"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Device' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iDevice = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Device' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Subdevice */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Subdevice"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Subdevice' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iSubdevice = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Subdevice' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Channel */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Channel"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Channel' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iChannel = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Channel' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Dir */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Dir"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Dir' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iDir = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Dir' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Value */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Value"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Value' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iValue = PyInt_AsLong(objSingleArg);
		}
		else if(PyLong_Check(objSingleArg)){
			singleList[i].iValue = PyLong_AsUnsignedLongMask(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Value' in start list entry %d",
					"Python (long) integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Time out */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "TimeOut"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'TimeOut' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iTimeOut = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'TimeOut' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Flags */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Flags"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Flags' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iFlags = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Flags' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Errno */
		if(!(objSingleArg = PyDict_GetItemString(objSingleEntry, "Errno"))){
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "Key 'Errno' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objSingleArg)){
			singleList[i].iErrno = PyInt_AsLong(objSingleArg);
		}
		else{
			PyMem_Free(singleList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Errno' in start list entry %d",
					"Python integer", objSingleArg->ob_type->tp_name, i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
	}

	iResult = meIOSingle(singleList, iCount, iFlags);
	if(iResult){
		PyMem_Free(singleList);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	for(i = 0; i < iCount; i++){
		if(singleList[i].iValue < 0){
			PyDict_SetItemString(PyList_GetItem(objSingleList, i), "Value", PyLong_FromUnsignedLong(singleList[i].iValue));
		}
		else{
			PyDict_SetItemString(PyList_GetItem(objSingleList, i), "Value", PyInt_FromLong(singleList[i].iValue));
		}
		PyDict_SetItemString(PyList_GetItem(objSingleList, i), "Errno", PyInt_FromLong(singleList[i].iErrno));
	}

	PyMem_Free(singleList);
	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOStreamConfig(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	meIOStreamConfig_t *pConfigList;
	int iCount;
	meIOStreamTrigger_t trigger;
	int iFifoIrqThreshold;
	int iFlags;
	int iResult;
	PyObject *objConfigList;
	PyObject *objConfigEntry;
	PyObject *objConfigArg;
	PyObject *objTrigger;
	char pcError[ME_ERROR_MSG_MAX_COUNT];
	int i;
	int64_t ticks;

	if(!PyArg_ParseTuple(args, "iiOOii:meIOStreamConfig", &iDevice, &iSubdevice, &objConfigList, &objTrigger, &iFifoIrqThreshold, &iFlags)) return NULL;

	if(!PyList_Check(objConfigList)){
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 3", "Python list", objConfigList->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iCount = PyList_GET_SIZE(objConfigList);
	pConfigList = PyMem_Malloc(sizeof(meIOStreamConfig_t) * iCount);
	if(!pConfigList)
		return PyErr_NoMemory();

	for(i = 0; i < iCount; i++){
		if(!(objConfigEntry = PyList_GetItem(objConfigList, i))){
			PyMem_Free(pConfigList);
			return NULL;
		}
		if(!PyDict_Check(objConfigEntry)){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as list item in argument 3",
					"Python dictionary",
				   	objConfigEntry->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Channel */
		if(!(objConfigArg = PyDict_GetItemString(objConfigEntry, "Channel"))){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "Key 'Channel' expected in config list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objConfigArg)){
			pConfigList[i].iChannel = PyInt_AsLong(objConfigArg);
		}
		else{
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Channel'",
					"Python integer", objConfigArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* StreamConfig */
		if(!(objConfigArg = PyDict_GetItemString(objConfigEntry, "StreamConfig"))){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "Key 'StreamConfig' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objConfigArg)){
			pConfigList[i].iStreamConfig = PyInt_AsLong(objConfigArg);
		}
		else{
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'StreamConfig'",
					"Python integer", objConfigArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Ref */
		if(!(objConfigArg = PyDict_GetItemString(objConfigEntry, "Ref"))){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "Key 'Ref' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objConfigArg)){
			pConfigList[i].iRef = PyInt_AsLong(objConfigArg);
		}
		else{
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Ref'",
					"Python integer", objConfigArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Flags */
		if(!(objConfigArg = PyDict_GetItemString(objConfigEntry, "Flags"))){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "Key 'Flags' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objConfigArg)){
			pConfigList[i].iFlags = PyInt_AsLong(objConfigArg);
		}
		else{
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Flags'",
					"Python integer", objConfigArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
	}

	if(!PyDict_Check(objTrigger)){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 5",
				"Python dictionary",
				objTrigger->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStartTrigType */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStartTrigType"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStartTrigType' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iAcqStartTrigType = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStartTrigType'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStartTrigEdge */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStartTrigEdge"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStartTrigEdge' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iAcqStartTrigEdge = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStartTrigEdge'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStartTrigChan */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStartTrigChan"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStartTrigChan' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iAcqStartTrigChan = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStartTrigChan'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStartTicks */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStartTicks"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStartTicks' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		ticks = PyInt_AsLong(objConfigArg);
		trigger.iAcqStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iAcqStartTicksHigh = 0;
	}
	else if(PyLong_Check(objConfigArg)){
		ticks = PyLong_AsLongLong(objConfigArg);
		trigger.iAcqStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iAcqStartTicksHigh = (ticks >> 32) & 0xFFFFFFFF;
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStartTicks'",
				"Python (long) integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ScanStartTrigType */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ScanStartTrigType"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ScanStartTrigType' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iScanStartTrigType = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ScanStartTrigType'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ScanStartTicks */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ScanStartTicks"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ScanStartTicks' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		ticks = PyInt_AsLong(objConfigArg);
		trigger.iScanStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iScanStartTicksHigh = 0;
	}
	else if(PyLong_Check(objConfigArg)){
		ticks = PyLong_AsLongLong(objConfigArg);
		trigger.iScanStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iScanStartTicksHigh = (ticks >> 32) & 0xFFFFFFFF;
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ScanStartTicks'",
				"Python (long) integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ConvStartTrigType */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ConvStartTrigType"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ConvStartTrigType' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iConvStartTrigType = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ConvStartTrigType'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ConvStartTicks */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ConvStartTicks"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ConvStartTicks' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyLong_Check(objConfigArg)){
		ticks = PyInt_AsLong(objConfigArg);
		trigger.iConvStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iConvStartTicksHigh = 0;
	}
	else if(PyLong_Check(objConfigArg)){
		ticks = PyLong_AsLongLong(objConfigArg);
		trigger.iConvStartTicksLow = ticks & 0xFFFFFFFF;
		trigger.iConvStartTicksHigh = (ticks >> 32) & 0xFFFFFFFF;
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ConvStartTicks'",
				"Python (long) integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ScanStopTrigType */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ScanStopTrigType"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ScanStopTrigType' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iScanStopTrigType = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ScanStopTrigType'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* ScanStopCount */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "ScanStopCount"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'ScanStopCount' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iScanStopCount = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'ScanStopCount'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStopTrigType */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStopTrigType"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStopTrigType' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iAcqStopTrigType = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStopTrigType'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* AcqStopCount */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "AcqStopCount"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'AcqStopCount' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iAcqStopCount = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'AcqStopCount'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	/* Flags */
	if(!(objConfigArg = PyDict_GetItemString(objTrigger, "Flags"))){
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "Key 'Flags' expected in trigger setup.");
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}
	if(PyInt_Check(objConfigArg)){
		trigger.iFlags = PyInt_AsLong(objConfigArg);
	}
	else{
		PyMem_Free(pConfigList);
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Flags'",
				"Python integer", objConfigArg->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iResult = meIOStreamConfig(iDevice, iSubdevice, pConfigList, iCount, &trigger, iFifoIrqThreshold, iFlags);
	if(iResult){
		PyMem_Free(pConfigList);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	PyMem_Free(pConfigList);
	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject *_wrap_meIOStreamRead(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iReadMode;
	int *piValues;
	int iCount;
	int iFlags;
	int iResult;
	PyArrayObject *objArray = NULL;
	int n_dimensions = 1;
	int dimensions[1];
	int type_num = PyArray_INT;

	if(!PyArg_ParseTuple(args, "iiiii:meIOStreamRead", &iDevice, &iSubdevice, &iReadMode, &iCount, &iFlags)) return NULL;

	piValues = PyMem_Malloc(sizeof(int) * iCount);
	if(!piValues)
		return PyErr_NoMemory();

	iResult = meIOStreamRead(iDevice, iSubdevice, iReadMode, piValues, &iCount, iFlags);
	if(iResult){
		PyMem_Free(piValues);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	piValues = PyMem_Realloc(piValues, sizeof(int) * iCount);

	dimensions[0] = iCount;
	if(!(objArray = (PyArrayObject *) PyArray_FromDimsAndData(n_dimensions, dimensions, type_num, (char *) piValues))){
		PyMem_Free(piValues);
		return NULL;
	}

	return PyArray_Return(objArray);
}


static PyObject *_wrap_meIOStreamWrite(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iWriteMode;
	int *piValues;
	int *piReturnValues;
	int iCount;
	int iFlags;
	int iResult;
	PyObject *objInputArray = NULL;
	PyArrayObject *objArray = NULL;
	PyArrayObject *objReturnArray = NULL;
	int n_dimensions = 1;
	int dimensions[1];
	int type_num = PyArray_INT;

	if(!PyArg_ParseTuple(args, "iiiOi:meIOStreamWrite", &iDevice, &iSubdevice, &iWriteMode, &objInputArray, &iFlags)) return NULL;

	objArray = (PyArrayObject *) PyArray_ContiguousFromObject(objInputArray, PyArray_INT, 1, 1);
	if(objArray == NULL)
		return NULL;

	iCount = objArray->dimensions[0];
	piValues = (int *) objArray->data;
	iResult = meIOStreamWrite(iDevice, iSubdevice, iWriteMode, piValues, &iCount, iFlags);
	if(iResult){

		Py_DECREF(objArray);
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	piReturnValues = PyMem_Malloc((sizeof(int) * objArray->dimensions[0]) - (sizeof(int) * iCount));
	if(!piReturnValues){
		Py_DECREF(objArray);
		return PyErr_NoMemory();
	}

	memcpy(piReturnValues, &piValues[iCount], (sizeof(int) * objArray->dimensions[0]) - (sizeof(int) * iCount));

	dimensions[0] = objArray->dimensions[0] - iCount;
	if(!(objReturnArray = (PyArrayObject *) PyArray_FromDimsAndData(n_dimensions, dimensions, type_num, (char *) piReturnValues))){
		Py_DECREF(objArray);
		PyMem_Free(piReturnValues);
		return NULL;
	}

	Py_DECREF(objArray);
	return PyArray_Return(objReturnArray);
}


static PyObject *_wrap_meIOStreamStart(PyObject *self, PyObject *args) {
	meIOStreamStart_t *startList;
	int iCount;
	int iFlags;
	int iResult;
	PyObject *objStartList;
	PyObject *objStartEntry;
	PyObject *objArg;
	PyObject *objErrno;
	char pcError[ME_ERROR_MSG_MAX_COUNT];
	int i;

	if(!PyArg_ParseTuple(args, "Oi:meIOStreamStart", &objStartList, &iFlags)) return NULL;

	if(!PyList_Check(objStartList)){
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 1", "Python list", objStartList->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iCount = PyList_GET_SIZE(objStartList);
	startList = PyMem_Malloc(sizeof(meIOStreamStart_t) * iCount);
	if(!startList)
		return PyErr_NoMemory();

	for(i = 0; i < iCount; i++){
		if(!(objStartEntry = PyList_GetItem(objStartList, i))){
			PyMem_Free(startList);
			return NULL;
		}
		if(!PyDict_Check(objStartEntry)){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as list item in argument 1",
					"Python dictionary",
				   	objStartEntry->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Device */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "Device"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'Device' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iDevice = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Device'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Subdevice */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "Subdevice"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'Subdevice' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iSubdevice = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Subdevice'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* StartMode */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "StartMode"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'StartMode' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iStartMode = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'StartMode'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Time out */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "TimeOut"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'TimeOut' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iTimeOut = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'TimeOut'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Flags */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "Flags"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'Flags' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iFlags = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Flags'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Errno */
		if(!(objArg = PyDict_GetItemString(objStartEntry, "Errno"))){
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "Key 'Errno' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			startList[i].iErrno = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(startList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Errno'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
	}

	iResult = meIOStreamStart(startList, iCount, iFlags);
	if(iResult){
		PyMem_Free(startList);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	for(i = 0; i < iCount; i++){
		PyDict_SetItemString(PyList_GetItem(objStartList, i), "Errno", objErrno = PyInt_FromLong(startList[i].iErrno));
	}

	PyMem_Free(startList);
	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOStreamStop(PyObject *self, PyObject *args) {
	meIOStreamStop_t *stopList;
	int iCount;
	int iFlags;
	int iResult;
	PyObject *objStopList;
	PyObject *objStopEntry;
	PyObject *objArg;
	PyObject *objErrno;
	char pcError[ME_ERROR_MSG_MAX_COUNT];
	int i;

	if(!PyArg_ParseTuple(args, "Oi:meIOStreamStop", &objStopList, &iFlags)) return NULL;

	if(!PyList_Check(objStopList)){
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 1", "Python list", objStopList->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		return NULL;
	}

	iCount = PyList_GET_SIZE(objStopList);
	stopList = PyMem_Malloc(sizeof(meIOStreamStop_t) * iCount);
	if(!stopList)
		return PyErr_NoMemory();

	for(i = 0; i < iCount; i++){
		if(!(objStopEntry = PyList_GetItem(objStopList, i))){
			PyMem_Free(stopList);
			return NULL;
		}
		if(!PyDict_Check(objStopEntry)){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as list item in argument 1",
					"Python dictionary",
				   	objStopEntry->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Device */
		if(!(objArg = PyDict_GetItemString(objStopEntry, "Device"))){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "Key 'Device' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			stopList[i].iDevice = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Device'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Subdevice */
		if(!(objArg = PyDict_GetItemString(objStopEntry, "Subdevice"))){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "Key 'Subdevice' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			stopList[i].iSubdevice = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Subdevice'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* StopMode */
		if(!(objArg = PyDict_GetItemString(objStopEntry, "StopMode"))){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "Key 'StopMode' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			stopList[i].iStopMode = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'StopMode'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Flags */
		if(!(objArg = PyDict_GetItemString(objStopEntry, "Flags"))){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "Key 'Flags' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			stopList[i].iFlags = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Flags'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}

		/* Errno */
		if(!(objArg = PyDict_GetItemString(objStopEntry, "Errno"))){
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "Key 'Errno' expected in start list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objArg)){
			stopList[i].iErrno = PyInt_AsLong(objArg);
		}
		else{
			PyMem_Free(stopList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Errno'",
					"Python integer", objArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
	}

	iResult = meIOStreamStop(stopList, iCount, iFlags);
	if(iResult){
		PyMem_Free(stopList);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	for(i = 0; i < iCount; i++){
		PyDict_SetItemString(PyList_GetItem(objStopList, i), "Errno", objErrno = PyInt_FromLong(stopList[i].iErrno));
	}

	PyMem_Free(stopList);
	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meIOStreamStatus(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iWait;
	int iStatus;
	int iCount;
	int iFlags;
	int iResult;
	PyObject *objStatus;
	PyObject *objCount;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "iiii:meIOStreamStatus", &iDevice, &iSubdevice, &iWait, &iFlags)) return NULL;

	iResult = meIOStreamStatus(iDevice, iSubdevice, iWait, &iStatus, &iCount, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objStatus = PyInt_FromLong(iStatus);
	if(!objStatus){
		return NULL;
	}
	objCount = PyInt_FromLong(iCount);
	if(!objCount){
		Py_DECREF(objStatus);
		return NULL;
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objStatus);
		Py_DECREF(objCount);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objStatus)){
		Py_DECREF(objResult);
		Py_DECREF(objStatus);
		Py_DECREF(objCount);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objCount)){
		Py_DECREF(objResult);
		Py_DECREF(objStatus);
		Py_DECREF(objCount);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meIOStreamFrequencyToTicks(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iTimer;
	double dFrequency;
	int iTicksLow;
	int iTicksHigh;
	int64_t iTicks;
	int iFlags;
	int iResult;
	PyObject *objFrequency;
	PyObject *objTicks;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "iiidi:meIOStreamFrequencyToTicks", &iDevice, &iSubdevice, &iTimer, &dFrequency, &iFlags)) return NULL;

	iResult = meIOStreamFrequencyToTicks(iDevice, iSubdevice, iTimer, &dFrequency, &iTicksLow, &iTicksHigh, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objFrequency = PyFloat_FromDouble(dFrequency);
	if(!objFrequency){
		return NULL;
	}
	iTicks = (int64_t) iTicksLow + ((int64_t) iTicksHigh << 32);
	objTicks = PyLong_FromUnsignedLongLong(iTicks);
	if(!objTicks){
		Py_DECREF(objFrequency);
		return NULL;
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objFrequency);
		Py_DECREF(objTicks);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objFrequency)){
		Py_DECREF(objFrequency);
		Py_DECREF(objTicks);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objTicks)){
		Py_DECREF(objFrequency);
		Py_DECREF(objTicks);
		Py_DECREF(objResult);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meIOStreamTimeToTicks(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iTimer;
	double dTime;
	int iTicksLow;
	int iTicksHigh;
	int64_t iTicks;
	int iFlags;
	int iResult;
	PyObject *objTime;
	PyObject *objTicks;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "iiidi:meIOStreamTimeToTicks", &iDevice, &iSubdevice, &iTimer, &dTime, &iFlags)) return NULL;

	iResult = meIOStreamTimeToTicks(iDevice, iSubdevice, iTimer, &dTime, &iTicksLow, &iTicksHigh, iFlags);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objTime = PyFloat_FromDouble(dTime);
	if(!objTime){
		return NULL;
	}
	iTicks = (int64_t) iTicksLow + ((int64_t) iTicksHigh >> 32);
	objTicks = PyLong_FromUnsignedLongLong(iTicks);
	if(!objTicks){
		Py_DECREF(objTime);
		return NULL;
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objTime);
		Py_DECREF(objTicks);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objTime)){
		Py_DECREF(objTime);
		Py_DECREF(objTicks);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objTicks)){
		Py_DECREF(objTime);
		Py_DECREF(objTicks);
		Py_DECREF(objResult);
		return NULL;
	}

	return objResult;
}


/*===========================================================================
  Functions to query the driver system
  =========================================================================*/

static PyObject *_wrap_meQueryDescriptionDevice(PyObject *self, PyObject *args) {
	int iDevice;
	char pcDescription[ME_DEVICE_DESCRIPTION_MAX_COUNT];
	int iResult;
	PyObject *objDescription;

	if(!PyArg_ParseTuple(args, "i:meQueryDescriptionDevice", &iDevice)) return NULL;
	iResult = meQueryDescriptionDevice(iDevice, pcDescription, sizeof(pcDescription));
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objDescription = PyString_FromString(pcDescription);
	if(!objDescription){
		return NULL;
	}
	return objDescription;
}


static PyObject *_wrap_meQueryInfoDevice(PyObject *self, PyObject *args) {
	int iDevice;
	int iVendorId;
	int iDeviceId;
	int iSerialNo;
	int iBusType;
	int iBusNo;
	int iDevNo;
	int iFuncNo;
	int iPlugged;
	int iResult;
	PyObject *objVendorId;
	PyObject *objDeviceId;
	PyObject *objSerialNo;
	PyObject *objBusType;
	PyObject *objBusNo;
	PyObject *objDevNo;
	PyObject *objFuncNo;
	PyObject *objPlugged;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "i:meQueryInfoDevice", &iDevice)) return NULL;
	iResult = meQueryInfoDevice(iDevice, &iVendorId, &iDeviceId, &iSerialNo, &iBusType, &iBusNo, &iDevNo, &iFuncNo, &iPlugged);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objVendorId = PyInt_FromLong(iVendorId);
	if(!objVendorId){
		return NULL;
	}
	objDeviceId = PyInt_FromLong(iDeviceId);
	if(!objDeviceId){
		Py_DECREF(objVendorId);
		return NULL;
	}
	objSerialNo = PyInt_FromLong(iSerialNo);
	if(!objSerialNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		return NULL;
	}
	objBusType = PyInt_FromLong(iBusType);
	if(!objBusType){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		return NULL;
	}
	objBusNo = PyInt_FromLong(iBusNo);
	if(!objBusNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		return NULL;
	}
	objDevNo = PyInt_FromLong(iDevNo);
	if(!objDevNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objBusNo);
		return NULL;
	}
	objFuncNo = PyInt_FromLong(iFuncNo);
	if(!objFuncNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		return NULL;
	}
	objPlugged = PyInt_FromLong(iPlugged);
	if(!objPlugged){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		return NULL;
	}

	objResult = PyTuple_New(8);
	if(!objResult){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		return NULL;
	}

	if(PyTuple_SetItem(objResult, 0, objVendorId)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objDeviceId)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 2, objSerialNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 3, objBusType)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 4, objBusNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 5, objDevNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 6, objFuncNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 7, objPlugged)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meQueryNameDevice(PyObject *self, PyObject *args) {
	int iDevice;
	char pcName[ME_DEVICE_NAME_MAX_COUNT];
	int iResult;
	PyObject *objName;

	if(!PyArg_ParseTuple(args, "i:meQueryNameDevice", &iDevice)) return NULL;
	iResult = meQueryNameDevice(iDevice, pcName, sizeof(pcName));
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objName = PyString_FromString(pcName);
	if(!objName){
		return NULL;
	}
	return objName;
}


static PyObject *_wrap_meQueryNameDeviceDriver(PyObject *self, PyObject *args) {
	int iDevice;
	char pcName[ME_DEVICE_DRIVER_NAME_MAX_COUNT];
	int iResult;
	PyObject *objName;

	if(!PyArg_ParseTuple(args, "i:meQueryNameDeviceDriver", &iDevice)) return NULL;
	iResult = meQueryNameDeviceDriver(iDevice, pcName, sizeof(pcName));
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objName = PyString_FromString(pcName);
	if(!objName){
		return NULL;
	}
	return objName;
}


static PyObject *_wrap_meQueryNumberDevices(PyObject *self, PyObject *args) {
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, ":meQueryNumberDevices")) return NULL;
	iResult = meQueryNumberDevices(&iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meQueryNumberSubdevices(PyObject *self, PyObject *args) {
	int iDevice;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "i:meQueryNumberSubdevices", &iDevice)) return NULL;
	iResult = meQueryNumberSubdevices(iDevice, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meQueryNumberChannels(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "ii:meQueryNumberChannels", &iDevice, &iSubdevice)) return NULL;
	iResult = meQueryNumberChannels(iDevice, iSubdevice, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meQueryNumberRanges(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iUnit;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "iii:meQueryNumberRanges", &iDevice, &iSubdevice, &iUnit)) return NULL;
	iResult = meQueryNumberRanges(iDevice, iSubdevice, iUnit, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meQueryRangeByMinMax(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	int iRange;
	int iResult;
	PyObject *objMin;
	PyObject *objMax;
	PyObject *objMaxData;
	PyObject *objRange;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "iiidd:meIOQueryRangeByMinMax", &iDevice, &iSubdevice, &iUnit, &dMin, &dMax)) return NULL;
	iResult = meQueryRangeByMinMax(iDevice, iSubdevice, iUnit, &dMin, &dMax, &iMaxData, &iRange);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objMin = PyFloat_FromDouble(dMin);
	if(!objMin){
		return NULL;
	}
	objMax = PyFloat_FromDouble(dMax);
	if(!objMax){
		Py_DECREF(objMin);
		return NULL;
	}
	objMaxData = PyInt_FromLong(iMaxData);
	if(!objMaxData){
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		return NULL;
	}
	objRange = PyInt_FromLong(iRange);
	if(!objRange){
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}

	objResult = PyTuple_New(4);
	if(!objResult){
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		Py_DECREF(objRange);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objMin)){
		Py_DECREF(objResult);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		Py_DECREF(objRange);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objMax)){
		Py_DECREF(objResult);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		Py_DECREF(objRange);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 2, objMaxData)){
		Py_DECREF(objResult);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		Py_DECREF(objRange);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 3, objRange)){
		Py_DECREF(objResult);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		Py_DECREF(objRange);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meQueryRangeInfo(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iRange;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	int iResult;
	PyObject *objUnit;
	PyObject *objMin;
	PyObject *objMax;
	PyObject *objMaxData;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "iii:meIOQueryRangeInfo", &iDevice, &iSubdevice, &iRange)) return NULL;
	iResult = meQueryRangeInfo(iDevice, iSubdevice, iRange, &iUnit, &dMin, &dMax, &iMaxData);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objUnit = PyInt_FromLong(iUnit);
	if(!objUnit){
		return NULL;
	}
	objMin = PyFloat_FromDouble(dMin);
	if(!objMin){
		Py_DECREF(objUnit);
		return NULL;
	}
	objMax = PyFloat_FromDouble(dMax);
	if(!objMax){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		return NULL;
	}
	objMaxData = PyInt_FromLong(iMaxData);
	if(!objMaxData){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		return NULL;
	}

	objResult = PyTuple_New(4);
	if(!objResult){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objUnit)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objMin)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 2, objMax)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 3, objMaxData)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meQuerySubdeviceByType(PyObject *self, PyObject *args) {
	int iDevice;
	int iStartSubdevice;
	int iType;
	int iSubtype;
	int iSubdevice;
	int iResult;
	PyObject *objSubdevice;

	if(!PyArg_ParseTuple(args, "iiii:meQuerySubdeviceByType", &iDevice, &iStartSubdevice, &iType, &iSubtype)) return NULL;
	iResult = meQuerySubdeviceByType(iDevice, iStartSubdevice, iType, iSubtype, &iSubdevice);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objSubdevice = PyInt_FromLong(iSubdevice);
	if(!objSubdevice){
		return NULL;
	}
	return objSubdevice;
}


static PyObject *_wrap_meQuerySubdeviceType(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iType;
	int iSubtype;
	int iResult;
	PyObject *objType;
	PyObject *objSubtype;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "ii:meQuerySubdeviceType", &iDevice, &iSubdevice)) return NULL;
	iResult = meQuerySubdeviceType(iDevice, iSubdevice, &iType, &iSubtype);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objType = PyInt_FromLong(iType);
	if(!objType){
		return NULL;
	}
	objSubtype = PyInt_FromLong(iSubtype);
	if(!objSubtype){
		Py_DECREF(objType);
		return NULL;
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objType)){
		Py_DECREF(objResult);
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objSubtype)){
		Py_DECREF(objResult);
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meQuerySubdeviceCaps(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iCaps;
	int iResult;
	PyObject *objCaps;

	if(!PyArg_ParseTuple(args, "ii:meQuerySubdeviceCaps", &iDevice, &iSubdevice)) return NULL;
	iResult = meQuerySubdeviceCaps(iDevice, iSubdevice, &iCaps);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objCaps = PyInt_FromLong(iCaps);
	if(!objCaps){
		return NULL;
	}

	return objCaps;
}


static PyObject *_wrap_meQuerySubdeviceCapsArgs(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iCap;
	int iArgs[8];
	int iCount = 8;
	int iResult;
	PyObject *objTuple;
	PyObject *objArg;
	int i;
	int err;

	if(!PyArg_ParseTuple(args, "iii:meQuerySubdeviceCapsArgs", &iDevice, &iSubdevice, &iCap)) return NULL;

	switch(iCap){
		case ME_CAP_AI_FIFO_SIZE:
		case ME_CAP_AO_FIFO_SIZE:
		case ME_CAP_CTR_WIDTH:
			iCount = 1;
			break;
		default:
			break;
	}

	iResult = meQuerySubdeviceCapsArgs(iDevice, iSubdevice, iCap, iArgs, iCount);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objTuple = PyTuple_New(iCount);
	if(!objTuple){
		return NULL;
	}

	for(i = 0; i < iCount; i++){
		objArg = PyInt_FromLong(iArgs[i]);
		if(!objArg){
			Py_DECREF(objTuple);
			return NULL;
		}
		err = PyTuple_SetItem(objTuple, i, objArg);
		if(err){
			Py_DECREF(objTuple);
			return NULL;
		}
	}

	return objTuple;
}


static PyObject *_wrap_meQueryVersionLibrary(PyObject *self, PyObject *args) {
	int iVersion;
	int iResult;
	PyObject *objVersion;

	if(!PyArg_ParseTuple(args, ":meQueryVersionLibrary")) return NULL;
	iResult = meQueryVersionLibrary(&iVersion);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objVersion = PyInt_FromLong(iVersion);
	if(!objVersion){
		return NULL;
	}
	return objVersion;
}


static PyObject *_wrap_meQueryVersionMainDriver(PyObject *self, PyObject *args) {
	int iVersion;
	int iResult;
	PyObject *objVersion;

	if(!PyArg_ParseTuple(args, ":meQueryVersionMainDriver")) return NULL;
	iResult = meQueryVersionMainDriver(&iVersion);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objVersion = PyInt_FromLong(iVersion);
	if(!objVersion){
		return NULL;
	}
	return objVersion;
}


static PyObject *_wrap_meQueryVersionDeviceDriver(PyObject *self, PyObject *args) {
	int iDevice;
	int iVersion;
	int iResult;
	PyObject *objVersion;

	if(!PyArg_ParseTuple(args, "i:meQueryVersionDeviceDriver", &iDevice)) return NULL;
	iResult = meQueryVersionDeviceDriver(iDevice, &iVersion);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objVersion = PyInt_FromLong(iVersion);
	if(!objVersion){
		return NULL;
	}
	return objVersion;
}


/*===========================================================================
  Common utility functions
  =========================================================================*/

static PyObject *_wrap_meUtilityExtractValues(PyObject *self, PyObject *args) {
	int iChannel;
	int *piAIBuffer;
	int iAIBufferCount;
	meIOStreamConfig_t *pConfigList;
	int iConfigListCount;
	int *piChanBuffer;
	int iChanBufferCount;
	int iResult;
	PyObject *objAIBuffer;
	PyArrayObject *objAIArray;
	PyObject *objConfigList;
	PyObject *objConfigEntry;
	PyObject *objConfigArg;
	PyArrayObject *objReturnArray = NULL;
	int n_dimensions = 1;
	int dimensions[1];
	int type_num = PyArray_INT;
	char pcError[ME_ERROR_MSG_MAX_COUNT];
	int i;

	if(!PyArg_ParseTuple(args, "iOO:meUtilityExtractValues", &iChannel, &objAIBuffer, &objConfigList)) return NULL;

	objAIArray = (PyArrayObject *) PyArray_ContiguousFromObject(objAIBuffer, PyArray_INT, 1, 1);
	if(objAIArray == NULL)
		return NULL;

	if(!PyList_Check(objConfigList)){
		snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument 3", "Python list", objConfigList->ob_type->tp_name);
		PyErr_SetString(PyExc_TypeError, pcError);
		Py_DECREF(objAIArray);
		return NULL;
	}

	piAIBuffer = (int *) objAIArray->data;
	iAIBufferCount = objAIArray->dimensions[0];

	iConfigListCount = PyList_GET_SIZE(objConfigList);
	pConfigList = PyMem_Malloc(sizeof(meIOStreamConfig_t) * iConfigListCount);
	if(!pConfigList){
		Py_DECREF(objAIArray);
		return PyErr_NoMemory();
	}

	for(i = 0; i < iConfigListCount; i++){
		if(!(objConfigEntry = PyList_GetItem(objConfigList, i))){
			PyMem_Free(pConfigList);
			Py_DECREF(objAIArray);
			return NULL;
		}
		if(!PyDict_Check(objConfigEntry)){
			PyMem_Free(pConfigList);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as list item in argument 3",
					"Python dictionary",
				   	objConfigEntry->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			Py_DECREF(objAIArray);
			return NULL;
		}

		if(!(objConfigArg = PyDict_GetItemString(objConfigEntry, "Channel"))){
			PyMem_Free(pConfigList);
			Py_DECREF(objAIArray);
			snprintf(pcError, sizeof(pcError), "Key 'Channel' expected in config list entry %d", i);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
		if(PyInt_Check(objConfigArg)){
			pConfigList[i].iChannel = PyInt_AsLong(objConfigArg);
		}
		else{
			PyMem_Free(pConfigList);
			Py_DECREF(objAIArray);
			snprintf(pcError, sizeof(pcError), "'%s' type expected (not '%s') as argument for 'Channel'",
					"Python integer", objConfigArg->ob_type->tp_name);
			PyErr_SetString(PyExc_TypeError, pcError);
			return NULL;
		}
	}

	iChanBufferCount = iAIBufferCount;
	piChanBuffer = PyMem_Malloc(sizeof(int) * iChanBufferCount);
	if(!piChanBuffer){
		PyMem_Free(pConfigList);
		Py_DECREF(objAIArray);
		return PyErr_NoMemory();
	}

	iResult = meUtilityExtractValues(iChannel, piAIBuffer, iAIBufferCount, pConfigList, iConfigListCount, piChanBuffer, &iChanBufferCount);
	if(iResult){
		PyMem_Free(pConfigList);
		PyMem_Free(piChanBuffer);
		Py_DECREF(objAIArray);

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	piChanBuffer = PyMem_Realloc(piChanBuffer, sizeof(int) * iChanBufferCount);
	if(!piChanBuffer){
		PyMem_Free(pConfigList);
		Py_DECREF(objAIArray);
		return PyErr_NoMemory();
	}

	dimensions[0] = iChanBufferCount;
	if(!(objReturnArray = (PyArrayObject *) PyArray_FromDimsAndData(n_dimensions, dimensions, type_num, (char *) piChanBuffer))){
		PyMem_Free(pConfigList);
		PyMem_Free(piChanBuffer);
		Py_DECREF(objAIArray);
		return NULL;
	}

	PyMem_Free(pConfigList);
	PyMem_Free(piChanBuffer);
	Py_DECREF(objAIArray);
	return PyArray_Return(objReturnArray);
}


static PyObject *_wrap_meUtilityPhysicalToDigital(PyObject *self, PyObject *args) {
	double dMin;
	double dMax;
	int iMaxData;
	double dPhysical;
	int iData;
	int iResult;
	PyObject *objData;

	if(!PyArg_ParseTuple(args, "ddid:meUtilityPhysicalToDigital", &dMin, &dMax, &iMaxData, &dPhysical)) return NULL;
	iResult = meUtilityPhysicalToDigital(dMin, dMax, iMaxData, dPhysical, &iData);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objData = PyInt_FromLong(iData);
	if(!objData){
		return NULL;
	}
	return objData;
}


static PyObject *_wrap_meUtilityDigitalToPhysical(PyObject *self, PyObject *args) {
	double dMin;
	double dMax;
	int iMaxData;
	int iData;
	int iModuleType;
	double dRefValue;
	double dPhysical;
	int iResult;
	PyObject *objPhysical;

	if(!PyArg_ParseTuple(args, "ddiiid:meUtilityDigitalToPhysical", &dMin, &dMax, &iMaxData, &iData, &iModuleType, &dRefValue)) return NULL;
	iResult = meUtilityDigitalToPhysical(dMin, dMax, iMaxData, iData, iModuleType, dRefValue, &dPhysical);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objPhysical = PyFloat_FromDouble(dPhysical);
	if(!objPhysical){
		return NULL;
	}
	return objPhysical;
}


static PyObject *_wrap_meUtilityPWMStart(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice1;
	int iSubdevice2;
	int iSubdevice3;
	int iRef;
	int iPrescaler;
	int iDutyCycle;
	int iFlag;
	int iResult;

	if(!PyArg_ParseTuple(args, "iiiiiiii:meUtilityPWMStart", &iDevice, &iSubdevice1, &iSubdevice2, &iSubdevice3, &iRef, &iPrescaler, &iDutyCycle, &iFlag)) return NULL;
	iResult = meUtilityPWMStart(iDevice, iSubdevice1, iSubdevice2, iSubdevice3, iRef, iPrescaler, iDutyCycle, iFlag);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meUtilityPWMStop(PyObject *self, PyObject *args) {
	int iDevice;
	int iSubdevice;
	int iResult;

	if(!PyArg_ParseTuple(args, "ii:meUtilityPWMStop", &iDevice, &iSubdevice)) return NULL;
	iResult = meUtilityPWMStop(iDevice, iSubdevice);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *_wrap_meUtilityPeriodToTicks(PyObject *self, PyObject *args) {
	int iBaseFreq;
	double dPeriod;
	unsigned int iTicks;
	int iResult;
	PyObject *objTicks;

	if(!PyArg_ParseTuple(args, "id:meUtilityPeriodToTicks", &iBaseFreq, &dPeriod)) return NULL;
	iResult = meUtilityPeriodToTicks(iBaseFreq, dPeriod, &iTicks);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objTicks = PyInt_FromLong(iTicks);
	if(!objTicks){
		return NULL;
	}
	return objTicks;
}


static PyObject *_wrap_meUtilityTicksToPeriod(PyObject *self, PyObject *args) {
	int iBaseFreq;
	unsigned int iTicks;
	double dPeriod;
	int iResult;
	PyObject *objPeriod;

	if(!PyArg_ParseTuple(args, "ii:meUtilityTicksToPeriod", &iBaseFreq, &iTicks)) return NULL;
	iResult = meUtilityTicksToPeriod(iBaseFreq, iTicks, &dPeriod);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objPeriod = PyFloat_FromDouble(dPeriod);
	if(!objPeriod){
		return NULL;
	}
	return objPeriod;
}


static PyObject *_wrap_meUtilityFrequencyToTicks(PyObject *self, PyObject *args) {
	int iBaseFreq;
	double dFrequency;
	unsigned int iTicks;
	int iResult;
	PyObject *objTicks;

	if(!PyArg_ParseTuple(args, "id:meUtilityFrequencyToTicks", &iBaseFreq, &dFrequency)) return NULL;
	iResult = meUtilityFrequencyToTicks(iBaseFreq, dFrequency, &iTicks);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objTicks = PyInt_FromLong(iTicks);
	if(!objTicks){
		return NULL;
	}
	return objTicks;
}


static PyObject *_wrap_meUtilityTicksToFrequency(PyObject *self, PyObject *args) {
	int iBaseFreq;
	unsigned int iTicks;
	double dFrequency;
	int iResult;
	PyObject *objFrequency;

	if(!PyArg_ParseTuple(args, "ii:meUtilityTicksToFrequency", &iBaseFreq, &iTicks)) return NULL;
	iResult = meUtilityTicksToFrequency(iBaseFreq, iTicks, &dFrequency);
	if(iResult){
		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objFrequency = PyFloat_FromDouble(dFrequency);
	if(!objFrequency){
		return NULL;
	}
	return objFrequency;
}


static PyObject *_wrap_meUtilityCodeDivider(PyObject *self, PyObject *args) {
	double dDivider;
	unsigned int iDivider;
	int iResult;
	PyObject *objDivider;

	if(!PyArg_ParseTuple(args, "d:meUtilityCodeDivider", &dDivider)) return NULL;
	iResult = meUtilityCodeDivider(dDivider, &iDivider);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objDivider = PyInt_FromLong(iDivider);
	if(!objDivider){
		return NULL;
	}
	return objDivider;
}


static PyObject *_wrap_meUtilityDecodeDivider(PyObject *self, PyObject *args) {
	unsigned int iDivider;
	double dDivider;
	int iResult;
	PyObject *objDivider;

	if(!PyArg_ParseTuple(args, "i:meUtilityDecodeDivider", &iDivider)) return NULL;
	iResult = meUtilityDecodeDivider(iDivider, &dDivider);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objDivider = PyFloat_FromDouble(dDivider);
	if(!objDivider){
		return NULL;
	}
	return objDivider;
}


/*===========================================================================
  Load configuration from file into driver system
  =========================================================================*/


static PyObject *_wrap_meConfigLoad(PyObject *self, PyObject *args) {
	char *pcFile;
	int iResult;

	if(!PyArg_ParseTuple(args, "z:meConfigLoad", &pcFile)) return NULL;
	iResult = meConfigLoad(pcFile);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


/*===========================================================================
  Functions to query a remote driver system
  =========================================================================*/

static PyObject *_wrap_meRQueryDescriptionDevice(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	char pcDescription[ME_DEVICE_DESCRIPTION_MAX_COUNT];
	int iResult;
	PyObject *objDescription;

	if(!PyArg_ParseTuple(args, "si:meRQueryDescriptionDevice", &pcHost, &iDevice)) return NULL;
	iResult = meRQueryDescriptionDevice(pcHost, iDevice, pcDescription, sizeof(pcDescription));
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objDescription = PyString_FromString(pcDescription);
	if(!objDescription){
		return NULL;
	}
	return objDescription;
}


static PyObject *_wrap_meRQueryInfoDevice(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iVendorId;
	int iDeviceId;
	int iSerialNo;
	int iBusType;
	int iBusNo;
	int iDevNo;
	int iFuncNo;
	int iPlugged;
	int iResult;
	PyObject *objVendorId;
	PyObject *objDeviceId;
	PyObject *objSerialNo;
	PyObject *objBusType;
	PyObject *objBusNo;
	PyObject *objDevNo;
	PyObject *objFuncNo;
	PyObject *objPlugged;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "si:meRQueryNumberSubdevices", &pcHost, &iDevice)) return NULL;
	iResult = meRQueryInfoDevice(pcHost, iDevice, &iVendorId, &iDeviceId, &iSerialNo, &iBusType, &iBusNo, &iDevNo, &iFuncNo, &iPlugged);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objVendorId = PyInt_FromLong(iVendorId);
	if(!objVendorId){
		return NULL;
	}
	objDeviceId = PyInt_FromLong(iDeviceId);
	if(!objDeviceId){
		Py_DECREF(objVendorId);
		return NULL;
	}
	objSerialNo = PyInt_FromLong(iSerialNo);
	if(!objSerialNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		return NULL;
	}
	objBusType = PyInt_FromLong(iBusType);
	if(!objBusType){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		return NULL;
	}
	objBusNo = PyInt_FromLong(iBusNo);
	if(!objBusNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		return NULL;
	}
	objDevNo = PyInt_FromLong(iDevNo);
	if(!objDevNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objBusNo);
		return NULL;
	}
	objFuncNo = PyInt_FromLong(iFuncNo);
	if(!objFuncNo){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		return NULL;
	}
	objPlugged = PyInt_FromLong(iPlugged);
	if(!objPlugged){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		return NULL;
	}

	objResult = PyTuple_New(8);
	if(!objResult){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		return NULL;
	}

	if(PyTuple_SetItem(objResult, 0, objVendorId)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objDeviceId)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 2, objSerialNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 3, objBusType)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 4, objBusNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 5, objDevNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 6, objFuncNo)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 7, objPlugged)){
		Py_DECREF(objVendorId);
		Py_DECREF(objDeviceId);
		Py_DECREF(objSerialNo);
		Py_DECREF(objBusType);
		Py_DECREF(objDevNo);
		Py_DECREF(objFuncNo);
		Py_DECREF(objPlugged);
		Py_DECREF(objResult);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meRQueryNameDevice(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	char pcName[ME_DEVICE_NAME_MAX_COUNT];
	int iResult;
	PyObject *objName;

	if(!PyArg_ParseTuple(args, "si:meRQueryNameDevice", &pcHost, &iDevice)) return NULL;
	iResult = meRQueryNameDevice(pcHost, iDevice, pcName, sizeof(pcName));
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objName = PyString_FromString(pcName);
	if(!objName){
		return NULL;
	}
	return objName;
}


static PyObject *_wrap_meRQueryNumberDevices(PyObject *self, PyObject *args) {
	char *pcHost;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "s:meRQueryNumberDevices", &pcHost)) return NULL;
	iResult = meRQueryNumberDevices(pcHost, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meRQueryNumberSubdevices(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "si:meRQueryNumberSubdevices", &pcHost, &iDevice)) return NULL;
	iResult = meRQueryNumberSubdevices(pcHost, iDevice, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meRQueryNumberChannels(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iSubdevice;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	if(!PyArg_ParseTuple(args, "sii:meRQueryNumberChannels", &pcHost, &iDevice, &iSubdevice)) return NULL;
	iResult = meRQueryNumberChannels(pcHost, iDevice, iSubdevice, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meRQueryNumberRanges(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iSubdevice;
	int iUnit;
	int iNumber;
	int iResult;
	PyObject *objNumber;

	iNumber = 0;
	if(!PyArg_ParseTuple(args, "siii:meRQueryNumberRanges", &pcHost, &iDevice, &iSubdevice, &iUnit)) return NULL;
	iResult = meRQueryNumberRanges(pcHost, iDevice, iSubdevice, iUnit, &iNumber);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objNumber = PyInt_FromLong(iNumber);
	if(!objNumber){
		return NULL;
	}
	return objNumber;
}


static PyObject *_wrap_meRQueryRangeInfo(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iSubdevice;
	int iRange;
	int iUnit;
	double dMin;
	double dMax;
	int iMaxData;
	int iResult;
	PyObject *objUnit;
	PyObject *objMin;
	PyObject *objMax;
	PyObject *objMaxData;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "siii:meIORQueryRangeInfo", &pcHost, &iDevice, &iSubdevice, &iRange)) return NULL;
	iResult = meRQueryRangeInfo(pcHost, iDevice, iSubdevice, iRange, &iUnit, &dMin, &dMax, &iMaxData);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objUnit = PyInt_FromLong(iUnit);
	if(!objUnit){
		return NULL;
	}
	objMin = PyFloat_FromDouble(dMin);
	if(!objMin){
		Py_DECREF(objUnit);
		return NULL;
	}
	objMax = PyFloat_FromDouble(dMax);
	if(!objMax){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		return NULL;
	}
	objMaxData = PyInt_FromLong(iMaxData);
	if(!objMaxData){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		return NULL;
	}

	objResult = PyTuple_New(4);
	if(!objResult){
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objUnit)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objMin)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 2, objMax)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 3, objMaxData)){
		Py_DECREF(objResult);
		Py_DECREF(objUnit);
		Py_DECREF(objMin);
		Py_DECREF(objMax);
		Py_DECREF(objMaxData);
		return NULL;
	}

	return objResult;
}


static PyObject *_wrap_meRQuerySubdeviceType(PyObject *self, PyObject *args) {
	char *pcHost;
	int iDevice;
	int iSubdevice;
	int iType;
	int iSubtype;
	int iResult;
	PyObject *objType;
	PyObject *objSubtype;
	PyObject *objResult;

	if(!PyArg_ParseTuple(args, "sii:meRQuerySubdeviceType", &pcHost, &iDevice, &iSubdevice)) return NULL;
	iResult = meRQuerySubdeviceType(pcHost, iDevice, iSubdevice, &iType, &iSubtype);
	if(iResult){

		PyErr_SetObject(meError, me_int_CreateError(iResult));
		return NULL;
	}

	objType = PyInt_FromLong(iType);
	if(!objType){
		return NULL;
	}
	objSubtype = PyInt_FromLong(iSubtype);
	if(!objSubtype){
		Py_DECREF(objType);
		return NULL;
	}

	objResult = PyTuple_New(2);
	if(!objResult){
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 0, objType)){
		Py_DECREF(objResult);
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}
	if(PyTuple_SetItem(objResult, 1, objSubtype)){
		Py_DECREF(objResult);
		Py_DECREF(objType);
		Py_DECREF(objSubtype);
		return NULL;
	}

	return objResult;
}


/*===========================================================================
  Method table
  =========================================================================*/

static PyMethodDef meMethods[] = {
	{ "meOpen", _wrap_meOpen, METH_VARARGS },
	{ "meClose", _wrap_meClose, METH_VARARGS },
	{ "meLockDriver", _wrap_meLockDriver, METH_VARARGS },
	{ "meLockDevice", _wrap_meLockDevice, METH_VARARGS },
	{ "meLockSubdevice", _wrap_meLockSubdevice, METH_VARARGS },
	{ "meIOIrqStart", _wrap_meIOIrqStart, METH_VARARGS },
	{ "meIOIrqStop", _wrap_meIOIrqStop, METH_VARARGS },
	{ "meIOIrqWait", _wrap_meIOIrqWait, METH_VARARGS },
	{ "meIOResetDevice", _wrap_meIOResetDevice, METH_VARARGS },
	{ "meIOResetSubdevice", _wrap_meIOResetSubdevice, METH_VARARGS },
	{ "meIOSingleConfig", _wrap_meIOSingleConfig, METH_VARARGS },
	{ "meIOSingle", _wrap_meIOSingle, METH_VARARGS },
	{ "meIOStreamConfig", _wrap_meIOStreamConfig, METH_VARARGS },
	{ "meIOStreamRead", _wrap_meIOStreamRead, METH_VARARGS },
	{ "meIOStreamWrite", _wrap_meIOStreamWrite, METH_VARARGS },
	{ "meIOStreamStart", _wrap_meIOStreamStart, METH_VARARGS },
	{ "meIOStreamStop", _wrap_meIOStreamStop, METH_VARARGS },
	{ "meIOStreamStatus", _wrap_meIOStreamStatus, METH_VARARGS },
	{ "meIOStreamFrequencyToTicks", _wrap_meIOStreamFrequencyToTicks, METH_VARARGS },
	{ "meIOStreamTimeToTicks", _wrap_meIOStreamTimeToTicks, METH_VARARGS },
	{ "meQueryDescriptionDevice", _wrap_meQueryDescriptionDevice, METH_VARARGS },
	{ "meQueryInfoDevice", _wrap_meQueryInfoDevice, METH_VARARGS },
	{ "meQueryNameDevice", _wrap_meQueryNameDevice, METH_VARARGS },
	{ "meQueryNameDeviceDriver", _wrap_meQueryNameDeviceDriver, METH_VARARGS },
	{ "meQueryNumberDevices", _wrap_meQueryNumberDevices, METH_VARARGS },
	{ "meQueryNumberSubdevices", _wrap_meQueryNumberSubdevices, METH_VARARGS },
	{ "meQueryNumberChannels", _wrap_meQueryNumberChannels, METH_VARARGS },
	{ "meQueryNumberRanges", _wrap_meQueryNumberRanges, METH_VARARGS },
	{ "meQueryRangeByMinMax", _wrap_meQueryRangeByMinMax, METH_VARARGS },
	{ "meQueryRangeInfo", _wrap_meQueryRangeInfo, METH_VARARGS },
	{ "meQuerySubdeviceByType", _wrap_meQuerySubdeviceByType, METH_VARARGS },
	{ "meQuerySubdeviceType", _wrap_meQuerySubdeviceType, METH_VARARGS },
	{ "meQuerySubdeviceCaps", _wrap_meQuerySubdeviceCaps, METH_VARARGS },
	{ "meQuerySubdeviceCapsArgs", _wrap_meQuerySubdeviceCapsArgs, METH_VARARGS },
	{ "meQueryVersionLibrary", _wrap_meQueryVersionLibrary, METH_VARARGS },
	{ "meQueryVersionMainDriver", _wrap_meQueryVersionMainDriver, METH_VARARGS },
	{ "meQueryVersionDeviceDriver", _wrap_meQueryVersionDeviceDriver, METH_VARARGS },
	{ "meUtilityExtractValues", _wrap_meUtilityExtractValues, METH_VARARGS },
	{ "meUtilityPhysicalToDigital", _wrap_meUtilityPhysicalToDigital, METH_VARARGS },
	{ "meUtilityDigitalToPhysical", _wrap_meUtilityDigitalToPhysical, METH_VARARGS },
	{ "meUtilityPWMStart", _wrap_meUtilityPWMStart, METH_VARARGS },
	{ "meUtilityPWMStop", _wrap_meUtilityPWMStop, METH_VARARGS },
	{ "meUtilityPeriodToTicks", _wrap_meUtilityPeriodToTicks, METH_VARARGS },
	{ "meUtilityTicksToPeriod", _wrap_meUtilityTicksToPeriod, METH_VARARGS },
	{ "meUtilityFrequencyToTicks", _wrap_meUtilityFrequencyToTicks, METH_VARARGS },
	{ "meUtilityTicksToFrequency", _wrap_meUtilityTicksToFrequency, METH_VARARGS },
	{ "meUtilityCodeDivider", _wrap_meUtilityCodeDivider, METH_VARARGS },
	{ "meUtilityDecodeDivider", _wrap_meUtilityDecodeDivider, METH_VARARGS },
	{ "meConfigLoad", _wrap_meConfigLoad, METH_VARARGS },
	{ "meRQueryDescriptionDevice", _wrap_meRQueryDescriptionDevice, METH_VARARGS },
	{ "meRQueryInfoDevice", _wrap_meRQueryInfoDevice, METH_VARARGS },
	{ "meRQueryNameDevice", _wrap_meRQueryNameDevice, METH_VARARGS },
	{ "meRQueryNumberDevices", _wrap_meRQueryNumberDevices, METH_VARARGS },
	{ "meRQueryNumberSubdevices", _wrap_meRQueryNumberSubdevices, METH_VARARGS },
	{ "meRQueryNumberChannels", _wrap_meRQueryNumberChannels, METH_VARARGS },
	{ "meRQueryNumberRanges", _wrap_meRQueryNumberRanges, METH_VARARGS },
	{ "meRQueryRangeInfo", _wrap_meRQueryRangeInfo, METH_VARARGS },
	{ "meRQuerySubdeviceType", _wrap_meRQuerySubdeviceType, METH_VARARGS },
	{ NULL, NULL }
};


/*===========================================================================
  Library initialization
  =========================================================================*/

void initmeDriver(void){
	PyObject *m, *d;

	/* Initialize the module */
	m = Py_InitModule("meDriver", meMethods);

	/* Load numerical python extension */
	import_array();

	/* Get the modules dictionary */
	d = PyModule_GetDict(m);

	/* Install custom error exception */
	meError = PyErr_NewException("meDriver.error", NULL, NULL);
	PyDict_SetItemString(d, "error", meError);

	/* Install the constants */
	me_InstallConstants(d, me_const_table);
}
