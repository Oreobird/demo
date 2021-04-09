/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>

#include "vesync_common.h"
#include "vesync_os.h"
#include "vesync_task.h"

#define VESYNC_TASK_NAME_MAX_LEN (32)

typedef struct _targ
{
    char name[VESYNC_TASK_NAME_MAX_LEN];
    void (*fn)(void *);
    void *arg;
} targ_t;

static void *task_entry(void *arg)
{
	targ_t *targ = arg;
	void (*fn)(void *) = targ->fn;
	void *farg = targ->arg;

	if (targ->name != NULL)
	{
		prctl(PR_SET_NAME, (unsigned long)targ->name, 0, 0, 0);
	}
	
	free(targ);
	fn(farg);

	return 0;
}

int vesync_task_new(const char *name, 
						  void (*fn)(void *),
						  void *arg,
						  int stack_size,
						  int prio,
						  vesync_task_t *task_handle)
{
	UNUSED(stack_size);
	UNUSED(prio);
	int ret = -1;
	pthread_t task_id;

	if (fn == NULL)
	{
		return -1;
	}
	
	targ_t *targ = malloc(sizeof(*targ));
	if (targ == NULL)
	{
		return -1;
	}

	if (name != NULL)
	{
		strncpy(targ->name, name, sizeof(targ->name) - 1);
	}
	
	targ->fn = fn;
	targ->arg = arg;
	
	ret = pthread_create(&task_id, NULL, task_entry, targ);
	if (ret == 0)
	{
		ret = pthread_detach(task_id);
	}
	else
	{
		free(targ);
	}

	if (task_handle)
	{
		task_handle->handle = (pthread_t*)task_id;
	}
	
	return ret;
}

void vesync_task_exit(long code)
{
	pthread_exit((void*)code);
}

const char *vesync_task_name(void)
{
	static char name[VESYNC_TASK_NAME_MAX_LEN];
	prctl(PR_GET_NAME, (unsigned long)name, 0, 0, 0);
	return name;
}
