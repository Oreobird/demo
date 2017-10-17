#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"

typedef struct fifo_ring_s {
    int r_cursor;
    int w_cursor;
    int length;
    void *data[0];
} fifo_ring_t;

fifo_ring_t *fifo_ring_create(int length)
{
    fifo_ring_t *thiz = NULL;

    if (length <= 1)
    {
        return NULL;
    }

    thiz = (fifo_ring_t *)malloc(sizeof(fifo_ring_t) + length * sizeof(void *));
    if (thiz != NULL)
    {
        thiz->r_cursor = 0;
        thiz->w_cursor = 0;
        thiz->length = length;
    }

    return thiz;
}

void fifo_ring_destroy(fifo_ring_t *thiz)
{
    SAFE_FREE(thiz);
}

int fifo_ring_push(fifo_ring_t *thiz, void *data)
{
    int w_cursor = 0;

    if (thiz == NULL)
    {
        return -1;
    }

    w_cursor = (thiz->w_cursor + 1) % thiz->length;
    if (w_cursor != thiz->r_cursor)
    {
        thiz->data[thiz->w_cursor] = data;
        thiz->w_cursor = w_cursor;
    }

    return 0;
}

int fifo_ring_pop(fifo_ring_t *thiz, void **data)
{
    if (thiz == NULL || data == NULL)
    {
        return -1;
    }

    if (thiz->r_cursor != thiz->w_cursor)
    {
        *data = thiz->data[thiz->r_cursor];
        thiz->r_cursor = (thiz->r_cursor + 1) % thiz->length;
    }

    return 0;
}

#ifdef TEST_FIFO_RING
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#define NR 10000

static void *push_thread(void *arg)
{
    fifo_ring_t *fifo_ring = NULL;
    int i = 0;
    int ret = -1;

    fifo_ring = (fifo_ring_t *)arg;
    if (fifo_ring == NULL)
    {
        return NULL;
    }

    printf("push thread create ok\n");

    while (i < NR)
    {
        ret = fifo_ring_push(fifo_ring, (void *)i);
        if (ret < 0)
        {
            usleep(10);
            continue;
        }

        printf("push data:%d\n", i);
        i++;
    }

    return NULL;
}

static void *pop_thread(void *arg)
{
    fifo_ring_t *fifo_ring = NULL;
    void *data = 0;
    int i = 0;
    int ret = -1;

    fifo_ring = (fifo_ring_t *)arg;
    if (fifo_ring == NULL)
    {
        return NULL;
    }

    printf("pop thread create ok\n");

    while (i < NR)
    {
        ret = fifo_ring_pop(fifo_ring, (void **)&data);
        if (ret < 0)
        {
            usleep(10);
            continue;
        }

        printf("pop data:%d\n", (int)data);
        i++;
    }

    return NULL;
}

void push_pop_test(int fifo_length)
{
    pthread_t push_tid = -1;
    pthread_t pop_tid = -1;
    fifo_ring_t *fifo_ring = NULL;
    int ret = -1;

    fifo_ring = fifo_ring_create(fifo_length);
    if (fifo_ring == NULL)
    {
        printf("fifo_create failed\n");
        return;
    }

    ret = pthread_create(&push_tid, NULL, (void *)push_thread, (void *)fifo_ring);
    if (ret < 0)
    {
        printf("push thread create failed\n");
    }

    ret = pthread_create(&pop_tid, NULL, (void *)pop_thread, (void *)fifo_ring);
    if (ret < 0)
    {
        printf("pop thread create failed\n");
    }

    pthread_join(push_tid, NULL);
    pthread_join(pop_tid, NULL);

    fifo_ring_destroy(fifo_ring);
}

int main(int argc, char **argv)
{
    push_pop_test(20);
    push_pop_test(200);
    push_pop_test(2000);
    return 0;
}
#endif
