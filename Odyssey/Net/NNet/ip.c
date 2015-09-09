/*************************************************************************/
/*                                                                       */
/*    CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.            */
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
*                                                                          *
*      part of:                                                            *
*      TCP/IP kernel for NCSA Telnet                                       *
*      by Tim Krauskopf                                                    *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*
*  Revision history:
*
*   10/87  Initial source release, Tim Krauskopf
*   2/88  typedefs of integer lengths, TK
*   5/88    clean up for 2.3 release, JKM   
*   9/91    Add input sanity checking, reorder tests, Nelson B. Bolyard
*
*/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  IP.C                                                       2.3            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  IP level routines, including ICMP                                         */
/*  also includes a basic version of UDP, not generalized yet                 */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                             */
/*                                                                          */
/*  global compenent data stuctures defined in this file                      */
/*                                                                            */
/* FUNCTIONS                                                                   */
/*  IP_Add_Multi                                                              */
/*  IP_Broadcast_Addr                                                         */
/*  IP_Canforward                                                             */
/*  IP_Checklen                                                               */
/*  IP_Delete_Multi                                                           */
/*  IP_Find_Route                                                             */
/*  IP_Forward                                                                */
/*  IP_Fragment                                                               */
/*  IP_Free_Queue_Element                                                     */
/*  IP_Get_Multi_Opt                                                          */
/*  IP_Get_Net_Mask                                                           */
/*  IP_Get_Opt                                                                */
/*  IP_Initialize                                                             */
/*  IP_Insert_Frag                                                            */
/*  IP_Interpret                                                              */
/*  IP_Lookup_Multi                                                           */
/*  IP_Option_Copy                                                            */
/*  IP_Reassembly                                                             */
/*  IP_Reassembly_Event                                                       */
/*  IP_Remove_Frag                                                            */
/*  IP_Send                                                                   */
/*  IP_Set_Multi_Opt                                                          */
/*  IP_Set_Opt                                                                */
/*  IP_Localaddr                                                              */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*  No other file dependencies                                                */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*  NAME                DATE        REMARKS                                   */
/*                                                                            */
/* Maiqi Qian      12/06/96           Fixed the time wrap around (spr0229)    */
/*                                                                            */
/******************************************************************************/

/*
*   Includes
*/
#include "protocol.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "target.h"
#include "tcp.h"
#include "tcp_errs.h"
#include "netevent.h"
#include "ip.h"
#include "net.h"
#include "icmp.h"
#include "mem_defs.h"
#include "sockext.h"
#include "igmp.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif

/*  This is the id field of outgoing IP packets. */
int16 IP_Ident;

#if INCLUDE_IP_FORWARDING
  /* This is a flag that controls the forwarding of IP packets. */
  INT         IP_Forwarding;

  /* This flag is used to control the sending of ICMP redirect messages. */
  INT         IP_Sendredirects;

  /* A cached IP forwarding route. */
  RTAB_ROUTE  IP_Forward_Rt;
#endif

#if INCLUDE_IP_REASSEMBLY
  IP_QUEUE      IP_Frag_Queue;
#endif

CHAR IP_Brd_Cast[IP_ADDR_LEN] = {(char)0xff, (char)0xff, (char)0xff, (char)0xff};
CHAR IP_Null[IP_ADDR_LEN]     = {0, 0, 0, 0};

/* class A mask */
const uchar   IP_A_Mask[4] = {255,0,0,0};

/* class B mask */
const uchar   IP_B_Mask[4] = {255,255,0,0};

/* class C mask */
const uchar   IP_C_Mask[4] = {255,255,255,0};

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Initialize                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Initialize the global data associated with the IP layer.             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*  Glen Johnson                                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      protinit                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Zero                                                         */
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
/*      Glen Johnson    03/18/98            Created Initial version      */
/*************************************************************************/
VOID IP_Initialize(VOID)
{
    /* Identification field of outgoing packets. */
    IP_Ident = 1;

#if INCLUDE_IP_FORWARDING
    /* Enable IP Forwarding by default. */
    IP_Forwarding = 1;

    /* Enable the sending of ICMP redirects by default. */
    IP_Sendredirects = 1;

    UTL_Zero((CHAR *)&IP_Forward_Rt, sizeof(IP_Forward_Rt));
#endif

#if INCLUDE_IP_REASSEMBLY
    IP_Frag_Queue.ipq_head = NU_NULL;
    IP_Frag_Queue.ipq_tail = NU_NULL;

    /* Record the timeout value for ip reasembly. This is defined in Nucleus
       PLUS clock ticks. So we divide by TICKS_PER_SECOND to get the number
       of seconds. */
    SNMP_ipReasmTimeout (IP_FRAG_TTL / TICKS_PER_SECOND);
#endif

}  /* end IP_Initialize */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Checklen                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Compute the IP checksum.                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IGMP_Interpret                                                   */
/*      IGMP_Send                                                        */
/*      IP_Interpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ipcheck                                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
uint16 IP_Checklen (int8 *s, uint16 len)
{
    if((len == 0) || (len > 2048))
    {
	    return (0xDEAD);  /* bad checksum, trust me! */
    } /* end if */
    return ipcheck ((uint16 *)s, len);
}   /* end IP_Checklen() */

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*      ipinterpret                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Called by the packet demuxer to interpret a new ip packet.  Checks      */
/*  the validity of the packet (checksum, flags) and then passes it         */
/*  on to the appropriate protocol handler.                                 */
/*                                                                          */
/* CALLED BY                                                                */
/*      NET_Demux                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      intswap                                                             */
/*      ipcheck                                                             */
/*      udpinterpret                                                        */
/*      TCP_Interpret                                                       */
/*      ICMP_Interpret                                                      */
/*      dll_update_lists                                                    */
/*                                                                          */
/*  NAME             DATE     REMARKS                                       */
/*                                                                          */
/*  Glen Johnson   03/15/96   Fixed the check for broadcast addrs.          */
/*                                                                          */
/****************************************************************************/
STATUS IP_Interpret (IPLAYER *pkt, DV_DEVICE_ENTRY *device, NET_BUFFER *buf_ptr)
{
	uint16              iplen;
	uint16              hlen;
    uint32              total;
    DV_DEVICE_ENTRY     *temp_dev;
    struct pseudotcp    tcp_chk;
    NET_BUFFER          *buf;

#if INCLUDE_IP_REASSEMBLY
    IP_QUEUE_ELEMENT    *fp;
    IPLAYER             *ip;
    NET_BUFFER          *a_buf;
#endif

#if INCLUDE_IP_MULTICASTING
    IP_MULTI            *ipm;
#endif

    /* Increment SNMP ipInReceives.  This value counts all packets received.
       Even those that have errors. */
    SNMP_ipInReceives_Inc;

#if DHCP_INCLUDED||BOOTP_INCLUDED
    /* If an IP address has not been attached to the device this packet should
       only be received if it is an IP packet and it is a broadcast packet.
     */
    if ( !(device->dev_flags & DV_UP) )
    {
	    if ( !comparen(pkt->ipdest, IP_Brd_Cast, IP_ADDR_LEN) )
	    {
	        /* Drop the packet by placing it back on the buffer_freelist. */
	        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);


	        return (NU_SUCCESS);
	    }
    }
#else
    /* Drop all packets until the device has an IP address attached. */
    if ( !(device->dev_flags & DV_UP) )
    {
	    /* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
	    return (NU_SUCCESS);
    }
#endif /* DHCP_INCLUDED */

    /*
	 *      Extract total length of packet
	 */
	buf_ptr->mem_total_data_len = iplen = intswap (pkt->tlen);

    /* Check to see if the total data length is less than the sum of the data 
       lengths the buffers in the chain. This at first sounds impossible. However
       data_len comes from the size reported by the driver. It is not unusual to 
       receive a packet that has been padded. The dirver does not distinguish 
       between real data and padded data. However, the IP header contains the 
       true data length. We want to use the smaller value.
    */
    for ( buf = buf_ptr, total = buf->mem_total_data_len;
	      buf;
	      buf = buf->next_buffer)
    {
    	if (buf->data_len > total)
	        buf->data_len = total;

	    total -= buf->data_len;
    }
    
	hlen = (pkt->versionandhdrlen & 0x0f) << 2;

	if ((hlen < sizeof(pkt))          /* Header too small */
		|| (iplen < hlen)                        /* Header inconsistent */
		|| (iplen > 2048))                         /* WAY too big */
	{
		NU_Tcp_Log_Error (TCP_BAD_IP_CKSUM, TCP_RECOVERABLE,
						  __FILE__, __LINE__);

	    /* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    /* Increment the number of IP packets received with header errors. */
	    SNMP_ipInHdrErrors_Inc;

	    return (1);                /* drop packet */
	} /* end if */


	/*
	 *      checksum verification of IP header
	 */

    if (IP_Checklen ((int8 *)pkt,
		    (uint16)((pkt->versionandhdrlen & 0x0f) << 1)))
	{
		NU_Tcp_Log_Error (TCP_BAD_IP_CKSUM, TCP_RECOVERABLE,
						  __FILE__, __LINE__);

	    /* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    /* Increment the number of IP packets received with header errors. */
	    SNMP_ipInHdrErrors_Inc;

	    return(1);              /* drop packet */
	}

	/* silently toss this legal-but-useless packet */
	if (iplen <= hlen)
	{
	    /* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    /* Increment the number of IP packets received with header errors. */
	    SNMP_ipInHdrErrors_Inc;

	    return (1);
	}

	/*
	 *      See if there are any IP options to be handled.
	 *      We don't understand IP options, post a warning to the user and drop
	 *      the packet.
	 */

	/* check for options in packet */
	if (hlen > sizeof (IPLAYER))
	{
		NU_Tcp_Log_Error (TCP_IP_WITH_OPTS, TCP_RECOVERABLE,
						  __FILE__, __LINE__);
    	/* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    /* Increment the number of IP packets received with header errors. */
	    SNMP_ipInHdrErrors_Inc;

	    return (1);
	} /* end if */

	iplen -= hlen;

    /* The following loop checks to see if the packet is for us.  A match occurs
       if the destination IP matches one of our IP addresses or if it is a
       broadcast address on a device that supports broadcasting.
    */
    for(temp_dev = DEV_Table.dv_head;
	    temp_dev != NU_NULL;
	    temp_dev = temp_dev->dev_next)
    {
	    /* Is there an exact match on the IP address. */
	    if (comparen(temp_dev->dev_addr.dev_ip_addr, pkt->ipdest, IP_ADDR_LEN))
	        break;

	    /* If the current device is the same one the packet was received on and
	       the device supports broadcast packets check for a match on supported
	       broadcast addresses.
	    */
	    if ( (temp_dev == device) && (temp_dev->dev_flags & DV_BROADCAST) )
	    {
	        /* If the destination is either of the broadcast addresses,
	           then keep it.
	        */
	        if (comparen(pkt->ipdest, IP_Null, IP_ADDR_LEN) ||
		    comparen(pkt->ipdest, IP_Brd_Cast, IP_ADDR_LEN))
	        {
		    break;
	        }

	        /* Is this a broadcast for our network. */
	        if (temp_dev->dev_addr.dev_net == *(uint32 *)pkt->ipdest)
		    break;
	        if (temp_dev->dev_addr.dev_net_brdcast == *(uint32 *)pkt->ipdest)
		    break;

	    }
    }

#if DHCP_INCLUDED||BOOTP_INCLUDED
    /* Before this packet is rejected check to see if the device has an
       IP_address.  If not then pass it to UDP.  It could be a DHCP response
       packet.
    */
    if ( !(device->dev_flags & DV_UP) )
    {
	    if (pkt->protocol == IP_UDP_PROT)
	    {
	        temp_dev = device;
	    }
    }
#endif /* DHCP_INCLUDED */

    /* If the destination IP address did not match any of ours, then check to 
       see if it is destined for a multicast address. */
    if (!temp_dev)
    {
    	if (IP_MULTICAST_ADDR(longswap(*(uint32 *)pkt->ipdest)))
	    {
#if INCLUDE_IP_MULTICASTING            
	        /* NOTE: If/when multicast routing support is added it will need to 
    	       be handled here. */

	        /* Do we belong to the multicast group on the receive device. */
	        ipm = IP_Lookup_Multi( *(uint32 *)pkt->ipdest, &device->dev_addr);
	        if (ipm != NU_NULL)
		        /* We belong to the multicast group. Set temp_dev so the checks 
		        below will be passed. */
		        temp_dev = device;
#else
    	    /* Multicasting support was not desired so drop the packet by 
	           placing it back on the buffer_freelist. */
	        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
	        return (1);

#endif /* INCLUDE_IP_MULTICASTING */
	    }
    }

    if (!temp_dev)
    {
#if INCLUDE_IP_FORWARDING
	    /* This packet is not for us. Attempt to forward the packet if
	       possible. */

	    /* Remove the buffer that we have been processing from the buffer_list.
	       The IP forwarding function will handle the deallocation. */
	    buf_ptr = (NET_BUFFER *)MEM_Buffer_Dequeue(&MEM_Buffer_List);

	    /* Initialize the deallocation list pointer. */
	    buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

	    if (IP_Forwarding)
	        IP_Forward(buf_ptr);

#else
    	/* Increment the number of IP packets received with the wrong IP addr.*/
	    SNMP_ipInAddrErrors_Inc;

    	/* Drop it. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
#endif

	    return (1);
    }

#if INCLUDE_IP_REASSEMBLY

    /* Swap the frags field so that we do not have to keep swapping it over and over
       again throughout that reasembly code. */
    pkt->frags = intswap (pkt->frags);

    /* If offset or IP_MF are set then this is a fragment and we must 
       reassemble. */
    if (pkt->frags & ~IP_DF)
	{
	    /* Increment the number of IP fragments that have been received. */
	    SNMP_ipReasmReqds_Inc;

	    /* Search the list of fragmented packets to see if at least one fragment
	       from the same packet was previously received. */
	    for (fp = IP_Frag_Queue.ipq_head; fp != NU_NULL; fp = fp->ipq_next)
	    {
	        /* Fragments are uniquely identified by IP id, source address, 
	           destination address, and protocol. */
	        if ( (pkt->ident == fp->ipq_id) && 
		     (*(uint32 *)pkt->ipsource == fp->ipq_source) &&
		     (*(uint32 *)pkt->ipdest == fp->ipq_dest) &&
		     (pkt->protocol == fp->ipq_protocol) )
	        {
		    break;
	        }
    	}
    
	    ip = pkt;

	    /* Adjust the IP length to not refelect the header. */
	    ip->tlen = intswap ( (uint16) (intswap(ip->tlen) - hlen));

	    /* Set ipf_mff if more fragments are expected. */
	    ((IP_FRAG *)ip)->ipf_mff &= ~1;
	    if (ip->frags & IP_MF)
	        ((IP_FRAG *)ip)->ipf_mff |= 1;

	    /* Convert the offset of this fragment to bytes and shift off the 3 flag bits. */
	    ip->frags <<= 3;

	    /* If this datagram is marked as having more fragments or this is not the 
	    first fragment, attempt reassembly. */
	    if (((IP_FRAG *)ip)->ipf_mff & 1 || ip->frags)
	    {

	        a_buf = IP_Reassembly((IP_FRAG *)ip, fp, buf_ptr);

	        if (a_buf == NU_NULL)
	        {
		    /* A complete packet could not yet be assembled, return. */
		    return (1);
	        }

	        /* If we make it here then a framented packet has been put back together. We need 
	           to set all pointers and other local variables to match what they would normally
	           be if this packet was simply a RX packet and not a reasembled one. */

	        /* Point the buffer pointer to the buffer of the reasembled packet. */
	        buf_ptr = a_buf;

	        /* Set the IP packet pointer to the IP header. */
	        pkt = (IPLAYER *) buf_ptr->data_ptr;
	    
	        /* Get the header length. */
	        hlen = (pkt->versionandhdrlen & 0x0f) << 2;

	        /* Strip off the IP header. */
	        buf_ptr->mem_total_data_len -= hlen;
	        buf_ptr->data_len           -= hlen;
	        buf_ptr->data_ptr           += hlen;

	        /* Increment the number of IP fragmented packets that have
	           successfully been reasmebled. */
	        SNMP_ipReasmOKs_Inc;
	    
	    }
	    else if (fp)
	    {
	        IP_Free_Queue_Element(fp);
	        
	        /* Drop this packet. */
	        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
	        return (1);
	    }
    }
#else

	/* If the processing of fragments is not desired, then drop this packet. */
    /* If offset and IP_MF are set then this is a fragment. */
	if (intswap(pkt->frags) & ~IP_DF)
	{
	    /* Drop the current buffer. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    return(1);
    }
#endif
    else    /* This is not a fragment. */
    {
	    /* Strip off the IP header. */
	    buf_ptr->mem_total_data_len -= hlen;
	    buf_ptr->data_len           -= hlen;
	    buf_ptr->data_ptr           += hlen;
    }

    /* Create the pseudo tcp header for upper layer protocols to 
       compute their checksum */
    tcp_chk.source  = *(uint32 *)pkt->ipsource;
    tcp_chk.dest    = *(uint32 *)pkt->ipdest;
    tcp_chk.z       = 0;
    tcp_chk.proto   = pkt->protocol;
    tcp_chk.tcplen  = (uint16)intswap((uint16)buf_ptr->mem_total_data_len);

	/* which protocol to handle this packet? */
    switch (pkt->protocol)
	{
	case IP_UDP_PROT:
	    /* Increment the number of IP packets successfully delivered. */
	    SNMP_ipInDelivers_Inc;
	     
	    return (udpinterpret(buf_ptr, &tcp_chk));

	case IP_TCP_PROT:
	    /* Increment the number of IP packets successfully delivered. */
	    SNMP_ipInDelivers_Inc;

	    /* pass tcplen on to TCP */
	    return (TCP_Interpret(buf_ptr, &tcp_chk));

	case IP_ICMP_PROT:

	    /* Increment the number of IP packets successfully delivered. */
	    SNMP_ipInDelivers_Inc;

	    return (ICMP_Interpret(buf_ptr, (uint32 *)pkt->ipsource));

#if (INCLUDE_IP_MULTICASTING)

	case IP_IGMP_PROT :

	    /* Increment the number of IP packets successfully delivered. */
	    SNMP_ipInDelivers_Inc;

	    return (IGMP_Interpret(buf_ptr, hlen));
#endif

		default:
	    NU_Tcp_Log_Error (TCP_IP_HIGH_LAYER, TCP_RECOVERABLE,
			      __FILE__, __LINE__);

	    /* Drop the packet by placing it back on the buffer_freelist. */
	    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

	    /* Increment the number of IP packets received with a invalid
	       protocol field. */
	    SNMP_ipInUnknownProtos_Inc;

	    return (1);
	}  /* end switch */
} /* IP_Interpret */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Send                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send an IP packet.                                               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ICMP_Reflect                                                     */
/*      IGMP_Send                                                        */
/*      IP_Forward                                                       */
/*      netusend                                                         */
/*      Send_SYN_FIN                                                     */
/*      tcp_retrasnmit                                                   */
/*      tcp_sendack                                                      */
/*      tcpresetfin                                                      */
/*      tcpsend                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      IP_Broadcast_Addr                                                */
/*      IP_Find_Route                                                    */
/*      IP_Fragment                                                      */
/*      ipcheck                                                          */
/*      longswap                                                         */
/*      RTAB_Free                                                        */
/*      UTL_Zero                                                         */
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
/*      Glen Johnson    03/18/98            Created initial version.     */
/*************************************************************************/
STATUS IP_Send(NET_BUFFER *buf_ptr, RTAB_ROUTE *ro, uint32 dest_ip, 
	       uint32 src_ip, int32 flags, INT ttl, INT protocol, INT tos, 
	       IP_MULTI_OPTIONS *mopt)
{
    IPLAYER         *ip_dgram;
    int16           hlen = sizeof (IPLAYER);
    DV_DEVICE_ENTRY *int_face;
    RTAB_ROUTE      iproute;
    SCK_SOCKADDR_IP *dest;
    STATUS          status;

    /* Here use a macro to resolve get a pointer to the IP header. */
    ip_dgram = IP_BUFF_TO_IP(buf_ptr);

    /* If this is a forwarded packet or if it is a raw IP packet, then don't
       mess with the IP header. */
    if (flags & (IP_FORWARDING | IP_RAWOUTPUT))
    {
	    hlen = (ip_dgram->versionandhdrlen & 0xf) << 2;
	    buf_ptr->mem_total_data_len = intswap(ip_dgram->tlen) - hlen;        
    }
    else
    {
	    /* Set the IP header length and the IP version. */
	    ip_dgram->versionandhdrlen = (hlen >> 2) | (IP_VERSION << 4);

	    /* Zero out the fragment field */
	    ip_dgram->frags = 0;

	    /* Set the IP packet ID. */
	    ip_dgram->ident = intswap ((uint16)IP_Ident++);

	    /* Set the type of service. */
	    ip_dgram->service = tos;

	    /* Set the total length (data and IP header) fo this packet. */
	    ip_dgram->tlen = intswap((int16)(buf_ptr->mem_total_data_len + hlen));

	    /* Set the time to live */
	    ip_dgram->ttl = (uint8)ttl;

	    /* Set the protocol. */
	    ip_dgram->protocol = (uint8)protocol;

	    /* Set the destination IP address. */
	    *(uint32 *)ip_dgram->ipdest = dest_ip;

	    /* Increment the number of IP packets transmitted. NOTE: this does
	       not include packets that are being forwarded, also if IP_RAWOUTPUT
	       support is added this incrementer will have to be moved. */
	    SNMP_ipOutRequests_Inc;
    }

    /* Update the length and data ptr for the head buffer. */
    buf_ptr->data_len           += hlen;
    buf_ptr->mem_total_data_len += hlen;
    buf_ptr->data_ptr           =  (uchar *)ip_dgram;

    /* If a route was not provided then point to the temporary
       route structure. */
    if (ro == NU_NULL)
    {
	    ro = &iproute;
	    UTL_Zero((CHAR *)ro, sizeof(*ro));
    }

    /* Point to the destination. */
    dest = &ro->rt_ip_dest;

    /* Check to see if there is a cached route.  If so verify that it is too the
       same destination and that it is still up. If it not free it and try
       again.
    */
    if ( ro->rt_route && ( ((ro->rt_route->rt_flags & RT_UP) == 0) ||
			    (dest->sck_addr != dest_ip)) )
    {
	    RTAB_Free(ro->rt_route);        
	    ro->rt_route = NU_NULL;
    }

    if (ro->rt_route == NU_NULL)
    {        
	    dest->sck_family = SK_FAM_IP;
	    dest->sck_len = sizeof (*dest);
	    dest->sck_addr = dest_ip;
    }

    /* NOTE:  Bypassing routing is not necessary yet, but may be supported in 
       future releases. */
#ifdef NOT_SUPPORTED
    /* Check to see if the caller specified that routing should be bypassed. */
    if (flags & IP_ROUTETOIF)
    {

    }
    else
#endif /* NOT_SUPPORTED */
    {
	if (ro->rt_route == NU_NULL)            
	    IP_Find_Route(ro);

	if (ro->rt_route == NU_NULL)
	{
	    /* Return host unreachable error.  The only resource allocated in
	       this function is a route, but we failed to find the route so it
	       is safe to return here.
	    */

	    /* Increment the number of packets that could not be delivered
	       because a route could not be found. */
	    SNMP_ipOutNoRoutes_Inc;
	    return (NU_HOST_UNREACHABLE);
	}

	int_face = ro->rt_route->rt_device;
	ro->rt_route->rt_use++;

	/* If the next hop is a gateway then set the destination ip address to
	   the gateway. */
	if (ro->rt_route->rt_flags & RT_GATEWAY)
	    dest = &ro->rt_route->rt_gateway;
    }

    /* Is this packet destined for a multicast address */
    if (IP_MULTICAST_ADDR(longswap(*(uint32 *)ip_dgram->ipdest)))
    {
#if INCLUDE_IP_MULTICASTING

	/* Mark this buffer as containing a multicast packet. */
	buf_ptr->mem_flags |= NET_MCAST;

	/* IP destination address is multicast. Make sure "dest" still points 
	   to the address in "ro". It may have been changed to point to a 
	   gateway address above
	*/
	dest = &ro->rt_ip_dest;
    
	/* Did the caller provide any multicast options. */    
	if (mopt != NU_NULL)
	{
	    /* Use the options provided by the caller. */
	    ip_dgram->ttl = (uint8)mopt->ipo_ttl;
	    if (mopt->ipo_device != NU_NULL)
		int_face = mopt->ipo_device;
	}
	else
	    ip_dgram->ttl = (uint8)IP_DEFAULT_MULTICAST_TTL;

	/* Confirm that the outgoing interface supports multicast. */
	if ((int_face->dev_flags & DV_MULTICAST) == 0)
	    return (NU_HOST_UNREACHABLE);

	/* If a source address has not been specified then use the source 
	   address of the outgoing interface. */
	if (src_ip == IP_ADDR_ANY)
	{
	    *(uint32 *)ip_dgram->ipsource = *(uint32 *)int_face->dev_addr.dev_ip_addr;
	    src_ip = *(uint32 *)int_face->dev_addr.dev_ip_addr;
	}

	/* NOTE: When multicastLoop Back and/or multicast routing are supported 
	   this is where it should be done. */
#else /* !INCLUDE_IP_MULTICASTING */
	
	/* If multicasting support was not desired then return an error. */
	return (NU_HOST_UNREACHABLE);

#endif /* INCLUDE_IP_MULTICASTING */

    }
    else
    {
	/* Has a source address been specified?  If not use the address of the
	   outgoing interface.  In the current release the source address will
	   probably always be known at this point.  */
	if (src_ip != IP_ADDR_ANY)
	    *(uint32 *)ip_dgram->ipsource = src_ip;
	else
	    *(uint32 *)ip_dgram->ipsource = *(uint32 *)int_face->dev_addr.dev_ip_addr;


	/* Check for broadcast destination address. If this is a broadcast make sure
	   the interface supports broadcasting and that the caller enabled 
	   broadcasting.
	*/
	if (IP_Broadcast_Addr(dest_ip, int_face))
	{
	    /* Does the interface support broadcasting. */
	    if ( (int_face->dev_flags & DV_BROADCAST) == 0)
		return (NU_UNRESOLVED_ADDR);
	
	    /* Did the caller enable broadcasting. */
	    if ( (flags  & IP_ALLOWBROADCAST) == 0)
		return (NU_ACCESS);

	    /* Inform the MAC layer to send this as a link-level broadcast. */
	    buf_ptr->mem_flags |= NET_BCAST;
	}
	else
	    /* Make sure the broadcast flag is clear. */
	    buf_ptr->mem_flags &= ~NET_BCAST;
    }


    /* If this packet is small enough send it.*/
    if (intswap(ip_dgram->tlen) <= int_face->dev_mtu)
    {

	    /* Compute the IP checksum. Note that the length expected by ipcheck is 
	       the length of the header in 16 bit half-words. */
	    ip_dgram->check = 0;
	    ip_dgram->check = ipcheck ((uint16 *)ip_dgram, (uint16)(hlen >> 1));

	    /* Set the packet type that is in the buffer. */
	    buf_ptr->mem_flags |= NET_IP;

	    /* Send the packet. */        
	    status = (*(int_face->dev_output)) (buf_ptr, int_face, dest, ro);
    }
    else /* This packet must be fragmented. */
    {
#if INCLUDE_IP_FRAGMENT

    	/* If the don't fragment bit is set return an error. */
	    if (ip_dgram->frags & intswap(IP_DF))
	    {
	        /* Increment the number of IP packets that could not be
	           fragmented. In this case becuase the don't fragment
	           bit is set. */
	        SNMP_ipFragFails_Inc;

	        return(NU_MSGSIZE);
	    }
		
    	status = IP_Fragment (buf_ptr, ip_dgram, int_face, dest, ro);
#else
	    return(NU_MSGSIZE);
#endif
    }

    if ( (ro == &iproute) && ((flags & IP_ROUTETOIF) == 0) && ro->rt_route )
        RTAB_Free(ro->rt_route);

    return (status);

} /* IP_Send */

#if INCLUDE_IP_FRAGMENT

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Fragment                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Fragment an IP packet.                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Send                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      MEM_Buffer_Chain_Dequeue                                         */
/*      IP_Option_Copy                                                   */
/*      MEM_Chain_Copy                                                   */
/*      ipcheck                                                          */
/*      MEM_Trim                                                         */
/*      MEM_One_Buffer_Chain_Free                                        */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Patti Hill      03/30/98        Fixed initialization, buffering, */
/*                                      fragment linking, and fragments  */
/*                                      of buffer chains                 */
/*      Patti Hill      04/15/98        More changes after Reassembly -> */
/*                                      Fragmentation testing            */
/*                                                                       */
/*************************************************************************/

STATUS IP_Fragment (NET_BUFFER *buf_ptr, IPLAYER *ip, DV_DEVICE_ENTRY *int_face, 
	     SCK_SOCKADDR_IP *dest, RTAB_ROUTE *ro)
{
    INT         len, hlen, tlen, f_hlen, first_len, data_len, total_data_len;
    INT         off;
	NET_BUFFER  *work_buf;
    NET_BUFFER  *f_buf = buf_ptr;
    NET_BUFFER  **next = &buf_ptr->next;
    STATUS      err = NU_SUCCESS;
    IPLAYER     *f_ip;
	INT         aloc_len = 0;

	hlen = (ip->versionandhdrlen & 0x0f) << 2;

    /* len is the number of data bytes in each fragment. Computed as the mtu of 
       the interface less the size of the header and rounded down to an 8-byte 
       boundary by clearing the low-order 3 bits (& ~7).
    */
    len = (int_face->dev_mtu - hlen) & ~7;
    first_len = len;

    /* Each fragment must be able to hold at least 8 bytes. */
    if (len < 8)
	return NU_MSGSIZE;

    tlen = intswap(ip->tlen);

	buf_ptr = f_buf;

    f_hlen = sizeof(IPLAYER);

	aloc_len = len;

    /* Create the fragments. */
    for (off = hlen + len; off < tlen; off += len)
    {
		if (off + len >= intswap(ip->tlen))         
			/* Shorten the length if this is the last fragment. */                      
			aloc_len = intswap(ip->tlen) - off;
		
		/* Allocate a buffer chain to build the fragment in. */
		f_buf = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, 
					 aloc_len + hlen);
	
		if (f_buf == NU_NULL)
	
		{
			err = NU_NOBUFS;
			
			/* Increment the number of IP packets that could not be
			fragmented. In this case becuase of no buffers. */          
			SNMP_ipFragFails_Inc;
			break;  
		}

		/* Initialize mem_dlist for deallocation */
		f_buf->mem_dlist = &MEM_Buffer_Freelist;

		/* Point to the location where the IP header will begin. */     
		f_buf->data_ptr = f_buf->mem_parent_packet + int_face->dev_hdrlen;
	
		/* Overlay the IP header so we can access the individual fields. */
		f_ip = (IPLAYER *)f_buf->data_ptr;

		*f_ip = *ip;

		/* If there are options in the original, packet copy them. */
		if (hlen > sizeof(IPLAYER))
	
		{           
			f_hlen = IP_Option_Copy(ip, f_ip) + sizeof(IPLAYER);        
			f_ip->versionandhdrlen = (f_hlen >> 2) | (IP_VERSION << 4);     
		}

	
		/* The IP header is the only data present. */   
		f_buf->data_len = f_hlen;

		/* Set the offset field for the fragment. */    
		f_ip->frags = ((off - hlen) >> 3) + (ip->frags & ~IP_MF);

	
		/* If MF is set in the original packet then it should be set in all 
		fragments. */
	
		if (ip->frags & IP_MF)      
			f_ip->frags |= IP_MF;
		
		/* Is this the last fragment. MF is set for every fragment except the      
		last one. Unless MF was set in the original packet. In that case MF 
	    should have already been set above. */
	
		if (off + len >= intswap(ip->tlen))         
			/* Shorten the length if this is the last fragment. */                      
			len = intswap(ip->tlen) - off;  
		else        
			/* This is not the last fragment. Set MF. */        
			f_ip->frags |= IP_MF;
	
		/* Set the new length. */
		f_ip->tlen = intswap((int16)(len + f_hlen));

		/* Copy data from the original packet into this fragment. */    
		MEM_Chain_Copy(f_buf, buf_ptr, off, len);
	
		/* Set the length of all data in this buffer chain. */  
		f_buf->mem_total_data_len = len + f_hlen;
	
		/* Clear the device pointer. */ 
		f_buf->mem_buf_device = NU_NULL;
	
		/* Swap the fragment field. */  
		f_ip->frags = intswap(f_ip->frags);
	
		/* Compute the IP header checksum. */   
		f_ip->check = 0;        
		f_ip->check = ipcheck((uint16 *)f_ip, (uint16)(f_hlen >> 1));
	
		/* Link this fragment into the list of fragments. */    
		*next = f_buf;
		next = &f_buf->next;

		/* Increment the number of fragments that have been created. */ 
		SNMP_ipFragCreates_Inc;    
	}

#if (SNMP_INCLUDED != 0)
    if (err == NU_SUCCESS)
	/* Increment the number of packets that have been fragmented. */
	SNMP_ipFragOKs_Inc;
#endif

	/* Convert the original packet into the first fragment */
	f_buf = buf_ptr;

	/* Update the first fragment by trimming what's been copied out. */
	MEM_Trim(f_buf, hlen + first_len - tlen);

	/* Determine the total data length of the first fragment */
	total_data_len = f_buf->mem_total_data_len;
	data_len = 0;

	/* Terminate the first fragment */
	for(work_buf = f_buf; data_len < total_data_len; work_buf = work_buf->next_buffer)
	{
		data_len = work_buf->data_len + data_len;
		if(data_len == total_data_len)
		{
			/* Deallocate the buffers that the first fragment does not need */
			MEM_One_Buffer_Chain_Free (work_buf->next_buffer, &MEM_Buffer_Freelist);
			work_buf->next_buffer = NU_NULL;
		}
	}

    /* Update the header in the first fragment (the original packet). */
    ip->tlen = intswap((int16)f_buf->mem_total_data_len);
    ip->frags = intswap((int16)(ip->frags | IP_MF));
    ip->check = 0;
    ip->check = ipcheck((uint16 *)ip, (uint16)(f_hlen >> 1));

    /* Send each fragment */        
	for (f_buf = buf_ptr; f_buf; f_buf = buf_ptr)
	{       
		buf_ptr = f_buf->next;  
		f_buf->next = NU_NULL;
	
		/* If the first packet can not be transmitted successfully, then abort and         
		free the rest. */
		if(err == NU_SUCCESS)
		{
            /* Set the packet type that is in the buffer. */
            f_buf->mem_flags |= NET_IP;

			if ((err = (*(int_face->dev_output)) (f_buf, int_face, dest, ro)) != NU_SUCCESS )
			{
				if (err != NU_UNRESOLVED_ADDR)
					MEM_One_Buffer_Chain_Free (f_buf, &MEM_Buffer_Freelist);
				else
					err = NU_SUCCESS;
			}
		}       
		else
			MEM_One_Buffer_Chain_Free (f_buf, &MEM_Buffer_Freelist);
	}

    return (NU_SUCCESS);

} /* IP_Fragment */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Option_Copy                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Fragment                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      dip         destination IP packet header                         */
/*      sip         source IP packet header                              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The length of the option data copied.                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/
INT IP_Option_Copy (IPLAYER *dip, IPLAYER *sip)
{
    uchar       *src, *dest;
    INT         cnt, opt, optlen;

    /* Point to the first byte of option data in each of the IP packets. */
    src = (uchar *) (sip + 1);
    dest = (uchar *) (dip + 1);

    cnt = (sip->versionandhdrlen << 2) - sizeof(IPLAYER);

    for (; cnt > 0; cnt -= optlen, src += optlen)
    {
	    opt = src[0];

	    /* Stop when the EOL option is encountered. */
	    if (opt == IP_OPT_EOL)
	        break;
	    
	    /* Copy NOPs to preserve alignment constraints. */
	    if (opt == IP_OPT_NOP)
	    {
	        *dest++ = IP_OPT_NOP;
	        optlen = 1;
	        continue;
	    }
	    else
	        optlen = src[IP_OPT_OLEN];

	    /* Truncate an option length that is too large. This should not occur. */
	    if (optlen > cnt)
	        optlen = cnt;

	    /* If the copied bit is set then copy the option. */
	    if (IP_OPT_COPIED(opt))
	    {
	        memcpy(dest, src, optlen);
	        dest += optlen;
	    }
    }

    /* Pad the option list, if necessary, out to a 4-byte boundary. */
    for (optlen = dest - (uchar *)(dip + 1); optlen & 0x3; optlen++)
	*dest++ = IP_OPT_EOL;

    return (optlen);

} /* IP_Option_Copy */

#endif /* INCLUDE_IP_FRAGMENT */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Broadcast_Addr                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks an IP address to see if it is a broadcast   */
/*      address.                                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Send                                                          */
/*      udpinterpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      dest        The IP address to be checked.                        */
/*      int_face    Pointer to the interface the IP address is being     */
/*                     used on.                                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      1          This is a broadcast address.                          */
/*      0          This is not a broadcast address.                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
INT IP_Broadcast_Addr(uint32 dest, DV_DEVICE_ENTRY *int_face)
{
    if ( (dest == IP_ADDR_ANY) || (dest == IP_ADDR_BROADCAST) )
	return 1;

    if ( (int_face->dev_flags & DV_BROADCAST) == 0)
    	return 0;

    if ( (int_face->dev_addr.dev_net == dest) ||
	 (int_face->dev_addr.dev_net_brdcast == dest) )
	    return 1;

    return 0;
} /* IP_Broadcast_Addr */

/* Check to see if the route is still valid. */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Find_Route                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks to see if a route is still valid. If the    */
/*      route is not valid or if a route has never been allocated. A new */
/*      route is found.                                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Forward                                                       */
/*      IP_Send                                                          */
/*      netlisten                                                        */
/*      netxopen                                                         */
/*      UDP_Cache_Route                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      RTAB_Find_Route                                                  */
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
/*      Glen Johnson    03/18/98       Created initial version.          */
/*************************************************************************/
VOID IP_Find_Route(RTAB_ROUTE *ro)
{
    if ( ro->rt_route && ro->rt_route->rt_device &&
	 (ro->rt_route->rt_flags & RT_UP) )
        return;

    ro->rt_route = RTAB_Find_Route(&ro->rt_ip_dest);
} /* IP_Find_Route */

#if INCLUDE_IP_FORWARDING

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Forward                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks attempts to forward an IP packet out of     */
/*      one of the network interfaces.                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Interpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ICMP_Send_Error                                                  */
/*      IP_Canforward                                                    */
/*      IP_Find_Route                                                    */
/*      IP_Send                                                          */
/*      MEM_Buffer_Enqueue                                               */
/*      RTAB_Free                                                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      buf_ptr     A buffer containing the datagram to forward.         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created intial version.          */
/*************************************************************************/
STATUS IP_Forward(NET_BUFFER *buf_ptr)
{
    SCK_SOCKADDR_IP     *sin;
    ROUTE_NODE          *rt;
    DEV_IF_ADDRESS      *d_addr;
    uint32              src, dest;
    INT                 type, code;
    STATUS              stat;
    INT                 deallocate_flag;
    IPLAYER             *ip_pkt;
    INT                 hlen;


    /* Initialize local variables. */
    type = 0;
    deallocate_flag = 0;
    dest = 0;
    ip_pkt = (IPLAYER *)buf_ptr->data_ptr;

    if ( (buf_ptr->mem_flags & NET_BCAST) ||
	 IP_Canforward(*(uint32 *)ip_pkt->ipdest) != NU_SUCCESS)
    {
	/* Deallocate the buffer. */
    MEM_One_Buffer_Chain_Free (buf_ptr,buf_ptr->mem_dlist);

	/* Increment the number of IP packets received with the wrong IP addr.*/
	SNMP_ipInAddrErrors_Inc;

	return (NU_INVALID_ADDRESS);
    }

    /* Check the time to live field. */
    if (ip_pkt->ttl <=  1)
    {
	ICMP_Send_Error(buf_ptr, ICMP_TIMXCEED, ICMP_TIMXCEED_TTL, 0,
			buf_ptr->mem_buf_device);

	/* Deallocate the buffer. */
    MEM_One_Buffer_Chain_Free (buf_ptr,buf_ptr->mem_dlist);

	return -1;
    }

    /* Decrement the time to live. */
    ip_pkt->ttl--;

    /* Increment the number of packets that we attempted to find a route
       and forward. */
    SNMP_ipForwDatagrams_Inc;

    sin = (SCK_SOCKADDR_IP *) &IP_Forward_Rt.rt_ip_dest;

    /* Check to see if the cached route is still valid. */
    if ( ((rt = IP_Forward_Rt.rt_route) == 0) ||
	 (*(uint32 *)ip_pkt->ipdest != sin->sck_addr) )
    {
	/* We can not used the cached route.  If there is one then free it. */
	if (IP_Forward_Rt.rt_route)
	{
	    RTAB_Free(IP_Forward_Rt.rt_route);
	    IP_Forward_Rt.rt_route = NU_NULL;
	}

	sin->sck_family = SK_FAM_IP;
	sin->sck_len = sizeof (*sin);
	sin->sck_addr = *(uint32 *)ip_pkt->ipdest;

	IP_Find_Route(&IP_Forward_Rt);

	/* Was a route found. */
	if (IP_Forward_Rt.rt_route == 0)
	{
	    /* Send ICMP host unreachable message. */
	    ICMP_Send_Error(buf_ptr, ICMP_UNREACH, ICMP_UNREACH_HOST, 0,
			    buf_ptr->mem_buf_device);

	    /* Deallocate the buffer. */
        MEM_One_Buffer_Chain_Free (buf_ptr,buf_ptr->mem_dlist);

	    /* Increment the number of packets that could not be delivered
	       because a route could not be found. */
	    SNMP_ipOutNoRoutes_Inc;

	    return (NU_HOST_UNREACHABLE);
	}

	rt = IP_Forward_Rt.rt_route;
    }

    if ( (rt->rt_device == buf_ptr->mem_buf_device) &&
	 ((rt->rt_flags & (RT_DYNAMIC | RT_MODIFIED)) == 00) &&
	 (*(uint32 *)rt->rt_rip2->ip_addr != 0) && IP_Sendredirects )
    {
	src = *(uint32 *)ip_pkt->ipsource;
	d_addr = &rt->rt_device->dev_addr;

	if ( (src & d_addr->dev_netmask) == d_addr->dev_net)
	{
	    if (rt->rt_flags & RT_GATEWAY)
		dest = *(uint32 *)rt->rt_gateway.sck_addr;
	    else
		dest = *(uint32 *)ip_pkt->ipdest;

	    type = ICMP_REDIRECT;
	    code = ICMP_REDIRECT_HOST;
	    /* Send ICMP host unreachable message. */
	    ICMP_Send_Error(buf_ptr, type, code, dest, buf_ptr->mem_buf_device);

	}
    }

    /* IP send expects the data pointer to point at the IP data not at the IP
       header. So move the pointer forward. */
    hlen = (ip_pkt->versionandhdrlen & 0x0f) << 2;
    buf_ptr->data_ptr += hlen;
    buf_ptr->data_len -= hlen;

    /* Forward the packet.  Because this is a forward many of the parameters are
       not required.  Specifically the length, the ttl, the protocol, and tos.
       NU_NULL is used for all of those parameters that will not be needed.
    */
    stat = IP_Send( buf_ptr, &IP_Forward_Rt, *(uint32 *)ip_pkt->ipdest, 
		    *(uint32 *)ip_pkt->ipsource, 
		    IP_FORWARDING | IP_ALLOWBROADCAST, NU_NULL,
		    NU_NULL, NU_NULL, NU_NULL);

    if ((stat != NU_SUCCESS) && (stat != NU_UNRESOLVED_ADDR))
    {
	/* Indicate that the transmit failed so we must deallocate the buffer.
	   We want to postpone deallocating the buffer until after the ICMP
	   message, if required, is sent. */
	deallocate_flag = 1;

	if (stat == NU_HOST_UNREACHABLE)
	{
	    type = ICMP_UNREACH;
	    code = ICMP_UNREACH_HOST;
	    /* Send ICMP host unreachable message. */
	    ICMP_Send_Error(buf_ptr, type, code, dest, buf_ptr->mem_buf_device);

	}

	if (deallocate_flag)
	    /* The packet was not sent.  Dealocate the buffer.  If the packet was
	       transmitted when the transmit complete interrupt occurs. */
        MEM_One_Buffer_Chain_Free ( buf_ptr,buf_ptr->mem_dlist);

	 /* Increment the number of IP packets received with the wrong IP addr.*/
	SNMP_ipInAddrErrors_Inc;

    }

    return (stat);

} /* IP_Forward */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Canforward                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function decides if an attempt should be made to forward    */
/*      an IP datagram.                                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ICMP_Send_Error                                                  */
/*      IP_Forward                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      longswap                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      dest        The destination IP address.                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      -1          Don't attempt forward the packet.                    */
/*      NU_SUCCESS  Forward the packet.                                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98            Created initial version.     */
/*************************************************************************/
STATUS IP_Canforward(uint32 dest)
{
    uint32 loopback_addr = 0x7f000001;  /* The loop back IP address */
    uint32 class_d       = 0xe0000000;  /* First class D address. */

    dest = longswap(dest);

    /* We want to reject the loop_back address, the network 0 address, and class
       D and class E addresses. */
    if ( (dest == loopback_addr)    ||
	 (dest == 0)                ||
	 (dest >= class_d) )
    {
    	return -1;
    }
    else
	    return NU_SUCCESS;

} /* IP_Canforward */

#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Get_Net_Mask                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the mask for a given class of IP addresses.*/
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      <Caller>                            <Description>                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      longswap                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ip_addr     An IP address for which a mask is desired.           */
/*      mask        Location where the mask will be copied.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      -1          The IP address is not a class A, B, or C address     */
/*                    a mask could not be returned.                      */
/*      NU_SUCCESS  The mask parameter was updated.                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98            Created initial version.     */
/*************************************************************************/
STATUS IP_Get_Net_Mask(CHAR *ip_addr, CHAR *mask)
{
    uint32          addr = longswap(*(uint32 *)ip_addr);

    if (IP_CLASSA_ADDR(addr))
    {
        memcpy(mask, IP_A_Mask, IP_ADDR_LEN);
	    return NU_SUCCESS;
    }
    else if (IP_CLASSB_ADDR(addr))
    {
	    memcpy(mask, IP_B_Mask, IP_ADDR_LEN);
	    return NU_SUCCESS;
    }
    else if (IP_CLASSC_ADDR(addr))
    {
    	memcpy(mask, IP_C_Mask, IP_ADDR_LEN);
	    return NU_SUCCESS;
    }
    else
        return -1;

} /* IP_Get_Net_Mask */

#if INCLUDE_IP_REASSEMBLY

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Free_Queue_Element                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Free a fragment reassembly header and any attached fragments.    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Interpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_remove                                                       */
/*      NU_Deallocate_Memory                                             */
/*      MEM_One_Buffer_Chain_Free                                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      fp          pointer to the element to free                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/
VOID IP_Free_Queue_Element(IP_QUEUE_ELEMENT *fp)
{
    IP_FRAG     *q;
    NET_BUFFER  *buf_ptr;

    /* Remove this fragment reassembly header from the list. */
    dll_remove(&IP_Frag_Queue, fp);

    for (q = fp->ipq_first_frag; q != NU_NULL; q = q->ipf_next)
    {
	    /* Get a pointer to the buffer that contains this packet. */
	    buf_ptr = (NET_BUFFER *)((char *)q - q->ipf_buf_offset);
	    
	    /* Deallocate the buffer. */
	    MEM_One_Buffer_Chain_Free (buf_ptr, buf_ptr->mem_dlist);
    }

    /* Deallocate the memory. */
    NU_Deallocate_Memory (fp);

} /* IP_Free_Queue_Element */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Reassembly                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Reassemble IP fragments into a complete datagram.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Interpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_enqueue                                                      */
/*      dll_remove                                                       */
/*      NU_Allocate_Memory                                               */
/*      NU_Deallocate_Memory                                             */
/*      IP_Insert_Frag                                                   */
/*      IP_Remove_Frag                                                   */
/*      MEM_Buffer_Chain_Free                                            */
/*      MEM_Buffer_Enqueue                                               */
/*      MEM_Cat                                                          */
/*      MEM_Trim                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*      UTL_Timerset                                                     */
/*      UTL_Timerunset                                                   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ip          pointer to an IP fragment                            */
/*      fp          fragment reassembly header                           */
/*      buf_ptr     pointer to the buffer that holds the current frag    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_NULL     All fragments have not yet been received.            */
/*      buf_ptr     A complete datagram has been built, buf_ptr is the   */
/*                    head of the chain that contains the complete       */
/*                    datagram.                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/
NET_BUFFER *IP_Reassembly(IP_FRAG *ip_pkt, IP_QUEUE_ELEMENT *fp, 
			  NET_BUFFER *buf_ptr)
{
    INT         hlen = ((ip_pkt->ipf_hl  & 0x0F) << 2);
    IP_FRAG     *q, *p;
    INT         i, total_length;
    INT         next;
    NET_BUFFER  *r_buf, *m;

    /* Since reassembly involves only the data of each fragment exclude the IP 
       header from each fragment. */
    buf_ptr->data_ptr           += hlen;
    buf_ptr->data_len           -= hlen;
    buf_ptr->mem_total_data_len -= hlen;

    /* If this is the first fragment to arrive, create a reassembly queue. */
    if (fp == NU_NULL)
    {
	    if (NU_Allocate_Memory( &System_Memory, (VOID **)&fp, 
				    sizeof(IP_QUEUE_ELEMENT), 
				    (UNSIGNED)NU_NO_SUSPEND ) != NU_SUCCESS)
	    {
	        /* Log an error and drop the packet. */
	        NU_Tcp_Log_Error (TCP_NO_MEMORY, TCP_SEVERE, __FILE__, __LINE__);
	        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
	        return (NU_NULL);
	    }

	    /* Insert this fragment reassembly header into the list. */
	    dll_enqueue(&IP_Frag_Queue, fp);

	    /* Save off the protocol. */
	    fp->ipq_protocol = ((IPLAYER *)ip_pkt)->protocol;
	    fp->ipq_id = ip_pkt->ipf_id;
	    
	    /* The source and destination ip address fields are used to link the 
	       fragments together. The next two lines copy the source and 
	       destination address before they are overwritten with pointer 
	       values. 
	    */
	    fp->ipq_source = (uint32)ip_pkt->ipf_next;
	    fp->ipq_dest = (uint32)ip_pkt->ipf_prev;

	    /* Insert this fragment into the list. */
	    fp->ipq_first_frag = ip_pkt;
	    ip_pkt->ipf_next = NU_NULL;
	    ip_pkt->ipf_prev = NU_NULL;

	     /* The protocol was saved off above. For a fragment this field is 
	       temporarily used to store the offset of the IP header from the start 
	       of the memory buffer. */

	    ip_pkt->ipf_buf_offset = (char HUGE *)ip_pkt - (char HUGE *)buf_ptr;

	    /* Set up a timer event to drop this fragment and any others received 
	       that are part of the same datagram, if the complete datagram is not 
	       received. */
	    UTL_Timerset(EV_IP_REASSEMBLY, (UNSIGNED)fp, IP_FRAG_TTL, (int32)0);
    }
    else
    {
	    /* The protocol was saved off in the first fragment RX. This field is 
	       used to store the offset of the IP header from the start of the memory buffer. */
	    ip_pkt->ipf_buf_offset = (char HUGE *)ip_pkt - (char HUGE *)buf_ptr;
       
	    /* Find a fragment which begins after this one. */
	    for (q = fp->ipq_first_frag; q != NU_NULL; q = q->ipf_next)
	    {
	        if (q->ipf_off > ip_pkt->ipf_off)
		    break;

	        /* Keep a pointer to the fragment before q is set to next. */
	        p = q;
	    }

	    /* If q is equal to NULL then the packet just RX does not belong before any of 
	       the ones already in queue. Since it will be appended to the end of the queue 
	       we need to check for overlapping of the last fragment in the queue. */
	    
	    if (q == NU_NULL)
	    {
	        /* q needs to be setup correctly so that we can perform that overlapping
	           check. First we will point q to the packet just RX. */
	        q = ip_pkt;

	        /* Set the previous pointer to point at the previous fragment. This comes from
	           the loop above. */
	        q->ipf_prev = p;

	        /* If there is a preceding fragment, it may provide some of our data 
	           already.  If so, drop the data from the incoming fragment.  If it 
	           provides all of the data, drop the current fragment. 
	        */
	        
	        i = (q->ipf_prev->ipf_off + intswap (q->ipf_prev->ipf_tlen)) - ip_pkt->ipf_off;
	        if (i > 0)
	        {
		        if ( i >= intswap (ip_pkt->ipf_tlen))
		        {
		            /* All of the received data is contained in the previous 
		               fragment. Drop this one. */
		            MEM_Buffer_Chain_Free ( &MEM_Buffer_List, 
						        &MEM_Buffer_Freelist );
		            return(NU_NULL);
		        }
		    
		        /* Trim the duplicate data from this fragment. */
		        MEM_Trim(buf_ptr, i);
		        ip_pkt->ipf_off += i;
		        ip_pkt->ipf_tlen = intswap ((uint16) (intswap (ip_pkt->ipf_tlen) - i));
	        }
	        
	        /* Set it back to NULL so that the overlapping code below is not executed. */
	        q = NU_NULL;

	    }

	    /* If there is a preceding fragment, it may provide some of our data 
	       already.  If so, drop the data from the incoming fragment.  If it 
	       provides all of the data, drop the current fragment. 
	    */
	    if ((q != NU_NULL) && (q->ipf_prev != NU_NULL))
	    {
	        i = (q->ipf_prev->ipf_off + intswap (q->ipf_prev->ipf_tlen)) - ip_pkt->ipf_off;
	        if (i > 0)
	        {
		        if ( i >= intswap (ip_pkt->ipf_tlen))
		        {
		            /* All of the received data is contained in the previous 
		               fragment. Drop this one. */
		            MEM_Buffer_Chain_Free ( &MEM_Buffer_List, 
					            &MEM_Buffer_Freelist );
		            return(NU_NULL);
		        }
		    
		        /* Trim the duplicate data from this fragment. */
		        MEM_Trim(buf_ptr, i);
		        ip_pkt->ipf_off += i;
		        ip_pkt->ipf_tlen = intswap ((uint16) (intswap (ip_pkt->ipf_tlen) - i));
	        }
	    }


	    /* While we overlap succeding fragments trim them or, if they are
	       completely covered, dequeue them. */
	    while ( (q != NU_NULL) && ((ip_pkt->ipf_off + intswap(ip_pkt->ipf_tlen)) > q->ipf_off))
	    {
	        i = (ip_pkt->ipf_off + intswap(ip_pkt->ipf_tlen)) - q->ipf_off;
	        if (i < intswap (q->ipf_tlen))
	        {
		        /* Trim the duplicate data from the start of the previous 
		           fragment. In this case q. */

		        r_buf = (NET_BUFFER *)(((char HUGE *)q) - (char HUGE *)q->ipf_buf_offset);
		        MEM_Trim(r_buf, i);
		        q->ipf_tlen = intswap ( (uint16) (intswap (q->ipf_tlen) - i));
		        q->ipf_off += i;
		        break;
	        }

	        /* The buffer was completely covered. Deallocate it. */
	        r_buf = (NET_BUFFER *)(((char HUGE *)q) - q->ipf_buf_offset);
	        MEM_One_Buffer_Chain_Free (r_buf, &MEM_Buffer_Freelist);
	        IP_Remove_Frag(q, fp);

	        /* Move on to the next fagment so we can check to see if any/all of it is
	           overlapping. */
	        q = q->ipf_next;

	    }
	    

	    /* Insert the new fragment in its place. The new fragment should be 
	       inserted into the list of fragments immediately in front of q. */
	    IP_Insert_Frag(ip_pkt, fp);

    }

    /* Remove this buffer from the buffer list. */
    MEM_Buffer_Dequeue (&MEM_Buffer_List);

    /* Check for complete reassembly. */
    next = 0;
    for ( q = fp->ipq_first_frag; q != NU_NULL; q = q->ipf_next)
    {
	    /* Does the fragment conatain the next offset. */
	    if (q->ipf_off != next)
	        return (NU_NULL);
	    
	    /* Update next to the offset of the next fragment. */
	    next += intswap (q->ipf_tlen);

	    /* Keep track of the last fragment checked. It is used below. */
	    p = q;
    }

    /* If the more fragments flag is set in the last fragment, then we 
       are expecting more. */
    if (p->ipf_mff & 1)
	return (NU_NULL);

    /* Clear the Fragment timeout event for this datagram. */
    UTL_Timerunset(EV_IP_REASSEMBLY, (UNSIGNED)fp, (int32)1);

    /* Reassembly is complete. Concatenate the fragments. */

    /* Get the first frag. */
    q = fp->ipq_first_frag;

    /* Point the return buffer to the memory buffer of the first frag. */
    r_buf = (NET_BUFFER *)(((char HUGE *)q) - q->ipf_buf_offset);

    /* Point to the next fragment in the list. This is done by getting the next frag
       and then backing up the pointer so that it points to the memory buffer for that
       IP fragment. */
    q = q->ipf_next;

    /* Loop through the rest concatenating them all together. */
    while (q != NU_NULL)
    {
	    /* Back the pointer up so that we are pointing at the buffer header. */  
	    m = (NET_BUFFER *)(((char HUGE *)q) - q->ipf_buf_offset);  
       
	    /* Concatenate the two fragments. */
	    MEM_Cat(r_buf, m);
    
	    /* Move to the next fragment in peperation for the next loop. */
	    q = q->ipf_next;
    }

    /* Create the header for the new IP packet by modifying the header of the
       first packet. */

    /* Get a pointer to the IP header. */
    ip_pkt = fp->ipq_first_frag;

    /* Put the protocol type back. */
    ((IPLAYER *)ip_pkt)->protocol = fp->ipq_protocol;

    /* Set the total length. */
    ip_pkt->ipf_tlen = intswap ((uint16)next);

    /* Remove the LSB that was used to denote packets that are expecting more frags. */
    ip_pkt->ipf_mff &= ~1;

    /* Get the source and destination IP addresses. */
    memcpy ( ((IPLAYER *) ip_pkt)->ipsource, &fp->ipq_source, 4);
    memcpy ( ((IPLAYER *) ip_pkt)->ipdest, &fp->ipq_dest, 4);
    
    /* Remove this fragment reassembly header from the list. */
    dll_remove(&IP_Frag_Queue, fp);
    
    /* Deallocate this fragment reassembly header. */
    NU_Deallocate_Memory (fp);

    /* Make the header visible. */
    r_buf->data_len += ((ip_pkt->ipf_hl  & 0x0F) << 2);
    r_buf->data_ptr -= ((ip_pkt->ipf_hl  & 0x0F) << 2);

    /* Compute and set the total length of the reassembled packet. */
    for (m = r_buf, total_length = 0; m; m = m->next_buffer)
	total_length += m->data_len;

    /* Store the total length in the buffer header. */
    r_buf->mem_total_data_len = total_length;

    return (r_buf);

} /* IP_Reassembly */




/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Reassembly_Event                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Reassembled the packets once a particular event occurs.          */
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
/*      dll_remove                                                       */
/*      MEM_Cat                                                          */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      NU_Deallocate_Memory                                             */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

VOID IP_Reassembly_Event(IP_QUEUE_ELEMENT *fp)
{
    NET_BUFFER      *buf_head, *buf;
    IP_FRAG         *ip_frag;

    /* Increment the number of IP fragmented packets that could not be
       reasembled. */
    SNMP_ipReasmFails_Inc;

    /* Remove this fragment reassembly header from the list of fragmented 
       datagrams that are being rebuilt. */
    dll_remove(&IP_Frag_Queue, fp);

    /* Get the first frag. */
    ip_frag = fp->ipq_first_frag;

    /* Point the head buffer to the memory buffer of the first frag. */
    buf_head = (NET_BUFFER *)(((char *)ip_frag) - ip_frag->ipf_buf_offset);

    /* Point to the next fragment in the list. This is done by getting the next frag
       and then backing up the pointer so that it points to the memory buffer for that
       IP fragment. */
    ip_frag = ip_frag->ipf_next;

    /* Loop through the rest concatenating them all together. */
    while (ip_frag != NU_NULL)
    {
	    /* Back the pointer up so that we are pointing at the buffer header. */        
	    buf = (NET_BUFFER *)(((char *)ip_frag) - ip_frag->ipf_buf_offset);
	    
	    /* Concatenate the two fragments. */
	    MEM_Cat(buf_head, buf);
    
	    /* Move to the next fragment in peperation for the next loop. */
	    ip_frag = ip_frag->ipf_next;
    }

    /* Now that they have all been put together release all the buffers back to the
       buffer freelist. */
    MEM_One_Buffer_Chain_Free (buf_head, &MEM_Buffer_Freelist);

    /* Deallocate this fragment reassembly header. */
    NU_Deallocate_Memory (fp);

}/* IP_Reassembly_Event */




/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Remove_Frag                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Removes a fragment from the list.                                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Reassembly                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

VOID IP_Remove_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *fp)
{
    /* There are three cases that must be handled. 1) The fragment is the first 
       one in the list. 2) The fragment is the last on in the list. 3) The 
       fragment is in the middle of the list. */
    if (ip == fp->ipq_first_frag)
    {
	    fp->ipq_first_frag = ip->ipf_next;

	    /* If there was another fragment on the list then set the previous 
	       pointer to NULL. */
	    if (fp->ipq_first_frag)
	        fp->ipq_first_frag->ipf_prev = NU_NULL;
    }
    /* Is the last fragment in the list. */
    else if (ip->ipf_next == NU_NULL)
    {
    	ip->ipf_prev->ipf_next = NU_NULL;
    }
    /* This fragment must be in the middle of the list. */
    else
    {
	    ip->ipf_prev->ipf_next = ip->ipf_next;
	    ip->ipf_next->ipf_prev = ip->ipf_prev;
    }

} /* IP_Remove_Frag */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Insert_Frag                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Inserts a fragment into the fragment linked list.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Reassembly                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

VOID IP_Insert_Frag(IP_FRAG *ip, IP_QUEUE_ELEMENT *fp)
{
    IP_FRAG     *q;
    IP_FRAG     *p;

    /* Find a fragment which begins after this one. */
    for (q = fp->ipq_first_frag; q != NU_NULL; q = q->ipf_next)
    {
    	p = q;
	    if (q->ipf_off > ip->ipf_off)
		    break;
    }

    /* There are three possiblilities, each of which is handled a little 
       differently. 1) The new fragment is the last in the list. 2) The new 
       fragment is the first in the list. 3) the new fragment is somewhere in 
       the middle. */

    /* New fragment is the last one. */
    if (q == NU_NULL) 
    {
	    p->ipf_next = ip;
	    ip->ipf_prev = p;
	    ip->ipf_next = NU_NULL;
    }
    /* New fragment is the first one. */
    else if (fp->ipq_first_frag == q)
    {
	    fp->ipq_first_frag = ip;
	    ip->ipf_prev = NU_NULL;
	    ip->ipf_next = q;
	    q->ipf_prev = ip;
    }
    /* New fragment is in the middle. */
    else
    {
	    q->ipf_prev->ipf_next = ip;
	    ip->ipf_prev = q->ipf_prev;
	    ip->ipf_next = q;
	    q->ipf_prev = ip;
    }

} /* IP_Insert_Frag */

#endif /* INCLUDE_IP_REASSEMBLY */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Get_Opt                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Gets a multicast option if IP multicasting is defined,           */
/*      otherwise returns NU_INVALID_OPTION.                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Getsockopt                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Get_Multi_Opt                                                 */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

STATUS IP_Get_Opt (int16 socketd, INT optname, void *optval, INT *optlen)
{
    STATUS      status = NU_SUCCESS;

    switch (optname)
    {

#if INCLUDE_IP_MULTICASTING

    case IP_MULTICAST_TTL :
	
    	status = IP_Get_Multi_Opt(socketd, optname, optval, optlen);
	break;

#endif /* INCLUDE_IP_MULTICASTING */

    default :
	    status = NU_INVALID_OPTION;
    }

    return (status);


}/* IP_Get_Opt */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Set_Opt                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Sets the option to NU_INVALID_OPTION unless IP multicasting is   */
/*      defined.                                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Setsockopt                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Set_Multi_Opt                                                 */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

STATUS IP_Set_Opt (int16 socketd, INT optname, void *optval, INT optlen)
{
    STATUS      status = NU_SUCCESS;

    switch (optname)
    {

#if INCLUDE_IP_MULTICASTING

    case IP_ADD_MEMBERSHIP :
    case IP_DROP_MEMBERSHIP     :
	
    	status = IP_Set_Multi_Opt(socketd, optname, optval, optlen);
	break;

#endif /* INCLUDE_IP_MULTICASTING */

    default :
	    status = NU_INVALID_OPTION;
    }

    return (status);

} /* IP_Setopt */

#if INCLUDE_IP_MULTICASTING

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Get_Multi_Opt                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Gets a multicast operation if IP multicasting is defined. At this*/
/*      time the only optiojn is the IP_MULTICAST_TTL.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Get_Opt                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

STATUS IP_Get_Multi_Opt(int16 socketd, INT optname, void *optval, INT *optlen)
{
    SOCKET              *sck = socket_list[socketd];
    STATUS              status;

    switch (optname)
    {
    case IP_MULTICAST_TTL :

	    *optlen = 1;
	    if (sck->s_moptions)
	        *(uint8 *)optval = sck->s_moptions->ipo_ttl;
	    else
	        *(uint8 *)optval = IP_DEFAULT_MULTICAST_TTL;
	    
	    status = NU_SUCCESS;
	    break;

    default:

    	status = NU_INVAL;
	break;
    }

    return (status);

} /* IP_Get_Multi_Opt */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Set_Multi_Opt                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Sets the Multicast option buffer to the socket.                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Set_Opt                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      NU_Deallocate_Memory                                             */
/*      longswap                                                         */
/*      IP_Add_Multi                                                     */
/*      IP_Delete_Multi                                                  */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

STATUS IP_Set_Multi_Opt(int16 socketd, INT optname, void *optval, INT optlen)
{
    SOCKET              *sck = socket_list[socketd];
    IP_MULTI_OPTIONS    *moptions;
    IP_MREQ             *mreq;
    DV_DEVICE_ENTRY     *dev;
    STATUS              status = NU_SUCCESS;
    INT                 i;

    /* Is there a multicast option buffer attached to the socket. */
    if (sck->s_moptions == NU_NULL)
    {
	    /* Allocate a multicast option buffer. */
	    if (NU_Allocate_Memory(&System_Memory, (VOID **)&sck->s_moptions,
			        sizeof (*sck->s_moptions), 
			        (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
	    {
	        return(NU_MEM_ALLOC);
	    }

	    /* Initialize the option buffer to the default values. */
	    moptions = sck->s_moptions;
	    moptions->ipo_device = NU_NULL;
	    moptions->ipo_ttl = IP_DEFAULT_MULTICAST_TTL;
	    moptions->ipo_loop = IP_DEFAULT_MULTICAST_LOOP;
	    moptions->ipo_num_mem = 0;
    }
    else
    	moptions = sck->s_moptions;

    switch (optname)
    {
    case IP_ADD_MEMBERSHIP :  /* Add a multicast group. */

	    /* Check the parameters. */
	    if ( (optval == NU_NULL) || (optlen != sizeof(IP_MREQ)) )
	    {
	        status = NU_INVAL;
	        break;
	    }

	    mreq = (IP_MREQ *)optval;

	    /* Is the multicast address valid. */
	    if (!IP_MULTICAST_ADDR(longswap(mreq->sck_multiaddr)))
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Was a specific interface requested. */
	    if (mreq->sck_inaddr == IP_ADDR_ANY)
	    {
	        /* If no interface address was given then use the first registered 
	           interface that is multicast capable. */
	        for (dev = DEV_Table.dv_head; dev != NU_NULL; dev = dev->dev_next)
		    if (dev->dev_flags & DV_MULTICAST)
		        break;

	    }
	    else
	    {
	        /* A specific interface was requested. Find it. Check for a match on
	           the IP address and verify the interface is multicast capable. */
	        for (dev = DEV_Table.dv_head; dev != NU_NULL; dev = dev->dev_next)
		    if ( *(uint32 *)dev->dev_addr.dev_ip_addr == mreq->sck_inaddr )
		        break;
	    }

	    /* Was an interface found? */
	    if ( (dev == NU_NULL) || (dev->dev_flags & DV_MULTICAST) == 0)
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* See if membership already exists or if all the membership slots 
	       are full. */
	    for (i = 0; i < moptions->ipo_num_mem; ++i)
	    {
	        if ( (moptions->ipo_membership[i]->ipm_device == dev) &&
		     (moptions->ipo_membership[i]->ipm_addr == mreq->sck_multiaddr))
		     break;
	    }

	    /* If a match was found the membership already exists. */
	    if (i < moptions->ipo_num_mem)
	    {
	        status = NU_ADDRINUSE;
	        break;
	    }

	    /* Are all the membership entries in use. */
	    if (i == IP_MAX_MEMBERSHIPS)
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Add a new record to the multicast address list for the interface. */
	    if ( (moptions->ipo_membership[i] = 
	           IP_Add_Multi(mreq->sck_multiaddr, dev)) == NU_NULL)
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Increment the number of references. */
	    moptions->ipo_num_mem++;
	    break;

    case IP_DROP_MEMBERSHIP :       /* Delete a multicast group. */

	    /* Check the parameters. */
	    if ( (optval == NU_NULL) || (optlen != sizeof(IP_MREQ)) )
	    {
	        status = NU_INVAL;
	        break;
	    }
	    
	    mreq = (IP_MREQ *)optval;

	    /* Is the multicast address valid. */
	    if (!IP_MULTICAST_ADDR(longswap(mreq->sck_multiaddr)))
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Was a interface address specified. */
	    if (mreq->sck_inaddr == IP_ADDR_ANY)
	    {
	        dev = NU_NULL;
	    }
	    else
	    {
	        /* A specific interface was requested. Find it. Check for a match on
	           the IP address and verify the interface is multicast capable. */
	        for (dev = DEV_Table.dv_head; dev != NU_NULL; dev = dev->dev_next)
		    if ( *(uint32 *)dev->dev_addr.dev_ip_addr == mreq->sck_inaddr )
		        break;

	        if (dev == NU_NULL)
	        {
		    status = NU_INVAL;
		    break;
	        }
	    }

	    /* Find the membership in the membership array. */
	    for (i = 0; i < moptions->ipo_num_mem; ++i)
	    {
	        if ( ((dev == NU_NULL) || 
		     (moptions->ipo_membership[i]->ipm_device == dev)) &&
		     (moptions->ipo_membership[i]->ipm_addr == mreq->sck_multiaddr))
		     break;
	    }

	    if (i == moptions->ipo_num_mem)
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Delete the multicast address record to which the membership points. */
	    IP_Delete_Multi(moptions->ipo_membership[i]);

	    /* Remove the now empty space in the membership array. */
	    for (++i; i < moptions->ipo_num_mem; ++i)
	        moptions->ipo_membership[i - 1] = moptions->ipo_membership[i];

	    moptions->ipo_num_mem--;

	    break;

    case IP_MULTICAST_TTL :       /* Set the TTL for this outgoing multicasts. */

	    /* Check the parameters. */
	    if ( (optval == NU_NULL) || (optlen != 1) )
	    {
	        status = NU_INVAL;
	        break;
	    }

	    /* Set the new TTL. */
	    moptions->ipo_ttl = *(uint8 *)optval;
	    break;

    default :

    	status = NU_INVALID_OPTION;
	    break;
    }

    /* If all the options have default values, then there is no need to keep 
       the structure. */
    if ( (moptions->ipo_device == NU_NULL) && 
	 (moptions->ipo_ttl == IP_DEFAULT_MULTICAST_TTL) &&
	 (moptions->ipo_loop == IP_DEFAULT_MULTICAST_LOOP) &&
	 (moptions->ipo_num_mem == 0) )
    {
	    NU_Deallocate_Memory(moptions);
	    sck->s_moptions = NU_NULL;
    }

    return (status);

} /* IP_Set_Multi_Opt */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Add_Multi                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Adds an entry into the multicast group list.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Set_Multi_Opt                                                 */
/*      IGMP_Initialize                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Lookup_Multi                                                  */
/*      NU_Allocate_Memory                                               */
/*      NU_Deallocate_Memory                                             */
/*      RTAB_ADD_Route                                                   */
/*      IGMP_Join                                                        */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

IP_MULTI *IP_Add_Multi(uint32 m_addr, DV_DEVICE_ENTRY *dev)
{
    IP_MULTI            *ipm;
    DEV_IF_ADDRESS      *if_addr = &dev->dev_addr;
    STATUS              status;
    DV_REQ              d_req;
    INT                 old_level;

    /*  Temporarily lockout interrupts. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Determine if the interface is already a member of the group. */    
    ipm = IP_Lookup_Multi(m_addr, if_addr);

    if (ipm != NU_NULL)
    {
	    /* The interface is already a member of the group. Increment the 
	       reference count. */
	    ipm->ipm_refcount++;

	    /*  Restore the previous interrupt lockout level.  */
	    NU_Control_Interrupts(old_level);
    }
    else
    {
	    /* This is a new group membership request. Allocate memory for it. */
	    status = NU_Allocate_Memory(&System_Memory, (VOID **)&ipm,
			        sizeof (*ipm), (UNSIGNED)NU_NO_SUSPEND);

	    if (status != NU_SUCCESS)
	        return(NU_NULL);

	    /* Initialize the multicast group. */
	    ipm->ipm_addr = m_addr;
	    ipm->ipm_device = dev;
	    ipm->ipm_refcount = 1;
	    
	    /* Link it into the group membership list for this interface. */
	    ipm->ipm_next = if_addr->dev_multiaddrs;
	    if_addr->dev_multiaddrs = ipm;

	    /*  Restore the previous interrupt lockout level.  */
	    NU_Control_Interrupts(old_level);

	    d_req.dvr_addr = m_addr;

	    /* Now ask the driver to update its multicast reception filter. */
	    if ( (dev->dev_ioctl == NU_NULL) ||
	         (*dev->dev_ioctl)(dev, DEV_ADDMULTI, &d_req) != NU_SUCCESS )
	    {
	        NU_Deallocate_Memory(ipm);
	        return (NU_NULL);
	    }

	    /* Add a route to this multicast address. */
	    RTAB_Add_Route(dev, m_addr, 0xffffffff, *(uint32 *)dev->dev_addr.dev_ip_addr, 
		          (int16)(RT_UP | RT_HOST) );

	    /* Inform IGMP of the new group membership. */
	    IGMP_Join(ipm);
    }

    return (ipm);

} /* IP_Add_Multi */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Delete_Multi                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Deletes an entry in the Multicast group list.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Set_Multi_Opt                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IGMP_Leave                                                       */
/*      Driver Specific Function to update list.                         */
/*      NU_Deallocate_Memory                                             */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

STATUS IP_Delete_Multi(IP_MULTI *ipm)
{
    IP_MULTI            **ptr;
    DEV_IF_ADDRESS      *if_addr = &ipm->ipm_device->dev_addr;
    DV_REQ              d_req;
    INT                 old_level;

    /*  Temporarily lockout interrupts. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    if (--ipm->ipm_refcount == 0)
    {
	    /* There are no remaining claims to this record; let IGMP know that 
    	   we are leaving the multicast group. */
	    IGMP_Leave(ipm);

	    /* unlink this one from the list. */
	    for ( ptr = &if_addr->dev_multiaddrs;
	          *ptr != ipm;
	          ptr = &(*ptr)->ipm_next)
	    {
	        continue;
	    }

	    d_req.dvr_addr = ipm->ipm_addr;

	    /*  Restore the previous interrupt lockout level.  */
	    NU_Control_Interrupts(old_level);

	    /* Notify the driver to update its multicast reception filter. */
	    (*ipm->ipm_device->dev_ioctl)(ipm->ipm_device, DEV_DELMULTI, &d_req);

	    NU_Deallocate_Memory(ipm);
    }
    else
    	/*  Restore the previous interrupt lockout level.  */
	    NU_Control_Interrupts(old_level);

    return (NU_SUCCESS);
	      
} /* IP_Delete_Multi */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Lookup_Multi                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Searches through the multicast group list for a match.           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IP_Add_Multi                                                     */
/*      IP_Interpret                                                     */
/*      IGMP_Interpret                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    03/18/98        Created initial version.         */
/*************************************************************************/

IP_MULTI *IP_Lookup_Multi(uint32 m_addr, DEV_IF_ADDRESS *if_addr)
{
    IP_MULTI            *ipm;

    /* Search the multicast group list for a match. */
    for ( ipm = if_addr->dev_multiaddrs; 
	  ipm != NU_NULL && ipm->ipm_addr != m_addr;
	  ipm = ipm->ipm_next );

    return ipm;

}/* IP_Lookup_Multi */

#endif /* INCLUDE_IP_MULTICASTING */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IP_Localaddr                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Searches through the attached devices to verify that the IP      */
/*      address passed in is a local IP address on the network.          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      RTAB_Redirect                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Glen Johnson    04/22/98        Created initial version.         */
/*************************************************************************/


INT IP_Localaddr(uint32 ip_addr)
{
    DV_DEVICE_ENTRY         *device;

    for (device = DEV_Table.dv_head; device; device = device->dev_next)
    {
	    if (device->dev_addr.dev_net == (ip_addr & device->dev_addr.dev_netmask))
	        /* The IP addr is directly connected. Return true. */
	        return 1;
    }

    /* Return false, the IP addr is not on a local network. */
    return 0;

} /* IP_Localaddr */
