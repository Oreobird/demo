/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_MUTEX_H__
#define __VESYNC_MUTEX_H__

#include "vesync_os.h"

#ifdef __cplusplus
extern "C" {
#endif

int vesync_mutex_new(vesync_mutex_t *mtx);

int vesync_mutex_lock(vesync_mutex_t *mtx, unsigned int timeout);

int vesync_mutex_unlock(vesync_mutex_t *mtx);

void vesync_mutex_free(vesync_mutex_t *mtx);

#ifdef __cplusplus
}
#endif

#endif

