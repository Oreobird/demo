/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <freertos/semphr.h>

#include "vesync_common.h"
#include "vesync_os_private.h"
#include "vesync_sem.h"

int vesync_sem_new(vesync_sem_t *sem)
{
	SemaphoreHandle_t sem_count = xSemaphoreCreateCounting(VESYNC_OS_SEM_MAX_COUNT, 0);
    sem->handle = sem_count;
	return 0;
}

int vesync_sem_wait(vesync_sem_t *sem, unsigned int timeout)
{
	NULL_PARAM_CHK(sem);
    int ret = xSemaphoreTake(sem->handle, timeout == VESYNC_OS_WAIT_FOREVER ? portMAX_DELAY : ms2tick(ms));
    return ret == pdPASS ? 0 : -1;
}

int vesync_sem_signal(vesync_sem_t *sem)
{
	NULL_PARAM_CHK(sem);
	xSemaphoreGive(sem->handle);
	return 0;
}

int vesync_sem_free(vesync_sem_t *sem)
{
	NULL_PARAM_CHK(sem);
	vSemaphoreDelete(sem->handle);
	return 0;
}

