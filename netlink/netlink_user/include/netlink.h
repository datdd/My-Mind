#define NETLINK_USER 31

struct nl_msg {
    int msg_type;
    char data[256];
};

void init_netlink_socket();
void send_netlink_message(struct nl_msg *msg);
void receive_netlink_message();
