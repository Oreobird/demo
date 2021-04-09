/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <unistd.h>
#include <gtest/gtest.h>
#include "vesync_log_private.h"
#include "vesync_task.h"
#include "vesync_sem.h"

static vesync_sem_t g_sem;

TEST(log_test, log_init)
{
	int ret = -1;
	ret = vesync_log_init(LOG_DEBUG);
	EXPECT_EQ(ret, 0);

	vesync_log_deinit();
}

TEST(log_test, log_level_set)
{
	int ret = -1;
	ret = vesync_log_init(LOG_DEBUG);
	EXPECT_EQ(ret, 0);

	vesync_log_level_set(LOG_ERROR);
	ret = vesync_log_level_get();
	EXPECT_EQ(ret, LOG_ERROR);
	
	vesync_log_raw_level_set(LOG_DEBUG);
	ret = vesync_log_raw_level_get();
	EXPECT_EQ(ret, LOG_DEBUG);
	
	vesync_log_deinit();
}

TEST(log_test, log_print)
{
	int ret = -1;
	ret = vesync_log_init(LOG_DEBUG);
	EXPECT_EQ(ret, 0);
	
	vesync_log_level_set(LOG_DEBUG);
	ret = vesync_log_level_get();
	EXPECT_EQ(ret, LOG_DEBUG);

	APP_LOG(LOG_INFO, "Test APP_LOG\n");
	OS_LOG(LOG_DEBUG, "Test OS_LOG\n");
	HAL_LOG(LOG_WARN, "Test HAL_LOG\n");
	SDK_LOG(LOG_ERROR, "Test SDK_LOG\n");

	vesync_log_deinit();
}

TEST(log_test, log_hex_print)
{
	int ret = -1;
	ret = vesync_log_init(LOG_DEBUG);
	EXPECT_EQ(ret, 0);
	
	char hex[10];
	for (int i = 0; i < (int)sizeof(hex); i++)
	{
		hex[i] = (char)i;
	}
	
	LOG_RAW_HEX(LOG_INFO, "hex:", hex, sizeof(hex));

	vesync_log_deinit();
}

static int log_transfer_cb(char* buf, int len)
{
	//printf("[log_transfer_cb] len=%d, buf:%s\n", len, buf);
	return 0;
}

static void log_test_task1(void *arg)
{
	int i = 0;
	int ret = -1;
	for (i = 0; i < 10; i++)
	{
		SDK_LOG(LOG_INFO, "log_test_task1: multi task test i=%d\n", i);
	}

	ret = vesync_sem_signal(&g_sem);
	EXPECT_EQ(ret, 0);
	
	vesync_task_exit(0);
}

static void log_test_task2(void *arg)
{
	int i = 0;
	int ret = -1;
	for (i = 0; i < 10; i++)
	{
		APP_LOG(LOG_INFO, "log_test_task2: multi task test i=%d\n", i);
	}

	ret = vesync_sem_signal(&g_sem);
	EXPECT_EQ(ret, 0);
	
	vesync_task_exit(0);
}

TEST(log_test, log_multi_task_print)
{
	int ret = -1;
	ret = vesync_log_init(LOG_DEBUG);
	EXPECT_EQ(ret, 0);

	vesync_log_level_set(LOG_DEBUG);

	vesync_log_transfer_cb_register(log_transfer_cb);
	
	ret = vesync_sem_new(&g_sem);
	EXPECT_EQ(ret, 0);

	ret = vesync_task_new("log_test_task1", log_test_task1, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);
	
	ret = vesync_task_new("log_test_task2", log_test_task2, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);
	
	vesync_sem_wait(&g_sem, VESYNC_OS_WAIT_FOREVER);
	EXPECT_EQ(ret, 0);

	ret = vesync_sem_free(&g_sem);
	EXPECT_EQ(ret, 0);

	vesync_log_deinit();
}

