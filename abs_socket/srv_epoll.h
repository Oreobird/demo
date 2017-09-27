#ifndef __SRV_EPOLL_H__
#define __SRV_EPOLL_H__

#include "abs_srv.h"

abs_srv_t *srv_epoll_create(const char *ip, int port);

#endif

