
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/ethernet.h>
#include <linux/netlink.h>

#include "prowlan_var.h"
#include "prowlan_ipc.h"

#define MAX_PAYLOAD 512

struct prowlan_nlattr_hdr {
	u_int16_t nla_len;
	u_int16_t nla_type;
}__attribute__ ((packed));

static int
netlink_event_send(const char *ifname, int group, int cmd, struct prowlan_req_data *wlanreq, char *extra)
{
	int sock_fd;
	struct sockaddr_nl from, to;
	struct nlmsghdr *h;
	struct iovec iov;
	struct msghdr nlinkmsg;
	struct prowlan_ifinfomsg *ifihdr;
	struct prowlan_nlattr_hdr *hdr;
	struct prowlan_event *event;
	char *ptr;
	size_t ifi_len, event_len, hdr_len;
	int bytes;

	/* FROM */
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    memset(&from, 0, sizeof(from));
    from.nl_family = AF_NETLINK;
    from.nl_pid = getpid(); /* self pid */
    from.nl_groups = PROWLANMGRP_WIRELESS;
    bind(sock_fd, (struct sockaddr*) &from, sizeof(from));

	/* TO */
	memset(&to, 0, sizeof(to));
	to.nl_family = AF_NETLINK;
	to.nl_pid = getpid();
	to.nl_groups = PROWLANMGRP_WIRELESS; /* multicast */

	/** Message format
	 *  ----------------------------------------------
	 *  | Netlink Header | ZyXEL Header | Attributes |
	 *  ----------------------------------------------
	 */

	/** Netlink Header
	 *      32       16     16        32      32
 	 *  -------------------------------------------
	 *  | Length  | Type | Flags | Sequence | PID |
	 *  -------------------------------------------
	 */

	h = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (NULL == h) {
		printf("Error on malloc: %s(%d)\n", strerror(errno), errno);
		return -1;	
	}

	/* Fill the netlink message header */
	h->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	h->nlmsg_type = PROWLANM_WIRELESS_EVENT;
	h->nlmsg_pid = getpid();  /* self pid */
	h->nlmsg_flags = 0; /* not used */

	iov.iov_base = (void *)h;
	iov.iov_len = h->nlmsg_len;

	memset(&nlinkmsg, 0 ,sizeof(nlinkmsg));
	nlinkmsg.msg_name = (void *)&to;
	nlinkmsg.msg_namelen = sizeof(to);
	nlinkmsg.msg_iov = &iov;
	nlinkmsg.msg_iovlen = 1;

	/** ZyXEL Header
     *      (15-16) * 8       16         32
	 * ------------------------------------------
     * | ifi_name[15-16] | ifi_type | ifi_index |
     * ------------------------------------------
	 */

	/* 1. assign prowlan_ifinfomsg */
	ifi_len = sizeof(struct prowlan_ifinfomsg);
	ifihdr = (struct prowlan_ifinfomsg *) malloc(ifi_len);
	if (NULL == ifihdr) {
		printf("Error on malloc: %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	memset(ifihdr, 0, ifi_len);
	strncpy(((struct prowlan_ifinfomsg *)ifihdr)->ifi_name, ifname, sizeof (((struct prowlan_ifinfomsg *)ifihdr)->ifi_name));
	((struct prowlan_ifinfomsg *)ifihdr)->ifi_type = 0;
	((struct prowlan_ifinfomsg *)ifihdr)->ifi_index = 0;

	/** Attributes
     *     32      sizeof(struct prowlan_event)
	 * -----------------------------------------
	 * | nlattr |             value            |
	 * -----------------------------------------
	 */

	/** nlattr
     *      16        16
     * ----------------------
     * | nla_len | nla_type |
     * ----------------------
     */
	/* 2. assign attributes of nlattr */
	hdr_len = sizeof(struct prowlan_nlattr_hdr);
	hdr = (struct prowlan_nlattr_hdr *) malloc(hdr_len);
	if (NULL == hdr) {
		printf("Error on malloc: %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	/* Fill header */
	memset(hdr, 0, hdr_len);
	hdr->nla_type = PROWLANM_ATTR_WIRELESS;

	/** value
     *  prowlan_event_hdr  prowlan_req_data
     * ------------------------------------
     * |        hdr      |      data      |
     * ------------------------------------
     */
	/* 3. assign attribute of value */
	event_len = sizeof(struct prowlan_event) + wlanreq->hdr.ext_len;
	event = (struct prowlan_event *) malloc(event_len);
	if (NULL == event) {
		printf("Error on malloc: %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	/* Fill event */
	memset(event, 0, event_len);
	event->hdr.len = event_len;
	event->hdr.cmd = cmd;
	memcpy(&event->data, wlanreq, wlanreq->hdr.len);
	if (wlanreq->hdr.ext_len > 0) {
		memcpy(&event->data + wlanreq->hdr.len, extra, wlanreq->hdr.ext_len);
	}
	
	/* Adjust nla_len (nlattr header length + value length) */
	hdr->nla_len = hdr_len + event_len;
	
	/* 4. attach ZyXEL Header and attributes */
	ptr = NLMSG_DATA(h);
	memcpy(ptr, ifihdr, ifi_len);
	memcpy(ptr + ifi_len, hdr, hdr_len);
	memcpy(ptr + ifi_len + hdr_len, event, event_len);

	/* 5. netlink sendmsg to user-space application */
	bytes = sendmsg(sock_fd, &nlinkmsg, 0);
	if (bytes < 0) {
		printf("error sendmsg %d\n", bytes);
	} else {
		printf("send message bytes = %d\n", bytes);
	}

	/* 6. free resources */
	if (ifihdr) {
		free(ifihdr);
	}
	if (h) {
		free(h);
	}
	if (hdr) {
		free(hdr);
	}
	if (event) {
		free(event);
	}

	if (sock_fd) {
		close(sock_fd);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	uint8 macaddr[ETHER_ADDR_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	struct prowlan_req_data ipc_req;

	ipc_req.hdr.len = sizeof(struct prowlan_req_data);
	ipc_req.hdr.ext_len = 0;
	memcpy(ipc_req.u.sta.macaddr, macaddr, ETHER_ADDR_LEN);

#if 1 // Demo
	const char ifname[] = "wifi0";
	netlink_event_send(ifname, PROWLANMGRP_WIRELESS, PROWE_MSG|PROWE_MSG_ASSOC, &ipc_req, NULL);
#endif

	return 0;
}

