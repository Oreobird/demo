/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_OS_H__
#define __VESYNC_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

//TODO: Move it to vesync_os_internal.h ???
#define VESYNC_OS_WAIT_FOREVER (0xFFFFFFFFu)
#define VESYNC_OS_NO_WAIT	(0x0)
#define VESYNC_OS_SEM_MAX_COUNT (64)
//end TODO

typedef struct _vesync_hanndle_t{
    void *handle;
} vesync_handle_t;

typedef vesync_handle_t vesync_task_t;
typedef vesync_handle_t vesync_mutex_t;
typedef vesync_handle_t vesync_sem_t;
typedef vesync_handle_t vesync_queue_t;
typedef vesync_handle_t vesync_timer_t;


#ifdef __cplusplus
}
#endif

#endif

