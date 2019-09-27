#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#ifndef NETLINK_EXAMPLE 21
#define NETLINK_EXAMPLE 21
#endif

enum nlexample_msg_types {
   NLEX_MSG_BASE = NLMSG_MIN_TYPE,
   NLEX_MSG_UPD = NLEX_MSG_BASE,
   NLEX_MSG_GET,
   NLEX_MSG_MAX
};

enum nlexample_attr {
   NLE_UNSPEC,
   NLE_MYVAR,
   __NLE_MAX,
};

#define NLE_MAX (__NLE_MAX - 1)
#define NLEX_GRP_MYVAR (1 << 0)

static struct sock *nlsk;
static int myvar;

static const
struct nla_policy nlex_policy[NLE_MAX + 1] = {
   [NLE_MYVAR] = {.type = NLA_U32},
};

static int nl_step2(struct nlattr *cda[], struct nlmsghdr *nlh);
static int nlex_notify(int rep, int pid);
static int nlex_unicast(int pid);


static int
nl_step(struct sk_buff *skb, struct nlmsghdr *nlh)
{
   int err;
   struct nlattr *cda[NLE_MAX + 1];
   struct nlattr *attr = NLMSG_DATA(nlh);
   int attrlen = nlh->nlmsg_len - NLMSG_SPACE(0);

#if 0
   if (security_netlink_recv(skb, CAP_NET_ADMIN))
      return -EPERM;
#endif

   if (nlh->nlmsg_len < NLMSG_SPACE(0))
      return -EINVAL;

   err = nla_parse(cda, NLE_MAX, attr, attrlen, nlex_policy);

   if (err < 0)
      return err;

   return nl_step2(cda, nlh);
}

static void
nl_callback(struct sk_buff *skb)
{
   netlink_rcv_skb(skb, &nl_step);
}

static int
nl_step2(struct nlattr *cda[],
      struct nlmsghdr *nlh)
{
   int echo = nlh->nlmsg_flags & NLM_F_ECHO;
   int pid = nlh->nlmsg_pid;

   switch(nlh->nlmsg_type)
   {
      case NLEX_MSG_UPD:
         if (!cda[NLE_MYVAR])
            return -EINVAL;

         myvar = nla_get_u32(cda[NLE_MYVAR]);
         nlex_notify(echo, pid);
         break;
      case NLEX_MSG_GET:
         nlex_unicast(pid);
         break;
      default:
         break;
   }

   return 0;
}

static int
nlex_notify(int rep, int pid)
{
   struct sk_buff *skb;

   skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
   if (skb == NULL)
      return -ENOMEM;

   nlmsg_put(skb, pid, rep, NLEX_MSG_UPD, 0, 0);
   nla_put_u32(skb, NLE_MYVAR, myvar);

   nlmsg_notify(nlsk, skb, pid,
         NLEX_GRP_MYVAR,
         rep, GFP_KERNEL);

   return 0;
}

static int
nlex_unicast(int pid)
{
   struct sk_buff *skb;

   skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
   if (skb == NULL)
      return -ENOMEM;

   nlmsg_put(skb, pid, 0, NLEX_MSG_UPD, 0, 0);
   nla_put_u32(skb, NLE_MYVAR, myvar);

   nlmsg_unicast(nlsk, skb, pid);
   return 0;
}

static int __init nlexample_init(void)
{
   nlsk = netlink_kernel_create(&init_net,
         NETLINK_EXAMPLE,
         NLEX_GRP_MAX,
         nl_callback,
         NULL,
         THIS_MODULE);

   if (nlsk == NULL)
   {
      printk(KERN_ERR "Can't create netlink\n");
      return -NOMEM;
   }

   return 0;
}

void __exit nlexample_exit(void)
{
   netlink_kernel_release(nlsk);
}

module_init(nlexample_init);
module_init(nlexample_exit);
