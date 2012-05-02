/**
 * @file meinternal.h
 *
 * @brief API headers.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

#ifndef _MEINTERNAL_H_
# define _MEINTERNAL_H_

/*=============================================================================
  PCI Vendor IDs
  ===========================================================================*/
# define PCI_VENDOR_ID_MEILHAUS					0x1402

/*=============================================================================
  USB Vendor IDs - SynapseUSB
  ===========================================================================*/
# define USB_VENDOR_ID_MEILHAUS					0x1B04

/*=============================================================================
  PCI Device IDs
  ===========================================================================*/

/* ME1000 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME1000			0x1000
#define PCI_DEVICE_ID_MEILHAUS_ME1000_A			0x100A
#define PCI_DEVICE_ID_MEILHAUS_ME1000_B			0x100B

/* ME1400 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME1400			0x1400
#define PCI_DEVICE_ID_MEILHAUS_ME140A			0x140A
#define PCI_DEVICE_ID_MEILHAUS_ME140B			0x140B
#define PCI_DEVICE_ID_MEILHAUS_ME14E0			0x14E0
#define PCI_DEVICE_ID_MEILHAUS_ME14EA			0x14EA
#define PCI_DEVICE_ID_MEILHAUS_ME14EB			0x14EB
#define PCI_DEVICE_ID_MEILHAUS_ME140C			0X140C
#define PCI_DEVICE_ID_MEILHAUS_ME140D			0X140D

/* ME1600 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME1600_4U		0x1604 /* 4 voltage outputs */
#define PCI_DEVICE_ID_MEILHAUS_ME1600_8U		0x1608 /* 8 voltage outputs */
#define PCI_DEVICE_ID_MEILHAUS_ME1600_12U		0x160C /* 12 voltage outputs */
#define PCI_DEVICE_ID_MEILHAUS_ME1600_16U		0x160F /* 16 voltage outputs */
#define PCI_DEVICE_ID_MEILHAUS_ME1600_16U_8I	0x168F /* 16 voltage and 8 current outputs */

/* ME4500 versions - XC150 */
#define PCI_DEVICE_ID_MEILHAUS_ME4550			0x4550 /* Standard version */ /* Low Cost */
#define PCI_DEVICE_ID_MEILHAUS_ME4550I			0x4551 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4550S			0x4552 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4550IS			0x4553 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4560			0x4560 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4560I			0x4561 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4560S			0x4562 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4560IS			0x4563 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4570			0x4570 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4570I			0x4571 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4570S			0x4572 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4570IS			0x4573 /* Isolated version with Sample and Hold */

/* ME4600 versions - XC150 */
#define PCI_DEVICE_ID_MEILHAUS_ME4610			0x4610 /* Jekyll */

#define PCI_DEVICE_ID_MEILHAUS_ME4650			0x4650 /* Standard version */ /* Low Cost */
#define PCI_DEVICE_ID_MEILHAUS_ME4650I			0x4651 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4650S			0x4652 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4650IS			0x4653 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4660			0x4660 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4660I			0x4661 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4660S			0x4662 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4660IS			0x4663 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4670			0x4670 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4670I			0x4671 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4670S			0x4672 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4670IS			0x4673 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4680			0x4680 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4680I			0x4681 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4680S			0x4682 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4680IS			0x4683 /* Isolated version with Sample and Hold */

/* ME4700 versions - XC200 */
#define PCI_DEVICE_ID_MEILHAUS_ME4750			0x4750 /* Standard version */ /* Low Cost */
#define PCI_DEVICE_ID_MEILHAUS_ME4750I			0x4751 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4750S			0x4752 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4750IS			0x4753 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4760			0x4760 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4760I			0x4761 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4760S			0x4762 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4760IS			0x4763 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4770			0x4770 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4770I			0x4771 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4770S			0x4772 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4770IS			0x4773 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4780			0x4780 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4780I			0x4781 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4780S			0x4782 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4780IS			0x4783 /* Isolated version with Sample and Hold */

/* ME4800 versions - XC200 */
#define PCI_DEVICE_ID_MEILHAUS_ME4850			0x4850 /* Low Cost version */
#define PCI_DEVICE_ID_MEILHAUS_ME4850I			0x4851 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4850S			0x4852 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4850IS			0x4853 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4860			0x4860 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4860I			0x4861 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4860S			0x4862 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4860IS			0x4863 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4870			0x4870 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4870I			0x4871 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4870S			0x4872 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4870IS			0x4873 /* Isolated version with Sample and Hold */

#define PCI_DEVICE_ID_MEILHAUS_ME4880			0x4880 /* Standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME4880I			0x4881 /* Isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME4880S			0x4882 /* Standard version with Sample and Hold */
#define PCI_DEVICE_ID_MEILHAUS_ME4880IS			0x4883 /* Isolated version with Sample and Hold */

/* ME6000 standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME6004   		0x6004
#define PCI_DEVICE_ID_MEILHAUS_ME6008   		0x6008
#define PCI_DEVICE_ID_MEILHAUS_ME600F   		0x600F

/* ME6000 isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME6014   		0x6014
#define PCI_DEVICE_ID_MEILHAUS_ME6018   		0x6018
#define PCI_DEVICE_ID_MEILHAUS_ME601F   		0x601F

/* ME6000 isle version */
#define PCI_DEVICE_ID_MEILHAUS_ME6034   		0x6034
#define PCI_DEVICE_ID_MEILHAUS_ME6038   		0x6038
#define PCI_DEVICE_ID_MEILHAUS_ME603F   		0x603F

/* ME6100 standard version */
#define PCI_DEVICE_ID_MEILHAUS_ME6104   		0x6104
#define PCI_DEVICE_ID_MEILHAUS_ME6108   		0x6108
#define PCI_DEVICE_ID_MEILHAUS_ME610F   		0x610F

/* ME6100 isolated version */
#define PCI_DEVICE_ID_MEILHAUS_ME6114   		0x6114
#define PCI_DEVICE_ID_MEILHAUS_ME6118   		0x6118
#define PCI_DEVICE_ID_MEILHAUS_ME611F   		0x611F

/* ME6100 isle version */
#define PCI_DEVICE_ID_MEILHAUS_ME6134   		0x6134
#define PCI_DEVICE_ID_MEILHAUS_ME6138   		0x6138
#define PCI_DEVICE_ID_MEILHAUS_ME613F   		0x613F

/* ME6000 standard version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6044   		0x6044
#define PCI_DEVICE_ID_MEILHAUS_ME6048   		0x6048
#define PCI_DEVICE_ID_MEILHAUS_ME604F   		0x604F

/* ME6000 isolated version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6054   		0x6054
#define PCI_DEVICE_ID_MEILHAUS_ME6058   		0x6058
#define PCI_DEVICE_ID_MEILHAUS_ME605F   		0x605F

/* ME6000 isle version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6074   		0x6074
#define PCI_DEVICE_ID_MEILHAUS_ME6078   		0x6078
#define PCI_DEVICE_ID_MEILHAUS_ME607F   		0x607F

/* ME6100 standard version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6144			0x6144
#define PCI_DEVICE_ID_MEILHAUS_ME6148   		0x6148
#define PCI_DEVICE_ID_MEILHAUS_ME614F   		0x614F

/* ME6100 isolated version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6154   		0x6154
#define PCI_DEVICE_ID_MEILHAUS_ME6158   		0x6158
#define PCI_DEVICE_ID_MEILHAUS_ME615F   		0x615F

/* ME6100 isle version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6174   		0x6174
#define PCI_DEVICE_ID_MEILHAUS_ME6178   		0x6178
#define PCI_DEVICE_ID_MEILHAUS_ME617F   		0x617F

/* ME6200 isolated version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6259			0x6259

/* ME6300 isolated version with DIO */
#define PCI_DEVICE_ID_MEILHAUS_ME6359			0x6359

/* ME630 */
#define PCI_DEVICE_ID_MEILHAUS_ME630			0x0630

/* ME8100 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME8100_A			0x810A
#define PCI_DEVICE_ID_MEILHAUS_ME8100_B  		0x810B

/* ME8200 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME8200_A			0x820A
#define PCI_DEVICE_ID_MEILHAUS_ME8200_B  		0x820B

/* ME9x versions */
#define PCI_DEVICE_ID_MEILHAUS_ME94				0x0940
#define PCI_DEVICE_ID_MEILHAUS_ME95				0x0950
#define PCI_DEVICE_ID_MEILHAUS_ME96				0x0960


/* ME0700 versions */
#define PCI_DEVICE_ID_MEILHAUS_ME0752			0x0752
#define PCI_DEVICE_ID_MEILHAUS_ME0754			0x0754

#define PCI_DEVICE_ID_MEILHAUS_ME0762			0x0762
#define PCI_DEVICE_ID_MEILHAUS_ME0764			0x0764

#define PCI_DEVICE_ID_MEILHAUS_ME0772			0x0772
#define PCI_DEVICE_ID_MEILHAUS_ME0774			0x0774

#define PCI_DEVICE_ID_MEILHAUS_ME0782			0x0782
#define PCI_DEVICE_ID_MEILHAUS_ME0784			0x0784


/* ME-1000 defines */
#define ME1000_NAME_DEVICE_ME1000			"ME-1000"
#define ME1000_DESCRIPTION_DEVICE_ME1000	"ME-1000 device, 128 digital i/o lines."

/* ME-1400 defines */
#ifndef SCALE_RT
# define ME1400_NAME_DEVICE_ME1400			"ME-1400"
# define ME1400_NAME_DEVICE_ME1400E			"ME-1400E"
# define ME1400_NAME_DEVICE_ME1400A			"ME-1400A"
# define ME1400_NAME_DEVICE_ME1400EA		"ME-1400EA"
# define ME1400_NAME_DEVICE_ME1400B			"ME-1400B"
# define ME1400_NAME_DEVICE_ME1400EB		"ME-1400EB"
# define ME1400_NAME_DEVICE_ME1400C			"ME-1400C"
# define ME1400_NAME_DEVICE_ME1400D			"ME-1400D"
#else	//Scale-RT
# define ME1400_NAME_DEVICE_ME1400			"ME-1400"
# define ME1400_NAME_DEVICE_ME1400E			"ME-1400e"
# define ME1400_NAME_DEVICE_ME1400A			"ME-1400a"
# define ME1400_NAME_DEVICE_ME1400EA		"ME-1400ea"
# define ME1400_NAME_DEVICE_ME1400B			"ME-1400b"
# define ME1400_NAME_DEVICE_ME1400EB		"ME-1400eb"
# define ME1400_NAME_DEVICE_ME1400C			"ME-1400c"
# define ME1400_NAME_DEVICE_ME1400D			"ME-1400d"
#endif

#define ME1400_DESCRIPTION_DEVICE_ME1400		"ME-1400 device, 24 digital i/o lines."
#define ME1400_DESCRIPTION_DEVICE_ME1400E		"ME-1400E device, 24 digital i/o lines."
#define ME1400_DESCRIPTION_DEVICE_ME1400A		"ME-1400A device, 24 digital i/o lines, 3 counters, 1 external interrupt."
#define ME1400_DESCRIPTION_DEVICE_ME1400EA		"ME-1400EA device, 24 digital i/o lines, 3 counters, 1 external interrupt."
#define ME1400_DESCRIPTION_DEVICE_ME1400B		"ME-1400B device, 48 digital i/o lines, 6 counters, 1 external interrupt."
#define ME1400_DESCRIPTION_DEVICE_ME1400EB		"ME-1400EB device, 48 digital i/o lines, 6 counters, 1 external interrupt."
#define ME1400_DESCRIPTION_DEVICE_ME1400C		"ME-1400C device, 24 digital i/o lines, 15 counters, 1 external interrupt."
#define ME1400_DESCRIPTION_DEVICE_ME1400D		"ME-1400D device, 48 digital i/o lines, 30 counters, 2 external interrupt."

/* ME-1600 defines */
#ifndef SCALE_RT
# define ME1600_NAME_DEVICE_ME16004U			"ME-1600/4U"
# define ME1600_NAME_DEVICE_ME16008U			"ME-1600/8U"
# define ME1600_NAME_DEVICE_ME160012U			"ME-1600/12U"
# define ME1600_NAME_DEVICE_ME160016U			"ME-1600/16U"
# define ME1600_NAME_DEVICE_ME160016U8I			"ME-1600/16U8I"
#else	//Scale-RT
# define ME1600_NAME_DEVICE_ME16004U			"ME-1600/4u"
# define ME1600_NAME_DEVICE_ME16008U			"ME-1600/8u"
# define ME1600_NAME_DEVICE_ME160012U			"ME-1600/12u"
# define ME1600_NAME_DEVICE_ME160016U			"ME-1600/16u"
# define ME1600_NAME_DEVICE_ME160016U8I			"ME-1600/16u8i"
#endif

#define ME1600_DESCRIPTION_DEVICE_ME16004U		"ME-1600/4U device, 4 voltage outputs."
#define ME1600_DESCRIPTION_DEVICE_ME16008U		"ME-1600/8U device, 8 voltage outputs."
#define ME1600_DESCRIPTION_DEVICE_ME160012U		"ME-1600/12U device, 12 voltage outputs."
#define ME1600_DESCRIPTION_DEVICE_ME160016U		"ME-1600/16U device, 16 voltage outputs."
#define ME1600_DESCRIPTION_DEVICE_ME160016U8I	"ME-1600/16U8I device, 16 voltage, 8 current outputs."

/* ME-4500 defines */
#ifndef SCALE_RT
# define ME4000_NAME_DEVICE_ME4550			"ME-4650F"
# define ME4000_NAME_DEVICE_ME4550I			"ME-4650IF"
# define ME4000_NAME_DEVICE_ME4550S			"ME-4650SF"
# define ME4000_NAME_DEVICE_ME4550IS		"ME-4650ISF"
# define ME4000_NAME_DEVICE_ME4560			"ME-4660F"
# define ME4000_NAME_DEVICE_ME4560I			"ME-4660IF"
# define ME4000_NAME_DEVICE_ME4560S			"ME-4660SF"
# define ME4000_NAME_DEVICE_ME4560IS		"ME-4660ISF"
# define ME4000_NAME_DEVICE_ME4570			"ME-4670F"
# define ME4000_NAME_DEVICE_ME4570I			"ME-4670IF"
# define ME4000_NAME_DEVICE_ME4570S			"ME-4670SF"
# define ME4000_NAME_DEVICE_ME4570IS		"ME-4670ISF"
#else	//Scale-RT
# define ME4000_NAME_DEVICE_ME4550			"ME-4550"
# define ME4000_NAME_DEVICE_ME4550I			"ME-4550i"
# define ME4000_NAME_DEVICE_ME4550S			"ME-4550s"
# define ME4000_NAME_DEVICE_ME4550IS		"ME-4550is"
# define ME4000_NAME_DEVICE_ME4560			"ME-4560"
# define ME4000_NAME_DEVICE_ME4560I			"ME-4560i"
# define ME4000_NAME_DEVICE_ME4560S			"ME-4560s"
# define ME4000_NAME_DEVICE_ME4560IS		"ME-4560is"
# define ME4000_NAME_DEVICE_ME4570			"ME-4570"
# define ME4000_NAME_DEVICE_ME4570I			"ME-4570i"
# define ME4000_NAME_DEVICE_ME4570S			"ME-4570s"
# define ME4000_NAME_DEVICE_ME4570IS		"ME-4570is"
#endif

/* ME-4600 defines */
#ifndef SCALE_RT
# define ME4000_NAME_DEVICE_ME4610			"ME-4610"
# define ME4000_NAME_DEVICE_ME4650			"ME-4650"
# define ME4000_NAME_DEVICE_ME4650I			"ME-4650I"
# define ME4000_NAME_DEVICE_ME4650S			"ME-4650S"
# define ME4000_NAME_DEVICE_ME4650IS		"ME-4650IS"
# define ME4000_NAME_DEVICE_ME4660			"ME-4660"
# define ME4000_NAME_DEVICE_ME4660I			"ME-4660I"
# define ME4000_NAME_DEVICE_ME4660S			"ME-4660S"
# define ME4000_NAME_DEVICE_ME4660IS		"ME-4660IS"
# define ME4000_NAME_DEVICE_ME4670			"ME-4670"
# define ME4000_NAME_DEVICE_ME4670I			"ME-4670I"
# define ME4000_NAME_DEVICE_ME4670S			"ME-4670S"
# define ME4000_NAME_DEVICE_ME4670IS		"ME-4670IS"
# define ME4000_NAME_DEVICE_ME4680			"ME-4680"
# define ME4000_NAME_DEVICE_ME4680I			"ME-4680I"
# define ME4000_NAME_DEVICE_ME4680S			"ME-4680S"
# define ME4000_NAME_DEVICE_ME4680IS		"ME-4680IS"
#else	//Scale-RT
# define ME4000_NAME_DEVICE_ME4610			"ME-4610"
# define ME4000_NAME_DEVICE_ME4650			"ME-4650"
# define ME4000_NAME_DEVICE_ME4650I			"ME-4650i"
# define ME4000_NAME_DEVICE_ME4650S			"ME-4650s"
# define ME4000_NAME_DEVICE_ME4650IS		"ME-4650is"
# define ME4000_NAME_DEVICE_ME4660			"ME-4660"
# define ME4000_NAME_DEVICE_ME4660I			"ME-4660i"
# define ME4000_NAME_DEVICE_ME4660S			"ME-4660s"
# define ME4000_NAME_DEVICE_ME4660IS		"ME-4660is"
# define ME4000_NAME_DEVICE_ME4670			"ME-4670"
# define ME4000_NAME_DEVICE_ME4670I			"ME-4670i"
# define ME4000_NAME_DEVICE_ME4670S			"ME-4670s"
# define ME4000_NAME_DEVICE_ME4670IS		"ME-4670is"
# define ME4000_NAME_DEVICE_ME4680			"ME-4680"
# define ME4000_NAME_DEVICE_ME4680I			"ME-4680i"
# define ME4000_NAME_DEVICE_ME4680S			"ME-4680s"
# define ME4000_NAME_DEVICE_ME4680IS		"ME-4680is"
#endif

/* ME-4700 defines */
#ifndef SCALE_RT
# define ME4000_NAME_DEVICE_ME4750			"ME-4650F"
# define ME4000_NAME_DEVICE_ME4750I			"ME-4650IF"
# define ME4000_NAME_DEVICE_ME4750S			"ME-4650SF"
# define ME4000_NAME_DEVICE_ME4750IS		"ME-4650ISF"
# define ME4000_NAME_DEVICE_ME4760			"ME-4660F"
# define ME4000_NAME_DEVICE_ME4760I			"ME-4660IF"
# define ME4000_NAME_DEVICE_ME4760S			"ME-4660SF"
# define ME4000_NAME_DEVICE_ME4760IS		"ME-4660ISF"
# define ME4000_NAME_DEVICE_ME4770			"ME-4670F"
# define ME4000_NAME_DEVICE_ME4770I			"ME-4670IF"
# define ME4000_NAME_DEVICE_ME4770S			"ME-4670SF"
# define ME4000_NAME_DEVICE_ME4770IS		"ME-4670ISF"
# define ME4000_NAME_DEVICE_ME4780			"ME-4680F"
# define ME4000_NAME_DEVICE_ME4780I			"ME-4680IF"
# define ME4000_NAME_DEVICE_ME4780S			"ME-4680SF"
# define ME4000_NAME_DEVICE_ME4780IS		"ME-4680ISF"
#else	//Scale-RT
# define ME4000_NAME_DEVICE_ME4750			"ME-4750"
# define ME4000_NAME_DEVICE_ME4750I			"ME-4750i"
# define ME4000_NAME_DEVICE_ME4750S			"ME-4750s"
# define ME4000_NAME_DEVICE_ME4750IS		"ME-4750is"
# define ME4000_NAME_DEVICE_ME4760			"ME-4760"
# define ME4000_NAME_DEVICE_ME4760I			"ME-4760i"
# define ME4000_NAME_DEVICE_ME4760S			"ME-4760s"
# define ME4000_NAME_DEVICE_ME4760IS		"ME-4760is"
# define ME4000_NAME_DEVICE_ME4770			"ME-4770"
# define ME4000_NAME_DEVICE_ME4770I			"ME-4770i"
# define ME4000_NAME_DEVICE_ME4770S			"ME-4770s"
# define ME4000_NAME_DEVICE_ME4770IS		"ME-4770is"
# define ME4000_NAME_DEVICE_ME4780			"ME-4780"
# define ME4000_NAME_DEVICE_ME4780I			"ME-4780i"
# define ME4000_NAME_DEVICE_ME4780S			"ME-4780s"
# define ME4000_NAME_DEVICE_ME4780IS		"ME-4780is"
#endif

/* ME-4800 defines */
#ifndef SCALE_RT
# define ME4000_NAME_DEVICE_ME4850			"ME-4850"
# define ME4000_NAME_DEVICE_ME4850I			"ME-4850I"
# define ME4000_NAME_DEVICE_ME4850S			"ME-4850S"
# define ME4000_NAME_DEVICE_ME4850IS		"ME-4850IS"
# define ME4000_NAME_DEVICE_ME4860			"ME-4860"
# define ME4000_NAME_DEVICE_ME4860I			"ME-4860I"
# define ME4000_NAME_DEVICE_ME4860S			"ME-4860S"
# define ME4000_NAME_DEVICE_ME4860IS		"ME-4860IS"
# define ME4000_NAME_DEVICE_ME4870			"ME-4870"
# define ME4000_NAME_DEVICE_ME4870I			"ME-4870I"
# define ME4000_NAME_DEVICE_ME4870S			"ME-4870S"
# define ME4000_NAME_DEVICE_ME4870IS		"ME-4870IS"
# define ME4000_NAME_DEVICE_ME4880			"ME-4880"
# define ME4000_NAME_DEVICE_ME4880I			"ME-4880I"
# define ME4000_NAME_DEVICE_ME4880S			"ME-4880S"
# define ME4000_NAME_DEVICE_ME4880IS		"ME-4880IS"
#else	//Scale-RT
# define ME4000_NAME_DEVICE_ME4850			"ME-4850"
# define ME4000_NAME_DEVICE_ME4850I			"ME-4850i"
# define ME4000_NAME_DEVICE_ME4850S			"ME-4850s"
# define ME4000_NAME_DEVICE_ME4850IS		"ME-4850is"
# define ME4000_NAME_DEVICE_ME4860			"ME-4860"
# define ME4000_NAME_DEVICE_ME4860I			"ME-4860i"
# define ME4000_NAME_DEVICE_ME4860S			"ME-4860s"
# define ME4000_NAME_DEVICE_ME4860IS		"ME-4860is"
# define ME4000_NAME_DEVICE_ME4870			"ME-4870"
# define ME4000_NAME_DEVICE_ME4870I			"ME-4870i"
# define ME4000_NAME_DEVICE_ME4870S			"ME-4870s"
# define ME4000_NAME_DEVICE_ME4870IS		"ME-4870is"
# define ME4000_NAME_DEVICE_ME4880			"ME-4880"
# define ME4000_NAME_DEVICE_ME4880I			"ME-4880i"
# define ME4000_NAME_DEVICE_ME4880S			"ME-4880s"
# define ME4000_NAME_DEVICE_ME4880IS		"ME-4880is"
#endif

#define ME4000_DESCRIPTION_DEVICE_ME4550	"ME-4650F device, 16 streaming analog inputs, 16 digital i/o lines, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4550I	"ME-4650IF opto isolated device, 16 streaming analog inputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4550S	"ME-4650F device, 16 streaming analog inputs (8 S&H), 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4550IS	"ME-4650IF opto isolated device, 16 streaming analog inputs (8 S&H), 16 digital i/o lines, 3 counters, 2 external interrupt, 1 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4560	"ME-4660F device, 16 streaming analog inputs, 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4560I	"ME-4660IF opto isolated device, 16 streaming analog inputs, 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4560S	"ME-4660F device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4560IS	"ME-4660IF opto isolated device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 16 digital i/o lines, 3 counters, 2 external interrupt, 1 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4570	"ME-4670F device, 32 streaming analog inputs, 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4570I	"ME-4670IF opto isolated device, 32 streaming analog inputs, 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4570S	"ME-4670SF device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."
#define ME4000_DESCRIPTION_DEVICE_ME4570IS	"ME-4670ISF opto isolated device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 2 frequency input, 2 frequency output."

#define ME4000_DESCRIPTION_DEVICE_ME4610	"ME-4610 device, 16 streaming analog inputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4650	"ME-4650 device, 16 streaming analog inputs, 32 digital i/o lines, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4650I	"ME-4650I opto isolated device, 16 streaming analog inputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4650S	"ME-4650 device, 16 streaming analog inputs (8 S&H), 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4650IS	"ME-4650I opto isolated device, 16 streaming analog inputs (8 S&H), 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4660	"ME-4660 device, 16 streaming analog inputs, 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4660I	"ME-4660I opto isolated device, 16 streaming analog inputs, 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4660S	"ME-4660 device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4660IS	"ME-4660I opto isolated device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4670	"ME-4670 device, 32 streaming analog inputs, 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4670I	"ME-4670I opto isolated device, 32 streaming analog inputs, 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4670S	"ME-4670S device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4670IS	"ME-4670IS opto isolated device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4680	"ME-4680 device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4680I	"ME-4680I opto isolated device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4680S	"ME-4680S device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4680IS	"ME-4680IS opto isolated device, 32 streaming analog inputs (8 S&H), 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."

#define ME4000_DESCRIPTION_DEVICE_ME4750	"ME-4650F device, 16 streaming analog inputs, 16 digital i/o lines, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4750I	"ME-4650IF opto isolated device, 16 streaming analog inputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4750S	"ME-4650F device, 16 streaming analog inputs (8 S&H), 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4750IS	"ME-4650IF opto isolated device, 16 streaming analog inputs (8 S&H), 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4760	"ME-4660F device, 16 streaming analog inputs, 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4760I	"ME-4660IF opto isolated device, 16 streaming analog inputs, 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4760S	"ME-4660F device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4760IS	"ME-4660IF opto isolated device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4770	"ME-4670F device, 32 streaming analog inputs, 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4770I	"ME-4670IF opto isolated device, 32 streaming analog inputs, 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4770S	"ME-4670SF device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4770IS	"ME-4670ISF opto isolated device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4780	"ME-4680F device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4780I	"ME-4680IF opto isolated device, 32 streaming analog inputs, 4 streaming analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4780S	"ME-4680SF device, 32 streaming analog inputs, 4 streaming analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."
#define ME4000_DESCRIPTION_DEVICE_ME4780IS	"ME-4680ISF opto isolated device, 32 streaming analog inputs (8 S&H), 4 streaming analog outputs, 16 digital i/o lines, 3 counters, 1 external interrupt, 4 frequency inputs, 4 frequency outputs."

#define ME4000_DESCRIPTION_DEVICE_ME4850	"ME-4650 device, 16 streaming analog inputs, 32 digital i/o lines, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4850I	"ME-4650I opto isolated device, 16 streaming analog inputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4850S	"ME-4650 device, 16 streaming analog inputs (8 S&H), 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4850IS	"ME-4650I opto isolated device, 16 streaming analog inputs (8 S&H), 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4860	"ME-4660 device, 16 streaming analog inputs, 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4860I	"ME-4660I opto isolated device, 16 streaming analog inputs, 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4860S	"ME-4660 device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4860IS	"ME-4660I opto isolated device, 16 streaming analog inputs (8 S&H), 2 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4870	"ME-4670 device, 32 streaming analog inputs, 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4870I	"ME-4670I opto isolated device, 32 streaming analog inputs, 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4870S	"ME-4670S device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4870IS	"ME-4670IS opto isolated device, 32 streaming analog inputs (8 S&H), 4 analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4880	"ME-4680 device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4880I	"ME-4680I opto isolated device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4880S	"ME-4680S device, 32 streaming analog inputs, 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."
#define ME4000_DESCRIPTION_DEVICE_ME4880IS	"ME-4680IS opto isolated device, 32 streaming analog inputs (8 S&H), 4 streaming analog outputs, 32 digital i/o lines, 3 counters, 1 external interrupt."

/* ME-6x00 defines */
#ifndef SCALE_RT
# define ME6000_NAME_DEVICE_ME60004			"ME-6000/4"
# define ME6000_NAME_DEVICE_ME60008			"ME-6000/8"
# define ME6000_NAME_DEVICE_ME600016		"ME-6000/16"
# define ME6000_NAME_DEVICE_ME6000I4		"ME-6000I/4"
# define ME6000_NAME_DEVICE_ME6000I8		"ME-6000I/8"
# define ME6000_NAME_DEVICE_ME6000I16		"ME-6000I/16"
# define ME6000_NAME_DEVICE_ME6000ISLE4		"ME-6000ISLE/4"
# define ME6000_NAME_DEVICE_ME6000ISLE8		"ME-6000ISLE/8"
# define ME6000_NAME_DEVICE_ME6000ISLE16	"ME-6000ISLE/16"
# define ME6000_NAME_DEVICE_ME61004			"ME-6100/4"
# define ME6000_NAME_DEVICE_ME61008			"ME-6100/8"
# define ME6000_NAME_DEVICE_ME610016		"ME-6100/16"
# define ME6000_NAME_DEVICE_ME6100I4		"ME-6100I/4"
# define ME6000_NAME_DEVICE_ME6100I8		"ME-6100I/8"
# define ME6000_NAME_DEVICE_ME6100I16		"ME-6100I/16"
# define ME6000_NAME_DEVICE_ME6100ISLE4		"ME-6100ISLE/4"
# define ME6000_NAME_DEVICE_ME6100ISLE8		"ME-6100ISLE/8"
# define ME6000_NAME_DEVICE_ME6100ISLE16	"ME-6100ISLE/16"
# define ME6000_NAME_DEVICE_ME60004DIO		"ME-6000/4/DIO"
# define ME6000_NAME_DEVICE_ME60008DIO		"ME-6000/8/DIO"
# define ME6000_NAME_DEVICE_ME600016DIO		"ME-6000/16/DIO"
# define ME6000_NAME_DEVICE_ME6000I4DIO		"ME-6000I/4/DIO"
# define ME6000_NAME_DEVICE_ME6000I8DIO		"ME-6000I/8/DIO"
# define ME6000_NAME_DEVICE_ME6000I16DIO	"ME-6000I/16/DIO"
# define ME6000_NAME_DEVICE_ME6000ISLE4DIO	"ME-6000ISLE/4/DIO"
# define ME6000_NAME_DEVICE_ME6000ISLE8DIO	"ME-6000ISLE/8/DIO"
# define ME6000_NAME_DEVICE_ME6000ISLE16DIO	"ME-6000ISLE/16/DIO"
# define ME6000_NAME_DEVICE_ME61004DIO		"ME-6100/4/DIO"
# define ME6000_NAME_DEVICE_ME61008DIO		"ME-6100/8/DIO"
# define ME6000_NAME_DEVICE_ME610016DIO		"ME-6100/16/DIO"
# define ME6000_NAME_DEVICE_ME6100I4DIO		"ME-6100I/4/DIO"
# define ME6000_NAME_DEVICE_ME6100I8DIO		"ME-6100I/8/DIO"
# define ME6000_NAME_DEVICE_ME6100I16DIO	"ME-6100I/16/DIO"
# define ME6000_NAME_DEVICE_ME6100ISLE4DIO	"ME-6100ISLE/4/DIO"
# define ME6000_NAME_DEVICE_ME6100ISLE8DIO	"ME-6100ISLE/8/DIO"
# define ME6000_NAME_DEVICE_ME6100ISLE16DIO	"ME-6100ISLE/16/DIO"
# define ME6000_NAME_DEVICE_ME6200I9DIO		"ME-6200I/9/DIO"
# define ME6000_NAME_DEVICE_ME6300I9DIO		"ME-6300I/9/DIO"
#else	//Scale-RT
# define ME6000_NAME_DEVICE_ME60004			"ME-6000/4"
# define ME6000_NAME_DEVICE_ME60008			"ME-6000/8"
# define ME6000_NAME_DEVICE_ME600016		"ME-6000/16"
# define ME6000_NAME_DEVICE_ME6000I4		"ME-6000i/4"
# define ME6000_NAME_DEVICE_ME6000I8		"ME-6000i/8"
# define ME6000_NAME_DEVICE_ME6000I16		"ME-6000i/16"
# define ME6000_NAME_DEVICE_ME6000ISLE4		"ME-6000isle/4"
# define ME6000_NAME_DEVICE_ME6000ISLE8		"ME-6000isle/8"
# define ME6000_NAME_DEVICE_ME6000ISLE16	"ME-6000isle/16"
# define ME6000_NAME_DEVICE_ME61004			"ME-6100/4"
# define ME6000_NAME_DEVICE_ME61008			"ME-6100/8"
# define ME6000_NAME_DEVICE_ME610016		"ME-6100/16"
# define ME6000_NAME_DEVICE_ME6100I4		"ME-6100i/4"
# define ME6000_NAME_DEVICE_ME6100I8		"ME-6100i/8"
# define ME6000_NAME_DEVICE_ME6100I16		"ME-6100i/16"
# define ME6000_NAME_DEVICE_ME6100ISLE4		"ME-6100isle/4"
# define ME6000_NAME_DEVICE_ME6100ISLE8		"ME-6100isle/8"
# define ME6000_NAME_DEVICE_ME6100ISLE16	"ME-6100isle/16"
# define ME6000_NAME_DEVICE_ME60004DIO		"ME-6000/4/dio"
# define ME6000_NAME_DEVICE_ME60008DIO		"ME-6000/8/dio"
# define ME6000_NAME_DEVICE_ME600016DIO		"ME-6000/16/dio"
# define ME6000_NAME_DEVICE_ME6000I4DIO		"ME-6000i/4/dio"
# define ME6000_NAME_DEVICE_ME6000I8DIO		"ME-6000i/8/dio"
# define ME6000_NAME_DEVICE_ME6000I16DIO	"ME-6000i/16/dio"
# define ME6000_NAME_DEVICE_ME6000ISLE4DIO	"ME-6000isle/4/dio"
# define ME6000_NAME_DEVICE_ME6000ISLE8DIO	"ME-6000isle/8/dio"
# define ME6000_NAME_DEVICE_ME6000ISLE16DIO	"ME-6000isle/16/dio"
# define ME6000_NAME_DEVICE_ME61004DIO		"ME-6100/4/dio"
# define ME6000_NAME_DEVICE_ME61008DIO		"ME-6100/8/dio"
# define ME6000_NAME_DEVICE_ME610016DIO		"ME-6100/16/dio"
# define ME6000_NAME_DEVICE_ME6100I4DIO		"ME-6100i/4/dio"
# define ME6000_NAME_DEVICE_ME6100I8DIO		"ME-6100i/8/dio"
# define ME6000_NAME_DEVICE_ME6100I16DIO	"ME-6100i/16/dio"
# define ME6000_NAME_DEVICE_ME6100ISLE4DIO	"ME-6100isle/4/dio"
# define ME6000_NAME_DEVICE_ME6100ISLE8DIO	"ME-6100isle/8/dio"
# define ME6000_NAME_DEVICE_ME6100ISLE16DIO	"ME-6100isle/16/dio"
# define ME6000_NAME_DEVICE_ME6200I9DIO		"ME-6200i/9/dio"
# define ME6000_NAME_DEVICE_ME6300I9DIO		"ME-6300i/9/dio"
#endif

#define ME6000_DESCRIPTION_DEVICE_ME60004			"ME-6000/4 device, 4 analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME60008			"ME-6000/8 device, 8 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME600016			"ME-6000/16 device, 16 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000I4			"ME-6000I/4 isolated device, 4 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000I8			"ME-6000I/8 isolated device, 8 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000I16			"ME-6000I/16 isolated device, 16 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE4		"ME-6000ISLE/4 isle device, 4 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE8		"ME-6000ISLE/8 isle device, 8 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE16		"ME-6000ISLE/16 isle device, 16 analog outputs"
#define ME6000_DESCRIPTION_DEVICE_ME61004			"ME-6100/4 device, 4 streaming analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME61008			"ME-6100/8 device, 4 streaming, 4 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME610016			"ME-6100/16 device, 4 streaming, 12 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100I4			"ME-6100I/4 isolated device, 4 streaming analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100I8			"ME-6100I/8 isolated device, 4 streaming, 4 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100I16			"ME-6100I/16 isolated device, 4 streaming, 12 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE4		"ME-6100ISLE/4 isle device, 4 streaming analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE8		"ME-6100ISLE/8 isle device, 4 streaming, 4 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE16		"ME-6100ISLE/16 isle device, 4 streaming, 12 single analog outputs."
#define ME6000_DESCRIPTION_DEVICE_ME60004DIO		"ME-6000/4/DIO device, 4 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME60008DIO		"ME-6000/8/DIO device, 8 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME600016DIO		"ME-6000/16/DIO device, 8 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000I4DIO		"ME-6000I/4/DIO isolated device, 4 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000I8DIO		"ME-6000I/8/DIO isolated device, 8 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000I16DIO		"ME-6000I/16/DIO isolated device, 16 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE4DIO	"ME-6000ISLE/4/DIO isle device, 4 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE8DIO	"ME-6000ISLE/8/DIO isle device, 8 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6000ISLE16DIO	"ME-6000ISLE/16/DIO isle device, 16 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME61004DIO		"ME-6100/4/DIO device, 4 streaming analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME61008DIO		"ME-6100/8/DIO device, 4 streaming, 4 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME610016DIO		"ME-6100/16/DIO device, 4 streaming, 12 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100I4DIO		"ME-6100I/4/DIO isolated device, 4 streaming analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100I8DIO		"ME-6100I/8/DIO isolated device, 4 streaming, 4 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100I16DIO		"ME-6100I/16/DIO isolated device, 4 streaming, 12 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE4DIO	"ME-6100ISLE/4/DIO isle device, 4 streaming analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE8DIO	"ME-6100ISLE/8/DIO isle device, 4 streaming, 4 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6100ISLE16DIO	"ME-6100ISLE/16/DIO isle device, 4 streaming, 12 single analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6200I9DIO		"ME-6200I/9/DIO isolated device, 9 analog outputs, 16 digital i/o lines."
#define ME6000_DESCRIPTION_DEVICE_ME6300I9DIO		"ME-6300I/9/DIO isolated device, 4 streaming, 5 single analog outputs, 16 digital i/o lines."

/* ME-630 defines */
#define ME630_NAME_DEVICE_ME630				"ME-630"

#define ME630_DESCRIPTION_DEVICE_ME630		"ME-630 device, up to 16 relay, 8 digital ttl input lines, 8 isolated digital input lines, 16 digital i/o lines, 2 external interrupts."

/* ME-8100 defines */
#ifndef SCALE_RT
# define ME8100_NAME_DEVICE_ME8100A			"ME-8100A"
# define ME8100_NAME_DEVICE_ME8100B			"ME-8100B"
#else	//Scale-RT
# define ME8100_NAME_DEVICE_ME8100A			"ME-8100a"
# define ME8100_NAME_DEVICE_ME8100B			"ME-8100b"
#endif

#define ME8100_DESCRIPTION_DEVICE_ME8100A	"ME-8100A opto isolated device, 16 digital input lines, 16 digital output lines."
#define ME8100_DESCRIPTION_DEVICE_ME8100B	"ME-8100B opto isolated device, 32 digital input lines, 32 digital output lines, 3 counters."

/* ME-8200 defines */
#ifndef SCALE_RT
# define ME8200_NAME_DEVICE_ME8200A			"ME-8200A"
# define ME8200_NAME_DEVICE_ME8200B			"ME-8200B"
#else	//Scale-RT
# define ME8200_NAME_DEVICE_ME8200A			"ME-8200a"
# define ME8200_NAME_DEVICE_ME8200B			"ME-8200b"
#endif

#define ME8200_DESCRIPTION_DEVICE_ME8200A	"ME-8200A opto isolated device, 8 digital output lines, 8 digital input lines, 16 digital i/o lines."
#define ME8200_DESCRIPTION_DEVICE_ME8200B	"ME-8200B opto isolated device, 16 digital output lines, 16 digital input lines, 16 digital i/o lines."

/* ME-9x defines */
#define ME9x_NAME_DEVICE_ME94				"ME-94"
#define ME9x_NAME_DEVICE_ME95				"ME-95"
#define ME9x_NAME_DEVICE_ME96				"ME-96"

#define ME9x_DESCRIPTION_DEVICE_ME94		"ME-94 device, 16 digital input lines, 2 external interrupt lines."
#define ME9x_DESCRIPTION_DEVICE_ME95		"ME-95 device, 16 digital output lines."
#define ME9x_DESCRIPTION_DEVICE_ME96		"ME-96 device, 8 digital input lines, 8 digital output lines, 2 external interrupt lines."

/* ME-0700 defines */
#define ME0700_NAME_DEVICE_ME0752			"ME-0750/2"
#define ME0700_NAME_DEVICE_ME0754			"ME-0750/4"
#define ME0700_NAME_DEVICE_ME0762			"ME-0760/2"
#define ME0700_NAME_DEVICE_ME0764			"ME-0760/4"
#define ME0700_NAME_DEVICE_ME0772			"ME-0770/2"
#define ME0700_NAME_DEVICE_ME0774			"ME-0770/4"
#define ME0700_NAME_DEVICE_ME0782			"ME-0780/2"
#define ME0700_NAME_DEVICE_ME0784			"ME-0780/4"

#define ME0700_DESCRIPTION_DEVICE_ME0752	"ME-0750 device, 2 current analog inputs, 2+8 anlog inputs, 16 digital i/o lines."
#define ME0700_DESCRIPTION_DEVICE_ME0754	"ME-0750 device, 4 current analog inputs, 4+8 anlog inputs, 16 digital i/o lines."
#define ME0700_DESCRIPTION_DEVICE_ME0762	"ME-0760 device, 2 current analog inputs, 2+8+16 anlog inputs, 16 digital i/o lines, 3 counters."
#define ME0700_DESCRIPTION_DEVICE_ME0764	"ME-0760 device, 4 current analog inputs, 4+8+16 anlog inputs, 16 digital i/o lines, 3 counters."
#define ME0700_DESCRIPTION_DEVICE_ME0772	"ME-0770 device, 2 current analog inputs, 2+8+16 anlog inputs, 4 single analog outputs, 16 digital i/o lines, 3 counters."
#define ME0700_DESCRIPTION_DEVICE_ME0774	"ME-0770 device, 4 current analog inputs, 4+8+16 anlog inputs, 4 single analog outputs, 16 digital i/o lines, 3 counters."
#define ME0700_DESCRIPTION_DEVICE_ME0782	"ME-0780 device, 2 current analog inputs, 2+8+16 anlog inputs, 4 streaming analog outputs, 16 digital i/o lines, 3 counters."
#define ME0700_DESCRIPTION_DEVICE_ME0784	"ME-0780 device, 4 current analog inputs, 4+8+16 anlog inputs, 4 streaming analog outputs, 16 digital i/o lines, 3 counters."

#define MEPHISTO_DESCRIPTION				"MephistoScope USB device, 24 digital i/o lines, 2 analog inputs."

/* ME-FF00 (FPGA board) defines */
#define PCI_DEVICE_ID_MEILHAUS_MEFF00		0xFF00
#define MEFF00_NAME_DEVICE_MEFF00			"ME-FPGA"
#define MEFF00_DESCRIPTION_DEVICE_MEFF00	"ME-FPGA, FPGA board."


#endif
