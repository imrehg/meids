/* Shared library for Meilhaus driver system.
 * ==========================================
 *
 *  Copyright (C) 2005 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifndef _ME_PTHREAD_H_
# define _ME_PTHREAD_H_

#include <pthread.h>

typedef struct pthread_rdwr
{
	int readers_reading;
	int writer_writing;
	pthread_mutex_t mutex;
	pthread_cond_t lock_free;
}
pthread_rdwr_t;

typedef void * pthread_rdwrattr_t;

# define pthread_rdwrattr_default NULL;

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp);
int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp);
int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp);

#endif
