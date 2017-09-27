#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "thread.h"
#include "task_queue.h"

typedef struct _thread_pool {                                                                                                                                                                           
    struct _thread **threads;
    volatile int alive_num;
    volatile int working_num;
    pthread_mutex_t count_lock;
    pthread_cond_t all_idle;
    struct _task_queue task_queue;
} thread_pool_t;

typedef struct _thread_pool* thpool_t;

thpool_t thpool_init(int threads_num);
int thpool_add_task(thpool_t, void (*task_fn)(void *), void *arg); 
void thpool_wait(thpool_t);
void thpool_pause(thpool_t);
void thpool_resume(thpool_t);
void thpool_destroy(thpool_t);
int thpool_num_threads_working(thpool_t);

#ifdef __cpluscplus
}
#endif
#endif
