#include "locker.h"
#include "shmer.h"
#include "shm_com.h"

typedef struct _shm_buf
{
	char buf[SHM_BUF_SIZE];
} shm_buf_t;

typedef struct _privinfo
{
	key_t key;
	int shm_id;
	shm_buf_t shm_buf;
	locker_t *locker;
	void *shm_addr;
} privinfo_t;

static int shm_map(shmer_t *thiz)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;

	priv->shm_addr = shmat(priv->shm_id, (void *)0, 0);
	if (priv->shm_addr == (void *)-1)
	{
		return ERR;
	}

	return OK;
}

static int shm_unmap(shmer_t *thiz)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	int ret = -1;

	ret = shmdt(priv->shm_addr);
	if (ret == -1)
	{
		return ERR;
	}

	return OK;
}

static int shm_del(shmer_t *thiz)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	int ret = -1;

	ret = shmctl(priv->shm_id, IPC_RMID, NULL);
	if (ret == -1)
	{
		return ERR;
	}

	return OK;

}

static void shm_destroy(shmer_t *thiz)
{
    if (thiz)
    {
        privinfo_t *priv = (privinfo_t *)thiz->priv;
		shm_unmap(thiz);
        locker_destroy(priv->locker);
        shm_del(thiz);
        free(thiz);
		thiz = NULL;
    }
}

static int shm_init(shmer_t *thiz)
{
	int shm_id = -1;
	int ret = ERR;
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	
	shm_id = shmget(priv->key, sizeof(shm_buf_t), 0666 | IPC_CREAT);
	if (shm_id == -1)
	{
		return ERR;
	}

	priv->shm_id = shm_id;

	ret = shm_map(thiz);
	if (ret == ERR)
	{
		shm_destroy(thiz);
		return ERR;
	}

	return OK;
}


static int shm_write(shmer_t *thiz, void *data, int len)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	shm_buf_t *p_shm_buf = (shm_buf_t *)priv->shm_addr;

	locker_lock(priv->locker);
	memcpy(p_shm_buf->buf, (char *)data, len);
	locker_unlock(priv->locker);

	return OK;
}

static int shm_read(shmer_t *thiz, void *data)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	shm_buf_t *p_shm_buf = (shm_buf_t *)priv->shm_addr;

	locker_lock(priv->locker);
	memcpy(data, (void *)p_shm_buf->buf, sizeof(p_shm_buf->buf));
	locker_unlock(priv->locker);

	return OK;
}

shmer_t *shmer_shm_create(const char *fname, locker_t *locker)
{
	shmer_t *thiz = (shmer_t *)malloc(sizeof(shmer_t) + sizeof(privinfo_t));
	if (thiz != NULL)
	{
		privinfo_t *priv = (privinfo_t *)thiz->priv;

		thiz->destroy = shm_destroy;
		thiz->write = shm_write;
		thiz->read = shm_read;

		priv->key = ftok(fname, 'b');
		priv->shm_id = -1;
		memset(&(priv->shm_buf), 0, sizeof(priv->shm_buf));
		priv->locker = locker;
		priv->shm_addr = (void *)-1;

		int ret = shm_init(thiz);
		if (ret == ERR)
		{
			shm_destroy(thiz);
			return NULL;
		}
	}

	return thiz;
}
