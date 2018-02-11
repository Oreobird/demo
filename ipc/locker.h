#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef ERR
#define ERR	(-1)
#endif
#ifndef OK
#define OK	(0)
#endif

struct _locker;
typedef struct _locker locker_t;

typedef void (*locker_destroy_fn_t)(locker_t *thiz);
typedef int (*lock_fn_t)(locker_t *thiz);
typedef int (*unlock_fn_t)(locker_t *thiz);

struct _locker
{
    locker_destroy_fn_t destroy;
    lock_fn_t lock;
    unlock_fn_t unlock;
    char priv[0];
};

static inline void locker_destroy(locker_t *thiz)
{
	if (thiz == NULL)
		return;
	if (thiz->destroy)
		thiz->destroy(thiz);
}

static inline int locker_lock(locker_t *thiz)
{
	if (thiz == NULL)
		return ERR;
	return thiz->lock ? thiz->lock(thiz) : ERR;
}

static inline int locker_unlock(locker_t *thiz)
{
	if (thiz == NULL)
		return ERR;
	return thiz->unlock ? thiz->unlock(thiz) : ERR;
}

#endif
