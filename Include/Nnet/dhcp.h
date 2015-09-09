/*************************************************************************/
/*                                                                       */
/*    Copyright (c) 1993 - 1996 by Accelerated Technology, Inc.  NET 2.3 */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
#ifndef _DHCP_H_
#define _DHCP_H_

#define IOSIZE                          1500 /* in/out buffer size */
#define STARTING_DELAY                  3 /* delay in seconds to start with */

#define NEXT_OPTION                     0       
#define STOP_PROCESSING                 1
#define ACCEPT_BIT                      0x01
#define DECLINE_BIT                     0x02

#define BOOTREQUEST                     1
#define BOOTREPLY                       2
/* RFC 2132 Vendor Extensions. */
#define DHCP_PAD                        0
#define DHCP_NETMASK                    1
#define DHCP_TIME_OFFSET                2
#define DHCP_ROUTE                      3
#define DHCP_TIME                       4
#define DHCP_NAME_SERVER                5
#define DHCP_DNS                        6
#define DHCP_LOG_SERVER                 7
#define DHCP_COOKIE_SERVER              8
#define DHCP_LPR_SERVER                 9
#define DHCP_IMPRESS_SERVER             10
#define DHCP_RESOURCE_SERVER            11
#define DHCP_HOSTNAME                   12
#define DHCP_BOOT_FILE_SIZE             13
#define DHCP_MERIT_DUMP_FILE            14
#define DHCP_DOMAIN_NAME                15
#define DHCP_SWAP_SERVER                16
#define DHCP_ROOT_PATH                  17
#define DHCP_EXTENSIONS_PATH            18
/* IP Layer Parameters per Host. */
#define DHCP_IP_FORWARDING              19
#define DHCP_NL_SOURCE_ROUTING          20
#define DHCP_POLICY_FILTER              21
#define DHCP_MAX_DARAGRAM_SIZE          22
#define DHCP_IP_TIME_TO_LIVE            23
#define DHCP_MTU_AGING_TIMEOUT          24
#define DHCP_MTU_PLATEAU_TABLE          25
/* IP Layer Parameters per Interface. */
#define DHCP_INTERFACE_MTU              26
#define DHCP_ALL_SUBNETS                27
#define DHCP_BROADCAST_ADDR             28
#define DHCP_MASK_DISCOVERY             29
#define DHCP_MASK_SUPPLIER              30
#define DHCP_ROUTER_DISCOVERY           31
#define DHCP_ROUTER_SOLICI_ADDR         32
#define DHCP_STATIC_ROUTE               33
/* Link Layer Parameters per Interface. */
#define DHCP_TRAILER_ENCAP              34
#define DHCP_ARP_CACHE_TIMEOUT          35
#define DHCP_ETHERNET_ENCAP             36
/* TCP Parameters. */
#define DHCP_TCP_DEFAULT_TTL            37
#define DHCP_TCP_KEEPALIVE_TIME         38
#define DHCP_TCP_KEEPALIVE_GARB         39
/* Application and Service Parameters. */
#define DHCP_NIS_DOMAIN                 40
#define DHCP_NIS                        41
#define DHCP_NTP_SERVERS                42
#define DHCP_VENDOR_SPECIFIC            43
#define DHCP_NetBIOS_NAME_SER           44
#define DHCP_NetBIOS_DATA_SER           45
#define DHCP_NetBIOS_NODE_TYPE          46
#define DHCP_NetBIOS_SCOPE              47
#define DHCP_X11_FONT_SERVER            48
#define DHCP_X11_DISPLAY_MGR            49

#define DHCP_NIS_PLUS_DOMAIN            64
#define DHCP_NIS_PLUS_SERVERS           65
#define DHCP_MOBILE_IP_HOME             68
#define DHCP_SMTP_SERVER                69
#define DHCP_POP3_SERVER                70
#define DHCP_NNTP_SERVER                71
#define DHCP_WWW_SERVER                 72
#define DHCP_FINGER_SERVER              73
#define DHCP_IRC_SERVER                 74
#define DHCP_STREETTALK_SERVER          75
#define DHCP_STDA_SERVER                76

/* DHCP Extensions */
#define DHCP_REQUEST_IP                 50
#define DHCP_IP_LEASE_TIME              51
#define DHCP_OVERLOAD                   52
#define DHCP_MSG_TYPE                   53
#define DHCP_SERVER_ID                  54
#define DHCP_REQUEST_LIST               55
#define DHCP_MESSAGE                    56
#define DHCP_MAX_MSG_SIZE               57
#define DHCP_RENEWAL_T1                 58
#define DHCP_REBINDING_T2               59
#define DHCP_VENDOR_CLASS_ID            60
#define DHCP_CLIENT_CLASS_ID            61

#define DHCP_TFTP_SERVER_NAME           66
#define DHCP_BOOT_FILE_NAME             67

#define DHCP_END                        255

#define DHCPDISCOVER                    1
#define DHCPOFFER                       2
#define DHCPREQUEST                     3
#define DHCPDECLINE                     4
#define DHCPACK                         5
#define DHCPNAK                         6
#define DHCPRELEASE                     7
#define DHCPINFORM                      8

#define ETHERNET10                      1
#define MACSIZE                         6
#define SUBNET_SIZE                     3
#define RETRIES_COUNT                   5
#define ARP_PACKET_SIZE                 64

/*
 * UDP port numbers, server and client.
 */
#define IPPORT_DHCPS                    67
#define IPPORT_DHCPC                    68
#define LARGEST_OPT_SIZE                255
#define MAX_DHCP_TIMEOUT                63
/* magic cookie value that mark the begining of vendor options part */
#define DHCP_COOKIE                     { 0x63, 0x82, 0x53, 0x63 }  

struct _DHCPLAYER
{
	uchar  dp_op;                           /* packet opcode type */
	uchar  dp_htype;                        /* hardware addr type */
	uchar  dp_hlen;                         /* hardware addr length */
	uchar  dp_hops;                         /* gateway hops */
	ulint  dp_xid;                          /* transaction ID */
	ushort dp_secs;                         /* seconds since boot began */
	ushort dp_flags;
	struct id_struct dp_ciaddr;             /* client IP address */
	struct id_struct dp_yiaddr;             /* 'your' IP address */
	struct id_struct dp_siaddr;             /* server IP address */
	struct id_struct dp_giaddr;             /* gateway IP address */
	uchar  dp_chaddr[16];                   /* client hardware address */
	uchar  dp_sname[64];                    /* server host name */
	uchar  dp_file[128];                    /* boot file name */
	uchar  dp_vend[275];                    /* vendor-specific area */
						/* as of RFC2131 it is variable length */
};
typedef struct _DHCPLAYER HUGE DHCPLAYER;

/* The following struct is used to pass in information to the NU_Dhcp    */
/* function call and also used to send information back to caller from   */
/* the DHCP server.                                                      */
/* In RFC2131 and  RFC2132, the vendor's option field has become a       */
/* variable length field.  The optfunc function pointer if non-null is   */
/* called for each option found in the vendor option field.  The values  */
/* passed to the optfunc function is device pointer, opcode, length &    */
/* pointer to value r=receive from server, s=send to server              */

struct dhcp_struct {
        uint8 dhcp_ip_addr[4];                       /* r/s new IP address of client. */
        uint8 dhcp_mac_addr[6];                      /* r MAC address of client. */
        uint8 dhcp_sname[64];                        /* r optional server host name field. */
        uint8 dhcp_file[128];                        /* r fully pathed filename */
        uint8 dhcp_siaddr[4];                        /* r DHCP server IP address */
        uint8 dhcp_giaddr[4];                        /* r Gateway IP address */
        uint8 dhcp_yiaddr[4];                        /* r Your IP address */
        uint8 dhcp_net_mask[4];                      /* r Net mask for new IP address */
        uint8 dhcp_server_id[4];                     /* r server ID being used. */
				/* Some DHCP servers do not send the final ACK packet, use */
				/* the flag below to control whether NU_Dhcp should wait */
				/* for ACK packet.  (Windows NT servers are one) */
        uint8 *dhcp_opts;            /* s options to be added to discover packet */
				/*   See RFC 2132 for valid options. */
        uint16 dhcp_opts_len;        /* s length in octets od opts field */
				/* If this function pointer is non null then it is called */
				/* for each DHCPOFFER packet received for a DHCPDISCOVER */
				/* packet. Function must return TRUE if DHCPOFFER is to be */
				/* accepted else discard and wait for another DHCPOFFER. */
        int (*dhcp_valfunc)(struct dhcp_struct *, CHAR *, const DHCPLAYER *);
				/* If this function pointer is non null then it is called */
				/* for each vendor option in the DHCPOFFER packet. */
        int (*dhcp_optfunc)(CHAR *, uint8, uint16, uint8 *);
};

typedef struct dhcp_struct DHCP_STRUCT;

/* The function prototypes known to the outside world. */
STATUS NU_Dhcp(DHCP_STRUCT *, CHAR *);
STATUS NU_Dhcp_Release(DHCP_STRUCT *, CHAR *);

#define DHCP_MAX_HEADER          (sizeof (DLAYER) + sizeof (IPLAYER) + sizeof (UDPLAYER) + sizeof (DHCPLAYER) )
#define DHCP_MAX_HEADER_SIZE     (DHCP_MAX_HEADER + (DHCP_MAX_HEADER % 4))

/* defined in the target.h file, in the Nucleus NET source code. */
#if(DHCP_VALIDATE_CALLBACK)
int dhcp_Validate( DHCP_STRUCT *, CHAR *, const DHCP *);
#endif
#if(DHCP_VENDOR_OPTS_CALLBACK)
int dhcp_Vendor_Options( CHAR *, uint8, uint16, uint8 *);
#endif

#endif
