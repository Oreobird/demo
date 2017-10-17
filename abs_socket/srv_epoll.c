#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "srv_epoll.h"

typedef struct _host {
    int fd;
    in_addr_t ip;
    unsigned char mac[6];
} host_t;

typedef struct _client {
    int num;
    struct _host host[MAX_CLIENT];
} client_t;

typedef struct _privinfo
{
    char ip[64];
    int port;
    int sockfd;
    struct _client clients;
} privinfo_t;


static int set_socket_nonblock(int fd)
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

#if 0
static int set_socket_block(int fd)
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
#endif

static int srv_epoll_create_socket(abs_srv_t *thiz)
{
    privinfo_t *priv = NULL;
    int fd = -1;
    int optval = 1;

    priv = (privinfo_t *)thiz->priv;
    if (priv == NULL)
        return -1;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (fd < 0)
    {
        dbg("create socket failed\n");
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        dbg("setsockopt failed\n");
        close(fd);
        return -1;
    }

    priv->sockfd = fd;
    set_socket_nonblock(priv->sockfd);

    return 0;
}

static void srv_epoll_destroy(abs_srv_t *thiz)
{
    if (thiz != NULL)
        free(thiz);
}

static int srv_epoll_setup(abs_srv_t *thiz)
{
    privinfo_t *priv = NULL;
    struct sockaddr_in addr;
    int ret = -1;

    priv = (privinfo_t *)thiz->priv;
    if (priv == NULL)
        return -1;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = strlen(priv->ip) != 0 ? inet_addr(priv->ip) : htonl(INADDR_ANY);
    addr.sin_port = htons(priv->port);

    ret = bind(priv->sockfd, (struct sockaddr*)&(addr), sizeof(addr));
    if (ret < 0)
    {
        dbg("bind addr %s failed\n", priv->ip);
        return -1;
    }

    ret = listen(priv->sockfd, MAX_CLIENT);
    if (ret < 0)
    {
        dbg("listen to addr %s failed\n", priv->ip);
        return -1;
    }

    return 0;
}

static void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

static int epoll_handle_connect(int epollfd, privinfo_t *priv)
{
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    int clifd = -1;

    while ((clifd = accept(priv->sockfd, (struct sockaddr *)&addr, &addr_len)) > 0)
    {
        dbg("accept connection from: %s\n", inet_ntoa(*((struct in_addr*)&addr.sin_addr.s_addr)));

        set_socket_nonblock(clifd);
        add_event(epollfd, clifd, EPOLLIN | EPOLLET);
    }
    return 0;
}

static int epoll_read(int epollfd, int clifd, char *buf, int len)
{
    if (clifd >= 0)
    {
        int data_len = 0;
        int nread = 0;
        while ((nread = read(clifd, buf + data_len, len - 1)) > 0)
        {
            data_len += nread;
        }

        if (nread < 0 && errno != EAGAIN)
        {
            dbg("read data failed\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLIN | EPOLLET);
            return -1;
        }
        else if (nread == 0)
        {
            dbg("client disconnted\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLIN | EPOLLET);
            return -1;
        }

        buf[data_len] = '\0';
        dbg("recv data: %s\n", buf);

        //modify_event(epollfd, clifd, EPOLLOUT | EPOLLET); // echo server
    }

    return 0;
}

static int epoll_write(int epollfd, int clifd, char *buf, int len)
{
    if (clifd >= 0)
    {
        int data_len = strlen(buf);
        int nwrite = 0;
        while (data_len > 0)
        {
            nwrite = write(clifd, buf, MIN((int)strlen(buf), len));
            if (nwrite < data_len)
            {
                break;
            }
            data_len -= nwrite;
        }
        if (nwrite < 0)
        {
            dbg("write data failed\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLOUT | EPOLLET);
            return -1;
        }

        dbg("send data: %s\n", buf);

        modify_event(epollfd, clifd, EPOLLIN | EPOLLET);
    }
    return 0;
}

static int epoll_handle_data(int epollfd, struct epoll_event *ev, char *buf, int len)
{
    int ret = -1;

    if (ev->events & EPOLLIN)
    {
        ret = epoll_read(epollfd, ev->data.fd, buf, len);
    }
    else if (ev->events & EPOLLOUT)
    {
        ret = epoll_write(epollfd, ev->data.fd, buf, len);
    }

    return ret;
}

static int srv_epoll_loop(abs_srv_t *thiz)
{
    struct epoll_event events[MAX_CLIENT];
    int i, nfds, epollfd = -1;
    int ret = -1;
    char buf[1500] = {0};
    privinfo_t *priv = NULL;

    priv = (privinfo_t *)thiz->priv;
    if (priv == NULL)
        return -1;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        dbg("epoll_create1 failed\n");
        return -1;
    }

    add_event(epollfd, priv->sockfd, EPOLLIN | EPOLLET);

    while (1)
    {
        dbg("epoll_waiting ....\n");
        nfds = epoll_wait(epollfd, events, MAX_CLIENT, -1);
        if (nfds == -1)
        {
            dbg("epoll_wait\n");
            return -1;
        }

        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == priv->sockfd && (events[i].events & EPOLLIN))
            {
                ret = epoll_handle_connect(epollfd, priv);
                if (ret < 0)
                {
                    dbg("epoll_handle_connect error\n");
                }
            }
            else
            {
                ret = epoll_handle_data(epollfd, &events[i], buf, sizeof(buf));
                if (ret < 0)
                {
                    dbg("epoll_handle_data error\n");
                }
            }
        }
    }
    return 0;
}

abs_srv_t *srv_epoll_create(const char *ip, int port)
{
    abs_srv_t *thiz = (abs_srv_t *)malloc(sizeof(abs_srv_t) + sizeof(privinfo_t));
    if (thiz != NULL)
    {
        int i = 0;
        privinfo_t *priv = (privinfo_t *)thiz->priv;

        thiz->create_socket = srv_epoll_create_socket;
        thiz->setup = srv_epoll_setup;
        thiz->loop = srv_epoll_loop;
        thiz->destroy = srv_epoll_destroy;

        if (ip)
            memcpy(priv->ip, ip, sizeof(priv->ip));

        priv->port = port;
        priv->sockfd = -1;

        memset(&priv->clients, 0, sizeof(priv->clients));
		for (i = 0; i < MAX_CLIENT; i++)
		{
			priv->clients.host[i].fd = -1;
		}
    }
    return thiz;
}

