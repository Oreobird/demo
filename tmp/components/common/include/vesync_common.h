/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_COMMON_H__
#define __VESYNC_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED(x) ((void)(x))

#define NULL_PARAM_CHK(param) \
    do {                      \
        if (param == NULL) {  \
            return -1;        \
        }                     \
    } while (0)
    
#define NULL_RET_CHK(ret)   \
	do {					\
		if (ret == NULL) {  \
			return -1;		\
		}					\
	} while (0)
			
#define ERR_RET_CHK(ret)   \
	do {					\
		if (ret < 0) {	\
			return -1;		\
		}					\
	} while (0)

#define IN_RANGE_CHK(x, min, max)         \
	do {                                  \
		if ((x) < (min) || (x) > (max)) { \
			return -1;                    \
		}                                 \
	} while (0)
	
#ifdef __cplusplus
}
#endif

#endif


