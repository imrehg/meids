classdef meIDS
    %meIDS MATLAB inferface class for Meilhaus ME-iDS.
    % Static pseudo-class for ME-iDS. Support Meilhaus boards: ME630,
    % ME1000, ME1400,ME1600, ME4600, ME6X00, ME8100, ME8200 and ME9X00

    properties (Constant = true)
        ME_VALUE_NOT_USED                 = hex2dec('0000');
        ME_VALUE_DEFAULT                  = hex2dec('0000');
        ME_NO_FLAGS                       = hex2dec('0000');

        ME_OPEN_NO_FLAGS                  = hex2dec('0000');
        ME_CLOSE_NO_FLAGS                 = hex2dec('0000');
        ME_LOCK_DRIVER_NO_FLAGS           = hex2dec('0000');
        ME_LOCK_DEVICE_NO_FLAGS           = hex2dec('0000');
        ME_LOCK_SUBDEVICE_NO_FLAGS        = hex2dec('0000');

        ME_LOCK_RELEASE                   = hex2dec('10001');
        ME_LOCK_SET                       = hex2dec('10002');
        ME_LOCK_CHECK                     = hex2dec('10003');

        ME_SWITCH_DISABLE                 = hex2dec('20001');
        ME_SWITCH_ENABLE                  = hex2dec('20002');

        ME_REF_NONE                       = hex2dec('00000');
        ME_REF_DIO_FIFO_LOW               = hex2dec('30001');
        ME_REF_DIO_FIFO_HIGH              = hex2dec('30002');

        ME_REF_CTR_PREVIOUS               = hex2dec('40001');
        ME_REF_CTR_INTERNAL_1MHZ          = hex2dec('40002');
        ME_REF_CTR_INTERNAL_10MHZ         = hex2dec('40003');
        ME_REF_CTR_EXTERNAL               = hex2dec('40004');

        ME_REF_AI_GROUND                  = hex2dec('50001');
        ME_REF_AI_DIFFERENTIAL            = hex2dec('50002');

        ME_REF_AO_GROUND                  = hex2dec('60001');
        ME_REF_AO_DIFFERENTIAL            = hex2dec('60002');

        ME_TRIG_CHAN_NONE                 = hex2dec('00000');
        ME_TRIG_CHAN_DEFAULT              = hex2dec('70001');
        ME_TRIG_CHAN_SYNCHRONOUS          = hex2dec('70002');

        ME_TRIG_TYPE_NONE                 = hex2dec('00000');
        ME_TRIG_TYPE_SW                   = hex2dec('80001');
        ME_TRIG_TYPE_THRESHOLD            = hex2dec('80002');
        ME_TRIG_TYPE_WINDOW               = hex2dec('80003');
        ME_TRIG_TYPE_EDGE                 = hex2dec('80004');
        ME_TRIG_TYPE_SLOPE                = hex2dec('80005');
        ME_TRIG_TYPE_EXT_DIGITAL          = hex2dec('80006');
        ME_TRIG_TYPE_EXT_ANALOG           = hex2dec('80007');
        ME_TRIG_TYPE_PATTERN              = hex2dec('80008');
        ME_TRIG_TYPE_TIMER                = hex2dec('80009');
        ME_TRIG_TYPE_COUNT                = hex2dec('8000A');
        ME_TRIG_TYPE_FOLLOW               = hex2dec('8000B');

        ME_TRIG_EDGE_NONE                 = hex2dec('00000');
        ME_TRIG_EDGE_ABOVE                = hex2dec('90001');
        ME_TRIG_EDGE_BELOW                = hex2dec('90002');
        ME_TRIG_EDGE_ENTRY                = hex2dec('90003');
        ME_TRIG_EDGE_EXIT                 = hex2dec('90004');
        ME_TRIG_EDGE_RISING               = hex2dec('90005');
        ME_TRIG_EDGE_FALLING              = hex2dec('90006');
        ME_TRIG_EDGE_ANY                  = hex2dec('90007');

        ME_TIMER_ACQ_START                = hex2dec('A0001');
        ME_TIMER_SCAN_START               = hex2dec('A0002');
        ME_TIMER_CONV_START               = hex2dec('A0003');
        ME_TIMER_FREQ                     = hex2dec('A0004');

        ME_IRQ_SOURCE_DIO_DEFAULT         = hex2dec('00000');
        ME_IRQ_SOURCE_DIO_PATTERN         = hex2dec('B0001');
        ME_IRQ_SOURCE_DIO_MASK            = hex2dec('B0002');
        ME_IRQ_SOURCE_DIO_LINE            = hex2dec('B0003');
        ME_IRQ_SOURCE_DIO_OVER_TEMP       = hex2dec('B0004');

        ME_IRQ_EDGE_NOT_USED						= hex2dec('00000');
        ME_IRQ_EDGE_RISING							= hex2dec('C0001');
        ME_IRQ_EDGE_FALLING							= hex2dec('C0002');
        ME_IRQ_EDGE_ANY								= hex2dec('C0003');

        ME_IO_IRQ_START_NO_FLAGS					= hex2dec('0000');
        ME_IO_IRQ_START_DIO_BIT						= hex2dec('0001');
        ME_IO_IRQ_START_DIO_BYTE					= hex2dec('0002');
        ME_IO_IRQ_START_DIO_WORD					= hex2dec('0004');
        ME_IO_IRQ_START_DIO_DWORD					= hex2dec('0008');
        ME_IO_IRQ_START_PATTERN_FILTERING			= hex2dec('0010');
        ME_IO_IRQ_START_EXTENDED_STATUS				= hex2dec('0020');

        ME_IO_IRQ_WAIT_NO_FLAGS						= hex2dec('0000');
        ME_IO_IRQ_WAIT_NORMAL_STATUS				= hex2dec('0001');
        ME_IO_IRQ_WAIT_EXTENDED_STATUS				= hex2dec('0002');

        ME_IO_IRQ_STOP_NO_FLAGS						= hex2dec('0000');

        ME_IO_IRQ_SET_CALLBACK_NO_FLAGS				= hex2dec('0000');

        ME_IO_RESET_DEVICE_NO_FLAGS					= hex2dec('0000');
        ME_IO_RESET_DEVICE_UNPROTECTED				= hex2dec('10000');

        ME_IO_RESET_SUBDEVICE_NO_FLAGS				= hex2dec('0000');

        ME_SINGLE_CONFIG_DIO_INPUT					= hex2dec('D0001');
        ME_SINGLE_CONFIG_DIO_OUTPUT					= hex2dec('D0002');
        ME_SINGLE_CONFIG_DIO_HIGH_IMPEDANCE			= hex2dec('D0003');
        ME_SINGLE_CONFIG_DIO_SINK					= hex2dec('D0004');
        ME_SINGLE_CONFIG_DIO_SOURCE					= hex2dec('D0005');
        ME_SINGLE_CONFIG_DIO_MUX32M					= hex2dec('D0006');
        ME_SINGLE_CONFIG_DIO_DEMUX32				= hex2dec('D0007');
        ME_SINGLE_CONFIG_DIO_BIT_PATTERN			= hex2dec('D0008');

        ME_SINGLE_CONFIG_MULTIPIN_IRQ				= hex2dec('D0009');
        ME_SINGLE_CONFIG_MULTIPIN_CLK				= hex2dec('D000a');

        ME_SINGLE_CONFIG_CTR_8254_MODE_0			= hex2dec('E0001');
        ME_SINGLE_CONFIG_CTR_8254_MODE_1			= hex2dec('E0002');
        ME_SINGLE_CONFIG_CTR_8254_MODE_2			= hex2dec('E0003');
        ME_SINGLE_CONFIG_CTR_8254_MODE_3			= hex2dec('E0004');
        ME_SINGLE_CONFIG_CTR_8254_MODE_4			= hex2dec('E0005');
        ME_SINGLE_CONFIG_CTR_8254_MODE_5			= hex2dec('E0006');

        ME_SINGLE_CONFIG_CTR_8254_MODE_DISABLE						= hex2dec('E0000');
        ME_SINGLE_CONFIG_CTR_8254_MODE_INTERRUPT_ON_TERMINAL_COUNT	= hex2dec('E0001');
        ME_SINGLE_CONFIG_CTR_8254_MODE_ONE_SHOT						= hex2dec('E0002');
        ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR				= hex2dec('E0003');
        ME_SINGLE_CONFIG_CTR_8254_MODE_SQUARE_WAVE					= hex2dec('E0004');
        ME_SINGLE_CONFIG_CTR_8254_MODE_SOFTWARE_TRIGGER				= hex2dec('E0005');
        ME_SINGLE_CONFIG_CTR_8254_MODE_HARDWARE_TRIGGER				= hex2dec('E0006');

        ME_IO_SINGLE_CONFIG_NO_FLAGS				= hex2dec('0000');
        ME_IO_SINGLE_CONFIG_DIO_BIT					= hex2dec('0001');
        ME_IO_SINGLE_CONFIG_DIO_BYTE				= hex2dec('0002');
        ME_IO_SINGLE_CONFIG_DIO_WORD				= hex2dec('0004');
        ME_IO_SINGLE_CONFIG_DIO_DWORD				= hex2dec('0008');
        ME_IO_SINGLE_CONFIG_MULTISIG_LED_ON			= hex2dec('0010');
        ME_IO_SINGLE_CONFIG_MULTISIG_LED_OFF		= hex2dec('0020');
        ME_IO_SINGLE_CONFIG_AI_RMS					= hex2dec('0040');
        ME_IO_SINGLE_CONFIG_CONTINUE				= hex2dec('0080');
        ME_IO_SINGLE_CONFIG_MULTIPIN				= hex2dec('0100');

        ME_IO_SINGLE_NO_FLAGS						= hex2dec('0000');
        ME_IO_SINGLE_NONBLOCKING					= hex2dec('0020');

        ME_DIR_INPUT								= hex2dec('F0001');
        ME_DIR_OUTPUT								= hex2dec('F0002');
        ME_DIR_SET_OFFSET							= hex2dec('F0003');

        ME_IO_SINGLE_TYPE_NO_FLAGS					= hex2dec('0000');
        ME_IO_SINGLE_TYPE_DIO_BIT					= hex2dec('0001');
        ME_IO_SINGLE_TYPE_DIO_BYTE					= hex2dec('0002');
        ME_IO_SINGLE_TYPE_DIO_WORD					= hex2dec('0004');
        ME_IO_SINGLE_TYPE_DIO_DWORD					= hex2dec('0008');
        ME_IO_SINGLE_TYPE_TRIG_SYNCHRONOUS			= hex2dec('0010');
        ME_IO_SINGLE_TYPE_NONBLOCKING				= hex2dec('0020');
        ME_IO_SINGLE_TYPE_WRITE_NONBLOCKING			= hex2dec('0020');
        ME_IO_SINGLE_TYPE_READ_NONBLOCKING			= hex2dec('0020');
        ME_IO_SINGLE_TYPE_FREQ_DIVIDER				= hex2dec('0040');
        ME_IO_SINGLE_TYPE_FREQ_START_LOW			= hex2dec('0080');
        ME_IO_SINGLE_TYPE_FREQ_START_SOFT			= hex2dec('0100');
        ME_IO_SINGLE_TYPE_FREQ_LAST_VALUE			= hex2dec('0200');


        ME_IO_STREAM_CONFIG_NO_FLAGS				= hex2dec('0000');
        ME_IO_STREAM_CONFIG_BIT_PATTERN				= hex2dec('0001');
        ME_IO_STREAM_CONFIG_WRAPAROUND				= hex2dec('0002');
        ME_IO_STREAM_CONFIG_SAMPLE_AND_HOLD			= hex2dec('0004');
        ME_IO_STREAM_CONFIG_HARDWARE_ONLY			= hex2dec('0008');

        ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS			= hex2dec('0000');

        ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS			= hex2dec('0000');

        ME_READ_MODE_BLOCKING						= hex2dec('100001');
        ME_READ_MODE_NONBLOCKING					= hex2dec('100002');

        ME_IO_STREAM_READ_NO_FLAGS					= hex2dec('0000');
        ME_IO_STREAM_READ_FRAMES					= hex2dec('0001');

        ME_WRITE_MODE_BLOCKING						= hex2dec('110001');
        ME_WRITE_MODE_NONBLOCKING					= hex2dec('110002');
        ME_WRITE_MODE_PRELOAD						= hex2dec('110003');

        ME_IO_STREAM_WRITE_NO_FLAGS					= hex2dec('0000');

        ME_IO_STREAM_START_NO_FLAGS					= hex2dec('0000');

        ME_START_MODE_BLOCKING						= hex2dec('120001');
        ME_START_MODE_NONBLOCKING					= hex2dec('120002');

        ME_IO_STREAM_START_TYPE_NO_FLAGS			= hex2dec('0000');
        ME_IO_STREAM_START_TYPE_TRIG_SYNCHRONOUS	= hex2dec('0010');

        ME_IO_STREAM_STOP_NO_FLAGS					= hex2dec('0000');

        ME_STOP_MODE_IMMEDIATE						= hex2dec('130001');
        ME_STOP_MODE_LAST_VALUE						= hex2dec('130002');

        ME_IO_STREAM_STOP_TYPE_NO_FLAGS				= hex2dec('0000');
        ME_IO_STREAM_STOP_TYPE_PRESERVE_BUFFERS		= hex2dec('0001');

        ME_WAIT_NONE								= hex2dec('140001');
        ME_WAIT_IDLE								= hex2dec('140002');
        ME_WAIT_BUSY								= hex2dec('140003');

        ME_STATUS_INVALID							= hex2dec('00000');
        ME_STATUS_IDLE								= hex2dec('150001');
        ME_STATUS_BUSY								= hex2dec('150002');
        ME_STATUS_ERROR								= hex2dec('150003');

        ME_IO_STREAM_STATUS_NO_FLAGS				= hex2dec('0000');

        ME_IO_STREAM_SET_CALLBACKS_NO_FLAGS			= hex2dec('0000');

        ME_IO_STREAM_NEW_VALUES_NO_FLAGS			= hex2dec('0000');

        ME_IO_TIME_TO_TICKS_NO_FLAGS				= hex2dec('0000');
        ME_IO_STREAM_TIME_TO_TICKS_NO_FLAGS			= hex2dec('0000');

        ME_IO_FREQUENCY_TO_TICKS_NO_FLAGS			= hex2dec('0000');
        ME_IO_STREAM_FREQUENCY_TO_TICKS_NO_FLAGS	= hex2dec('0000');

        ME_MODULE_TYPE_MULTISIG_NONE				= hex2dec('000000');
        ME_MODULE_TYPE_MULTISIG_DIFF16_10V			= hex2dec('160001');
        ME_MODULE_TYPE_MULTISIG_DIFF16_20V			= hex2dec('160002');
        ME_MODULE_TYPE_MULTISIG_DIFF16_50V			= hex2dec('160003');
        ME_MODULE_TYPE_MULTISIG_CURRENT16_0_20MA	= hex2dec('160004');
        ME_MODULE_TYPE_MULTISIG_RTD8_PT100			= hex2dec('160005');
        ME_MODULE_TYPE_MULTISIG_RTD8_PT500			= hex2dec('160006');
        ME_MODULE_TYPE_MULTISIG_RTD8_PT1000			= hex2dec('160007');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_B			= hex2dec('160008');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_E			= hex2dec('160009');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_J			= hex2dec('16000A');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_K			= hex2dec('16000B');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_N			= hex2dec('16000C');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_R			= hex2dec('16000D');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_S			= hex2dec('16000E');
        ME_MODULE_TYPE_MULTISIG_TE8_TYPE_T			= hex2dec('16000F');
        ME_MODULE_TYPE_MULTISIG_TE8_TEMP_SENSOR		= hex2dec('160010');

        ME_CAPS_NONE								= hex2dec('00000');
        ME_CAPS_TRIG_DIGITAL						= hex2dec('08000');
        ME_CAPS_TRIG_ANALOG							= hex2dec('10000');
        ME_CAPS_TRIG_EDGE_RISING					= hex2dec('20000');
        ME_CAPS_TRIG_EDGE_FALLING					= hex2dec('40000');
        ME_CAPS_TRIG_EDGE_ANY						= hex2dec('80000');

        ME_CAPS_DIO_DIR_BIT							= hex2dec('00001');
        ME_CAPS_DIO_DIR_BYTE						= hex2dec('00002');
        ME_CAPS_DIO_DIR_WORD						= hex2dec('00004');
        ME_CAPS_DIO_DIR_DWORD						= hex2dec('00008');
        ME_CAPS_DIO_SINK_SOURCE						= hex2dec('00010');
        ME_CAPS_DIO_BIT_PATTERN_IRQ					= hex2dec('00020');
        ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_RISING		= hex2dec('00040');
        ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_FALLING		= hex2dec('00080');
        ME_CAPS_DIO_BIT_MASK_IRQ_EDGE_ANY			= hex2dec('00100');
        ME_CAPS_DIO_OVER_TEMP_IRQ					= hex2dec('00200');
        ME_CAPS_DIO_TRIG_SYNCHRONOUS				= hex2dec('04000');
        ME_CAPS_DIO_TRIG_DIGITAL					= hex2dec('08000');
        ME_CAPS_DIO_TRIG_ANALOG						= hex2dec('10000');
        ME_CAPS_DIO_TRIG_EDGE_RISING				= hex2dec('20000');
        ME_CAPS_DIO_TRIG_EDGE_FALLING				= hex2dec('40000');
        ME_CAPS_DIO_TRIG_EDGE_ANY					= hex2dec('80000');

        ME_CAPS_CTR_CLK_PREVIOUS					= hex2dec('00001');
        ME_CAPS_CTR_CLK_INTERNAL_1MHZ				= hex2dec('00002');
        ME_CAPS_CTR_CLK_INTERNAL_10MHZ				= hex2dec('00004');
        ME_CAPS_CTR_CLK_EXTERNAL					= hex2dec('00008');

        ME_CAPS_AI_TRIG_SYNCHRONOUS					= hex2dec('00001');
        ME_CAPS_AI_TRIG_SIMULTANEOUS				= hex2dec('00001')
        ME_CAPS_AI_FIFO								= hex2dec('00002');
        ME_CAPS_AI_FIFO_THRESHOLD					= hex2dec('00004');
        ME_CAPS_AI_SAMPLE_HOLD						= hex2dec('00008');
        ME_CAPS_AI_TRIG_DIGITAL						= hex2dec('08000');
        ME_CAPS_AI_TRIG_ANALOG						= hex2dec('10000');
        ME_CAPS_AI_TRIG_EDGE_RISING					= hex2dec('20000');
        ME_CAPS_AI_TRIG_EDGE_FALLING				= hex2dec('40000');
        ME_CAPS_AI_TRIG_EDGE_ANY					= hex2dec('80000');

        ME_CAPS_AO_TRIG_SYNCHRONOUS					= hex2dec('00001');
        ME_CAPS_AO_TRIG_SIMULTANEOUS				= hex2dec('00001');
        ME_CAPS_AO_FIFO								= hex2dec('00002');
        ME_CAPS_AO_FIFO_THRESHOLD					= hex2dec('00004');
        ME_CAPS_AO_TRIG_DIGITAL						= hex2dec('08000');
        ME_CAPS_AO_TRIG_ANALOG						= hex2dec('10000');
        ME_CAPS_AO_TRIG_EDGE_RISING					= hex2dec('20000');
        ME_CAPS_AO_TRIG_EDGE_FALLING				= hex2dec('40000');
        ME_CAPS_AO_TRIG_EDGE_ANY					= hex2dec('80000');

        ME_CAPS_EXT_IRQ_EDGE_RISING					= hex2dec('00001');
        ME_CAPS_EXT_IRQ_EDGE_FALLING				= hex2dec('00002');
        ME_CAPS_EXT_IRQ_EDGE_ANY					= hex2dec('00004');

        ME_CAP_AI_FIFO_SIZE							= hex2dec('1D0000');
        ME_CAP_AI_BUFFER_SIZE						= hex2dec('1D0001');
        ME_CAP_AI_CHANNEL_LIST_SIZE					= hex2dec('1D0002');

        ME_CAP_AO_FIFO_SIZE							= hex2dec('1F0000');
        ME_CAP_AO_BUFFER_SIZE						= hex2dec('1F0001');

        ME_CAP_CTR_WIDTH							= hex2dec('200000');

        ME_UNIT_INVALID								= hex2dec('000000');
        ME_UNIT_VOLT								= hex2dec('170001');
        ME_UNIT_AMPERE								= hex2dec('170002');
        ME_UNIT_ANY									= hex2dec('170003');

        ME_TYPE_INVALID								= hex2dec('000000');
        ME_TYPE_AO									= hex2dec('180001');
        ME_TYPE_AI									= hex2dec('180002');
        ME_TYPE_DIO									= hex2dec('180003');
        ME_TYPE_DO									= hex2dec('180004');
        ME_TYPE_DI									= hex2dec('180005');
        ME_TYPE_CTR									= hex2dec('180006');
        ME_TYPE_EXT_IRQ								= hex2dec('180007');
        ME_TYPE_FREQ_IO								= hex2dec('180008');
        ME_TYPE_FREQ_O								= hex2dec('180009');
        ME_TYPE_FREQ_I								= hex2dec('18000A');


        ME_SUBTYPE_INVALID							= hex2dec('000000');
        ME_SUBTYPE_SINGLE							= hex2dec('190001');
        ME_SUBTYPE_STREAMING						= hex2dec('190002');
        ME_SUBTYPE_CTR_8254							= hex2dec('190003');
        ME_SUBTYPE_ANY								= hex2dec('190004');
        ME_SUBTYPE_CTR								= hex2dec('190006');
        ME_SUBTYPE_DIGITAL							= hex2dec('190007');
        ME_SUBTYPE_ANALOG							= hex2dec('190008');

        ME_BUS_TYPE_INVALID							= hex2dec('000000');
        ME_BUS_TYPE_ANY								= hex2dec('1A0000');
        ME_BUS_TYPE_PCI								= hex2dec('1A0001');
        ME_BUS_TYPE_USB								= hex2dec('1A0002');
        ME_BUS_TYPE_LAN_PCI							= hex2dec('1A0101');
        ME_BUS_TYPE_LAN_USB							= hex2dec('1A0102');

        ME_PLUGGED_INVALID							= hex2dec('000000');
        ME_PLUGGED_ANY								= hex2dec('1B0000');
        ME_PLUGGED_IN								= hex2dec('1B0001');
        ME_PLUGGED_OUT								= hex2dec('1B0002');

        ME_EXTENSION_TYPE_INVALID					= hex2dec('000000');
        ME_EXTENSION_TYPE_NONE						= hex2dec('1C0001');
        ME_EXTENSION_TYPE_MUX32M					= hex2dec('1C0002');
        ME_EXTENSION_TYPE_DEMUX32					= hex2dec('1C0003');
        ME_EXTENSION_TYPE_MUX32S					= hex2dec('1C0004');

        ME_ACCESS_TYPE_INVALID						= hex2dec('000000');
        ME_ACCESS_TYPE_ANY							= hex2dec('1D0000');
        ME_ACCESS_TYPE_LOCAL						= hex2dec('1D0001');
        ME_ACCESS_TYPE_REMOTE						= hex2dec('1D0002');

        ME_CONF_LOAD_CUSTOM_DRIVER					= hex2dec('01000');

        ME_PWM_START_NO_FLAGS						= hex2dec('00000');
        ME_PWM_START_CONNECT_INTERNAL				= hex2dec('00001');

        ME_QUERY_NO_FLAGS 							= hex2dec('00000');

        ME_ERRNO_CLEAR_FLAGS						= hex2dec('00001');


        ME_ERROR_MSG_MAX_COUNT            = 256;
        ME_DEVICE_DRIVER_NAME_MAX_COUNT   = 64;
        ME_DEVICE_NAME_MAX_COUNT          = 64;
        ME_DEVICE_DESCRIPTION_MAX_COUNT   = 256;

        meStructSingle = struct('iDevice', 0, 'iSubdevice', 0, 'iChannel', 0, 'iDir', 0, 'iValue', 0,'iTimeOut', 0, 'iFlags', 0, 'iErrno', 0);

        meStructStreamStart = struct('iDevice', 0, 'iSubdevice', 0, 'iStartMode', 0,'iTimeOut', 0, 'iFlags', 0, 'iErrno', 0);
        meStructStreamStop = struct('iDevice', 0, 'iSubdevice', 0, 'iStopMode', 0, 'iFlags', 0, 'iErrno', 0);

        meStructStreamConfig = struct('iChannel', 0, 'iStreamConfig', 0, 'iRef', 0,'iFlags', 0);
        meStructStreamTrigger = struct('iAcqStartTrigType', 0, 'iAcqStartTrigEdge', 0, 'iAcqStartTrigChan', 0, 'iAcqStartTicksLow', 0, 'iAcqStartTicksHigh', 0,'iAcqStartArgs', zeros(1,10, 'int32'), 'iScanStartTrigType', 0, 'iScanStartTicksLow', 0, 'iScanStartTicksHigh', 0, 'iScanStartArgs', zeros(1,10, 'int32'), 'iConvStartTrigType', 0, 'iConvStartTicksLow', 0, 'iConvStartTicksHigh', 0, 'iConvStartArgs', zeros(1,10, 'int32'), 'iScanStopTrigType', 0, 'iScanStopCount', 0, 'iScanStopArgs', zeros(1,10, 'int32'), 'iAcqStopTrigType', 0, 'iAcqStopCount', 0, 'iAcqStopArgs', zeros(1,10, 'int32'), 'iFlags', 0);
    end

    methods(Static)
        function [err] = meOpen(Flags)
        %meOpen Open MEiDS
        %   Open Meilhaus MEiDS

             if not(libisloaded('meIDSmain'))
                loadlibrary('meIDSmain', [matlabroot '\extern\include\medriver.h']);
                err = calllib('meIDSmain', 'meOpen', Flags);
            else
                err = 0;
            end
        end

        function [err] = meClose(Flags)
        %meClose Close MEiDS

            if libisloaded('meIDSmain')
                err = calllib('meIDSmain', 'meClose', Flags);
                unloadlibrary('meIDSmain');
            else
                err = 0;
            end
        end

        function [err] = meLockDriver(Lock, Flags)
        %meLockDriver Lock MEiDS.

            err = calllib('meIDSmain', 'meLockDriver', Lock, Flags);
        end

        function [err] = meLockDevice(Device, Lock, Flags)
        %meLockDevice Lock device.

            err = calllib('meIDSmain', 'meLockDevice', Device, Lock, Flags);
        end

        function [err] = meLockSubdevice(Device, Subdevice, Lock, Flags)
        %meLockSubdevice Lock sub-device.

            err = calllib('meIDSmain', 'meLockSubdevice', Device, Subdevice, Lock, Flags);
        end

        function [Message] = meErrorGetMessage(ErrorCode)
        %meErrorGetMessage

            Buff = [int8(1:meIDS.ME_ERROR_MSG_MAX_COUNT) 0];
            pBuff = libpointer('voidPtr', Buff);

            [err, Message] = calllib('meIDSmain', 'meErrorGetMessage', ErrorCode, pBuff, meDefines.ME_ERROR_MSG_MAX_COUNT);

            if (err ~= 0)
                Message = 'Can not decode error number.';
            end

            clear pBuff;
        end

        function [err, Message] = meErrorGetLastMessage()
        %meErrorGetLastMessage

            Buff = [int8(1:meIDS.ME_ERROR_MSG_MAX_COUNT) 0];
            pBuff = libpointer('voidPtr', Buff);

            [err, Message] = calllib('meIDSmain', 'meErrorGetLastMessage', pBuff, meDefines.ME_ERROR_MSG_MAX_COUNT);

            if (err ~= 0)
                Message = 'Can not read ERRNO.';
            end

            clear pBuff;
        end

        function [err] = meErrorSetDefaultProc(Switch)
        %meErrorSetDefaultProc

        	err = calllib('meIDSmain', 'meErrorSetDefaultProc', Switch);
        end

        function [err] = meIOResetDevice(Device, Flags)
        %meIOResetDevice Reset device

            err = calllib('meIDSmain', 'meIOResetDevice', Device, Flags);
        end

        function [err] = meIOResetSubdevice(Device, Subdevice, Flags)
        %meIOResetSubdevice Reset device

            err = calllib('meIDSmain', 'meIOResetSubdevice', Device, Subdevice, Flags);
        end

        function [err] = meIOIrqStart(Device, Subdevice, Channel, IrqSource, IrqEdge, IrqArg, Flags)
        %meIOIrqStart Start IRQ routine.

            err = calllib('meIDSmain', 'meIOIrqStart', Device, Subdevice, Channel, IrqSource, IrqEdge, IrqArg, Flags);
        end

        function [err] = meIOIrqStop(Device, Subdevice, Channel, Flags)
        %meIOIrqStop Stop IRQ routine.

            err = calllib('meIDSmain', 'meIOIrqStop', Device, Subdevice, Channel, Flags);
        end

        function [err, IrqCount, IrqState] = meIOIrqWait(Device, Subdevice, Channel, TimeOut, Flags)
        %meIOIrqWait Wait for IRQ.

            iIrqCount = int32(0);
            piIrqCount = libpointer('int32Ptr', iIrqCount);
            iIrqState = int32(0);
            piIrqState = libpointer('int32Ptr', iIrqState);

            err = calllib('meIDSmain', 'meIOIrqWait', Device, Subdevice, Channel, piIrqCount, piIrqState, TimeOut, Flags);

            IrqCount = get(piIrqCount, 'Value');
            IrqState = get(piIrqState, 'Value');

            clear piIrqCount;
            clear piIrqState;
        end

        function [err] = meIOSingleConfig(Device, Subdevice, Channel, SingleConfig, Ref, TrigChan, TrigType, TrigEdge, Flags)
        %meIOSingleConfig Configure subdevice for single mode.
        %   Configure subdevice for single mode.

            err = calllib('meIDSmain', 'meIOSingleConfig', Device, Subdevice, Channel, SingleConfig, Ref, TrigChan, TrigType, TrigEdge, Flags);
        end

        function [err, Single] = meIOSingle(sSingle, Flags)
        %meIOSingle Read/Write in single mode
        %   Read/Write subdevice in single mode.

            Count = size(sSingle, 2);
            psSingle = libpointer('meIOSinglePtr', sSingle);

            err = calllib('meIDSmain', 'meIOSingle', psSingle, Count, Flags);

            Single = meIDS.meStructSingle;
            for idx = 1:Count
                Single(idx) = get(psSingle + (idx - 1), 'Value');
                if (Single(idx).iErrno ~= 0)
                    clear psSingle;
                    return;
                end
            end
            clear psSingle;
        end

        function [err] = meIOStreamConfig(Device, Subdevice, sConfig, sTrigger, FifoIrqThreshold, Flags)
        %meIOStreamConfig

            psConfig = libpointer('meIOStreamConfigPtr', sConfig);
            psTrigger = libpointer('meIOStreamTriggerPtr', sTrigger);

            err = calllib('meIDSmain', 'meIOStreamConfig', Device, Subdevice, psConfig, size(sConfig, 2), psTrigger, FifoIrqThreshold, Flags);

            clear psConfig;
            clear psTrigger;
        end

        function [err, Start] = meIOStreamStart(sStart, Flags)
        %meIOStreamStart Summary of this function goes here

            Count = size(sStart, 2);
            psStart = libpointer('meIOStreamStartPtr', sStart);

            err = calllib('meIDSmain', 'meIOStreamStart', psStart, Count, Flags);

            Start = meIDS.meStructStreamStart;
            for idx = 1:Count
                Start(idx) = get(psStart + (idx - 1), 'Value');
                if (Start(idx).iErrno ~= 0)
                    clear pStart;
                    return;
                end
            end
            clear pStart;
        end

        function [err, Stop] = meIOStreamStop(sStop, Flags)
        %meIOStreamStop Stop streaming.

            Count = size(sStop, 2);
            psStop = libpointer('meIOStreamStopPtr', sStop);

            err = calllib('meIDSmain', 'meIOStreamStop', psStop, Count, Flags);

            Stop = meIDS.meStructStreamStop;
            for idx = 1:Count
                Stop(idx) = get(psStop + (idx - 1), 'Value');
                if (Stop(idx).iErrno ~= 0)
                    clear psStop;
                    return;
                end
            end
            clear psStop;
        end

        function [err, Status, Count] = meIOStreamStatus(Device, Subdevice, Wait, Flags)
        %meIOStreamStatus Status of stream operation.

            iCount = int32(0);
            piCount = libpointer('int32Ptr', iCount);

            iStatus = int32(0);
            piStatus = libpointer('int32Ptr', iStatus);

            err = calllib('meIDSmain', 'meIOStreamStatus', Device, Subdevice, Wait, piStatus, piCount, Flags);

            Count = get(piCount, 'Value');
            Status = get(piStatus, 'Value');

            clear piCount;
            clear piStatus;
        end

        function [err, Digital] = meIOStreamRead(Device, Subdevice, ReadMode, Count, Flags)
        %meIOStreamRead Read stream buffer.

            iBuff = zeros(1, Count, 'int32');
            piBuff = libpointer('int32Ptr', iBuff);
            iCount = int32(Count);
            piCount = libpointer('int32Ptr', iCount);

            err = calllib('meIDSmain', 'meIOStreamRead', Device, Subdevice, ReadMode, piBuff, piCount, Flags);

            Digital = get(piBuff, 'Value');

            Count = get(piCount, 'Value');
            Digital = Digital(1:Count);

            clear piCount;
            clear piBuff;
        end

        function [err, Count] = meIOStreamWrite(Device, Subdevice, WriteMode, Values, Flags)
        %meIOStreamWrite Write data for streaming.

            iCount = int32(size(Values, 2));
            piCount = libpointer('int32Ptr', iCount);

            iBuff = int32(Values);
            piBuff = libpointer('int32Ptr', iBuff);

            err = calllib('meIDSmain', 'meIOStreamWrite', Device, Subdevice, WriteMode, piBuff, piCount, Flags);
            Count = get(piCount, 'Value');

            clear piCount;
            clear pBuff;
        end

        function [err, Time, TicksLow, TicksHigh] = meIOStreamTimeToTicks(Device, Subdevice, Timer, Time, Flags)
        %meIOStreamTimeToTicks

            dTime = double(Time);
            pdTime = libpointer('doublePtr', dTime);
            iTicksLow = int32(0);
            piTicksLow = libpointer('int32Ptr', iTicksLow);
            iTicksHigh = int32(0);
            piTicksHigh = libpointer('int32Ptr', iTicksHigh);

            err = calllib('meIDSmain', 'meIOStreamTimeToTicks', Device, Subdevice, Timer, pdTime, piTicksLow, piTicksHigh, Flags);
            Time = get(pdTime, 'Value');
            TicksLow = get(piTicksLow, 'Value');
            TicksHigh = get(piTicksHigh, 'Value');

            clear pdTime;
            clear piTicksLow;
            clear piTicksHigh;
        end

        function [err, Frequency, TicksLow, TicksHigh] = meIOStreamFrequencyToTicks(Device, Subdevice, Timer, Frequency, Flags)
        %meIOStreamFrequencyToTicks

            dFrequency = double(Frequency);
            pdFrequency = libpointer('doublePtr', dFrequency);
            iTicksLow = int32(0);
            piTicksLow = libpointer('int32Ptr', iTicksLow);
            iTicksHigh = int32(0);
            piTicksHigh = libpointer('int32Ptr', iTicksHigh);

            err = calllib('meIDSmain', 'meIOStreamFrequencyToTicks', Device, Subdevice, Timer, pdFrequency, piTicksLow, piTicksHigh, Flags);
            Frequency = get(pdFrequency, 'Value');
            TicksLow = get(piTicksLow, 'Value');
            TicksHigh = get(piTicksHigh, 'Value');

            clear pdFrequency;
            clear piTicksLow;
            clear piTicksHigh;
        end

        function [err, ChanBuffer] = meUtilityExtractValues(Channel, AIBuffer, Config)
        %meUtilityExtractValues

            Count = size(AIBuffer, 2);
            iAIBuffer = int32(AIBuffer);
            piAIBuffer = libpointer('int32Ptr', iAIBuffer);

            ConfigCount = size(Config, 2);
            pConfig = libpointer('meIOStreamConfigPtr', Config);

            iChanBuffer = zeros(1, Count, 'int32');
            piChanBuffer = libpointer('int32Ptr', iChanBuffer);

            iChanBufferCount = int32(Count);
            piChanBufferCount = libpointer('int32Ptr', iChanBufferCount);

            err  = calllib('meIDSmain', 'meUtilityExtractValues', Channel, piAIBuffer, Count, pConfig, ConfigCount, piChanBuffer, piChanBufferCount);

            Count = get(piChanBufferCount, 'Value');
            ChanBuffer = get(piChanBuffer, 'Value');
            ChanBuffer = ChanBuffer(1:Count);

            clear piChanBufferCount;
            clear piChanBuffer;
            clear pConfigList;
            clear piAIBuffer;
        end

        function [err, Physical] = meUtilityDigitalToPhysical(MinVal, MaxVal, MaxData, Digital, ModuleType, RefValue)
        %meUtilityDigitalToPhysical

            dPhysical = double(0);
            pdPhysical = libpointer('double', dPhysical);

            err = calllib('meIDSmain', 'meUtilityDigitalToPhysical', MinVal, MaxVal, MaxData, Digital, ModuleType, RefValue, pdPhysical);
            Physical = get(pdPhysical, 'Value');

            clear pdPhysical;
        end

        function [err, Physical] = meUtilityDigitalToPhysicalV(MinVal, MaxVal, MaxData, Digital, ModuleType, RefValue)
        %meUtilityDigitalToPhysicalV

            Count = size(Digital, 2);
            iDigital = int32(Digital);
            piDigital = libpointer('int32Ptr', iDigital);

            dPhysical = zeros(1, Count, 'double');
            pdPhysical = libpointer('doublePtr', dPhysical);

            err = calllib('meIDSmain', 'meUtilityDigitalToPhysicalV', MinVal, MaxVal, MaxData, piDigital, Count, ModuleType, RefValue, pdPhysical);

            Physical = get(pdPhysical, 'Value');

            clear piDigital;
            clear pdPhysical;
        end

        function [err, Digital] = meUtilityPhysicalToDigital(MinVal, MaxVal, MaxData, Physical)
        %meUtilityPhysicalToDigital

            iDigital = int32(0);
            piDigital = libpointer('int32Ptr', iDigital);

            err = calllib('meIDSmain', 'meUtilityPhysicalToDigital', MinVal, MaxVal, MaxData, Physical, piDigital);
            Digital = get(piDigital, 'Value');
            clear piDigital;
        end

        function [err, Digital] = meUtilityPhysicalToDigitalV(MinVal, MaxVal, MaxData, Physical)
        %meUtilityPhysicalToDigitalV

            Count = size(Physical, 2);
            dPhysical = double(Physical);
            pdPhysical = libpointer('doublePtr', dPhysical);

            iDigital = zeros(1, Count, 'int32');
            piDigital = libpointer('int32Ptr', iDigital);

            err = calllib('meIDSmain', 'meUtilityPhysicalToDigitalV', MinVal, MaxVal, MaxData, pdPhysical, Count, piDigital);

            Digital = get(piDigital, 'Value');

            clear piDigital;
            clear pdPhysical;
        end

        function [err, Version] = meQueryVersionLibrary()
        %meQueryVersionLibrary Get MEiDS version.

            iVersion = int32(0);
            piVersion = libpointer('int32Ptr', iVersion);

            [err, Version] = calllib('meIDSmain', 'meQueryVersionLibrary', piVersion);

            clear piVersion;
        end

        function [err, Version] = meQueryVersionMainDriver()
        %meQueryVersionMainDriver Get MEiDS driver version.

            iVersion = int32(0);
            piVersion = libpointer('int32Ptr', iVersion);

            [err, Version] = calllib('meIDSmain', 'meQueryVersionMainDriver', piVersion);

            clear piVersion;
        end

        function [err, Version] = meQueryVersionDeviceDriver(Device)
        %meQueryVersionDeviceDriver Get version of board specific driver.

            iVersion = int32(0);
            piVersion = libpointer('int32Ptr', iVersion);

            [err, Version] = calllib('meIDSmain', 'meQueryVersionDeviceDriver', Device, piVersion);

            clear piVersion;
        end

        function [err, Name]  = meQueryNameDeviceDriver(Device)
        %meQueryNameDeviceDriver Get driver name.

            Buff = [int8(1:meIDS.ME_DEVICE_NAME_MAX_COUNT) 0];
            pBuff = libpointer('voidPtr', Buff);

            [err, Name] = calllib('meIDSmain', 'meQueryNameDeviceDriver', Device, pBuff, meDefines.ME_DEVICE_NAME_MAX_COUNT);

            clear pBuff;
        end

        function [err, Description] = meQueryDescriptionDevice(Device)
        %meQueryDescriptionDevice Get device description.

            Buff = [int8(1:meIDS.ME_DEVICE_DESCRIPTION_MAX_COUNT) 0];
            pBuff = libpointer('voidPtr', Buff);

            [err, Description] = calllib('meIDSmain', 'meQueryDescriptionDevice', Device, pBuff, meDefines.ME_DEVICE_DESCRIPTION_MAX_COUNT);

            clear pBuff;
        end

        function [err, Name] = meQueryNameDevice(Device)
        %meQueryNameDevice Get device name.

            Buff = [int8(1:meIDS.ME_DEVICE_NAME_MAX_COUNT) 0];
            pBuff = libpointer('voidPtr', Buff);

            [err, Name] = calllib('meIDSmain', 'meQueryNameDevice', Device, pBuff, meDefines.ME_DEVICE_NAME_MAX_COUNT);

            clear pBuff;
        end

        function [err, VendorId, DeviceId, SerialNo, BusType, BusNo, DevNo, FuncNo, Plugged] = meQueryInfoDevice(Device)
        %meQueryInfoDevice Get device signature.
            iVendorId = int32(0);
            piVendorId = libpointer('int32Ptr', iVendorId);
            iDeviceId = int32(0);
            piDeviceId = libpointer('int32Ptr', iDeviceId);
            iSerialNo = int32(0);
            piSerialNo = libpointer('int32Ptr', iSerialNo);
            iBusType = int32(0);
            piBusType = libpointer('int32Ptr', iBusType);
            iBusNo = int32(0);
            piBusNo = libpointer('int32Ptr', iBusNo);
            iDevNo = int32(0);
            piDevNo = libpointer('int32Ptr', iDevNo);
            iFuncNo = int32(0);
            piFuncNo = libpointer('int32Ptr', iFuncNo);
            iPlugged = int32(0);
            piPlugged = libpointer('int32Ptr', iPlugged);

            [err, VendorId, DeviceId, SerialNo, BusType, BusNo, DevNo, FuncNo, Plugged] = calllib('meIDSmain', 'meQueryInfoDevice', Device, piVendorId, piDeviceId, piSerialNo, piBusType, piBusNo, piDevNo, piFuncNo, piPlugged);

            clear piVendorId;
            clear piDeviceId;
            clear piSerialNo;
            clear piBusType;
            clear piBusNo;
            clear piDevNo;
            clear piFuncNo;
            clear piPlugged;
        end

        function [err, Number] = meQueryNumberDevices()
        %meQueryNumberDevices Get number of devices in registered system.

            iNumber = int32(0);
            piNumber = libpointer('int32Ptr', iNumber);

            err = calllib('meIDSmain', 'meQueryNumberDevices', piNumber);
            Number = get(piNumber, 'Value');

            clear piNumber;
        end

        function [err, Number] = meQueryNumberSubdevices(Device)
        %meQueryNumberSubdevices Get number of sub-devices.

            iNumber = int32(0);
            piNumber = libpointer('int32Ptr', iNumber);

            [err, Number] = calllib('meIDSmain', 'meQueryNumberSubdevices', Device, piNumber);

            clear piNumber;
        end

        function [err, Number] = meQueryNumberChannels(Device, Subdevice)
        %meQueryNumberChannels Get number of channels on sub-devices.

            iNumber = int32(0);
            piNumber = libpointer('int32Ptr', iNumber);

            [err, Number] = calllib('meIDSmain', 'meQueryNumberChannels', Device, Subdevice, piNumber);

            clear piNumber;

        end

        function [err, Number] = meQueryNumberRanges(Device, Subdevice, Unit)
        %meQueryNumberRanges Get number of ranges supported by sub-devices.

            iNumber = int32(0);
            piNumber = libpointer('int32Ptr', iNumber);

            [err, Number] = calllib('meIDSmain', 'meQueryNumberRanges', Device, Subdevice, Unit, piNumber);

            clear piNumber;
        end

        function [err, Type, Subtype] = meQuerySubdeviceType(Device, Subdevice)
        %meQuerySubdeviceType Get subdevice type.

            iType = int32(0);
            piType = libpointer('int32Ptr', iType);
            iSubtype = int32(0);
            piSubtype = libpointer('int32Ptr', iSubtype);

            [err, Type, Subtype] = calllib('meIDSmain', 'meQuerySubdeviceType', Device, Subdevice, piType, piSubtype);

            clear piType;
            clear piSubtype;
        end

        function [err, Subdevice] = meQuerySubdeviceByType(Device, StartSubdevice, Type, Subtype)
        %meQuerySubdeviceByType Search for subdevice.

            iSubdevice = int32(0);
            piSubdevice = libpointer('int32Ptr', iSubdevice);

            err = calllib('meIDSmain', 'meQuerySubdeviceByType', Device, StartSubdevice, Type, Subtype, piSubdevice);

            Subdevice = get(piSubdevice, 'Value');
            clear piSubdevice;
        end

        function [err, MinVal, MaxVal, MaxData, Unit] = meQueryRangeInfo(Device, Subdevice, Range)
        %meQueryRangeInfo Get range parameters.

            iMin = int32(0);
            piMin = libpointer('doublePtr', iMin);
            iMax = int32(0);
            piMax = libpointer('doublePtr', iMax);
            iMaxData = int32(0);
            piMaxData = libpointer('int32Ptr', iMaxData);
            iUnit = int32(0);
            piUnit = libpointer('int32Ptr', iUnit);

            err = calllib('meIDSmain', 'meQueryRangeInfo', Device, Subdevice, Range, piUnit, piMin, piMax, piMaxData);

            Unit = get(piUnit, 'Value');
            MinVal = get(piMin, 'Value');
            MaxVal = get(piMax, 'Value');
            MaxData = get(piMaxData, 'Value');

            clear piMin;
            clear piMax;
            clear piMaxData;
            clear piUnit;
        end

        function [err, MinVal, MaxVal, MaxData, Range] = meQueryRangeByMinMax(Device, Subdevice, Unit, MinVal, MaxVal, MaxData)
        %meQueryRangeByMinMax Query for aproperiate rang.

            iMin = double(MinVal);
            piMin = libpointer('doublePtr', iMin);
            iMax = double(MaxVal);
            piMax = libpointer('doublePtr', iMax);
            iMaxData = int32(MaxData);
            piMaxData = libpointer('int32Ptr', iMaxData);
            iRange = int32(0);
            piRange = libpointer('int32Ptr', iRange);

            [err, MinVal, MaxVal, MaxData, Range] = calllib('meIDSmain', 'meQueryRangeByMinMax', Device, Subdevice, Unit, piMin, piMax, piMaxData, piRange);

            clear piMin;
            clear piMax;
            clear piMaxData;
            clear piRange;
        end

        function [err, Caps] = meQuerySubdeviceCaps(Device, Subdevice)
        %meQuerySubdeviceCaps Get subdevice CAPS.

            iCaps = int32(0);
            piCaps = libpointer('int32Ptr', iCaps);

            [err, Caps] = calllib('meIDSmain', 'meQuerySubdeviceCaps', Device, Subdevice, piCaps);

            clear piCaps;
        end

        function [err, CapsArg] = meQuerySubdeviceCapsArgs(Device, Subdevice, Caps)
        %meQuerySubdeviceCapsArgs Get subdevice CAPS argument.

            iCapsArg = int32(0);
            piCapsArg = libpointer('int32Ptr', iCapsArg);

            [err, CapsArg] = calllib('meIDSmain', 'meQuerySubdeviceCapsArgs', Device, Subdevice, Caps, piCapsArg, 1);

            clear piCapsArg;
        end

        function [err] = meUtilityPWMStart(Device, Counter1, Counter2, Counter3, Ref, Prescaler, DutyCycle, Flag)
        %meUtilityPWMStart

            err = calllib('meIDSmain', 'meUtilityPWMStart', Device, Counter1, Counter2, Counter3, Ref, Prescaler, DutyCycle, Flag);
        end

        function [err] = meUtilityPWMStop(Device, Counter1)
        %meUtilityPWMStop

            err = calllib('meIDSmain', 'meUtilityPWMStop', Device, Counter1);
        end

    end
end

