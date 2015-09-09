/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      RIP2.H                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      RIP2 - Routing Internet Protocol  V2.0                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Holds the defines for the RIP2 protocol.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Accelerated Technology Inc.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*  global compenent data stuctures defined in this file                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      No functions defined in this file                                */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      No other file dependencies                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME                            DATE            REMARKS          */
/*                                                                       */
/*************************************************************************/

#ifndef _RIP2_H_
#define _RIP2_H_

#include "socketd.h"
#include "target.h"

#define RIP2_TIMER_ID   100
#define ALLOCATED_ENTRY 99

#define IOSIZE          1500        /* in/out buffer size */
#define STARTING_DELAY  3           /* delay in seconds to start with */

#define NEXT_OPTION     0
#define STOP_PROCESSING 1
#define ACCEPT_BIT      0x01
#define DECLINE_BIT     0x02

#define RIP_PORT        520

#define RIP2_BTIME      30

#define BOOTREQUEST     1
#define BOOTREPLY       2

#define BCAST_IDX       0   /* broadcast index # for listening socket array */
#define MCAST_IDX       1   /* multicast index # for listening socket array */

#define ETHERNET10      1
#define MACSIZE         6
#define SUBNET_SIZE     3
#define RETRIES_COUNT   5
#define ARP_PACKET_SIZE 64

#define MAX_DELETES     25
#define MAX_PER_PACKET  25
#define DELETE_INTERVAL (180 * TICKS_PER_SECOND)

#define RIP_PKT_SIZE    20
#define RT_INFINITY     16
#define LOOPBACK_ADDR       0x7f000000
#define CLASSA_BROADCAST    0x00ffffff
#define CLASSB_BROADCAST    0x0000ffff
#define CLASSC_BROADCAST    0x000000ff

/* map to known network names */
#ifndef ntohs
#define ntohs(x)        intswap(x)
#endif
#ifndef ntohl
#define ntohl(x)        longswap(x)
#endif
#ifndef htons
#define htons(x)        intswap(x)
#endif
#ifndef htonl
#define htonl(x)        longswap(x)
#endif

/* Following defines have these meansing. */
/* SEND_NONE   - No RIP1 or 2 messages are ever broadcast or multicast */
/* SEND_RIP1   - Only RIP1 messages are ever broadcast, NO multicast */
/* SEND_RIP2   - Only RIP2 messages are ever broadcast, NO multicast */
/* SEND_BOTH   - Both RIP1 & RIP2 messages are ever broadcast, NO multicast */
/* SEND_MULTI  - Only RIP-2 messages are ever multicast, NO broadcast */
#define SEND_NONE       0x00
#define SEND_RIP1       0x01
#define SEND_RIP2       0x02
#define SEND_BOTH       (SEND_RIP1 | SEND_RIP2)
#define SEND_MULTI      0x20

/* RECV_RIP1   - Only process RIP1 packets. */
/* RECV_RIP2   - Only process RIP2 packets. */
/* RECV_BOTH   - Process both RIP1 and RIP2 packets */
#define RECV_RIP1       0x01
#define RECV_RIP2       0x02
#define RECV_BOTH       (RECV_RIP1 | RECV_RIP2)

#define RIP2_REQUEST    1
#define RIP2_RESPONSE   2
#define RIP2_TRACEON    3   /* obsolete, to be ignored */
#define RIP2_TRACEOFF   4   /* obsolete, to be ignored */
#define RIP2_RESERVED   5

#define RIP1_VERSION    1
#define RIP2_VERSION    2

#define RIP2_FFFF       0xFFFF

struct rip2_packet {
    uint8 command;
    uint8 version;          /* as of RFC1722 this can be 1 or 2 */
    uint16 unused;          /* not used in RIPv1 RFC1058 */
    uint16 af_id;           /* Address Family Identifier */
    uint16 routetag;        /* not used in RIPv1 RFC1058 */
    uint8 ip_addr[4];       /* IP Address */
    uint8 submask[4];       /* not used in RIPv1 RFC1058 */
    uint8 nexthop[4];       /* not used in RIPv1 RFC1058 */
    uint32 metric;          /* must be a value of 1 to 16 */
};
typedef struct rip2_packet RIP2_PACKET;

struct rip2_header {
    uint8 command;
    uint8 version;          /* as of RFC1722 this can be 1 or 2 */
    int16 unused;
};

typedef struct rip2_header RIP2_HEADER;

struct rip2_entry {
    uint16 af_id;           /* Address Family Identifier */
    uint16 routetag;        /* not used in RIPv1 RFC1058 */
    uint8 ip_addr[4];       /* IP address */
    uint8 submask[4];       /* not used in RIPv1 RFC1058 */
    uint8 nexthop[4];       /* not used in RIPv1 RFC1058 */
    uint32 metric;          /* must be a value of 1 to 16 */
};
typedef struct rip2_entry RIP2_ENTRY;

struct rip2_auth_entry {
    uint16 af_id;           /* will always be 0xFFFF for this type packet */
    uint16 authtype;        /* not used in RIPv1 RFC1058 */
    uint8 auth[16];         /* authication entry */
};
typedef struct rip2_auth_entry RIP2_AUTH_ENTRY;

struct rip2_authentication {
    uint16 authtype;        /* must be set to 2 */
    uint8 auth[16];         /* authication entry */
    /* the auth field must be left justified and zero padded to 16 octets */
};
typedef struct rip2_authentication RIP2_AUTH;

/* The function prototypes known to the outside world. */
int NU_Rip2(RIP2_AUTH *);

#endif
