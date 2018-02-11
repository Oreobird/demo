#include <signal.h>

#include "locker.h"
#include "shmer.h"
#include "sem_posix.h"
#include "sem_sysv.h"
#include "shm_com.h"
#include "mmap_com.h"

void ignore_signal(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

int msg_produce(char *msg, int len)
{
	if (msg == NULL || len <= 0)
	{
		return ERR;
	}

    if (fgets(msg, len, stdin) == NULL)
    {
        return ERR;
    }

	msg[len - 1] = '\0';
    return OK;
}

int main(void)
{
    char msg[SHM_BUF_SIZE] = {0};
    shmer_t *shmer = NULL;
    locker_t *locker = NULL;
    int ret = ERR;
    int init_value = 0;

    ignore_signal();

#if defined(AS_MUTEX)
    init_value = 1;
#endif

#if defined(SYSV_SEM)
    locker = locker_sem_sysv_create(".", init_value);
#elif defined(POSIX_SEM)
    locker = locker_sem_posix_create("my_posix_sem", init_value);
#endif

#if defined(TEST_SHM)
    shmer = shmer_shm_create(".", locker);
#elif defined(TEST_MMAP)
    shmer = shmer_mmap_create("/home/zgs/github/demo/ipc/mymap", locker);
#endif
    if (shmer == NULL)
    {
        locker_destroy(locker);
        return ERR;
    }

    do
    {
        ret = msg_produce(msg, sizeof(msg));
        if (ret == ERR)
        {
            break;
        }

        #if defined(AS_MUTEX)
        ret = shmer_write(shmer, msg, sizeof(msg));
        #else
        ret = shmer_sync_write(shmer, msg, sizeof(msg));
        #endif
        if (ret == ERR)
        {
            break;
        }
    } while (strncmp(msg, "quit", 4) != 0);

    shmer_destroy(shmer);

    return OK;
}
