/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <gtest/gtest.h>
#include "vesync_os.h"
#include "vesync_task.h"
#include "vesync_mutex.h"
#include "vesync_sem.h"

static vesync_mutex_t g_mutex;

static vesync_sem_t g_sem;

static void mutex_lock_test_task1(void *arg)
{
	int i = 0;
	int ret = -1;
	int *pcount = (int *)arg;

	for (i = 0; i < 100; i++)
	{
		ret = vesync_mutex_lock(&g_mutex, VESYNC_OS_WAIT_FOREVER);
		EXPECT_EQ(ret, 0);
		
		(*pcount)++;

		ret = vesync_mutex_unlock(&g_mutex);
		EXPECT_EQ(ret, 0);
	}

	ret = vesync_sem_signal(&g_sem);
	EXPECT_EQ(ret, 0);
	
	vesync_task_exit(0);
}

static void mutex_lock_test_task2(void *arg)
{
	int i = 0;
	int ret = -1;
	int *pcount = (int *)arg;

	for (i = 0; i < 100; i++)
	{
		ret = vesync_mutex_lock(&g_mutex, VESYNC_OS_WAIT_FOREVER);
		EXPECT_EQ(ret, 0);
		
		(*pcount)--;

		ret = vesync_mutex_unlock(&g_mutex);
		EXPECT_EQ(ret, 0);
	}

	ret = vesync_sem_signal(&g_sem);
	EXPECT_EQ(ret, 0);
	
	vesync_task_exit(0);
}

TEST(mutex_test, mutex_lock)
{
	int ret = -1;
	ret = vesync_mutex_new(&g_mutex);
	EXPECT_EQ(ret, 0);

	ret = vesync_sem_new(&g_sem);
	EXPECT_EQ(ret, 0);

	int count = 0;
	ret = vesync_task_new("mutex_lock_test_task1", mutex_lock_test_task1, &count, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);
	
	ret = vesync_task_new("mutex_lock_test_task2", mutex_lock_test_task2, &count, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);
	
	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);
	
	vesync_mutex_free(&g_mutex);
	ret = vesync_sem_free(&g_sem);
	EXPECT_EQ(ret, 0);
	
	EXPECT_EQ(count, 0);
}

#if 0
static vesync_mutex_t g_mutex1;

static void mutex_deadlock_test_task1(void *arg)
{
	int ret = -1;
	ret = vesync_mutex_lock(&g_mutex, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);

	ret = vesync_mutex_lock(&g_mutex1, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);

	vesync_mutex_unlock(&g_mutex);
	vesync_sem_signal(&g_sem);
}

static void mutex_deadlock_test_task2(void *arg)
{
	int ret = -1;
	ret = vesync_mutex_lock(&g_mutex1, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);

	ret = vesync_mutex_lock(&g_mutex, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);

	vesync_mutex_unlock(&g_mutex1);
	vesync_sem_signal(&g_sem);
}


TEST(mutex_test, mutex_deadlock)
{
	
	int ret = -1;
	ret = vesync_mutex_new(&g_mutex);
	EXPECT_EQ(ret, 0);

	ret = vesync_mutex_new(&g_mutex1);
	EXPECT_EQ(ret, 0);

	ret = vesync_sem_new(&g_sem);
	EXPECT_EQ(ret, 0);

	ret = vesync_task_new("mutex_lock_test_task1", mutex_deadlock_test_task1, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);
	
	ret = vesync_task_new("mutex_lock_test_task2", mutex_deadlock_test_task2, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);
	
	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);
	
	vesync_mutex_free(&g_mutex);
	vesync_sem_free(&g_sem);
}
#endif

