/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_OS_INTERNAL_H__
#define __VESYNC_OS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/FreeRTOS.h>

#define ms2tick(ms) (((ms)+portTICK_PERIOD_MS-1)/portTICK_PERIOD_MS)


#ifdef __cplusplus
}
#endif

#endif


