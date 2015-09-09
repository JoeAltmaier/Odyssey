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
/*      ip                                                    4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will handle defines relating to the IP layer.     */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Accelerated Technology Inc.                                         */
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
/****************************************************************************/


#include "protocol.h"
#include "rip2.h"
#include "rtab.h"
#include "mem_defs.h"
#ifndef IP_H
#define IP_H

typedef struct IP_FRAG_STRUCT {
    uint8                   ipf_hl;        /* version and header length */
    uint8                   ipf_mff;       /* overlays IP TOS: use low bit to 
                                              avoid destroying tos; copied from
                                              (ip service & IP_MF) */
    uint16                  ipf_tlen;         
    uint16                  ipf_id;         
    uint16                  ipf_off;       /* Contains only the fragment offset,
                                              unlike the normal IP field which 
                                              contains offset and flags fields. 
                                            */
    uint8                   ipf_ttl;
    uint8                   ipf_buf_offset; /* The protocol field is used to 
                                               store the offset of the IP layer 
                                               from the start of the memory 
                                               buffer.
                                            */
    uint16                  ipf_check;            
    struct IP_FRAG_STRUCT   *ipf_next;
    struct IP_FRAG_STRUCT   *ipf_prev;
} HUGE IP_FRAG;

typedef struct _IP_QUEUE_ELEMENT {
    struct _IP_QUEUE_ELEMENT    *ipq_next;
    struct _IP_QUEUE_ELEMENT    *ipq_prev;
    IP_FRAG                     *ipq_first_frag;
    uint32                      ipq_source;
    uint32                      ipq_dest;
    uint16                      ipq_id;
    uchar                       ipq_protocol;
    uchar                       ipq_pad;  /* The pad is added to make this 
                                             structure an even number of long 
                                             words. 
                                           */
} IP_QUEUE_ELEMENT;

typedef struct _IP_QUEUE
{
     IP_QUEUE_ELEMENT       *ipq_head;
     IP_QUEUE_ELEMENT       *ipq_tail;
} IP_QUEUE;



struct _IP_MULTI
{
    uint32              ipm_addr;
    DV_DEVICE_ENTRY     *ipm_device;
    uint32              ipm_refcount;
    uint32              ipm_timer;
    IP_MULTI            *ipm_next;
};

#define     IP_MAX_MEMBERSHIPS          10

struct _IP_MULTI_OPTIONS
{
    DV_DEVICE_ENTRY     *ipo_device;
    IP_MULTI            *ipo_membership[IP_MAX_MEMBERSHIPS];    
    uint8               ipo_ttl;
    uint8               ipo_loop;
    uint8               ipo_num_mem;
};

/* Multicast definitions. */
#define IP_DEFAULT_MULTICAST_TTL        1  /* By default keep multicast packets 
                                              on the local segment. */
#define IP_DEFAULT_MULTICAST_LOOP       0  /* Loop back is not yet supported. */


/* RFC 1122 recommends a default ttl for fragments of 60 to 120 seconds. */
#define IP_FRAG_TTL     (TICKS_PER_SECOND * 60)


/* The following macro returns a pointer to the IP header within a buffer. */
#define IP_BUFF_TO_IP(buff_ptr) \
        (IPLAYER *)(buff_ptr->data_ptr - sizeof(IPLAYER))


#define IP_FORWARDING       (1<<0)
#define IP_RAWOUTPUT        (1<<1)
#define IP_ROUTETOIF        (1<<2)
#define IP_ALLOWBROADCAST   (1<<3)

#define IP_VERSION      0x4

#define IP_DF           0x4000      /* don't fragment flag */
#define IP_MF           0x2000      /* more fragments flag */

#define IP_ADDR_ANY             0x0UL
#define IP_ADDR_BROADCAST       0xFFFFFFFFUL

#define IP_TIME_TO_LIVE     30
#define IP_TYPE_OF_SERVICE  0

/* Length of IP address in bytes */
#define IP_ADDR_LEN        4

#define IP_CLASSA_ADDR(i)        (((long)(i) & 0x80000000) == 0)
#define IP_CLASSB_ADDR(i)        (((long)(i) & 0xc0000000) == 0x80000000)
#define IP_CLASSC_ADDR(i)        (((long)(i) & 0xe0000000) == 0xc0000000)
#define IP_CLASSD_ADDR(i)        (((long)(i) & 0xf0000000) == 0xe0000000)
#define IP_MULTICAST_ADDR(i)     IP_CLASSD_ADDR(i)
#define IP_EXPERIMENTAL_ADDR(i)  (((long)(i) & 0xf0000000) == 0xf0000000)

/* The Standard protocol types for IP packets. */
#define IP_UDP_PROT     17
#define IP_TCP_PROT     6
#define IP_ICMP_PROT    1
#define IP_IGMP_PROT    2

/*
 * Definitions for options.
 */
#define	IP_OPT_COPIED(o)		((o)&0x80)
#define	IP_OPT_CLASS(o)	        ((o)&0x60)
#define	IP_OPT_NUMBER(o)		((o)&0x1f)

#define	IP_OPT_CONTROL		0x00
#define	IP_OPT_RESERVED1	0x20
#define	IP_OPT_DEBMEAS		0x40
#define	IP_OPT_RESERVED2	0x60

#define	IP_OPT_EOL		    0		/* end of option list */
#define	IP_OPT_NOP		    1		/* no operation */

#define	IP_OPT_RR		    7		/* record packet route */
#define	IP_OPT_TS		    68		/* timestamp */
#define	IP_OPT_SECURITY		130		/* provide s,c,h,tcc */
#define	IP_OPT_LSRR		    131		/* loose source route */
#define	IP_OPT_SATID		136		/* satnet id */
#define	IP_OPT_SSRR		    137		/* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define	IP_OPT_OPTVAL       0       /* option ID */
#define	IP_OPT_OLEN         1       /* option length */
#define IP_OPT_OFFSET       2       /* offset within option */
#define	IP_OPT_MINOFF       4       /* min value of above */


/* External declarations. */
extern CHAR IP_Brd_Cast[IP_ADDR_LEN];
extern CHAR IP_Null[IP_ADDR_LEN];
extern const uchar   IP_A_Mask[4];
extern const uchar   IP_B_Mask[4];
extern const uchar   IP_C_Mask[4];

/* Function prototypes. */
VOID   IP_Initialize(VOID);
STATUS IP_Interpret (IPLAYER *p, DV_DEVICE_ENTRY *device, NET_BUFFER *buf_ptr);
STATUS IP_Send(NET_BUFFER *buf_ptr, RTAB_ROUTE *ro, uint32 dest_ip, uint32 src_ip,
               int32 flags, INT ttl, INT protocol, INT tos, 
               IP_MULTI_OPTIONS *mopt);
uint16 IP_Checklen (int8 *, uint16);
VOID   IP_Find_Route(RTAB_ROUTE *ro);
STATUS IP_Forward(NET_BUFFER *buf_ptr);
STATUS IP_Canforward(uint32 dest);
STATUS IP_Get_Net_Mask(CHAR *ip_addr, CHAR *mask);
NET_BUFFER *IP_Reassembly(IP_FRAG *, IP_QUEUE_ELEMENT *, NET_BUFFER *);
VOID IP_Reassembly_Event(IP_QUEUE_ELEMENT *);
VOID IP_Free_Queue_Element(IP_QUEUE_ELEMENT *);
VOID IP_Insert_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *);
VOID IP_Remove_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *);
INT  IP_Broadcast_Addr(uint32 dest, DV_DEVICE_ENTRY *int_face);
STATUS IP_Fragment (NET_BUFFER *, IPLAYER *, DV_DEVICE_ENTRY *, 
                    SCK_SOCKADDR_IP *, RTAB_ROUTE *);
INT IP_Option_Copy (IPLAYER *dip, IPLAYER *sip);

STATUS   IP_Set_Opt (int16, INT, void *, INT);
IP_MULTI *IP_Lookup_Multi(uint32 m_addr, DEV_IF_ADDRESS *if_addr);
STATUS   IP_Set_Multi_Opt(int16, INT, void *, INT);
IP_MULTI *IP_Add_Multi(uint32 m_addr, DV_DEVICE_ENTRY *dev);
STATUS   IP_Delete_Multi(IP_MULTI *ipm);
STATUS   IP_Get_Multi_Opt(int16, INT, void *, INT *);
STATUS   IP_Get_Opt (int16, INT, void *, INT *);
INT      IP_Localaddr(uint32 ip_addr);


#define RTFREE(x)

#endif
