/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <gtest/gtest.h>
#include "vesync_memory.h"

TEST(mm_test, malloc_zero)
{
	int *ptr = (int *) vesync_malloc(0);
	EXPECT_TRUE(ptr);
}

TEST(mm_test, calloc_free)
{
	const int COUNT = 1024;
	int i = 0;
	int *ptr = (int *) vesync_malloc(sizeof(int) * COUNT);
	EXPECT_TRUE(ptr);

	memset(ptr, 0, COUNT);

	for (i = 0; i < COUNT; i++)
	{
		*(ptr + i) = i;
	}

	for (i = 0; i < COUNT; i++)
	{
		ASSERT_EQ((int)*(ptr + i), i);
	}

	vesync_free(ptr);
}

