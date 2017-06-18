#include <time.h>
#include <signal.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PAYLOAD     (256)
#define NETLINK_DEMO 25

struct msg_type {
	unsigned char mac_addr[6];
	__be32 ip_addr;
	__be32 source_dev_ip;
};

static int send_netlink_msg(struct msg_type data)
{
    struct nlmsghdr *nh = NULL;
    struct sockaddr_nl sa;
    struct iovec iov;
    struct msghdr msg;
    int nl_fd = -1;
    int ret = -1;

    nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_DEMO);
    if (nl_fd < 0)
    {
        printf("netlink socket failed\n");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = 0;
    sa.nl_pid = 0;

    ret = bind(nl_fd, (struct sockaddr *) &sa, sizeof(sa));
    if (ret < 0)
    {
        printf("netlink bind failed\n");
        close(nl_fd);
        return -1;
    }

    nh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (nh == NULL)
    {
        printf("nlmsghdr malloc failed\n");
        close(nl_fd);
        return -1;
    }

    memset(nh, 0, MAX_PAYLOAD);
    nh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nh->nlmsg_pid = 0;
    nh->nlmsg_flags = 0;
    nh->nlmsg_type = NLMSG_NOOP;

    memcpy(NLMSG_DATA(nh), &data, sizeof(data));

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void *)nh;
    iov.iov_len = nh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(nl_fd, &msg, 0);

    close(nl_fd);
    free(nh);
    return 0;
}

int main(int argc, char **argv)
{
    struct msg_type msg;
    char mac[6] = {0};

    memcpy(msg.mac_addr, mac, sizeof(msg.mac_addr));
    msg.ip_addr = 0;
    msg.source_dev_ip = 0;

    send_netlink_msg(msg);

    return 0;
}
