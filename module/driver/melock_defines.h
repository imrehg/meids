#ifdef __KERNEL__

# ifndef _MELOCK_DEFINES_H_
#  define _MELOCK_DEFINES_H_

#  if defined(SCALE_RT)
#   include <asm/atomic.h>
#   include <linux/sched.h>
#   include <rtdm/rtdm_driver.h>

	typedef struct
	{
		rtdm_lock_t			lock;
		rtdm_lock_t			pre_lock;
		rtdm_lockctx_t		irq_context;
		atomic_t			busy_flag;
	} me_lock_t;

#   define ME_INIT_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
			rtdm_lock_init(&(LOCK)->pre_lock); \
			rtdm_lock_init(&(LOCK)->lock); \
			atomic_set(&((LOCK)->busy_flag), 0); \
	}

#   define ME_SET_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
			rtdm_lock_get(&(LOCK)->pre_lock); \
			rtdm_lock_get(&(LOCK)->lock); \
	}

#   define ME_RELESE_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
			rtdm_lock_put(&(LOCK)->lock); \
			rtdm_lock_put(&(LOCK)->pre_lock); \
	}

#   define ME_SET_LOCK_SAVE(LOCK, IRQ_NO) \
	if (LOCK != NULL) \
	{ \
			rtdm_lock_get(&(LOCK)->pre_lock); \
			rtdm_lock_get_irqsave(&(LOCK)->lock, (LOCK)->irq_context); \
			atomic_inc(&((LOCK)->busy_flag)); \
	}

#   define ME_RELESE_LOCK_RESTORE(LOCK, IRQ_NO) \
	if (LOCK != NULL) \
	{ \
			atomic_dec(&((LOCK)->busy_flag)); \
			rtdm_lock_put_irqrestore(&(LOCK)->lock, (LOCK)->irq_context); \
			rtdm_lock_put(&(LOCK)->pre_lock); \
	}

#   define ME_IRQ_TEST_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		if(atomic_read(&((LOCK)->busy_flag))) \
		{ \
			PINFO("BUSY! Postpone IRQ.\n"); \
			return ME_ERRNO_SUBDEVICE_BUSY; \
		} \
	}

#   define ME_IRQ_LOCK_RESTORE(LOCK) \
	if (LOCK != NULL) \
	{ \
	}


#  else  // SCALE_RT

#   if !defined(ME_ATRENATIVE_LOCKS)
#   include <linux/spinlock.h>

	typedef spinlock_t me_lock_t;

#    define ME_INIT_LOCK(LOCK)			if (LOCK != NULL) { spin_lock_init(LOCK); }
#    define ME_SET_LOCK(LOCK)			if (LOCK != NULL) { spin_lock(LOCK); }
#    define ME_RELESE_LOCK(LOCK)		if (LOCK != NULL) { spin_unlock(LOCK); }
#    define ME_IRQ_TEST_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		if (!spin_trylock(LOCK)) \
		{ \
			return ME_ERRNO_SUBDEVICE_BUSY; \
		} \
	}

#   else	// ME_ATRENATIVE_LOCKS

#   include <asm/atomic.h>
#   include <linux/sched.h>

	struct me_atomic_lock
	{
		atomic_t enter_lock;
		atomic_t exit_lock;
	};
	typedef struct me_atomic_lock me_lock_t;

#    define ME_INIT_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		atomic_set(&((LOCK)->enter_lock), 0); \
		atomic_set(&((LOCK)->exit_lock), 1); \
	}

#    define ME_SET_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		int magicID; \
		magicID = atomic_inc_return(&((LOCK)->enter_lock)); \
		PDEBUG_MUTEX("magicID=%x %x->%x\n", magicID, atomic_read(&((LOCK)->enter_lock)), atomic_read(&((LOCK)->exit_lock))); \
		while (atomic_read(&((LOCK)->exit_lock)) != magicID); \
		PDEBUG_MUTEX("LOCK SET =%x->%x\n", atomic_read(&((LOCK)->enter_lock)), atomic_read(&((LOCK)->exit_lock))); \
	}

#    define ME_RELESE_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		if (((atomic_read(&((LOCK)->enter_lock))+1) & 0xFFFF) != (atomic_read(&((LOCK)->exit_lock)) & 0xFFFF)) \
			atomic_inc(&((LOCK)->exit_lock)); \
		PDEBUG_MUTEX("LOCK RELESED %x->%x\n", atomic_read(&((LOCK)->enter_lock)), atomic_read(&((LOCK)->exit_lock))); \
	}

#    define ME_IRQ_TEST_LOCK(LOCK) \
	if (LOCK != NULL) \
	{ \
		if (((atomic_read(&((LOCK)->enter_lock))+1) & 0xFFFF) != (atomic_read(&((LOCK)->exit_lock)) & 0xFFFF)) \
		{ \
			return ME_ERRNO_SUBDEVICE_BUSY; \
		} \
		else \
		{ \
			ME_SET_LOCK(LOCK); \
		} \
	}

#   endif	// ME_ATRENATIVE_LOCKS

#   define ME_SET_LOCK_SAVE(LOCK, IRQ_NO) \
	if (LOCK != NULL) \
	{ \
    	if (IRQ_NO) \
		{ \
			disable_irq(IRQ_NO); \
			PDEBUG_MUTEX("disable_irq(%d)\n", IRQ_NO); \
		} \
		ME_SET_LOCK(LOCK); \
	}

#   define ME_RELESE_LOCK_RESTORE(LOCK, IRQ_NO) \
	if (LOCK != NULL) \
	{ \
		ME_RELESE_LOCK(LOCK); \
		if (IRQ_NO) \
		{ \
			enable_irq(IRQ_NO); \
			PDEBUG_MUTEX("enable_irq(%d)\n", IRQ_NO); \
		} \
	}

#   define ME_IRQ_LOCK_RESTORE(LOCK) if (LOCK) { ME_RELESE_LOCK(LOCK); }


#  endif  // SCALE_RT
# endif		//__KERNEL__
#endif		//_MELOCK_DEFINES_H_
