#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "bsem.h"
#include "thread.h"
#include "task_queue.h"
#include "thread_pool.h"

volatile int threads_on_hold;
volatile int threads_keepalive;

thpool_t thpool_init(int threads_num)
{
    thpool_t p_thpool = NULL;
    int i;

    threads_on_hold = 0;
    threads_keepalive = 1;

    if (threads_num < 0)
    {
        threads_num = 0;
    }

    p_thpool = (thpool_t)malloc(sizeof(thread_pool_t));
    if (!p_thpool)
    {
        return NULL;
    }

    p_thpool->alive_num = 0;
    p_thpool->working_num = 0;

    if (task_queue_init(&p_thpool->task_queue) < 0)
    {
        free(p_thpool);
        return NULL;
    }

    p_thpool->threads = (thread_t**)malloc(threads_num * sizeof(thread_t *));
    if (p_thpool->threads == NULL)
    {
        task_queue_destroy(&p_thpool->task_queue);
        free(p_thpool);
        return NULL;
    }

    pthread_mutex_init(&p_thpool->count_lock, NULL);
    pthread_cond_init(&p_thpool->all_idle, NULL);

    for (i = 0; i < threads_num; i++)
    {
        thread_init(&p_thpool->threads[i], p_thpool, i);
    }

    while (p_thpool->alive_num != threads_num) {}

    return p_thpool;
}

int thpool_add_task(thpool_t p_thpool, void (*task_fn)(void *), void *arg)
{
    task_t *p_task = NULL;

    p_task = (task_t *)malloc(sizeof(task_t));
    if (p_task == NULL)
    {
        return -1;
    }

    p_task->task_fn = task_fn;
    p_task->arg = arg;

    task_queue_push(&p_thpool->task_queue, p_task);

    return 0;
}

void thpool_wait(thpool_t p_thpool)
{
    pthread_mutex_lock(&p_thpool->count_lock);
    while (p_thpool->task_queue.len || p_thpool->working_num)
    {
        pthread_cond_wait(&p_thpool->all_idle, &p_thpool->count_lock);
    }
    pthread_mutex_unlock(&p_thpool->count_lock);
}

void thpool_destroy(thpool_t thiz)
{
    if (thiz == NULL)
        return;

    volatile int threads_total = thiz->alive_num;
    threads_keepalive = 0;

    double TIMEOUT = 1.0;
    time_t start, end;
    double tpassed = 0.0;
    time(&start);
    while (tpassed < TIMEOUT && thiz->alive_num)
    {
        bsem_post_all(thiz->task_queue.has_task);
        time(&end);
        tpassed = difftime(end, start);
    }

    while (thiz->alive_num)
    {
        bsem_post_all(thiz->task_queue.has_task);
        sleep(1);
    }

    task_queue_destroy(&thiz->task_queue);

    int i;
    for (i = 0; i < threads_total; i++)
    {
        thread_destroy(thiz->threads[i]);
    }
    free(thiz->threads);
    free(thiz);
}

void thpool_pause(thpool_t thiz)
{
    int i;
    for (i = 0; i < thiz->alive_num; i++)
    {
        pthread_kill(thiz->threads[i]->pthread, SIGUSR1);
    }
}

void thpool_resume(thpool_t thiz)
{
    //TODO
    (void)thiz;
    threads_on_hold = 0;
}

int thpool_num_threads_working(thpool_t thiz)
{
    return thiz->working_num;
}

