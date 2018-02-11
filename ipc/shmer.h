#ifndef __SHMER_H__
#define __SHMER_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "locker.h"

#define SHM_BUF_SIZE	(2048)

struct _shmer;
typedef struct _shmer shmer_t;

typedef void (*destroy_fn_t)(shmer_t *thiz);
typedef int (*write_fn_t)(shmer_t *thiz, void *data, int len);
typedef int (*read_fn_t)(shmer_t *thiz, void *data);
typedef int (*sync_write_fn_t)(shmer_t *thiz, void *data, int len);
typedef int (*sync_read_fn_t)(shmer_t *thiz, void *data);

struct _shmer
{
	destroy_fn_t destroy;
	write_fn_t write;
	read_fn_t read;
    sync_write_fn_t sync_write;
    sync_read_fn_t sync_read;
	char priv[0];
};

static inline void shmer_destroy(shmer_t *thiz)
{
	if (thiz == NULL)
		return;
	if (thiz->destroy)
		thiz->destroy(thiz);
}

static inline int shmer_write(shmer_t *thiz, void *data, int len)
{
	if (thiz == NULL)
		return ERR;
	return thiz->write ? thiz->write(thiz, data, len) : ERR;
}

static inline int shmer_read(shmer_t *thiz, void *data)
{
	if (thiz == NULL)
		return ERR;
	return thiz->read ? thiz->read(thiz, data) : ERR;
}

static inline int shmer_sync_write(shmer_t *thiz, void *data, int len)
{
	if (thiz == NULL)
		return ERR;
	return thiz->write ? thiz->sync_write(thiz, data, len) : ERR;
}

static inline int shmer_sync_read(shmer_t *thiz, void *data)
{
	if (thiz == NULL)
		return ERR;
	return thiz->read ? thiz->sync_read(thiz, data) : ERR;
}

#endif
