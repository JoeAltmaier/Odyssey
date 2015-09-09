/*************************************************************************/
/*                                                                       */
/*    CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.            */
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
/***************************************************************************
 *                                                                         *
 *     part of:                                                            *
 *     TCP/IP kernel for NCSA Telnet                                       *
 *     by Tim Krauskopf                                                    *
 *                                                                         *
 *     National Center for Supercomputing Applications                     *
 *     152 Computing Applications Building                                 *
 *     605 E. Springfield Ave.                                             *
 *     Champaign, IL  61820                                                *
 *                                                                         *
 ***************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  UDP.C                                                     4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  UDP Protocol routines                                                   */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  UDP_Read                                                                */
/*  UDP_Send                                                                */
/*  UDP_Append                                                              */
/*  UDP_Cache_Route                                                         */
/*  udpinterpret                                                            */
/*                                                                          */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*	No other file dependencies												*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/*																			*/
/****************************************************************************/

/*
 *  Includes
 */
#include "nucleus.h"
#include "target.h"
#include "mem_defs.h"
#include "protocol.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "tcp_errs.h"
#include "netevent.h"
#include "arp.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif
#include "ip.h"
#include "sockext.h"

/* Import external variables */
extern NET_BUFFER_HEADER MEM_Buffer_Freelist;
extern NU_TASK           NU_EventsDispatcher_ptr;
extern NU_TASK           timer_task_ptr;

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*      udpinterpret                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*   Take an incoming UDP packet and make it available to the user level    */
/*   routines.  Currently keeps the last packet coming in to a port.        */
/*                                                                          */
/*   Limitations :                                                          */
/*                                                                          */
/*   Can only listen to one UDP port at a time, only saves the last         */
/*   packet received on that port.  Port numbers should be assigned         */
/*   like TCP ports.                                                        */
/*                                                                          */
/* CALLED BY                                                                */
/*      IP_Interpret                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      MEM_Buffer_Chain_Free                                               */
/*      MEM_Buffer_Dequeue                                                  */
/*      NU_Tcp_Log_Error                                                    */
/*      tcpcheck                                                            */
/*      UDP_Append                                                          */
/*                                                                          */
/*  NAME             DATE     REMARKS                                       */
/*                                                                          */
/*  Glen Johnson   03/14/96   Fixed problem in the search of the uportlist  */
/*                            for the destination UDP port.                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
int16 udpinterpret (NET_BUFFER *buf_ptr, struct pseudotcp *tcp_chk)
{
    uint16          hischeck, mycheck;
    uint16          i;
    struct uport    *uptr;
    UDPLAYER        *p;

    /* Increment the number of UDP packets received. */
    SNMP_udpInDatagrams_Inc;

    /* Grab a pointer to the udp layer */
    p = (UDPLAYER *)buf_ptr->data_ptr;

    /* Was a checksum computed by the foreign host?
       If so verfiy the checksum.
    */
    if (p->check)
    {
        hischeck = p->check;
        p->check = 0;

        /* Perform the checksum. */
        mycheck = tcpcheck( (uint16 *) tcp_chk, buf_ptr);

        /* If a checksum of zero is computed it should be replaced with 0xffff. */
        if (mycheck == 0)
            mycheck = 0xFFFF;

        if (hischeck != mycheck)
        {
            /* The Checksum failed log an error and drop the packet. */
            NU_Tcp_Log_Error (TCP_UDP_CKSUM, TCP_RECOVERABLE, __FILE__,
                              __LINE__);

            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of datagrams received with errors. */
            SNMP_udpInErrors_Inc;

            return (2);
        }
        p->check = hischeck;                  /* put it back */
    }


    /*
     *  did we want this data ?  If not, then let it go, no comment
     *  If we want it, copy the relevent information into our structure
     */
    uptr = NU_NULL;
    for (i = 0; i < NUPORTS; i++)
    {
        /* Check to make sure this entry in the uportlist actually points to a
           port structure, and check for the destination port matching the local
           port.  Short circuit evaluation will cause the test to fail
           immediately if the pointer to the port structure is NULL.
        */
        if ((uportlist[i]) && (p->dest ==  uportlist[i]->up_lport))
        {
            /* When a match is found our search is over. */
            uptr = uportlist[i];
            break;
        }
    }  /* end for i =0 to NUPORTS */

    /*  If we did not find a port then we are not waiting for this
        so return.  */
    if (uptr == NU_NULL)
    {
        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

        /* Increment the number of datagrams received for which there was no
            port. */
        SNMP_udpNoPorts_Inc;

        return(-1);
    }

    /* Make sure there is room to add the received packet, then call UDP_Append 
       to add this packet to the ports receive list. */
    if (uptr->in_dgrams >= UMAXDGRAMS)
    {
        /* Increment the number of datagrams received with errors. */
        SNMP_udpInErrors_Inc;

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        return(-1);
    }

    /* Pull the current buffer chain off of the receive list. It will be added 
       to the a port's receive list by UDP_Append. */ 
    MEM_Buffer_Dequeue(&MEM_Buffer_List);

    /* Add the received packet to the appropriate port's receive list. */
    UDP_Append(i, buf_ptr);

    return (NU_SUCCESS);
}   /* udpinterpret */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UDP_Append                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Add a received packet to a sockets receive list.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      udpinterpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Enqueue                                               */
/*      TL_Put_Event                                                     */
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
STATUS UDP_Append(INT port_index, NET_BUFFER *buf_ptr)
{
    struct uport *uptr = uportlist[port_index];

    /* Strip off the UDP layer. */
    buf_ptr->data_ptr           += sizeof (UDPLAYER);
    buf_ptr->data_len           -= sizeof (UDPLAYER);
    buf_ptr->mem_total_data_len -= sizeof (UDPLAYER);

    /* Place the datagram onto this ports datagram list. */
    MEM_Buffer_Enqueue(&uptr->dgram_list, buf_ptr);

    /* Update the number of buffered datagrams. */
    uptr->in_dgrams++;

    /* If there is a task pending data on the port, then set an event to
       resume that task. */
    if(uptr->RXTask)
        TL_Put_Event (UDPDATA, port_index);

    return (NU_SUCCESS);

} /* UDP_Append */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UDP_Read                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Get the data from the UDP buffer and transfer it into your buffer.   */
/*  Returns the number of bytes transferred or -1 of none available.     */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Recv_From                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      MEM_Buffer_Chain_Free                                            */
/*                                                                       */
/*************************************************************************/
int16 UDP_Read (struct uport *uptr, char *buffer, struct addr_struct *from,
              struct sock_struct *sockptr)
{
    UDPLAYER    HUGE    *pkt_ptr;
    IPLAYER             *ip_pkt_ptr;
    INT                 bytes_copied;
    NET_BUFFER          *buf_ptr;
    uchar       HUGE    *ch_ptr;

    /* Check to see if there are any packets waiting. */
    if (uptr->dgram_list.head == NU_NULL)
        return (NU_NO_DATA_TRANSFER);

    /* Set up a pointer to the packet stored in first buffer in the list */
    ch_ptr = uptr->dgram_list.head->data_ptr;
    ch_ptr -= sizeof (UDPLAYER);

    pkt_ptr = (UDPLAYER *) ch_ptr;
    ip_pkt_ptr = (IPLAYER *) (ch_ptr - sizeof (IPLAYER));

    /* Move the data into the caller's buffer, copy it from the buffer chain. */

    /* Get a pointer to the data */
    buf_ptr = uptr->dgram_list.head;

    /* Do the parent buffer first */
    memcpy(buffer, buf_ptr->data_ptr, buf_ptr->data_len);

    /* Update the bytes copied. */
    bytes_copied = buf_ptr->data_len;

    /* Loop through the chain if needed and copy all buffers. */
    while (buf_ptr->next_buffer != NU_NULL)
    {
        /* Move to the next buffer in the chain */
        buf_ptr = buf_ptr->next_buffer;

        /* Copy the data */
        memcpy(buffer + bytes_copied, buf_ptr->data_ptr, buf_ptr->data_len);

        /* Update the bytes copied. */
        bytes_copied += buf_ptr->data_len;

     } /* end while there are buffers in the chain */

    /*  Get his IP number.  */
    memcpy (&(from->id), ip_pkt_ptr->ipsource, IP_ADDR_LEN);

    /*  Get his port number. */
    from->port = (int16)intswap(pkt_ptr->source);

    /*  Update the socket descriptor with foreign address
        information.  */
    memcpy (&(sockptr->foreign_addr.ip_num), ip_pkt_ptr->ipsource, IP_ADDR_LEN);
    sockptr->foreign_addr.port_num = (int16)intswap(pkt_ptr->source);

    /* Place this buffer back onto the free list. */
    MEM_Buffer_Chain_Free (&uptr->dgram_list, &MEM_Buffer_Freelist);

    /* Update the number of buffered datagrams. */
    uptr->in_dgrams--;

    return (bytes_copied);

} /* UDP_Read */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UDP_Send                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Send some data out in a udp packet.                                  */
/*                                                                       */
/*  Returns 0 on ok send, non-zero for an error                          */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Send_To                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ARP_Find_Entry                                                   */
/*      intswap                                                          */
/*      IP_Send                                                          */
/*      MEM_Buffer_Chain_Dequeue                                         */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      SCK_Suspend_Task                                                 */
/*      NU_Current_Task_Pointer                                          */
/*      UDP_Cache_Route                                                  */
/*      UTL_Checksum                                                     */
/*                                                                       */
/*************************************************************************/
int16 UDP_Send (struct uport *uptr, uint8 *buffer, uint16 nbytes,
                uint16 sock_options)
{
    UDPLAYER            *udp_pkt;
    NET_BUFFER          *buf_ptr, *work_buf;
    STATUS              stat;
    SCK_SOCKADDR_IP     dest;
    NU_TASK             *task_ptr;
    int32               bytes_left;
    uint8               *work_ptr;
    struct sock_struct  *sock_ptr = socket_list[uptr->up_socketd]; 

    /* Before we do anything else make sure a route to the host is up. */
    if ( (stat = UDP_Cache_Route (uptr, *(uint32 *)uptr->up_faddr)) != NU_SUCCESS)
        return stat;

    /* Extract the local IP address to use from the route. */
    memcpy (uptr->up_laddr, 
            uptr->up_route.rt_route->rt_device->dev_addr.dev_ip_addr, 4);

    /*  Don't send more than we have concluded is our maximum.  */
    if (nbytes > UMAXLEN)
        nbytes = UMAXLEN;

    /* Allocate a buffer, or chain of buffers, to place the packet in. The size is 
       increased by the various protocols header sizes to make room for them.*/
    buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, nbytes 
        + NET_MAX_UDP_HEADER_SIZE);
    
    if(buf_ptr == NU_NULL)
    {
        return (NU_NO_BUFFERS);
    }

    /* Compute the data size with the UDP header. */
    buf_ptr->mem_total_data_len = (nbytes + sizeof (UDPLAYER));

    /* Set the deallocation list pointer. */
    buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

    /* Set the data pointer to the correct location. */
    buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + 
            (NET_MAX_UDP_HEADER_SIZE - sizeof (UDPLAYER)));

    /* Set the UDP header pointer to that it can be filled in. */
    udp_pkt = (UDPLAYER *)buf_ptr->data_ptr;

    /* Initialize the local and foreign port numbers. */
    udp_pkt->source = uptr->up_lport;
    udp_pkt->dest   = uptr->up_fport;
    
    /*  Get the length of the buffer.  */
    udp_pkt->length = intswap ((int16)buf_ptr->mem_total_data_len);

    /*  Move the data into the output buffer. This will depend on the size of
        the data. The data may need to be copied into the multiple buffers
        in the buffer chain. */

    /* Get the total number of bytes that must be copied. */
    bytes_left = nbytes;

    /* Will it all fit into one buffer? */
    if (bytes_left <= (NET_PARENT_BUFFER_SIZE - NET_MAX_UDP_HEADER_SIZE))
    {
        /* Store the number of bytes held by this buffer, this includes the 
           protocol headers. */
        buf_ptr->data_len = buf_ptr->mem_total_data_len;

        /* Copy the data after the UDP header. */
        memcpy  (buf_ptr->data_ptr + sizeof (UDPLAYER), buffer, nbytes);
    }
    else
    {

        /* Fill the parent buffer in the chain. This one is slightly smaller than
           the rest in the chain. */
        memcpy (buf_ptr->data_ptr + sizeof (UDPLAYER), buffer, NET_PARENT_BUFFER_SIZE - 
            NET_MAX_UDP_HEADER_SIZE);

        /* Take off the bytes just copied from the total bytes left. */
        bytes_left -= (NET_PARENT_BUFFER_SIZE - NET_MAX_UDP_HEADER_SIZE);

        /* Store the number of bytes in this buffer. */
        buf_ptr->data_len = ((NET_PARENT_BUFFER_SIZE - NET_MAX_UDP_HEADER_SIZE)
            + sizeof (UDPLAYER));

        /* Get a work pointer and bump it the number of bytes just copied. */
        work_ptr = (buffer + (NET_PARENT_BUFFER_SIZE - NET_MAX_UDP_HEADER_SIZE));

        /* Get a work buffer pointer to the buffer chain */
        work_buf = buf_ptr;

        /* Break the rest up into the multiple buffers in the chain. */
        do 
        {
            /* Move to the next buffer in the chain */
            work_buf = work_buf->next_buffer;

            /* If the bytes left will fit into one buffer then copy them over */
            if (bytes_left <= NET_MAX_BUFFER_SIZE)
            {
                /* Copy the rest of the data. */
                memcpy (work_buf->mem_packet, work_ptr, bytes_left);

                /* Set the data ptr */
                work_buf->data_ptr = work_buf->mem_packet;

                /* Store the number of bytes in this buffer. */
                work_buf->data_len = bytes_left;
            }
            else
            {
                /* Copy all that will fit into a single buffer */
                memcpy (work_buf->mem_packet, work_ptr, NET_MAX_BUFFER_SIZE);

                /* Update the buffer pointer */
                work_ptr += NET_MAX_BUFFER_SIZE;

                /* Set the data ptr */
                work_buf->data_ptr = work_buf->mem_packet;

                /* Store the number of bytes in this buffer. */
                work_buf->data_len = NET_MAX_BUFFER_SIZE;
            }

            /* Update the data bytes left to copy. */
            bytes_left -= NET_MAX_BUFFER_SIZE;

        } while (bytes_left > 0);

    } /* end if it will fit into one buffer */

    /* Clear the checksum field before calcualting the real checksum. */
    udp_pkt->check = 0;

    /*  Calculate the checksum. */
    udp_pkt->check = UTL_Checksum(buf_ptr, uptr->up_laddr, uptr->up_faddr, 
                                    IP_UDP_PROT);
    
    /* If a checksum of zero is computed it should be replaced with 0xffff. */
    if (udp_pkt->check == 0)
        udp_pkt->check = 0xFFFF;

    /* Send this packet. */
    stat = IP_Send( (NET_BUFFER *)buf_ptr, &uptr->up_route, 
                    uptr->up_route.rt_ip_dest.sck_addr, *(uint32 *)uptr->up_laddr, 
                    IP_ALLOWBROADCAST, IP_TIME_TO_LIVE, IP_UDP_PROT, 
                    IP_TYPE_OF_SERVICE, sock_ptr->s_moptions);

    if (stat == NU_SUCCESS)
    {
        /* Increment the number of UDP datagrams transmitted. */
        SNMP_udpoutDatagrams_Inc;

    }
    else if (stat == NU_UNRESOLVED_ADDR)
    {

        task_ptr = NU_Current_Task_Pointer();
       
        if ( (task_ptr != &NU_EventsDispatcher_ptr) && 
             (task_ptr != &timer_task_ptr) )
        {
            SCK_Suspend_Task(task_ptr);

            /* We need to know if the hardware address was successfully resolved so 
               look up the IP address of the next hop. If the next hop is a gateway 
               then see if the address for the gateway was resolved.  Else see if 
               the address for the remote host was resolved. */
            if (uptr->up_route.rt_route->rt_flags & RT_GATEWAY)
                dest.sck_addr = uptr->up_route.rt_route->rt_gateway.sck_addr;
            else
                dest.sck_addr = *(uint32 *)uptr->up_faddr;

            if (ARP_Find_Entry(&dest) != NU_NULL)
            {
                /* Increment the number of UDP datagrams transmitted. */
                SNMP_udpoutDatagrams_Inc;

            }
            else
                return -1;
        }

    }
    else
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated later by the apropriate MAC layer
           TX routine. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);

        return -1;
    }

    /*  If the send went ok, then return the number of bytes sent. */
    return((int16)nbytes);

}  /* UDP_Send */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UDP_Cache_Route                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Cache a route for a UDP socket.                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      UDP_Send                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Find_Route                                                    */
/*      RTAB_Free                                                        */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
STATUS UDP_Cache_Route (struct uport *uprt, uint32 ip_addr)
{
    RTAB_ROUTE      *ro;
    uint32          search_addr = ip_addr;

    /* If the destination is the limited broadcast address, update it to the
       broadcast address for the primary network interface. This is for the 
       purposes of route discovery below. */
    if (search_addr == IP_ADDR_BROADCAST)
        search_addr = DEV_Table.dv_head->dev_addr.dev_net_brdcast;

    ro = &uprt->up_route;

    /* If there is already a route cached but the destination IP addresses 
       don't match then free the route. This comparison is done to the real 
       IP address not the search address, they will only be different when a 
       route for the limited broadcast address is desired. */
    if (ro->rt_route && (ro->rt_ip_dest.sck_addr != ip_addr) )
    {
        RTAB_Free(ro->rt_route);
        ro->rt_route = NU_NULL;
    }

    /* If there is no cached route then try to find one. */
    if ( ro->rt_route == NU_NULL )
    {
        ro->rt_ip_dest.sck_addr = search_addr;
        ro->rt_ip_dest.sck_family = SK_FAM_IP;
        IP_Find_Route(ro);
    }

    /* If it was the limited broadcast address (255.255.255.255) that a 
       route was desired for then the route address will be wrong. Go ahead 
       and update it for all cases.
    */
    ro->rt_ip_dest.sck_addr = ip_addr;
    
    if (ro->rt_route)
    {
        return NU_SUCCESS;
    }
    else
        return (NU_HOST_UNREACHABLE);

} /* UDP_Cache_Route */
