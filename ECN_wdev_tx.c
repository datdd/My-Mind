#if 1
   {
      UCHAR *pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
      UINT16 TypeLen = 0;
      if(pSrcBuf)
      {
         TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
         if(TypeLen == ETH_TYPE_ARP)
         {
            struct arphdr {
               UINT16		ar_hrd;		/* format of hardware address	*/
               UINT16		ar_pro;		/* format of protocol address	*/
               unsigned char	ar_hln;		/* length of hardware address	*/
               unsigned char	ar_pln;		/* length of protocol address	*/
               UINT16		ar_op;		/* ARP opcode (command)		*/
#if 0
               /*
               *	 Ethernet looks like this : This bit is variable sized however...
               */
               unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
               unsigned char		ar_sip[4];		/* sender IP address		*/
               unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
               unsigned char		ar_tip[4];		/* target IP address		*/
#endif
            };

         struct ethhdr {
            unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
            unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
            UINT16		h_proto;		/* packet type ID field	*/
         };
         struct ethhdr *pethhdr = (struct ethhdr *)pSrcBuf;
         MTWF_PRINT("\n\n\n***************************START LOG***************************\n");
         MTWF_PRINT("===== %s():%u =====\n", __FUNCTION__, __LINE__);
         MTWF_PRINT("ETH_TYPE_ARP 0x%x\n", pethhdr->h_proto);
         MTWF_PRINT("h_dest: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_dest));
         MTWF_PRINT("h_source: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_source));
            

            struct arphdr *arp = NULL;
            __be32 sip, tip;  /* Source and target address */
            u32 saddr, taddr; /* Source and destination addresses */
            unsigned char sha[6], tha[6];
            unsigned char *arp_ptr;

            arp = (struct arphdr *)(pethhdr + 1);
            if (arp->ar_op == 0x2)
            {
               arp_ptr = (unsigned char *)(arp + 1);
               memcpy(sha, arp_ptr, 6);
               arp_ptr += 6;
               memcpy(&sip, arp_ptr, 4);
               arp_ptr += 4;
               memcpy(tha, arp_ptr, 6);
               arp_ptr += 6;
               memcpy(&tip, arp_ptr, 4);

               /* Convert network endianness to host endiannes */
               saddr = ntohl(sip);
               taddr = ntohl(tip);

               /* Print packet route */
               printk("%s: %pI4h -> %pI4h\n", __FUNCTION__, &saddr, &taddr);
               printk("SHA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sha));
               printk("THA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tha));
            }
            MTWF_PRINT("*************************END LOG*****************************\n");
         }
      }
   }
#endif
