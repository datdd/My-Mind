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

void init_netlink_socket() {
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); // unique process ID
    src_addr.nl_groups = 0; // not joining any multicast group

    if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
        perror("socket bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

void send_message(const char *msg) {
    struct nlmsghdr *nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(strlen(msg) + 1));
    memset(nlh, 0, NLMSG_SPACE(strlen(msg) + 1));
    nlh->nlmsg_len = NLMSG_SPACE(strlen(msg) + 1);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), msg);

    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // For Linux Kernel
    dest_addr.nl_groups = 0;

    sendto(sock_fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    free(nlh);
}

int main() {
    init_netlink_socket();
    send_message("Hello from user space");
    close(sock_fd);
    return 0;
}
