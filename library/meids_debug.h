/**
 * @file meids_debug.h
 *
 * @brief Debugging defines for ME-iDS library.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author Krzysztof Gantzke	(k.gantzke@meilhaus.de)
 */

#ifndef _MEIDS_DEBUG_H_
# define _MEIDS_DEBUG_H_

#include <syslog.h>

/* Switch on/off to enable/disenable debug messages. */
# ifdef LIBMEDEBUG_TEST
#  ifndef LIBMEDEBUG_CRITICAL_ERROR
#   define LIBMEDEBUG_CRITICAL_ERROR
#  endif
#  ifndef LIBMEDEBUG_ERROR
#   define LIBMEDEBUG_ERROR
#  endif
#  ifndef LIBMEDEBUG_WARNING
#   define LIBMEDEBUG_WARNING
#  endif

#  ifndef LIBMEDEBUG_DEBUG
#   define LIBMEDEBUG_DEBUG
#  endif
#  ifndef LIBMEDEBUG_INFO
#   define LIBMEDEBUG_INFO
#  endif
# endif

/* Only to be sure. */
# undef LIBPDEVELOP
# undef LIBPCRITICALERROR
# undef LIBPERROR
# undef LIBPWARNING
# undef LIBPDEBUG
# undef LIBPINFO
# undef LIBPXML

/* For developing purpose. */
#ifdef LIBMEDEBUG_DEVELOP
#define LIBPDEVELOP(fmt, args...) syslog(LOG_CRIT, "<%s:%s:%i> " fmt, __FILE__, __FUNCTION__, __LINE__, ##args)
#else
#define LIBPDEVELOP(fmt, args...) /* Do nothing. */
#endif

/* Critical, for ME-iDS, errors in program execution. This stuff MUST be handled. */
#ifdef LIBMEDEBUG_CRITICAL_ERROR
#define LIBPCRITICALERROR(fmt, args...) syslog(LOG_CRIT, "ME_LIB: <%s:%i> " fmt, __FILE__, __LINE__, ##args)
#else
#define LIBPCRITICALERROR(fmt, args...) /* Do nothing. */
#endif

/* Errors in program. This MUST be checked and corrected. */
#ifdef LIBMEDEBUG_ERROR
#define LIBPERROR(fmt, args...) syslog(LOG_ERR, "ME_LIB: <%s:%i> " fmt, __FILE__, __LINE__, ##args)
#else
#define LIBPERROR(fmt, args...) /* Do nothing. */
#endif

/* Errors in program. This MUST be checked and corrected. */
#ifdef LIBMEDEBUG_XML
#define LIBPXML(fmt, args...) syslog(LOG_ERR, "ME_XML: <%s:%i> " fmt, __FILE__, __LINE__, ##args)
#else
#define LIBPXML(fmt, args...) /* Do nothing. */
#endif

/* Somethihg strange. Lets check. */
#ifdef LIBMEDEBUG_WARNING
#define LIBPWARNING(fmt, args...) syslog(LOG_WARNING, "ME_LIB: <%s:%i> " fmt, __FILE__, __LINE__, ##args)
#else
#define LIBPWARNING(fmt, args...) /* Do nothing. */
#endif

/* Control messages ->> call flow */
#ifdef LIBMEDEBUG_DEBUG
#define LIBPDEBUG(fmt, args...) syslog(LOG_DEBUG, "ME_LIB: <%s> " fmt, __FUNCTION__, ##args)
#else
#define LIBPDEBUG(fmt, args...) /* Do nothing. */
#endif

/* Informational messages. Low importance. */
#ifdef LIBMEDEBUG_INFO
#define LIBPINFO(fmt, args...) syslog(LOG_INFO, "ME_LIB: " fmt, ##args)
#else
#define LIBPINFO(fmt, args...) /* Do nothing. */
#endif

/* Execution time. */
#ifdef LIBMEDEBUG_TIMESTAMPS
#define LIBPEXECTIME(fmt, args...) syslog(LOG_INFO, "ME_LIB: %s: " fmt, __FUNCTION__, ##args)
#else
#define LIBPEXECTIME(fmt, args...) /* Do nothing. */
#endif

#endif	//_MEIDS_DEBUG_H_
