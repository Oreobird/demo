#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include "bsem.h"

typedef struct _task
{
    struct _task *prev;
    void (*task_fn)(void *arg);
    void *arg;
} task_t;

typedef struct _task_queue
{
    pthread_mutex_t rwmutex;
    task_t *front;
    task_t *rear;
    bsem_t *has_task;
    int len;
} task_queue_t;

int task_queue_init(task_queue_t *p_task_q);
void task_queue_clear(task_queue_t *p_task_q);
void task_queue_push(task_queue_t *p_task_q, task_t *p_task);
task_t *task_queue_pull(task_queue_t *p_task_q);
void task_queue_destroy(task_queue_t *p_task_q);

#endif
