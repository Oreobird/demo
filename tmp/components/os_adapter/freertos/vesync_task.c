/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "vesync_common.h"
#include "vesync_task.h"

#define VESYNC_TASK_NAME_MAX_LEN (16)

typedef struct _targ
{
    char name[VESYNC_TASK_NAME_MAX_LEN];
    void (*fn)(void *);
    void *arg;
} targ_t;


static void task_entry(void *arg)
{
    targ_t *targ = arg;
    void (*fn)(void *) = targ->fn;
    void *farg = targ->arg;
    free(targ);

    fn(farg);

    vTaskDelete(NULL);
}

int vesync_task_new(const char *name, 
						  void (*fn)(void *),
						  void *arg,
						  int stack_size,
						  int prio,
						  vesync_task_t *task_handle)
{
	BaseType_t ret;
	TaskHandle_t task;
	targ_t *targ = malloc(sizeof(targ_t));
	if (targ == NULL)
	{
		return -1;
	}

	targ->fn = fn;
	targ->arg = arg;
	
	ret = xTaskCreate(task_entry,
                      name,
                      stack_size/sizeof(portSTACK_TYPE),
                      NULL,
                      prio,
                      &task);
	
    if (ret != pdPASS)
    {
		free(targ);
		return -1;
    }

	if (task_handle)
	{
		task_handle->handle = task;
	}

	return 0;
}

void vesync_task_exit(long code)
{
	UNUSED(code);
	vTaskDelete(NULL);
}

const char *vesync_task_name(void)
{
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
    return task->pcTaskName;
}
