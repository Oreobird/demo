/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <gtest/gtest.h>
#include "vesync_os.h"
#include "vesync_task.h"

static void task_new_test(void *arg)
{
	vesync_task_exit(0);
}

TEST(task_test, task_new)
{
	int ret = 0;
	ret = vesync_task_new(NULL, task_new_test, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	ret = vesync_task_new("task_new_test", NULL, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, -1);

	ret = vesync_task_new("task_new_test", task_new_test, NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	vesync_task_t task;
	ret = vesync_task_new("task_new_test", task_new_test, NULL, 1024, 0, &task);
	EXPECT_EQ(ret, 0);
	printf("task id: %d\n", *(int*)(task.handle));
}

static void task_getname_test(void *arg)
{
	EXPECT_TRUE(std::string(vesync_task_name()) == std::string("task_getname_test"));
	vesync_task_exit(0);
}

TEST(task_test, task_get_name)
{
	int ret = 0;
	ret = vesync_task_new("task_getname_test", 
						  task_getname_test,
						  NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	ret = vesync_task_new(NULL, 
						  task_getname_test,
						  NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);

	ret = vesync_task_new("", 
						  task_getname_test,
						  NULL, 1024, 0, NULL);
	EXPECT_EQ(ret, 0);
}

