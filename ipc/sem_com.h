#ifndef __SEM_COM_H__
#define __SEM_COM_H__

#include "locker.h"

locker_t *locker_sem_create(const char *fname, int init_value);

#endif
