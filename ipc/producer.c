#include <signal.h>
#include "shmer.h"

void ignore_signal(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGSTOP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void msg_produce(char *msg, int len)
{
	if (msg == NULL || len <= 0)
	{
		return ERR;
	}

	snprintf(msg, len, "I am producer");
	msg[len - 1] = '\0';
}

int main(int argc, char **argv)
{
	char msg[SHM_BUF_SIZE] = {0};
	shmer_t *shmer = NULL;
	int ret = ERR;
	
	ignore_signal();
	shmer = shmer_shm_setup('.');

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

	msg_produce(msg, sizeof(msg));
	
	ret = shmer_write(shmer, msg, sizeof(msg));
	if (ret == ERR)
	{
		return ERR;
	}

	
	return OK;
}
