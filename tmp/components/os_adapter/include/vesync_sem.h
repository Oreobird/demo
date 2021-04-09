/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_SEM_H__
#define __VESYNC_SEM_H__

#include "vesync_os.h"

#ifdef __cplusplus
extern "C" {
#endif

int vesync_sem_new(vesync_sem_t *sem);

int vesync_sem_wait(vesync_sem_t *sem, unsigned int timeout);

int vesync_sem_signal(vesync_sem_t *sem);

int vesync_sem_free(vesync_sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif


