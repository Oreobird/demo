#ifndef __SRV_SELECT_H__
#define __SRV_SELECT_H__

#include "abs_srv.h"

abs_srv_t *srv_select_create(const char *ip, int port);

#endif
