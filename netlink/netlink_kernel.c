#include <linux/module.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/inet.h> /*in_aton()*/
#include <net/netlink.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("netlink demo Module");
MODULE_AUTHOR("Zhong Guanshi");
MODULE_ALIAS("netlink_demo");

#define NETLINK_DEMO 25

struct msg_type {
	unsigned char mac_addr[6];
	__be32 ip_addr;
	__be32 source_dev_ip;
};

static struct sock *nl_sk;

static void netlink_demo_rcv(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    struct msg_type *msg = NULL;

printk("netlink demo rcv\n");
    if (skb->len >= nlmsg_total_size(0)) {
        nlh = nlmsg_hdr(skb);

        msg = (struct msg_type *)NLMSG_DATA(nlh);
        if (msg) {
            printk("reply dev-->mac: %pM, ip: %pI4, source_dev_ip:%pI4\n", 
			msg->mac_addr, &msg->ip_addr, &msg->source_dev_ip);
        }
    }
}

static int netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input	= netlink_demo_rcv,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_DEMO, &cfg);
	if (!nl_sk) {
		printk ("netlink_kernel_create failed!\n");
		return -ENOMEM;
	}

	return 0;
}

static void netlink_exit(void)
{
	netlink_kernel_release(nl_sk);
}

static int __init netlink_demo_init(void)
{
	int ret = 0;

	printk("init\n");
	ret = netlink_init();
	if (ret < 0) {
		printk ("netlink_demo_init failed!\n");
		return -1;
	}

	return ret;
}

static void __exit netlink_demo_exit(void)
{
	netlink_exit();
	return;
}

module_init(netlink_demo_init);
module_exit(netlink_demo_exit);
