meids-python
------------
This module encapsulates the access to the Meilhaus data acquisition boards.
It provides an interface to the ME-iDS (Meilhaus intelligent Driver System) API
documented in the ME-iDS manual.

It is released under the GNU Lesser GPL, see LICENCE.txt for more details.

(C) 2006 Guenter Gebhardt
(C) 2009 Krzysztof Gantzke <k.gantzke@meilhaus.de>


Features
--------
- Function based interface to the ME-iDS API on Windows and Linux.


Requirements
------------
- Python developement package 2.7 or newer.
- Python NumPy development pachage.
- On Windows meDriver.dll version 1.0 with Meilhaus driver system API or newer
- On Linux libmedriver.so shared object 1.0.0 with Meilhaus driver system API or newer


Installation
------------
Before we can install the extension module it has to be compiled by Distutils.
This requires a properly installed C-compiler on your system.

Extract files from the archive, open a shell/console in that directory and
let Distutils do the rest: "python setup.py install"

The files get compiled installed in the "Lib/site-packages" directory in newer
Python versions.


Interface Functions
-------------------

- Functions to access the driver system
  -------------------------------------

	meOpen(
		Flags		# Python integer
	)
	Return value:	None


	meClose(
		Flags		# Python integer
	)
	Return value:	None


	meLockDriver(
		Lock,		# Python integer
		Flags		# Python integer
	)
	Return value:	None


	meLockDevice(
		Device,		# Python integer
		Lock,		# Python integer
		Flags		# Python integer
	)
	Return value:	None


	meLockSubdevice(
		Device,		# Python integer
		Subdevice,	# Python integer
		Lock,		# Python integer
		Flags		# Python integer
	)
	Return value:	None


- Functions to perform I/O on a device
  ------------------------------------

	meIOIrqStart(
		Device,		# Python integer
		Subdevice,	# Python integer
		Channel,	# Python integer
		IrqSource,	# Python integer
		IrqEdge,	# Python integer
		IrqArg,		# Python integer
		Flags		# Python integer
	)
	Return value:	None

	meIOIrqStop(
		Device,		# Python integer
		Subdevice,	# Python integer
		Channel,	# Python integer
		Flags		# Python integer
	)
	Return value:	None

	meIOIrqWait(
		Device,		# Python integer
		Subdevice,	# Python integer
		Channel,	# Python integer
		TimeOut,	# Python integer
		Flags		# Python integer
	)
	Return value:	(IrqCount,	# Python integer
					Value)		# Python integer

	meIOResetDevice(
		Device,		# Python integer
		Flags		# Python integer
	)
	Return value:	None


	meIOResetSubdevice(
		Device,		# Python integer
		Subdevice,	# Python integer
		Flags		# Python integer
	)
	Return value:	None


	meIOSingleConfig(
		Device,			# Python integer
		Subdevice,		# Python integer
		Channel,		# Python integer
		SingleConfig,	# Python integer
		Ref,			# Python integer
		TrigChan,		# Python integer
		TrigType,		# Python integer
		TrigEdge,		# Python integer
		Flags			# Python integer
	)
	Return value:	None


	meIOSingle(
		SingleList		# Python list
		Flags			# Python integer
	)
	SingleList item:	# Python dictionary
	SingleList item keys:
		'Device'		# Python integer
		'Subdevice'		# Python integer
		'Channel'		# Python integer
		'Dir'			# Python integer
		'Value'			# Python integer
		'TimeOut'		# Python integer
		'Flags'			# Python integer
		'Errno'			# Python integer

	Return value:		None


	meIOStreamConfig(
		Device				# Python integer
		Subdevice			# Python integer
		ConfigList			# Python list
		Trigger				# Python dictionary
		FifoIrqThreshold	# Python integer
		Flags				# Python integer
	)
	ConfigList item:	# Python dictionary
	ConfigList item keys:
		'Channel'		# Python integer
		'StreamConfig'	# Python integer
		'Ref'			# Python integer
		'Flags'			# Python integer

	Trigger keys:
		'AcqStartTrigType'	# Python integer
		'AcqStartTrigEdge'	# Python integer
		'AcqStartTrigChan'	# Python integer
		'AcqStartTicks'		# Python integer
		'ScanStartTrigType'	# Python integer
		'ScanStartTicks'	# Python integer
		'ConvStartTrigType'	# Python integer
		'ConvStartTicks'	# Python integer
		'ScanStopTrigType'	# Python integer
		'ScanStopCount'		# Python integer
		'AcqStopTrigType'	# Python integer
		'AcqStopCount'		# Python integer
		'Flags'				# Python integer

	Return value:		None


	meIOStreamRead(
		Device,				# Python integer
		Subdevice,			# Python integer
		ReadMode,			# Python integer
		Count,				# Python integer
		Flags				# Python integer
	)
	Return value:
		Values read			# One dimensional numerical python integer array


	meIOStreamWrite(
		Device,				# Python integer
		Subdevice,			# Python integer
		WriteMode,			# Python integer
		Values,				# One dimensional numerical python integer array
		Flags				# Python integer
	)
	Values items:			# Python integer

	Return value:
		Remaining Values	# One dimensional numerical python integer array


	meIOStreamStart(
		StartList,			# Python list
		Flags				# Python integer
	)
	StartList item:			# Python dictionary
	StartList item keys:
		'Device'			# Python integer
		'Subdevice'			# Python integer
		'StartMode'			# Python integer
		'TimeOut'			# Python integer
		'Flags'				# Python integer
		'Errno'				# Python integer

	Return value:			None


	meIOStreamStop(
		StopList,			# Python list
		Flags				# Python integer
	)
	StopList item:			# Python dictionary
	StopList item keys:
		'Device'			# Python integer
		'Subdevice'			# Python integer
		'StopMode'			# Python integer
		'Flags'				# Python integer
		'Errno'				# Python integer

	Return value:			None


	meIOStreamStatus(
		Device,				# Python integer
		Subdevice,			# Python integer
		Wait,				# Python iteger
		Flags				# Python integer
	)
	Return value:	(Status,	# Python integer
					 Count)		# Python integer


	meIOStreamFrequencyToTicks(
		Device,				# Python integer
		Subdevice,			# Python integer
		Timer,				# Python integer
		Frequency,			# Python float
		Flags				# Python integer
	)
	Return value:	(AchievedFrequency,	# Python float
			 		Ticks)				# Python integer


	meIOStreamTimeToTicks(
		Device,				# Python integer
		Subdevice,			# Python integer
		Timer,				# Python integer
		Time,				# Python float
		Flags				# Python integer
	)
	Return value:	(AchievedTime,	# Python float
			 		Ticks)			# Python integer


- Functions to query the driver system
  ------------------------------------

	meQueryDescriptionDevice(
		Device,					# Python integer
	)
	Return Value: Description	# Python string


	meQueryInfoDevice(
		Device,					# Python integer
	)
	Return Value:	(VendorId,	# Python integer
					DeviceId,	# Python integer
					SerialNo,	# Python integer
					BusType,	# Python integer
					BusNo,		# Python integer
					DevNo,		# Python integer
					FuncNo,		# Python integer
					Plugged)	# Python integer


	meQueryNameDevice(
		Device,					# Python integer
	)
	Return Value: Name			# Python string


	meQueryNameDeviceDriver(
		Device,					# Python integer
	)
	Return Value: Name			# Python string


	meQueryNumberDevices(
	)
	Return Value: Number		# Python integer


	meQueryNumberSubdevices(
		Device,					# Python integer
	)
	Return Value: Number		# Python integer


	meQueryNumberChannels(
		Device,					# Python integer
		Subdevice,				# Python integer
	)
	Return Value: Number		# Python integer


	meQueryNumberRanges(
		Device,					# Python integer
		Subdevice,				# Python integer
		Unit					# Python integer
	)
	Return Value: Number		# Python integer


	meQueryRangeByMinMax(
		Device,					# Python integer
		Subdevice,				# Python integer
		Unit,					# Python integer
		Min,					# Python integer
		Max,					# Python integer
	);
	Return value:	(Min,		# Python float
			 		Max,		# Python float
					MaxData		# Python integer
					Range)		# Python integer


	meQueryRangeInfo(
		Device,					# Python integer
		Subdevice,				# Python integer
		Range					# Python integer
	);
	Return value:	(Unit,		# Python integer
					Min,		# Python float
			 		Max,		# Python float
					MaxData)	# Python integer


	meQuerySubdeviceByType(
		Device,					# Python integer
		StartSubdevice,			# Python integer
		Type,					# Python integer
		Subtype					# Python integer
	);
	Return Value:
		Subdevice				# Python integer


	meQuerySubdeviceCaps(
		Device,					# Python integer
		Subdevice				# Python integer
	);
	Return Value:
		Caps					# Python integer


	meQuerySubdeviceCapsArgs(
			Device,				# Python integer
			Subdevice,			# Python integer
			Cap					# Python integer
	);
	Return Value:
			Args				# Python list

	meQuerySubdeviceType(
		Device,					# Python integer
		Subdevice				# Python integer
	);
	Return Value:	(Type,		# Python integer
					Subtype)	# Python integer

	meQueryVersionLibrary(
	)
	Return Value:
		Version					# Python integer


	meQueryVersionMainDriver(
	)
	Return Value:
		Version					# Python integer
	

	meQueryVersionDeviceDriver(
		Device,					# Python integer
	)
	Return Value:
		Version					# Python integer


- Common utility functions
--------------------------

	meUtilityExtractValues(
		Channel,				# Python integer
		AIBuffer,				# One dimensional numerical python integer array
		ConfigList,				# Python list
	);
	ConfigList item:			# Python dictionary
	ConfigList item keys:
		'Channel'				# Python integer
		'StreamConfig'			# Python integer
		'Ref'					# Python integer
		'Flags'					# Python integer

	Return Value:
		ChanBuffer				# One dimensional numerical python integer array


	meUtilityPhysicalToDigital(
		Min,					# Python float
		Max,					# Python float
		MaxData,				# Python integer
		Physical,				# Python float
	)
	Return Value:
		Data					# Python integer


	meUtilityDigitalToPhysical(
		Min,					# Python float
		Max,					# Python float
		MaxData,				# Python integer
		Data,					# Python integer
		ModuleType,				# Python integer
		RefValue				# Python integer
	)
	Return Value:
		Physical				# Python float


	meUtilityPWMStart(
		Device,					# Python integer
		Subdevice,				# Python integer
		Ref,					# Python integer
		Prescaler,				# Python integer
		DutyCycle,				# Python integer
	)
	Return Value: None


	meUtilityPWMStop(
		Device,					# Python integer
		Subdevice,				# Python integer
	)
	Return Value: None


Constants
---------

	ME_VALUE_NOT_USED
	ME_LOCK_RELEASE
	ME_LOCK_SET
	ME_LOCK_CHECK
	ME_OPEN_NO_FLAGS
	ME_CLOSE_NO_FLAGS
	ME_LOCK_DRIVER_NO_FLAGS
	ME_LOCK_DEVICE_NO_FLAGS
	ME_LOCK_SUBDEVICE_NO_FLAGS
	ME_ERROR_MSG_MAX_COUNT
	ME_SWITCH_DISABLE
	ME_SWITCH_ENABLE
	ME_REF_DIO_FIFO_LOW
	ME_REF_DIO_FIFO_HIGH
	ME_REF_CTR_INTERNAL_PREVIOUS
	ME_REF_CTR_INTERNAL_1MHZ
	ME_REF_CTR_INTERNAL_1MHZ
	ME_REF_CTR_EXTERNAL
	ME_REF_AI_GROUND
	ME_REF_AI_DIFFERENTIAL
	ME_REF_AO_GROUND
	ME_TRIG_CHAN_DEFAULT
	ME_TRIG_CHAN_SYNCHRONOUS
	ME_TRIG_TYPE_NONE
	ME_TRIG_TYPE_SW
	ME_TRIG_TYPE_THRESHOLD
	ME_TRIG_TYPE_WINDOW
	ME_TRIG_TYPE_EDGE
	ME_TRIG_TYPE_SLOPE
	ME_TRIG_TYPE_EXT_DIGITAL
	ME_TRIG_TYPE_EXT_ANALOG
	ME_TRIG_TYPE_PATTERN
	ME_TRIG_TYPE_TIMER
	ME_TRIG_TYPE_COUNT
	ME_TRIG_TYPE_FOLLOW
	ME_TRIG_EDGE_ABOVE
	ME_TRIG_EDGE_UNDER
	ME_TRIG_EDGE_ENTRY
	ME_TRIG_EDGE_EXIT
	ME_TRIG_EDGE_RISING
	ME_TRIG_EDGE_FALLING
	ME_TRIG_EDGE_ANY
	ME_TIMER_ACQ_START
	ME_TIMER_SCAN_START
	ME_TIMER_CONV_START
	ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS
	ME_IRQ_SOURCE_DIO_PATTERN
	ME_IRQ_SOURCE_DIO_MASK
	ME_IRQ_SOURCE_DIO_LINE
	ME_IRQ_SOURCE_DIO_OVER_TEMP
	ME_IRQ_EDGE_RISING
	ME_IRQ_EDGE_FALLING
	ME_IRQ_EDGE_ANY
	ME_IO_IRQ_START_NO_FLAGS
	ME_IO_IRQ_START_DIO_BIT
	ME_IO_IRQ_START_DIO_BYTE
	ME_IO_IRQ_START_DIO_WORD
	ME_IO_IRQ_START_DIO_DWORD
	ME_IO_IRQ_WAIT_NO_FLAGS
	ME_IO_IRQ_STOP_NO_FLAGS
	ME_IO_IRQ_SET_CALLBACK_NO_FLAGS
	ME_IO_RESET_DEVICE_NO_FLAGS
	ME_IO_RESET_SUBDEVICE_NO_FLAGS
	ME_SINGLE_CONFIG_DIO_INPUT
	ME_SINGLE_CONFIG_DIO_OUTPUT
	ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE
	ME_SINGLE_CONFIG_DIO_SINK
	ME_SINGLE_CONFIG_DIO_SOURCE
	ME_SINGLE_CONFIG_DIO_MUX32M
	ME_SINGLE_CONFIG_DIO_DEMUX32
	ME_SINGLE_CONFIG_DIO_BIT_PATTERN
	ME_SINGLE_CONFIG_CTR_8254_MODE_0
	ME_SINGLE_CONFIG_CTR_8254_MODE_1
	ME_SINGLE_CONFIG_CTR_8254_MODE_2
	ME_SINGLE_CONFIG_CTR_8254_MODE_3
	ME_SINGLE_CONFIG_CTR_8254_MODE_4
	ME_SINGLE_CONFIG_CTR_8254_MODE_5
	ME_IO_SINGLE_CONFIG_NO_FLAGS0
	ME_IO_SINGLE_CONFIG_DIO_BIT1
	ME_IO_SINGLE_CONFIG_DIO_BYTE2
	ME_IO_SINGLE_CONFIG_DIO_WORD4
	ME_IO_SINGLE_CONFIG_DIO_DWORD
	ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON
	ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF
	ME_IO_SINGLE_CONFIG_AI_RMS
	ME_IO_SINGLE_NO_FLAGS
	ME_DIR_INPUT
	ME_DIR_OUTPUT
	ME_IO_SINGLE_TYPE_NO_FLAGS
	ME_IO_SINGLE_TYPE_DIO_BIT
	ME_IO_SINGLE_TYPE_DIO_BYTE
	ME_IO_SINGLE_TYPE_DIO_WORD
	ME_IO_SINGLE_TYPE_DIO_DWORD
	ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS
	ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING
	ME_IO_STREAM_CONFIG_NO_FLAGS
	ME_IO_STREAM_CONFIG_BIT_PATTERN
	ME_IO_STREAM_CONFIG_WRAPAROUND
	ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD
	ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS
	ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS
	ME_READ_MODE_BLOCKING
	ME_READ_MODE_NONBLOCKING
	ME_IO_STREAM_READ_NO_FLAGS
	ME_IO_STREAM_READ_FRAMES
	ME_WRITE_MODE_BLOCKING
	ME_WRITE_MODE_NONBLOCKING
	ME_WRITE_MODE_PRELOAD
	ME_IO_STREAM_WRITE_NO_FLAGS
	ME_IO_STREAM_START_NO_FLAGS
	ME_START_MODE_BLOCKING
	ME_START_MODE_NONBLOCKING
	ME_IO_STREAM_START_TYPE_NO_FLAGS
	ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS
	ME_IO_STREAM_STOP_NO_FLAGS
	ME_STOP_MODE_IMMEDIATE
	ME_STOP_MODE_LAST_VALUE
	ME_IO_STREAM_STOP_TYPE_NO_FLAGS
	ME_WAIT_NONE
	ME_WAIT_IDLE
	ME_STATUS_INVALID
	ME_STATUS_IDLE
	ME_STATUS_BUSY
	ME_STATUS_ERROR
	ME_IO_STREAM_STATUS_NO_FLAGS
	ME_IO_STREAM_SET_CALLBACKS_NO_FLAGS
	ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS
	ME_MODULE_TYPE_MULTISIG_NONE
	ME_MODULE_TYPE_MULTISIG_DIFF16_10V
	ME_MODULE_TYPE_MULTISIG_DIFF16_20V
	ME_MODULE_TYPE_MULTISIG_DIFF16_50V
	ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA
	ME_MODULE_TYPE_MULTISIG_RTD8_PT100
	ME_MODULE_TYPE_MULTISIG_RTD8_PT500
	ME_MODULE_TYPE_MULTISIG_RTD8_PT1000
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S
	ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T
	ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR
	ME_CAPS_DIO_DIR_BIT	
	ME_CAPS_DIO_DIR_BYTE
	ME_CAPS_DIO_DIR_WORD
	ME_CAPS_DIO_DIR_DWORD	
	ME_CAPS_DIO_SINK_SOURCE
	ME_CAPS_DIO_BIT_PATTERN_IRQ
	ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING
	ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING
	ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY
	ME_CAPS_DIO_OVER_TEMP_IRQ
	ME_CAPS_CTR_CLK_PREVIOUS
	ME_CAPS_CTR_CLK_INTERNAL_1MHZ
	ME_CAPS_CTR_CLK_INTERNAL_10MHZ
	ME_CAPS_CTR_CLK_EXTERNAL
	ME_CAPS_AI_TRIG_SIMULTANEOUS
	ME_CAPS_AI_FIFO
	ME_CAPS_AI_FIFO_THRESHOLD
	ME_CAPS_AO_TRIG_SIMULTANEOUS
	ME_CAPS_AO_FIFO
	ME_CAPS_AO_FIFO_THRESHOLD
	ME_CAPS_EXT_IRQ_EDGE_RISING
	ME_CAPS_EXT_IRQ_EDGE_FALLING
	ME_CAPS_EXT_IRQ_EDGE_ANY
	ME_CAP_AI_FIFO_SIZE
	ME_CAP_AO_FIFO_SIZE
	ME_CAP_CTR_WIDTH
	ME_UNIT_INVALID
	ME_UNIT_VOLT
	ME_UNIT_AMPERE
	ME_UNIT_ANY
	ME_TYPE_INVALID
	ME_TYPE_AO
	ME_TYPE_AI
	ME_TYPE_DIO
	ME_TYPE_DO
	ME_TYPE_DI
	ME_TYPE_CTR
	ME_TYPE_EXT_IRQ
	ME_SUBTYPE_INVALID
	ME_SUBTYPE_SINGLE
	ME_SUBTYPE_STREAMING
	ME_SUBTYPE_ANY
	ME_CAPS_DIO_DIR_BIT
	ME_CAPS_DIO_DIR_BYTE
	ME_CAPS_DIO_DIR_WORD
	ME_CAPS_DIO_DIR_DWORD
	ME_DEVICE_DRIVER_NAME_MAX_COUNT
	ME_DEVICE_NAME_MAX_COUNT
	ME_DEVICE_DESCRIPTION_MAX_COUNT
	ME_BUS_TYPE_INVALID
	ME_BUS_TYPE_PCI
	ME_BUS_TYPE_USB
	ME_PLUGGED_INVALID
	ME_PLUGGED_IN
	ME_PLUGGED_OUT
	ME_EXTENTION_TYPE_INVALID
	ME_EXTENTION_TYPE_NONE
	ME_EXTENTION_TYPE_MUX32M
	ME_EXTENTION_TYPE_DEMUX32
	ME_EXTENTION_TYPE_MUX32S
	ME_ACCESS_TYPE_INVALID
	ME_ACCESS_TYPE_LOCAL
	ME_ACCESS_TYPE_REMOTE

Module specific exception
-------------------------

If an error occures in a call to the ME driver library, the module raises
the exception meDriver.error and sets the exception value to a string,
which describes the error.


Short example
-------------

import meDriver
import sys

try:
	meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
	singleListEntry = {
			'Device': 0,
			'Subdevice': 0,
			'Channel': 0,
			'Dir': meDriver.ME_DIR_OUTPUT,
			'Value': 0xAA,
			'TimeOut': 0,
			'Flags': meDriver.ME_IO_SINGLE_TYPE_NO_FLAGS,
			'Errno': meDriver.ME_ERRNO_SUCCESS
			}
	singleList = [ singleListEntry ]
	meDriver.meIOSingle(singleList, meDriver.ME_IO_SINGLE_NO_FLAGS)
	meDriver.meClose(meDriver.ME_CLOSE_NO_FLAGS)
except meDriver.error, value:
	print "Error:", value
	sys.exit(1)
sys.exit(0)


References
----------

- Python: http://www.python.org
- win32all: http://starship.python.net/crew/mhammond
  and http://www.activestate.com/Products/ActivePython/win32all.html
- Meilhaus Electronic GmbH: http://www.meilhaus.com


