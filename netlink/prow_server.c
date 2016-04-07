#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "prowlan_ipc.h"

#define PROWLANM_WIRELESS_EVENT	1
#define PROWLANMGRP_WIRELESS	1
#define PROWLANM_ATTR_WIRELESS	0x8000

#define NLA_OK(nla,len) \
	((len) > 0 && (nla)->nla_len >= sizeof(struct nlattr) && \
		(nla)->nla_len <= (len))
#define NLA_NEXT(nla,attrlen) \
		((attrlen) -= NLA_ALIGN((nla)->nla_len), \
			(struct nlattr *) (((char *)(nla)) + NLA_ALIGN((nla)->nla_len)))

#define GET_TYPE(date)  (0xFF00 & date)
#define GET_SUBTYPE(date)   (0x00FF & date)

int initSocket(void)
{
	int sock;
	struct sockaddr_nl local;

	sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (sock < 0) {
		perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK)");
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

	return sock;
}

int eventRecv(char *e, char *ifi_name)
{
	struct prowlan_event *event;
	struct prowlan_req_data *prowlan_req;

	if(e)
	{
		event = (struct prowlan_event *) e;
		prowlan_req = &event->data;
	}

	printf("in %s, e->len %d, e->cmd 0x%X\n", __FUNCTION__, event->hdr.len, event->hdr.cmd);

	switch (GET_TYPE(event->hdr.cmd)) {
		case PROWE_MSG:
			switch (GET_SUBTYPE(event->hdr.cmd)) {
				case PROWE_MSG_ASSOC:
					printf("receive message PROWE_MSG_ASSOC\n");
					break;
				default:
					break;
			}
		default:
			break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	int left;
	char buf[1024];
	int attrlen, nlmsg_len ;
	struct prowlan_ifinfomsg *ifinfomsg;
	struct nlattr *attr;

	if ((sock = initSocket()) < 0) {
 		return 0;
	}

#if 0
    if (daemon(1, 1) < 0) {
        printf("daemon failed\n");
        return -1;
    }
#endif

	fromlen = sizeof(from);
	while (1) {
		left = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *) &from, &fromlen);

		printf("left=%d\n", left);

		if (left > 0) {
			h = (struct nlmsghdr *) buf;
			while (left >= (int) sizeof(*h)) {
				int len, plen;
				len = h->nlmsg_len;
				plen = len - sizeof(*h);

				if (len > left || plen < 0) {
					printf("in %s, Malformed netlink message: len=%d left=%d plen=%d\n", __FUNCTION__, len, left, plen);
					break;
				}

				switch (h->nlmsg_type) {
					case PROWLANM_WIRELESS_EVENT:

						nlmsg_len = NLMSG_ALIGN(sizeof(struct prowlan_ifinfomsg));
						ifinfomsg = (struct prowlan_ifinfomsg *) (((char *) NLMSG_DATA(h)));

						attrlen = h->nlmsg_len - nlmsg_len;

						attr = (struct nlattr *) (((char *) NLMSG_DATA(h)) + nlmsg_len);

						printf("in %s, type(%x), subtype(%x), nla_len (%x), nla_type (%x)\n",
									__FUNCTION__, GET_TYPE(attr->nla_type), GET_SUBTYPE(attr->nla_type), attr->nla_len, attr->nla_type);

						while (NLA_OK(attr, attrlen)) {
							printf("NLA_OK\n");
							if (attr->nla_type == PROWLANM_ATTR_WIRELESS) {
								eventRecv((((char*)attr) + NLA_ALIGN(sizeof(struct nlattr))),ifinfomsg->ifi_name);
							}
							attr = NLA_NEXT(attr, attrlen);
						}

						break;
					default:
						printf("in %s, nlmsg_type(%x), type(%x), subtype(%x)\n",
									__FUNCTION__, h->nlmsg_type, GET_TYPE(h->nlmsg_type), GET_SUBTYPE(h->nlmsg_type));
						break;
				}

				len = NLMSG_ALIGN(len);
				left -= len;
				h = (struct nlmsghdr *) ((char *) h + len);
			}
		}
	}

	return 0;
}

