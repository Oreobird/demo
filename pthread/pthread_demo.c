#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *fun1(void)
{
    while (1)
    {
        printf("this is fun 1\n");
        sleep(2);
    }
}

void *fun2(void)
{
    while (1)
    {
        printf("this is fun 2\n");
        sleep(5);
    }
}

int main(int argc, char **argv)
{
    pthread_t tid[2];
    int i = 0;

    pthread_create(&tid[0], NULL, (void*)fun1, (void *)NULL);
    pthread_create(&tid[2], NULL, (void*)fun2, (void *)NULL);

    for (i = 0; i < 2; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
