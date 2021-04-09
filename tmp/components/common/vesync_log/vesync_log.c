/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vesync_common.h"
#include "vesync_memory.h"
#include "vesync_log_private.h"


static vesync_log_t *s_log = NULL;

static str_map_tbl_t map_tbl[] = {
	{COMP_HAL, "HAL", LOG_DEBUG, "D"},
	{COMP_OS,  "OS",  LOG_INFO,  "I"},
	{COMP_SDK, "SDK", LOG_WARN,  "W"},
	{COMP_APP, "APP", LOG_ERROR, "E"}
};

int vesync_log_init(LOG_LEVEL_E level)
{
	// singleton
	if (s_log != NULL)
	{
		return 0;
	}
	
	int ret = -1;
	s_log = vesync_malloc(sizeof(vesync_log_t));
	NULL_RET_CHK(s_log);

	s_log->level = level;
	s_log->raw_level = level;
	s_log->print_cb = printf; // 回调函数，可以通过参数传递进来，由APP/HAL进行注册 ???
	s_log->transfer_cb = NULL;
	
#ifdef ENABLE_LOG_SEM
	ret = vesync_mutex_new(&(s_log->mutex));
	if (ret < 0)
	{
		vesync_free(s_log);
		return -1;
	}
#endif

	return 0;
}

void vesync_log_deinit(void)
{
	if (s_log)
	{
		#ifdef ENABLE_LOG_SEM
		vesync_mutex_free(&(s_log->mutex));
		#endif
		vesync_free(s_log);
		s_log = NULL;
	}
}

void vesync_log_level_set(LOG_LEVEL_E level)
{
	s_log->level = level;
}

void vesync_log_raw_level_set(LOG_LEVEL_E level)
{
	s_log->raw_level = level;
}

LOG_LEVEL_E vesync_log_level_get(void)
{
	return s_log != NULL ? s_log->level : LOG_INVALID;
}

LOG_LEVEL_E vesync_log_raw_level_get(void)
{
	return s_log != NULL ? s_log->raw_level : LOG_INVALID;
}

void vesync_log_print(TYPE_COMPONENT_E comp, LOG_LEVEL_E level, const char *func, int line, const char *format, ...)
{
	if (s_log == NULL || s_log->print_cb == NULL)
	{
		//printf("print function is not registered!!\n");
		return;
	}

	if (level >= s_log->level && LOG_DISABLE != s_log->level)
	{
		va_list arg_ptr;

#ifdef ENABLE_LOG_SEM
		vesync_mutex_lock(&(s_log->mutex), 2000);
#endif

		memset(s_log->log_buf, 0, sizeof(s_log->log_buf));

		char *str = s_log->log_buf;

		//format参数为flash中的变量
		//char fmt[VESYNC_LOG_BUFF_MAX];
		//memcpy(fmt, format, VESYNC_LOG_BUFF_MAX);			//使用内存中的数组fmt拷贝定义在flash中的数组format
		va_start(arg_ptr, format);						//获取函数可变参列表的起始位置，注意使用的是format，非fmt
		vsnprintf(str, VESYNC_LOG_BUFF_MAX - 1, format, arg_ptr);	//格式化字符串，注意使用的是fmt，非format，ets_vsnprintf不能直接读取FLASH
		va_end(arg_ptr);

		if (s_log->transfer_cb)
		{
			s_log->transfer_cb(str, strlen(str));
		} 

		char *comp_str = "";
		char *level_str = "";
		for (int i = 0; i < sizeof(map_tbl)/sizeof(str_map_tbl_t); i++)
		{
			if (comp == map_tbl[i].comp)
			{
				comp_str = map_tbl[i].comp_str;
			}

			if (level == map_tbl[i].level)
			{
				level_str = map_tbl[i].level_str;
			}
		}

		// 实际打印函数，可以根据实际使用系统，由HAL/APP层进行注册
		if (s_log->print_cb)
		{
			s_log->print_cb("[%s %s %s:%d] %s", comp_str, level_str, func, line, str);
		}
		
#ifdef ENABLE_LOG_SEM
		vesync_mutex_unlock(&(s_log->mutex));
#endif
	}
}

void vesync_log_hex_print(LOG_LEVEL_E level, const char *p_prefix, void *p_data, unsigned short length)
{	
    if (s_log->print_cb == NULL || p_data == NULL)
    {
        //printf("print function is not registered!!\n");
        return;
    }

    if (level >= s_log->raw_level && LOG_DISABLE != s_log->raw_level)
    {
        if (NULL != p_prefix)
        {
            s_log->print_cb("%s ", p_prefix);
        }
		
        unsigned short hex_len = ((length > VESYNC_LOG_BUFF_MAX) ? VESYNC_LOG_BUFF_MAX : length);
        char *hex = (char *)vesync_malloc(hex_len);
        if (NULL == hex)
        {
            return;
        }
		
		memset(hex, 0, hex_len);
        memcpy(hex, p_data, hex_len);

        for (unsigned short i = 0; i < hex_len ; i++)
        {
            s_log->print_cb("%02X ", hex[i]);
        }
		
        s_log->print_cb("\n");
		
        vesync_free(hex);
    }
}

void vesync_log_transfer_cb_register(log_transfer_cb_t cb)
{
    s_log->transfer_cb = cb;
}

