#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define MAX_CLIENT 10
#define BUFF_SIZE 1024

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

    init_client();
    select_loop();

    return 0;
}
