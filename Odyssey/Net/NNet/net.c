/****************************************************************************/
/*                                                                          */
/*       CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.            */
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
/*    NET.C                                                    4.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file will hold all the routines which are used to interface with */
/*    the hardware.  They will handle the basic functions of of setup, xmit,*/
/*    receive, etc.  This file will change depending on the type of chip(s) */
/*    you are using to handle the TCP/IP interface.  These file are generic */
/*    and will need to be changed for your specified interface.  This file  */
/*    will use structure overlays for the chip(s) with the offset defines   */
/*    from the file chipint.h.                                              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Craig L. Meredith                                                     */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    Check in file 'chipint.h'                                             */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*     NET_Demux                                                            */
/*     NET_Ether_Send                                                       */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  other file dependencies                                                 */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                 DATE    REMARKS                                    */
/*                                                                          */
/* Craig L. Meredith   02/03/93  Inital version.                            */
/* Craig L. Meredith   02/09/94  Release new version.                       */
/* Craig L. Meredith   02/16/94  Added new macros for the inportb, outportb.*/
/* Craig L. Meredith   02/22/94  Converted the I/O parameter to a ulint.    */
/*                               Also removed the INTEL_80X86 define since  */
/*                               this file is only for the DOS port         */
/* Glen Johnson        10/26/94  Modified NET_Demux and initbuffer to handle*/
/*                               buffer pointer management.                 */
/* Glen Johnson        10/05/95  Added the function NET_Send.               */
/*                                                                          */
/****************************************************************************/
/*
*
* Portions of this program were written by:       */
/****************************************************************************
*                                                                           *
*     part of:                                                              *
*     TCP/UDP/ICMP/IP Network kernel for NCSA Telnet                        *
*     by Tim Krauskopf                                                      *
*                                                                           *
*     National Center for Supercomputing Applications                       *
*     152 Computing Applications Building                                   *
*     605 E. Springfield Ave.                                               *
*     Champaign, IL  61820                                                  *
*                                                                           *
*****************************************************************************/

#ifdef PLUS
  #include "nucleus.h"
#else  /* !PLUS */
  #include "nu_extr.h"    /* added during ATI mods - 11/1/92, bgh */
#endif /* !PLUS */
#include "target.h"
#include "protocol.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "externs.h"
#include "tcp_errs.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif
#include "net.h"
#include "arp.h"
#include "data.h"
#include "mem_defs.h"

/*  The ethernet hardware broadcast address */
const uint8  NET_Ether_Broadaddr[DADDLEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
#ifndef NULL
#define	NULL	0
#endif


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NET_Demux                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Find the packets in the buffer, determine their lowest level         */
/*  packet type and call the correct interpretation routines             */
/*                                                                       */
/*  the 'all' parameter tells NET_Demux whether it should attempt to     */
/*  empty the input packet buffer or return after the first packet       */
/*  is dealt with.                                                       */
/*                                                                       */
/*  returns the number of packets demuxed                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      netsleep                                                         */
/*      NU_EventDispatcher                                               */
/*      timer_task                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      *receive function for the device                                 */
/*                                                                       */
/*************************************************************************/
STATUS NET_Demux (VOID)
{
    INT nmuxed = 0;
    DV_DEVICE_ENTRY           *device;

    /* Process all received packets. */
    while(MEM_Buffer_List.head)
    {
        /* Point to the device on which this packet was received. */
        device = MEM_Buffer_List.head->mem_buf_device;

        /* Call the receive function for that device. */
        (*(device->dev_input))();

        /* Increment the number of processed packets. */
        nmuxed++;
    }

    return (nmuxed);

} /* NET_Demux */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NET_Ether_Input                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Examines input ethernet packet and determines what type of       */
/*      packet it is.  It then calls the proper interpret routine.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Net_Demux                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ARP_Interpret                                                    */
/*      MEM_Buffer_Chain_Free                                            */
/*      IP_Interpret                                                     */
/*                                                                       */
/*************************************************************************/

STATUS NET_Ether_Input (VOID)
{
    uint16              getcode;
    DLAYER              *ether_pkt;
    DV_DEVICE_ENTRY     *device;

    /* Point to the packet. */
    ether_pkt = (DLAYER *)(MEM_Buffer_List.head->data_ptr);

    /* Get a pointer to the device. */
    device = MEM_Buffer_List.head->mem_buf_device;

    /* If this packet was addressed to either a broadcast or multicast address
       set the appropriate flag. */
    if (memcmp(NET_Ether_Broadaddr, ether_pkt->dest, DADDLEN) == 0)
    {
        MEM_Buffer_List.head->mem_flags |= NET_BCAST;
        SNMP_ifInNUcastPkts_Inc(device->dev_index);
#ifdef XMIB_RMON1
        RMON_BroadcastPkts_Inc(device->dev_index);
#endif
    }
    else if (ether_pkt->dest[0] & 1)
    {
        MEM_Buffer_List.head->mem_flags |= NET_MCAST;
        SNMP_ifInNUcastPkts_Inc(device->dev_index);
#ifdef XMIB_RMON1
        RMON_MulticastPkts_Inc(device->dev_index);
#endif

    }
    else
        SNMP_ifInUcastPkts_Inc(device->dev_index);

    /* What type of packet is this? */
    getcode = ether_pkt->type;




    switch (intswap(getcode))
    {             /* what to do with it? */
        case EARP:
        case ERARP:
    	    /* Strip the ethernet header and size off of the packet */
    	    MEM_Buffer_List.head->data_ptr           += device->dev_hdrlen;
            MEM_Buffer_List.head->data_len           -= device->dev_hdrlen;
            /* This is an ARP packet */


            ARP_Interpret ((ARP_LAYER *)MEM_Buffer_List.head->data_ptr, device);

            /* We are finished with the ARP packet. */
            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            break;

        case EIP:

            MEM_Buffer_List.head->data_len           -= device->dev_hdrlen;
	    move_ip_data((unsigned char *)MEM_Buffer_List.head->data_ptr,
				    device->dev_hdrlen,
				    MEM_Buffer_List.head->data_len); 
            /* This is an IP packet. */
            IP_Interpret ((IPLAYER *)MEM_Buffer_List.head->data_ptr, device, MEM_Buffer_List.head);
            break;

        default:
            /* Deallocate the space taken up by this useless packet. */
            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            SNMP_ifInUnknownProtos_Inc(device->dev_index);

            break;

    } /* end switch */

    return (NU_SUCCESS);

} /* NET_Ether_Input */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NET_Ether_Send                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is the hardware layer transmission function for    */
/*      Ethernet.  Other physical mediums (serial, token ring, etc.)     */
/*      will have their own transmit functions.  Given a packet to       */
/*      transmit this function first resolves the hardware layer address */
/*      and then queues the packet for transmission.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpsend                                                          */
/*      tcp_sendack                                                      */
/*      tcpresetfin                                                      */
/*      arp_reply                                                        */
/*      arp_request                                                      */
/*      UDP_Send                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*      RTAB_Find_Route                                                  */
/*      ARP_Resolve                                                      */
/*      intswap                                                          */
/*      device->device_start                                             */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      MEM_Buffer_Enqueue                                               */
/*      NU_Control_Interrupts                                            */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
STATUS NET_Ether_Send(NET_BUFFER *buf_ptr, DV_DEVICE_ENTRY *device,
                      SCK_SOCKADDR_IP *dest, RTAB_ROUTE *ro)
{
#ifndef PACKET 
    INT             old_int;
#endif
    ROUTE_NODE      *rt = NU_NULL;
    uchar           mac_dest[DADDLEN];
    DLAYER          *eh;                /* Ethernet header pointer */
    int16           type;
    STATUS          stat;

    /* Verify that the device is up. */
    if ( (device->dev_flags & (DV_UP | DV_RUNNING)) != (DV_UP | DV_RUNNING) )
        return (NU_HOST_UNREACHABLE);

    if (ro)
        rt = ro->rt_route;

    /* Verify the route is up. Both gwroute and rt are set equal to the route
       located by IP_Send. */
    if(rt)
    {
        /* If the route is not up then try to locate an alternate route. */
        if ((rt->rt_flags & RT_UP) == 0)
        {
            if ( (rt = RTAB_Find_Route(dest)) != NU_NULL)
                rt->rt_refcnt--;
            else
                return (NU_HOST_UNREACHABLE);
        }
    }

    switch(dest->sck_family)
    {
        case SK_FAM_IP:

            /* Resolve the MAC address. */
            if ( (stat = ARP_Resolve(device, dest, mac_dest, buf_ptr)) != NU_SUCCESS )
                return stat;

            type = EIP;

            break;


        case SK_FAM_UNSPEC:
            /* Family is unspecified.  This should be an ARP packet. */

            /* Point to the ethernet header information, and extract the type
               and destination address.
            */
            eh = (DLAYER *)&dest->sck_port;
            memcpy (mac_dest, eh->dest, DADDLEN);
            type = eh->type;

            break;

        default:
            return -1;

    }

    /* Move back the data pointer to make room for the MAC layer and adjust the 
       packet length. */
    buf_ptr->data_ptr           -= sizeof (DLAYER);
    buf_ptr->data_len           += sizeof (DLAYER);
    buf_ptr->mem_total_data_len += sizeof (DLAYER);

    /* Point to the packet. */
    eh = (DLAYER *)buf_ptr->data_ptr;

    /* Initialize the ethernet header. */
    eh->type = intswap(type);
    memcpy (eh->dest, mac_dest, DADDLEN);
    memcpy (eh->me, device->dev_mac_addr, DADDLEN);

#ifdef PACKET

    /* Pass the packet to the device. */
    device->dev_start(device, buf_ptr);

    /* The packet has been transmitted.  Deallocate the buffer. */
    MEM_One_Buffer_Chain_Free (buf_ptr, buf_ptr->mem_dlist);
#else

    old_int = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Place the buffer on the device's transmit queue. */
    MEM_Buffer_Enqueue(&device->dev_transq, buf_ptr);

    /* If this is the first buffer in the transmit queue we need to resume the
     * transmit task.  If not another packet is already being transmitted. */
    if(device->dev_transq.head == buf_ptr)    
    {
        NU_Control_Interrupts(old_int);

        /* Resume the the transmit task. */
        device->dev_start(device, buf_ptr);
    }
    else
        NU_Control_Interrupts(old_int);

#endif

    return(NU_SUCCESS);

}  /* NET_Ether_Send */

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*    NET_Add_Multi                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    When an ethernet multicast group is joined this function adds a entry */
/*    to the list of ethernet multicast addresses a device should receive.  */
/*    If the specified address is already registered with the device the    */
/*    reference count is incremented.                                       */
/*                                                                          */
/* CALLED BY                                                                */
/*      Ethernet drivers                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_Control_Interrupts                                               */
/*      NU_Allocate_memory                                                  */
/*      NET_MAP_IP_TO_ETHER_MULTI                                           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Glen Johnson       03/06/98      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
STATUS NET_Add_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req)
{
    uint8       multi_addr[6];
    INT         irq_level;
    NET_MULTI   *em;

    irq_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Convert the IP address to a multicast ethernet address. */
    NET_MAP_IP_TO_ETHER_MULTI(&d_req->dvr_addr, multi_addr);

    /* Verify that the ethernet multicast address is valid. */
    if (((multi_addr[0] & 0xFF) != 1) || ((multi_addr[2] & 0xFF) != 0x5e))
    {
        NU_Control_Interrupts(irq_level);
        return (-1);
    }

    /* Has this address already been added to the list. */
    for ( em = dev->dev_ethermulti;
          em != NULL && (memcmp(em->nm_addr, multi_addr, 6) != 0);
          em = em->nm_next);

    if(em != NU_NULL)
    {
        /* Found a match. Increment the reference count. */
        em->nm_refcount++;
        NU_Control_Interrupts(irq_level);
        return (NU_SUCCESS);
    }

    /* This is a new address. Allocate some memory for it. */
    if (NU_Allocate_Memory(&System_Memory, (VOID **)&em,
                            sizeof (*em), (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
    {
        NU_Control_Interrupts(irq_level);
        return(NU_MEM_ALLOC);
    }

    /* Initialize the new entry. */
    memcpy(em->nm_addr, multi_addr, 6);
    em->nm_device = dev;
    em->nm_refcount = 1;

    /* Link it into the list. */
    em->nm_next = dev->dev_ethermulti;
    dev->dev_ethermulti = em;

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(irq_level);

    return(NU_RESET);

} /* NET_Add_Multi */

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*    NET_Del_Multi                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    When an ethernet multicast group is dropped this function will delete */
/*    the entry from the list of ethernet multicast addresses this device   */
/*    should receive if the refernce count drops to 0.  Else the reference  */
/*    count is decremented.                                                 */
/*                                                                          */
/* CALLED BY                                                                */
/*      Ethernet drivers                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_Control_Interrupts                                               */
/*      NU_Deallocate_memory                                                */
/*      NET_MAP_IP_TO_ETHER_MULTI                                           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME             DATE              REMARKS                          */
/*                                                                          */
/*  Glen Johnson       03/06/98      Created Initial Version                */
/*                                                                          */
/****************************************************************************/
STATUS NET_Del_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req)
{
    uint8       multi_addr[6];
    INT         irq_level;
    NET_MULTI   *em;
    NET_MULTI   **ptr;

    irq_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Convert the IP address to a multicast ethernet address. */
    NET_MAP_IP_TO_ETHER_MULTI(&d_req->dvr_addr, multi_addr);

    /* Verify that the ethernet multicast address is valid. */
    if (((multi_addr[0] & 0xFF) != 1) || ((multi_addr[2] & 0xFF) != 0x5e))
    {
        NU_Control_Interrupts(irq_level);
        return (-1);
    }

    /* Find this address in the list. */
    for ( em = dev->dev_ethermulti;
          em != NULL && (memcmp(em->nm_addr, multi_addr, 6) != 0);
          em = em->nm_next);

    if(em == NU_NULL)
    {
        /* Found a match. Increment the reference count. */
        NU_Control_Interrupts(irq_level);
        return (NU_INVAL);
    }

    /* If this is not the last refernce then return after decremanting the 
       reference count. */
    if (--em->nm_refcount != 0)
    {
        NU_Control_Interrupts(irq_level);
        return(NU_SUCCESS);
    }

    /* If we made this far then there are no more references to this entry.
       So unlink and deallocte it. */
    for ( ptr = &em->nm_device->dev_ethermulti;
          *ptr != em;
          ptr = &(*ptr)->nm_next)
        continue;

    *ptr = (*ptr)->nm_next;

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(irq_level);

    /* Deallocate the structure. */
    NU_Deallocate_Memory(em);

    return(NU_RESET);

} /* NET_Add_Multi */

move_ip_data(unsigned char *data, unsigned long hdrlen, unsigned long datalen)
{
	unsigned char	*src;
	unsigned char	*dst;
	int		i;
	
	
	src = (unsigned char *)((unsigned long)data + hdrlen);
	dst = data; 
	
	for(i = 0; i < datalen; i++)
		dst[i] = src[i];
}
