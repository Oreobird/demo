#include <pthread.h>
#include <stdio.h>

#include "thread_pool.h"

void task1(void)
{
    printf("Thread #%u working on %s\n", (int)pthread_self(), __func__);
}

void task2(void)
{
    printf("Thread #%u working on %s\n", (int)pthread_self(), __func__);
}

int main(int argc, char **argv)
{
    printf("Making thread pool with 4 threads\n");
    thpool_t thpool = thpool_init(4);

    printf("Adding 40 tasks to thread pool\n");
    int i;
    for (i = 0; i < 20; i++)
    {
        thpool_add_task(thpool, (void*)task1, NULL);
        thpool_add_task(thpool, (void*)task2, NULL);
    }

    printf("Killing thread pool\n");
    thpool_destroy(thpool);

    return 0;
}
