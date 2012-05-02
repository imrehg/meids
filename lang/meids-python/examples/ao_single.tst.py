#!/usr/bin/env python

import meDriver
import sys
import time
import string

""" Python version of ao_single.tst
"""
def GetAO(NoDev, NoSubDev, NoChannel):
    #Using range 0
    (iUnit, dMin, dMax, iMaxData) = meDriver.meQueryRangeInfo(NoDev, NoSubDev, 0)
    #Read from sub-device
    SingleDict = {
        'Device':   NoDev,
        'Subdevice':    NoSubDev,
        'Channel':  NoChannel,
        'Dir':      meDriver.ME_DIR_INPUT,
        'Value':    0,
        'TimeOut':  meDriver.ME_VALUE_NOT_USED,
        'Flags':    meDriver.ME_IO_SINGLE_TYPE_NO_FLAGS,
        'Errno':    0
    }
    SingleList = [SingleDict]
    meDriver.meIOSingle(SingleList, meDriver.ME_IO_SINGLE_NO_FLAGS)
    print "Device [%d:%d/%d] GET:%f(0x%x)" % (NoDev, NoSubDev, NoChannel,  meDriver.meUtilityDigitalToPhysical(dMin, dMax, iMaxData, SingleList[0]['Value'], meDriver.ME_MODULE_TYPE_MULTISIG_NONE, 0),  SingleList[0]['Value'])
 
def SetAO(NoDev, NoSubDev, NoChannel):
    #Get maximalaceppted value
    #Using range 0
    (iUnit, dMin, dMax, iMaxData) = meDriver.meQueryRangeInfo(NoDev, NoSubDev, 0)
    STEPS = 9
    step = (dMax-dMin)/(STEPS-1)

    #Configure sub-device for output
    meDriver.meIOSingleConfig(NoDev, NoSubDev, NoChannel, 0,  meDriver.ME_REF_AO_GROUND, meDriver.ME_TRIG_CHAN_DEFAULT, meDriver.ME_TRIG_TYPE_SW, meDriver.ME_TRIG_EDGE_NONE, meDriver.ME_IO_SINGLE_NO_FLAGS)
    for i in range(STEPS):
        #Write to sub-device
        SingleDict = {
            'Device':   NoDev,
            'Subdevice':    NoSubDev,
            'Channel':  NoChannel,
            'Dir':      meDriver.ME_DIR_OUTPUT,
            'Value':    meDriver.meUtilityPhysicalToDigital(dMin, dMax, iMaxData, dMin + (step * i)),
            'TimeOut':  meDriver.ME_VALUE_NOT_USED,
            'Flags':    meDriver.ME_IO_SINGLE_TYPE_NO_FLAGS,
            'Errno':    0
        }
        SingleList = [SingleDict]
        meDriver.meIOSingle(SingleList, meDriver.ME_IO_SINGLE_NO_FLAGS)
        print "Device [%d:%d/%d] SET:%f(0x%x)" % (NoDev, NoSubDev, NoChannel,  dMin + (step * i),  SingleList[0]['Value'])
        #Read back
        GetAO(NoDev, NoSubDev, NoChannel)
        time.sleep(1)


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
                for meNoSubdevices in range(meNumberSubdevices):
                    #Get all analog output sub-devices
                    meNoSubdevices = meDriver.meQuerySubdeviceByType(meNoDevices, meNoSubdevices, meDriver.ME_TYPE_AO, meDriver.ME_SUBTYPE_ANY)
                    try:
                        meNumberChannels = meDriver.meQueryNumberChannels(meNoDevices,  meNoSubdevices)
                    except meDriver.error:
                        meNumberChannels = 0
                    for meNoChannels in range(meNumberChannels):
                        #Do test for each channel
                        SetAO(meNoDevices, meNoSubdevices,   meNoChannels)
                    meDriver.meIOResetSubdevice(meNoDevices, meNoSubdevices, meDriver.ME_VALUE_NOT_USED)
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

