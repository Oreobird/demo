/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <gtest/gtest.h>
#include <pthread.h>

#include "vesync_os.h"
#include "vesync_timer.h"
#include "vesync_sem.h"

static vesync_sem_t g_sem;

static void timeout_cb(void* arg)
{
	printf("timeout_cb\n");
	long timeout = *(long*)arg;
	printf("----timeout: %ldms\n", timeout);
	vesync_sem_signal(&g_sem);
}


TEST(timer_test, timer_new)
{
	int ret = 0;
	vesync_timer_t timer;
	long timeout = 500;

	ret = vesync_sem_new(&g_sem);
	EXPECT_EQ(ret, 0);
	
	ret = vesync_timer_new(&timer, timeout_cb, (void*)timeout, timeout, 1);
	EXPECT_EQ(ret, 0);

	ret = vesync_timer_start(&timer);
	EXPECT_EQ(ret, 0);

	printf("111\n");
	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	#if 0
	ret = vesync_timer_stop(&timer);
	EXPECT_EQ(ret, 0);
	#endif
	printf("222\n");

	ret = vesync_timer_free(&timer);
	EXPECT_EQ(ret, 0);

	vesync_sem_free(&g_sem);
}

