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

struct s_client {
    int fd;
    int (*create_socket) (void);
    int (*connect_srv) (const char *ip, int port);
    int (*send_to_srv) (char *msg, int len);
    int (*recv_from_srv) (char *msg);
    void (*close_connect) (void);
} g_client;

int connect_server(const char *ip, int port)
{
    struct sockaddr_in srv;
    int ret = -1;
    
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(ip);
    srv.sin_port = htons(port);

    ret = connect(g_client.fd, (struct sockaddr*)&srv, sizeof(srv));
    if (ret < 0)
    {
        dbg("connect to server failed\n");
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
   struct sockaddr_in addr;
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
    
    ret = init_client(&g_client);
    if (ret < 0)
    {
        dbg("init_client failed\n");
        return -1;
    }

    g_client.fd = g_client.create_socket();
    g_client.connect_srv("127.0.0.1", 6666);
    for (i = 0; i < 100; i++)
    {
        g_client.send_to_srv(msg, sizeof(msg)); 
        sleep(1);
    }
    g_client.close_connect();
    return 0;
}
