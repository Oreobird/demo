#ifndef __SEM_SYSV_H__
#define __SEM_SYSV_H__

#include "locker.h"

locker_t *locker_sem_sysv_create(const char *fname, int init_value);

#endif
