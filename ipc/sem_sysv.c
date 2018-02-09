#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "sem_sysv.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

typedef struct _privinfo
{
    key_t key;
    int sem_id;
    int init_value;
} privinfo_t;

static int sem_sysv_init(locker_t *thiz)
{
	int sem_id = -1;
    int ret = -1;
	privinfo_t *priv = NULL;
    union semun sem_union;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;

	sem_id = semget(priv->key, 1, 0666 | IPC_CREAT);
	if (sem_id == -1)
	{
		return ERR;
	}

	priv->sem_id = sem_id;
    sem_union.val = priv->init_value;
    ret = semctl(sem_id, 0, SETVAL, sem_union);
    if (ret < 0)
    {
        return ERR;
    }

	return OK;
}

static int sem_sysv_del(locker_t *thiz)
{
	int ret = -1;
	union semun sem_union;
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

static void sem_sysv_destroy(locker_t *thiz)
{
	if (thiz)
	{
		sem_sysv_del(thiz);
		free(thiz);
		thiz = NULL;
	}
}

static int sem_sysv_p(locker_t *thiz)
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

static int sem_sysv_v(locker_t *thiz)
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

locker_t *locker_sem_sysv_create(const char *fname, int init_value)
{
	locker_t *thiz = (locker_t *)malloc(sizeof(locker_t) + sizeof(privinfo_t));
	if (thiz != NULL)
	{
		privinfo_t *priv = (privinfo_t *)thiz->priv;

		thiz->destroy = sem_sysv_destroy;
		thiz->lock = sem_sysv_p;
		thiz->unlock = sem_sysv_v;

		priv->key = ftok(fname, 'a');
		priv->sem_id = -1;
        priv->init_value = init_value;

		int ret = sem_sysv_init(thiz);
		if (ret == ERR)
		{
			sem_sysv_destroy(thiz);
			return NULL;
		}
	}

	return thiz;
}
