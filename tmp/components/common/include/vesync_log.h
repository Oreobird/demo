/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_LOG_H__
#define __VESYNC_LOG_H__


#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief log level：ALL < DEBUG < INFO < WARN < ERROR < OFF
 */
typedef enum _LOG_LEVEL_E_
{
	LOG_INVALID = 0,    
	LOG_DEBUG   = 1,    
	LOG_INFO    = 2,    
	LOG_WARN    = 3,    
	LOG_ERROR   = 4,    
	LOG_DISABLE = 5,    /* disable log */
} LOG_LEVEL_E;

/*
 * @brief Log component flag
 */
typedef enum _TYPE_COMPONENT_E_
{
	COMP_HAL	= 0,	
	COMP_OS,	
	COMP_SDK,
	COMP_APP,			
} TYPE_COMPONENT_E;

/*
 * @brief Callback function of log print
 * @param[in] const char * format       format param 
              ...                variable param
 * @return typedef int (*
 */
typedef int (*print_cb_t)(const char *format, ...);


/*
 * @brief Callback function of translation for accessing to log
 *
 */
typedef int (*log_transfer_cb_t)(char*, int);


/*
 * @function vesync_log_init
 * @brief Log init
 * @param[in] LOG_LEVEL_E level  
 * @param[out] None
 * @return void
*/
int vesync_log_init(LOG_LEVEL_E level);


/*
 * @function vesync_log_deinit
 * @brief Log deinit
 * @return void
*/
void vesync_log_deinit(void);


/**
 * @brief  Register callback function of translation for accessing to log
 *
 * @param  cb: callback function
 */
void vesync_log_transfer_cb_register(log_transfer_cb_t cb);


/*
 * @function vesync_log_level_set
 * @brief Set log level
 * @param[in] LOG_LEVEL_E level  
 * @param[out] None
 * @return void
*/
void vesync_log_level_set(LOG_LEVEL_E level);


/*
 * @function vesync_log_raw_level_set
 * @brief  Set log level
 * @param[in] LOG_LEVEL_E level
 * @param[out] None
 * @return void
*/
void vesync_log_raw_level_set(LOG_LEVEL_E level);


/*
 * @function vesync_log_printf
 * @brief Log print
 * @param[in] TYPE_COMPONENT_E comp  
              LOG_LEVEL_E level   
              const char *func    
              int line            
              const char *format  
              ...                 
 * @return void
*/
void vesync_log_print(TYPE_COMPONENT_E comp, LOG_LEVEL_E level, const char *func, int line, const char *format, ...);


/*
 * @brief Log with hex format
 *
 * @param level: log level
 * @param p_prefix: log string prefix
 * @param p_data: log data
 * @param length: log data length
 * @return void
 */
void vesync_log_hex_print(LOG_LEVEL_E level, const char *p_prefix, void *p_data, unsigned short length);


/* log as hex format*/
#define LOG_RAW_HEX(level, prefix, buf, len) 											\
	do 																					\
	{																					\
		vesync_log_hex_print(level, prefix, buf, len); 									\
	}while(0)

/* HAL log API */
#define HAL_LOG(level, format,...) 														\
	do 																					\
	{																					\
		vesync_log_print(COMP_HAL, level, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); 	\
	}while(0)

/* OS log API */
#define OS_LOG(level, format,...) 														\
	do																					\
	{																					\
		vesync_log_print(COMP_OS, level, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);	\
	}while(0)

	
/* APP log API */
#define APP_LOG(level, format,...) 														\
	do 																					\
	{																					\
		vesync_log_print(COMP_APP, level, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); 	\
	}while(0)

#ifdef __cplusplus
	}
#endif

#endif


