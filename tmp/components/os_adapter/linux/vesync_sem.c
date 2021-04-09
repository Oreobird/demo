/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include "vesync_common.h"
#include "vesync_os.h"
#include "vesync_sem.h"

int vesync_sem_new(vesync_sem_t *sem)
{
	sem_t *psem = sem_open("sem", O_CREAT, 0666, 0);
	if (psem == SEM_FAILED)
	{
		return -1;
	}

	sem->handle = psem;
	
	return 0;
}

int vesync_sem_wait(vesync_sem_t *sem, unsigned int timeout_ms)
{
	int sec = 0;
	int nsec = 0;

	NULL_PARAM_CHK(sem);
	
	if (timeout_ms == VESYNC_OS_WAIT_FOREVER)
	{
		printf("sem_wait\n");
		return sem_wait(sem->handle);
	} 
	else if (timeout_ms == VESYNC_OS_NO_WAIT)
	{
		return sem_trywait(sem->handle);
	}
	
	struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    sec = timeout_ms / 1000;
    nsec = (timeout_ms % 1000) * 1000000;

    ts.tv_nsec += nsec;
    sec += (ts.tv_nsec / 1000000000);
    ts.tv_nsec %= 1000000000;
    ts.tv_sec += sec;

    return sem_timedwait(sem->handle, &ts);
}

int vesync_sem_signal(vesync_sem_t *sem)
{
	int ret = -1;
	ret = sem_post(sem->handle);
	if (ret < 0)
	{
		return -1;
	}
	return 0;
}

int vesync_sem_free(vesync_sem_t *sem)
{
	sem_close(sem->handle);
	int ret = sem_unlink("sem");
	if (ret < 0)
	{
		return -1;
	}

	return 0;
}

