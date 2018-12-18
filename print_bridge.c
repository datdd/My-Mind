#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/netfilter_bridge.h>

static struct nf_hook_ops nfho;

static unsigned int pbridge_hook_func(const struct nf_hook_ops *ops,
                                      struct sk_buff *skb,
                                      const struct net_device *in,
                                      const struct net_device *out,
                                      int (*okfn)(struct sk_buff *))
{
   /*
   *	Extract fields
   */
   if (eth_hdr(skb)->h_proto == ETH_P_ARP)
   {
      struct arphdr *arp;
      __be32 sip, tip;  /* Source and target address */
      u32 saddr, taddr; /* Source and destination addresses */
      unsigned char sha[6], tha[6];
      unsigned char *arp_ptr;

      arp = arp_hdr(skb);

      printk("DATDD %s:%s:%d skb->protocol: %04X, ar_op: %d\n", __FILE__,
             __FUNCTION__, __LINE__, skb->protocol, arp->ar_op);

      arp_ptr = (unsigned char *)(arp + 1);
      memcpy(sha, arp_ptr, 6);
      arp_ptr += skb->dev->addr_len;
      memcpy(&sip, arp_ptr, 4);
      arp_ptr += 4;
      memcpy(tha, arp_ptr, 6);
      arp_ptr += skb->dev->addr_len;
      memcpy(&tip, arp_ptr, 4);

      /* Convert network endianness to host endiannes */
      saddr = ntohl(sip);
      taddr = ntohl(tip);

      /* Print packet route */
      printk("%s: %pI4h -> %pI4h\n", __FUNCTION__, &saddr, &taddr);

      printk("SHA: %02x:%02x:%02x:%02x:%02x:%02x\n",
             (unsigned char)sha[0],
             (unsigned char)sha[1],
             (unsigned char)sha[2],
             (unsigned char)sha[3],
             (unsigned char)sha[4],
             (unsigned char)sha[5]);

      printk("THA: %02x:%02x:%02x:%02x:%02x:%02x\n",
             (unsigned char)tha[0],
             (unsigned char)tha[1],
             (unsigned char)tha[2],
             (unsigned char)tha[3],
             (unsigned char)tha[4],
             (unsigned char)tha[5]);
   }
   return NF_ACCEPT;
}

static int __init ptcp_init(void)
{
#if 1
   int res;

   nfho.hook = (nf_hookfn *)pbridge_hook_func; /* hook function */
   nfho.hooknum = NF_BR_PRE_ROUTING;           /* received packets */
   nfho.pf = PF_BRIDGE;                        /* IPv4 */
   nfho.priority = NF_BR_PRI_BRNF;             /* max hook priority */
   nfho.owner = THIS_MODULE;

   res = nf_register_hook(&nfho);
   if (res < 0)
   {
      pr_err("print_bridge: error in nf_register_hook()\n");
      return res;
   }

   pr_debug("print_bridge: loaded\n");
#endif

   return 0;
}

static void __exit ptcp_exit(void)
{
   nf_unregister_hook(&nfho);
   pr_debug("print_bridge: unloaded\n");
}

module_init(ptcp_init);
module_exit(ptcp_exit);

MODULE_AUTHOR("datdd");
MODULE_DESCRIPTION("Module for printing bridge packet data");
MODULE_LICENSE("GPL");