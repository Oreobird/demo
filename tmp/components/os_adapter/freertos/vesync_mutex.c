/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <freertos/semphr.h>

#include "vesync_common.h"
#include "vesync_os_private.h"
#include "vesync_mutex.h"


int vesync_mutex_new(vesync_mutex_t *mtx)
{
	SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    mtx->handle = mutex;
    return mutex != NULL ? 0 : -1;
}

int vesync_mutex_lock(vesync_mutex_t *mtx, unsigned int timeout)
{
	if (mtx) 
	{
        xSemaphoreTake(mtx->handle, timeout == VESYNC_OS_WAIT_FOREVER ? portMAX_DELAY : ms2tick(timeout));
    }
    return 0;
}

int vesync_mutex_unlock(vesync_mutex_t *mtx)
{
	if (mtx) 
	{
		xSemaphoreGive(mtx->handle);
	}
	return 0;
}

void vesync_mutex_free(vesync_mutex_t *mtx)
{
	vSemaphoreDelete(mtx->handle);
}

