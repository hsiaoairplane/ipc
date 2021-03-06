
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define PROWLANMGRP_WIRELESS 1

#define MAX_PAYLOAD 1024

int main(int argc, char *argv[])
{
    struct sockaddr_nl src_addr;
	struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg;
	int bytes;

	/* From */
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (sock_fd < 0) {
		perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK)");
		return -1;
	}

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    src_addr.nl_groups = PROWLANMGRP_WIRELESS;

	if (bind(sock_fd, (struct sockaddr*) &src_addr, sizeof(src_addr)) < 0) {
		perror("bind(netlink)");
		close(sock_fd);
		return -1;
	}

	/* To */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = getpid();
    dest_addr.nl_groups = PROWLANMGRP_WIRELESS;

    nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    /* Fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;
    /* Fill in the netlink message payload */
    strcpy(NLMSG_DATA(nlh), "Hello World!");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    bytes = sendmsg(sock_fd, &msg, 0);
	if (bytes < 0) {
		printf("Error code %d\n", bytes);
	} else {
		printf("Send message bytes %d\n", bytes);
	}

    close(sock_fd);

	return 0;
}

