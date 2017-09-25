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

int set_socket_nonblock(int fd)
{
    int flags = -1;
    int ret = -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        dbg("fcntl get socket error:%s\n", strerror(errno));
        return -1;
    }

    flags |= O_NONBLOCK;

    ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        dbg("fcntl set socket error:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int set_socket_block(int fd)
{
    int flags = -1;
    int ret = -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        dbg("fcntl get socket error:%s\n", strerror(errno));
        return -1;
    }

    flags &= (~O_NONBLOCK);

    ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        dbg("fcntl set socket error:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

#if 0
int main(int argc, char **argv)
{
    int ret = -1;
    char *ip = NULL;

    if (argc == 2)
    {
        ip = argv[1];
    }

    sock_fd = create_socket();
    if (sock_fd < 0)
    {
        return -1;
    }

    ret = build_server(ip, 6666);
    if (ret < 0)
    {
        return -1;
    }

    //init_client();
    //select_loop();

    epoll_loop();

    return 0;
}
#endif

#ifdef TEST_SELECT_SRV
int main(int argc, char **argv)
{
    abs_srv_t *select_srv = NULL;
    char *ip = NULL;
    int ret = -1;

    if (argc == 2)
        ip = argv[1];

    select_srv = srv_select_create(ip, 6666);
    if (select_srv == NULL)
        return -1;

	ret = srv_create_socket(select_srv);
	if (ret < 0)
		return -1;

	ret = srv_setup(select_srv);
	if (ret < 0)
		return -1;

	ret = srv_loop(select_srv);
	if (ret < 0)
		return -1;

	return 0;
}
#endif
