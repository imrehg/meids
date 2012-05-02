#!/usr/bin/env python

import meDriver
import os
import sys
import time

"""Python version of query.tst
"""

#Definitions
typeSubdevice = {
 		meDriver.ME_SUBTYPE_INVALID : 'INVALID',
 		meDriver.ME_SUBTYPE_SINGLE : 'SINGLE',
 		meDriver.ME_SUBTYPE_STREAMING : 'STREAMING',
 		meDriver.ME_SUBTYPE_CTR_8254 : '8254',
 		meDriver.ME_SUBTYPE_ANY : 'ANY'}

typeDevice = {
 		meDriver.ME_TYPE_INVALID :'INVALID',
 		meDriver.ME_TYPE_AO : 'ANALOG OUTPUT',
 		meDriver.ME_TYPE_AI : 'ANALOG INPUT',
 		meDriver.ME_TYPE_DIO : 'DIGITAL INPUT/OUTPUT',
 		meDriver.ME_TYPE_DO : 'DIGITAL OUTPUT',
 		meDriver.ME_TYPE_DI : 'DIGITAL INPUT',
 		meDriver.ME_TYPE_CTR : 'COUNTER',
 		meDriver.ME_TYPE_EXT_IRQ : 'EXTERNAL IRQ',
 		meDriver.ME_TYPE_FREQ_IO : 'FREQUENCY INPUT/OUTPUT',
 		meDriver.ME_TYPE_FREQ_O : 'FREQUENCY OUTPUT',
 		meDriver.ME_TYPE_FREQ_I : 'FREQUENCY INPUT'}

def main():
    try:
        #Initialization of ME-iDS
        meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
        #Get library version
        LibVer = meDriver.meQueryVersionLibrary()
        LibVer_Special = LibVer / 0x1000000
        LibVer_API = LibVer / 0x10000
        LibVer_Relase = LibVer / 0x100
        LibVer_Compilation = LibVer
        print "Library version is %02x.%02x.%02x.%02x" % (LibVer_Special%0xFF, LibVer_API&0xFF, LibVer_Relase&0xFF, LibVer_Compilation&0xFF)
        #Get number of registered devices
        meNumberDevices = meDriver.meQueryNumberDevices()
        if meNumberDevices > 0:
            #Get main driver version
            DrvVer = meDriver.meQueryVersionMainDriver()
            DrvVer_Special = DrvVer / 0x1000000
            DrvVer_API = DrvVer / 0x10000
            DrvVer_Relase = DrvVer / 0x100
            DrvVer_Compilation = DrvVer
            print "Main driver version is %02x.%02x.%02x.%02x" % (DrvVer_Special&0xFF, DrvVer_API&0xFF, DrvVer_Relase&0xFF, DrvVer_Compilation&0xFF)
            print "%i devices detected by driver system" % (meNumberDevices)

            for meNoDevices in range(meNumberDevices):
                #For each device get: 
                # - device name
                # - device description
                # - driver name
                # - driver version
                # - number sub-devices
                print
                print "Device %i:" % (meNoDevices)
                print "========="
                print "Device name is %s" % (meDriver.meQueryNameDevice(meNoDevices))
                print "Driver name is %s" % (meDriver.meQueryNameDeviceDriver(meNoDevices))
                print "Device description: %s" % (meDriver.meQueryDescriptionDevice(meNoDevices))
                DrvVer = meDriver.meQueryVersionDeviceDriver(meNoDevices)
                DrvVer_Special = DrvVer / 0x1000000
                DrvVer_API = DrvVer / 0x10000
                DrvVer_Relase = DrvVer / 0x100
                DrvVer_Compilation = DrvVer
                print "Device driver version is %02x.%02x.%02x.%02x" % (DrvVer_Special&0xFF, DrvVer_API&0xFF, DrvVer_Relase&0xFF, DrvVer_Compilation&0xFF)

                meNumberSubdevices = meDriver.meQueryNumberSubdevices(meNoDevices)
                for meNoSubdevices in range(meNumberSubdevices):
                    #For each sub-device get: 
                    # - type
                    # - sub-type
                    (meType, meSubType) = meDriver.meQuerySubdeviceType(meNoDevices, meNoSubdevices)
                    print "\tSubdevice %d is of type %s (0x%X) and subtype %s (0x%X)" % (meNoSubdevices, typeDevice[meType], meType , typeSubdevice[meSubType], meSubType);
                    print "\t\tSubdevice %d has %d channels" % (meNoSubdevices, meDriver.meQueryNumberChannels(meNoDevices, meNoSubdevices));
                    if (meType == meDriver.ME_TYPE_AI) or (meType == meDriver.ME_TYPE_AO):
                        #For  AI and AO sub-device get number ranges 
                        meNumberRanges = meDriver.meQueryNumberRanges(meNoDevices, meNoSubdevices, meDriver.ME_UNIT_ANY);
                        print "\t\tSubdevice %d has %d ranges:" % (meNoSubdevices, meNumberRanges);
                        for meNoRanges in range(meNumberRanges):
                            (meUnit,  meMin,  meMax,  meMax_data) = meDriver.meQueryRangeInfo(meNoDevices, meNoSubdevices,  meNoRanges)
                            print "\t\t\tRange %d: Unit = 0x%X, Min = %lf, Max = %lf, Max Data = %d" %(meNoRanges, meUnit,  meMin,  meMax,  meMax_data);

    except meDriver.error, Val:
        print sys.exc_info()
        print Val

    except:
        print sys.exc_info()
        pass
    finally:
        #Close ME-iDS
        meDriver.meClose(meDriver.ME_CLOSE_NO_FLAGS)

main()
