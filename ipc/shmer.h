#ifndef __SHMER_H__
#define __SHMER_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "locker.h"

#define SHM_BUF_SIZE	(2048)

struct _shmer;
typedef struct _shmer shmer_t;

typedef int (*create_fn)(shmer_t *thiz);
typedef void (*destroy_fn)(shmer_t *thiz);
typedef int (*write_fn)(shmer_t *thiz, void *data, int len);
typedef int (*read_fn)(shmer_t *thiz, void *data);
typedef int (*map_fn)(shmer_t *thiz);
typedef int (*unmap_fn)(shmer_t *thiz);

struct _shmer
{
	create_fn create;
	destroy_fn destroy;
	write_fn write;
	read_fn read;
	map_fn map;
	unmap_fn unmap;
	char priv[0];
};

static inline int shmer_create(shmer_t *thiz)
{
	if (thiz == NULL)
		return ERR;
	return thiz->create ? thiz->create(thiz) : ERR;
}

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

static inline int shmer_map(shmer_t *thiz)
{
	if (thiz == NULL)
		return ERR;
	return thiz->map ? thiz->map(thiz) : ERR;
}

static inline int shmer_unmap(shmer_t *thiz)
{
	if (thiz == NULL)
		return ERR;
	return thiz->unmap ? thiz->unmap(thiz) : ERR;
}
#endif
