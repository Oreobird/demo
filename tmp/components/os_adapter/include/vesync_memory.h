/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_MEMORY_H__
#define __VESYNC_MEMORY_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *vesync_malloc(uint32_t size);

void *vesync_calloc(uint32_t nmemb, uint32_t size);

void *vesync_realloc(void *ptr, uint32_t size);

void vesync_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
