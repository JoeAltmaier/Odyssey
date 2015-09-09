/*************************************************************************/
/*                                                                       */
/*      Copyright (c) 1993 - 1998 by Accelerated Technology, Inc.        */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/*
*
*Portions of this program were written by:       */
/***************************************************************************
*                                                                          *
*      part of:                                                            *
*      TCP/IP kernel for NCSA Telnet                                       *
*      by Tim Krauskopf                                                    *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*                                                                          *
*                                                                          *
****************************************************************************
*
* 
*/
/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   PROTOCOL.H                                         NET 4.0               */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file contains the structure definitions for each type of             */
/*  protocol that this program wishes to handle.  A companion file,           */
/*  'protinit.c' initializes sample versions of each type of header,          */
/*  improving the efficiency of sending packets with constants in most        */
/*  of the fields.                                                            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*      NAME            DATE                    REMARKS                       */
/*                                                                            */
/*    Glen Johnson      04/30/96        Made some changes based on            */
/*                                      recommendations of K. Goto            */
/*                                      of Hitachi.                           */
/******************************************************************************/


#ifndef PROTOCOL_H
#define PROTOCOL_H
#ifdef PLUS
    #include "nucleus.h"
#else
    #include "nu_defs.h"
    #include "nu_extr.h"
#endif
#include "target.h"
#include "mem_defs.h"
#include "rip2.h"
#include "rtab.h"
#include "bootp.h"

#ifndef PLUS
  #define VOID                  void
  typedef int                   STATUS;
  typedef unsigned long         UNSIGNED;
#endif

/*
 * The MTU value depends on the hardware.  Ethernet has a maximum MTU of
 * 1500 bytes.
 */
#define ETHERNET_MTU    1514
#define MTU             ETHERNET_MTU

/*
 * The Maximum amount of data that can be sent in a TCP segment is some what
 * less than the MTU.  This is because a header is added to the packet
 * at each layer (TCP layer, IP layer, and Hardware layer).
 * So the max amount of data that can be sent in a segment
 * is MTU - (sizeof all headers);
 */
/* Maximum TCP message size */
#define MAX_SEGMENT_LEN  (MTU  - (sizeof(DLAYER) + sizeof(IPLAYER) + sizeof(TCPLAYER)))

/* Maximum UDP message size */
#define UMAXLEN          (MTU  - (sizeof(DLAYER) + sizeof(IPLAYER) + sizeof(UDPLAYER)))



/************************************************************************/
/*  Ethernet frames
*      All Ethernet transmissions should use this Ethernet header which
*   denotes what type of packet is being sent.
*
*   The header is 14 bytes.  The first 6 bytes are the target's hardware
*   Ethernet address, the second 6 are the sender's hardware address and
*   the last two bytes are the packet type.  Some packet type definitions
*   are included here.
*
*   the two-byte packet type is byte-swapped, PC is lo-hi, Ether is hi-lo
*/

#define  EXNS    0x0600            /* probably need swapping */
#define  EIP     0x0800
#define  EARP    0x0806
#define  ERARP   0x8035            /* I guess this is RARP */
#define  ECHAOS  0x0804

typedef struct ether
{
    uint8 dest[DADDLEN];            /* where the packet is going */
    uint8 me[DADDLEN];              /* who am i to send this packet */
    uint16 type;                    /* Ethernet packet type  */
} DLAYER;

/*************************************************************************/
/*  Dave Plummer's  Address Resolution Protocol (ARP) (RFC-826) and 
*   Finlayson, Mann, Mogul and Theimer's Reverse ARP packets.
*
*   Note that the 2 byte ints are byte-swapped.  The protocols calls for
*   in-order bytes, and the PC is lo-hi ordered.
*   
*/
#define RARPR   0x0004       /*  RARP reply, from host, needs swap */
#define RARPQ   0x0003       /*  RARP request, needs swapping */
#define ARPREP  0x0002       /*  reply, byte swapped when used */
#define ARPREQ  0x0001       /*  request, byte-swapped when used */
#define ARPPRO  0x0800       /*  IP protocol, needs swapping */

/********************* Hardware Type Defines ***************************/
#define ETHER           0x0001   /*  Ethernet hardware type, needs swapping */
#define HARDWARE_TYPE   ETHER

struct tqhdr
{
     struct tqe *flink, *blink;
};

/* Define a timer queue element for TCP and IP timer events */
struct tqe
{
        struct tqe   *flink,
                     *blink;
        struct tqhdr duplist;
        UNSIGNED  tqe_event,
                  tqe_data;
#ifdef PLUS
        UNSIGNED  duetime;
#else  /* PLUS */
        int16     duetime;
        int16     pad;

#endif /* PLUS */
        void      (*tqe_callback)();
        int32     tqe_ext_data;
};
typedef struct tqe tqe_t;

/***********************************************************************/
/*   Internet protocol
*
*/
typedef struct iph
{
        uint8  versionandhdrlen;
                                 /* I prefer to OR them myself */
                                 /* each half is four bits */
        uint8  service;          /* type of service for IP */
        uint16 tlen,             /* total length of IP packet */
               ident,            /* these are all BYTE-SWAPPED! */
               frags;            /* combination of flags and value */
        uint8  ttl,              /* time to live */
               protocol;         /* higher level protocol type */
        uint16 check;            /* header checksum, byte-swapped */
        uint8  ipsource[4],      /* IP addresses */
               ipdest[4];
} IPLAYER;


/**************************************************************************/
/*  TCP protocol
*	  define both headers required and create a data type for a typical
*	  outgoing TCP packet (with IP header)
*   
*  Note:  So far, there is no way to handle IP options fields
*	which are associated with a TCP packet.  They are mutually exclusive
*	for both receiving and sending.  Support may be added later.
*
*   The tcph and iph structures can be included in many different types of
*   arbitrary data structures and will be the basis for generic send and
*   receive subroutines later.  For now, the packet structures are optimized 
*   for packets with no options fields.  (seems to be almost all of them from
*   what I've observed.
*/

typedef struct tcph
{
    uint16 source,dest;         /* TCP port numbers, all byte-swapped */
    uint32 seq,ack;             /* sequence, ACK numbers */
    uint8  hlen,                /* length of TCP header in 4 byte words */
           flags;               /* flag fields */
    uint16 window,              /* advertised window, byte-swapped */
           check,               /* TCP checksum of whole packet */
           urgent;              /* urgent pointer, when flag is set */
} TCPLAYER;

/*
*  used for computing checksums in TCP
*/
struct pseudotcp
{
    uint32  source;     /* Source IP address. */
    uint32  dest;       /* Destination IP address. */
    uint8   z;          /* zero */
    uint8   proto;      /* protocol number */
    uint16  tcplen;     /* byte-swapped length field */
};

/* 
*  flag field definitions, first two bits undefined
*/

#define TURG	0x20
#define TACK	0x10
#define TPUSH	0x08
#define TRESET	0x04
#define TSYN	0x02
#define TFIN	0x01

struct TCP_Window
{
    uint32 nxt,                  /* sequence number, not byte-swapped */
           ack;                  /* what the other machine acked */
    int32  lasttime;             /* (signed) used for timeout checking */
    NET_BUFFER_HEADER packet_list;
    NET_BUFFER        *nextPacket;
    NET_BUFFER_HEADER ooo_list;  /* Contains out of order packets. */
    uint16 num_packets;
    uint16 size,                 /* size of window advertised */
           port,                 /* port numbers from one host or another */
           contain;              /* how many bytes in queue? */
    uint8  push;                 /* flag for TCP push */
    uint8  pad[3];               /* correcl alignment for 32 bits CPU */
};

typedef struct TCP_Window TCP_WINDOW;

struct port
{
    struct  TCP_Window  in, out;
    TCPLAYER        tcpout;         /* pre-initialized as much as possible */
    uint8           tcp_laddr[4];   /* Local IP address. */
    uint8           tcp_faddr[4];   /* Foreign IP address */
    uint32          maxSendWin;     /* Max send window. */
    RTAB_ROUTE      tp_route;       /* A cached route. */

    uint16  credit;             /* choked-down window for fast hosts */
    uint16  sendsize;           /* MTU value for this connection */
    uint16  rto;                /* retrans timeout */
    int16  suspended_for;       /* The reason for the task suspending, i.e., no
                                   buffers available, waiting for data, etc., */
    uint16 pindex;              /* added by Randy */
    uint16 portFlags;
    struct TASK_TABLE_STRUCT *task_entry;
    INT		p_socketd;		    /* The socket associated with this port. */
    int16   task_num;

    uint8  state;                /* connection state */
    int8  xmitFlag;             /* Used to indicate an timer event has been
                                   created to transmit a partial packet. */
    int8  probeFlag;            /* This flag will be set when a window probe
                                   is required. */
    int8  closeFlag;            /* This flag is set when a connection is
                                   closing. */
    int8  selectFlag;            /* This flag is set whenever a call to
                                    NU_Select results in a task timing out. */
};

typedef struct port PORT;

/***************************************************************************/
/*  Port Flags                                                             */
/*                                                                         */
#define ACK_TIMER_SET     0x0001
#define SELECT_SET        0x0002

/*************************************************************************/
/*  TCP states
*	 each connection has an associated state in the connection flow.
*	 the order of the states now matters, those less than a certain
*	 number are the "inactive" states.
*/
#define SCLOSED         1
#define SLISTEN         2
#define SSYNR           3
#define SSYNS           4
#define SEST            5
#define SFW1            6
#define SFW2            7
#define SCLOSING        8
#define STWAIT          9
#define SCWAIT          10
#define SLAST           11

/*************************************************************************/
/*  UDP
*   User Datagram Protocol
*   Each packet is an independent datagram, no sequencing information
*
*   UDP uses the identical checksum to TCP
*/

typedef struct udph
{
        uint16 source,
               dest;                    /* port numbers, all byte-swapped */
        uint16 length,                  /* length of packet, including hdr */
                check;                  /* TCP checksum of whole packet */
} UDPLAYER;

struct uport
{
        NET_BUFFER_HEADER      dgram_list;     /* Header for a linked list
                                                   of received datagrams. */
#ifdef PLUS
        NU_TASK                *RXTask;         /* receive task waiting on an
                                                   event */
        NU_TASK                *TXTask;         /* transmit task waiting on an
                                                   event */
#else
        int16                   RXTask;         /* receive task waiting on an
                                                   event */
        int16                   TXTask;         /* transmit task waiting on an
                                                   event */
#endif
        RTAB_ROUTE              up_route;       /* A cached route. */
        uint32                  uportFlags;     /* What type of events are
                                                   currently pending. */
        INT                     up_socketd;     /* the socket associated with 
                                                   this port. */
        uint16                  listen;         /* what port should this one
                                                   listen to? */
        uint16                  length;         /* how much data arrived in
                                                   last packet? */
        uint8                   up_laddr[4];    /* Local IP address */
        uint8                   up_faddr[4];    /* Foreign IP address */
        uint16                  up_lport;       /* Local port number */
        uint16                  up_fport;       /* Foreign port number */
        uint16                  in_dgrams;      /* The number of data grams
                                                   currently buffered */
        uint8                   out_stale;      /* have we read this packet
                                                   yet? */
        uint8                   pad;            /* correct alignment for 32
                                                   bits CPU */
};

/*  
*  events which can occur and be placed into the event queue
*/
#define NEVENTS 50

#endif  /* PROTOCOL */


