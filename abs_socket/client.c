#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define BUFF_SIZE 1024
#define TIME_OUT 5

struct s_client {
    int fd;
    int (*create_socket) (void);
    int (*connect_srv) (const char *ip, int port);
    int (*send_to_srv) (char *msg, int len);
    int (*recv_from_srv) (char *msg);
    void (*close_connect) (void);
} g_client;

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

int check_connect_timeout(int fd)
{
    struct timeval timeout;
    fd_set rdset;
    int optval;
    int ret = -1;
    socklen_t len;

    if (errno == EINPROGRESS)
    {
        len = sizeof(int);
        timeout.tv_sec = TIME_OUT;
        timeout.tv_usec = 0;
        FD_ZERO(&rdset);
        FD_SET(fd, &rdset);

        ret = select(fd + 1, NULL, &rdset, NULL, &timeout);
        if (ret < 0)
        {
            dbg("Connect error, err msg:%s\n", strerror(errno));
            return -1;
        }
        else if (ret == 0)
        {
            dbg("Connect timeout, err msg:%s\n", strerror(errno));
            return -1;
        }
        else
        {
            ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &len);
            if (ret < 0)
            {
                dbg("getsockopt(SO_ERROR):%s\n", strerror(errno));
                return -1;
            }

            if (optval)
            {
               dbg("Client connect error:%s\n", strerror(optval));
               return -1;
           }
        }
    }
    else
    {
        dbg("Connect to server failed\n");
        return -1;
    }

    return 0;
}

int connect_server(const char *ip, int port)
{
    struct sockaddr_in srv;
    int ret = -1;
    
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(ip);
    srv.sin_port = htons(port);

    ret = set_socket_nonblock(g_client.fd);
    if (ret < 0)
    {
        close(g_client.fd);
        return -1;
    }

    ret = connect(g_client.fd, (struct sockaddr*)&srv, sizeof(srv));
    if (ret != 0)
    {
        ret = check_connect_timeout(g_client.fd);
        if (ret < 0)
        {
            close(g_client.fd);
            return -1;
        }
    }

    ret = set_socket_block(g_client.fd);
    if (ret < 0)
    {
        close(g_client.fd);
        return -1;
    }

    return 0;
}

int send_to_server(char *msg, int len)
{
    if (g_client.fd > 0)
    {
        send(g_client.fd, msg, len, 0); 
    }
    return 0;
}

void close_connect(void)
{
    if (g_client.fd > 0)
    {
        close(g_client.fd);
    }
}

int create_socket(void)
{
   int fd = -1;

   fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
   if (fd < 0)
   {
       dbg("creage_socket failed\n");
       return -1;
   }

   return fd;
}



static int init_client(struct s_client *client)
{
    if (client == NULL)
    {
        return -1;
    }

    client->fd = -1;
    client->create_socket = create_socket;
    client->connect_srv = connect_server;
    client->send_to_srv = send_to_server;
    client->close_connect = close_connect;
    return 0;
}

int main(int argc, char **argv)
{
    int ret = -1;
    int i = 0; 
    char msg[BUFF_SIZE] = {"Hello I am client"};

    (void)argc;
    (void)argv;
    
    ret = init_client(&g_client);
    if (ret < 0)
    {
        dbg("init_client failed\n");
        return -1;
    }

    g_client.fd = g_client.create_socket();
    ret = g_client.connect_srv("127.0.0.1", 6666);
    if (ret < 0)
    {
        dbg("connect server failed\n");
        return -1;
    }

    for (i = 0; i < 100; i++)
    {
        g_client.send_to_srv(msg, strlen(msg)); 
        dbg("send msg:%s\n", msg);
        sleep(2);
    }
    g_client.close_connect();
    return 0;
}
