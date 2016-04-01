
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
//#include <linux/netlink.h>
#include <libnl3/netlink/msg.h>

#include "prowlan_var.h"
#include "prowlan_ipc.h"

static int
netlink_fill_wlaninfo(struct nl_msg *msg, int msg_type, int attr_type, char *data, int data_len)
{
	struct prowlan_ifinfomsg *ifi;
	struct nlmsghdr *nlh;

	nlh = nlmsg_put(msg, 0, 0, msg_type, sizeof(struct prowlan_ifinfomsg), 0);
	if (nlh == NULL)
		return -EMSGSIZE;

	ifi = nlmsg_data(nlh);
#if 0
	if (dev != NULL) {
		memcpy(&ifi->ifi_name, dev->name, IFNAMSIZ);
		ifi->ifi_type = dev->type;
		ifi->ifi_index = dev->ifindex;
	}
#endif

	/* Add the prowlan data in the netlink packet */
	NLA_PUT(msg, attr_type, data_len, data);

//	return nlmsg_end(nlh);
	return 0;

nla_put_failure:
//	nlmsg_cancel(msg, nlh);
	return -EMSGSIZE;
}

static struct nl_msg *
msg_prowlan_event(unsigned int nlgroup, int cmd, char *event, int event_len)
{
	int err, attr_type, msg_type;
	struct nl_msg *msg;

	msg = nlmsg_alloc();
	if (!msg)
		return;

	switch (PROWLANM_ATTR(cmd)) {
	case PROWLANM_ATTR_WIRELESS:
		msg_type = PROWLANM_WIRELESS_EVENT;
		attr_type = PROWLANM_ATTR_WIRELESS;
		break;
	default:
		printf("%s: can't get attribute type (%X) !!!\n", __func__, PROWLANM_ATTR(cmd));
		break;
	}

#if 1
	netlink_fill_wlaninfo(msg, msg_type, attr_type, event, event_len);
#else
	err = netlink_fill_wlaninfo(msg, msg_type, attr_type, event, event_len);
	if (err < 0) {
		printf("%s error\n", __FUNCTION__);
		return;
	}
#endif

	return msg;

//	NETLINK_CB(msg).dst_group = nlgroup;
//	skb_queue_tail(&prowlan_nlevent_queue, skb);
//	tasklet_schedule(&prowlan_nlevent_tasklet);
	return;
}

static int
prowlan_send_event(unsigned int group, unsigned int cmd, struct prowlan_req_data *wlanreq, char *extra) /* obsolete extra ? */
{
	struct prowlan_event *event;		/* Mallocated whole event */
	int event_len;
	int bytes;

	/* Create temporary buffer to hold the event */
	event_len = sizeof(struct prowlan_event) + wlanreq->hdr.ext_len;
	event = malloc(event_len);
	if (event == NULL)
		return -1;

	memset(event, 0x00, event_len);
	/* Fill event */
	event->hdr.len = event_len;
	event->hdr.cmd = cmd;
	memcpy((&event->data), (char *) wlanreq, wlanreq->hdr.len);
	if (wlanreq->hdr.ext_len > 0)
		memcpy((((char *)&(event->data)) + wlanreq->hdr.len), (char*) extra, wlanreq->hdr.ext_len);

	printf("event_len = %d\n", event_len);

#if 1
	/* Send via the Prowlan Netlink event channel */
	struct nl_msg *msg;
	struct nl_sock *sock_fd;
	int err;

	msg = msg_prowlan_event(group, cmd, (char *) event, event->hdr.len);
	if (NULL == msg) {
		printf("msg NULL\n");
		return -1;
	}

	if (!(sock_fd = nl_socket_alloc())) {
		printf("Unable to allocate netlink socket\n");
		return -1;
	}

	if ((err = nl_connect(sock_fd, NETLINK_USERSOCK)) < 0) {
		nl_perror(err, "Unable to connect socket");
		nl_socket_free(sock_fd);
		return err;
	}

	//bytes = nl_send(sock_fd, msg);
	//bytes = nl_send_auto(sock_fd, msg);
	bytes = nl_send_auto_complete(sock_fd, msg);

	printf("bytes=%d\n", bytes);

	if (bytes < 0) {
		nl_perror(bytes, "Failed to send message");
		return bytes;
	}

	nl_close(sock_fd);
	nl_socket_free(sock_fd);
#endif

	/* Cleanup */
	free(event);

	return 0;
}

int main(int argc, char *argv[])
{
	uint8 macaddr[ETHER_ADDR_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	struct prowlan_req_data ipc_req;

	ipc_req.hdr.len = sizeof(struct prowlan_req_data);
	ipc_req.hdr.ext_len = 0;
	memcpy(ipc_req.u.sta.macaddr, macaddr, ETHER_ADDR_LEN);

	prowlan_send_event(PROWLANMGRP_WIRELESS, PROWE_MSG|PROWE_MSG_ASSOC, &ipc_req, NULL);

	return 0;
}

