/**
 * @file me_common.h
 *
 * @brief Stuff for USB and PCI drivers.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

#ifndef _ME_COMMON_H_
# define _ME_COMMON_H_

#  ifndef ME_VERSION_DRIVER
/* Unknown version */
#   define ME_VERSION_DRIVER			0xFFFFFFFF
#  endif

# define ME_MAX_UNHANDLED_IRQ			0xFF
#endif	//_ME_COMMON_H_
