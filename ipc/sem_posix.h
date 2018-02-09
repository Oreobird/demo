#ifndef __SEM_POSIX_H__
#define __SEM_POSIX_H__

#include "locker.h"

locker_t *locker_sem_posix_create(const char *fname, int init_value);

#endif
