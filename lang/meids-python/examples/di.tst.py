#!/usr/bin/env python

import meDriver
import sys
import time
import string

""" Python version of di.tst
"""
def GetDI(NoDev, NoSubDev, Name):
    #Configure sub-device for input
    meDriver.meIOSingleConfig(NoDev, NoSubDev, 0, meDriver.ME_SINGLE_CONFIG_DIO_INPUT, meDriver.ME_REF_NONE, meDriver.ME_TRIG_CHAN_NONE, meDriver.ME_TRIG_TYPE_NONE, meDriver.ME_TRIG_EDGE_NONE, meDriver.ME_IO_SINGLE_NO_FLAGS)
    #Read from sub-device
    SingleDict = {
        'Device':   NoDev,
        'Subdevice':    NoSubDev,
        'Channel':  0,
        'Dir':      meDriver.ME_DIR_INPUT,
        'Value':    0,
        'TimeOut':  meDriver.ME_VALUE_NOT_USED,
        'Flags':    meDriver.ME_IO_SINGLE_TYPE_NO_FLAGS,
        'Errno':    0
    }
    SingleList = [SingleDict]
    meDriver.meIOSingle(SingleList, meDriver.ME_IO_SINGLE_NO_FLAGS)
    print "Device %d: %s%d value=0x%x" % (NoDev, Name, NoSubDev, SingleList[0]['Value'])
 
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
                    #Get all digital input sub-devices
                    meNoSubdevices = meDriver.meQuerySubdeviceByType(meNoDevices, meNoSubdevices, meDriver.ME_TYPE_DI, meDriver.ME_SUBTYPE_ANY)
                    GetDI(meNoDevices, meNoSubdevices,  "DI")
                    meNoSubdevices = meNoSubdevices + 1
            except meDriver.error:
                pass
            try:
                for meNoSubdevices in range(meNumberSubdevices):
                    #Get all digital input/output sub-devices
                    meNoSubdevices = meDriver.meQuerySubdeviceByType(meNoDevices, meNoSubdevices, meDriver.ME_TYPE_DIO, meDriver.ME_SUBTYPE_ANY)
                    GetDI(meNoDevices, meNoSubdevices,  "DIO")
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

