#ifndef __SHMER_H__
#define __SHMER_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "locker.h"

#define SHM_BUF_SIZE	(2048)

struct _shmer;
typedef struct _shmer shmer_t;

typedef void (*destroy_fn)(shmer_t *thiz);
typedef int (*write_fn)(shmer_t *thiz, void *data, int len);
typedef int (*read_fn)(shmer_t *thiz, void *data);

struct _shmer
{
	destroy_fn destroy;
	write_fn write;
	read_fn read;
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

#endif
