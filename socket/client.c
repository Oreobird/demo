#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define BUFF_SIZE 1024

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int sock_fd = -1;

static int connect_server(const char *ip, int port)
{
    struct sockaddr_in addr;
    int ret = -1;
    struct sockaddr_in server_addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(0);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        dbg("create socket failed\n");
        return -1;
    }

    ret = bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)); 
    if (ret < 0)
    {
        dbg("client bind port failed\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (ip == NULL || inet_aton(ip, &server_addr.sin_addr) == 0)
    {
        dbg("server ip addr error\n");
        return -1;
    }
    server_addr.sin_port = htons(port);
    socklen_t server_addr_len = sizeof(server_addr);

    ret = connect(sock_fd, (struct sockaddr *)&server_addr, server_addr_len);
    if (ret < 0)
    {
        dbg("can not connect to %s\n", ip);
        return -1;
    }
    
    return 0;
}

static int talk_to_server(void)
{
    char buf[BUFF_SIZE] = {0};
    int data_len;
#if 0
    data_len = recv(sock_fd, buf, sizeof(buf), 0);
    if (data_len < 0)
    {
        dbg("recv data from server\n"); 
        return -1;
    }
#endif
    snprintf(buf, sizeof(buf), "%s", "Hello I am client\n");
    send(sock_fd, buf, sizeof(buf), 0);
    return 0;
}

static void close_connection(void)
{
    if (sock_fd < 0)
    {
        close(sock_fd);
    }
}
int main(int argc, char **argv)
{
    int i = 0;

    if (connect_server("127.0.0.1", 6666) < 0)
    {
        return -1;
    }

    for (i = 0; i < 1000; i++)
    {
        talk_to_server();
        sleep(3);
    }

    close_connection();
    return 0;
}
