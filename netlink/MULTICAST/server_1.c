#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define PROWLANMGRP_WIRELESS 1
#define MAX_PAYLOAD 1024

int main(int argc, char *argv[])
{
	struct sockaddr_nl local;
	struct msghdr msg;
	struct iovec iov;
	struct nlmsghdr *nlh = NULL;
	int sock;
	int bytes;

    sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    if (sock < 0) {
        perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_USERSOCK)");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
	local.nl_pid = getpid();
    local.nl_groups = PROWLANMGRP_WIRELESS;
    if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
        perror("bind(netlink)");
        close(sock);
        return -1;
    }

    if (daemon(1, 1) < 0) {
        printf("daemon failed\n");
        return -1;
    }

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	/* Fill the netlink message header */ 
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid(); /* self pid */ 
	nlh->nlmsg_flags = 0;

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&(local);
	msg.msg_namelen = sizeof(local);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while (1) {
		bytes = recvmsg(sock, &msg, 0);
		if (bytes < 0) {
			printf("recvmsg error: %d\n", bytes);
		}

		printf("Server 1 => Received message payload: %s\n", (char *) NLMSG_DATA(msg.msg_iov->iov_base));
    }

    return sock;
}

