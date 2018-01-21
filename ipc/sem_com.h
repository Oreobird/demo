#ifndef __SEM_COM_H__
#define __SEM_COM_H__

#ifndef ERR
#define ERR	(-1)
#endif
#ifndef OK
#define OK	(0)
#endif

locker_t *locker_sem_setup(char *fname, int init_value);

#endif
