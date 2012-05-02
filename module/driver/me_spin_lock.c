
/**
 * @file me_spin_lock.c
 *
 * @brief Implementation of locks. (PCI and USB have different needs.)
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

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/thread_info.h>

# include "me_spin_lock.h"
# include "me_debug.h"

///*************************************  PCI  *********************************///
#if defined(ME_PCI) || defined(SCALE_RT)
#if !defined(SCALE_RT)
# if !defined(ME_ATRENATIVE_LOCKS)

#  define PDEBUG_MUTEX_LOCK_STATUS(protector) \
	if (1)	\
	{ \
		int raw_lock = protector->subdevice_lock.raw_lock.slock; \
		raw_lock = ((raw_lock>>16) & 0xFFFF) - (raw_lock & 0xFFFF); \
		PDEBUG_MUTEX("Spinlock raw status: %s (%x).\n", (raw_lock) ? "locked" : "unlocked", raw_lock); \
		PDEBUG_MUTEX("Spinlock     status: %s (%x).\n", (protector->status) ? "locked" : "unlocked", protector->status); \
	}

# else	// ME_ATRENATIVE_LOCKS

#  define PDEBUG_MUTEX_LOCK_STATUS(protector) \
	if (1)	\
	{ \
		int raw_lock = atomic_read(&protector->subdevice_lock.lock); \
		PDEBUG_MUTEX("Spinlock raw status: %s (%x).\n", (!raw_lock) ? "locked" : "unlocked", raw_lock); \
		PDEBUG_MUTEX("Spinlock     status: %s (%x).\n", (protector->status) ? "locked" : "unlocked", protector->status); \
	}

# endif	// ME_ATRENATIVE_LOCKS
#else	// SCALE_RT
#  define PDEBUG_MUTEX_LOCK_STATUS(protector)
#endif	// SCALE_RT

int me_lock_context(subdevice_protector_t* protector)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
		return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}
# endif	// PROTECTOR_CHECK

 	ME_SET_LOCK(&protector->subdevice_lock);

# ifdef PROTECTOR_CHECK
EXIT:
	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	protector->status = 1;
	protector->pid = current_thread_info()->task->pid;
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

int me_unlock_context(subdevice_protector_t* protector)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
		return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}

    protector->status = 0;
    protector->pid = -1;
#endif

	ME_RELESE_LOCK(&protector->subdevice_lock);

# ifdef PROTECTOR_CHECK
EXIT:
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

int me_lock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
		return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

    if (protector->blocked_irq)
    {
        PERROR_MUTEX("Spinlock: IRQ%d blocked.\n", protector->blocked_irq);
        if (dev->irq_no != protector->blocked_irq)
        {
            PERROR_MUTEX("Spinlock: WRONG IRQ number. dev->%d != protector->%d\n", dev->irq_no, protector->blocked_irq);
        }
    }

	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}
    if (dev->irq_no)
    {
        protector->blocked_irq = dev->irq_no;
    }
# endif

    ME_SET_LOCK_SAVE(&protector->subdevice_lock, dev->irq_no);

# ifdef PROTECTOR_CHECK
EXIT:
	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

    protector->pid = current_thread_info()->task->pid;
    protector->status = 1;
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

int me_unlock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

    if (!protector->blocked_irq)
    {
        PERROR_MUTEX("Spinlock: IRQ not blocked.\n");
    }
    else if (dev->irq_no != protector->blocked_irq)
    {
        PERROR_MUTEX("Spinlock: WRONG IRQ number. dev->%d != protector->%d\n", dev->irq_no, protector->blocked_irq);
    }

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}

    protector->pid = -1;
    protector->status = 0;

    if (dev->irq_no)
    {
        protector->blocked_irq = 0;
    }
# endif

    ME_RELESE_LOCK_RESTORE(&protector->subdevice_lock, dev->irq_no);

# ifdef PROTECTOR_CHECK
EXIT:
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

/** In interrupt context. */
int me_handler_protector(subdevice_protector_t* protector)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}
# endif	// PROTECTOR_CHECK

	ME_IRQ_TEST_LOCK(&protector->subdevice_lock);

# ifdef PROTECTOR_CHECK
EXIT:
	if (protector->status)
	{
		PERROR_MUTEX("Marked as LOCKED!\n");
		err = ME_MUTEX_DEATHLOCK;
	}
	else if (protector->pid != -1)
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (protector->blocked_irq)
	{
		PERROR_MUTEX("Spinlock: IRQ%d blocked.\n", protector->blocked_irq);
	}

	protector->pid = current_thread_info()->task->pid;
	protector->status = 1;
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

int me_free_handler_protector(subdevice_protector_t* protector)
{
# ifdef PROTECTOR_CHECK
	int err = ME_ERRNO_SUCCESS;
# endif	// PROTECTOR_CHECK

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	PDEBUG_MUTEX_LOCK_STATUS(protector);

	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

    if (protector->blocked_irq)
    {
        PERROR_MUTEX("Spinlock: IRQ%d blocked.\n", protector->blocked_irq);
    }

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as NOT LOCKED.\n");
		err = ME_MUTEX_DEATHLOCK;

		goto EXIT;
	}

	protector->pid = -1;
	protector->status = 0;

# endif

	ME_IRQ_LOCK_RESTORE(&protector->subdevice_lock);

# ifdef PROTECTOR_CHECK
EXIT:
	return err;
# else
	return ME_ERRNO_SUCCESS;
# endif
}

///************************* USB  *********************************************///
#elif defined(ME_USB)
int me_lock_context(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
		return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
# endif	// PROTECTOR_CHECK

# ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
# endif // PROTECTOR_CHECK

		down(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
# endif
	return err;
}

int me_unlock_context(subdevice_protector_t* protector)
{
	int err = 0;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}

	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
# endif
		up(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
	}
#endif
	return err;
}

int me_lock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
# endif	// PROTECTOR_CHECK

# ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
# endif // PROTECTOR_CHECK

		down(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
# endif
	return err;
}

int me_unlock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
	int err = 0;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}

	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
# endif
		up(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
	}
#endif
	return err;
}

/** In interrupt context. */

int me_handler_protector(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
  return ME_ERRNO_SUCCESS;

#  ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
#  endif // PROTECTOR_CHECK

#  ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
#  endif // PROTECTOR_CHECK
		down(&protector->subdevice_semaphore);
		err = ME_ERRNO_SUCCESS;
#  ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
#  endif // PROTECTOR_CHECK
	return err;
}

int me_free_handler_protector(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
  return ME_ERRNO_SUCCESS;

#  ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_WRONG_STATUS;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}
#  endif	// PROTECTOR_CHECK

#  ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
#  endif	// PROTECTOR_CHECK
		up(&protector->subdevice_semaphore);

#  ifdef PROTECTOR_CHECK
	}
#  endif	// PROTECTOR_CHECK
	return err;
}

///************************* MEPHISTO  *********************************************///
#elif defined(ME_USB)
int me_lock_context(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
		return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
# endif	// PROTECTOR_CHECK

# ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
# endif // PROTECTOR_CHECK

		down(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
# endif
	return err;
}

int me_unlock_context(subdevice_protector_t* protector)
{
	int err = 0;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}

	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
# endif
		up(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
	}
#endif
	return err;
}

int me_lock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
# endif	// PROTECTOR_CHECK

# ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
# endif // PROTECTOR_CHECK

		down(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
# endif
	return err;
}

int me_unlock_protector(subdevice_protector_t* protector, me_general_dev_t* dev)
{
	int err = 0;

	if (!protector)
  return ME_ERRNO_SUCCESS;

# ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_DEATHLOCK;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}

	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
# endif
		up(&protector->subdevice_semaphore);
# ifdef PROTECTOR_CHECK
	}
#endif
	return err;
}

/** In interrupt context. */

int me_handler_protector(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
 		return ME_ERRNO_SUCCESS;

#  ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid == protector->pid))
	{
		PERROR_MUTEX("DEATHLOCK PID %d.\n", protector->pid);
		err = ME_MUTEX_DEATHLOCK;
	}

	if (!protector->status && (protector->pid != -1))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
			err = ME_MUTEX_FORCE;
		}
	}
#  endif // PROTECTOR_CHECK

#  ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
#  endif // PROTECTOR_CHECK
		down(&protector->subdevice_semaphore);
		err = ME_ERRNO_SUCCESS;
#  ifdef PROTECTOR_CHECK
		protector->pid = current_thread_info()->task->pid;
		protector->status = 1;
	}
#  endif // PROTECTOR_CHECK
	return err;
}

int me_free_handler_protector(subdevice_protector_t* protector)
{
	int err = ME_ERRNO_SUCCESS;

	if (!protector)
  return ME_ERRNO_SUCCESS;

#  ifdef PROTECTOR_CHECK
	if (protector->status && (current_thread_info()->task->pid != protector->pid))
	{
		PERROR_MUTEX("WRONG PID %d.\n", protector->pid);
		err = ME_MUTEX_WRONG_PID;
	}

	if (!protector->status)
	{
		PERROR_MUTEX("Marked as not locked.\n");
		err = ME_MUTEX_WRONG_STATUS;
	}

	if (err)
	{
		if (atomic_read(&protector->subdevice_semaphore.count) == 0)
		{
			PERROR_MUTEX("Semaphore: down.\n");
			err = ME_MUTEX_FORCE;
		}
		else
		{
			PERROR_MUTEX("Semaphore: up.\n");
		}
	}
#  endif	// PROTECTOR_CHECK

#  ifdef PROTECTOR_CHECK
	if (!err || (err == ME_MUTEX_FORCE))
	{
		protector->pid = -1;
		protector->status = 0;
#  endif	// PROTECTOR_CHECK
		up(&protector->subdevice_semaphore);

#  ifdef PROTECTOR_CHECK
	}
#  endif	// PROTECTOR_CHECK
	return err;
}
#endif  // ME_MEPHISTO
