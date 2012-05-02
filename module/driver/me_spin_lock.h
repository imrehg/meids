/**
 * @file me_spin_lock.h
 *
 * @brief Defins of locks. (PCI and USB have different needs.)
 * @note Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 */

/*
 * Copyright (C) 2008 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef __KERNEL__

# ifndef _ME_SPIN_LOCK_H_
#  define _ME_SPIN_LOCK_H_

#  include "melock_defines.h"
#  include "mesubdevice.h"
#  include "medevice.h"
#  include "me_error.h"

int me_lock_context(subdevice_protector_t* protector);
int me_unlock_context(subdevice_protector_t* protector);

int me_lock_protector(subdevice_protector_t* protector, me_general_dev_t* dev);
int me_unlock_protector(subdevice_protector_t* protector, me_general_dev_t* dev);

int me_handler_protector(subdevice_protector_t* protector);
int me_free_handler_protector(subdevice_protector_t* protector);


/** Use in dlock and slock */
#  define ME_LOCK(lock) \
	if(1) \
	{ \
		PDEBUG_LOCKS("LOCK %s\n", #lock); \
		ME_SET_LOCK(lock); \
	}

#  define ME_UNLOCK(lock) \
	if(1) \
	{ \
		PDEBUG_LOCKS("UNLOCK %s\n", #lock); \
		ME_RELESE_LOCK(lock); \
	}
/*
#  define ME_TRYLOCK(lock) \
	if(1) \
	{ \
		PDEBUG_LOCKS("TRYLOCK %s\n", #lock); \
		ME_TRY_TO_SET_LOCK(lock); \
	}
*/
/** Use only as entry point in interrupt handelers. */
#  define ME_HANDLER_PROTECTOR \
	PDEBUG_MUTEX("ME_HANDLER_PROTECTOR\n"); \
	if (me_handler_protector(&instance->base.subdevice_lock)) \
	{ \
			PDEBUG_MUTEX("HANDLER LOCK IN USE!\n"); \
			return ME_ERRNO_SUCCESS; \
	} \
	else \
	{ \
			PDEBUG_MUTEX("HANDLER LOCK SETTED\n"); \
	}

/** Use only as exit point in interrupt handelers. */
#  define ME_FREE_HANDLER_PROTECTOR \
	PDEBUG_MUTEX("ME_FREE_PROTECTOR\n"); \
	if (me_free_handler_protector(&instance->base.subdevice_lock)) \
	{ \
		PDEBUG_MUTEX("PROTECTOR UNLOCK ERROR!\n"); \
	} \
	else \
	{ \
		PDEBUG_MUTEX("PROTECTOR LOCK RELEASED\n"); \
	}

/** Use for subdevices without interrupt. */
#  define ME_SUBDEVICE_LOCK \
	PDEBUG_MUTEX("LOCK SUBDEVICE\n"); \
	if (me_lock_context(&instance->base.subdevice_lock)) \
	{ \
			PDEBUG_MUTEX("SUBDEVICE LOCK ERROR!\n"); \
	} \
	else \
	{ \
			PDEBUG_MUTEX("SUBDEVICE LOCK SETTED\n"); \
	}

#  define ME_SUBDEVICE_UNLOCK \
	PDEBUG_MUTEX("UNLOCK SUBDEVICE\n"); \
	if (me_unlock_context(&instance->base.subdevice_lock)) \
	{ \
		PDEBUG_MUTEX("SUBDEVICE UNLOCK ERROR!\n"); \
	} \
	else \
	{ \
		PDEBUG_MUTEX("SUBDEVICE LOCK RELEASED\n"); \
	}

#  define ME_SPIN_LOCK(lock) \
	if(1) \
	{ \
		PDEBUG_MUTEX("LOCK %s\n", #lock); \
		ME_SET_LOCK(lock); \
	}

#  define ME_SPIN_UNLOCK(lock) \
	if(1) \
	{ \
		PDEBUG_MUTEX("UNLOCK %s\n", #lock); \
		ME_RELESE_LOCK(lock); \
	}

/// 'protector' must be 'irq_protector_t' structure!
#  define ME_LOCK_PROTECTOR \
	PDEBUG_MUTEX("ME_LOCK_PROTECTOR\n"); \
	if (me_lock_protector(&instance->base.subdevice_lock, instance->base.dev)) \
	{ \
		PDEBUG_MUTEX("PROTECTOR LOCK ERROR!\n"); \
	} \
	else \
	{ \
		PDEBUG_MUTEX("PROTECTOR LOCK SETTED\n"); \
	}

#  define ME_UNLOCK_PROTECTOR \
	PDEBUG_MUTEX("ME_UNLOCK_PROTECTOR\n"); \
	if (me_unlock_protector(&instance->base.subdevice_lock, instance->base.dev)) \
	{ \
		PDEBUG_MUTEX("PROTECTOR UNLOCK ERROR!\n"); \
	} \
	else \
	{ \
		PDEBUG_MUTEX("PROTECTOR LOCK RELEASED\n"); \
	}

#  if !defined(ME_USB)

#   define ME_IRQ_LOCK(lock) \
	if(1) \
	{ \
		PINFO("IRQ lock is just a dummy in PCI driver.\n"); \
	}

#   define ME_IRQ_UNLOCK(lock) \
	if(1) \
	{ \
		PINFO("IRQ lock is just a dummy in PCI driver.\n"); \
	}

#   define ME_DMA_LOCK(lock) \
	if(1) \
	{ \
		PERROR("DMA mode not available in PCI driver.\n"); \
	}

#   define ME_DMA_UNLOCK(lock) \
	if(1) \
	{ \
		PERROR("DMA mode not available in PCI driver.\n"); \
	}

#  else	// ME_USB

#   define ME_IRQ_LOCK(lock) \
	if(1) \
	{ \
		PDEBUG_MUTEX("IRQ LOCK %s\n", #lock); \
		down(&lock); \
	}

#   define ME_IRQ_UNLOCK(lock) \
	if(1) \
	{ \
		PDEBUG_MUTEX("IRQ UNLOCK %s\n", #lock); \
		up(&lock); \
	}

#   define ME_DMA_LOCK(lock) \
	if(1) \
	{ \
		PDEBUG_DMA_MUTEX("DMA LOCK %s\n", #lock); \
		down(lock); \
	}

#   define ME_DMA_UNLOCK(lock) if(1) \
	{ \
		PDEBUG_DMA_MUTEX("DMA UNLOCK %s\n", #lock); \
		up(lock); \
	}

#  endif	// ME_PCI/ME_USB
# endif		//_ME_SPIN_LOCK_H_
#endif		//__KERNEL__
