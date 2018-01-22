#ifndef __SHM_COM_H__
#define __SHM_COM_H__

#include "locker.h"
#include "shmer.h"

shmer_t *shmer_shm_setup(const char *fname, locker_t *locker);

#endif
