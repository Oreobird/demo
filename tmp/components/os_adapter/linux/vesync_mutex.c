/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */
 
#include <stdlib.h>
#include <pthread.h>
#include "vesync_common.h"
#include "vesync_os.h"
#include "vesync_mutex.h"

int vesync_mutex_new(vesync_mutex_t *mtx)
{
	pthread_mutex_t *pmtx = malloc(sizeof(pthread_mutex_t));
	if (pmtx == NULL)
	{
		return -1;
	}

	pthread_mutex_init(pmtx, NULL);
	mtx->handle = pmtx;
	return 0;
}

int vesync_mutex_lock(vesync_mutex_t *mtx, unsigned int timeout)
{
	UNUSED(timeout);
	
	if (mtx)
	{
		pthread_mutex_lock(mtx->handle);
	}
	return 0;
}

int vesync_mutex_unlock(vesync_mutex_t *mtx)
{
	if (mtx)
	{
		pthread_mutex_unlock(mtx->handle);
	}
	return 0;
}

void vesync_mutex_free(vesync_mutex_t *mtx)
{
	if (mtx)
	{
		pthread_mutex_destroy(mtx->handle);
		free(mtx->handle);
	}
}

