#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include "mmap_com.h"

typedef struct _map_buf
{
	char buf[SHM_BUF_SIZE];
} map_buf_t;

typedef struct _privinfo
{
	int fd;
	locker_t *locker;
	map_buf_t map_buf;
	void *map_addr;
} privinfo_t;


static int mmap_map(shmer_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    lseek(priv->fd, sizeof(priv->map_buf) - 1, SEEK_SET);
    write(priv->fd, "", 1);

    priv->map_addr = mmap(NULL, sizeof(priv->map_buf), PROT_READ|PROT_WRITE, MAP_SHARED, priv->fd, 0);
    if (priv->map_addr == MAP_FAILED)
    {
        return ERR;
    }

    return OK;
}

static int mmap_unmap(shmer_t *thiz)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	int ret = -1;

	ret = munmap(priv->map_addr, sizeof(priv->map_buf));
	if (ret < 0)
	{
		return ERR;
	}

	return OK;
}

static int mmap_init(shmer_t *thiz)
{
	return mmap_map(thiz);
}

static void mmap_destroy(shmer_t *thiz)
{
    if (thiz)
    {
        privinfo_t *priv = (privinfo_t *)thiz->priv;
		if (priv->fd >= 0)
		{
			close(priv->fd);
		}

		mmap_unmap(thiz);
        locker_destroy(priv->locker);
        free(thiz);
    }
}

static int mmap_write(shmer_t *thiz, void *data, int len)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	map_buf_t *p_map_buf = (map_buf_t *)priv->map_addr;

	locker_lock(priv->locker);
	memcpy(p_map_buf->buf, (char *)data, len);
	locker_unlock(priv->locker);

	return OK;
}

static int mmap_read(shmer_t *thiz, void *data)
{
	privinfo_t *priv = (privinfo_t *)thiz->priv;
	map_buf_t *p_map_buf = (map_buf_t *)priv->map_addr;

	locker_lock(priv->locker);
	memcpy(data, (void *)p_map_buf->buf, sizeof(p_map_buf->buf));
	locker_unlock(priv->locker);

	return OK;
}

shmer_t *shmer_mmap_create(const char *fname, locker_t *locker)
{
	shmer_t *thiz = (shmer_t *)malloc(sizeof(shmer_t) + sizeof(privinfo_t));
	if (thiz != NULL)
	{
		privinfo_t *priv = (privinfo_t *)thiz->priv;

		thiz->destroy = mmap_destroy;
		thiz->write = mmap_write;
		thiz->read = mmap_read;

		priv->fd = open(fname, O_CREAT|O_RDWR, 0777);
		if (priv->fd == -1)
		{
			free(thiz);
			thiz = NULL;
			return NULL;
		}

		memset(&(priv->map_buf), 0, sizeof(priv->map_buf));
		priv->locker = locker;
		priv->map_addr = (void *)-1;

		int ret = mmap_init(thiz);
		if (ret == ERR)
		{
			mmap_destroy(thiz);
			return NULL;
		}
	}

	return thiz;
}
