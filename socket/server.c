#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define MAX_CLIENT 10

struct s_host {
    int fd;
    in_addr_t ip;
    unsigned char mac[6];
};

struct s_client {
    int num;
    struct s_host cli[MAX_CLIENT];
}

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

    return 0;
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

    ret = listen(sock_fd, 5);
    if (ret < 0)
    {
        dbg("listen to addr %s failed\n", ip);
        return -1;
    }
    
    return 0;
}

static int wait_for_connection(void)
{
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    g_client[g_client.num] = accept(sock_fd, (struct sockaddr*)&addr, &addr_len);
    if (g_client[g_client.num] < 0)
    {
        dbg("accept client failed\n");
        return -1;
    }

    g_client.num++;
    
    return 0;
}

static void handle_connection(void)
{

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

    while (1)
    {
        if (wait_for_connection() < 0)
        {
            close(sock_fd);
            return -1;
        }

        handle_connection();
    }
}
