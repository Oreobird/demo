#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "sem_posix.h"

typedef struct _privinfo
{
    char name[20];
    sem_t *sem;
    int init_value;
} privinfo_t;

static int sem_posix_open(locker_t *thiz)
{
	privinfo_t *priv = NULL;

	priv = (privinfo_t *)thiz->priv;
	if (priv == NULL)
		return ERR;

	priv->sem = sem_open(priv->name, O_CREAT, 0666, priv->init_value);
	if (priv->sem == SEM_FAILED)
	{
		return ERR;
	}

	return OK;
}

static int sem_posix_unlink(locker_t *thiz)
{
    int ret = -1;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    ret = sem_unlink(priv->name);
    if (ret < 0)
    {
        return ERR;
    }

    return OK;
}

static void sem_posix_del(locker_t *thiz)
{
	if (thiz)
	{
		sem_posix_unlink(thiz);
		free(thiz);
		thiz = NULL;
	}
}

static int sem_posix_block_p(locker_t *thiz)
{
    int ret = -1;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    ret = sem_wait(priv->sem);
    if (ret < 0)
    {
        return ERR;
    }

    return OK;
}

#if 0
static int sem_posix_unblock_p(locker_t *thiz)
{
    int ret = -1;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    ret = sem_trywait(priv->sem);
    if (ret < 0)
    {
        return ERR;
    }

    return OK;
}
#endif

static int sem_posix_v(locker_t *thiz)
{
    int ret = -1;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    ret = sem_post(priv->sem);
    if (ret < 0)
    {
        return ERR;
    }

    return OK;
}

locker_t *locker_sem_posix_create(const char *fname, int init_value)
{
    locker_t *thiz = (locker_t *)malloc(sizeof(locker_t) + sizeof(privinfo_t));
    if (thiz != NULL)
    {
        privinfo_t *priv = (privinfo_t *)thiz->priv;

        thiz->destroy = sem_posix_del;
        thiz->lock = sem_posix_block_p;
        thiz->unlock = sem_posix_v;

        memcpy(priv->name, fname, sizeof(priv->name));
        priv->sem = SEM_FAILED;
        priv->init_value = init_value;

        int ret = sem_posix_open(thiz);
        if (ret == ERR)
        {
            sem_posix_del(thiz);
            return NULL;
        }
    }

    return thiz;
}

