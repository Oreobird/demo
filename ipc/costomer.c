#include <signal.h>
#include "locker.h"
#include "shmer.h"

#include "sem_com.h"
#include "shm_com.h"

int main(void)
{
    char msg[SHM_BUF_SIZE] = {0};
    shmer_t *shmer = NULL;
    locker_t *locker = NULL;
    int ret = ERR;

    locker = locker_sem_setup(".", 1);
    shmer = shmer_shm_setup(".", locker);

    ret = shmer_create(shmer);
    if (ret == ERR)
    {
        return ERR;
    }

    ret = shmer_map(shmer);
    if (ret == ERR)
    {
        return ERR;
    }

    ret = shmer_read(shmer, msg);
    if (ret == ERR)
    {
        shmer_unmap(shmer);
        shmer_destroy(shmer);
        return ERR;
    }

    printf("Consumer msg: %s\n", msg);

    ret = shmer_unmap(shmer);
    if (ret == ERR)
    {
        shmer_destroy(shmer);
        return ERR;
    }

    shmer_unmap(shmer);
    shmer_destroy(shmer);
    return OK;
}

