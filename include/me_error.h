/**
 * @file me_error.h
 *
 * @brief Additional errors' definitions. For internal use only.
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _ME_ERROR_H_
# define _ME_ERROR_H_
# include "../common/meerror.h"

# define MEDRV_INTERNAL_ERROR_BASE 			0x1000
/// MUTEX
# define ME_MUTEX_WRONG_PID 				(MEDRV_INTERNAL_ERROR_BASE + 0x001)
# define ME_MUTEX_WRONG_STATUS 				(MEDRV_INTERNAL_ERROR_BASE + 0x002)
# define ME_MUTEX_DEATHLOCK 				(MEDRV_INTERNAL_ERROR_BASE + 0x003)
# define ME_MUTEX_FORCE 					(MEDRV_INTERNAL_ERROR_BASE + 0x004)

/// Firmware
// Firmware in EEPROM - Can not be overwrited.
# define ME_FIRMWARE_DOWNLOAD_DISABLE		(MEDRV_INTERNAL_ERROR_BASE + 0x100)
// Getting firmware failed
# define ME_FIRMWARE_ERROR 					(MEDRV_INTERNAL_ERROR_BASE + 0x101)
// Initialization was unsuccesful
# define ME_FIRMWARE_INIT_ERROR 			(MEDRV_INTERNAL_ERROR_BASE + 0x102)
// Writing was unsuccesful
# define ME_FIRMWARE_DONE_ERROR 			(MEDRV_INTERNAL_ERROR_BASE + 0x103)

# define MEiDS_INTERNAL_ERROR_BASE 			(MEDRV_INTERNAL_ERROR_BASE + 0x1000)
/// XML
# define MEiDS_XML_ERROR_BASE       		(MEiDS_INTERNAL_ERROR_BASE + 0x1000)
# define MEiDS_ERRNO_XML_ENTRY_NOT_FOUND  	(MEiDS_XML_ERROR_BASE + 1)
#endif
