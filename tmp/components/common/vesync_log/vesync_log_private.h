/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_LOG_INTERNAL_H__
#define __VESYNC_LOG_INTERNAL_H__

#include "vesync_log.h"
#include "vesync_mutex.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ENABLE_LOG_SEM

#define VESYNC_LOG_BUFF_MAX	(1536)

typedef struct _map_tbl
{
	TYPE_COMPONENT_E comp;
	char comp_str[4];
	LOG_LEVEL_E level;
	char level_str[2];
} str_map_tbl_t;


typedef struct _vesync_log
{
	LOG_LEVEL_E level;
	LOG_LEVEL_E raw_level;
#ifdef ENABLE_LOG_SEM
	vesync_mutex_t mutex;
#endif
	char log_buf[VESYNC_LOG_BUFF_MAX];

	print_cb_t print_cb;
	log_transfer_cb_t transfer_cb;
} vesync_log_t;


/* SDK log API */
#define SDK_LOG(level, format,...) 													\
	do 																					\
	{																					\
		vesync_log_print(COMP_SDK, level, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); 	\
	}while(0)


LOG_LEVEL_E vesync_log_level_get(void);
LOG_LEVEL_E vesync_log_raw_level_get(void);	

#ifdef __cplusplus
}
#endif

#endif

