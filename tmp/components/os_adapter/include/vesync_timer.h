/*
 * Copyright © Vesync Technologies Co.Ltd. 2019-2021. All rights reserved.
 */

#ifndef __VESYNC_TIMER_H__
#define __VESYNC_TIMER_H__

#include "vesync_os.h"

#ifdef __cplusplus
extern "C" {
#endif

int vesync_timer_new(vesync_timer_t *timer, 
					 	  	void (*cb)(void*),
					 		void *cb_arg,
					 		long timeout_ms,
					 		int type);

int vesync_timer_start(vesync_timer_t *timer);

int vesync_timer_stop(vesync_timer_t *timer);

int vesync_timer_free(vesync_timer_t *timer);


#ifdef __cplusplus
}
#endif

#endif

