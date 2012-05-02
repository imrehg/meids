/**
 * @file mephisto_defines.h
 *
 * @note Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2011 Meilhaus Electronic GmbH (support@meilhaus.de)
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

# ifndef _MEPHISTO_DEFINES_H_
#  define _MEPHISTO_DEFINES_H_

///Default endpoints
#  define MEPHISTO_EP_IN	0x81
#  define MEPHISTO_EP_OUT	0x02

typedef enum
{
	MEPHISTO_CMD_SetupWrite = 0,
	MEPHISTO_CMD_SetupRead,
	MEPHISTO_CMD_SetAmplitude,
	MEPHISTO_CMD_SetOffset,
	MEPHISTO_CMD_SetTimeBase,
	MEPHISTO_CMD_SetMemory,
	MEPHISTO_CMD_SetTrigger,
	MEPHISTO_CMD_SetMode,
	MEPHISTO_CMD_SetRtClock,
	MEPHISTO_CMD_Run,
	MEPHISTO_CMD_Break,
	MEPHISTO_CMD_Write,
	MEPHISTO_CMD_Read,
	MEPHISTO_CMD_Inquiry,
	MEPHISTO_CMD_Restart,
	MEPHISTO_CMD_FirmwareUpload,
	MEPHISTO_CMD_Calibration
}mephisto_cmd_e;

typedef enum
{
	MEPHISTO_MODE_voltmeter = 0,
	MEPHISTO_MODE_voltmeter_raw,
	MEPHISTO_MODE_voltmeterRMS,
	MEPHISTO_MODE_voltmeterRMS_raw,
	MEPHISTO_MODE_oscilloscope,
	MEPHISTO_MODE_data_logger,
	MEPHISTO_MODE_logic_analizer,
	MEPHISTO_MODE_digital_logger
}mephisto_mode_e;

typedef union
{
  float value;
  struct
  {
    unsigned int value_fraction : 23;
    unsigned int value_exponent : 8;
    unsigned int value_sign : 1;
  }__attribute__ ((packed));
}float_t;

typedef union
{
	uint32_t 		value;
	uint16_t 		svalue[2];
	unsigned char	text[4];
	float_t			fvalue;

}__attribute__ ((packed)) MEPHISTO_modes_tu;

typedef struct
{
	MEPHISTO_modes_tu	amplitude_0;			//s0 - float
	MEPHISTO_modes_tu	amplitude_1;			//s1 - float

	MEPHISTO_modes_tu	offset_0;				//s2 - float
	MEPHISTO_modes_tu	offset_1;				//s3 - float

	MEPHISTO_modes_tu	time_base;				//s4 - float

	MEPHISTO_modes_tu	memory_depth;			//s5 - float

	MEPHISTO_modes_tu	trigger_point;			//s6 - float
	MEPHISTO_modes_tu	trigger_channel;		//s7 - uint32_t
	MEPHISTO_modes_tu	trigger_type;			//s8 - uint32_t
	MEPHISTO_modes_tu	upper_trigger_level;	//s9 - float
	MEPHISTO_modes_tu	lower_trigger_level;	//s10 - float

	MEPHISTO_modes_tu	data_GPIO;				//s11 - uint32_t
	MEPHISTO_modes_tu	dir_GPIO;				//s12 - uint32_t
}__attribute__ ((packed)) Setup_arg_send_t;

typedef struct
{
	MEPHISTO_modes_tu	amplitude_0;				//r0 - float
	MEPHISTO_modes_tu	amplitude_1;				//r1 - float

	MEPHISTO_modes_tu	offset_0;					//r2 - float
	MEPHISTO_modes_tu	offset_1;					//r3 - float

	MEPHISTO_modes_tu	correction_0;				//r4 - float
	MEPHISTO_modes_tu	correction_1;				//r5 - float

	MEPHISTO_modes_tu	time_base;					//r6 - float

	MEPHISTO_modes_tu	memory_depth;				//r7 - float

	MEPHISTO_modes_tu	trigger_point;				//r8 - float
	MEPHISTO_modes_tu	trigger_channel;			//r9 - uint32_t
	MEPHISTO_modes_tu	trigger_type;				//r10 - uint32_t
	MEPHISTO_modes_tu	upper_trigger_level;		//r11 - float
	MEPHISTO_modes_tu	lower_trigger_level;		//r12 - float

	MEPHISTO_modes_tu	data_GPIO;				//r13 - uint32_t
	MEPHISTO_modes_tu	dir_GPIO;				//r14 - uint32_t
}__attribute__ ((packed)) Setup_arg_recive_t;

typedef struct
{
	MEPHISTO_modes_tu		data_GPIO;
	MEPHISTO_modes_tu		dir_GPIO;
}__attribute__ ((packed)) GPIO_arg_t;

typedef struct
{
	MEPHISTO_modes_tu		channel;
	MEPHISTO_modes_tu		value;
}__attribute__ ((packed)) AmplitudeOffset_arg_send_t;

typedef struct
{
	MEPHISTO_modes_tu		amplitude;
	MEPHISTO_modes_tu		offset;
	MEPHISTO_modes_tu		correction;
}__attribute__ ((packed)) AmplitudeOffset_arg_recive_t;

typedef struct
{
	MEPHISTO_modes_tu		channel;
	MEPHISTO_modes_tu		trigger_type;
	MEPHISTO_modes_tu		upper_trigger_level;
	MEPHISTO_modes_tu		lower_trigger_level;
}__attribute__ ((packed)) Trigger_arg_send_t;

typedef struct
{
	MEPHISTO_modes_tu		upper_trigger_level;
	MEPHISTO_modes_tu		lower_trigger_level;
}__attribute__ ((packed)) Trigger_arg_recive_t;

typedef struct
{
	MEPHISTO_modes_tu		mode;
}__attribute__ ((packed)) SetMode_arg_t;


typedef enum
{
	MEPHISTO_AI_STATUS_idle = 0,
	MEPHISTO_AI_STATUS_error,
	MEPHISTO_AI_STATUS_fifo_error,
	MEPHISTO_AI_STATUS_buffer_error,
	MEPHISTO_AI_STATUS_timeout,
	MEPHISTO_AI_STATUS_configured,
	MEPHISTO_AI_STATUS_start,
	MEPHISTO_AI_STATUS_run
}mephisto_AI_status_e;


# endif	//_MEPHISTO_DEFINES_H_
#endif	//__KERNEL__
