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

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define MAX_CLIENT 10
#define BUFF_SIZE 1024

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct s_host {
    int fd;
    in_addr_t ip;
    unsigned char mac[6];
};

struct s_client {
    int num;
    struct s_host cli[MAX_CLIENT];
};

int sock_fd = -1;
struct s_client g_client;

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
static int create_socket(void)
{
    int fd = -1;
    int optval = 1;

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

    return fd;
}

static int build_server(const char *ip, int port)
{
    struct sockaddr_in addr;
    int ret = -1;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip != NULL ? inet_addr(ip) : htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    ret = bind(sock_fd, (struct sockaddr*)&(addr), sizeof(addr));
    if (ret < 0)
    {
        dbg("bind addr %s failed\n", ip);
        return -1;
    }

    ret = listen(sock_fd, MAX_CLIENT);
    if (ret < 0)
    {
        dbg("listen to addr %s failed\n", ip);
        return -1;
    }
    
    return 0;
}

static int wait_for_client(void)
{
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    g_client.cli[g_client.num].fd = accept(sock_fd, (struct sockaddr*)&addr, &addr_len);
    if (g_client.cli[g_client.num].fd < 0)
    {
        dbg("accept client failed\n");
        return -1;
    }

    g_client.num++;
    
    return 0;
}

static void init_client(void)
{
    int i = 0;
    for (i = 0; i < MAX_CLIENT; i++)
    {
        g_client.cli[i].fd = -1;
    }
    g_client.num = 0;
}

static int handle_connect(fd_set rfds)
{
    struct sockaddr_in addr;
    unsigned int addr_len;
    int cli_fd = -1;
    int i = 0;
    
    if (FD_ISSET(sock_fd, &rfds))
    {
        cli_fd = accept(sock_fd, (struct sockaddr*)&addr, &addr_len);
        if (cli_fd == -1)
        {
            dbg("accept failed\n");
            return -1;
        }

        for (i = 0; i < MAX_CLIENT; i++)
        {
            if (g_client.cli[i].fd < 0)
            {
                g_client.cli[i].fd = cli_fd;
                g_client.cli[i].ip = addr.sin_addr.s_addr;
                g_client.num++;
                break;
            }
        }
    }
}

static int handle_data(fd_set rfds, char *buf, int len)
{
    int i = 0;
    int data_len = -1;

    for (i = 0; i < MAX_CLIENT; i++)
    {
        if (g_client.cli[i].fd > 0 && FD_ISSET(g_client.cli[i].fd, &rfds))
        {
            data_len = recv(g_client.cli[i].fd, (char *) buf, len, 0);
            /* client close connection or error */
            if (data_len <= 0)
            {
                close(g_client.cli[i].fd);
                g_client.cli[i].fd = -1;
                g_client.num--;
                break; 
            }
            dbg("recv data from %s: %s\n", inet_ntoa(*((struct in_addr*)&g_client.cli[i].ip)), buf);
        }
    }
    return 0;
}

static void select_loop(void)
{
    fd_set rfds;
    fd_set wfds;
    char buf[BUFF_SIZE] = {0};
    struct timeval timeout;
    int i = 0;
    int sock_max;
    int ret;
    
    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(sock_fd, &rfds);
        sock_max = sock_fd;

        for (i = 0; i < MAX_CLIENT; i++)
        {
            if (g_client.cli[i].fd > 0)
            {
                FD_SET(g_client.cli[i].fd, &rfds);
                sock_max = MAX(sock_max, g_client.cli[i].fd);
            }
        }

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        ret = select((int) sock_max + 1, &rfds, NULL,NULL, &timeout);
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

        handle_connect(rfds);
        handle_data(rfds, buf, sizeof(buf));
    }
}

static int epoll_handle_connect(int epollfd, struct epoll_event *ev)
{
    int ret = -1;
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    int conn_sock = accept(sock_fd, (struct sockaddr *)&addr, &addr_len);
    if (conn_sock == -1)
    {
        dbg("accept failed\n");
        return -1;
    }
                
    set_socket_nonblock(conn_sock);
    ev->events = EPOLLIN | EPOLLET;
    ev->data.fd = conn_sock;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, ev);
    if (ret == -1)
    {
        dbg("epoll_ctl: conn_sock\n");
        return -1;
    }
    return 0;
}

static int epoll_read(struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    if (event->data.fd >= 0)
    {
        int sockfd = event->data.fd;
        int data_len = 0;
        data_len = read(sockfd, buf, len);
        if (data_len < 0)
        {
            dbg("read data failed\n");
            close(sockfd);
            event->data.fd = -1;
        }
        else if (data_len == 0)
        {
            dbg("client disconnted\n");
            close(sockfd);
            event->data.fd = -1;
        }
        buf[data_len] = '\0';
        dbg("recv data: %s\n", buf);
        ev->data.fd = sockfd;
        ev->events = EPOLLOUT|EPOLLET;
    }
}
static int epoll_write(struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    int data_len = -1;
    int sockfd = event->data.fd;
    data_len = write(sockfd, buf, MIN(strlen(buf), len));
    if (data_len < 0)
    {
        dbg("write data failed\n");
        close(sockfd);
        event->data.fd = -1;
    }

    dbg("send data: %s\n", buf);

    ev->data.fd = sockfd;
    ev->events = EPOLLIN | EPOLLET;
    return 0;
}
static int epoll_handle_data(int epollfd, struct epoll_event *ev, struct epoll_event *event, char *buf, int len)
{
    int ret = -1;

    if (ev->events & EPOLLIN)
    {
        ret = epoll_read(ev, event, buf, len);
    }
    else if (ev->events & EPOLLOUT)
    {
        ret = epoll_write(ev, event, buf, len);
    }

    epoll_ctl(epollfd, EPOLL_CTL_MOD, event->data.fd, ev);
    return ret;
}

static int epoll_loop(void)
{
    struct epoll_event ev, events[MAX_CLIENT];
    int i, nfds, epollfd = -1;
    int ret = -1;
    char buf[1500] = {0};

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        dbg("epoll_create1 failed\n");
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = sock_fd;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_fd, &ev);
    if (ret == -1)
    {
        dbg("epoll_ctl failed\n");
        return -1;
    }

    while (1)
    {
        nfds = epoll_wait(epollfd, events, MAX_CLIENT, -1);
        if (nfds == -1)
        {
            dbg("epoll_wait\n");
            return -1;
        }

        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == sock_fd)
            {
                epoll_handle_connect(epollfd, &ev);
            }
            else
            {
                epoll_handle_data(epollfd, &ev, &events[i], buf, sizeof(buf));
            }
        }
    }
    return 0;
}

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
