#ifndef __MMAP_COM_H__
#define __MMAP_COM_H__

#include "locker.h"
#include "shmer.h"

shmer_t *shmer_mmap_create(const char *fname, locker_t *locker);

#endif
