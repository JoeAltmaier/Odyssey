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
/*      NET                                                   4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will handle defines relating to mac layer.        */
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

#ifndef NET_H
#define NET_H

/*
*	Defines which have to do with Ethernet addressing.
*	Ethernet has 6 bytes of hardware address.
*/
#define DADDLEN         6               /* Length of Ethernet header in bytes */

/* Flags for indicating the destination address of a packet in a buffer. */
#define NET_BCAST       0x01
#define NET_MCAST       0x02

/* This macro determines if an ethernet address is a multicast ethernet address.
   It works on the most significant four bytes of the ethernet address.  */
#define NET_MULTICAST_ADDR(i)        (((long)(i) & 0xffffff00) == 0x01005e00)

/* This macro maps a multicast IP address to a multicast ethernet address. */
/* uint8 *ip_addr
   uint8 ether_addr[6] 
*/
#define NET_MAP_IP_TO_ETHER_MULTI(ip_addr, ether_addr) \
{    \
    (ether_addr)[0] = 0x01; \
    (ether_addr)[1] = 0x00; \
    (ether_addr)[2] = 0x5e; \
    (ether_addr)[3] = ((uint8 *)ip_addr)[1] & 0x7f; \
    (ether_addr)[4] = ((uint8 *)ip_addr)[2]; \
    (ether_addr)[5] = ((uint8 *)ip_addr)[3]; \
}

typedef struct _NET_MULTI NET_MULTI;
struct _NET_MULTI
{
    NET_MULTI       *nm_next;
    DV_DEVICE_ENTRY *nm_device;
    uint32          nm_refcount;
    uint8           nm_addr[6];
    uint8           nm_pad[2];   /* Add padding to make life easier on platforms
                                   that require word alignment. */
};


/*  Global data structures declared in NET.C. */
extern const uint8  NET_Ether_Broadaddr[DADDLEN];

STATUS NET_Ether_Send(NET_BUFFER *buf_ptr, DV_DEVICE_ENTRY *device, 
                      SCK_SOCKADDR_IP *dest, RTAB_ROUTE *ro);

STATUS NET_Ether_Input (VOID);

#endif /* NET_H */
