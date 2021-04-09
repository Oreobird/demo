/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_TASK_H__
#define __VESYNC_TASK_H__

#include "vesync_os.h"

#ifdef __cplusplus
extern "C" {
#endif

int vesync_task_new(const char *name, 
						  void (*fn)(void *),
						  void *arg,
						  int stack_size,
						  int prio,
						  vesync_handle_t *task_handle);

void vesync_task_exit(long code);

const char *vesync_task_name(void);


#ifdef __cplusplus
}
#endif

#endif

