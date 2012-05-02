/**
 * @note Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * @file : me_internal.h
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _ME_INTERNAL_H_
# define _ME_INTERNAL_H_

# include "../common/meinternal.h"

/*=============================================================================
  PCI Vendor IDs
  ===========================================================================*/
# define MEILHAUS_PCI_VENDOR_ID						PCI_VENDOR_ID_MEILHAUS

/*=============================================================================
  USB Vendor IDs - SynapseUSB
  ===========================================================================*/
# define MEILHAUS_USB_VENDOR_ID						USB_VENDOR_ID_MEILHAUS
# define NET2282_PCI_VENDOR_ID						0x228217cc

/*=============================================================================
  Drivers' names
  ===========================================================================*/
#  if defined(ME_PCI)
#   if defined(ME_USB)
#    error PCI and USB declared!
#    undefine ME_USB
#   endif
#   if defined(ME_COMEDI)
#    error PCI and COMEDI declared!
#    undefine ME_COMEDI
#   endif
#   define ME_NAME_DRIVER							"memainPCI"
#   define ME_NAME_NODE								"medriverPCI"
#   define ME0600_NAME								"me0600PCI"	/*"me630"*/
#   define ME0700_NAME								"me0700PCI"	/*"akson"*/
#   define ME0900_NAME								"me0900PCI"	/*"me9x"*/
#   define ME1000_NAME								"me1000PCI"
#   define ME1400_NAME								"me1400PCI"
#   define ME1600_NAME								"me1600PCI"
#   define ME4500_NAME								"me4500PCI"
#   define ME4600_NAME								"me4600PCI"
#   define ME4700_NAME								"me4700PCI"
#   define ME4800_NAME								"me4800PCI"
#   define ME6000_NAME								"me6000PCI"
#   define ME8100_NAME								"me8100PCI"
#   define ME8200_NAME								"me8200PCI"
#   define MEFF00_NAME								"meFF00PCI"
#  elif defined(ME_USB)
#   if defined(ME_COMEDI)
#    error USB and COMEDI declared!
#    undefine ME_COMEDI
#   endif
#   define ME_NAME_DRIVER							"memainUSB"
#   define ME_NAME_NODE								"medriverUSB"
#   define ME0600_NAME								"me0600USB"	/*"me630"*/
#   define ME0700_NAME								"me0700USB"	/*"akson"*/
#   define ME0900_NAME								"me0900USB"	/*"me9x"*/
#   define ME1000_NAME								"me1000USB"
#   define ME1400_NAME								"me1400USB"
#   define ME1600_NAME								"me1600USB"
#   define ME4500_NAME								"me4500USB"
#   define ME4600_NAME								"me4600USB"
#   define ME4700_NAME								"me4700USB"
#   define ME4800_NAME								"me4800USB"
#   define ME6000_NAME								"me6000USB"
#   define ME8100_NAME								"me8100USB"
#   define ME8200_NAME								"me8200USB"
#  elif defined(ME_COMEDI)
#   define ME0600_NAME								"me0600COMEDI"	/*"me630"*/
#   define ME0700_NAME								"me0700COMEDI"	/*"akson"*/
#   define ME0900_NAME								"me0900COMEDI"	/*"me9x"*/
#   define ME1000_NAME								"me1000COMEDI"
#   define ME1400_NAME								"me1400COMEDI"
#   define ME1600_NAME								"me1600COMEDI"
#   define ME4500_NAME								"me4500COMEDI"
#   define ME4600_NAME								"me4600COMEDI"
#   define ME4700_NAME								"me4700COMEDI"
#   define ME4800_NAME								"me4800COMEDI"
#   define ME6000_NAME								"me6000COMEDI"
#   define ME8100_NAME								"me8100COMEDI"
#   define ME8200_NAME								"me8200COMEDI"
#  elif defined(ME_MEPHISTO)
#   define ME_NAME_DRIVER							"meMEPHISTO"
#   define ME_NAME_NODE								"mephistoSC"
#   define MEPHISTO_NAME							"MephistoScope"
#  else
#   error NO VALID DRIVER TYPE declared!
#  endif


/* ME-630 */
# define PCI_DEVICE_ID_MEILHAUS_ME0630				PCI_DEVICE_ID_MEILHAUS_ME630

# define ME0600_NAME_DEVICE_ME0630					ME630_NAME_DEVICE_ME630
# define ME0600_DESCRIPTION_DEVICE_ME0630			ME630_DESCRIPTION_DEVICE_ME630

/* ME0900 */
# define PCI_DEVICE_ID_MEILHAUS_ME0940				PCI_DEVICE_ID_MEILHAUS_ME94
# define PCI_DEVICE_ID_MEILHAUS_ME0950				PCI_DEVICE_ID_MEILHAUS_ME95
# define PCI_DEVICE_ID_MEILHAUS_ME0960				PCI_DEVICE_ID_MEILHAUS_ME96

# define ME0900_NAME_DEVICE_ME0940					ME9x_NAME_DEVICE_ME94
# define ME0900_NAME_DEVICE_ME0950					ME9x_NAME_DEVICE_ME95
# define ME0900_NAME_DEVICE_ME0960					ME9x_NAME_DEVICE_ME96

# define ME0900_DESCRIPTION_DEVICE_ME0940			ME9x_DESCRIPTION_DEVICE_ME94
# define ME0900_DESCRIPTION_DEVICE_ME0950			ME9x_DESCRIPTION_DEVICE_ME95
# define ME0900_DESCRIPTION_DEVICE_ME0960			ME9x_DESCRIPTION_DEVICE_ME96

/* Error defines */
# define EMPTY_NAME_DEVICE							"ME-???"
# define EMPTY_DESCRIPTION_DEVICE					"ME-??? unknown device"

# define ME_UNKNOWN_RELEASE							0x00000000
#endif
