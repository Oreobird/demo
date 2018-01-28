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

    locker = locker_sem_create(".", 1);
#if defined(TEST_SHM)
    shmer = shmer_shm_create(".", locker);
#elif defined(TEST_MMAP)
	shmer = shmer_mmap_create('/dev/zero', locker);
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

