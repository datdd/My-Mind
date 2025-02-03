#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <net/sock.h>

#define NETLINK_USER 31  // Protocol must match user-space
static struct sock *nl_sk = NULL;

// static void netlink_recv_msg(struct sk_buff *skb) {
//     struct nlmsghdr *nlh;
//     struct sk_buff *skb_out;
//     int msg_size;
//     char *msg = "Hello from kernel";
//     int pid;
//     int res;

//     nlh = (struct nlmsghdr*)skb->data;
//     printk(KERN_INFO "Netlink received: %s\n", (char*)NLMSG_DATA(nlh));

//     // Respond to the user process
//     pid = nlh->nlmsg_pid; // Get sender's PID
//     msg_size = strlen(msg);

//     skb_out = nlmsg_new(msg_size, 0);
//     if (!skb_out) {
//         printk(KERN_ERR "Failed to allocate skb\n");
//         return;
//     }

//     nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
//     strncpy(NLMSG_DATA(nlh), msg, msg_size);
//     res = nlmsg_unicast(nl_sk, skb_out, pid);

//     if (res < 0)
//         printk(KERN_ERR "Error sending message to user\n");
// }

static void netlink_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    
    printk(KERN_INFO "Netlink: Message received in kernel\n");

    if (!skb) {
        printk(KERN_ERR "Netlink: Received NULL skb\n");
        return;
    }

    nlh = (struct nlmsghdr*)skb->data;
    if (!nlh) {
        printk(KERN_ERR "Netlink: Received NULL nlmsghdr\n");
        return;
    }

    printk(KERN_INFO "Netlink received: %s\n", (char*)NLMSG_DATA(nlh));
}


static int __init netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
        .input = netlink_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ERR "Failed to create Netlink socket\n");
        return -ENOMEM;
    } else {
        printk(KERN_INFO "Netlink socket created successfully\n");
    }

    printk(KERN_INFO "Netlink module loaded\n");
    return 0;
}

static void __exit netlink_exit(void) {
    netlink_kernel_release(nl_sk);
    printk(KERN_INFO "Netlink module unloaded\n");
}

module_init(netlink_init);
module_exit(netlink_exit);
MODULE_LICENSE("GPL");
