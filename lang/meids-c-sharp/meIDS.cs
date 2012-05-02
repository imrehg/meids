using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace meIDS_CS
{
    /// <summary>
    /// In dieser Klasse sind alle Konstanten definiert und Strukturen beschrieben.
    /// Allerdings sind noch keine Schnittstellen-Methoden definiert. Für einen
    /// Zugriff auf den Treiber muss die konkrete Klasse <see cref="Meilhaus::Driver"/> oder
    /// <see cref="Meilhaus::Net::Driver"/> benutzt werden.
    /// Stand: 21. Februar 2007
    /// </summary>
    public sealed class meIDS
    {
        #region Constants

        /*==================================================================
          General
          ================================================================*/

        public const int ME_VALUE_NOT_USED						= 0x0;
        public const int ME_VALUE_DEFAULT						= 0x0;
		public const int ME_NO_FLAGS								= 0x0;
        public const int ME_VALUE_INVALID						= ~0x0;

        /*==================================================================
          Defines common to access functions
          ================================================================*/

        public const int ME_LOCK_RELEASE							= 0x00010001;
        public const int ME_LOCK_SET								= 0x00010002;
        public const int ME_LOCK_CHECK							= 0x00010003;

        /*==================================================================
          Defines meOpen function
          ================================================================*/

        public const int ME_OPEN_NO_FLAGS						= 0x0;

        /*==================================================================
          Defines meClose function
          ================================================================*/

        public const int ME_CLOSE_NO_FLAGS						= 0x0;
		public const int ME_CLOSE_FORCE							= 0x10000000;


        /*==================================================================
          Defines meLockDriver function
          ================================================================*/

        public const int ME_LOCK_DRIVER_NO_FLAGS					= 0x0;

        /*==================================================================
          Defines meLockDevice function
          ================================================================*/

        public const int ME_LOCK_DEVICE_NO_FLAGS					= 0x0;

        /*==================================================================
          Defines meLockSubdevice function
          ================================================================*/

        public const int ME_LOCK_SUBDEVICE_NO_FLAGS				= 0x0;


        /*==================================================================
          Defines common to error functions
          ================================================================*/

        public const int ME_ERROR_MSG_MAX_COUNT					= 256;

        public const int ME_SWITCH_DISABLE						= 0x00020001;
        public const int ME_SWITCH_ENABLE						= 0x00020002;

        /*==================================================================
          Defines common to io functions
          ================================================================*/

        public const int ME_REF_NONE								= 0x00000000;
        public const int ME_REF_DIO_FIFO_LOW						= 0x00030001;
        public const int ME_REF_DIO_FIFO_HIGH					= 0x00030002;

        public const int ME_REF_CTR_PREVIOUS						= 0x00040001;
        public const int ME_REF_CTR_INTERNAL_1MHZ				= 0x00040002;
        public const int ME_REF_CTR_INTERNAL_10MHZ				= 0x00040003;
        public const int ME_REF_CTR_EXTERNAL						= 0x00040004;

        public const int ME_REF_AI_GROUND						= 0x00050001;
        public const int ME_REF_AI_DIFFERENTIAL					= 0x00050002;

        public const int ME_REF_AO_GROUND						= 0x00060001;
        public const int ME_REF_AO_DIFFERENTIAL					= 0x00060002;

        public const int ME_TRIG_CHAN_NONE						= 0x00000000;
        public const int ME_TRIG_CHAN_DEFAULT					= 0x00070001;
        public const int ME_TRIG_CHAN_SYNCHRONOUS				= 0x00070002;

        public const int ME_TRIG_TYPE_NONE						= 0x00000000;
        public const int ME_TRIG_TYPE_SW							= 0x00080001;
        public const int ME_TRIG_TYPE_THRESHOLD					= 0x00080002;
        public const int ME_TRIG_TYPE_WINDOW						= 0x00080003;
        public const int ME_TRIG_TYPE_EDGE						= 0x00080004;
        public const int ME_TRIG_TYPE_SLOPE						= 0x00080005;
        public const int ME_TRIG_TYPE_EXT_DIGITAL				= 0x00080006;
        public const int ME_TRIG_TYPE_EXT_ANALOG					= 0x00080007;
        public const int ME_TRIG_TYPE_PATTERN					= 0x00080008;
        public const int ME_TRIG_TYPE_TIMER						= 0x00080009;
        public const int ME_TRIG_TYPE_COUNT						= 0x0008000A;
        public const int ME_TRIG_TYPE_FOLLOW						= 0x0008000B;

        public const int ME_TRIG_EDGE_NONE						= 0x00000000;
        public const int ME_TRIG_EDGE_ABOVE						= 0x00090001;
        public const int ME_TRIG_EDGE_BELOW						= 0x00090002;
        public const int ME_TRIG_EDGE_ENTRY						= 0x00090003;
        public const int ME_TRIG_EDGE_EXIT						= 0x00090004;
        public const int ME_TRIG_EDGE_RISING						= 0x00090005;
        public const int ME_TRIG_EDGE_FALLING					= 0x00090006;
        public const int ME_TRIG_EDGE_ANY						= 0x00090007;

        public const int ME_TIMER_ACQ_START						= 0x000A0001;
        public const int ME_TIMER_SCAN_START						= 0x000A0002;
        public const int ME_TIMER_CONV_START						= 0x000A0003;

        /*==================================================================
          Defines for meIOIrqStart function
          ================================================================*/

        public const int ME_IRQ_SOURCE_DIO_DEFAULT				= 0x00000000;
        public const int ME_IRQ_SOURCE_DIO_PATTERN				= 0x000B0001;
        public const int ME_IRQ_SOURCE_DIO_MASK					= 0x000B0002;
        public const int ME_IRQ_SOURCE_DIO_LINE					= 0x000B0003;
        public const int ME_IRQ_SOURCE_DIO_OVER_TEMP				= 0x000B0004;

        public const int ME_IRQ_EDGE_NOT_USED					= 0x00000000;
        public const int ME_IRQ_EDGE_RISING						= 0x000C0001;
        public const int ME_IRQ_EDGE_FALLING						= 0x000C0002;
        public const int ME_IRQ_EDGE_ANY							= 0x000C0003;

        public const int ME_IO_IRQ_START_NO_FLAGS				= 0x0;
        public const int ME_IO_IRQ_START_DIO_BIT					= 0x1;
        public const int ME_IO_IRQ_START_DIO_BYTE				= 0x2;
        public const int ME_IO_IRQ_START_DIO_WORD				= 0x4;
        public const int ME_IO_IRQ_START_DIO_DWORD				= 0x8;
        public const int ME_IO_IRQ_START_PATTERN_FILTERING		= 0x10;
        public const int ME_IO_IRQ_START_EXTENDED_STATUS			= 0x20;

        /*==================================================================
          Defines for meIOIrqWait function
          ================================================================*/

        public const int ME_IO_IRQ_WAIT_NO_FLAGS					= 0x0;
        public const int ME_IO_IRQ_WAIT_NORMAL_STATUS			= 0x1;
        public const int ME_IO_IRQ_WAIT_EXTENDED_STATUS			= 0x2;

        /*==================================================================
          Defines for meIOIrqStop function
          ================================================================*/

        public const int ME_IO_IRQ_STOP_NO_FLAGS					= 0x0;

        /*==================================================================
          Defines for meIOIrqSetCallback function
          ================================================================*/

        public const int ME_IO_IRQ_SET_CALLBACK_NO_FLAGS			= 0x0;

        /*==================================================================
          Defines for meIOResetDevice function
          ================================================================*/

        public const int ME_IO_RESET_DEVICE_NO_FLAGS				= 0x0;
        public const int ME_IO_RESET_DEVICE_UNPROTECTED			= 0x10000;

        /*==================================================================
          Defines for meIOResetSubdevice function
          ================================================================*/

        public const int ME_IO_RESET_SUBDEVICE_NO_FLAGS			= 0x0;

        /*==================================================================
          Defines for meIOSingleConfig function
          ================================================================*/

        public const int ME_SINGLE_CONFIG_DIO_INPUT				= 0x000D0001;
        public const int ME_SINGLE_CONFIG_DIO_OUTPUT				= 0x000D0002;
        public const int ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE		= 0x000D0003;
        public const int ME_SINGLE_CONFIG_DIO_SINK				= 0x000D0004;
        public const int ME_SINGLE_CONFIG_DIO_SOURCE				= 0x000D0005;
        public const int ME_SINGLE_CONFIG_DIO_MUX32M				= 0x000D0006;
        public const int ME_SINGLE_CONFIG_DIO_DEMUX32			= 0x000D0007;
        public const int ME_SINGLE_CONFIG_DIO_BIT_PATTERN		= 0x000D0008;
        public const int ME_SINGLE_CONFIG_MULTIPIN_IRQ			= 0x000D0009;
        public const int ME_SINGLE_CONFIG_MULTIPIN_CLK			= 0x000D000a;

        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_0		= 0x000E0001;
        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_1		= 0x000E0002;
        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_2		= 0x000E0003;
        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_3		= 0x000E0004;
        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_4		= 0x000E0005;
        public const int ME_SINGLE_CONFIG_CTR_8254_MODE_5		= 0x000E0006;

        public const int ME_IO_SINGLE_CONFIG_NO_FLAGS			= 0x00;
        public const int ME_IO_SINGLE_CONFIG_DIO_BIT				= 0x01;
        public const int ME_IO_SINGLE_CONFIG_DIO_BYTE			= 0x02;
        public const int ME_IO_SINGLE_CONFIG_DIO_WORD			= 0x04;
        public const int ME_IO_SINGLE_CONFIG_DIO_DWORD			= 0x08;
        public const int ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON		= 0x10;
        public const int ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF	= 0x20;
        public const int ME_IO_SINGLE_CONFIG_AI_RMS				= 0x40;
        public const int ME_IO_SINGLE_CONFIG_CONTINUE			= 0x80;
        public const int ME_IO_SINGLE_CONFIG_MULTIPIN			= 0x100;

        /*==================================================================
          Defines for meIOSingle function
          ================================================================*/

        public const int ME_IO_SINGLE_NO_FLAGS					= 0x0;
        public const int ME_IO_SINGLE_NONBLOCKING				= 0x20;

        public const int ME_DIR_INPUT							= 0x000F0001;
        public const int ME_DIR_OUTPUT							= 0x000F0002;

        public const int ME_IO_SINGLE_TYPE_NO_FLAGS				= 0x00;
        public const int ME_IO_SINGLE_TYPE_DIO_BIT				= 0x01;
        public const int ME_IO_SINGLE_TYPE_DIO_BYTE				= 0x02;
        public const int ME_IO_SINGLE_TYPE_DIO_WORD				= 0x04;
        public const int ME_IO_SINGLE_TYPE_DIO_DWORD				= 0x08;
        public const int ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS		= 0x10;
		public const int ME_IO_SINGLE_TYPE_NONBLOCKING			= 0x20;
        public const int ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING		= 0x20;

        /*==================================================================
          Defines for meIOStreamConfig function
          ================================================================*/

        public const int ME_IO_STREAM_CONFIG_NO_FLAGS			= 0x0;
        public const int ME_IO_STREAM_CONFIG_BIT_PATTERN			= 0x1;
        public const int ME_IO_STREAM_CONFIG_WRAPAROUND			= 0x2;
        public const int ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD		= 0x4;
        public const int ME_IO_STREAM_CONFIG_HARDWARE_ONLY		= 0x8;

        public const int ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS		= 0x0;

        public const int ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS		= 0x0;

        /*==================================================================
          Defines for meIOStreamRead function
          ================================================================*/

        public const int ME_READ_MODE_BLOCKING					= 0x00100001;
        public const int ME_READ_MODE_NONBLOCKING				= 0x00100002;

        public const int ME_IO_STREAM_READ_NO_FLAGS				= 0x0;
        public const int ME_IO_STREAM_READ_FRAMES				= 0x1;

        /*==================================================================
          Defines for meIOStreamWrite function
          ================================================================*/

        public const int ME_WRITE_MODE_BLOCKING					= 0x00110001;
        public const int ME_WRITE_MODE_NONBLOCKING				= 0x00110002;
        public const int ME_WRITE_MODE_PRELOAD					= 0x00110003;

        public const int ME_IO_STREAM_WRITE_NO_FLAGS				= 0x00000000;

        /*==================================================================
          Defines for meIOStreamStart function
          ================================================================*/

        public const int ME_IO_STREAM_START_NO_FLAGS				= 0x00000000;
        public const int ME_IO_STREAM_START_NONBLOCKING			= 0x20;

        public const int ME_START_MODE_BLOCKING					= 0x00120001;
        public const int ME_START_MODE_NONBLOCKING				= 0x00120002;

        public const int ME_IO_STREAM_START_TYPE_NO_FLAGS		= 0x0;
        public const int ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS= 0x1;

        /*==================================================================
          Defines for meIOStreamStop function
          ================================================================*/

        public const int ME_IO_STREAM_STOP_NO_FLAGS				= 0x00000000;
        public const int ME_IO_STREAM_STOP_NONBLOCKING			= 0x20;

        public const int ME_STOP_MODE_IMMEDIATE					= 0x00130001;
        public const int ME_STOP_MODE_LAST_VALUE					= 0x00130002;

        public const int ME_IO_STREAM_STOP_TYPE_NO_FLAGS			= 0x00000000;
        public const int ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS	= 0x00000001;

        /*==================================================================
          Defines for meIOStreamStatus function
          ================================================================*/

        public const int ME_WAIT_NONE							= 0x00140001;
        public const int ME_WAIT_IDLE							= 0x00140002;
        public const int ME_WAIT_BUSY							= 0x00140003;

        public const int ME_STATUS_INVALID						= 0x00000000;
        public const int ME_STATUS_IDLE							= 0x00150001;
        public const int ME_STATUS_BUSY							= 0x00150002;
        public const int ME_STATUS_ERROR							= 0x00150003;

        public const int ME_IO_STREAM_STATUS_NO_FLAGS			= 0x00000000;

        /*==================================================================
          Defines for meIOStreamSetCallbacks function
          ================================================================*/

        public const int ME_IO_STREAM_SET_CALLBACKS_NO_FLAGS		= 0x00000000;


        /*==================================================================
          Defines for meIOFrequencyToTicks function
          ================================================================*/

        public const int ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS		= 0x00000000;
        public const int ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS= 0x00000000;
		
        /*==================================================================
          Defines for meIOTimeToTicks function
          ================================================================*/

        public const int ME_IO_TIME_TO_TICKS_NO_FLAGS		= 0x00000000;
        public const int ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS	= 0x00000000;

        /*==================================================================
          Defines for module types
          ================================================================*/

        public const int ME_MODULE_TYPE_MULTISIG_NONE					= 0x00000000;
        public const int ME_MODULE_TYPE_MULTISIG_DIFF16_10V				= 0x00160001;
        public const int ME_MODULE_TYPE_MULTISIG_DIFF16_20V				= 0x00160002;
        public const int ME_MODULE_TYPE_MULTISIG_DIFF16_50V				= 0x00160003;
        public const int ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA		= 0x00160004;
        public const int ME_MODULE_TYPE_MULTISIG_RTD8_PT100				= 0x00160005;
        public const int ME_MODULE_TYPE_MULTISIG_RTD8_PT500				= 0x00160006;
        public const int ME_MODULE_TYPE_MULTISIG_RTD8_PT1000				= 0x00160007;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B				= 0x00160008;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E				= 0x00160009;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J				= 0x0016000A;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K				= 0x0016000B;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N				= 0x0016000C;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R				= 0x0016000D;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S				= 0x0016000E;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T				= 0x0016000F;
        public const int ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR			= 0x00160010;

        /*==================================================================
          Defines for meQuerySubdeviceCaps function
          ================================================================*/

        public const int ME_CAPS_DNONE							= 0x00000000;

        public const int ME_CAPS_TRIG_DIGITAL					= 0x00008000;
        public const int ME_CAPS_TRIG_ANALOG						= 0x00010000;
        public const int ME_CAPS_TRIG_EDGE_RISING				= 0x00020000;
        public const int ME_CAPS_TRIG_EDGE_FALLING				= 0x00040000;
        public const int ME_CAPS_TRIG_EDGE_ANY					= 0x00080000;
		
        public const int ME_CAPS_DIO_DIR_BIT						= 0x00000001;
        public const int ME_CAPS_DIO_DIR_BYTE					= 0x00000002;
        public const int ME_CAPS_DIO_DIR_WORD					= 0x00000004;
        public const int ME_CAPS_DIO_DIR_DWORD					= 0x00000008;
        public const int ME_CAPS_DIO_SINK_SOURCE					= 0x00000010;
        public const int ME_CAPS_DIO_BIT_PATTERN_IRQ				= 0x00000020;
        public const int ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING	= 0x00000040;
        public const int ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING	= 0x00000080;
        public const int ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY		= 0x00000100;
        public const int ME_CAPS_DIO_OVER_TEMP_IRQ				= 0x00000200;
        public const int ME_CAPS_DIO_TRIG_SYNCHRONOUS			= 0x00004000;
        public const int ME_CAPS_DIO_TRIG_DIGITAL				= 0x00008000;
        public const int ME_CAPS_DIO_TRIG_ANALOG					= 0x00010000;
        public const int ME_CAPS_DIO_TRIG_EDGE_RISING			= 0x00020000;
        public const int ME_CAPS_DIO_TRIG_EDGE_FALLING			= 0x00040000;
        public const int ME_CAPS_DIO_TRIG_EDGE_ANY				= 0x00080000;

        public const int ME_CAPS_CTR_CLK_PREVIOUS				= 0x00000001;
        public const int ME_CAPS_CTR_CLK_INTERNAL_1MHZ			= 0x00000002;
        public const int ME_CAPS_CTR_CLK_INTERNAL_10MHZ			= 0x00000004;
        public const int ME_CAPS_CTR_CLK_EXTERNAL				= 0x00000008;

        public const int ME_CAPS_AI_TRIG_SYNCHRONOUS				= 0x00000001;
        public const int ME_CAPS_AI_TRIG_SIMULTANEOUS			= 0x00000001;
        public const int ME_CAPS_AI_FIFO							= 0x00000002;
        public const int ME_CAPS_AI_FIFO_THRESHOLD				= 0x00000004;
        public const int ME_CAPS_AI_TRIG_DIGITAL					= 0x00008000;
        public const int ME_CAPS_AI_TRIG_ANALOG					= 0x00010000;
        public const int ME_CAPS_AI_TRIG_EDGE_RISING				= 0x00020000;
        public const int ME_CAPS_AI_TRIG_EDGE_FALLING			= 0x00040000;
        public const int ME_CAPS_AI_TRIG_EDGE_ANY				= 0x00080000;
                                                                                
        public const int ME_CAPS_AO_TRIG_SYNCHRONOUS				= 0x00000001;
        public const int ME_CAPS_AO_TRIG_SIMULTANEOUS			= 0x00000001;
        public const int ME_CAPS_AO_FIFO							= 0x00000002;
        public const int ME_CAPS_AO_FIFO_THRESHOLD				= 0x00000004;
        public const int ME_CAPS_AO_TRIG_DIGITAL					= 0x00008000;
        public const int ME_CAPS_AO_TRIG_ANALOG					= 0x00010000;
        public const int ME_CAPS_AO_TRIG_EDGE_RISING				= 0x00020000;
        public const int ME_CAPS_AO_TRIG_EDGE_FALLING			= 0x00040000;
        public const int ME_CAPS_AO_TRIG_EDGE_ANY				= 0x00080000;

        public const int ME_CAPS_EXT_IRQ_EDGE_RISING				= 0x00000001;
        public const int ME_CAPS_EXT_IRQ_EDGE_FALLING			= 0x00000002;
        public const int ME_CAPS_EXT_IRQ_EDGE_ANY				= 0x00000004;

        /*==================================================================
          Defines for meQuerySubdeviceCapsArgs function
          ================================================================*/

        public const int ME_CAP_AI_FIFO_SIZE						= 0x001D0000;
        public const int ME_CAP_AI_BUFFER_SIZE					= 0x001D0001;
        public const int ME_CAP_AI_CHANNEL_LIST_SIZE				= 0x001D0002;
        public const int ME_CAP_AI_MAX_TRESHOLD_SIZE				= 0x001D0003;

        public const int ME_CAP_AO_FIFO_SIZE						= 0x001F0000;
        public const int ME_CAP_AO_BUFFER_SIZE					= 0x001F0001;
        public const int ME_CAP_AO_CHANNEL_LIST_SIZE				= 0x001F0002;
        public const int ME_CAP_AO_MAX_TRESHOLD_SIZE				= 0x001F0003;

        public const int ME_CAP_CTR_WIDTH						= 0x00200000;

        /*==================================================================
          Defines common to query functions
          ================================================================*/

        public const int ME_UNIT_INVALID							= 0x00000000;
        public const int ME_UNIT_VOLT							= 0x00170001;
        public const int ME_UNIT_AMPERE							= 0x00170002;
        public const int ME_UNIT_ANY								= 0x00170003;

        public const int ME_TYPE_INVALID							= 0x00000000;
        public const int ME_TYPE_AO								= 0x00180001;
        public const int ME_TYPE_AI								= 0x00180002;
        public const int ME_TYPE_DIO								= 0x00180003;
        public const int ME_TYPE_DO								= 0x00180004;
        public const int ME_TYPE_DI								= 0x00180005;
        public const int ME_TYPE_CTR								= 0x00180006;
        public const int ME_TYPE_EXT_IRQ							= 0x00180007;

        public const int ME_SUBTYPE_INVALID						= 0x00000000;
        public const int ME_SUBTYPE_SINGLE						= 0x00190001;
        public const int ME_SUBTYPE_STREAMING					= 0x00190002;
        public const int ME_SUBTYPE_CTR_8254						= 0x00190003;
        public const int ME_SUBTYPE_ANY							= 0x00190004;

        public const int ME_DEVICE_DRIVER_NAME_MAX_COUNT			= 64;
        public const int ME_DEVICE_NAME_MAX_COUNT				= 64;

        public const int ME_DEVICE_DESCRIPTION_MAX_COUNT			= 256;

        public const int ME_BUS_TYPE_INVALID						= 0x00000000;
        public const int ME_BUS_TYPE_PCI							= 0x001A0001;
        public const int ME_BUS_TYPE_USB							= 0x001A0002;

        public const int ME_PLUGGED_INVALID						= 0x00000000;
        public const int ME_PLUGGED_ANY							= 0x001B0000;
        public const int ME_PLUGGED_IN							= 0x001B0001;
        public const int ME_PLUGGED_OUT							= 0x001B0002;

        public const int ME_EXTENSION_TYPE_INVALID				= 0x00000000;
        public const int ME_EXTENSION_TYPE_NONE					= 0x001C0001;
        public const int ME_EXTENSION_TYPE_MUX32M				= 0x001C0002;
        public const int ME_EXTENSION_TYPE_DEMUX32				= 0x001C0003;
        public const int ME_EXTENSION_TYPE_MUX32S				= 0x001C0004;

        public const int ME_ACCESS_TYPE_INVALID					= 0x00000000;
        public const int ME_ACCESS_TYPE_LOCAL					= 0x001D0001;
        public const int ME_ACCESS_TYPE_REMOTE					= 0x001D0002;

        public const int ME_CONF_LOAD_CUSTOM_DRIVER				= 0x00001000;
        public const int ME_PWM_START_NO_FLAGS					= 0x00000000;
        public const int ME_PWM_START_CONNECT_INTERNAL			= 0x00000001;
        public const int ME_QUERY_NO_FLAGS						= 0x00000000;
        public const int ME_ERRNO_CLEAR_FLAGS					= 0x00000001;

        #endregion

        #region Error codes

        public const int ME_ERRNO_SUCCESS					    = 0;
        public const int ME_ERRNO_INVALID_DEVICE				    = 1;
        public const int ME_ERRNO_INVALID_SUBDEVICE			    = 2;
        public const int ME_ERRNO_INVALID_CHANNEL			    = 3;
        public const int ME_ERRNO_INVALID_SINGLE_CONFIG		    = 4;
        public const int ME_ERRNO_INVALID_REF				    = 5;
        public const int ME_ERRNO_INVALID_TRIG_CHAN			    = 6;
        public const int ME_ERRNO_INVALID_TRIG_TYPE			    = 7;
        public const int ME_ERRNO_INVALID_TRIG_EDGE			    = 8;
        public const int ME_ERRNO_INVALID_TIMEOUT			    = 9;
        public const int ME_ERRNO_INVALID_FLAGS				    = 10;
        public const int ME_ERRNO_OPEN						    = 11;
        public const int ME_ERRNO_CLOSE						    = 12;
        public const int ME_ERRNO_NOT_OPEN					    = 13;
        public const int ME_ERRNO_INVALID_DIR				    = 14;
        public const int ME_ERRNO_PREVIOUS_CONFIG			    = 15;
        public const int ME_ERRNO_NOT_SUPPORTED				    = 16;
        public const int ME_ERRNO_SUBDEVICE_TYPE				    = 17;
        public const int ME_ERRNO_USER_BUFFER_SIZE			    = 18;
        public const int ME_ERRNO_LOCKED						    = 19;
        public const int ME_ERRNO_NOMORE_SUBDEVICE_TYPE		    = 20;
        public const int ME_ERRNO_TIMEOUT					    = 21;
        public const int ME_ERRNO_SIGNAL						    = 22;
        public const int ME_ERRNO_INVALID_IRQ_SOURCE			    = 23;
        public const int ME_ERRNO_THREAD_RUNNING				    = 24;
        public const int ME_ERRNO_START_THREAD				    = 25;
        public const int ME_ERRNO_CANCEL_THREAD				    = 26;
        public const int ME_ERRNO_NO_CALLBACK				    = 27;
        public const int ME_ERRNO_USED						    = 28;
        public const int ME_ERRNO_INVALID_UNIT				    = 29;
        public const int ME_ERRNO_INVALID_MIN_MAX			    = 30;
        public const int ME_ERRNO_NO_RANGE					    = 31;
        public const int ME_ERRNO_INVALID_RANGE				    = 32;
        public const int ME_ERRNO_SUBDEVICE_BUSY				    = 33;
        public const int ME_ERRNO_INVALID_LOCK				    = 34;
        public const int ME_ERRNO_INVALID_SWITCH				    = 35;
        public const int ME_ERRNO_INVALID_ERROR_MSG_COUNT	    = 36;
        public const int ME_ERRNO_INVALID_STREAM_CONFIG		    = 37;
        public const int ME_ERRNO_INVALID_CONFIG_LIST_COUNT	    = 38;
        public const int ME_ERRNO_INVALID_ACQ_START_TRIG_TYPE	= 39;
        public const int ME_ERRNO_INVALID_ACQ_START_TRIG_EDGE	= 40;
        public const int ME_ERRNO_INVALID_ACQ_START_TRIG_CHAN	= 41;
        public const int ME_ERRNO_INVALID_ACQ_START_TIMEOUT		= 42;
        public const int ME_ERRNO_INVALID_ACQ_START_ARG			= 43;
        public const int ME_ERRNO_INVALID_SCAN_START_TRIG_TYPE	= 44;
        public const int ME_ERRNO_INVALID_SCAN_START_ARG			= 45;
        public const int ME_ERRNO_INVALID_CONV_START_TRIG_TYPE	= 46;
        public const int ME_ERRNO_INVALID_CONV_START_ARG			= 47;
        public const int ME_ERRNO_INVALID_SCAN_STOP_TRIG_TYPE	= 48;
        public const int ME_ERRNO_INVALID_SCAN_STOP_ARG		    = 49;
        public const int ME_ERRNO_INVALID_ACQ_STOP_TRIG_TYPE	    = 50;
        public const int ME_ERRNO_INVALID_ACQ_STOP_ARG		    = 51;
        public const int ME_ERRNO_SUBDEVICE_NOT_RUNNING		    = 52;
        public const int ME_ERRNO_INVALID_READ_MODE			    = 53;
        public const int ME_ERRNO_INVALID_VALUE_COUNT		    = 54;
        public const int ME_ERRNO_INVALID_WRITE_MODE			    = 55;
        public const int ME_ERRNO_INVALID_TIMER				    = 56;
        public const int ME_ERRNO_DEVICE_UNPLUGGED			    = 57;
        public const int ME_ERRNO_USED_INTERNAL				    = 58;
        public const int ME_ERRNO_INVALID_DUTY_CYCLE			    = 59;
        public const int ME_ERRNO_INVALID_WAIT				    = 60;
        public const int ME_ERRNO_CONNECT_REMOTE				    = 61;
        public const int ME_ERRNO_COMMUNICATION				    = 62;
        public const int ME_ERRNO_INVALID_SINGLE_LIST		    = 63;
        public const int ME_ERRNO_INVALID_MODULE_TYPE		    = 64;
        public const int ME_ERRNO_INVALID_START_MODE			    = 65;
        public const int ME_ERRNO_INVALID_STOP_MODE			    = 66;
        public const int ME_ERRNO_INVALID_FIFO_IRQ_THRESHOLD	    = 67;
        public const int ME_ERRNO_INVALID_POINTER			    = 68;
        public const int ME_ERRNO_CREATE_EVENT				    = 69;
        public const int ME_ERRNO_LACK_OF_RESOURCES			    = 70;
        public const int ME_ERRNO_CANCELLED					    = 71;
        public const int ME_ERRNO_RING_BUFFER_OVERFLOW		    = 72;
        public const int ME_ERRNO_RING_BUFFER_UNDEFFLOW		    = 73;
        public const int ME_ERRNO_INVALID_IRQ_EDGE			    = 74;
        public const int ME_ERRNO_INVALID_IRQ_ARG			    = 75;
        public const int ME_ERRNO_INVALID_CAP				    = 76;
        public const int ME_ERRNO_INVALID_CAP_ARG_COUNT		    = 77;
        public const int ME_ERRNO_INTERNAL					    = 78;                                                                             public const int ME_ERRNO_VALUE_OUT_OF_RANGE		        = 79;
        public const int ME_ERRNO_HARDWARE_BUFFER_OVERFLOW		= 80;
        public const int ME_ERRNO_HARDWARE_BUFFER_UNDERFLOW		= 81;
        public const int ME_ERRNO_CONFIG_LOAD_FAILED		        = 82;
        public const int ME_ERRNO_INVALID_ERROR_NUMBER			= 83;

        #endregion

        #region Typen

        #region IOSingle

        /// <summary>
        ///    Diese Struktur enthält alle notwendigen Parameter um einen Einzelwert
        ///    an einer beliebiegen Komponente auszugeben oder einzulesen.
        /// </summary>
        /// <remarks>
        ///    <para>
        ///       Verwenden Sie diese Struktur bitte nur wenn sie genau wissen was
        ///       die einzelnen Flags bewirken. Wollen Sie allerdings eine generische
        ///       Applikation erstellen und die zu benutzenden Parameter nur in einer
        ///       gemeinsamen Variablen speichern, so bietet sich diese Struktur an.
        ///       Ungültige oder widersprüchliche Kombinationen müssen allerdings in
        ///       diesem Falle jedoch selbst von Ihnen abgefangen werden.
        ///    </para>
        ///    <para>
        ///       Für weniger erfahrene Anwender sind die Strukturen
        ///       <see cref="IOSingleDIOParams"/> bereits konkretisierter und dieser
        ///       Struktur vorzuziehen.
        ///    </para>
        /// </remarks>
        /// <seealso cref="Meilhaus::Driver::IOSingle(ref IOSingleParams, IOSingleFlags)"/>
        /// <seealso cref="Meilhaus::Net::Driver::IOSingle(ref IOSingleParams, IOSingleFlags)"/>
        [StructLayout(LayoutKind.Sequential)]
        public struct meIOSingle_t
        {
            /// <summary>
            ///    Der Index des anzusprechenden Gerätes.
            /// </summary>
            /// <remarks>
            ///     Der angegebene Wert muss im Bereich zwischen 0 und
            ///    <c><see cref="Meilhaus::Driver::QueryNumberDevices()"/> - 1</c> bzw.
            ///    <c><see cref="Meilhaus::Net::Driver::QueryNumberDevices()"/> - 1</c> liegen.
            /// </remarks>
            public int iDevice;


            /// <summary>
            ///    Der Index der gewünschten Komponente.
            /// </summary>
            /// <remarks>
            ///    Der angegebene Wert muss im Bereich zwischen 0 und
            ///    <c><see cref="Meilhaus::Driver::QueryNumberSubdevices()"/> - 1</c> bzw.
            ///    <c><see cref="Meilhaus::Net::Driver::QueryNumberSubdevices()"/> - 1</c> liegen.
            /// </remarks>
            public int iSubdevice;


            /// <summary>
            ///    Der Index des Ports auf welchem der Wert #Value ausgegeben
            ///    (#Dir = Dir::Output) bzw. eingelesen (#Dir = Dir::Input) wird.
            /// </summary>
            /// <remarks>
            ///    Der angegebene Wert muss im Bereich zwischen 0 und
            ///    <c><see cref="Meilhaus::Driver::QueryNumberChannels()"/> - 1</c> bzw.
            ///    <c><see cref="Meilhaus::Net::Driver::QueryNumberChannels()"/> - 1</c> liegen.
            /// </remarks>
            public int iChannel;


            /// <summary>
            ///    Legt die Operation als Ein- oder Ausgabe fest.
            /// </summary>
            /// <remarks>
            ///    <para>
            ///       Unterstützt die Komponente lediglich eine Erfassung, so ist
            ///       lediglich der Wert Dir::Input erlaubt.
            ///    </para>
            ///    <para>
            ///       Kann die Komponente Daten ausgeben, so spezifiziert die Konstante
            ///       Dir::Output den zu setzenden Wert. Soll der aktuelle bzw. der
            ///       zuletzt gesetzte Wert nochmals ermittelt werden, so kann mittels
            ///       Dir::Input dieser wieder bestimmt werden.
            ///    </para>
            ///    <para>
            ///       Unterstützt eine Komponente die Ein- und Ausgabe, so hängen die
            ///       gültigen Werte davon ab wie der Port mittels <c><see cref="Driver"/>::::IOSingleConfig</c>
            ///       bzw. <c><see cref="Meilhaus::Net::Driver"/>::::IOSingleConfig</c> konfiguriert
            ///       wurde. Und eine der beiden vorherigen Aussagen sind wieder anzuwenden.
            ///    </para>
            /// </remarks>
            public int iDir;


            /// <summary>
            ///    Bei einer Ausgabeoperation (vgl. #Dir) muss hier vor Aufruf der
            ///    Funktion <c><see cref="Meilhaus::Driver::IOSingle(ref IOSingleParams, IOSingleFlags)"/></c>
            ///    bzw. <c><see cref="Meilhaus::Net::Driver::IOSingle(ref IOSingleParams, IOSingleFlags)"/></c>
            ///    der zu setzende Wert gespeichert werden. Soll im Gegensatz dazu
            ///    ein Wert erfasst werden, so kann diese Variable ignoriert werden.
            ///    Nach erfolgreichem Aufruf von <c><see cref="Meilhaus::Driver::IOSingle(ref IOSingleParams, IOSingleFlags)"/></c>
            ///    enthält diese anschließend den bestimmten Messwert.
            /// </summary>
            public int iValue;


            /// <summary>
            ///    Zeit in Millisekunden nachdem mit der nächsten Operation fortgesetzt
            ///    wird.
            /// </summary>
            /// <remarks>
            ///    Wurde in <c><see cref="Meilhaus::Driver"/>::::IOSingleConfig</c> bzw.
            ///    <c><see cref="Meilhaus::Net::Driver"/>::::IOSingleConfig</c> keine externen
            ///    Trigger konfiguriert, so wird ein evtl. enthaltener Wert in
            ///    diesem Struktur-Mitglied ignoriert.
            /// </remarks>
            public int iTimeOut;


            /// <summary>
            ///    Hier lassen sich weitere Optionen angeben welche das Verhalten
            ///    bei der Ausführung beeinflussen.
            /// </summary>
            /// <remarks>
            ///    Beachten Sie bitte hier, dass nicht alle Optionen von jeder
            ///    Komponente unterstützt werden.
            /// </remarks>
            public int iFlags;


            /// <summary>
            ///    Keine Bedeutung vor Aufruf von <c><see cref="Meilhaus::Driver::IOSingleConfig"/></c>
            ///    bzw. <c><see cref="Meilhaus::Net::Driver::IOSingleConfig"/></c>. Nach Aufruf
            ///    kann diese einen möglichen Fehlercode enthalten.
            /// </summary>
            public int iErrno;
        }

        #endregion

        /// <summary>
        /// TODO
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct meIOStreamConfig_t
        {
            public int iChannel;
            public int iStreamConfig;
            public int iRef;
            public int iFlags;
        }

        /// <summary>
        /// TODO
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct meIOStreamStart_t
        {
            /// <summary>
            /// TODO
            /// </summary>
            public int iDevice;

            /// <summary>
            /// TODO
            /// </summary>
            public int iSubdevice;

            /// <summary>
            /// TODO
            /// </summary>
            public int iStartMode;

            /// <summary>
            /// TODO
            /// </summary>
            public int iTimeOut;

            /// <summary>
            /// TODO
            /// </summary>
            public int iFlags;

            /// <summary>
            /// TODO
            /// </summary>
            public int iErrno;
        }


        [StructLayout(LayoutKind.Sequential)]
        public struct meIOStreamStop_t
        {
            public int iDevice;
            public int iSubdevice;
            public int iStopMode;
            public int iFlags;
            public int iErrno;
        }


        /// <summary>
        /// TODO
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        public struct meIOStreamTrigger_t
        {
            public int iAcqStartTrigType;
            public int iAcqStartTrigEdge;
            public int iAcqStartTrigChan;
            public int iAcqStartTicksLow;
            public int iAcqStartTicksHigh;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I4, SizeConst = 10)]
            public int[] iAcqStartArgs;
            public int iScanStartTrigType;
            public int iScanStartTicksLow;
            public int iScanStartTicksHigh;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I4, SizeConst = 10)]
            public int[] iScanStartArgs;
            public int iConvStartTrigType;
            public int iConvStartTicksLow;
            public int iConvStartTicksHigh;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I4, SizeConst = 10)]
            public int[] iConvStartArgs;
            public int iScanStopTrigType;
            public int iScanStopCount;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I4, SizeConst = 10)]
            public int[] iScanStopArgs;
            public int iAcqStopTrigType;
            public int iAcqStopCount;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I4, SizeConst = 10)]
            public int[] iArcStopArgs;
            public int iFlags;
        }

        #endregion


        #region Delegaten

        /// <summary>
        /// TODO
        /// </summary>
        /// <param name="functionName"></param>
        /// <param name="errorCode"></param>
        /// <returns></returns>
        public delegate int meErrorCB_t(string functionName, int errorCode);

        /// <summary>
        /// TODO
        /// </summary>
        /// <param name="deviceIndex"></param>
        /// <param name="subdeviceIndex"></param>
        /// <param name="count"></param>
        /// <param name="context"></param>
        /// <param name="errorCode"></param>
        /// <returns></returns>
        public delegate int meIOStreamCB_t(int iDevice, int iSubdevice, int iCount, IntPtr piContext, int iErrorCode);


        /// <summary>
        /// TODO
        /// </summary>
        /// <param name="deviceIndex"></param>
        /// <param name="subdeviceIndex"></param>
        /// <param name="channelIndex"></param>
        /// <param name="irqCount"></param>
        /// <param name="value"></param>
        /// <param name="context"></param>
        /// <param name="errorCode"></param>
        /// <returns></returns>
        public delegate int meIOIrqCB_t(int iDevice, int iSubdevice, int IChannel, int iIrqCount, int ivalue, IntPtr piContext, int iErrorCode);

        #endregion

    /// <summary>
    /// Vorlage für die Verwendung des Meilhaus Treibersystems aus C#-Projekten.
    /// Stand: 21. Februar 2007
    /// </summary>

        #region meClose

        /// <summary>
        /// <c><see cref="Close()"/></c> gibt alle verwendeten und reservierten Ressourcen
        /// wieder frei. Bitte stellen sie sicher, dass diese Funktion in ihrer
        /// Applikation aufgerufen bevor diese beendet wird. Dies sollte nach den
        /// Richtlinien von
        /// <a href="http://msdn2.microsoft.com/de-de/library/fs2xkftw(VS.80).aspx">Microsoft</a>
        /// in der Methode
        /// <c><a class="el" href="http://msdn2.microsoft.com/de-de/library/system.idisposable.dispose(VS.80).aspx">Dispose(bool)</a></c>
        /// erfolgen.
        /// </summary>
        /// <example>
        /// Beispiel:
        /// <code>
        /// protected override void Dispose(bool disposing)
        /// {
        ///    try
        ///    {
        ///       if (disposing)
        ///       {
        ///          ...        //Freigabe von verwalteten Ressourcen
        ///       }
        ///       ...           //Freigabe von unverwalteten Ressourcen
        ///       if (Meilhaus.Driver.Close(Meilhaus.Driver.CloseFlags.NoFlags) != Meilhaus.ErrNo.Success)
        ///       {
        ///          ...        //Fehlerbehandlung (keine Ausnahme auslösen!)
        ///       }
        ///    }
        ///    finally
        ///    {
        ///       base.Dispose(disposing);
        ///    }
        /// }
        /// </code>
        /// </example>
        /// <param name="flags">
        /// Momentan immer <c><see cref="NoFlags"/></c>.
        /// </param>
        /// <returns>
        /// Konnten alle Ressourcen wieder freigegeben werden, so wird als Rückgabewert
        /// <c><see cref="Success"/></c> zurückgegeben.
        /// </returns>
        /// <seealso cref="Open"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meClose(int flags);

        #endregion


        #region meErrorGetLastMessage

        /// <summary>
        /// Diese Funktion ist gedacht für einen Aufruf unmittelbar nach einem
        /// vorherigem fehlgeschlagenen Funktionsaufruf.
        /// </summary>
        /// <param name="message">
        /// <para>Eine Referenz auf eine Variable vom Typ
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">string</a></c> bzw.
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">String</a></c>.</para>
        /// <para>(Die Variable braucht nicht initialisiert zu sein; kann also den
        /// Wert null besitzen)</para>
        /// </param>
        /// <returns>
        /// Nur wenn der Rückgabetyp <c><see cref="Success"/></c> zurückgegeben wird,
        /// enthält die übergebene Variable die Fehlermeldung.
        /// </returns>
        /// <seealso cref="ErrorGetMessage"/>
//        [DllImport("meIDSmain.dll")]
//        private static extern int meErrorGetLastMessage(StringBuilder buffer, int count);

        #endregion


        #region meErrorGetMessage

        /// <summary>
        /// Konvertiert einen Fehlercode in eine Zeichenkette.
        /// </summary>
        /// <param name="errNo">
        /// Ein numerischer Wert welcher jedoch als Konstante der Aufzählung
        /// <c><see cref="ErrNo"/></c> angegeben werden sollte.</param>
        /// <param name="message">
        /// <para>Eine Referenz auf eine Variable vom Typ
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">string</a></c> bzw.
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">String</a></c>.</para>
        /// <para>(Die Variable braucht nicht initialisiert zu sein; kann also den
        /// Wert null besitzen)</para>
        /// </param>
        /// <returns>
        /// Nur wenn der Rückgabetyp <c><see cref="Success"/></c> zurückgegeben wird,
        /// enthält die übergebene Variable <paramref name="message"/> message
        /// die Fehlermeldung.
        /// </returns>
        /// <seealso cref="ErrorGetLastMessage"/>
//        [DllImport("meIDSmain.dll")]
//        private static extern int meErrorGetMessage(int errorCode, StringBuilder msg, int count);

        #endregion


        #region meErrorSetDefaultProc

        /// <summary>
        /// Aktiviert oder Deaktiviert die installierte Funktion mittels
        /// <c><see cref="ErrorSetUserProc"/></c>.
        /// </summary>
        /// <param name="value">
        /// Eine Konstante der Auflistung <c><see cref="Switch"/></c>.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meErrorSetDefaultProc(int set);

        #endregion


        #region meErrorSetUserProc

        /// <summary>
        /// Registriert eine benutzerdefinierte Funktion welche immer dann
        /// aufgerufen wird wenn ein Fehler auftritt. Voraussetzung hierfür
        /// ist allerdings, dass die Verwendung der globalen Fehlerroutine in
        /// <c><see cref="ErrorSetDefaultProc"/></c> deaktiviert wurde.
        /// <remarks>
        /// Sobald sie eine .NET-Funktion registriert haben, müssen Sie
        /// unbedingt darauf achten, dass der verwendete Delegat nicht
        /// freigegeben wird solange dieser verwendet wird. Unterbinden können
        /// dies mit dem Befehl
        /// <c><a class="el" href="http://msdn.microsoft.com/library/deu/default.asp?url=/library/deu/cpref/html/frlrfsystemgcclasskeepalivetopic.asp">GC.KeepAlive(object)</a></c>.
        /// </remarks>
        /// </summary>
        /// <param name="errorProc">
        /// Ein Delegat welcher auf die aufzurufende Methode verweist.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meErrorSetUserProc(meErrorCB_t errorProc);

        #endregion


        #region meIOIrqSetCallback

        /// <summary>
        /// Registriert eine Methode für eine bestimmte Komponente. Diese wird
        /// aufgerufen wenn <see cref="IOIrqStart(uint, uint, uint, IRQSourceDIO, IRQEdge, uint, IOIRQStartFlags)"/> aufgerufen wird und die
        /// festgelegten Bedingungen zutreffen.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="callback">
        /// Geben Sie hier den Delegate an, welcher auf die aufzurufende Methode
        /// verweist.</param>
        /// <param name="callbackContext">
        /// Benötigen Sie keine zusätzlichen Informationen über den aktuellen
        /// Systemzustand, so können Sie hier null übergeben. Andernfalls gehen
        /// Sie folgendermaßen vor um ein gewünschtes Objekt der Ereignismethode
        /// zu übergeben
        /// <example>
        /// (siehe auch
        /// <a href="http://msdn.microsoft.com:80/msdnmag/issues/02/08/CQA/">Call Unmanaged DLLs from C#, Killing Processes Cleanly</a>).
        /// <code>
        /// {
        ///    ...
        ///    GCHandle objectHandle = new GCHandle();
        ///    try
        ///    {
        ///       objectHandle = GCHandle.Alloc(yourObject);
        ///       if (Meilhaus.Driver.IOIrqSetCallback(deviceIndex,
        ///                                            subdeviceIndex,
        ///                                            callback, GCHandle.ToIntPtr(objectHandle),
        ///                                            Meilhaus.Driver.IOIrqSetCallbackFlags.NoFlags) != Meilhaus.Errno.Success)
        ///    }
        ///    finally
        ///    {
        ///       if (objectHandle.IsAllocated)
        ///          objectHandle.Free();               //Manuell angelegter Speicherbereich hier wieder freigeben
        ///    }
        ///    ...
        /// }
        /// 
        /// private (static) int EventMethod(uint deviceIndex, uint subdeviceIndex, uint channelIndex, uint irqCount, uint value, IntPtr context, ErrNo errorCode)
        /// {
        ///    GCHandle objectHandle = GCHandle.FromIntPtr(context);        //Schreibweise A
        ///    GCHandle objectHandle = (GCHandle)context;                   //Schreibweise B
        ///    yourClass yourObject = (yourClass)handle.Target;
        ///    ...
        /// }
        /// </code>
        /// </example>
        /// </param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOIrqStart"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOIrqSetCallback(int device, int subdevice, meIOIrqCB_t callback, IntPtr callbackContext, int flags);

        #endregion


        #region meIOIrqStart

        /// <summary>
        /// Aktiviert die Interrupt-Verarbeitung der angegebenen Komponente.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="channelIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberChannels()"/> - 1</c>.</param>
        /// <param name="irqSource">Wählt die Interrupt-Quelle.</param>
        /// <param name="irqEdge">Wählt die gewünschte Flanke.</param>
        /// <param name="irqArg">Bedingung für Interruptauslösung.</param>
        /// <param name="flags">Weitere Einstellmöglichkeiten.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOIrqSetCallback"/>
        /// <seealso cref="IOIrqStop"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOIrqStart(int device, int subdevice, int channel, int irqSource, int irqEdge, int irqArg, int flags);

        #endregion


        #region meIOIrqStop

        /// <summary>
        /// Stopt einen zuvor gestarteten
        /// <c><see cref="IOIrqStart(uint, uint, uint, IRQSourceDIO, IRQEdge, uint, IOIRQStartFlags)"/></c>
        /// Vorgang wieder.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="channelIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberChannels()"/> - 1</c>.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOIrqStart"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOIrqStop(int device, int subdevice, int channel, int flags);

        #endregion


        #region meIOIrqWait

        /// <summary>
        /// Wartet bis ein Interrupt eintrifft.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="channelIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberChannels()"/> - 1</c>.</param>
        /// <param name="irqCount">
        /// Anzahl der aufgetretenen Interrupts.</param>
        /// <param name="value">
        /// Status-Wert (Hardware spezifisch).</param>
        /// <param name="timeout">
        /// Maximale Wartezeit in Sekunden.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOIrqStart"/>
        /// <seealso cref="IOIrqSetCallback"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOIrqWait(int device, int subdeviceI, int channel, out int irqCount, out int value, int timeout, int flags);

        #endregion


        #region meIOResetDevice

        /// <summary>
        /// Alle laufenden Operationen des spezifizierten Geräts werden abgebrochen.
        /// Das Gerät wird in den Grundzustand versetzt.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOResetSubdevice"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOResetDevice(int device, int flags);

        #endregion


        #region meIOResetSubdevice

        /// <summary>
        /// Setzt die angegebene Komponente zurück.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOResetSubdevice(int device, int subdevice, int flags);

        #endregion


        #region meIOSingle

        [DllImport("meIDSmain.dll")]
        public static extern int meIOSingle(ref meIOSingle_t singleList, int count, int flags);

        #endregion


        #region meIOSingleConfig

        /// <summary>
        /// Konfiguration von Ein-/Ausgängen.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="channelIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberChannels()"/> - 1</c>.</param>
        /// <param name="config">
        /// Generelle Arbeitsweise des digitalen Ports.</param>
        /// <param name="reference">
        /// Die hier verwendeten Parameter spielen nur eine Rolle bei Ausgabe
        /// eines Musters.</param>
        /// <param name="trigChan">Art der Triggerung.</param>
        /// <param name="trigType">Startpunkt für Triggerung.</param>
        /// <param name="trigEdge">
        /// Sofern der Digitalport eine externe Triggerung unterstützt, kann hier
        /// die Flanke angegeben werden.</param>
        /// <param name="flags">Portbreite</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOStreamConfig"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOSingleConfig(int device, int subdevice, int channel, int singleConfig, int refrence, int trigChan, int trigType, int trigEdge, int flags);


        #endregion


        #region meIOStreamConfig

        /// <summary>
        /// Konfiguratoion von timergesteuerte Erfassung.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="configValue">
        /// Ein einzulesenden Einzelwert.
        /// </param>
        /// <param name="trigger">Parameter für die intern verwendeten Timer.</param>
        /// <param name="FifoIrqThreshold">
        /// Anzahl der FIFO-Werte, als Schwelle für Interrupt.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOSingleConfig"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamConfig(int device, int subdevice, ref meIOStreamConfig_t configList, int count, ref meIOStreamTrigger_t trigger, int FifoIrqThreshold, int flags);


        #endregion


        #region meIOStreamFrequencyToTicks


        /// <summary>
        /// Konvertiert eine angegebene Zeit in System-Takte.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="timer">
        /// Legt fest welcher interne Zähler als Umrechnungsbasis benutzt werden soll.</param>
        /// <param name="frequency">
        /// Gibt die Anzahl der Takte zurück welche innerhalb dieser Frequenz
        /// auftreten. Geben Sie hier '0' an wenn sie die kleinste messbare Frequenz
        /// bestimmen wollen.</param>
        /// <param name="ticks">
        /// Referenz auf eine Variable welche die Anzahl der Takte aufnimmt.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOStreamTimeToTicks"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamFrequencyToTicks(int device, int subdevice, int timer, ref double frequency, out int ticksLow, out int ticksHigh, int flags);

        #endregion


        #region meIOStreamRead

        /// <summary>
        /// Holt einen oder mehrere Datenwerte ab.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="readMode">Die möglichen Konstanten können unter
        /// <c><see cref="ReadMode"/></c> nachgelesen werden.</param>
        /// <param name="values">Referenz auf ein Array welches die max. Anzahl
        /// von Datenwerte (siehe <paramref name="count"/> count ) aufnehmen kann.</param>
        /// <param name="count">
        /// Bei Funktionsaufruf repräsentiert dieser Wert die Anzahl der abzuholenden
        /// Datenwerte. Nach Funktionsaufruf enthält dieser Parameter die tatsächliche
        /// Anzahl der Datenwerte (nur im <c><see cref="NonBlocking"/></c>-Modus).</param>
        /// <param name="flags"></param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamRead(int device, int subdevice, int readMode, out int values, ref int count, int flags);

        #endregion


        #region meIOStreamSetCallback

        /// <summary>
        /// Die hier angegebenen Delegaten werden registriert und aufgerufen
        /// sobald eine Datenübertragung mit
        /// <c><a class="el" href="class_meilhaus_1_1_driver.html#b9d45175ac582192a6ef19b00ffb0f66">IOStreamStart(ref IOStreamStartStruct)</a></c>
        /// oder
        /// <c><a class="el" href="class_meilhaus_1_1_driver.html#85af56f7939163bfe89158356c4acc3f">IOStreamStart(ref IOStreamStartStruct[])</a></c>
        /// gestartet wird. Optional kann ein Objekt mit weiteren Informationen
        /// an die Ereignismethode übergeben werden.
        /// </summary>
        /// <example>
        /// Anlegen und Verwenden eines Delegaten:
        /// <code>
        /// {
        ///    ...
        ///    Meilhaus.Driver.IOStreamCB callback = new Meilhaus.Driver.IOStreamCB(EventMethod);
        ///    if (Meilhaus.Driver.IOStreamSetCallbacks(deviceIndex,
        ///                                             subdeviceIndex,
        ///                                             null, IntPtr.Zero,          //Keine Benachrichtigung wenn Datenübertragung begonnen wird. In diesem Falle kann bzw. sollte für Kontext IntPtr.Zero angegeben werden.
        ///                                             callback, IntPtr.Zero,      //Übergabe des Delegaten bewirkt ein Aufruf der Methode 'EventMethod' sobald Daten zur Verfügung stehen bzw. der Hardware-Puffer aufgefüllt werden kann/muss
        ///                                             null, IntPtr.Zero,          //Keine Benachrichtigung wenn Datenübertragung begonnen wird. In diesem Falle kann bzw. sollte für Kontext IntPtr.Zero angegeben werden.
        ///                                             Meilhaus.Driver.IOStreamSetCallbacksFlags.NoFlags) != Meilhaus.Errno.Success)
        ///    {
        ///       ...       //Fehlerhandling
        ///    }
        ///    ...
        ///    GC.KeepAlive(callback);                              //Die automatische Freigabe des Speicherbereichs für den Delegaten verhindern solange er von 'unmanaged code' verwendet wird
        /// }
        /// 
        /// private (static) int EventMethod(uint deviceIndex, uint subdeviceIndex, uint count, IntPtr context, Meilhaus.Driver.Errno errorCode)
        /// {
        /// }
        /// </code>
        /// </example>
        /// <example>
        /// Übergabe von beliebigen Objekten an eine Ereignismethode (siehe auch
        /// <a href="http://msdn.microsoft.com:80/msdnmag/issues/02/08/CQA/">Call Unmanaged DLLs from C#, Killing Processes Cleanly</a>).
        /// <code>
        /// {
        ///    ...
        ///    GCHandle objectHandle = new GCHandle();
        ///    try
        ///    {
        ///       objectHandle = GCHandle.Alloc(yourObject);
        ///       if (Meilhaus.Driver.IOStreamSetCallbacks(deviceIndex,
        ///                                                subdeviceIndex,
        ///                                                callback, GCHandle.ToIntPtr(objectHandle),   //Benachrichtigung bei Start mit Schreibweise 1
        ///                                                null, IntPtr.Zero,
        ///                                                callback, (IntPtr)objectHandle,              //Benachrichtigung bei Beendigung mit Schreibweise 2
        ///                                                Meilhaus.Driver.IOStreamSetCallbacksFlags.NoFlags) != Meilhaus.Errno.Success)
        ///    }
        ///    finally
        ///    {
        ///       if (objectHandle.IsAllocated)
        ///          objectHandle.Free();               //Manuell angelegter Speicherbereich hier wieder freigeben
        ///    }
        ///    ...
        /// }
        /// 
        /// private (static) int EventMethod(uint deviceIndex, uint subdeviceIndex, uint count, IntPtr context, Meilhaus.Driver.Errno errorCode)
        /// {
        ///    GCHandle objectHandle = GCHandle.FromIntPtr(context);        //Schreibweise A
        ///    GCHandle objectHandle = (GCHandle)context;                   //Schreibweise B
        ///    yourClass yourObject = (yourClass)handle.Target;
        ///    ...
        /// }
        /// </code>
        /// </example>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="startCB">
        /// Eine Referenz auf einen Delegaten oder null wenn keine Ereignismethode
        /// beim Start aufgeführt werden soll.</param>
        /// <param name="startCBContext">
        /// Die Adresse eines Objektes welches an die Ereignismethode weitergegeben
        /// werden soll. Benötigt die Ereignismethode keine weiteren Informationen
        /// über die Umgebung, so kann hier null übergeben werden. Dieser Parameter
        /// hat keine Bedeutung wenn keine Ereignismethode benutzt werden soll
        /// (<paramref name="startCB"/> startCB = null). In diesem Falle sollte hier
        ///  ebenfalls null übergeben werden.</param>
        /// <param name="newValuesCB">
        /// Wird hier ein Delegat angegeben, so wird dieser immer dann aufgerufen
        /// wenn neue Werte zur Verfügung stehen bzw. neue Werte nachgeladen werden
        /// können/sollten. Ist diese Benachrichtigung nicht nötig, so kann hier
        /// der Wert null übergeben werden.</param>
        /// <param name="newValuesCBContext">
        /// Die Adresse eines Objektes welches an die Ereignismethode weitergegeben
        /// werden soll. Benötigt die Ereignismethode keine weiteren Informationen
        /// über die Umgebung, so kann hier null übergeben werden. Dieser Parameter
        /// hat keine Bedeutung wenn keine Ereignismethode benutzt werden soll
        /// (<paramref name="newValuesCB"/> newValuesCB = null). In diesem Falle
        ///  sollte hier ebenfalls null übergeben werden.</param>
        /// <param name="endCB">
        /// Der Aufruf des hier angegebenen Delegaten signalisiert das Ende der
        /// gestarteten Übertragung. Hier keine Benachrichtigungsmethode
        /// (null) anzugeben macht nur Sinn wenn der Modus <see cref="Blocking"/> in
        /// <c><a class="el" href="class_meilhaus_1_1_driver.html#b9d45175ac582192a6ef19b00ffb0f66">IOStreamStart(ref IOStreamStartStruct)</a></c>
        /// bzw.
        /// <c><a class="el" href="class_meilhaus_1_1_driver.html#85af56f7939163bfe89158356c4acc3f">IOStreamStart(ref IOStreamStartStruct[])</a></c>
        /// verwendet wird.</param>
        /// <param name="endCBContext">
        /// Die Adresse eines Objektes welches an die Ereignismethode weitergegeben
        /// werden soll. Benötigt die Ereignismethode keine weiteren Informationen
        /// über die Umgebung, so kann hier null übergeben werden. Dieser Parameter
        /// hat keine Bedeutung wenn keine Ereignismethode benutzt werden soll
        /// (<paramref name="newValuesCB"/> newValuesCB = null). In diesem Falle
        /// sollte hier ebenfalls null übergeben werden.</param>
        /// <param name="flags">Zur Zeit werden keine weiteren Parameter unterstützt.</param>
        /// <returns>Diese Methode sollte immer <c><see cref="Success"/></c> zurückgeben.
        /// </returns>
        /// <seealso cref="IOStreamStart"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamSetCallbacks(int device, int subdevice, meIOStreamCB_t startCB, IntPtr startCBContext, meIOStreamCB_t newValuesCB, IntPtr newValuesCBContext, meIOStreamCB_t endCB, IntPtr endCBContext, int flags);

        #endregion


        #region meIOStreamStart

        /// <summary>
        /// Startet die Abarbeitung der übergebenen Konfiguration in <see cref="IOStreamConfig"/>.
        /// </summary>
        /// <param name="startList">
        /// Startet die Übertragung auf den angegebenen Komponenten.
        /// </param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamStart(ref meIOStreamStart_t startList, int count, int flags);

        #endregion


        #region meIOStreamStatus

        /// <summary>
        /// Mit dieser Funktion kann geprüft werden ob die gewünschte Anzahl von
        /// Messwerten eingelesen bzw. geschrieben wurden.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="wait">
        /// Beschreibt das Rückkehr-Verhalten der Funktion.</param>
        /// <param name="status">
        /// Beschreibt den aktuellen Betriebszustand nach Rückkehr der Funktion.
        /// Dieser Wer ist nur gültig wenn die Funktion mit <c><see cref="Success"/></c>
        /// terminiert ist.</param>
        /// <param name="count">
        /// Beschreibt die Anzahl der (bereits) eingelesenen Messwerte bzw. die
        /// Anzahl der noch freien Speicherplätze im Ausgabepuffer.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamStatus(int device, int subdevice, int wait, out int status, out int count, int flags);

        #endregion


        #region meIOStreamStop

        /// <summary>
        /// Stoppt eine laufende Übertragung.
        /// </summary>
        /// <param name="startList">
        /// Eine Liste mit allen zu stoppenden Komponenten.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamStop(ref meIOStreamStop_t stopList, int count, int flags);

        #endregion


        #region meIOStreamTimeToTicks


        /// <summary>
        /// Konvertiert eine angegebene Zeit in System-Takte.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="timer">
        /// Legt fest welcher interne Zähler als Umrechnungsbasis benutzt werden soll.</param>
        /// <param name="time">
        /// Die Zeit welche in Takte umgerechnet werden soll. Wählen Sie hier '1' (1s)
        /// wenn sie die Geschwindigkeit des eingebauten Taktgebers ermitteln wollen.
        /// '0' für die Bestimmung der minimal erforderlichen Ticks für die angegebene
        /// Operation.</param>
        /// <param name="ticks">
        /// Referenz auf eine Variable welche die Anzahl der Takte aufnimmt.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOStreamFrequencyToTicks"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamTimeToTicks(int device, int subdevice, int timer, ref double time, out int ticksLow, out int ticksHigh, int flags);

        #endregion


        #region meIOStreamWrite

        /// <summary>
        /// Gibt Datenwerte auf der angegebenen Komponente aus.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// </param>
        /// <param name="writeMode">
        /// Beschreibt den Modus wie die Datenverarbeitung stattfinden soll.</param>
        /// <param name="values">
        /// Schreibt die angegebenen Werte in den Ausgabepuffer.</param>
        /// <param name="count">
        /// In diesem Ausgabeparameter wird die Anzahl der Messwerte gespeichert,
        /// welche in den Ausgabepuffer übernommen werden konnten.</param>
        /// <param name="flags">
        /// Reserviert. Momentan keine anderen Konstanten ausser <c><see cref="NoFlags"/></c>
        /// verfügbar.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="IOStreamRead"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meIOStreamWrite(int device, int subdevice, int writeMode, ref int values, ref int count, int flags);

        #endregion


        #region meLockDevice

        /// <summary>
        /// Sperrt ein komplettes Gerät vor Zugriffen aus anderen Applikationen. Sollte diese
        /// nicht mehr benötigt werden, so muss diese unverzüglich freigegeben werden.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.
        /// </param>
        /// <param name="lockMode">
        /// Die möglichen Paramerter sind unter <c><see cref="Lock"/></c> aufgelistet.
        /// </param>
        /// <param name="flags">
        /// Zur Zeit werden keine Parameter unterstützt. Bitte benutzen Sie die
        /// Konstante <c><see cref="NoFlags"/></c>.</param>
        /// <returns>
        /// Wurde der Applikation nach Aufruf dieser Funktion der exklusive Zugriff
        /// auf das Gerät gewährt, so wird immer mit <c><see cref="Success"/></c>
        /// bestätigt.
        /// </returns>
        /// <seealso cref="LockSubdevice"/>
        /// <seealso cref="LockDriver"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meLockDevice(int device, int lockMode, int flags);

        #endregion


        #region meLockDriver

        /// <summary>
        /// Sperrt ein komplettes Gerät vor Zugriffen aus anderen Applikationen. Sollte diese
        /// nicht mehr benötigt werden, so muss diese unverzüglich freigegeben werden.
        /// </summary>
        /// <param name="lockMode">
        /// Die möglichen Paramerter sind unter <c><see cref="Lock"/></c> aufgelistet.
        /// </param>
        /// <param name="flags">
        /// Zur Zeit werden keine Parameter unterstützt. Bitte benutzen Sie die
        /// Konstante <c><see cref="NoFlags"/></c>.</param>
        /// <returns>
        /// Wurde der Applikation nach Aufruf dieser Funktion der exklusive Zugriff
        /// auf den Treiber gewährt, so wird immer mit <c><see cref="Success"/></c>
        /// bestätigt.
        /// </returns>
        /// <seealso cref="LockDevice"/>
        /// <seealso cref="LockSubdevice"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meLockDriver(int lockMode, int flags);

        #endregion


        #region meLockSubdevice

        /// <summary>
        /// Sperrt eine Komponente vor Zugriffen aus anderen Applikationen. Sollte diese
        /// nicht mehr benötigt werden, so muss diese unverzüglich freigegeben werden.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.
        /// </param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.
        /// </param>
        /// <param name="lockMode">
        /// Die möglichen Paramerter sind unter <c><see cref="Lock"/></c> aufgelistet.
        /// </param>
        /// <param name="flags">
        /// Zur Zeit werden keine Parameter unterstützt. Bitte benutzen Sie die
        /// Konstante <c><see cref="NoFlags"/></c>.</param>
        /// <returns>
        /// Wurde der Applikation nach Aufruf dieser Funktion der exklusive Zugriff
        /// auf die Komponente gewährt, so wird immer mit <c><see cref="Success"/></c>
        /// bestätigt.
        /// </returns>
        /// <seealso cref="LockDevice"/>
        /// <seealso cref="LockDriver"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meLockSubdevice(int device, int subdevice, int lockMode, int flags);

        #endregion


        #region meOpen

        /// <summary>
        /// Bevor die Funktionen des Treibers benutzt werden können, muss diese
        /// Funktion aufgerufen werden. Andernfalls werden alle anderen Zugriffe
        /// fehlschlagen.
        /// </summary>
        /// <param name="flags">Momentan immer <c><see cref="NoFlags"/></c>.</param>
        /// <returns>
        /// Wurde von <c><see cref="Open()"/></c> die Konstante <c><see cref="Success"/></c>
        /// zurückgegeben, dann kann das Treibersystem verwendet werden. Im
        /// Fehlerfalle braucht <c><see cref="Close(CloseFlags)"/></c> nicht aufgerufen
        /// werden!
        /// </returns>
        /// <seealso cref="Close"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meOpen(int flags);

        #endregion


        #region meQueryDescriptionDevice

        /// <summary>
        /// Gibt eine ausführliche Beschreibung über die Funktionalität des
        /// gewünschten Gerätes zurück.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="name">
        /// <para>Eine Referenz auf eine Variable vom Typ string bzw. String.</para>
        /// <para>(Die Variable braucht nicht initialisiert zu sein; kann also den
        /// Wert null besitzen)</para>
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird die Konstante <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryInfoDevice"/>
        /// <seealso cref="QueryNameDevice"/>
        /// <seealso cref="QueryNumberDevices"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meQueryDescriptionDevice(int deviceIndex, StringBuilder name, int count);

        #endregion


        #region meQueryInfoDevice

        /// <summary>
        /// Gibt eine Vielzahl von Informationen über das Gerät zurück.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="vendor">
        /// Referenz auf eine Variable des Typs <c><see cref="Vendor"/></c>. Nach Aufruf
        /// dieser Funktion enthält diese den Hersteller ihres Produkts. Nur wenn
        /// diese den Wert <c><a class="el" href="86bd3f81b3152cfc80515c3af20599f6ee149b35642bd783f40230775b27431b">Meilhaus</a></c> enthält, besitzen Sie eine
        /// hochwertige Karte oder hochwärtiges Gerät direkt aus unserem Hause. Da
        /// die Firma Meilhaus jedoch auch Produkte anderer Hersteller vertreibt,
        /// kann diese Puffer-Variable durchaus auch andere Werte enthalten.</param>
        /// <param name="deviceId">
        /// Hier wird ein Puffer auf eine Ganzzahl erwartet. Nach Beendigung der
        /// Funktion enthält diese die Produktbezeichnung hexadezimal kodiert.</param>
        /// <param name="serialNo">
        /// Wollen Sie eine Karte oder ein Gerät eindeutig identifizieren, so ist
        /// der Rückgabewert in dieser Variablen ihr Referenzwert. Jede produzierte
        /// Karte bzw. produziertes Gerät besitzt eine eindeutige Nummer.</param>
        /// <param name="busType">
        /// Anhand des Bus-Types können Rückschlüsse gezogen werden ob es sich um ein
        /// internes oder externes Gerät handelt.</param>
        /// <param name="busNo">
        /// Nummer unter welchem das Gerät vom System verwaltet wird.</param>
        /// <param name="slot">
        /// Nummer unter welchem das Gerät vom System verwaltet wird.</param>
        /// <param name="funcNo">
        /// Funktionsnummer</param>
        /// <param name="plugged">
        /// Gibt wieder ob ein bekanntes Gerät momentan angeschlossen ist oder nicht.
        /// </param>
        /// <returns>
        /// Wurden alle übergebenen Puffer gefüllt, so wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryDescriptionDevice"/>
        /// <seealso cref="QueryNameDevice"/>
        /// <seealso cref="QueryNumberDevices"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryInfoDevice(     int deviceIndex,
                                                        out int vendorId,
                                                        out int deviceId,
                                                        out int serialNo,
                                                        out int busType,
                                                        out int busNo,
                                                        out int slot,
                                                        out int funcNo,
                                                        out int plugged     );

        #endregion


        #region meQueryNameDevice

        /// <summary>
        /// Löst eine Geräte-Nummer zu einem aussagekräftigen Namen auf.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Nummer zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="name">
        /// <para>Eine Referenz auf eine Variable vom Typ
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">string</a></c> bzw.
        /// <c><a href="http://msdn2.microsoft.com/de-de/library/system.string(VS.80).aspx">String</a></c>.</para>
        /// <para>(Die Variable braucht nicht initialisiert zu sein; kann also den
        /// Wert null besitzen)</para>
        /// </param>
        /// <returns>
        /// Sofern keine Probleme aufgetreten sind, wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryInfoDevice"/>
        /// <seealso cref="QueryDescriptionDevice"/>
        /// <seealso cref="QueryNumberDevices"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNameDevice(int device, StringBuilder name, int count);

        #endregion


        #region meQueryNameDeviceDriver

        /// <summary>
        /// Liefert zu einer bestimmten Geräte-Nummer den dazugehörigen Namen
        /// des Treibermoduls zurück.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="name">
        /// <para>Referenz auf eine Variable welche den Namen aufnimmt.</para>
        /// <para>(Die Variable braucht nicht initialisiert zu sein; kann also den
        /// Wert null besitzen)</para>
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird die Konstante <c><see cref="Success"/></c>
        /// an den Aufrufer zurückgeben.</returns>
        /// <seealso cref="QueryVersionDeviceDriver"/>
        /// <seealso cref="QueryVersionMainDriver"/>
        /// <seealso cref="QueryVersionLibrary"/>

        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNameDeviceDriver(int device, StringBuilder name, int count);

        #endregion


        #region meQueryNumberChannels

        /// <summary>
        /// Anzahl von Ports welche die Funktionsgruppe unterstützt.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="number">
        /// Referenz auf einen Speicherplatz für die zurückgegebene Anzahl von Ports.</param>
        /// <returns></returns>
        /// <returns>
        /// Tritt bei der Ausführung dieser Funktion ein Fehler auf, so wird ein Wert ungleich
        /// <c><see cref="Success"/></c> zurückgegeben.
        /// </returns>
        /// <seealso cref="QueryNumberDevices"/>
        /// <seealso cref="QueryNumberSubdevices"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNumberChannels(int device, int subdevice, out int number);

        #endregion


        #region meQueryNumberDevices

        /// <summary>
        /// Anzahl der bekannten Geräte und Karten.
        /// <remarks>
        /// Achtung: Es müssen nicht alle auch physikalisch vorhanden sein. Prüfen
        /// lässt sich dies mit der Funktion <c><see cref="QueryInfoDevice"/></c> durch
        /// den Parameter <paramref name="plugged"/> plugged.
        /// </remarks>
        /// </summary>
        /// <param name="count">
        /// Referenz auf eine Variable für die Speicherung der Anzahl.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.
        /// </returns>
        /// <seealso cref="QueryNumberSubdevices"/>
        /// <seealso cref="QueryNumberChannels"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNumberDevices(out int count);

        #endregion

        #region meQueryNumberRanges

        /// <summary>
        /// Bestimmt die Anzahl der möglichen Messbereiche für die Funktion
        /// <c><see cref="QueryRangeInfo"/></c>.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// </param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// </param>
        /// <param name="unit">
        /// Die Konstante <c>Any</c> gibt alle verfügbaren Bereiche zurück.
        /// Andere Werte geben nur eine Teilmenge davon zurück.
        /// </param>
        /// <param name="number">
        /// Referenz auf einen Speicherplatz welcher die Indexgrenze für die Funktion
        /// <c><see cref="QueryRangeInfo"/></c> aufnimmt.
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.
        /// </returns>
        /// <seealso cref="QueryRangeInfo"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNumberRanges(int device, int subdevice, int unit, out int number);

        #endregion


        #region meQueryNumberSubdevices

        /// <summary>
        /// Ermittelt die vorhandenen Funktionsgruppen eines Gerätes.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="number">
        /// Variable nimmt die Anzahl der Funktionsgruppen auf.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.
        /// </returns>
        /// <seealso cref="QueryNumberDevices"/>
        /// <seealso cref="QueryNumberChannels"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryNumberSubdevices(int device, out int number);

        #endregion


        #region meQueryRangeByMinMax

        /// <summary>
        /// Kennt man das zu messende bzw. auszugebende Intervall, so kann mit
        /// Hilfe dieser Funktion der passende Messbereich direkt bestimmt werden.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// </param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// </param>
        /// <param name="unit">
        /// Hier muss die gleiche Einheit gewählt werden, welche die die Parameter
        /// <paramref name="min"/> min und <paramref name="max"/> max besitzen.
        /// </param>
        /// <param name="min">Untere Grenze.</param>
        /// <param name="max">Obere Grenze.</param>
        /// <param name="maxData">Auflösung des zurückgegebenen Bereichs.</param>
        /// <param name="rangeIndex">
        /// Index des Bereiches. Wird für andere Funktionen benötigt.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.
        /// </returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryRangeByMinMax(int device, int subdevice, int unit, ref double min, ref double max, out int maxData, out int rangeIndex);

        #endregion


        #region meQueryRangeInfo

        /// <summary>
        /// Ermittelt alle verfügbaren Messbereiche bzw. Ausgabebereiche.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="rangeIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberRanges()"/> - 1</c>.</param>
        /// <param name="unit">
        /// Die gewünschte Einheit von der die verfügbaren Bereiche zurückgegeben
        /// werden sollen.</param>
        /// <param name="min">
        /// Referenz auf einen Puffer welcher den kleinsten gültigen Wert innerhalb
        /// des Intervalls beschreibt.</param>
        /// <param name="max">
        /// Referenz auf einen Puffer welcher den größten gültigen Wert innerhalb
        /// des Intervalls beschreibt.</param>
        /// <param name="maxData">
        /// Der zurückgegebene Wert dieses Puffers beschreibt die Auflösung dieses
        /// Bereiches. Also die Anzahl der möglichen unterschiedlichen Werten innerhalb
        /// des Intervalls <paramref name="min"/> min und <paramref name="max"/> max.
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryNumberRanges"/>
        /// <seealso cref="QueryRangeByMinMax"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryRangeInfo(int device, int subdevice, int range, out int unit, out double min, out double max, out int maxData);

        #endregion


        #region meQuerySubdeviceByType

        /// <summary>
        /// Sucht nach einer angegebenen Komponente mit der gewünschten Funktionalität.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="startSubdeviceIndex">
        /// Bei dem ersten Aufruf der Funktion muss hier 0 übergeben werden. Bei
        /// allen weiteren der Wert welcher in <paramref name="subdeviceIndex"/> subdeviceIndex
        /// zurückgegeben wurde.
        /// </param>
        /// <param name="type">
        /// Die zu suchende Komponente falls vorhanden.</param>
        /// <param name="subType">
        /// Die Funktionsweise der zu suchenden Komponente.</param>
        /// <param name="subdeviceIndex">
        /// Falls eine solche Komponente mit den angegebenen Eigenschaften gefunden
        /// wurde, so wird der Index für einen Zugriff in die hier angegebene Referenz
        /// einer Puffer-Variable gespeichert.</param>
        /// <returns>
        /// Wurde eine Komponente gefunden so wird <c><see cref="Success"/></c> zurückgegeben.
        /// Nur dann ist der <paramref name="subdeviceIndex"/> subdeviceIndex gültig.
        /// </returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meQuerySubdeviceByType(int device, int startSubdevice, int unit, int subType, out int subdevice);

        #endregion


        #region meQuerySubdeviceType

        /// <summary>
        /// Ermittelt den Typ und die Funktionsweise der angegebenen Komponente.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="type">
        /// Referenz auf einen Puffer welches den Typ der angegebenen Komponente
        /// aufnimmt.</param>
        /// <param name="subType">
        /// Nach erfolgreicher Beendigung dieser Funktion steht in der übergebenen
        /// Variablen die Information, ob nur Einzelwerte ausgegeben werden können
        /// oder ob die Hardware Datenfolgen (sog. Streams) verarbeiten kann.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryInfoDevice"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQuerySubdeviceType(int device, int subdevice, out int type, out int subType);

        #endregion


        #region meQuerySubdeviceCaps

        [DllImport("meIDSmain.dll")]
        public static extern int meQuerySubdeviceCaps(int device, int subdevice, out int caps);

        #endregion

        #region meQuerySubdeviceCapsArgs

        [DllImport("meIDSmain.dll")]
        public static extern int meQuerySubdeviceCapsArgs(int device, int subdevice, int caps, ref int args, int count);

        #endregion

        #region meQueryVersionDeviceDriver

        /// <summary>
        /// Gibt die Version des Gerätetreibers zurück. Die oberen 16 Bit geben dabei die
        /// Hauptversion wieder und die unteren 16 Bit spezifizieren die Unterversion.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="version">
        /// Referenz auf einen Speicherplatz für die zurückgegebene Versionsnummer.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryVersionLibrary"/>
        /// <seealso cref="QueryVersionMainDriver"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryVersionDeviceDriver(int device, out int version);

        #endregion


        #region meQueryVersionLibrary

        /// <summary>
        /// Gibt die Version der Dynamic Link Library (DLL) wieder, welche die API
        /// Funktionen zur Verfügung stellt.
        /// </summary>
        /// <param name="version">
        /// Referenz auf einen Speicherplatz in welchen die Version gespeichert wird.
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// </returns>
        /// <seealso cref="QueryVersionDeviceDriver"/>
        /// <seealso cref="QueryVersionMainDriver"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryVersionLibrary(out int version);

        #endregion


        #region meQueryVersionMainDriver

        /// <summary>
        /// Alle Geräte von Meilhaus besitzen zusätzlich einen gemeinsamen Treiber.
        /// Die Version dieses übergeordneten Gerätetreibers kann mit dieser Funktion
        /// ermittelt werden.
        /// </summary>
        /// <param name="version">
        /// Referenz auf einen Speicherplatz in welchen die Version gespeichert wird.
        /// </param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="QueryVersionDeviceDriver"/>
        /// <seealso cref="QueryVersionLibrary"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meQueryVersionMainDriver(out int version);

        #endregion


        #region meUtilityDigitalToPhysical

        /// <summary>
        /// Konvertiert einen binäre Messwert in einen logischen Messwert.
        /// </summary>
        /// <param name="min">Die untere Grenze des benutzten Messbereichs.</param>
        /// <param name="max">Die obere Grenze des benutzten Messbereichs.</param>
        /// <param name="maxData">Die unterstützte Auflösung.</param>
        /// <param name="data">Der zu konvertierende Wert.</param>
        /// <param name="moduleType">Das benutzte Aufsteckmodul.</param>
        /// <param name="refValue">Nur relevant wen Sie ein RTD-Modul verwenden.</param>
        /// <param name="physical">
        /// Referenz auf eine Puffervariable welche den berechneten Wert aufnimmt.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <see cref="QueryRangeInfo"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meUtilityDigitalToPhysical(double min, double max, int maxData, int data, int moduleType, double refValue, out double physical);

        #endregion


        #region meUtilityExtractValues

        /// <summary>
        /// Filtert die Messwerte eines bestimmten Kanals aus einem Datenstrom
        /// und gibt diese zurück.
        /// </summary>
        /// <param name="channelIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberChannels()"/> - 1</c>.</param>
        /// <param name="values">
        /// Referenz auf einen Puffer mit den Roh-Daten. Gewöhnlich ist dies
        /// das selbe Array, welches direkt von <c><see cref="IOStreamRead"/></c>
        /// zurückgegeben wird.</param>
        /// <param name="configList">
        /// Diese Information wird benötigt damit die Roh-Daten wieder einem
        /// Kanal zugeordnet werden können.</param>
        /// <param name="chanBuffer">
        /// Referenz auf einen Puffer welcher die gefundenen Werte aufnimmt.</param>
        /// <param name="chanBufferCount">
        /// Gibt die Anzahl der gefundenen Werte zurück. Diese wurden in das
        /// Feld <paramref name="chanBuffer"/> channBuffer kopiert.</param>
        /// <returns></returns>

//        [DllImport("meIDSmain.dll")]
//        private static extern int meUtilityExtractValues(int channelIndex, ref int dataBuffer, int dataCount, ref meIOStreamConfig_t configList, int configListCount, ref int chanBuffer, ref int chanBufferCount);

        #endregion


        #region meUtilityPhysicalToDigital

        /// <summary>
        /// Konvertiert einen logischen Messwert in einen binären Messwert.
        /// </summary>
        /// <param name="min">Die untere Grenze des benutzten Messbereichs.</param>
        /// <param name="max">Die obere Grenze des benutzten Messbereichs.</param>
        /// <param name="maxData">Die unterstützte Auflösung.</param>
        /// <param name="data">Der zu konvertierende Wert.</param>
        /// <param name="moduleType">Das benutzte Aufsteckmodul.</param>
        /// <param name="refValue">Nur relevant wen Sie ein RTD-Modul verwenden.</param>
        /// <param name="physical">
        /// Referenz auf eine Puffervariable welche den berechneten Wert aufnimmt.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <see cref="QueryRangeInfo"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meUtilityPhysicalToDigital(double min, double max, int maxData, double physical, out int data);


        #endregion


        #region meUtilityPWMStart

        /// <summary>
        /// Konfiguration und start eine Pulseweiten-Modulation.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <param name="reference">
        /// Legt die Taktquelle fest.</param>
        /// <param name="prescaler">
        /// Wert für Vorteiler (Zähler 0) im Bereich 2...65535.</param>
        /// <param name="dutyCycle">
        /// Tastverhältnis des Ausgangssignals von 1%-99% in 1%-Schritten.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        /// <seealso cref="UtilityPWMStop"/>
        [DllImport("meIDSmain.dll")]
        public static extern int meUtilityPWMStart(int deviceIndex, int subdeviceIndex1, int subdeviceIndex2, int subdeviceIndex3, int reference, int prescaler, int dutyCycle, int flags);

        #endregion


        #region meUtilityPWMStop

        /// <summary>
        /// Beendet eine zuvor gestartete Pulseweiten-Modulation mittels
        /// <c><see cref="UtilityPWMStop"/></c> wieder.
        /// </summary>
        /// <param name="deviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberDevices()"/> - 1</c>.</param>
        /// <param name="subdeviceIndex">
        /// Eine Ganzzahl zwischen 0 und <c><see cref="QueryNumberSubdevices()"/> - 1</c>.</param>
        /// <returns>
        /// Bei erfolgreicher Ausführung wird der Wert <c><see cref="Success"/></c>
        /// zurückgegeben.</returns>
        [DllImport("meIDSmain.dll")]
        public static extern int meUtilityPWMStop(int device, int subdevice1);

        #endregion
    }
}

