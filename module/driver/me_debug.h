/**
 * @file me_debug.h
 *
 * @brief Debugging defines.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

#ifdef __KERNEL__

#include <linux/stringify.h>

# ifndef _ME_DEBUG_H_
# define _ME_DEBUG_H_

#include <linux/kernel.h>

//Messages control.

#ifdef MEDEBUG_TEST_ALL			/* Switch to enable all info messages. */
# ifndef MEDEBUG_TEST
#  define MEDEBUG_TEST
# endif
# ifndef MEDEBUG_TEST_INFO
#  define MEDEBUG_TEST_INFO
# endif
# ifndef MEDEBUG_DEBUG_REG
#  define MEDEBUG_DEBUG_REG		/* Switch to enable registry access debuging messages. */
# endif
# ifndef MEDEBUG_DEBUG_LIST
#  define MEDEBUG_DEBUG_LIST		/* Switch to enable list menagment messages. */
# endif
# ifndef MEDEBUG_DEBUG_LOCKS
#  define MEDEBUG_DEBUG_LOCKS		/* Switch to enable locking messages. */
# endif
#endif

#ifdef MEDEBUG_TEST_INFO		/* Switch to enable info  and test messages. */
# ifndef MEDEBUG_INFO
#  define MEDEBUG_INFO			/* Switch to enable info messages. */
# endif
# ifndef MEDEBUG_TEST
#  define MEDEBUG_TEST
# endif
#endif

#ifdef MEDEBUG_TEST			/* Switch to enable debug test messages. */
# ifndef MEDEBUG_DEBUG
#  define MEDEBUG_DEBUG			/* Switch to enable debug messages. */
# endif
# ifndef MEDEBUG_ERROR
#  define MEDEBUG_ERROR			/* Switch to enable error messages. */
# endif
#endif

#ifdef MEDEBUG_ERROR			/* Switch to enable error messages. */
# ifndef MEDEBUG_ERROR_CRITICAL		/* Also critical error messages. */
#  define MEDEBUG_ERROR_CRITICAL	/* Switch to enable high importance error messages. */
# endif
#endif

#ifdef MEDEBUG_ERROR_CRITICAL
# ifndef MEDEBUG_MUTEX_ERROR
#  define MEDEBUG_MUTEX_ERROR
# endif
#endif


#ifdef MEDEBUG_DEBUG_MUTEX
# ifndef MEDEBUG_MUTEX_ERROR
#  define MEDEBUG_MUTEX_ERROR
# endif
#endif

#undef PDEBUG				/* Only to be sure. */
#undef PINFO				/* Only to be sure. */
#undef PERROR				/* Only to be sure. */
#undef PERROR_CRITICAL		/* Only to be sure. */
#undef PDEBUG_REG			/* Only to be sure. */
#undef PDEBUG_TRANS			/* Only to be sure. */
#undef PDEBUG_TRANS_FLOAT	/* Only to be sure. */
#undef PDEBUG_LOCKS			/* Only to be sure. */
#undef PDEBUG_MUTEX			/* Only to be sure. */
#undef PDEBUG_MUTEX_ERROR	/* Only to be sure. */
#undef PDEBUG_LIST			/* Only to be sure. */
#undef PDEBUG_BUF			/* Only to be sure. */
#undef PSECURITY			/* Only to be sure. */
#undef PLOG					/* Only to be sure. */

#if defined (ME_PCI)
# define ME_DRV "ME_PCI"
#elif defined (ME_USB)
# define ME_DRV "ME_USB"
#else
# define ME_DRV "MEiDS"
#endif

#ifdef MEDEBUG_DEBUG
# define PDEBUG(fmt, args...) \
	printk(KERN_DEBUG "%s D: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_LIST
# define PDEBUG_LIST(fmt, args...) \
	printk(KERN_INFO "%s l: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG_LIST(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_MUTEX
# define PDEBUG_MUTEX(fmt, args...) \
	printk(KERN_INFO "%s M: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG_MUTEX(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_DMA_MUTEX
# define PDEBUG_DMA_MUTEX(fmt, args...) \
	printk(KERN_INFO "%s M: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG_DMA_MUTEX(fmt, args...)
#endif

#ifdef MEDEBUG_MUTEX_ERROR
# define PERROR_MUTEX(fmt, args...) \
	printk(KERN_CRIT "%s MC: <%s:%i> " fmt, ME_DRV, __FUNCTION__, __LINE__, ##args)
#else
# define PERROR_MUTEX(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_LOCKS
# define PDEBUG_LOCKS(fmt, args...) \
	printk(KERN_INFO "%s L: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG_LOCKS(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_TRANS
# define PDEBUG_TRANS(fmt, args...) \
	printk(KERN_DEBUG "%s T: <%s:%d> " fmt, ME_DRV, __FUNCTION__, __LINE__, ##args)
#else
# define PDEBUG_TRANS(fmt, args...)
#endif

#if defined(MEDEBUG_DEBUG_TRANS) && defined(MEDEBUG_DEBUG_TRANS_FLOAT)
# define PDEBUG_TRANS_FLOAT(fmt, args...) \
	printk(KERN_DEBUG "%s T: <%s:%d> " fmt, ME_DRV, __FUNCTION__, __LINE__, ##args)
#else
# define PDEBUG_TRANS_FLOAT(fmt, args...)
#endif

#ifdef MEDEBUG_DEBUG_REG
# define PDEBUG_REG(fmt, args...) \
	printk(KERN_DEBUG "%s R: " fmt, ME_DRV, ##args)
#else
# define PDEBUG_REG(fmt, args...)
#endif

#ifdef MEDEBUG_INFO
# define PINFO(fmt, args...) \
	printk(KERN_INFO "%s I: " fmt, ME_DRV, ##args)
#else
# define PINFO(fmt, args...)
#endif

#ifdef MEDEBUG_ERROR
# define PERROR(fmt, args...) \
	printk(KERN_ERR "%s E: <%s:%i> " fmt, ME_DRV, __FILE__, __LINE__, ##args)
#else
# define PERROR(fmt, args...)
#endif

#ifdef MEDEBUG_ERROR_CRITICAL
# define PERROR_CRITICAL(fmt, args...) \
	printk(KERN_CRIT "%s C: <%s:%i> " fmt, ME_DRV, __FILE__, __LINE__, ##args)
#else
# define PERROR_CRITICAL(fmt, args...)
#endif

#ifdef MEDEBUG_SPEED_TEST
# define PSPEED(fmt, args...) \
	printk(KERN_DEBUG "%s S: " fmt, ME_DRV, ##args)
#else
# define PSPEED(fmt, args...)
#endif

# define PMULTILINE(fmt, args...) \
	printk(fmt, ##args)

//This debug is only to detect logical errors!
# define PSECURITY(fmt, args...) \
	printk(KERN_CRIT"ME_DRV SECURITY: <%s:%s:%i> " fmt, ME_DRV, __FILE__, __FUNCTION__, __LINE__, ##args)
//This debug is to keep track in customers' system
# define PLOG(fmt, args...) \
	printk(KERN_INFO "%s: " fmt, ME_DRV, ##args)

#ifdef MEDEBUG_DEBUG_BUF
# define PDEBUG_BUF(fmt, args...) \
	printk(KERN_INFO "%s b: <%s> " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PDEBUG_BUF(fmt, args...)
#endif


//This debug is to check new parts during development
#ifdef MEDEBUG_DEVELOP
# define PDEVELOP(fmt, args...) \
	printk(KERN_CRIT "%s: <%s:%s:%i> " fmt, ME_DRV, __FILE__, __FUNCTION__, __LINE__, ##args)
#else
# define PDEVELOP(fmt, args...)
#endif

#ifdef MEDEBUG_TIMESTAMPS
# define PEXECTIME(fmt, args...) \
	printk(KERN_INFO "%s: %s " fmt, ME_DRV, __FUNCTION__, ##args)
#else
# define PEXECTIME(fmt, args...)
#endif

# endif	//_MEDEBUG_H_
#endif	//__KERNEL__
