#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "netlink.h"

#define NETLINK_USER 31

static struct sockaddr_nl src_addr, dest_addr;
static int sock_fd;

void netlink_init() {
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); // unique PID
    src_addr.nl_groups = 0; // not joining any multicast group

    if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

void netlink_send_msg(const char *msg) {
    struct nlmsghdr *nlh;
    int msg_size = strlen(msg);
    
    nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(msg_size));
    nlh->nlmsg_len = NLMSG_SPACE(msg_size);
    nlh->nlmsg_pid = src_addr.nl_pid; // sender PID
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), msg);

    sendto(sock_fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    free(nlh);
}

void netlink_recv_msg() {
    struct nlmsghdr *nlh;
    char buffer[1024];

    recv(sock_fd, buffer, sizeof(buffer), 0);
    nlh = (struct nlmsghdr*)buffer;

    printf("Received message: %s\n", (char*)NLMSG_DATA(nlh));
}

void netlink_cleanup() {
    close(sock_fd);
}