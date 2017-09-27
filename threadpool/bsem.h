#ifndef __BSEM_H__
#define __BSEM_H__

typedef struct _bsem
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int val;
} bsem_t;

void bsem_init(bsem_t *p_sem, int val);
void bsem_reset(bsem_t *p_sem);
void bsem_post(bsem_t *p_sem);
void bsem_post_all(bsem_t *p_sem);
void bsem_wait(bsem_t *p_sem);

#endif
