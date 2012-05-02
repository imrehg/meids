%AISimpleStream AI stream simple test
meIDS.meOpen(meIDS.ME_OPEN_NO_FLAGS);
for l=1:100
    DataBlockSize = 1000;
    Range = 0;    
    
    [err, NoDev] = meIDS.meQueryNumberDevices();
    if NoDev == 0
        return;
    end
    NoDev = NoDev-1;
    [err, NoSubDev] = meIDS.meQuerySubdeviceByType(NoDev, 0, meIDS.ME_TYPE_AI, meIDS.ME_SUBTYPE_STREAMING);
    if err ~= 0
        return;
    end

    err = meIDS.meIOResetSubdevice(NoDev, NoSubDev, meIDS.ME_IO_RESET_DEVICE_NO_FLAGS);
    if err ~= 0
        return;
    end

    Trigger = meIDS.meStructStreamTrigger;
    Trigger.iAcqStartTrigType = meIDS.ME_TRIG_TYPE_SW;
    Trigger.iAcqStartTrigEdge = meIDS.ME_TRIG_EDGE_NONE;
    Trigger.iAcqStartTrigChan = meIDS.ME_TRIG_CHAN_DEFAULT;
    Trigger.iAcqStartTicksLow = 66;
    Trigger.iAcqStartTicksHigh = 0;
    Trigger.iScanStartTrigType = meIDS.ME_TRIG_TYPE_TIMER;
    Trigger.iScanStartTicksLow = 33000;
    Trigger.iScanStartTicksHigh = 0;
    Trigger.iConvStartTrigType = meIDS.ME_TRIG_TYPE_TIMER;
    Trigger.iConvStartTicksLow = 66;
    Trigger.iConvStartTicksHigh = 0;
    Trigger.iScanStopTrigType = meIDS.ME_TRIG_TYPE_NONE;
    Trigger.iScanStopCount = 0;
    Trigger.iAcqStopTrigType = meIDS.ME_TRIG_TYPE_COUNT;
    Trigger.iAcqStopCount = DataBlockSize;

    Config = meIDS.meStructStreamConfig;
    Config.iStreamConfig = Range;
    Config.iRef = meIDS.ME_REF_AI_GROUND;

    err = meIDS.meIOStreamConfig(NoDev, NoSubDev, Config, Trigger, DataBlockSize, meIDS.ME_IO_STREAM_CONFIG_NO_FLAGS);
    if err ~= 0
        return;
    end

    Start = meIDS.meStructStreamStart;
    Start.iDevice = NoDev;
    Start.iSubdevice = NoSubDev;
    Start.iStartMode = meIDS.ME_START_MODE_BLOCKING;
    Start.iTimeOut = 0;

    err = meIDS.meIOStreamStart(Start, meIDS.ME_IO_STREAM_START_NO_FLAGS);
    if err ~= 0
        return;
    end

    [err, Digital] = meIDS.meIOStreamRead(NoDev, NoSubDev, meIDS.ME_READ_MODE_BLOCKING, DataBlockSize, meIDS.ME_IO_STREAM_READ_NO_FLAGS);
    if err ~= 0
        return;
    end

    Stop = meIDS.meStructStreamStop;
    Stop.iDevice = NoDev;
    Stop.iSubdevice = NoSubDev;
    Stop.iStopMode = meIDS.ME_STOP_MODE_IMMEDIATE;

    err = meIDS.meIOStreamStop(Stop, meIDS.ME_IO_STREAM_STOP_NO_FLAGS);
    if err ~= 0
        return;
    end

    [err, MinVal, MaxVal, MaxData, Unit] = meIDS.meQueryRangeInfo(NoDev, NoSubDev, Range);
    if err ~= 0
        return;
    end
    [err, Physical] = meIDS.meUtilityDigitalToPhysicalV(MinVal, MaxVal, MaxData, Digital, meIDS.ME_MODULE_TYPE_MULTISIG_NONE, meIDS.ME_VALUE_NOT_USED);
    if err ~= 0
        return;
    end
    ploter = plot(Physical);
    title('MEiDS for MATLAB: analog input simple example.');
    
pause(0.1);
end 
meIDS.meClose(meIDS.ME_CLOSE_NO_FLAGS);