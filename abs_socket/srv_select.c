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

#include "srv_select.h"

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

static int srv_select_create_socket(abs_srv_t *thiz)
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
    return 0;
}

static void srv_select_destroy(abs_srv_t *thiz)
{
    if (thiz != NULL)
        free(thiz);
}

static int srv_select_setup(abs_srv_t *thiz)
{
    privinfo_t *priv = NULL;
    struct sockaddr_in addr;
    int ret = -1;
	int i;

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

static int handle_connect(privinfo_t *priv, fd_set rfds)
{
    struct sockaddr_in addr;
    unsigned int addr_len;
    int cli_fd = -1;
    int i = 0;

    if (FD_ISSET(priv->sockfd, &rfds))
    {
        cli_fd = accept(priv->sockfd, (struct sockaddr*)&addr, &addr_len);
        if (cli_fd == -1)
        {
            dbg("accept failed\n");
            return -1;
        }

        dbg("accept connection from: %s\n", inet_ntoa(*((struct in_addr*)&addr.sin_addr.s_addr)));

        for (i = 0; i < MAX_CLIENT; i++)
        {
            if (priv->clients.host[i].fd < 0)
            {
                priv->clients.host[i].fd = cli_fd;
                priv->clients.host[i].ip = addr.sin_addr.s_addr;
                priv->clients.num++;
                break;
            }
        }
    }
}

static int handle_data(privinfo_t *priv, fd_set rfds, char *buf, int len)
{
    int i = 0;
    int data_len = -1;

    for (i = 0; i < MAX_CLIENT; i++)
    {
        if (priv->clients.host[i].fd > 0 && FD_ISSET(priv->clients.host[i].fd, &rfds))
        {
            data_len = recv(priv->clients.host[i].fd, (char *) buf, len, 0);
            /* client close connection or error */
            if (data_len <= 0)
            {
                close(priv->clients.host[i].fd);
                priv->clients.host[i].fd = -1;
                priv->clients.num--;
                break;
            }
            dbg("recv data from %s: %s\n", inet_ntoa(*((struct in_addr*)&priv->clients.host[i].ip)), buf);
        }
    }
    return 0;
}

static int srv_select_loop(abs_srv_t *thiz)
{
    privinfo_t *priv = NULL;
    fd_set rfds;
    fd_set wfds;
    char buf[BUFF_SIZE] = {0};
    struct timeval timeout;
    int i = 0;
    int sock_max;
    int ret;

    priv = (privinfo_t *)thiz->priv;
    if (priv == NULL)
        return -1;

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(priv->sockfd, &rfds);
        sock_max = priv->sockfd;

        for (i = 0; i < MAX_CLIENT; i++)
        {
            if (priv->clients.host[i].fd > 0)
            {
                FD_SET(priv->clients.host[i].fd, &rfds);
                sock_max = MAX(sock_max, priv->clients.host[i].fd);
            }
        }

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        ret = select((int) sock_max + 1, &rfds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            dbg("select error\n");
            break;
        }
        else if (ret == 0)
        {
            dbg("timeout ...\n");
            continue;
        }

        handle_connect(priv, rfds);
        handle_data(priv, rfds, buf, sizeof(buf));
    }

}

abs_srv_t *srv_select_create(const char *ip, int port)
{
    abs_srv_t *thiz = (abs_srv_t *)malloc(sizeof(abs_srv_t) + sizeof(privinfo_t));

    if (thiz != NULL)
    {
		int i = 0;
        privinfo_t *priv = (privinfo_t *)thiz->priv;

        thiz->create_socket = srv_select_create_socket;
        thiz->setup = srv_select_setup;
        thiz->loop = srv_select_loop;
        thiz->destroy = srv_select_destroy;

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
