/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993 - 1998 by Accelerated Technology, Inc.           */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*      bootp                                                 4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will handle bootstrap protocol defines -- RFC 951.*/
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Craig L. Meredith, Accelerated Technology Inc.                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  global compenent data stuctures defined in this file                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      No functions defined in this file                                   */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      No other file dependencies                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Craig L. Meredith       04/10/93        Initial version.            */
/*      Craig L. Meredith       08/17/93        Added header, Neil's mods.  */
/*      Donald R. Sharer        03/20/98        Modified for Net 4.0.       */
/*                                                                          */
/****************************************************************************/

#ifndef BOOTP_H
#define BOOTP_H

#include "socketd.h"
#include "target.h"

#define BOOTREQUEST     1
#define BOOTREPLY       2
/*
 * UDP port numbers, server and client.
 */
#define IPPORT_BOOTPS       67
#define IPPORT_BOOTPC       68

#define VM_STANFORD     "STAN"  /* v_magic for Stanford */

#define VM_RFC1048      "\143\202\123\143"


/*  BOOTP Vendor Extensions  */

#define BOOTP_PAD       0
#define BOOTP_SUBNET    1
#define BOOTP_TIMEOFF   2
#define BOOTP_GATEWAY   3
#define BOOTP_TIMESERV  4
#define BOOTP_NAMESERV  5
#define BOOTP_DOMNAME   6
#define BOOTP_LOGSERV   7
#define BOOTP_COOKSRV   8
#define BOOTP_LPRSRV    9
#define BOOTP_IMPRSRV   10
#define BOOTP_RLPSRV    11
#define BOOTP_HOSTNAME  12
#define BOOTP_BFILSZ    13
#define BOOTP_END       255
#define BOOTP_COOKIE    { 0x63,0x82, 0x53, 0x63 }      
#define LARGEST_OPT_SIZE                255
#define MAX_BOOTP_TIMEOUT                63

/* v_flags values */
#define VF_PCBOOT       1              /* an IBMPC or Mac wants environment info */
#define VF_HELP         2              /* help me, I'm not registered */
#define TAG_BOOTFILE_SIZE       13     /* tag used by vend fields rfc 1048 */

#define BOOTP_RETRIES       6    /* The maximum number of times bootp will send
				  * a request before giving up.     */
#define MAX_BOOTP_TIMEOUT   63   /* The maximum time bootp will wait for a
				  * response before retransmitting a request. */
typedef struct bootp HUGE BOOTPLAYER;
struct bootp
{
	uchar  bp_op;      /* packet opcode type */
	uchar  bp_htype;   /* hardware addr type */
	uchar  bp_hlen;    /* hardware addr length */
	uchar  bp_hops;    /* gateway hops */
	ulint  bp_xid;     /* transaction ID */
	ushort bp_secs;    /* seconds since boot began */
	ushort bp_unused;
	struct id_struct bp_ciaddr;  /* client IP address */
	struct id_struct bp_yiaddr;  /* 'your' IP address */
	struct id_struct bp_siaddr;  /* server IP address */
	struct id_struct bp_giaddr;  /* gateway IP address */
	uchar  bp_chaddr[16];  /* client hardware address */
	uchar  bp_sname[64];   /* server host name */
	uchar  bp_file[128];   /* boot file name */
	uchar  bp_vend[64];    /* vendor-specific area */
};


struct bootp_struct {
        uint8 bp_ip_addr[4];                       /* r/s new IP address of client. */
        uint8 bp_mac_addr[6];                      /* r MAC address of client. */
        uint8 bp_sname[64];                        /* r optional server host name field. */
        uint8 bp_file[128];                        /* r fully pathed filename */
        uint8 bp_siaddr[4];                        /* r DHCP server IP address */
        uint8 bp_giaddr[4];                        /* r Gateway IP address */
        uint8 bp_yiaddr[4];                        /* r Your IP address */
        uint8 bp_net_mask[4];                      /* r Net mask for new IP address */
};

typedef struct bootp_struct BOOTP_STRUCT;

/*
 * "vendor" data permitted for Stanford boot clients.
 */
struct vend
{
	uchar  v_magic[4]; /* magic number */
	ulint  v_flags;    /* flags/opcodes, etc. */
	uchar  v_unused[56];   /* currently unused */
};
#define BOOTP_MAX_HEADER          (sizeof (DLAYER) + sizeof (IPLAYER) + sizeof (UDPLAYER) + sizeof (BOOTPLAYER) )
#define BOOTP_MAX_HEADER_SIZE     (BOOTP_MAX_HEADER + (BOOTP_MAX_HEADER % 4))

#endif  /* BOOTP_H */
