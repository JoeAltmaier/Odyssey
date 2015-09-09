/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993 by Accelerated Technology, Inc.                  */
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
/*      DNS.H                                                 4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will handle domain processing defines.            */
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
/*                                                                          */
/****************************************************************************/

#ifndef DNS_H
#define DNS_H

#include "target.h"

/* Default DNS Server IP address. */
#define DNS_IP_Addr_1       206
#define DNS_IP_Addr_2       202
#define DNS_IP_Addr_3       34
#define DNS_IP_Addr_4       2

/* Max and Min size defintions. */
#define     DNS_MAX_LABEL_SIZE      63
#define     DNS_MAX_NAME_SIZE       255
#define     DNS_MAX_MESSAGE_SIZE    512
#define     DNS_MAX_ATTEMPTS        5
#define     DNS_MIN_NAME_ALLOC      51  /* 50 characters + NULL terminator. */

/* This is the port DNS servers listen for queries on. */
#define     DNS_PORT                53

/* Resource Record (RR) type codes: */
#define DNS_TYPE_A      1           /* A host address (RR)    */
#define DNS_TYPE_PTR    12          /* A domain name ptr (RR) */

/* RR Class definitions.  The only one we care about is the Internet class. */
#define DNS_CLASS_IN    1           /* Internet class */

/*
 *  flag masks for the flags field of the DNS header
 */
#define DNS_QR         0x8000          /* query=0, response=1 */
#define DNS_OPCODE     0x7100          /* opcode, see below */
#define DNS_AA         0x0400          /* Authoritative answer */
#define DNS_TC         0x0200          /* Truncation, response was cut off at 512 */
#define DNS_RD         0x0100          /* Recursion desired */
#define DNS_RA         0x0080          /* Recursion available */
#define DNS_RCODE_MASK 0x000F

/* opcode possible values: */
#define DNS_OPQUERY    0    /* a standard query */
#define DNS_OPIQ       1    /* an inverse query */
#define DNS_OPCQM      2    /* a completion query, multiple reply */
#define DNS_OPCQU      3    /* a completion query, single reply */

/* the rest reserved for future */
#define DNS_ROK        0    /* okay response */
#define DNS_RFORM      1    /* format error */
#define DNS_RFAIL      2    /* their problem, server failed */
#define DNS_RNAME      3    /* name error, we know name doesn't exist */
#define DNS_RNOPE      4    /* no can do request */
#define DNS_RNOWAY     5    /* name server refusing to do request */
#define DNS_WILD       255  /* wildcard for several of the classifications */

/* All DNS messages have a header defined as follows. */
typedef struct _DNS_PKT_HEADER
{
    uint16      dns_id;
    uint16      dns_flags;
    uint16      dns_qdcount;
    uint16      dns_ancount;
    uint16      dns_nscount;
    uint16      dns_arcount;

} DNS_PKT_HEADER;

/*
 *  A resource record is made up of a compressed domain name followed by
 *  this structure.  All of these ints need to be byteswapped before use.
 */
typedef struct _DNS_RR
{
    uint16      dns_type;           /* resource record type=DTYPEA */
    uint16      dns_class;          /* RR class=DIN */
    uint32      dns_ttl;            /* time-to-live, changed to 32 bits */
    uint16      dns_rdlength;       /* length of next field */
    char        dns_rdata[1];       /* data field */
} DNS_RR;

/* This structure is defines what a host looks like. */
typedef struct _DNS_HOST
{
    struct _DNS_HOST    *dns_next;
    struct _DNS_HOST    *dns_previous;
    UNSIGNED            dns_ttl;            /* Time To Live for this entry.  A
                                               value of 0 is used to indicate a
                                               permanent entry. */
    INT                 dns_name_size;      /* The size of the name in this
                                               entry. */
    CHAR                *dns_name;          /* Host name. */
    CHAR                dns_ipaddr[4];      /* Host IP address. */
} DNS_HOST;

/* Define the head of the linked list of HOSTs. */
typedef struct _DNS_HOST_LIST
{
    DNS_HOST    *dns_head;
    DNS_HOST    *dns_tail;
} DNS_HOST_LIST;


/* Function prototypes. */
STATUS      DNS_Resolve(CHAR *name, CHAR *ip_addr, UNSIGNED *ttl, INT type);
STATUS      DNS_Initialize(VOID);
INT         DNS_Build_Query(CHAR *data, VOID **buffer, INT type);
INT         DNS_Query(CHAR *buffer, int16 q_size);
INT         DNS_Pack_Domain_Name (CHAR *dst, CHAR *src);
INT         DNS_Unpack_Domain_Name(CHAR *dst, CHAR *src, CHAR *buf_begin);
STATUS      DNS_Extract_Data (DNS_PKT_HEADER *pkt, CHAR *ip_addr, UNSIGNED *ttl,
                              INT type);
DNS_HOST    *DNS_Add_Host(CHAR *name, CHAR *ip_addr, UNSIGNED ttl);
DNS_HOST    *DNS_Find_Host_By_Name(CHAR *name);
STATUS      DNS_Addr_To_String(CHAR *addr, CHAR *new_name);
DNS_HOST    *DNS_Find_Host_By_Addr(CHAR *addr);
STATUS      DNS_Extract_Name (DNS_PKT_HEADER *pkt, CHAR *new_name,
                              UNSIGNED *ttl);

#endif  /* DNS_H */
