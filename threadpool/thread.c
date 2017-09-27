#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>
#include "task_queue.h"
#include "thread.h"
#include "thread_pool.h"

extern int threads_on_hold;
extern int threads_keepalive;

int thread_init(thread_t **thiz, thpool_t p_thpool, int id)
{
    *thiz = (thread_t *)malloc(sizeof(thread_t));
    if (thiz == NULL)
    {
        return -1;
    }

    (*thiz)->p_thpool = p_thpool;
    (*thiz)->id = id;

    pthread_create(&(*thiz)->pthread, NULL, (void *)thread_do, (*thiz));
    pthread_detach((*thiz)->pthread);
    return 0;
}

void thread_hold(int sig_id)
{
    (void)sig_id;
    threads_on_hold = 1;
    while (threads_on_hold)
    {
        sleep(1);
    }
}

void *thread_do(thread_t *thiz)
{
    char thread_name[128] = {0};
    sprintf(thread_name, "thread-pool-%d", thiz->id);

    prctl(PR_SET_NAME, thread_name);

    thpool_t p_thpool = thiz->p_thpool;

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = thread_hold;
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        printf("%s: cannot handle SIGUSR1\n", __func__);
    }

    pthread_mutex_lock(&p_thpool->count_lock);
    p_thpool->alive_num++;
    pthread_mutex_unlock(&p_thpool->count_lock);

    while (threads_keepalive)
    {
        bsem_wait(p_thpool->task_queue.has_task);
        if (threads_keepalive)
        {
            pthread_mutex_lock(&p_thpool->count_lock);
            p_thpool->working_num++;
            pthread_mutex_unlock(&p_thpool->count_lock);

            void (*task_fn)(void *);
            void *arg;
            task_t *p_task = task_queue_pull(&p_thpool->task_queue);
            if (p_task)
            {
                task_fn = p_task->task_fn;
                arg = p_task->arg;
                task_fn(arg);
                free(p_task);
            }

            pthread_mutex_lock(&p_thpool->count_lock);
            p_thpool->working_num--;
            if (!p_thpool->working_num)
            {
                pthread_cond_signal(&p_thpool->all_idle);
            }
            pthread_mutex_unlock(&p_thpool->count_lock);
        }
    }

    pthread_mutex_lock(&p_thpool->count_lock);
    p_thpool->alive_num--;
    pthread_mutex_unlock(&p_thpool->count_lock);
    return NULL;
}

void thread_destroy(thread_t *thiz)
{
    free(thiz);
}
