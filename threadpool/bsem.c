#include <stdlib.h>
#include <pthread.h>
#include "bsem.h"

void bsem_init(bsem_t *p_sem, int val)
{
    if (val < 0 || val > 1)
    {
        exit(1);
    }

    pthread_mutex_init(&(p_sem->mutex), NULL);
    pthread_cond_init(&(p_sem->cond), NULL);
    p_sem->val = val;
}

void bsem_reset(bsem_t *p_sem)
{
    bsem_init(p_sem, 0);
}

void bsem_post(bsem_t *p_sem)
{
    pthread_mutex_lock(&p_sem->mutex);
    p_sem->val = 1;
    pthread_cond_signal(&p_sem->cond);
    pthread_mutex_unlock(&p_sem->mutex);
}

void bsem_post_all(bsem_t *p_sem)
{
    pthread_mutex_lock(&p_sem->mutex);
    p_sem->val = 1;
    pthread_cond_broadcast(&p_sem->cond);
    pthread_mutex_unlock(&p_sem->mutex);
}

void bsem_wait(bsem_t *p_sem)
{
    pthread_mutex_lock(&p_sem->mutex);
    while (p_sem->val != 1)
    {
        pthread_cond_wait(&p_sem->cond, &p_sem->mutex);
    }
    p_sem->val = 0;
    pthread_mutex_unlock(&p_sem->mutex);
}
