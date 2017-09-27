#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "srv_select.h"
#include "srv_epoll.h"

int main(int argc, char **argv)
{
    abs_srv_t *srv = NULL;
    char *ip = NULL;
    int ret = -1;

    if (argc == 2)
        ip = argv[1];

#if defined(TEST_SELECT)
    srv = srv_select_create(ip, 6666);
#elif defined(TEST_EPOLL)
    srv = srv_epoll_create(ip, 6666);
#endif
    if (srv == NULL)
        return -1;

	ret = srv_create_socket(srv);
	if (ret < 0)
		return -1;

	ret = srv_setup(srv);
	if (ret < 0)
		return -1;

	ret = srv_loop(srv);
	if (ret < 0)
		return -1;

	return 0;
}
