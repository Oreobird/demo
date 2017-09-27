#include <stdlib.h>
#include <pthread.h>
#include "bsem.h"
#include "task_queue.h"

int task_queue_init(task_queue_t *p_task_q)
{
    p_task_q->len = 0;
    p_task_q->front = NULL;
    p_task_q->rear = NULL;

    p_task_q->has_task = (bsem_t *)malloc(sizeof(bsem_t));
    if (p_task_q->has_task == NULL)
    {
        return -1;
    }

    pthread_mutex_init(&p_task_q->rwmutex, NULL);
    bsem_init(p_task_q->has_task, 0);

    return 0;
}

void task_queue_clear(task_queue_t *p_task_q)
{
    while (p_task_q->len)
    {
        free(task_queue_pull(p_task_q));
    }
    p_task_q->front = NULL;
    p_task_q->rear = NULL;
    bsem_reset(p_task_q->has_task);
    p_task_q->len = 0;
}

void task_queue_push(task_queue_t *p_task_q, task_t *p_task)
{
    pthread_mutex_lock(&p_task_q->rwmutex);
    p_task->prev = NULL;

    if (p_task_q->len == 0)
    {
        p_task_q->front = p_task;
        p_task_q->rear = p_task;
    }
    else
    {
        p_task_q->rear->prev = p_task;
        p_task_q->rear = p_task;
    }

    p_task_q->len++;

    bsem_post(p_task_q->has_task);
    pthread_mutex_unlock(&p_task_q->rwmutex);
}

task_t *task_queue_pull(task_queue_t *p_task_q)
{
    pthread_mutex_lock(&p_task_q->rwmutex);
    task_t *p_task = p_task_q->front;

    if (p_task_q->len == 1)
    {
        p_task_q->front = NULL;
        p_task_q->rear = NULL;
        p_task_q->len = 0;
    }
    else if (p_task_q->len > 1)
    {
        p_task_q->front = p_task->prev;
        p_task_q->len--;
        bsem_post(p_task_q->has_task);
    }

    pthread_mutex_unlock(&p_task_q->rwmutex);
    return p_task;
}

void task_queue_destroy(task_queue_t *p_task_q)
{
    task_queue_clear(p_task_q);
    free(p_task_q->has_task);
}

