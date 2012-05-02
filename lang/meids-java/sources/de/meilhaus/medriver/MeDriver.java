package de.meilhaus.medriver;

public class MeDriver {

	/*==================================================================
	  Defines common to access functions
	  ================================================================*/

	public static final int ME_LOCK_RELEASE = 0x0;
	public static final int ME_LOCK_SET = 0x1;
	public static final int ME_LOCK_CHECK = 0x2;

	/*==================================================================
	  Defines meOpen function
	  ================================================================*/

	public static final int ME_OPEN_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines meClose function
	  ================================================================*/

	public static final int ME_CLOSE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines meLockDriver function
	  ================================================================*/

	public static final int ME_LOCK_DRIUER_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines meLockDevice function
	  ================================================================*/

	public static final int ME_LOCK_DEVICE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines meLockSubdevice function
	  ================================================================*/

	public static final int ME_LOCK_SUBDEVICE_NO_FLAGS = 0x0;


	/*==================================================================
	  Defines common to io functions
	  ================================================================*/

	public static final int ME_REF_DIO_FIFO_LOW = 0x0;
	public static final int ME_REF_DIO_FIFO_HIGH = 0x1;

	public static final int ME_REF_CTR_INTERNAL_PREVIOUS = 0x0;
	public static final int ME_REF_CTR_INTERNAL_1MHZ = 0x1;
	public static final int ME_REF_CTR_INTERNAL_10MHZ = 0x2;
	public static final int ME_REF_CTR_EXTERNAL = 0x3;

	public static final int ME_REF_AI_GROUND = 0x0;
	public static final int ME_REF_AI_DIFFERENTIAL = 0x1;

	public static final int ME_REF_AO_GROUND = 0x0;

	public static final int ME_TRIG_CHAN_DEFAULT = 0x0;
	public static final int ME_TRIG_CHAN_SYNCHRONOUS = 0x1;

	public static final int ME_TRIG_TYPE_NONE = 0x0;
	public static final int ME_TRIG_TYPE_SW = 0x1;
	public static final int ME_TRIG_TYPE_THRESHOLD = 0x2;
	public static final int ME_TRIG_TYPE_WINDOW = 0x3;
	public static final int ME_TRIG_TYPE_EDGE = 0x4;
	public static final int ME_TRIG_TYPE_SLOPE = 0x5;
	public static final int ME_TRIG_TYPE_EXT_DIGITAL = 0x6;
	public static final int ME_TRIG_TYPE_EXT_ANALOG = 0x7;
	public static final int ME_TRIG_TYPE_PATTERN = 0x8;
	public static final int ME_TRIG_TYPE_TIMER = 0x9;
	public static final int ME_TRIG_TYPE_COUNT = 0xA;
	public static final int ME_TRIG_TYPE_FOLLOW = 0xB;

	public static final int ME_TRIG_EDGE_ABOVE = 0x0;
	public static final int ME_TRIG_EDGE_UNDER = 0x1;
	public static final int ME_TRIG_EDGE_ENTRY = 0x2;
	public static final int ME_TRIG_EDGE_EXIT = 0x3;
	public static final int ME_TRIG_EDGE_RISING = 0x4;
	public static final int ME_TRIG_EDGE_FALLING = 0x5;
	public static final int ME_TRIG_EDGE_ANY = 0x6;

	public static final int ME_TIMER_ACQ_START = 0x0;
	public static final int ME_TIMER_SCAN_START = 0x1;
	public static final int ME_TIMER_CONV_START = 0x2;

	/*==================================================================
	  Defines for meIOIrqStart function
	  ================================================================*/

	public static final int ME_IRQ_SOURCE_DIO_PATTERN = 0x0;
	public static final int ME_IRQ_SOURCE_DIO_MASK = 0x1;
	public static final int ME_IRQ_SOURCE_DIO_LINE = 0x2;

	public static final int ME_IRQ_EDGE_RISING = 0x0;
	public static final int ME_IRQ_EDGE_FALLING = 0x1;
	public static final int ME_IRQ_EDGE_ANY = 0x2;

	public static final int ME_IO_IRQ_START_NO_FLAGS = 0x0;
	public static final int ME_IO_IRQ_START_DIO_BIT = 0x1;
	public static final int ME_IO_IRQ_START_DIO_BYTE = 0x2;
	public static final int ME_IO_IRQ_START_DIO_WORD = 0x4;
	public static final int ME_IO_IRQ_START_DIO_DWORD = 0x8;

	/*==================================================================
	  Defines for meIOIrqStop function
	  ================================================================*/

	public static final int ME_IO_IRQ_STOP_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOIrqWait function
	  ================================================================*/

	public static final int ME_IO_IRQ_WAIT_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOResetDevice function
	  ================================================================*/

	public static final int ME_IO_RESET_DEVICE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOResetSubdevice function
	  ================================================================*/

	public static final int ME_IO_RESET_SUBDEVICE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOSingleConfig function
	  ================================================================*/

	public static final int ME_SINGLE_CONFIG_DIO_INPUT = 0x0;
	public static final int ME_SINGLE_CONFIG_DIO_OUTPUT = 0x1;
	public static final int ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE = 0x2;
	public static final int ME_SINGLE_CONFIG_DIO_SINK = 0x3;
	public static final int ME_SINGLE_CONFIG_DIO_SOURCE = 0x4;
	public static final int ME_SINGLE_CONFIG_DIO_MUX32M = 0x5;
	public static final int ME_SINGLE_CONFIG_DIO_DEMUX32 = 0x6;
	public static final int ME_SINGLE_CONFIG_DIO_BIT_PATTERN = 0x7;

	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_0 = 0x0;
	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_1 = 0x1;
	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_2 = 0x2;
	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_3 = 0x3;
	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_4 = 0x4;
	public static final int ME_SINGLE_CONFIG_CTR_8254_MODE_5 = 0x5;

	public static final int ME_IO_SINGLE_CONFIG_NO_FLAGS = 0x00;
	public static final int ME_IO_SINGLE_CONFIG_DIO_BIT = 0x01;
	public static final int ME_IO_SINGLE_CONFIG_DIO_BYTE = 0x02;
	public static final int ME_IO_SINGLE_CONFIG_DIO_WORD = 0x04;
	public static final int ME_IO_SINGLE_CONFIG_DIO_DWORD = 0x08;
	public static final int ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON = 0x10;
	public static final int ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF = 0x20;
	public static final int ME_IO_SINGLE_CONFIG_AI_RMS = 0x40;

	/*==================================================================
	  Defines for meIOSingle function
	  ================================================================*/

	public static final int ME_IO_SINGLE_NO_FLAGS = 0x0;

	public static final int ME_DIR_INPUT = 0x0;
	public static final int ME_DIR_OUTPUT = 0x1;

	public static final int ME_IO_SINGLE_TYPE_NO_FLAGS = 0x00;
	public static final int ME_IO_SINGLE_TYPE_DIO_BIT = 0x01;
	public static final int ME_IO_SINGLE_TYPE_DIO_BYTE = 0x02;
	public static final int ME_IO_SINGLE_TYPE_DIO_WORD = 0x04;
	public static final int ME_IO_SINGLE_TYPE_DIO_DWORD = 0x08;
	public static final int ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS = 0x10;
	public static final int ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING = 0x20;

	/*==================================================================
	  Defines for meIOStreamFrequencyToTicks function
	  ================================================================*/

	public static final int ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOStreamConfig function
	  ================================================================*/

	public static final int ME_IO_STREAM_CONFIG_NO_FLAGS = 0x0;
	public static final int ME_IO_STREAM_CONFIG_DIO_BIT_PATTERN = 0x1;
	public static final int ME_IO_STREAM_CONFIG_WRAPAROUND = 0x2;

	public static final int ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS = 0x0;

	public static final int ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOStreamRead function
	  ================================================================*/

	public static final int ME_READ_MODE_BLOCKING = 0x0;
	public static final int ME_READ_MODE_NONBLOCKING = 0x1;

	public static final int ME_IO_STREAM_READ_NO_FLAGS = 0x0;
	public static final int ME_IO_STREAM_READ_FRAMES = 0x1;

	/*==================================================================
	  Defines for meIOStreamWrite function
	  ================================================================*/

	public static final int ME_WRITE_MODE_BLOCKING = 0x0;
	public static final int ME_WRITE_MODE_NONBLOCKING = 0x1;
	public static final int ME_WRITE_MODE_PRELOAD = 0x2;

	public static final int ME_IO_STREAM_WRITE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOStreamStart function
	  ================================================================*/

	public static final int ME_IO_STREAM_START_NO_FLAGS = 0x0;

	public static final int ME_START_MODE_BLOCKING = 0x0;
	public static final int ME_START_MODE_NONBLOCKING = 0x1;

	public static final int ME_IO_STREAM_START_TYPE_NO_FLAGS = 0x0;
	public static final int ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS = 0x1;

	/*==================================================================
	  Defines for meIOStreamStop function
	  ================================================================*/

	public static final int ME_IO_STREAM_STOP_NO_FLAGS = 0x0;

	public static final int ME_STOP_MODE_IMMEDIATE = 0x0;
	public static final int ME_STOP_MODE_LAST_VALUE = 0x1;

	public static final int ME_IO_STREAM_STOP_TYPE_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOStreamStatus function
	  ================================================================*/

	public static final int ME_WAIT_NONE = 0x0;
	public static final int ME_WAIT_IDLE = 0x1;

	public static final int ME_STATUS_INVALID = 0x0;
	public static final int ME_STATUS_IDLE = 0x1;
	public static final int ME_STATUS_BUSY = 0x2;
	public static final int ME_STATUS_ERROR = 0x3;

	public static final int ME_IO_STREAM_STATUS_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for meIOStreamTimeToTicks function
	  ================================================================*/

	public static final int ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS = 0x0;

	/*==================================================================
	  Defines for module types
	  ================================================================*/

	public static final int ME_MODULE_TYPE_MULTISIG_NONE = 0x0;
	public static final int ME_MODULE_TYPE_MULTISIG_DIFF16_10V = 0x1;
	public static final int ME_MODULE_TYPE_MULTISIG_DIFF16_20V = 0x2;
	public static final int ME_MODULE_TYPE_MULTISIG_DIFF16_50V = 0x3;
	public static final int ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA = 0x4;
	public static final int ME_MODULE_TYPE_MULTISIG_RTD8_PT100 = 0x5;
	public static final int ME_MODULE_TYPE_MULTISIG_RTD8_PT500 = 0x6;
	public static final int ME_MODULE_TYPE_MULTISIG_RTD8_PT1000 = 0x7;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B = 0x8;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E = 0x9;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J = 0xA;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K = 0xB;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N = 0xC;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R = 0xD;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S = 0xE;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T = 0xF;
	public static final int ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR = 0x10;

	/*==================================================================
	  Defines common to query functions
	  ================================================================*/

	public static final int ME_UNIT_INVALID = 0x0;
	public static final int ME_UNIT_VOLT = 0x1;
	public static final int ME_UNIT_AMPERE = 0x2;
	public static final int ME_UNIT_ANY = 0x3;

	public static final int ME_TYPE_INVALID = 0x0;
	public static final int ME_TYPE_AO = 0x1;
	public static final int ME_TYPE_AI = 0x2;
	public static final int ME_TYPE_DIO = 0x3;
	public static final int ME_TYPE_DO = 0x4;
	public static final int ME_TYPE_DI = 0x5;
	public static final int ME_TYPE_CTR = 0x6;
	public static final int ME_TYPE_EXT_IRQ = 0x7;

	public static final int ME_SUBTYPE_INVALID = 0x0;
	public static final int ME_SUBTYPE_SINGLE = 0x1;
	public static final int ME_SUBTYPE_STREAMING = 0x2;
	public static final int ME_SUBTYPE_ANY = 0x3;

	public static final int ME_DEVICE_DRIVER_NAME_MAX_COUNT = 64;
	public static final int ME_DEVICE_NAME_MAX_COUNT = 64;

	public static final int ME_DEVICE_DESCRIPTION_MAX_COUNT = 256;

	public static final int ME_BUS_TYPE_INVALID = 0x0;
	public static final int ME_BUS_TYPE_PCI = 0x1;
	public static final int ME_BUS_TYPE_USB = 0x2;

	public static final int ME_PLUGGED_INVALID = 0x0;
	public static final int ME_PLUGGED_IN = 0x1;
	public static final int ME_PLUGGED_OUT = 0x2;

	public static final int ME_EXTENTION_TYPE_INVALID = 0x0;
	public static final int ME_EXTENTION_TYPE_NONE = 0x1;
	public static final int ME_EXTENTION_TYPE_MUX32M = 0x2;
	public static final int ME_EXTENTION_TYPE_DEMUX32 = 0x3;
	public static final int ME_EXTENTION_TYPE_MUX32S = 0x4;

	public static final int ME_ACCESS_TYPE_INVALID = 0x0;
	public static final int ME_ACCESS_TYPE_LOCAL = 0x1;
	public static final int ME_ACCESS_TYPE_REMOTE = 0x2;


	/*===========================================================================
	  Functions to access the driver system
	  =========================================================================*/

	public native void meOpen(int iFlags) throws java.io.IOException;
	public native void meClose(int iFlags) throws java.io.IOException;

	public native void meLockDriver(int iLock, int iFlags) throws java.io.IOException;
	public native void meLockDevice(int iDevice, int iLock, int iFlags) throws java.io.IOException;
	public native void meLockSubdevice(
			int iDevice,
			int iSubdevice,
			int iLock,
			int iFlags) throws java.io.IOException;


	/*===========================================================================
	  Functions to perform I/O on a device
	  =========================================================================*/

	public native void meIOIrqStart(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iIrqSource,
			int iIrqEdge,
			int iIrqArg,
			int iFlags) throws java.io.IOException;
	public native void meIOIrqStop(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iFlags) throws java.io.IOException;
	public native void meIOIrqWait(
			int iDevice,
			int iSubdevice,
			int iChannel,
			MeIrq irq,
			int iTimeOut,
			int iFlags) throws java.io.IOException;

	public native void meIOResetDevice(int iDevice, int iFlags) throws java.io.IOException;
	public native void meIOResetSubdevice(int iDevice, int iSubdevice, int iFlags) throws java.io.IOException;

	public native void meIOSingleConfig(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iSingleConfig,
			int iRef,
			int iTrigChan,
			int iTrigType,
			int iTrigEdge,
			int iFlags) throws java.io.IOException;
	public native void meIOSingle(MeIOSingle[] singleList, int iFlags) throws java.io.IOException;

	public native void meIOStreamConfig(
			int iDevice,
			int iSubdevice,
			MeIOStreamConfig[] configList,
			MeIOStreamTrigger trigger,
			int iFifoIrqThreshold,
			int iFlags) throws java.io.IOException;
	public native int[] meIOStreamRead(
			int iDevice,
			int iSubdevice,
			int iReadMode,
			int iCount,
			int iFlags) throws java.io.IOException;
	public native int[] meIOStreamWrite(
			int iDevice,
			int iSubdevice,
			int iWriteMode,
			int[] values,
			int iFlags) throws java.io.IOException;
	public native void meIOStreamStart(MeIOStreamStart[] startList, int iFlags) throws java.io.IOException;
	public native void meIOStreamStop(MeIOStreamStop[] stopList, int iFlags) throws java.io.IOException;
	public native void meIOStreamStatus(
			int iDevice,
			int iSubdevice,
			int iWait,
			MeStatus status,
			int iFlags) throws java.io.IOException;
	public native void meIOStreamFrequencyToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			MeTicks ticks,
			int iFlags) throws java.io.IOException;
	public native void meIOStreamTimeToTicks(
			int iDevice,
			int iSubdevice,
			int iTimer,
			MeTicks ticks,
			int iFlags) throws java.io.IOException;


	/*===========================================================================
	  Functions to query the driver system
	  =========================================================================*/

	public native String meQueryDescriptionDevice(int iDevice) throws java.io.IOException;

	public native void meQueryInfoDevice(int iDevice, MeInfo info) throws java.io.IOException;

	public native String meQueryNameDevice(int iDevice) throws java.io.IOException;
	public native String meQueryNameDeviceDriver(int iDevice) throws java.io.IOException;

	public native int meQueryNumberDevices() throws java.io.IOException;
	public native int meQueryNumberSubdevices(int iDevice) throws java.io.IOException;
	public native int meQueryNumberChannels(int iDevice, int iSubdevice) throws java.io.IOException;
	public native int meQueryNumberRanges(
			int iDevice,
			int iSubdevice,
			int iChannel,
			int iUnit) throws java.io.IOException;

	public native void meQueryRangeByMinMax(
			int iDevice,
			int iSubdevice,
			int iChannel,
			MeRange range) throws java.io.IOException;
	public native void meQueryRangeInfo(
			int iDevice,
			int iSubdevice,
			int iChannel,
			MeRange range) throws java.io.IOException;

	public native int meQuerySubdeviceByType(
			int iDevice,
			int iStartSubdevice,
			int iType,
			int iSubtype) throws java.io.IOException;
	public native void meQuerySubdeviceType(int iDevice, int iSubdevice, MeType type) throws java.io.IOException;


	public native int meQueryVersionLibrary() throws java.io.IOException;
	public native int meQueryVersionMainDriver() throws java.io.IOException;
	public native int meQueryVersionDeviceDriver(int iDevice) throws java.io.IOException;


	/*===========================================================================
	  Common utility functions
	  =========================================================================*/

	public native int[] meUtilityExtractValues(
			int iChannel,
			int[] aiBuffer,
			MeIOStreamConfig[] configList) throws java.io.IOException;

	public native double meUtilityDigitalToPhysical(
			double dMin,
			double dMax,
			int iMaxData,
			int iData,
			int iModuleType,
			double dRefValue) throws java.io.IOException;

	public native int meUtilityPhysicalToDigital(
			double dMin,
			double dMax,
			int iMaxData,
			double dPhysical) throws java.io.IOException;

	public native void meUtilitPWMStart(
			int iDevice,
			int iSubdevice,
			int iRef,
			int iPrescaler,
			int iDutyCycle) throws java.io.IOException;
	public native void meUtilityPWMStop(int iDevice, int iSubdevice) throws java.io.IOException;


	/*===========================================================================
	  The extension library to load
	  =========================================================================*/

	static {
		System.loadLibrary("medriverj");
	}
}
