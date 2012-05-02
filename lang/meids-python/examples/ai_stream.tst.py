#!/usr/bin/env python

import meDriver
import sys
import time
import string

""" Python version of ai_stream.tst
"""
def GetStreamAI(NoDev, NoSubDev):
    DataBlockSize = 0x10000
    DataReadSize = 0x1000
    AIBufSize = 0x400
    
    #Preaparing trigger list
    TrigList = {'AcqStartTrigType'	: meDriver.ME_TRIG_TYPE_SW,
                    'AcqStartTrigEdge'	: meDriver.ME_TRIG_EDGE_NONE,
                    'AcqStartTrigChan'	: meDriver.ME_TRIG_CHAN_DEFAULT,
                    'AcqStartTicks'	: long(66),
                    'ScanStartTrigType'	: meDriver.ME_TRIG_TYPE_TIMER,
                    'ScanStartTicks'	: long(66),
                    'ConvStartTrigType'	: meDriver.ME_TRIG_TYPE_TIMER,
                    'ConvStartTicks'	: long(33000),    #1kHz
                    'ScanStopTrigType'	: meDriver.ME_TRIG_TYPE_NONE,
                    'ScanStopCount'	: 0,
                    'AcqStopTrigType'	: meDriver.ME_TRIG_TYPE_COUNT,
                    'AcqStopCount'	: DataBlockSize,
                    'Flags'		: meDriver.ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS}
    #Preaparing config list
    ConfList = []
    ConfEntry = {'Channel': 0,
                    'StreamConfig': 0,
                    'Ref': meDriver.ME_REF_AI_GROUND,
                    'Flags': meDriver.ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS}
    ConfList.append(ConfEntry)
    #Preaparing start list
    StartList = []
    StartEntry = {'Device': NoDev,
                 'Subdevice': NoSubDev,
                 'StartMode': meDriver.ME_START_MODE_BLOCKING,	#This is returning when stream STARTS!
                 'TimeOut': 0,
                 'Flags': meDriver.ME_IO_STREAM_START_TYPE_NO_FLAGS, 
                 'Errno': 0}
    StartList.append(StartEntry)

    #Preaparing stop list
    StopList = []
    StopEntry = {'Device': NoDev,
                    'Subdevice': NoSubDev,
                    'StopMode': meDriver.ME_STOP_MODE_IMMEDIATE,
                    'Flags': meDriver.ME_IO_STREAM_STOP_TYPE_NO_FLAGS,
                    'Errno': 0}
    StopList.append(StopEntry)

    #Using range 0
    (iUnit, dMin, dMax, iMaxData) = meDriver.meQueryRangeInfo(NoDev, NoSubDev, 0)
    
    meDriver.meIOResetSubdevice(NoDev, NoSubDev, meDriver.ME_VALUE_NOT_USED)
    #Configure
    print "Configure AI"
    meDriver.meIOStreamConfig(NoDev, NoSubDev, ConfList, TrigList, AIBufSize, meDriver.ME_IO_STREAM_CONFIG_NO_FLAGS)
    print "Start AI"
    meDriver.meIOStreamStart(StartList, meDriver.ME_IO_STREAM_START_NO_FLAGS)
    
    print "Work"
    totalCount = 0
    while (totalCount<DataBlockSize):
        #Read data (blocking mode)
        AIBufferCount=AIBufSize
        AIBuffer = meDriver.meIOStreamRead(NoDev, NoSubDev, meDriver.ME_READ_MODE_BLOCKING, AIBufferCount, meDriver.ME_IO_STREAM_READ_NO_FLAGS)
        #Show some samples
        print "\tSample 0x%x => %f" % (totalCount,  meDriver.meUtilityDigitalToPhysical( dMin, dMax, iMaxData, AIBuffer[0], meDriver.ME_MODULE_TYPE_MULTISIG_NONE, 0))
        totalCount = totalCount + len(AIBuffer)
    
    print "Stop AI"
    meDriver.meIOStreamStop(StopList, meDriver.ME_IO_STREAM_STOP_NO_FLAGS)
    print "AI stopped."

def main():
    try:
        #Initialization of driver
        meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
        #Get library version
        LibVer = meDriver.meQueryVersionLibrary()
        LibVer_Special = LibVer / 0x1000000
        LibVer_API = LibVer / 0x10000
        LibVer_Relase = LibVer / 0x100
        LibVer_Compilation = LibVer
        print "Library version is %02x.%02x.%02x.%02x" % (LibVer_Special%0xFF, LibVer_API&0xFF, LibVer_Relase&0xFF, LibVer_Compilation&0xFF)
        #Get main driver version
        DrvVer = meDriver.meQueryVersionMainDriver()
        DrvVer_Special = DrvVer / 0x1000000
        DrvVer_API = DrvVer / 0x10000
        DrvVer_Relase = DrvVer / 0x100
        DrvVer_Compilation = DrvVer
        print "Main driver version is %02x.%02x.%02x.%02x" % (DrvVer_Special&0xFF, DrvVer_API&0xFF, DrvVer_Relase&0xFF, DrvVer_Compilation&0xFF)
        #Get number of registered devices
        meNumberDevices = meDriver.meQueryNumberDevices()
        print "%i devices in system" % (meNumberDevices)
        print
        for meNoDevices in range(meNumberDevices):
            meNumberSubdevices = meDriver.meQueryNumberSubdevices(meNoDevices)
            try:
                meNoSubdevices = 0
                while meNoSubdevices<meNumberSubdevices:
                    #Get all streamed analog input sub-devices
                    meNoSubdevices = meDriver.meQuerySubdeviceByType(meNoDevices, meNoSubdevices, meDriver.ME_TYPE_AI, meDriver.ME_SUBTYPE_STREAMING)
                    GetStreamAI(meNoDevices, meNoSubdevices)
                    meDriver.meIOResetSubdevice(meNoDevices, meNoSubdevices, meDriver.ME_VALUE_NOT_USED);
                    meNoSubdevices = meNoSubdevices + 1
            except meDriver.error:
                pass

    except meDriver.error, Val:
        print sys.exc_info()
        print Val
    except:
        print sys.exc_info()
        pass
    finally:
        #Close ME-iDS
        meDriver.meClose(meDriver.ME_OPEN_NO_FLAGS)

#Start execute
main()

