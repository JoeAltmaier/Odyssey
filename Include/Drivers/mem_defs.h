/******************************************************************************/
/*                                                                            */
/*      Copyright (c) 1993 - 1999 by Accelerated Technology, Inc.             */
/*                                                                            */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the           */
/* subject matter of this material.  All manufacturing, reproduction,         */
/* use, and sales rights pertaining to this subject matter are governed       */
/* by the license agreement.  The recipient of this software implicitly       */
/* accepts the terms of the license.                                          */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   MEM_DEFS.H                                           4.0                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file contains the linked list structure definitions used by NET.     */
/*  These lists are used for buffering of incoming and outgoing packets.      */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*  Uriah T. Pollock                                                          */
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
/*      NAME                DATE                  REMARKS                     */
/*                                                                            */
/*    Uriah T. Pollock    12/22/97      Created Initial Version.              */
/*    Uriah T. Pollock    01/18/99      Changed the placement of fields in the*/
/*                                       buffer structure. Done to support the*/
/*                                       DEC21143 driver.                     */
/*                                                                            */
/******************************************************************************/

#ifndef MEM_DEFS_H
#define MEM_DEFS_H

/* Flags for indicating the destination address of a packet in a buffer. */
#define NET_BCAST       0x01
#define NET_MCAST       0x02
#define NET_IP          0x04
#define NET_LCP         0x08
#define NET_IPCP        0x10
#define NET_PAP         0x20
#define NET_CHAP        0x40

typedef struct packet_queue_header NET_BUFFER_HEADER;

/* This is a definition of the largest Media Access Layer header. It is used in the 
   definitions below when deciding how far to offset into a buffer. */
#define NET_MAX_MAC_HEADER_SIZE         (sizeof(DLAYER))

/* Define the size of the header for the parent buffer header */
#define NET_MAX_TCP_HEADER              (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (TCPLAYER))
#define NET_MAX_TCP_HEADER_SIZE         (NET_MAX_TCP_HEADER + (NET_MAX_TCP_HEADER % 4))

#define NET_MAX_UDP_HEADER              (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (UDPLAYER))
#define NET_MAX_UDP_HEADER_SIZE         (NET_MAX_UDP_HEADER + (NET_MAX_UDP_HEADER % 4))

#define NET_MAX_ARP_HEADER              (NET_MAX_MAC_HEADER_SIZE + sizeof (ARP_LAYER))
#define NET_MAX_ARP_HEADER_SIZE         (NET_MAX_ARP_HEADER  + (NET_MAX_ARP_HEADER % 4))

#define NET_MAX_ICMP_HEADER             (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (ICMP_LAYER))
#define NET_MAX_ICMP_HEADER_SIZE        (NET_MAX_ICMP_HEADER  + (NET_MAX_ICMP_HEADER % 4))

#define NET_PPP_HEADER_OFFSET           NET_MAX_MAC_HEADER_SIZE
#define NET_PPP_HEADER_OFFSET_SIZE      (NET_PPP_HEADER_OFFSET  + (NET_PPP_HEADER_OFFSET % 4))

#define NET_ETHER_HEADER_OFFSET         NET_MAX_MAC_HEADER_SIZE
#define NET_ETHER_HEADER_OFFSET_SIZE    (NET_ETHER_HEADER_OFFSET  + (NET_ETHER_HEADER_OFFSET % 4))


#define NET_SLIP_HEADER_OFFSET          NET_MAX_MAC_HEADER_SIZE
#define NET_SLIP_HEADER_OFFSET_SIZE     (NET_SLIP_HEADER_OFFSET  + (NET_SLIP_HEADER_OFFSET % 4))

#define NET_MAX_IP_HEADER             (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER))
#define NET_MAX_IP_HEADER_SIZE        (NET_MAX_IP_HEADER + (NET_MAX_IP_HEADER % 4))

#define NET_MAX_IGMP_HEADER            (NET_MAX_MAC_HEADER_SIZE + sizeof (IPLAYER) + sizeof (IGMP_LAYER))
#define NET_MAX_IGMP_HEADER_SIZE       (NET_MAX_IGMP_HEADER + (NET_MAX_IGMP_HEADER % 4))

#define NET_FREE_BUFFER_THRESHOLD       74
#define NET_MAX_BUFFER_SIZE             1584
#define NET_PARENT_BUFFER_SIZE          (NET_MAX_BUFFER_SIZE - sizeof(struct _me_bufhdr))


typedef struct packet_queue_element HUGE   NET_BUFFER;

struct _me_bufhdr
{
    int32                       seqnum;
    NET_BUFFER_HEADER           *dlist;
    struct _DV_DEVICE_ENTRY     *buf_device;
    uint16                      option_len;
    int16                       retransmits;
    uint16                      flags;
    uint16                      tcp_data_len;           /* size of the data in a TCP packet. */
    uint32                      total_data_len;         /* size of the entire buffer,
                                                                   sum of all in the chain    */
};

#if 0
/* Define the queue element used to hold a packet */
struct packet_queue_element
{
    NET_BUFFER                          *next;        /* next buffer chain in the list */
    NET_BUFFER                          *next_buffer; /* next buffer in this chain */
    uint8                       HUGE    *data_ptr;
    uint32                              data_len;     /* size of this buffer */
    union
    {
        struct _me_pkthdr
        {
            struct  _me_bufhdr      me_buf_hdr;
            uint8                   parent_packet[NET_PARENT_BUFFER_SIZE];
        } me_pkthdr;

        uint8 packet[NET_MAX_BUFFER_SIZE];

    } me_data;
};
#endif

/* Define the queue element used to hold a packet */
struct packet_queue_element
{
    union
    {
        struct _me_pkthdr
        {
            uint8                   parent_packet[NET_PARENT_BUFFER_SIZE];
            struct  _me_bufhdr      me_buf_hdr;
        } me_pkthdr;

        uint8 packet[NET_MAX_BUFFER_SIZE];

    } me_data;

    NET_BUFFER                          *next;        /* next buffer chain in the list */
    NET_BUFFER                          *next_buffer; /* next buffer in this chain */
    uint8                       HUGE    *data_ptr;
    uint32                              data_len;     /* size of this buffer */
};


/* These definitions make it easier to access fields within a packet. */
#define mem_seqnum              me_data.me_pkthdr.me_buf_hdr.seqnum
#define mem_dlist               me_data.me_pkthdr.me_buf_hdr.dlist
#define mem_buf_device          me_data.me_pkthdr.me_buf_hdr.buf_device
#define mem_option_len          me_data.me_pkthdr.me_buf_hdr.option_len
#define mem_retransmits         me_data.me_pkthdr.me_buf_hdr.retransmits
#define mem_flags               me_data.me_pkthdr.me_buf_hdr.flags
#define mem_tcp_data_len        me_data.me_pkthdr.me_buf_hdr .tcp_data_len
#define mem_total_data_len      me_data.me_pkthdr.me_buf_hdr.total_data_len
#define mem_parent_packet       me_data.me_pkthdr.parent_packet
#define mem_packet              me_data.packet

/* Define the header for the buffer queue */
struct packet_queue_header
{
     NET_BUFFER *head;
     NET_BUFFER *tail;
};

/* Define a generic queue header */
struct queue_header
{
    struct queue_element *head, *tail;
};
typedef struct queue_header NET_QUEUE_HEADER;

/* Define a generic queue element */
struct queue_element
{
        struct queue_element *next, *next_buffer;
};
typedef struct queue_element NET_QUEUE_ELEMENT;

/* Define the buffer suspension list structure. This list will
   hold tasks that are waiting to transmit because of lack of
   memory buffers. */
struct _mem_suspension_element
{
    struct _mem_suspension_element *flink;
    struct _mem_suspension_element *blink;
    NU_TASK                        *waiting_task;
};

struct _mem_suspension_list
{
    struct _mem_suspension_element *head;
    struct _mem_suspension_element *tail;
};

typedef struct _mem_suspension_list     NET_BUFFER_SUSPENSION_LIST;
typedef struct _mem_suspension_element  NET_BUFFER_SUSPENSION_ELEMENT;

/* Global data structures declared in MEM.C */
extern uint16                       MEM_Buffers_Used;
extern NET_BUFFER_SUSPENSION_LIST   MEM_Buffer_Suspension_List;

#endif
