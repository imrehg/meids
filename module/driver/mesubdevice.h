/**
 * @file mesubdevice.h
 *
 * @brief Provides the subdevice base class.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
 * Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
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
# ifndef _MESUBDEVICE_H_
#  define _MESUBDEVICE_H_

#  include <linux/version.h>

#  include "me_types.h"
#  include "me_ioctl.h"
#  include "meslock.h"

#  include <linux/fs.h>
#  include <linux/list.h>
#  include <linux/workqueue.h>
#  include <asm/atomic.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#  include <asm/semaphore.h>
#else
#  include <linux/semaphore.h>
#endif

// // This two macros are removed in kernel 2.6.37. Restore them!
// #ifndef init_MUTEX
// # define init_MUTEX(sem)         sema_init(sem, 1)
// #endif
// #ifndef init_MUTEX_LOCKED
// # define init_MUTEX_LOCKED(sem)  sema_init(sem, 0)
// #endif

#include "melock_defines.h"

/**
 * @brief Macro used to enter a subdevice.
 */
#define ME_SUBDEVICE_ENTER	\
	if (1) \
	{ \
		int err; \
		if (!instance) \
		{ \
			PERROR_CRITICAL("NULL pointer to subdevice instance!\n"); \
			return ME_ERRNO_INTERNAL; \
		} \
		err = me_slock_enter(&instance->base.lock, filep); \
		if(err) \
		{ \
			PERROR("Cannot enter subdevice.\n"); \
			return err; \
		} \
	}


/**
 * @brief Macro used to exit a subdevice.
 */
#define ME_SUBDEVICE_EXIT	\
	if (1) \
	{\
		int err; \
		err = me_slock_exit(&instance->base.lock, filep); \
		if(err) \
		{ \
			PERROR("Cannot exit subdevice.\n"); \
			return err; \
		} \
	}

/**
 * @brief Macro used to check if pointer is not NULL.
 */
#  define CHECK_POINTER(pointer) \
	if (!pointer) \
	{ \
		PERROR("Invalid pointer.\n"); \
		return ME_ERRNO_INVALID_POINTER;	\
	}

typedef struct// subdevice_protector
{
#  if !defined(ME_USB)
	me_lock_t			subdevice_lock;
#else
	// Subdevice context semaphore
	struct semaphore	subdevice_semaphore;
#  endif

# ifdef PROTECTOR_CHECK
	volatile int		status;
	volatile int		blocked_irq;
	pid_t				pid;
# endif

} subdevice_protector_t;

/**
 * @brief The subdevice base class.
 */
typedef struct me_subdevice
{
	void* dev;

	// Attributes
	unsigned int idx;			/**< The index of subdevice. */
	struct list_head list;		/**< Enables the subdevice to be added to a dynamic list. */
	me_slock_t lock;			/**< Used by user application in order to lock the subdevice for exclusive usage. */
	subdevice_protector_t subdevice_lock;	/**< Interrupt blocking lock structure to protect interrupt handler. */

	// Methods
	int (*me_subdevice_io_irq_start)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int irq_source, int irq_edge, int irq_arg, int flags);

	int (*me_subdevice_io_irq_wait)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int* irq_count, int* value, int time_out, int flags);

	int (*me_subdevice_io_irq_stop)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int flags);

	int (*me_subdevice_io_irq_test)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int flags);

	int (*me_subdevice_io_reset_subdevice)(struct me_subdevice* subdevice, struct file* filep,
										int flags);

	int (*me_subdevice_io_single_config)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);

	int (*me_subdevice_io_single_read)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int* value, int time_out, int flags);

	int (*me_subdevice_io_single_write)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int value, int time_out, int flags);

	int (*me_subdevice_io_stream_config)(struct me_subdevice* subdevice, struct file* filep,
										meIOStreamSimpleConfig_t* config_list, int count, meIOStreamSimpleTriggers_t* trigger, int fifo_irq_threshold, int flags);

	int (*me_subdevice_io_stream_new_values)(struct me_subdevice* subdevice, struct file* filep,
										int time_out, int* count, int flags);

	int (*me_subdevice_io_stream_read)(struct me_subdevice* subdevice, struct file* filep,
										int read_mode, int* values, int* count, int timeout, int flags);

	int (*me_subdevice_io_stream_start)(struct me_subdevice* subdevice, struct file* filep,
										int start_mode, int time_out, int flags);

	int (*me_subdevice_io_stream_status)(struct me_subdevice* subdevice, struct file* filep,
										int wait, int* status, int* count, int flags);

	int (*me_subdevice_io_stream_stop)(struct me_subdevice* subdevice, struct file* filep,
										int stop_mode, int time_out, int flags);

	int (*me_subdevice_io_stream_write)(struct me_subdevice* subdevice, struct file* filep,
										int write_mode, int* values, int* count, int timeout, int flags);

	int (*me_subdevice_lock_subdevice)(struct me_subdevice* subdevice, struct file* filep,
										int lock, int flags);

	int (*me_subdevice_set_offset)(struct me_subdevice* subdevice, struct file* filep,
										int channel, int range, int* offset, int flags);

	int (*me_subdevice_query_number_channels)(struct me_subdevice* subdevice,
									  	int* number);

	int (*me_subdevice_query_number_ranges)(struct me_subdevice* subdevice,
		   								int unit, 	int* count);

	int (*me_subdevice_query_range_by_min_max)(struct me_subdevice* subdevice,
		   								int unit,  	int* min, 	int* max, 	int* maxdata, 	int* range);

	int (*me_subdevice_query_range_info)(struct me_subdevice* subdevice,
		   									int range, int* unit, int* min, int* max, int* maxdata);

	int (*me_subdevice_query_subdevice_type)(struct me_subdevice* subdevice,
		   										int* type, int* subtype);

	int (*me_subdevice_query_subdevice_caps)(struct me_subdevice* subdevice,
		  									 	int* caps);

	int (*me_subdevice_query_subdevice_caps_args)(struct me_subdevice* subdevice,
													int cap, int* args, int* count);

	int (*me_subdevice_query_timer)(struct me_subdevice* subdevice,
													int timer, int* base_frequency, uint64_t* min_ticks, uint64_t* max_ticks);

	int (*me_subdevice_config_load)(struct me_subdevice* subdevice, struct file* filep,
													void* config);

	int (*me_subdevice_query_version_firmware)(struct me_subdevice* subdevice,
													int* firmware);

	void (*me_subdevice_destructor)(struct me_subdevice* subdevice);

	// Main IRQ handling point.
	int (*me_subdevice_irq_handle)(struct me_subdevice* subdevice,
									uint32_t irq_status);

	int (*me_subdevice_postinit)(struct me_subdevice* subdevice,
									void* args);
} me_subdevice_t;

/**
 * @brief Initializes a subdevice structure.
 *
 * @param subdevice The subdevice structure to initialize.
 * @return 0 on success.
 */
int me_subdevice_init(me_subdevice_t* subdevice);

/**
 * @brief Deinitializes a subdevice structure.
 *
 * @param subdevice The subdevice structure to initialize.
 */
void me_subdevice_deinit(me_subdevice_t* subdevice);


# endif	//_MESUBDEVICE_H_
#endif	//__KERNEL__
