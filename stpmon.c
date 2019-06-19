#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/if_bridge.h>
#include <unistd.h>

static const char *const br_port_state_names[] = {
   [BR_STATE_DISABLED] = "disabled",
   [BR_STATE_LISTENING] = "listening",
   [BR_STATE_LEARNING] = "learning",
   [BR_STATE_FORWARDING] = "forwarding",
   [BR_STATE_BLOCKING] = "blocking",
};

void parseRtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
   memset(tb, 0, sizeof(struct rtattr *) * (max + 1));

   while (RTA_OK(rta, len))
   {
      if (rta->rta_type <= max) {
         tb[rta->rta_type] = rta;
      }
      rta = RTA_NEXT(rta,len);
   }
}

int main(int argc, char *argv[])
{
   int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

   if (fd < 0)
   {
      printf("Failed to create netlink socket: %s\n", (char*)strerror(errno));
      return 1;
   }

   struct sockaddr_nl local;
   char buf[8192];
   struct iovec iov;
   iov.iov_base = buf;
   iov.iov_len = sizeof(buf);

   memset(&local, 0, sizeof(local));

   local.nl_family = AF_NETLINK;
   local.nl_groups = RTMGRP_LINK;
   local.nl_pid = getpid();

   struct msghdr msg;  
   {
      msg.msg_name = &local;
      msg.msg_namelen = sizeof(local);
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;
   }   

   if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0)
   {
      printf("Failed to bind netlink socket: %s\n", (char*)strerror(errno));
      close(fd);
      return 1;
   }   

   while (1)
   {
      ssize_t nll = recvmsg(fd, &msg, MSG_DONTWAIT);

      if (nll < 0)
      {
         if (errno == EINTR || errno == EAGAIN)
         {
            continue;
         }

         printf("Failed to read netlink: %s", (char*)strerror(errno));
         continue;
      }

      if (msg.msg_namelen != sizeof(local))
      {
         printf("Invalid length of the sender address struct\n");
         continue;
      }

      // message parser
      struct nlmsghdr *nlp = (struct nlmsghdr *) buf;
      for(;NLMSG_OK(nlp, nll); nlp = NLMSG_NEXT(nlp, nll))
      {
         char *ifName = NULL;
         uint8_t protinfo;

         if (nlp->nlmsg_type == RTM_NEWLINK)
         {
            struct ifinfomsg *ifi;
            struct rtattr *tb[IFLA_MAX + 1];
            ifi = (struct ifinfomsg*) NLMSG_DATA(nlp);
            parseRtattr(tb, IFLA_MAX, IFLA_RTA(ifi), nlp->nlmsg_len);

            if (tb[IFLA_IFNAME])
            {
               ifName = (char*)RTA_DATA(tb[IFLA_IFNAME]);
            }

            if (tb[IFLA_PROTINFO])
            {
               protinfo = *(uint8_t *)RTA_DATA(tb[IFLA_PROTINFO]);
               printf("Network interface %s, protinfo: %d, state: %s\n",
                  ifName, protinfo, br_port_state_names[protinfo]);
            }
         }
      }
   }

   close(fd);

   return 0;
}
