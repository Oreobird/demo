/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */
 
#include <stdlib.h>
#include "vesync_memory.h"

void *vesync_malloc(uint32_t size) 
{
	return malloc(size);
}

void *vesync_calloc(uint32_t nmemb, uint32_t size)
{
	return calloc(nmemb, size);
}

void *vesync_realloc(void *ptr, uint32_t size)
{
	return realloc(ptr, size);
}

void vesync_free(void *ptr)
{
	if (ptr)
	{
		free(ptr);
		ptr = NULL;
	}

}

