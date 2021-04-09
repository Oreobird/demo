/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

#include "vesync_timer.h"

typedef struct _timer_param
{
	long timeout_ms;
	int type;
} timer_param_t;

static timer_param_t s_tparam;

typedef void (*sigev_notify_fn)(union sigval);

int vesync_timer_new(vesync_timer_t *timer, 
						   void (*cb)(void*),
						   void *cb_arg,
						   long timeout_ms, 
						   int type)
{
	int ret = -1;
	struct sigevent sev;
	timer_t timer_id;
	
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_value.sival_ptr = cb_arg;
	sev.sigev_notify_function = (sigev_notify_fn)cb;
	sev.sigev_notify_attributes = NULL;

	s_tparam.timeout_ms = timeout_ms;
	s_tparam.type = type;
	
	ret = timer_create(CLOCK_REALTIME, &sev, &timer_id);
	if (ret != 0)
	{
		return -1;
	}

	if (timer)
	{
		timer->handle = timer_id;
		printf("new timer_id=%d\n", *(int*)timer_id);
	}
	
	return 0;
}
 
int vesync_timer_start(vesync_timer_t *timer)
{
	int ret = -1;
	struct itimerspec its;
	printf("start timer_id=%p\n", timer->handle);

	its.it_value.tv_sec = s_tparam.timeout_ms / 1000;
	its.it_value.tv_nsec = s_tparam.timeout_ms * 1000000;

	its.it_interval.tv_sec = (s_tparam.type == 0) ? its.it_value.tv_sec : 0;
	its.it_interval.tv_nsec = (s_tparam.type == 0) ? its.it_value.tv_nsec : 0;
	
	ret = timer_settime(timer->handle, TIMER_ABSTIME, &its, NULL);
	if (ret < 0)
	{
		return -1;
	}
	
	return 0;
}
 
int vesync_timer_stop(vesync_timer_t *timer)
{
	int ret = -1;
	struct itimerspec its;

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	
	ret = timer_settime(timer->handle, TIMER_ABSTIME, &its, NULL);
	if (ret < 0)
	{
		return -1;
	}
	
	return 0;
}
 
int vesync_timer_free(vesync_timer_t *timer)
{
	if (timer)
	{
		timer_delete(timer->handle);
	}
}


