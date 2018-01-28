#include <signal.h>

#include "locker.h"
#include "shmer.h"
#include "sem_com.h"
#include "shm_com.h"

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

    ignore_signal();

    locker = locker_sem_create(".", 1);
    shmer = shmer_shm_create(".", locker);

    do
    {
        ret = msg_produce(msg, sizeof(msg));
        if (ret == ERR)
        {
            break;
        }

        ret = shmer_write(shmer, msg, sizeof(msg));
        if (ret == ERR)
        {
            break;
        }
    } while (strncmp(msg, "quit", 4) != 0);

    shmer_destroy(shmer);

    return OK;
}
