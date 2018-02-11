#include <signal.h>
#include "locker.h"
#include "shmer.h"

#include "sem_sysv.h"
#include "sem_posix.h"
#include "shm_com.h"
#include "mmap_com.h"

int main(void)
{
    char msg[SHM_BUF_SIZE] = {0};
    shmer_t *shmer = NULL;
    locker_t *locker = NULL;
    int ret = ERR;

#if defined(SYSV_SEM)
    locker = locker_sem_sysv_create(".", 1);
#elif defined(POSIX_SEM)
    locker = locker_sem_posix_create("my_posix_sem", 1);
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
		ret = shmer_read(shmer, msg);
	    if (ret == ERR)
	    {
	        shmer_destroy(shmer);
	        return ERR;
	    }

	    printf("Consumer msg: %s\n", msg);
    } while (strncmp(msg, "quit", 4) != 0);

    shmer_destroy(shmer);

    return OK;
}
