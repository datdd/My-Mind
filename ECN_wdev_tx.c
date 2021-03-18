#if 1
        struct ethhdr {
            unsigned char   h_dest[ETH_ALEN];   /* destination eth addr */
            unsigned char   h_source[ETH_ALEN]; /* source ether addr    */
            UINT16      h_proto;        /* packet type ID field */
        };
        struct arphdr {
           UINT16       ar_hrd;     /* format of hardware address   */
           UINT16       ar_pro;     /* format of protocol address   */
           unsigned char    ar_hln;     /* length of hardware address   */
           unsigned char    ar_pln;     /* length of protocol address   */
           UINT16       ar_op;      /* ARP opcode (command)     */
        };
        PNDIS_PACKET pRxPacket = pPacket;
        PUCHAR pPktHdr, pLayerHdr;
    
        if(pRxPacket)
        {
            pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
            struct ethhdr *pethhdr = (struct ethhdr *)pPktHdr;
            struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
            if (pethhdr->h_proto == ETH_P_IP)
            {
                pLayerHdr = (pPktHdr + 14);
                if (*(pLayerHdr + 9) == 0x11) //UDP
                {
                    PUCHAR pUdpHdr;
                    UINT16 srcPort, dstPort;
                    
                    pUdpHdr = pLayerHdr + 20;
                    srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
                    dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));
    
                    if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
                    {
                        //dump_skb(pOSPkt, pOSPkt->len);
                        PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
    
                        bootpHdr = pUdpHdr + 8;
                        dhcpHdr = bootpHdr + 236;
                        pCliHwAddr = (bootpHdr+28);
    
                        MTWF_PRINT("%s():%d Client Hw Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            __FUNCTION__, __LINE__, PRINT_MAC(pCliHwAddr));
                    }
                }
                else if (*(pLayerHdr + 9) == 0x01) //ICMP
                {
                    PUCHAR pIcmpHdr;
                    u32 saddr, daddr; /* Source and destination addresses */
                    __be32 sip, dip;  /* Source and destination address */

                    /* Convert network endianness to host endiannes */
                    memcpy(&sip, pLayerHdr + 12, 4);
                    memcpy(&dip, pLayerHdr + 16, 4);
                    saddr = ntohl(sip);
                    daddr = ntohl(dip);
                    
                    pIcmpHdr = pLayerHdr + 20;
    
                    MTWF_PRINT("\n\n\n*************************** TX START LOG ICMP ***************************\n");
                    MTWF_PRINT("dev: %s\n", pOSPkt->dev->name);
                    MTWF_PRINT("h_dest: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_dest));
                    MTWF_PRINT("h_source: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_source));
                    MTWF_PRINT("ICMP TYPE %.2x %s %pI4h -> %pI4h\n", *pIcmpHdr, *pIcmpHdr == 0x8 ? "request" : "reply",
                        &saddr, &daddr);
                    MTWF_PRINT("*************************** END LOG ICMP ***************************\n");
                }
            }
#if 0
            else if (pethhdr->h_proto == ETH_TYPE_ARP)
            {
                MTWF_PRINT("\n\n\n***********************TX START LOG ARP***********************\n");
                MTWF_PRINT("h_dest: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_dest));
                MTWF_PRINT("h_source: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pethhdr->h_source));
    
                struct arphdr *arp = NULL;
                __be32 sip, tip;  /* Source and target address */
                u32 saddr, taddr; /* Source and destination addresses */
                unsigned char sha[6], tha[6];
                unsigned char *arp_ptr;
    
                arp = (struct arphdr *)(pethhdr + 1);
                if (arp->ar_op == ARPOP_REPLY)
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
                    printk("%s: %pI4h -> %pI4h\n", arp->ar_op == 0x2 ? "REPLY" : "REQUEST", &saddr, &taddr);
                    printk("SHA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sha));
                    printk("THA: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tha));
                }
                MTWF_PRINT("*************************END LOG ARP*************************\n");
            }
#endif
        }
#endif
