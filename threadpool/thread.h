#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include "thread_pool.h"

typedef struct _thread {
    int id;
    pthread_t pthread;
    struct _thread_pool *p_thpool;
} thread_t;

int thread_init(thread_t **thiz, struct _thread_pool *p_thpool, int id);
void *thread_do(thread_t *thiz);
void thread_hold(int sig_id);
void thread_destroy(thread_t *thiz);

#endif
