#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/netfilter_bridge.h>

#include "brdbg.h"

static struct nf_hook_ops nfho;

void hex_dump(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen)
{
	unsigned char *pt;
	int x;

	pt = pSrcBufVA;
	printk("%s: %p, len = %d\n", str, pSrcBufVA, SrcBufLen);
	for (x = 0; x < SrcBufLen; x++) {
		if (x % 16 == 0)
			printk("0x%04x : ", x);
		printk("%02x ", ((unsigned char)pt[x]));
		if (x % 16 == 15)
			printk("\n");
	}
	printk("\n");
}

static unsigned int
pbridge_hook_func(const struct nf_hook_ops *ops,
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
        printk("%s: %pI4h -> %pI4h\n", arp->ar_op == 0x2 ? "REPLY" : "REQUEST", &saddr, &taddr);
        printk("SHA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sha));
        printk("THA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tha));
   }
   return NF_ACCEPT;
}

static int __init ptcp_init(void)
{
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