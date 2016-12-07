
#include <net/if.h>

/* ----------------------- PROWLAN EVENT TYPE ----------------------- */
#define PROWE_MSG   0x8000  /* Control Message */
#define PROWE_MSG_ASSOC   0x0001 /* Association/Reassociation */
#define PROWE_MSG_DIASSOC 0x0002 /* Disassociation */
#define PROWE_MSG_DEAUTH  0x0003 /* DeAuth */
#define PROWE_MSG_UPLINK_CONNECTED  0x0004 /* For ZyMesh, When VAP(station mode) connected to up link send notify ZyMesh app */
#define PROWE_MSG_UPLINK_DISCONNECTED   0x0005 /* For ZyMesh, When VAP(station mode) disconnected to up link send notify ZyMesh app */

#define PROWE_INFO  0x8100  /* Configure Infomation */
#define PROWE_INFO_STAIP   0x0001 /* mac + extra ip */
#define PROWE_INFO_CHANNEL 0x0002 /* Channel */
#define PROWE_INFO_SSID 0x0003 /* SSID */
#define PROWE_INFO_SECURITY 0x0004 /* security mode */
#define PROWE_INFO_ROLE 0x0005 /* wlan mode */
#define PROWE_INFO_DEL 0x0006 /* VAP delete */
#define PROWE_INFO_VAP 0x0007

#define PROWE_UTIL  0x8200  /* UTIL */
#define PROWE_UTIL_RESET   0x0001 /* Reset DUT's Config to default*/
#define PROWE_UTIL_REBOOT   0x0002 /* Reboot DUT */
#define PROWE_UTIL_RESETIF  0x0003 /* UPDOWN VAP */
#define PROWE_UTIL_BB_PANIC  0x0004 /* BB panic */
#define PROWE_UTIL_SETUP_RADIO  0x0005 /* Set up wifiX VAPs.
                                          For target assert, wifi1 VAPs should be re-established
                                          after successful target assert recovery */

#define PROWE_OFFLOAD_EVENT  0x8300  /* Offload engine event */
#define PROWE_OFFLOAD_EVENT_BMISS_TIMEOUT   0x0001 /* Beacon miss timeout event */
#define PROWE_OFFLOAD_EVENT_BMISS_DETECT    0x0002 /* Beacon miss detect event */
#define PROWE_OFFLOAD_EVENT_PN_CHECK_FAILED     0x0003 /* PN check failed */

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed int int8;

struct prowlan_ifinfomsg
{
	char	ifi_name[IFNAMSIZ];
	unsigned short	ifi_type;		/* ARPHRD_* */
	int		ifi_index;		/* Link index	*/
};

/* ----------------------- WIRELESS EVENTS ----------------------- */
/*
 * Wireless events are carried through the rtnetlink socket to user
 * space. They are encapsulated in the IFLA_WIRELESS field of
 * a RTM_NEWLINK message.
 */

struct prowlan_req_hdr {
    uint16 len;  /* u data length */
    uint16 ext_len;  /* extra data length */
};

struct prowlan_sta_info {
    uint16 vid;
    uint8 macaddr[6];
};

struct prowlan_chann_info {
    uint32 freq; /* Setting in MHz */
    uint8 ieee; /* IEEE Channel number */
    uint8 reason; /* freq set reason */
    int8 cw_extoffset; /* channel width extern channel offset */
    uint8 vht_ch_freq; /* 11ac 80MHz channel center frequency*/
};

struct prowlan_ssid_info {
    uint32 len;              /* length in bytes */
    uint8 name[32];  /* ssid contents, max length is 32 that include "/0" */
};

struct prowlan_op_mode {
    uint16 opmode;   /* wlan role */
    uint16 flags;
};

struct prowlan_uplink_status {
    uint16 status;
    uint8 macaddr[6];
    char    ifi_name[IFNAMSIZ];

};

struct prowlan_vap_info{
    uint8 macaddr[6];
};

struct prowlan_reboot_info {
    uint8 reason[64];    /* reboot reason */
};

struct prowlan_setup_radio {
    uint8 radio_num;     /* radio number. 0: wifi0, 1: wifi1. */
};

struct prowlan_req_data
{
    struct prowlan_req_hdr hdr;     /* 4 bytes */
    union{
        struct prowlan_sta_info sta;
        struct prowlan_chann_info chann;
        struct prowlan_ssid_info ssid;
        struct prowlan_ssid_info sec_mode;
        struct prowlan_op_mode wlan_mode;
        struct prowlan_uplink_status uplink_status;
        struct prowlan_vap_info vap_info;
        struct prowlan_reboot_info reboot_info;
        struct prowlan_setup_radio radio_info;
    }u;
};

struct prowlan_event_hdr {
	uint16 len;			/* Real length of this stuff */
	uint16 cmd;			/* IOCTL */
};

struct prowlan_event {
	struct prowlan_event_hdr hdr;
	struct prowlan_req_data	data;		/* IOCTL fixed payload */
};

