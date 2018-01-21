#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "sem_com.h"

typedef struct _privinfo
{
	key_t key;
	int sem_id;	
} privinfo_t;


static int sem_init(int sem_id, int init_value)
{
	int ret = -1;
	union semun sem_union = {0};
	
	sem_union.val = init_value;
	
	ret = semctl(sem_id, 0, SETVAL, sem_union);
	if (ret < 0)
	{
		return ERR;
	}

	return OK;
}

static int sem_create(locker_t *thiz)
{
	int sem_id = -1;
	privinfo_t *priv = NULL;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;
	
	sem_id = semget(priv->key, 1, 0666 | IPC_CREAT);
	if (sem_id == -1)
	{
		return ERR;
	}

	priv->sem_id = sem_id;
	
	return sem_init(priv->sem_id, priv->init_value);
}

static int sem_del(locker_t *thiz)
{
	int ret = -1;
	union semun sem_union = {0};
	privinfo_t *priv = NULL;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;
	
	ret = semctl(priv->sem_id, 0, IPC_RMID, sem_union);
	if (ret < 0)
	{
		return ERR;
	}

	return OK;
}

static void sem_destroy(locker_t *thiz)
{
	if (thiz)
	{
		sem_del(thiz);
		free(thiz);
	}
}

static int sem_p(locker_t *thiz)
{
	int ret = -1;
	struct sembuf sem_b = {0};
	privinfo_t *priv = NULL;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;
	
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	
	ret = semop(priv->sem_id, &sem_b, 1);
	if (ret < 0)
	{
		return ERR;
	}

	return OK;
}

static int sem_v(locker_t *thiz)
{
	int ret = -1;
	struct sembuf sem_b = {0};
	privinfo_t *priv = NULL;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;
	
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	
	ret = semop(priv->sem_id, &sem_b, 1);
	if (ret < 0)
	{
		return ERR;
	}

	return OK;
}

locker_t *locker_sem_setup(char *fname, int init_value)
{
	locker_t *thiz = (locker_t *)malloc(sizeof(locker_t) + sizeof(privinfo_t));
	if (thiz != NULL)
	{
		privinfo_t *priv = (privinfo_t *)thiz->priv;

		thiz->create = sem_create;
		thiz->destroy = sem_destroy;
		thiz->lock = sem_p;
		thiz->unlock = sem_v;

		priv->key = ftok(fname, 'a');
		priv->sem_id = -1;
	}

	return thiz;
}
