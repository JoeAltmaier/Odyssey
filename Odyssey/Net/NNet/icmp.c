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
/*      ICMP.C                                            4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ICMP - Internet Control Message Protocol                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      <Data Structure> - <Description>                                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      ICMP_Interpret                  Interprets an ICMP request.      */
/*      ICMP_Echo_Reply                 Echoes out a reply to the        */
/*                                      incoming ICMP request.           */
/*      ICMP_Reflect                    Send an ICMP echo response.      */                                
/*      ICMP_Send_Error                 Sends an ICMP packet.            */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*        ICMP.H                        Holds the defines for ICMP.      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#include "nucleus.h"
#include "target.h"
#include "protocol.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "tcp_errs.h"
#include "netevent.h"
#include "dev.h"
#include "ip.h"
#include "net.h"
#include "icmp.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif

extern sint SQwait;
extern sint OKpackets;

/*************************************************************************/
/*                                                                       */
/*                                                                       */
/*      ICMP_Interpret                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Process received ICMP datagrams.                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Interpret                    Interprets an IP packet.         */ 
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Check_Buffer_Chain           Calculates checksum for each     */
/*                                      buffer chain.                    */
/*      ICMP_Echo_Reply                 Echoes out a reply to the        */
/*                                      incoming ICMP request.           */
/*      MEM_Buffer_Chain_Free           Removes the first node and each  */
/*                                      one in the chain from the source */
/*                                      list and places them at the tail */
/*                                      of the destination list as       */
/*                                      individual nodes.                */
/*      MEM_Buffer_Dequeue              Removes and returns the first    */
/*                                      node in a linked list.           */
/*      NU_TCP_Log_Error                Logs Errors.                     */
/*      RTAB_Redirect                   Redirects the route.             */                              
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
STATUS ICMP_Interpret (NET_BUFFER *buf_ptr, uint32 *ip_source)
{
    ICMP_LAYER      *icp;
    INT             i;
    uint32          src, dst, gateway;

    icp = (ICMP_LAYER *)(buf_ptr->data_ptr);

    /* Increment the total number of ICMP messages received. */
    SNMP_icmpInMsgs_Inc;

    i = icp->icmp_type;

    if (icp->icmp_cksum)
    {        /* ignore if chksum=0 */
        if (IP_Check_Buffer_Chain (buf_ptr))
        {
			NU_Tcp_Log_Error (TCP_ICMP_CKSUM, TCP_RECOVERABLE,
							  __FILE__, __LINE__);

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of ICMP messages received with errors. */
            SNMP_icmpInErrors_Inc;

            return (-1);
        } /* end if */
    } /* end if */

    switch(i)
    {
        case ICMP_ECHO:                    /* ping request sent to me */

            /* Increment the number of Echo requests received. */
            SNMP_icmpInEchos_Inc;

            icp->icmp_type = 0;                /* echo reply type */

            /* Remove the buffer from the buffer list. The buffer will be 
               reused, it will be sent back as a echo reply. */
            MEM_Buffer_Dequeue (&MEM_Buffer_List);

            ICMP_Echo_Reply (buf_ptr);     /* send back */

            /* Increment the number of Echo replies sent. */
            SNMP_icmpOutEchoReps_Inc;


            break;

        case ICMP_REDIRECT:

            dst = *(uint32 *)icp->icmp_ip.ipdest;
            gateway = icp->icmp_gwaddr;
            src = *(uint32 *)ip_source;

            RTAB_Redirect(dst, gateway, RT_GATEWAY | RT_HOST, src);

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of Redirect messages received. */
            SNMP_icmpInRedirects_Inc;

            break;

        case ICMP_SOURCEQUENCH:
            
            OKpackets = 0;
            SQwait += 100;

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of Source Quench messages received. */
            SNMP_icmpInSrcQuenchs_Inc;

             break;

        default:

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of ICMP messages received with errors. */
            SNMP_icmpInErrors_Inc;
            break;

    } /* end switch */

    return (0);
}   /* end icmpinterpret() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ICMP_Send_Error                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send an ICMP packet.                                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Forward                      Checks and attempts to forward   */
/*                                      an ip packet.                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ICMP_Reflect                    Send an ICMP echo response.      */                                
/*      MEM_Buffer_Dequeue              Removes and returns the first    */
/*                                      node in a linked list.           */
/*      NU_TCP_Log_Error                Logs Errors.                     */
/*      IP_Canforward                   Decides if an attempt should be  */
/*                                      made to forward an IP datagram.  */
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
VOID ICMP_Send_Error(NET_BUFFER *buf_ptr, INT type, INT code, uint32 dest,
                     DV_DEVICE_ENTRY *device)
{

    NET_BUFFER      *send_buf;
    IPLAYER         *ip_pkt;
    ICMP_LAYER      *icmp_shdr;
    int16           icmplen;


    ip_pkt = (IPLAYER *)buf_ptr->data_ptr;


    /* Do not generate an ICMP error message in response to an ICMP packet,
       except for the case where we are responding to an echo request. */
    if((ip_pkt->protocol == IP_ICMP_PROT) && ((type != (ICMP_ECHOREPLY))))
    {
       if ( type != ICMP_REDIRECT)
         return;
    }
    /* Do not send an error in response to a broadcast packet. */
    if(buf_ptr->mem_flags & (NET_BCAST | NET_MCAST))
        return;

    /* Perform aditional verification on the packet.  We want to reject the ICMP
       error for the same reasons that we refuse to forward a packet. */
    if ( IP_Canforward(*(uint32 *)ip_pkt->ipdest) != NU_SUCCESS)
        return;


    /* Allocate a buffer to build the ICMP packet in. */
    if ((send_buf = (NET_BUFFER *)MEM_Buffer_Dequeue(&MEM_Buffer_Freelist))== NU_NULL)
    {
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_SEVERE,
                          __FILE__, __LINE__);

        SNMP_icmpOutErrors_Inc;

        return;
    }

    send_buf->mem_dlist = &MEM_Buffer_Freelist;

    /* Begin filling in ICMP header. */
    /* Fill in the type. */
    icmp_shdr = (ICMP_LAYER *)(send_buf->mem_parent_packet + (NET_MAX_ICMP_HEADER_SIZE - sizeof (ICMP_LAYER)));
    icmp_shdr->icmp_type = type;

    
    if (type == ICMP_REDIRECT)
    {
        icmp_shdr->icmp_gwaddr = dest;

        SNMP_icmpOutRedirects_Inc;
    }
    else
    {
        icmp_shdr->icmp_void = 0;

        if (type == ICMP_PARAPROB)
        {
            icmp_shdr->icmp_pptr = code;
            code = 0;

            SNMP_icmpOutParmProbs_Inc;
        }
        else if ( (type == ICMP_UNREACH) &&
                  (code == ICMP_UNREACH_NEEDFRAG && device) )
        {
            icmp_shdr->icmp_nextmtu = (int16)device->dev_mtu;

            SNMP_icmpOutDestUnreachs_Inc;
        }
    }

    icmp_shdr->icmp_code = code;

    /* Compute the sizeof the data that needs to be copied from the bad packet.
       We will copy the IP header plus 8 bytes of data. */
    icmplen = ((ip_pkt->versionandhdrlen & 0x0f) << 2) + 8;

    /* Copy the old IP packet into the data portion of the ICMP packet. */
    memcpy(&icmp_shdr->icmp_ip, ip_pkt, icmplen);


    /* Add the the size of the ICMP header to the ICMP length. */
    icmplen += sizeof(ICMP_LAYER);

    /* Point to where the new IP header should begin. */
    send_buf->data_ptr = (uint8 *)icmp_shdr - sizeof(IPLAYER);


    /* Copy the IP header. */
    memcpy(send_buf->data_ptr, ip_pkt, sizeof(IPLAYER));

    send_buf->data_ptr = (uint8 *)icmp_shdr;
    send_buf->mem_buf_device = device;

    send_buf->mem_total_data_len = icmplen + sizeof(IPLAYER);
    send_buf->data_len =  icmplen + sizeof(IPLAYER);
    
    ICMP_Reflect(send_buf);

} /* ICMP_Send_Error */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ICMP_Reflect                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send an ICMP echo response.                                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ICMP_Echo_Reply                 Echoes out a reply to the        */
/*                                      incoming ICMP request.           */
/*      ICMP_Send_Error                 Sends an ICMP packet.            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Check_Buffer_Chain           Calculates checksum for each     */
/*                                      buffer chain.                    */
/*      IP_Send                         Sends an IP packet.              */
/*      MEM_Buffer_Chain_Free           Removes the first node and each  */
/*                                      one in the chain from the source */
/*                                      list and places them at the tail */
/*                                      of the destination list as       */
/*                                      individual nodes.                */
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
VOID ICMP_Reflect(NET_BUFFER *buf_ptr)
{
    IPLAYER         *ip_pkt;
    uint32          t, icmpsrc;
    DV_DEVICE_ENTRY *temp_dev;
    ICMP_LAYER      *icmp_l;
    STATUS          stat;

    /*  Point to the ICMP packet.  */
    ip_pkt = (IPLAYER *)(buf_ptr->data_ptr - sizeof (IPLAYER)); 

    /* Keep a copy of the original destination. */
    t = *(uint32 *)ip_pkt->ipdest;
    
    /* Make the source the new destination. */
    memcpy(ip_pkt->ipdest, ip_pkt->ipsource, IP_ADDR_LEN);

    /* Set the new source address.  If an exact math can not be found then use
       the source address for the receiving interface. */
    for(temp_dev = DEV_Table.dv_head;
        temp_dev;
        temp_dev = temp_dev->dev_next)
    {
        /* Is there an exact match on the IP address. */
        if ( *(uint32 *)temp_dev->dev_addr.dev_ip_addr == t)
            break;
    }

    /* If a match was found use the IP address for that device, else use the IP
       address for the receiving device. */
    if (temp_dev)
        icmpsrc = *(uint32 *)temp_dev->dev_addr.dev_ip_addr;
    else
        icmpsrc = *(uint32 *)buf_ptr->mem_buf_device->dev_addr.dev_ip_addr;


    /* Compute the ICMP checksum. */
    icmp_l = (ICMP_LAYER *)(buf_ptr->data_ptr);

    icmp_l->icmp_cksum = 0;
    icmp_l->icmp_cksum = IP_Check_Buffer_Chain (buf_ptr);

    /* Send this packet. */
    stat = IP_Send(buf_ptr, NU_NULL, *(uint32 *)ip_pkt->ipdest,
                icmpsrc, NU_NULL, IP_TIME_TO_LIVE, IP_ICMP_PROT, 0, NU_NULL);

    if ((stat != NU_SUCCESS) && (stat != NU_UNRESOLVED_ADDR))
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated when the transmit complete
           interrupt occurs. */
        MEM_One_Buffer_Chain_Free (buf_ptr, buf_ptr->mem_dlist);
    }

    /* Increment the number of ICMP messages sent. */
    SNMP_icmpOutMsgs_Inc;

} /* ICMP_Reflect */

/******************************************************************************/
/*                                                                            */
/* FUNCTION                                                                   */
/*                                                                            */
/*      ICMP_Echo_Reply                                                       */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   send out an icmp packet, probably in response to a ping operation        */
/*   interchanges the source and destination addresses of the packet,         */
/*   puts in my addresses for the source and sends it                         */
/*                                                                            */
/*   does not change any of the ICMP fields, just the IP and dlayers          */
/*   returns 0 on okay send, nonzero on error                                 */
/*                                                                            */
/* CALLED BY                                                                  */
/*      ICMP_Interpret                  Interprets an ICMP request.           */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*      ICMP_Reflect                    Sends an ICMP echo response.          */
/*                                                                            */
/*  NAME             DATE     REMARKS                                         */
/*                                                                            */
/*  Glen Johnson   05/01/96   Fixed a bug in the transmission of the ICMP     */
/*                            packet.                                         */
/*                                                                            */
/******************************************************************************/
STATUS ICMP_Echo_Reply (NET_BUFFER *buf_ptr)
{

    /* Do not send an error in response to a broadcast packet. */
    if(buf_ptr->mem_flags & (NET_BCAST | NET_MCAST))
        return(0);

    /* Set the deallocation list. */
    buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

    ICMP_Reflect(buf_ptr);

    return (NU_SUCCESS);

}  /* end ICMP_Echo_Reply */

