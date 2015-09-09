/*************************************************************************/
/*                                                                       */
/*    CopyrIght (c)  1993 - 1997 Accelerated Technology, Inc.            */
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
* Portions of this program were written by:       */
/****************************************************************************
*                                                                           *
*     part of:                                                              *
*     TCP/IP kernel for NCSA Telnet                                         *
*     by Tim Krauskopf                                                      *
*                                                                           *
*     National Center for Supercomputing Applications                       *
*     152 Computing Applications Building                                   *
*     605 E. Springfield Ave.                                               *
*     Champaign, IL  61820                                                  *
*                                                                           *
*****************************************************************************/

/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*      ARP.C                                           NET 4.0               */
/*                                                                            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*      This file contains the implementation of ARP (Address Resolution      */
/*      Protocol).                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*      ARP_Cache               An array that contains the ARP cache.         */
/*      ARP_Res_List            This is a list of packets that are awaitng    */
/*                              the resolution of an address before thay can  */
/*                              be transmitted.                               */
/*      ARP_Res_Count           Global counter used to assign a unique ID     */
/*                              to each entry in ARP_Res_List.                */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*     ARP_Build_Pkt            Construct an ARP packet.                      */
/*     ARP_Cache_Update                                                       */
/*     ARP_Event                                                              */
/*     ARP_Find_Entry           Search the ARP cache.                         */
/*     ARP_Init                 Initialize the ARP Module.                    */
/*     ARP_Interpret            Process a received ARP Packet.                */
/*     ARP_Reply                Send an ARP reply.                            */
/*     ARP_Request                                                            */
/*     ARP_Resolve              Resolve an Ethernet address.                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*     protocol.h        External structure definitions for each type         */
/*                       of prtocol this program handles.                     */
/*     externs.h         External definitions for functions in NCSA           */
/*                       Telnet.                                              */
/*     nucleus.h         contains system constants common to both the         */
/*                       application and Nucleus PLUS components.             */
/*     tcpdefs.h         definitions for Nucleus - UDP program                */
/*     data.h            Declarations of global variables for TCP/IP          */
/*                       libraries                                            */
/*     target.h          Defines applicable to IBM PC/ PC-DOS environmentttttt*/
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/* NAME            DATE                    REMARKS                            */
/*                                                                            */
/* G. Johnson      06/06/96           Added a valid_entry field to the ARP    */
/*                                    cache.                                  */
/* Maiqi Qian      12/06/96           Fixed the time wrap around (spr0229)    */
/*                                                                            */
/******************************************************************************/

/*
* Includes
*/
#ifdef PLUS
  #include "nucleus.h"
#else
  #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
  #include "nu_extr.h"
#endif
#include "target.h"
#include "protocol.h"
#include "net_extr.h"
#include "socketd.h"
#include "tcp_errs.h"
#include "dev.h"
#include "arp.h"
#include "ip.h"
#include "net.h"
#include "netevent.h"
#include "data.h"
#include "externs.h"
#if SNMP_INCLUDED

#include "snmp_g.h"

#endif
/* This is our ARP cache. */
ARP_ENTRY ARP_Cache[ARP_CACHE_LENGTH];

/* This is the resolve list.  An item is placed on this list when a MAC address
   needs to be resolved.  It is removed once the address has been resolved or
   when failure occurs.
 */
ARP_RESOLVE_LIST  ARP_Res_List;

uint16            ARP_Res_Count;

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Build_Pkt                                                            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function will get a free buffer and fill in as much of the         */
/*    ARP packet fields as possible.  All other fields must be updated by     */
/*    the calling function.                                                   */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    ARP_Request               Send an ARP request.                          */
/*    ARP_Reply                 Send an ARP reply.                            */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    MEM_Buffer_Dequeue        Remove the first item from a list.            */
/*    intswap                   Swap the bytes in an integer.                 */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    dev                       The device to transmit the packet on.         */
/*    tipnum                    The target IP number.                         */
/*    thardware                 The target hardware address.                  */
/*    pkt_type                  The ARP packet type, reply or request.        */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NET_BUFFER                Pointer to the buffer in which the packet     */
/*                              was constructed.                              */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      02/10/95    Initial version.                          */
/*    Glen Johnson      05/21/97    Re-designed for NET 4.0                   */
/*                                                                            */
/******************************************************************************/
NET_BUFFER *ARP_Build_Pkt (DV_DEVICE_ENTRY *dev, uint8 *tipnum, const uint8 *thardware,
                       int16 pkt_type)
{
    NET_BUFFER  *buf_ptr;
    ARP_LAYER   *arp_pkt;


    /* Allocate a buffer to place the arp packet in. */
    if ((buf_ptr = MEM_Buffer_Dequeue(&MEM_Buffer_Freelist)) == NU_NULL)
    {
        return (NU_NULL);
    }

    /* Initialize each field in the allocated buffer. */
    buf_ptr->mem_total_data_len = buf_ptr->data_len = sizeof(ARP_LAYER);
    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + (NET_MAX_ARP_HEADER_SIZE 
        - sizeof (ARP_LAYER));
    buf_ptr->mem_seqnum     = 0;
    buf_ptr->mem_dlist      = (NET_BUFFER_HEADER *) &MEM_Buffer_Freelist;
    buf_ptr->next           = NU_NULL;
    buf_ptr->next_buffer    = NU_NULL;

    /* Set up a pointer to the packet. */
    arp_pkt = (ARP_LAYER *)buf_ptr->data_ptr;

    arp_pkt->arp_hrd = intswap (HARDWARE_TYPE);  /*  Ether = 1 */
    arp_pkt->arp_pro = intswap (ARPPRO);         /* IP protocol = 0x0800 */
    arp_pkt->arp_hln = DADDLEN;                  /* Ethernet hardware length */
    arp_pkt->arp_pln = 4;                        /* IP length = 4 */

    /* sender's hardware addr */
    memcpy(arp_pkt->arp_sha, dev->dev_mac_addr, DADDLEN);

    memcpy(arp_pkt->arp_tha, thardware, DADDLEN);

    /* sender's IP addr */
    memcpy(arp_pkt->arp_spa, dev->dev_addr.dev_ip_addr, IP_ADDR_LEN);

    /* put in IP address we want */
    memcpy(arp_pkt->arp_tpa, tipnum, IP_ADDR_LEN);

    /* Set the packet type.  Either a request or a response. */
    arp_pkt->arp_op = intswap(pkt_type);

    return(buf_ptr);

} /* end ARP_Build_Pkt */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Reply                                                                */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function sends an ARP reply.  Called whenever an ARP request is    */
/*    received.                                                               */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    ARP_Interpret             Processes received ARP packets.               */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    ARP_Build_Pkt             Construct an ARP packet.                      */
/*    RTAB_Find_Route            Get a route to the target IP.                */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    tipnum                    The target IP number.                         */
/*    thardware                 The target hardware address.                  */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_NO_BUFFERS             Indicates failure to allocate a buffer.       */
/*    NU_HOST_UNREACHABLE       Indicates that a route could not be found.    */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Re-designed for NET 4.0                   */
/*                                                                            */
/******************************************************************************/
STATUS ARP_Reply(uint8 *thardware, uint8 *tipnum)
{
    NET_BUFFER              *buf_ptr;
    ROUTE_NODE              *rt;
    SCK_SOCKADDR_IP         dest;
    DV_DEVICE_ENTRY         *dev;
    ARP_MAC_HEADER          mh;

    /* Find the route. */
    dest.sck_family = SK_FAM_IP;
    dest.sck_len = sizeof (dest);
    memcpy(&dest.sck_addr, tipnum, IP_ADDR_LEN); 

    if ( (rt = RTAB_Find_Route(&dest)) == NU_NULL)
        return (NU_HOST_UNREACHABLE);

    /* Point to the interface associated with this route. */
    dev = rt->rt_device;

    if ( (buf_ptr = ARP_Build_Pkt(dev, tipnum, (const uchar *)thardware,
                                  ARPREP)) == NU_NULL )
    {
        return (NU_NO_BUFFERS);
    }

    UTL_Zero(&mh, sizeof(mh));

    /* This is a psuedo MAC hedaer that is passed to the MAC layer send function. 
       The MAC layer information that is know is filled in. A family of 
       SK_FAM_UNSPEC lets the MAC layer know that this is not an IP datagram and 
       it should not try to resolve a hardware address. */
    memcpy (mh.ar_mac.ar_mac_ether.dest, thardware, DADDLEN);
    mh.ar_mac.ar_mac_ether.type = EARP;
    mh.ar_family = SK_FAM_UNSPEC;
    mh.ar_len = sizeof(mh);    

    /* Send the ARP Packet. */
    (*dev->dev_output)( buf_ptr, dev, (SCK_SOCKADDR_IP *)&mh, NU_NULL);

    return (NU_SUCCESS);

}  /* end ARP_Reply */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Interpret                                                            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   Interpret ARP packets.                                                   */
/*   Look at incoming ARP packet and make required assessment of              */
/*   usefulness, check to see if we requested this packet, clear              */
/*   all appropriate flags.                                                   */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    NET_Ether_Input           Examines received ethernet packets.           */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    ARP_Cache_Update          Add an entry to the ARP cache.                */
/*    ARP_Reply                 Send an ARP reply.                            */
/*    comparen                  Compare two strings.                          */
/*    dll_remove                Remove an item from a list.                   */
/*    NU_Deallocate_Memory      Deallocate a block of memory.                 */
/*    intswap                   Swap the bytes in an integer.                 */
/*    NU_Tcp_Log_Error          Log an error.                                 */
/*    UTL_Timerunset               Clear a timer event.                       */
/*    NU_Resume_Task            Resume a task.                                */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    a_pkt                     Pointer to the ARP packet to process.         */
/*    device                    Pointer to the device the packet was receied  */
/*                                on.                                         */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_NO_ACTION              Indicates that the packet was determined      */
/*                                to be unacceptable.                         */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Re-designed for NET 4.0                   */
/*                                                                            */
/******************************************************************************/
STATUS ARP_Interpret(ARP_LAYER *a_pkt, DV_DEVICE_ENTRY *device)
{
    ARP_RESOLVE_ENTRY   *ar_entry;
    uint8               my_ip[IP_ADDR_LEN];
    ARP_MAC_HEADER      mh;


    memcpy(my_ip, device->dev_addr.dev_ip_addr, IP_ADDR_LEN);

    /* Check to see if the packet was for me or if it was sent by someone using
       my IP address.  If neither then return.  Most ARP packets should fall
       into this category.
     */
    if ( !(comparen (a_pkt->arp_tpa, my_ip, 4)) &&
         !(comparen (a_pkt->arp_spa, my_ip, 4)) )
    {
#if RARP_INCLUDED

        /* Is this a RARP response to my RARP request. */
        if ( comparen(my_ip, IP_Null, 4) && (a_pkt->arp_op == intswap (RARPR))
             && comparen (a_pkt->arp_tha, device->dev_mac_addr, DADDLEN) )
        {
            /* Search the ARP_Res_List for a match. */
            for(ar_entry = ARP_Res_List.ar_head;
                ar_entry != NU_NULL;
                ar_entry = ar_entry->ar_next)
            {
                /* A match is found when we find an entry for a RARP request. */
                if ( (ar_entry->ar_pkt_type == RARPQ) &&
                     (ar_entry->ar_device == device) )
                    break;
            }

            /* Was a match found. */
            if (ar_entry)
            {
                /* Copy the newly discovered IP address. */
                memcpy(device->dev_addr.dev_ip_addr, a_pkt->arp_tpa, IP_ADDR_LEN);

                /* Clear the timer event. */
                UTL_Timerunset(RARP_REQUEST, ar_entry->ar_id, 1);

                /* Resume the task. */
                NU_Resume_Task(ar_entry->ar_task);
            }
        }
#endif /* RARP_INCLUDED */

        /* This packet was destined for someone else, so return. */
        return (NU_SUCCESS);
    }

    /* If the source IP address is the same as mine then someone is trying to
       use my IP address.
     */
    if ( comparen (a_pkt->arp_spa, my_ip, 4) )
    {
        NU_Tcp_Log_Error (TCP_SAME_IP, TCP_RECOVERABLE, __FILE__, __LINE__);
    }

    /*
    *  check packet's desired IP address translation to see if it wants
    *  me to answer.
    */
    if ( (a_pkt->arp_op == intswap (ARPREQ)) &&
         (comparen (a_pkt->arp_tpa, my_ip, 4)))
    {
        ARP_Cache_Update (a_pkt->arp_spa, a_pkt->arp_sha);   /* keep her address for me */
        ARP_Reply(a_pkt->arp_sha, a_pkt->arp_spa);      /* proper reply */
        return (NU_SUCCESS);
    }

    /*
     *  Check for a reply that I probably asked for.
     */
    if (comparen (a_pkt->arp_tpa, my_ip, IP_ADDR_LEN)
        && (a_pkt->arp_op == intswap (ARPREP))
        && (a_pkt->arp_hrd == intswap (HARDWARE_TYPE))
        && (a_pkt->arp_hln == DADDLEN) && (a_pkt->arp_pln == 4))
    {
        ARP_Cache_Update (a_pkt->arp_spa, a_pkt->arp_sha);

        /* Are there any IP packets pending the resolution of a MAC address?  If
           so check to see if this ARP response resolves any of those.
         */
        for (ar_entry = ARP_Res_List.ar_head;
             ar_entry != NU_NULL;
             ar_entry = ar_entry->ar_next)
        {
            if (comparen(&ar_entry->ar_dest, a_pkt->arp_spa, 4))
                break;
        }

        /* Was a match found? */
        if (ar_entry)
        {
            /* Resume the waiting task */
            NU_Resume_Task(ar_entry->ar_task);

            /* Clear the timer event. */
            UTL_Timerunset(ARPRESOLVE, ar_entry->ar_id, 1);

            /* Send the pending IP packet.  The IP packet has already been
               formed.  The route has previously been determined.  All we have
               to do here is build the Ethernet Header and send it.
             */
            UTL_Zero(&mh, sizeof(mh));

            /* This is a psuedo MAC hedaer that is passed to the MAC layer send function. 
               The MAC layer information that is know is filled in. A family of 
               SK_FAM_UNSPEC lets the MAC layer know that it should not try to 
               resolve a hardware address. */
            memcpy (mh.ar_mac.ar_mac_ether.dest, a_pkt->arp_sha, DADDLEN);
            mh.ar_mac.ar_mac_ether.type = EIP;
            mh.ar_family = SK_FAM_UNSPEC;
            mh.ar_len = sizeof(mh);    

            /* Send the packet. */
            (*ar_entry->ar_device->dev_output)(ar_entry->ar_buf_ptr,
                                               ar_entry->ar_device,
                                               (SCK_SOCKADDR_IP *)&mh, NU_NULL);

            dll_remove((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);

            NU_Deallocate_Memory(ar_entry);
        }

        return (NU_SUCCESS);
    }

    return (NU_NO_ACTION);
} /* ARP_Interpret */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Init                                                                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   Initialize the ARP module.                                               */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*      Glen Johnson,       Accelerated Technology.                           */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    protinit                  Protocol initialization.                      */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    UTL_Zero                  Zero out a memory area.                       */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    none                                                                    */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    none                                                                    */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Created initial version for NET 4.0       */
/*                                                                            */
/******************************************************************************/
VOID ARP_Init (VOID)
{
    /* Clear the ARP Cache */
    UTL_Zero(ARP_Cache, sizeof(ARP_ENTRY) * ARP_CACHE_LENGTH);

    /* The resolve list is initially empty. */
    ARP_Res_List.ar_head = NU_NULL;
    ARP_Res_List.ar_tail = NU_NULL;

    ARP_Res_Count = 0;

}  /* ARP_Init */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Resolve                                                              */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   This function attempts to resolve an ethernet hardware address.  If      */
/*   unable to find an entry in the ARP cache it will queue the packet, an IP */
/*   packet, that needs to be sent.  An ARP event will be created to send an  */
/*   ARP request.  The IP packet is transmitted once the address is resolved. */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*      Glen Johnson,       Accelerated Technology.                           */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    NET_Ether_Send            Send an ethernet packet.                      */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    ARP_Find_Entry            Lookup an entry in the ARP cache.             */
/*    dll_enqueue               Add an item to a list.                        */
/*    NU_Allocate_Memory        Allocate memory.                              */
/*    TL_Put_Event               Send an event to the Events Dispatcher.      */
/*    NU_Current_Task_Pointer   Get the current task pointer.                 */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    int_face                  Pointer to the device to on.                  */
/*    dest                      Pointer to the IP destination.                */
/*    mac_dest                  Pointer to the hardware address.  This will   */
/*                                will be filled in if found.                 */
/*    buf_ptr                   Pointer to a buffer containing an IP packet.  */
/*    a_entry                   Pointer to ARP cache entry.  This will be     */
/*                                filled in.                                  */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_UNRESOLVED_ADDR        Indicates the address is unresolved.  The IP  */
/*                                is queued pending resolution.               */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Created initial version for NET 4.0       */
/*                                                                            */
/******************************************************************************/
STATUS ARP_Resolve(DV_DEVICE_ENTRY *int_face, SCK_SOCKADDR_IP *ip_dest, 
                   uchar *mac_dest, NET_BUFFER *buf_ptr)
{
    ARP_RESOLVE_ENTRY   *ar_entry;
    STATUS              stat;
    ARP_ENTRY           *a_entry;

    /* If this is a broadcast packet then simply return the ethernet broadcast 
       address. */
    if (buf_ptr->mem_flags & NET_BCAST)
    {
        memcpy(mac_dest, NET_Ether_Broadaddr, DADDLEN);
        return (NU_SUCCESS);
    }

    if (buf_ptr->mem_flags & NET_MCAST)
    {
        NET_MAP_IP_TO_ETHER_MULTI(&ip_dest->sck_addr, mac_dest);
        return (NU_SUCCESS);
    }

    /* Initialize the ARP entry pointer to NULL.  It will be updated if a
       match is found in the ARP cache.
     */
    a_entry = NU_NULL;

    /* Check the ARP cache for the destination.  If found, return. */
    if ( (a_entry = ARP_Find_Entry(ip_dest)) != NU_NULL)
    {
        memcpy(mac_dest, a_entry->arp_mac_addr, DADDLEN);
        return (NU_SUCCESS);
    }

    if ((stat = NU_Allocate_Memory(&System_Memory, (VOID **)&ar_entry,
                        sizeof(*ar_entry),
                        (UNSIGNED)NU_NO_SUSPEND)) != NU_SUCCESS)
    {
        return (stat);
    }

    ar_entry->ar_id         = ARP_Res_Count++;
    ar_entry->ar_device     = int_face;
    ar_entry->ar_dest       = ip_dest->sck_addr;
    ar_entry->ar_send_count = 0;
    ar_entry->ar_task       = NU_Current_Task_Pointer();
    ar_entry->ar_buf_ptr    = buf_ptr;
    ar_entry->ar_pkt_type   = ARPREQ;

    /* Order is not important.  Simply add the new entry to the front of the
       list. */
    dll_enqueue((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);

    TL_Put_Event (ARPRESOLVE, ar_entry->ar_id);

    return(NU_UNRESOLVED_ADDR);
}  /* end ARP_Resolve */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Find_Entry                                                           */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   This function searches the ARP cache for a matching entry.               */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*      Glen Johnson,       Accelerated Technology.                           */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    ARP_Resolve               Resolve an ethernet address.                  */
/*    doconnect                 Establish a TCP connection.                   */
/*    UDP_Send                  Send a UDP packet.                            */
/*    tcp_xmit                  Send a TCP packet.                            */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    n_clicks                  Get the clock tick.                           */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    dest                      IP address for which an ethernet address is   */
/*                              desired.                                      */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    ARP_ENTRY                 A pointer to an entry in the ARP cache.  NULL */
/*                                on failure.                                 */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Created initial version for NET 4.0       */
/*                                                                            */
/******************************************************************************/
ARP_ENTRY *ARP_Find_Entry(SCK_SOCKADDR_IP *dest)
{
    INT         i;
    ARP_ENTRY   *a_entry = NU_NULL;

    /* Search the cache for the target IP number. */
    for (i = 0; i < ARP_CACHE_LENGTH; i++)
    {
        if ( dest->sck_addr == ARP_Cache[i].arp_ip_addr)
        {
            /* We found the entry. */
            a_entry = &ARP_Cache[i];
            break;
        }
    }

    /* If an entry was found, it has not timed out, and the entry is valid, the
       return a pointer to this entry.  Else return NULL
     */
    if ( a_entry && (INT32_CMP((a_entry->arp_time + CACHETO), n_clicks()) > 0)
         && (a_entry->arp_flags & ARP_UP) )
        return (a_entry);
    else
        return NU_NULL;

} /* ARP_Find_Entry */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   ARP_Request                                                              */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   Send an ARP request.                                                     */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*      Glen Johnson,       Accelerated Technology.                           */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    ARP_Event                                                               */
/*    DEV_Attach_IP_To_Device                                                 */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    ARP_Build_Packet                                                        */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    dest                      IP address for which an ethernet address is   */
/*                              desired.                                      */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    ARP_ENTRY                 A pointer to an entry in the ARP cache.  NULL */
/*                                on failure.                                 */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      05/21/97    Created initial version for NET 4.0       */
/*                                                                            */
/******************************************************************************/
STATUS ARP_Request(DV_DEVICE_ENTRY *device, uint32 *tip, uint8 *thardware,
                   INT protocol_type, int16 arp_type)
{
    NET_BUFFER      *buf_ptr;
    ARP_MAC_HEADER  mh;


    /* Build the ARP request packet.  The target hardware address is unknown at
       this point, so pass in a string of NULL characters as the target hardware
       address.
     */
    if ((buf_ptr = ARP_Build_Pkt(device, (uint8 *)tip,
                                 thardware, arp_type)) == NU_NULL)
    {
        return -1;
    }

    UTL_Zero(&mh, sizeof(mh));

    /* This is a psuedo MAC hedaer that is passed to the MAC layer send function. 
       The MAC layer information that is know is filled in. A family of 
       SK_FAM_UNSPEC lets the MAC layer know that this is not an IP datagram and 
       it should not try to resolve a hardware address. */
    memcpy (mh.ar_mac.ar_mac_ether.dest, NET_Ether_Broadaddr, DADDLEN);
    mh.ar_mac.ar_mac_ether.type = protocol_type;
    mh.ar_family = SK_FAM_UNSPEC;
    mh.ar_len = sizeof(mh);    

    /* Send the ARP Packet. */
    (*device->dev_output)(buf_ptr, device, (SCK_SOCKADDR_IP *)&mh, NU_NULL);

    return (NU_SUCCESS);

} /* ARP_Request */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ARP_Cache_Update                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Add an entry to the ARP cache.                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ARP_Interpret                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      n_clicks                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      <Inputs>                            <Description>                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
INT ARP_Cache_Update(uint8 *ipn, uint8 *hrdn)
{
    int16 i, found = -1;
    int32 timer;

    /*
     * linear search to see if we already have this entry
     */
    for (i=0; i < ARP_CACHE_LENGTH; i++)
        if (comparen (ipn, &ARP_Cache[i].arp_ip_addr, 4))
        {
            found = i;
            break;
        }
    /*
     *  if that IP number is not already here, take the oldest
     *  entry.
     *  If it is already here, update the info and reset the timer.
     *  These were pre-initialized to 0, so if any are blank, they
     *  will be
     *  taken first because they are faked to be oldest.
     */
    if (found<0)
    {
        timer = ARP_Cache[0].arp_time;
        found = 0;
        for (i=1; i < ARP_CACHE_LENGTH; i++)
        {
            if (INT32_CMP(ARP_Cache[i].arp_time, timer) < 0)
            {  /* exclude gateways */
                found = i;
                timer = ARP_Cache[i].arp_time;
            }  /* end if ARP_Cache check */
        }  /* end for ARP_CACHE_LENGTH*/
    }  /* end if found < 0 */

    /*
     *   do the update to the cache
     */
    memcpy (ARP_Cache[found].arp_mac_addr, hrdn, DADDLEN);
    memcpy(&ARP_Cache[found].arp_ip_addr, ipn, 4);
    ARP_Cache[found].arp_time = n_clicks();
    ARP_Cache[found].arp_flags = ARP_UP;

    return (found);
} /* ARP_Cache_Update */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ARP_Event                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Process an ARP timer event.                                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_EventsDispatcher                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ARP_Request                                                      */
/*      dll_remove                                                       */
/*      NU_Deallocate_Memory                                             */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      NU_Resume_Task                                                   */
/*      UTL_Timerset                                                     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      <Inputs>                            <Description>                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
VOID ARP_Event(uint16 id)
{
    ARP_RESOLVE_ENTRY  *ar_entry;

    for(ar_entry = ARP_Res_List.ar_head;
        ar_entry != NU_NULL;
        ar_entry = ar_entry->ar_next)
    {
        if (ar_entry->ar_id == id)
            break;
    }

    /* Was an entry found? */
    if(ar_entry)
    {
        switch (ar_entry->ar_pkt_type)
        {

        case ARPREQ :

            /* We will send at most 5 ARP requests.  After sending the 5th
               timeout, and restart the pending task. */
            if (ar_entry->ar_send_count < 5)
            {
                /* Send the ARP request. */
                ARP_Request(ar_entry->ar_device, &ar_entry->ar_dest,
                            (uint8 *)"\0\0\0\0\0\0", EARP, ARPREQ);

                /* Setup a timer event to send the next one. */
                UTL_Timerset(ARPRESOLVE, id, ARPTO, (int32)0);

                /* Increment the number of tries. */
                ar_entry->ar_send_count++;
            }
            else
            {
                /* After 5 tries we have not received a response.  Cleanup. */
                /* Restart the pending task. */
                NU_Resume_Task(ar_entry->ar_task);

                /*  Deallocate this buffer.  */
                MEM_One_Buffer_Chain_Free (ar_entry->ar_buf_ptr, &MEM_Buffer_Freelist);

                /* Deallocate the structure. */
                dll_remove((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);
                NU_Deallocate_Memory(ar_entry);
            }

            break;

#if RARP_INCLUDED

        case RARPQ :

            if (ar_entry->ar_send_count < 5)
            {
                /* At this point the device is DV_RUNNING (It has been
                   initialized) but not DV_UP (There is no IP address attached.
                   In order to send a packet the device must be up and running.
                   So temporarily set the device to up so the RARP request can
                   be sent. */
                ar_entry->ar_device->dev_flags |= DV_UP;

                /* Send the RARP request. */
                ARP_Request(ar_entry->ar_device, (uint32 *)IP_Null,
                            ar_entry->ar_device->dev_mac_addr,
                            ERARP, RARPQ);

                /* Clear the DV_UP flag. */
                ar_entry->ar_device->dev_flags &= (~DV_UP);

                /* Setup a timer event to send the next one in one second. */
                UTL_Timerset(ARPRESOLVE, id, TICKS_PER_SECOND, (int32)0);

                /* Increment the number of tries. */
                ar_entry->ar_send_count++;
            }
            else
            {
                /* A match was found, start the pending task. */
                NU_Resume_Task(ar_entry->ar_task);

                /* Deallocate the structure. */
                dll_remove((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);
                NU_Deallocate_Memory(ar_entry);
            }

            break;

#endif /* RARP_INCLUDED */

            /* This should never be possible, but is here to appease the
                compiler. */
        default :
            ;

        } /* switch */

    }
} /* ARP_Event */

#if RARP_INCLUDED
/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   NU_Rarp                                                                  */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    This function is responsible for resolving the IP address of this host. */
/*    It is the entry point for applications that need to use RARP (Reverse   */
/*    Address Resolution Protocol).  RARP is typically required by disk-less  */
/*    workstations.  Such workstations have no means to store their IP        */
/*    address locally.                                                        */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    Applications                                                            */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    NU_Obtain_Semaphore       Grab the semaphore.                           */
/*    NU_Release_Semaphore      Release the semaphore.                        */
/*    NU_Allocate_Memory        Allocate a block of memory.                   */
/*    NU_Deallocate_Memory      Deallocate a block of memory.                 */
/*    SCK_Suspend_Task          Suspend a task.                               */
/*    NU_Current_Task_Pointer   Get a pointer to the current TCB.             */
/*    dll_dequeue               Remove the first item from a list.            */
/*    dll_remove                Remove an item from anywhere in a list.       */
/*    dll_enqueue               Add an item to the tail of a list.            */
/*    NET_Send                  Send a packet.                                */
/*    UTL_Timerset              Create a timer event.                         */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_MEM_ALLOC              Memory allocation failure.                    */
/*    NU_NO_BUFFERS             No buffers a re available to build the pkt in.*/
/*    NU_RARP_INIT_FAILED       A response was not received to the request.   */
/*    NU_SUCCESS                The IP was successfully resolved.             */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/04/97    Created initial version.                  */
/*                                                                            */
/******************************************************************************/
STATUS ARP_Rarp(CHAR *device_name)
{
    ARP_RESOLVE_ENTRY   *ar_entry;
    DV_DEVICE_ENTRY     *device;
    CHAR                mask[IP_ADDR_LEN];

    /*  Don't let any other users in until we are done.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    /* Look up the device for which an IP address needs to be resolved. */
    if ( (device = DEV_Get_Dev_By_Name(device_name)) == NU_NULL )
        return (NU_INVALID_PARM);

    /* At this point the device is not completely up yet because there is no IP
       address attached to the device.  However, the device must have been
       initialized before Rarp is called. */
    if ( !(device->dev_flags & DV_RUNNING) )
        return (NU_HOST_UNREACHABLE);

    /* At this point the device is DV_RUNNING (It has been intialized) but
       not DV_UP (There is no IP address attached.  In order to send a
       packet the device must be up and running.  So temporarily set the
       device to up so the RARP request can be sent. */
    device->dev_flags |= DV_UP;

    /* Send the RARP request. */
    if ( ARP_Request(device, (uint32 *)IP_Null,  device->dev_mac_addr, ERARP,
                     RARPQ) != NU_SUCCESS)
    {
        return (NU_NO_BUFFERS);
    }

    /* Clear the DV_UP flag. */
    device->dev_flags &= (~DV_UP);


    /* Allocate memory for an ARP resolve entry.  This structure is used to keep
       track of this resolution attempt. */
    if (NU_Allocate_Memory(&System_Memory, (VOID **)&ar_entry,
                           sizeof(*ar_entry),
                           (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
    {
        return (NU_MEM_ALLOC);
    }

    /* Initialize the entry structure. */
    ar_entry->ar_id         = ARP_Res_Count++;
    ar_entry->ar_device     = device;
    ar_entry->ar_dest       = 0xffffffff;
    ar_entry->ar_send_count = 1;            /* Already sent once above. */
    ar_entry->ar_task       = NU_Current_Task_Pointer();
    ar_entry->ar_buf_ptr    = NU_NULL;
    ar_entry->ar_pkt_type   = RARPQ;

    /* Order is not important.  Simply add the new entry to the end of the
       list. */
    dll_enqueue((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);

    /* Transmit the next one in a second. */
    UTL_Timerset (RARP_REQUEST, ar_entry->ar_id, TICKS_PER_SECOND, 0);

    /* Suspend this task pending the resolution of our IP address or a
       timeout. */
    SCK_Suspend_Task(NU_Current_Task_Pointer());

    if (*(uint32 *)device->dev_addr.dev_ip_addr != NU_NULL)
    {
        /* Get the mask associated with an address of this type. */
        IP_Get_Net_Mask(device->dev_addr.dev_ip_addr, mask);

        DEV_Attach_IP_To_Device(device_name, device->dev_addr.dev_ip_addr,
                                mask);
    }

    /* Deallocate the ARP resolve entry structure. */
    dll_remove((tqe_t *)&ARP_Res_List, (tqe_t *)ar_entry);
    NU_Deallocate_Memory(ar_entry);

#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif

    /* Did we find the IP address. */
    if (*(uint32 *)device->dev_addr.dev_ip_addr)
        return (NU_SUCCESS);
    else
        return (NU_RARP_INIT_FAILED);

}   /* end ARP_Rarp */

#endif /* RARP_INCLUDED */
