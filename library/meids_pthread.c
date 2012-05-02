/* Thread implementation for Meilhaus driver system.
 * =================================================
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
 *  Author:	Guenter Gebhardt
 *  Author:	Krzysztof Gantzke	<k.gantzke@meilhaus.de>
 */

#ifdef __KERNEL__
# error This is user space library!
#endif	//__KERNEL__

#include "meids_pthread.h"

int pthread_rdwr_init_np(pthread_rdwr_t *rdwrp, pthread_rdwrattr_t *attrp)
{
	rdwrp->readers_reading = 0;
	rdwrp->writer_writing = 0;
	pthread_mutex_init(& (rdwrp->mutex), NULL);
	pthread_cond_init(& (rdwrp->lock_free), NULL);
	return 0;
}

int pthread_rdwr_rlock_np(pthread_rdwr_t *rdwrp)
{
	pthread_mutex_lock(& (rdwrp->mutex));

	while (rdwrp->writer_writing)
	{
		pthread_cond_wait(& (rdwrp->lock_free), & (rdwrp->mutex));
	}

	rdwrp->readers_reading++;
	pthread_mutex_unlock(& (rdwrp->mutex));

	return 0;
}

int pthread_rdwr_runlock_np(pthread_rdwr_t *rdwrp)
{
	pthread_mutex_lock(& (rdwrp->mutex));

	if (rdwrp->readers_reading == 0)
	{
		pthread_mutex_unlock(& (rdwrp->mutex));
		return -1;
	}
	else
	{
		rdwrp->readers_reading--;

		if (rdwrp->readers_reading == 0)
		{
			pthread_cond_signal(& (rdwrp->lock_free));
		}

		pthread_mutex_unlock(& (rdwrp->mutex));

		return 0;
	}

	return 0;
}

int pthread_rdwr_wlock_np(pthread_rdwr_t *rdwrp)
{
	pthread_mutex_lock(& (rdwrp->mutex));

	while (rdwrp->writer_writing || rdwrp->readers_reading)
	{
		pthread_cond_wait(& (rdwrp->lock_free), & (rdwrp->mutex));
	}

	rdwrp->writer_writing++;
	pthread_mutex_unlock(& (rdwrp->mutex));

	return 0;
}

int pthread_rdwr_wunlock_np(pthread_rdwr_t *rdwrp)
{
	pthread_mutex_lock(& (rdwrp->mutex));

	if (rdwrp->writer_writing == 0)
	{
		pthread_mutex_unlock(& (rdwrp->mutex));
		return -1;
	}
	else
	{
		rdwrp->writer_writing = 0;
		pthread_cond_broadcast(& (rdwrp->lock_free));
		pthread_mutex_unlock(& (rdwrp->mutex));
		return 0;
	}

	return 0;
}
