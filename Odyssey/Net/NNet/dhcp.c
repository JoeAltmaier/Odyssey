/****************************************************************************/
/*                                                                          */
/*    CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.               */
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
/*  DHCP                                                      4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This file will contain all the DHCP routines.                       */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Kelly Wiles, Xact Inc.                                              */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      NU_Dhcp                        Creates a DHCP request packet and    */
/*                                     sends to the DHCP server to obtain an*/
/*                                     IP address for the specified named   */
/*                                     device.                              */
/*      NU_Dhcp_Release                Releases the address obtained by the */
/*                                     DHCP process.                        */        
/*      NU_rand                        Random Number Generator.             */
/*      DHCP_init                      Initializes DHCP Packet.             */
/*      DHCP_Process_Packets           Processes the Incoming DHCP Reply.   */
/*      DHCP_Process_ACK               Processes the Incoming DHCP ACK.     */
/*      DHCP_request                   Creates the DHCP request packet.     */
/*      DHCP_release                   Build DHCP Release Packet.           */
/*      ds_ptr->valfunc                User defined validation function.    */
/*      ds_ptr->optfunc                User defined vendor option function. */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      DHCP.H                         Holds the defines necessary for      */  
/*                                     DHCP to function properly.           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*  Don Sharer      03/10/98        Added Net 4.0 Implementation.           */
/*                                                                          */
/****************************************************************************/

/*
*       Includes
*/

#include "nucleus.h"
#include "tcpdefs.h"
#include "tcp_errs.h"
#include "externs.h"
#include "bootp.h"
#include "data.h"
#include "dhcp.h"
#include "rtab.h"
#include "rip2.h"


static ulint dhcp_xid;          /* global unique value */
static ulong next = 1;
/* Define prototypes for funciton references. */
static uint16 DHCP_init(DHCPLAYER *, DHCP_STRUCT *, CHAR *dv_name);
static uint16 DHCP_request(DHCPLAYER *, DHCP_STRUCT *);
static uint16 DHCP_release(DHCPLAYER *, DHCP_STRUCT *);
int DHCP_Process_Packets( int16, DHCP_STRUCT *, CHAR *dv_name, uint16);
int DHCP_Process_ACK( int16, DHCP_STRUCT *, uint16);
int16  NU_rand(void);



/* This list is used to hold on to the DHCP request that will be sent. By using
   this list the need to build the request each time it is transmitted is
   avoided. Instead the request is built once and then reused if
   retransmissions are necessary. */

NET_BUFFER_HEADER DHCP_List;
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      NU_Dhcp                                         4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a DHCP request packet and sends it         */
/*      to the DHCP server to obtain an IP address for the specified     */
/*      named device.                                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application Layer Function.                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DHCP_init                      Initializes DHCP Packet.          */
/*      NU_Socket                      Creates a Socket.                 */
/*      NU_TCP_Log_Error               Logs Errors.                      */
/*      NU_Bind                        Binds a Socket.                   */
/*      NU_Close_Socket                Closes a Socket.                  */
/*      UTL_Zero                       Zeroes out a Variable.            */
/*      DEV_Get_DEV_By_Name            Gets a Device structure by name.  */
/*      memcpy                         Copies a specified memory location*/
/*                                     to another.                       */
/*      memcmp                         Compares a specified memory       */
/*                                     location with another.            */
/*      MEM_Buffer_Chain_Dequeue       Dequeues any incoming buffers.    */
/*      intswap                        Swaps high low byte of a word.    */
/*      tcpcheck                       Calculates checksum for UDP/TCP   */
/*                                     packets.                          */
/*      ipcheck                        Calculates the checksum of the IP */
/*                                     packet.                           */
/*      DEV_Attach_IP_to_Device        Attaches the IP address to a      */
/*                                     particular device.                */
/*      NU_rand                        Random Number Generator.          */
/*      DHCP_Process_Packet            Processes the Incoming DHCP Reply.*/
/*      DHCP_Process_ACK               Processes the Incoming DHCP ACK.  */
/*      DHCP_request                   Creates the DHCP request packet.  */
/*      *(int_face->dev_start)         Transmits Packet.                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ds_ptr                         Pointer to DHCP Structure that    */
/*                                     contains data that can be obtained*/
/*                                     at the application layer.         */
/*      dv_name                        Pointer to Devices name.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      retval                         Retruns 0 if successful. Returns  */
/*                                     value less than 1 if unsuccessful.*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*      Don Sharer(ATI) 03-04-1998      Modified for Net 4.0             */
/*                                                                       */
/*************************************************************************/

int NU_Dhcp(DHCP_STRUCT *ds_ptr, CHAR *dv_name)
{
     int                 i;
     int                 ret;
     int                 found = 0;
     int16               socketd;
     uint16              delay, delay_mask = STARTING_DELAY;
     int16               failed = 0,retval=0;
     uint16              nbytes;
     struct addr_struct  clientaddr;
     DHCPLAYER           *dhcp_ptr;   /* DHCP uses a Bootp type struct */
     NET_BUFFER          *buf_ptr;
     UDPLAYER            *udp_pkt;
     IPLAYER             *ip_ptr;
     uint8               *buffer;
     NET_BUFFER          *next_buf=0;
     int16               hlen = sizeof (IPLAYER);
     int32               total_size, current_size;
     INT                 length;
     int32               temp_data_len,temp_total_data_len;
     struct pseudotcp    tcp_chk;
     DV_DEVICE_ENTRY     *int_face = 0;
     STATUS              status;
     int32               flags;
     SCK_SOCKADDR        sa;
     DLAYER              *ether_header;


	/* Create a socket and bind to it, so that we can receive packets. */
     socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, NU_NONE);
     if( socketd < NU_SUCCESS )
     {
	switch( socketd )
	    {
		    case NU_INVALID_PROTOCOL:
			     NU_Tcp_Log_Error( DHCP_INVALID_PROTOCOL, TCP_FATAL,
					      __FILE__, __LINE__);
			     break;
		    case NU_NO_MEMORY:
			     NU_Tcp_Log_Error( DHCP_NO_MEMORY, TCP_FATAL,
					       __FILE__, __LINE__);
			     break;
		    case NU_NO_SOCKET_SPACE:
			     NU_Tcp_Log_Error( DHCP_NO_SOCKET_SPACE, TCP_FATAL,
					       __FILE__, __LINE__);
			     break;
		    default:
			     NU_Tcp_Log_Error( DHCP_UNKNOWN, TCP_FATAL,
					       __FILE__, __LINE__);
			     break;
	  }
	  retval= -1;
    }

    /* build local address and port to bind to. */
    clientaddr.family = NU_FAMILY_IP;
    clientaddr.port   = IPPORT_DHCPC;
    clientaddr.id.is_ip_addrs[0] = (unsigned char) 0;
    clientaddr.id.is_ip_addrs[1] = (unsigned char) 0;
    clientaddr.id.is_ip_addrs[2] = (unsigned char) 0;
    clientaddr.id.is_ip_addrs[3] = (unsigned char) 0;
    clientaddr.name = "DHCP";

  
    ret = NU_Bind(socketd, &clientaddr, 0);
    if( ret != socketd )
    {
	    NU_Tcp_Log_Error( DHCP_SOCKET_BIND, TCP_FATAL,
			    __FILE__, __LINE__);
	    retval = ret;
    }


    UTL_Zero(ds_ptr->dhcp_mac_addr, sizeof(ds_ptr->dhcp_mac_addr));
    UTL_Zero(ds_ptr->dhcp_siaddr, sizeof(ds_ptr->dhcp_siaddr));
    UTL_Zero(ds_ptr->dhcp_giaddr, sizeof(ds_ptr->dhcp_giaddr));
    UTL_Zero(ds_ptr->dhcp_sname, sizeof(ds_ptr->dhcp_sname));
    UTL_Zero(ds_ptr->dhcp_file, sizeof(ds_ptr->dhcp_file));



    /* get a pointer to the interface structure. */
    int_face = DEV_Get_Dev_By_Name(dv_name);

    /* make sure that is a good one. */
    if( int_face == (DV_DEVICE_ENTRY *)0 )
    {
          retval = -1;
    }

    if (int_face->dev_mtu < DHCP_MAX_HEADER_SIZE)
        return (NU_DHCP_INIT_FAILED);

    /* copy device function to retrive MAC address */
    memcpy(ds_ptr->dhcp_mac_addr, int_face->dev_mac_addr, DADDLEN);


    /* make sure that the MAC address is good. */
    if( memcmp(ds_ptr->dhcp_mac_addr, "\0\0\0\0\0\0", 6) == 0 )
    {
          retval = -1;
    }        

    buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, DHCP_MAX_HEADER_SIZE);

    if (buf_ptr == NU_NULL)
        return (NU_NO_BUFFERS);


    buf_ptr->mem_dlist = &DHCP_List;

    status = NU_Allocate_Memory(&System_Memory,(VOID **) &buffer,sizeof(DHCPLAYER), NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
        return (NU_DHCP_INIT_FAILED);


	
    /* Set up pointers to each of the headers that make up a DHCP packet. */
    /*  Offset DHCP Pointer for even word address  */
    dhcp_ptr = (DHCPLAYER *)(buf_ptr->mem_parent_packet + 
                (DHCP_MAX_HEADER_SIZE - (sizeof (DHCPLAYER)- 1)));
    udp_pkt =  (UDPLAYER *) (((char *)dhcp_ptr) - sizeof (UDPLAYER));
    ip_ptr =  (IPLAYER *) (((char *)udp_pkt) - sizeof(IPLAYER));


    /* build the first DHCP discovery message */
    nbytes = DHCP_init((DHCPLAYER *)buffer, ds_ptr, dv_name);
    if( (nbytes % 2) != 0 )                 /* make it an even number */
         nbytes++;




    /* Initialize the local and foreign port numbers. */
    udp_pkt->source = intswap(IPPORT_DHCPC);
    udp_pkt->dest   = intswap(IPPORT_DHCPS);

    /*  Set up the UDP header. */

    /* As of RFC2131 and RFC3232 the vendor options field is a */
	/* variable length field up to the UDP packet size limit. */

    /*  Get the length of the buffer.  */
    udp_pkt->length = intswap((uint16)(nbytes + sizeof (UDPLAYER)));

    udp_pkt->check = 0;


    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + NET_MAX_UDP_HEADER_SIZE;
    total_size = nbytes;

    /*  Chain the DHCP Request Packet */

    if (total_size > NET_PARENT_BUFFER_SIZE)
    {
       current_size = NET_PARENT_BUFFER_SIZE - sizeof(IPLAYER) - NET_ETHER_HEADER_OFFSET_SIZE;
       total_size =   total_size - current_size;
    }
    else
    {
       current_size =  total_size;
    }

    /*  Copy DHCP Packet into first Buffer */
    memcpy(buf_ptr->data_ptr , buffer, current_size);

    /*  Set the Data Length to the Size of bytes copied.  */
    buf_ptr->data_len = current_size;

    /*  Set the Total data length to the Number of bytes in a DHCP Packet. */
    buf_ptr->mem_total_data_len = nbytes;

    /*  Increment DHCP Buffer to be at the number of bytes copied.  */
    buffer = buffer + current_size;

    /*  Check if another packet is chained.  */
    if (buf_ptr->next_buffer != NU_NULL)
    {
        next_buf = buf_ptr->next_buffer;
    }

    /*  Check to make sure there is data to store in the mem_packet */

    while ((total_size) && (next_buf != NU_NULL))
    {

         if (total_size > NET_MAX_BUFFER_SIZE)
         {
             
             current_size = NET_MAX_BUFFER_SIZE;
         }
         else
         {
             current_size = total_size;
         }
         total_size = total_size - current_size;

         /*  Copy the remaining data in the chaining packets  */
         memcpy(next_buf->mem_packet,buffer,current_size);

         /*  Set the Data pointer to the remainder of the packets.  */
         next_buf->data_ptr = next_buf->mem_packet;
         next_buf->next = NU_NULL;
         next_buf->data_len = current_size;


         buffer = buffer + current_size;

         if (next_buf->next_buffer != NU_NULL)
         {
            next_buf = next_buf->next_buffer;
         }

    }


    /* Increment the Packet data pointer to the UDP layer */
    buf_ptr->data_ptr -= sizeof(UDPLAYER);

    /*  Copy the udp_pkt into the parent_packet.  */
    memcpy(buf_ptr->data_ptr,(uint8 *)udp_pkt,sizeof(UDPLAYER));

    /*  Increment the total data length */
    buf_ptr->mem_total_data_len += sizeof(UDPLAYER);

    /*  Increment the data length of this packet.  */
    buf_ptr->data_len += sizeof(UDPLAYER);

    /*  Calculate UDP Checksum  */

    tcp_chk.source  = 0x0;
    tcp_chk.dest    = 0xFFFFFFFF;
    tcp_chk.z       = 0;
    tcp_chk.proto   = IP_UDP_PROT;
    tcp_chk.tcplen  = intswap((uint16)buf_ptr->mem_total_data_len);
    udp_pkt->check =  tcpcheck( (uint16 *)&tcp_chk, buf_ptr);



    /* If a checksum of zero is computed it should be replaced with 0xffff. */
    if (udp_pkt->check == 0)
        udp_pkt->check = 0xFFFF;



    /* Set the IP header length and the IP version. */
    ip_ptr->versionandhdrlen = (hlen >> 2) | (IP_VERSION << 4);

    /* Set to no fragments and don't fragment. */
    /* ip_ptr->frags = intswap(IP_DF); */
    ip_ptr->frags = 0;

    /* Set the IP packet ID. */
    ip_ptr->ident = 0;

    /* Set the type of service. */
    ip_ptr->service = 0;

    length = nbytes + (uint16)sizeof(UDPLAYER);

    /* Set the total length (data and IP header) for this packet. */
    ip_ptr->tlen = intswap((int16)(length + hlen));

    /* Set the time to live */
    ip_ptr->ttl = IP_TIME_TO_LIVE;

    /* Set the protocol. */
    ip_ptr->protocol = IP_UDP_PROT;

    /* we are doing a broadcast, so we do not need this fields */
    memset(ip_ptr->ipsource, 0, 4);
    memcpy(ip_ptr->ipdest, "\377\377\377\377", 4);

    /* Compute the IP checksum. Note that the length expected by */
    /* ipcheck is the length of the header in 16 bit half-words. */
    ip_ptr->check = 0;
    ip_ptr->check = ipcheck ((uint16 *)ip_ptr, (uint16)(hlen >> 1));


    /*  Set the buffer pointer to the IP Layer.  */

    buf_ptr->data_ptr -= sizeof(IPLAYER);

    /*  Add the IPLAYER to the total data length */

    buf_ptr->mem_total_data_len += sizeof(IPLAYER);   
    temp_total_data_len =  buf_ptr->mem_total_data_len;

    /*  Set the data length of the current packet.  */
    buf_ptr->data_len += sizeof(IPLAYER);

    temp_data_len =  buf_ptr->data_len;

    /*  Copy the IP header into the parent packet of the buffer chain.  */

    memcpy(buf_ptr->data_ptr,(uint8 *)ip_ptr,sizeof(IPLAYER));

    /*  Set initial timeout delay  */
    delay = (delay_mask & NU_rand()) + 1;


    /*  Set up Mac layer  */
    ether_header = (DLAYER *)sa.sck_data;
    memcpy(ether_header->dest, NET_Ether_Broadaddr, DADDLEN);
    memcpy(ether_header->me, ds_ptr->dhcp_mac_addr, DADDLEN);
    ether_header->type = EIP;

    sa.sck_family = SK_FAM_UNSPEC;
    sa.sck_len = sizeof(sa);


    for( i = 0; i < RETRIES_COUNT; i++ )
    {
        /* Send the packet. */

        /* Grab the semaphore because we are about to change the interface
           that the DHCP request will be sent over. */
        NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);

        /* The device will not send packets until an IP address is attached.
           Temporarily trick it into thinking an IP address is attached so this
           request can be sent.  Then set it back. */
        flags = int_face->dev_flags;
        int_face->dev_flags |= (DV_UP | DV_RUNNING);

        /* Send the packet. */
        status = (*(int_face->dev_output)) (buf_ptr, int_face,
                                           (SCK_SOCKADDR_IP *)&sa, NU_NULL);
        int_face->dev_flags = flags;

        NU_Release_Semaphore(&TCP_Resource);


    
        if( status != NU_SUCCESS )
        {
             NU_Tcp_Log_Error( DHCP_SEND_FAILED, TCP_FATAL,
                                __FILE__, __LINE__);
             retval = -1;
             break;
        }



        /* Look at each packet on buffer_list to see if it is mine */
        found = DHCP_Process_Packets(socketd, ds_ptr, dv_name, delay);
        if( found )               /* found the packet */
        {
             retval = 0;
             break;
        }                         /* found the packet */

        delay_mask++;
        delay = (delay_mask & NU_rand()) + 1;

        found = 0;

        retval= DHCP_RECV_FAILED;

        /*  Get the the packet off the Trasmitted list and retrasnmit again. */
        buf_ptr= DHCP_List.head;
        buf_ptr->data_ptr = DHCP_List.head->mem_parent_packet + NET_ETHER_HEADER_OFFSET_SIZE;
        buf_ptr->data_len = temp_data_len;
        buf_ptr->mem_total_data_len = temp_total_data_len;

    }

    /* The returned value from DHCP_Process_Packet is tested here */
    /* to see if the IP address was accepted by the caller. */
    if( (found & ACCEPT_BIT) == ACCEPT_BIT )
    {
         /*  Dequeue Peviously used buffer  */

         MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);

         buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, DHCP_MAX_HEADER_SIZE);

         if (buf_ptr == NU_NULL)
               return (NU_NO_BUFFERS);


         buf_ptr->mem_dlist = &DHCP_List;

         /* Set up pointers to each of the headers that make up a DHCP packet. */
         dhcp_ptr = (DHCPLAYER *)(buf_ptr->mem_parent_packet + 
                    (DHCP_MAX_HEADER_SIZE - (sizeof (DHCPLAYER) - 1) ));
         udp_pkt = (UDPLAYER *) (((char *)dhcp_ptr) - sizeof (UDPLAYER));
         ip_ptr = (IPLAYER *) (((char *)udp_pkt) - sizeof(IPLAYER));


         /*  ) out the allocated buffer for the DHCP requests.  */
         memset(buffer,0,sizeof(DHCPLAYER));

         /* build DHCP request packet */
         nbytes = DHCP_request((DHCPLAYER *)buffer, ds_ptr);
         if( (nbytes % 2) != 0 )                 /* make it an even number */
              nbytes++;


         /* build up the DLAYER */


         /* Initialize the local and foreign port numbers. */
         udp_pkt->source = intswap(IPPORT_DHCPC);
         udp_pkt->dest   = intswap(IPPORT_DHCPS);

         /*  Set up the UDP header. */

         /* As of RFC2131 and RFC3232 the vendor options field is a */
         /* variable length field up to the UDP packet size limit. */

         /*  Get the length of the buffer.  */
         udp_pkt->length = intswap ((uint16)(nbytes + sizeof (UDPLAYER)));

         udp_pkt->check = 0;


         buf_ptr->data_ptr = buf_ptr->mem_parent_packet + NET_MAX_UDP_HEADER_SIZE;
         total_size = nbytes;

         /*  Chain the DHCP Request Packet */

         if (total_size > NET_PARENT_BUFFER_SIZE)
         {
             current_size = NET_PARENT_BUFFER_SIZE - sizeof(IPLAYER) - NET_ETHER_HEADER_OFFSET_SIZE;
             total_size =   total_size - current_size;
         }
         else
         {
             current_size =  total_size;
         }

         /*  Copy DHCP Packet into first Buffer */
         memcpy(buf_ptr->data_ptr , buffer, current_size);

         /*  Set the Data Length to the Size of bytes copied.  */
         buf_ptr->data_len = current_size;

         /*  Set the Total data length to the Number of bytes in a DHCP Packet. */
         buf_ptr->mem_total_data_len = nbytes;

         /*  Increment DHCP Buffer to be at the number of bytes copied.  */
         buffer = buffer + current_size;

         /*  Check if another packet is chained.  */
         if (buf_ptr->next_buffer != NU_NULL)
         {
             next_buf = buf_ptr->next_buffer;
         }

        /*  Check to make sure there is data to store in the mem_packet */

        while ((total_size) && (next_buf != NU_NULL))
        {

             if (total_size > NET_MAX_BUFFER_SIZE)
             {
             
                 current_size = NET_MAX_BUFFER_SIZE;
             }
             else
             {
                 current_size = total_size;
             }
             total_size = total_size - current_size;

             /*  Copy the remaining data in the chaining packets  */
             memcpy(next_buf->mem_packet,buffer,current_size);

             /*  Set the Data pointer to the remainder of the packets.  */
             next_buf->data_ptr = next_buf->mem_packet;
             next_buf->next = NU_NULL;
             next_buf->data_len = current_size;


             buffer = buffer + current_size;

             if (next_buf->next_buffer != NU_NULL)
             {
                 next_buf = next_buf->next_buffer;
             }

        }


        /* Increment the Packet data pointer to the UDP layer */
        buf_ptr->data_ptr -= sizeof(UDPLAYER);

        /*  Copy the udp_pkt into the parent_packet.  */
        memcpy(buf_ptr->data_ptr,(uint8 *)udp_pkt,sizeof(UDPLAYER));

        /*  Increment the total data length */
        buf_ptr->mem_total_data_len += sizeof(UDPLAYER);

        /*  Increment the data length of this packet.  */
        buf_ptr->data_len += sizeof(UDPLAYER);

        /*  Calculate UDP Checksum  */

        tcp_chk.source  = 0x0;
        tcp_chk.dest    = 0xFFFFFFFF;
        tcp_chk.z       = 0;
        tcp_chk.proto   = IP_UDP_PROT;
        tcp_chk.tcplen  = intswap((uint16)buf_ptr->mem_total_data_len);
        udp_pkt->check =  tcpcheck( (uint16 *)&tcp_chk, buf_ptr);

        /* If a checksum of zero is computed it should be replaced with 0xffff. */
        if (udp_pkt->check == 0)
            udp_pkt->check = 0xFFFF;

        /* Set the IP header length and the IP version. */
        ip_ptr->versionandhdrlen = (hlen >> 2) | (IP_VERSION << 4);

        /* Set to no fragments and don't fragment. */

        ip_ptr->frags = 0;

        /* Set the IP packet ID. */
        ip_ptr->ident = 0;

        /* Set the type of service. */
        ip_ptr->service = 0;

        length = nbytes + sizeof(UDPLAYER);

        /* Set the total length (data and IP header) for this packet. */
        ip_ptr->tlen = intswap((int16)(length + hlen));

        /* Set the time to live */
        ip_ptr->ttl = IP_TIME_TO_LIVE;
	    
        /* Set the protocol. */
        ip_ptr->protocol = IP_UDP_PROT;

        /* we are doing a broadcast, so we do not need this fields */
        memset(ip_ptr->ipsource, 0, 4);
        memcpy(ip_ptr->ipdest, "\377\377\377\377", 4);

        /* Compute the IP checksum. Note that the length expected by */
        /* ipcheck is the length of the header in 16 bit half-words. */
        ip_ptr->check = 0;
        ip_ptr->check = ipcheck ((uint16 *)ip_ptr, (uint16)(hlen >> 1));

        /*  Set the buffer pointer to the IP Layer.  */

        buf_ptr->data_ptr -= sizeof(IPLAYER);

        /*  Add the IPLAYER to the total data length */

        buf_ptr->mem_total_data_len += sizeof(IPLAYER);   
        temp_total_data_len =  buf_ptr->mem_total_data_len;

        /*  Set the data length of the current packet.  */
        buf_ptr->data_len += sizeof(IPLAYER);

        temp_data_len =  buf_ptr->data_len;

        /*  Copy the IP header into the parent packet of the buffer chain.  */

        memcpy(buf_ptr->data_ptr,(uint8 *)ip_ptr,sizeof(IPLAYER));

        delay = STARTING_DELAY;
        delay = (delay_mask & NU_rand()) + 1;


        ether_header = (DLAYER *)sa.sck_data;
        memcpy(ether_header->dest, NET_Ether_Broadaddr, DADDLEN);
        memcpy(ether_header->me, ds_ptr->dhcp_mac_addr, DADDLEN);
        ether_header->type = EIP;

        sa.sck_family = SK_FAM_UNSPEC;
        sa.sck_len = sizeof(sa);

        found = 0;

        for( i = 0; i < RETRIES_COUNT; i++ )
        {

             /* Grab the semaphore because we are about to change the interface
                that the DHCP request will be sent over. */
             NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);

             /* The device will not send packets until an IP address is attached.
                Temporarily trick it into thinking an IP address is attached so this
                request can be sent.  Then set it back. */
             flags = int_face->dev_flags;
             int_face->dev_flags |= (DV_UP | DV_RUNNING);

             /* Send the packet. */
             status = (*(int_face->dev_output)) (buf_ptr, int_face,
                       (SCK_SOCKADDR_IP *)&sa, NU_NULL);
             int_face->dev_flags = flags;

             NU_Release_Semaphore(&TCP_Resource);

             if( status != NU_SUCCESS )
             {
                 NU_Tcp_Log_Error( DHCP_SEND_FAILED, TCP_FATAL,
                                   __FILE__, __LINE__);
                 retval = -1;
                 break;
             }
#if(DHCP_ACK_NOWAIT)

		 found = 1;
		 break;
#else

             found = DHCP_Process_ACK(socketd, ds_ptr, delay);
             if( found == 1 )
                 break;                                         /* found the packet */
             if( found == 2 )
             {
                 ext_loop++;
                 break;
             }

             delay_mask++;
             found = 0;
             delay = (delay_mask & NU_rand()) + 1;
             buf_ptr= DHCP_List.head;
             buf_ptr->data_ptr = DHCP_List.head->mem_parent_packet + NET_ETHER_HEADER_OFFSET_SIZE;
             buf_ptr->data_len = temp_data_len;
             buf_ptr->mem_total_data_len = temp_total_data_len;


#endif

         
        }

        if( found == 1 )
        {
            status = DEV_Attach_IP_To_Device(dv_name,
                                             ds_ptr->dhcp_yiaddr,
                                             ds_ptr->dhcp_net_mask);
            if( status != NU_SUCCESS )
            {
                NU_Tcp_Log_Error( DHCP_ATTACH_IP, TCP_FATAL,
                                  __FILE__, __LINE__);
                failed++;
            }  /* if status */
        } /*  If found */
    } /*  Accept Bit  */
    else
    {
         NU_Tcp_Log_Error( DHCP_CONFIG_INTERFACE, TCP_FATAL,
                           __FILE__, __LINE__);
         failed++;
    }

    MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    NU_Deallocate_Memory(buffer);

    NU_Close_Socket(socketd);
    if (retval != 0)
       return(retval);

   return((int32)(failed));
}                                       /* NU_Dhcp */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      NU_Dhcp_Release                                 4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function releases the address acquired by the NU_Dhcp       */
/*      function call.                                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application Layer Function                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_TCP_Log_Error               Logs Errors.                      */
/*      UTL_Zero                       Zeroes out a Variable.            */
/*      DEV_Get_DEV_By_Name            Gets a Device structure by name.  */
/*      memcpy                         Copies a specified memory location*/
/*                                     to another.                       */
/*      MEM_Buffer_Chain_Dequeue       Dequeues any incoming buffers.    */
/*      intswap                        Swaps high low byte of a word.    */
/*      tcpcheck                       Calculates checksum for UDP/TCP   */
/*                                     packets.                          */
/*      ipcheck                        Calculates the checksum of the IP */
/*                                     packet.                           */
/*      DHCP_release                   Build DHCP Release Packet.        */
/*      memset                         Set memory to a particular value. */
/*      *(int_face->dev_start)         Transmits Packet.                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ds_ptr                         Pointer to DHCP Structure that    */
/*                                     contains data that can be obtained*/
/*                                     at the application layer.         */
/*      dv_name                        Pointer to Devices name.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns 0 if NU_SUCCESS, otherwise a -1 is returned.             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*      Don Sharer      03-04-1998      Updated for Net 4.0              */
/*                                                                       */
/*************************************************************************/

STATUS NU_Dhcp_Release( DHCP_STRUCT *ds_ptr, CHAR *dv_name)
{
    uint16            nbytes;
    DHCPLAYER         *dhcp_ptr;          /* DHCP uses a Bootp type struct */
    NET_BUFFER        *buf_ptr;
    UDPLAYER          *udp_pkt;
    IPLAYER           *ip_ptr;
    DLAYER            *ether_header;
    NET_BUFFER        *next_buf=0;
    int32             total_size, current_size;
    struct pseudotcp  tcp_chk;
    INT               hlen = sizeof (IPLAYER);
    INT               length;
    DV_DEVICE_ENTRY   *int_face = 0;
    STATUS            status;
    int32             flags;
    SCK_SOCKADDR      sa;
    uint8             *buffer;

    /* get a pointer to the interface structure. */
    int_face = DEV_Get_Dev_By_Name(dv_name);
    if (int_face->dev_mtu < DHCP_MAX_HEADER_SIZE)
        return (NU_DHCP_INIT_FAILED);

    status = NU_Allocate_Memory(&System_Memory,(VOID **) &buffer,sizeof(BOOTPLAYER), NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
        return (NU_DHCP_INIT_FAILED);

    memset(buffer,0,sizeof(DHCPLAYER));
    buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, DHCP_MAX_HEADER_SIZE);

    if(buf_ptr == NU_NULL)
    {
         NU_Tcp_Log_Error( DHCP_NO_BUFFERS, TCP_FATAL,
				  __FILE__, __LINE__);
         return (NU_NO_BUFFERS);
    }



    buf_ptr->mem_dlist = &DHCP_List;





    /* Set up pointers to each of the headers that make up a DHCP packet. */
    dhcp_ptr = (DHCPLAYER *)(buf_ptr->mem_parent_packet + 
                   (DHCP_MAX_HEADER_SIZE - (sizeof (DHCPLAYER)- 1) ));
    udp_pkt = (UDPLAYER *) (((char *)dhcp_ptr) - sizeof (UDPLAYER));
    ip_ptr = (IPLAYER *) (((char *)udp_pkt) - sizeof(IPLAYER));



	/* build the DHCP release message */
    nbytes = DHCP_release((DHCPLAYER *)buffer, ds_ptr);
		

    if( (nbytes % 2) != 0 )                 /* make it an even number */
           nbytes++;



    /* Initialize the local and foreign port numbers. */
    udp_pkt->source = intswap(IPPORT_DHCPC);
    udp_pkt->dest   = intswap(IPPORT_DHCPS);

    /*  Set up the UDP header. */

    /* As of RFC2131 and RFC3232 the vendor options field is a */
    /* variable length field up to the UDP packet size limit. */

    /*  Get the length of the buffer.  */
    udp_pkt->length = intswap ((uint16)(nbytes + sizeof (UDPLAYER)));

    udp_pkt->check = 0;


    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + NET_MAX_UDP_HEADER_SIZE;
    total_size = nbytes;

    /*  Chain the DHCP Request Packet */

    if (total_size > NET_PARENT_BUFFER_SIZE)
    {
       current_size = NET_PARENT_BUFFER_SIZE - sizeof(IPLAYER) - NET_ETHER_HEADER_OFFSET_SIZE;
       total_size =   total_size - current_size;
    }
    else
    {
       current_size =  total_size;
    }

    /*  Copy DHCP Packet into first Buffer */
    memcpy(buf_ptr->data_ptr , buffer, current_size);

    /*  Set the Data Length to the Size of bytes copied.  */
    buf_ptr->data_len = current_size;

    /*  Set the Total data length to the Number of bytes in a DHCP Packet. */
    buf_ptr->mem_total_data_len = nbytes;

    /*  Increment DHCP Buffer to be at the number of bytes copied.  */
    buffer = buffer + current_size;

    /*  Check if another packet is chained. */
    if (buf_ptr->next_buffer != NU_NULL)
    {
        next_buf = buf_ptr->next_buffer;
    }

    /*  Check to make sure there is data to store in the mem_packet */

    while ((total_size) && (next_buf != NU_NULL))
    {

         if (total_size > NET_MAX_BUFFER_SIZE)
         {
             
             current_size = NET_MAX_BUFFER_SIZE;
         }
         else
         {
             current_size = total_size;
         }
         total_size = total_size - current_size;

         /*  Copy the remaining data in the chaining packets  */
         memcpy(next_buf->mem_packet,buffer,current_size);

         /*  Set the Data pointer to the remainder of the packets.  */
         next_buf->data_ptr = next_buf->mem_packet;
         next_buf->next = NU_NULL;
         next_buf->data_len = current_size;


         buffer = buffer + current_size;

         if (next_buf->next_buffer != NU_NULL)
         {
            next_buf = next_buf->next_buffer;
         }

    }


    /* Increment the Packet data pointer to the UDP layer */
    buf_ptr->data_ptr -= sizeof(UDPLAYER);

    /*  Copy the udp_pkt into the parent_packet.  */
    memcpy(buf_ptr->data_ptr,(uint8 *)udp_pkt,sizeof(UDPLAYER));

    /*  Increment the total data length */
    buf_ptr->mem_total_data_len += sizeof(UDPLAYER);

    /*  Increment the data length of this packet.  */
    buf_ptr->data_len += sizeof(UDPLAYER);

    /*  Calculate UDP Checksum  */

    tcp_chk.source  = 0x0;
    tcp_chk.dest    = 0xFFFFFFFF;
    tcp_chk.z       = 0;
    tcp_chk.proto   = IP_UDP_PROT;
    tcp_chk.tcplen  = intswap((uint16)buf_ptr->mem_total_data_len);
    udp_pkt->check =  tcpcheck( (uint16 *)&tcp_chk, buf_ptr);

    /* If a checksum of zero is computed it should be replaced with 0xffff. */
    if (udp_pkt->check == 0)
        udp_pkt->check = 0xFFFF;

    /* Set the IP header length and the IP version. */
    ip_ptr->versionandhdrlen = (hlen >> 2) | (IP_VERSION << 4);

    /* Set to no fragments and don't fragment. */

    ip_ptr->frags = 0;

    /* Set the IP packet ID. */
    ip_ptr->ident = 0;

    /* Set the type of service. */
    ip_ptr->service = 0;

    length = nbytes + sizeof(UDPLAYER);

    /* Set the total length (data and IP header) for this packet. */
    ip_ptr->tlen = intswap((int16)(length + hlen));

    /* Set the time to live */
    ip_ptr->ttl = IP_TIME_TO_LIVE;

    /* Set the protocol. */
    ip_ptr->protocol = IP_UDP_PROT;
    memset(ip_ptr->ipsource, 0, 4);

    memcpy(ip_ptr->ipdest, "\377\377\377\377", 4);

    /* Compute the IP checksum. Note that the length expected by */
    /* ipcheck is the length of the header in 16 bit half-words. */
    ip_ptr->check = 0;
    ip_ptr->check = ipcheck ((uint16 *)ip_ptr, (uint16)(hlen >> 1));

    /*  Set the buffer pointer to the IP Layer.  */

    buf_ptr->data_ptr -= sizeof(IPLAYER);

    /*  Add the IPLAYER to the total data length */

    buf_ptr->mem_total_data_len += sizeof(IPLAYER);

    /*  Set the data length of the current packet.  */

    buf_ptr->data_len += sizeof(IPLAYER);

    /*  Copy the IP header into the parent packet of the buffer chain.  */

    memcpy(buf_ptr->data_ptr,(uint8 *)ip_ptr,sizeof(IPLAYER));


    ether_header = (DLAYER *)sa.sck_data;
    memcpy(ether_header->dest, NET_Ether_Broadaddr, DADDLEN);
    memcpy(ether_header->me, ds_ptr->dhcp_mac_addr, DADDLEN);
    ether_header->type = EIP;

    sa.sck_family = SK_FAM_UNSPEC;
    sa.sck_len = sizeof(sa);


    /* Grab the semaphore because we are about to change the interface
       that the DHCP request will be sent over. */
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);

    /* The device will not send packets until an IP address is attached.
       Temporarily trick it into thinking an IP address is attached so this
       request can be sent.  Then set it back. */

    flags = int_face->dev_flags;
    int_face->dev_flags |= (DV_UP | DV_RUNNING);

    /* Send the packet. */
    status = (*(int_face->dev_output)) (buf_ptr, int_face,
             (SCK_SOCKADDR_IP *)&sa, NU_NULL);
    int_face->dev_flags = flags;

    NU_Release_Semaphore(&TCP_Resource);

    if( status != NU_SUCCESS )
    {
         NU_Tcp_Log_Error( DHCP_SEND_FAILED, TCP_FATAL,
                           __FILE__, __LINE__);
         return(-1);
    }

    MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    NU_Deallocate_Memory(buffer);
    return(0);
}                                       /* NU_Dhcp_Release */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      DHCP_Process_Packets                            4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function handles processing a DHCP returned packet.         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Dhcp                 Creates a DHCP request packet and sends  */
/*                              to the DHCP server to obtain an IP       */
/*                              address for the specified named device.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory      Allocate Memory.                         */
/*      NU_Deallocate_Memory    Deallocates Memory.                      */
/*      ds_ptr->valfunc         User defined validation function.        */
/*      ds_ptr->optfunc         user defined vendor option function.     */
/*      NU_TCP_Log_Error        Log a TCP error.                         */
/*      NU_FD_Init              Sets all bits in a bitmap to zero.       */
/*      NU_FD_Set               Sets a bit in a bitmap.                  */
/*      NU_Select               Check for data on multiple sockets.      */
/*      NU_FD_Check             Checks to see if a particular bit has    */
/*                              been set in a bitmap.                    */
/*      NU_Recv_From            Receive Data from a socket descriptor.   */
/*      memcpy                  Copies one section of memory to another. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      socketd                 Socket Descriptor to retrieve data from. */    
/*      ds_ptr                  Pointer to DHCP Structure that contains  */
/*                              data that can be obtained at the         */
/*                              application layer.                       */
/*      dv_name                 Pointer to Devices name.                 */
/*      timeout                 Timeout value used for NU_Select.        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      found                   Set to a 1 if our packet is  found, else */
/*                              retruns a 0.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*      Donald R.Sharer 03-10-98        Updated for Net 4.0              */
/*                                                                       */
/*************************************************************************/

int DHCP_Process_Packets(int16 socketd, DHCP_STRUCT *ds_ptr, CHAR *dv_name, uint16 timeout)
{
    int         ret;
    int16       flen;
    int         ext_loop = 0;
    char        *inbuf;
    int16       found = 0;
    uint8       dhcp_cookie[4] = DHCP_COOKIE;
    uint32      vend_cookie;
    uint8       *pt;
    ulint       local_xid;
    DHCPLAYER   *dhcp_ptr;          /* DHCP uses a Bootp type struct */
    FD_SET      readfs, writefs, exceptfs;
    STATUS      status;
    struct addr_struct fromaddr;
    static uint8 opcode = DHCP_PAD;
    static int16 length = 0;
    static uint8 tmp[256];


    status = NU_Allocate_Memory(&System_Memory, (VOID **)&inbuf, IOSIZE,
				(UNSIGNED)NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {
	 NU_Tcp_Log_Error( DHCP_NO_MEMORY, TCP_FATAL, __FILE__, __LINE__);
	 return (NU_NO_SOCK_MEMORY);
    }

    do 
    {
	 NU_FD_Init(&readfs);
	 NU_FD_Set(socketd, &readfs);

	 ret = NU_Select(1, &readfs, &writefs, &exceptfs,
			 (timeout * TICKS_PER_SECOND));

	 if( ret == NU_NO_DATA )
		break;

	 if(NU_FD_Check(socketd, &readfs) == NU_FALSE)
		break;

	 fromaddr.family = NU_FAMILY_IP;
	 fromaddr.port = 0;
	 ret = NU_Recv_From(socketd, inbuf, IOSIZE, 0, &fromaddr, &flen);

	 if( ret < 0 )
	 {
	       NU_Tcp_Log_Error( DHCP_RECV_FAILED, TCP_FATAL,
				__FILE__, __LINE__);
	       break;
	 }

	 /* get pointer to DHCP packet. */
	 dhcp_ptr = (DHCPLAYER *)inbuf;

	 memcpy(&local_xid, &dhcp_ptr->dp_xid, sizeof(ulint) );

	 /* see if packet is the returning response to the packet I sent */
	 if( local_xid != dhcp_xid )
		continue;
		
	 /* see if the packet is an NAK type packet */
	 if( dhcp_ptr->dp_vend[6] == DHCPNAK )
		break;
		
	 /* if the callback functions are not used then accept the very
	    first packet.
	 */
         if( ds_ptr->dhcp_valfunc == 0 && ds_ptr->dhcp_optfunc == 0 )
	       found++;

         if( ds_ptr->dhcp_valfunc != 0 )
	 {
	       /* call user function to see if I should use this DHCPOFFER */
               if( ds_ptr->dhcp_valfunc(ds_ptr, dv_name, dhcp_ptr) == 0 )
		     continue;
	 }
		
         ds_ptr->dhcp_siaddr[0] = dhcp_ptr->dp_siaddr.is_ip_addrs[0];
         ds_ptr->dhcp_siaddr[1] = dhcp_ptr->dp_siaddr.is_ip_addrs[1];
         ds_ptr->dhcp_siaddr[2] = dhcp_ptr->dp_siaddr.is_ip_addrs[2];
         ds_ptr->dhcp_siaddr[3] = dhcp_ptr->dp_siaddr.is_ip_addrs[3];

         ds_ptr->dhcp_giaddr[0] = dhcp_ptr->dp_giaddr.is_ip_addrs[0];
         ds_ptr->dhcp_giaddr[1] = dhcp_ptr->dp_giaddr.is_ip_addrs[1];
         ds_ptr->dhcp_giaddr[2] = dhcp_ptr->dp_giaddr.is_ip_addrs[2];
         ds_ptr->dhcp_giaddr[3] = dhcp_ptr->dp_giaddr.is_ip_addrs[3];

         ds_ptr->dhcp_yiaddr[0] = dhcp_ptr->dp_yiaddr.is_ip_addrs[0];
         ds_ptr->dhcp_yiaddr[1] = dhcp_ptr->dp_yiaddr.is_ip_addrs[1];
         ds_ptr->dhcp_yiaddr[2] = dhcp_ptr->dp_yiaddr.is_ip_addrs[2];
         ds_ptr->dhcp_yiaddr[3] = dhcp_ptr->dp_yiaddr.is_ip_addrs[3];

         memcpy(ds_ptr->dhcp_sname, dhcp_ptr->dp_sname, sizeof(ds_ptr->dhcp_sname) );
         memcpy(ds_ptr->dhcp_file, dhcp_ptr->dp_file, sizeof(ds_ptr->dhcp_file) );
	 memcpy( &vend_cookie, dhcp_ptr->dp_vend, 4);

	 /* test to see if cookie is a vendor cookie */
	 if( vend_cookie == *(uint32 *)dhcp_cookie )
	 {
	      /* Now loop thur vendor options, passing them to user */
	      /* callback function. */
	      pt = (uint8 *)dhcp_ptr->dp_vend;
	      pt += 4;                                                        /* move past cookie */
	      for( ;*pt != DHCP_END; )
	      {
		   if( *pt == DHCP_PAD )                   /* move past PAD bytes */
		   {
			pt++;
			continue;
		   }
				
		   opcode = *pt++;                        /* save opcode */
		   length = *pt++;                        /* save length */
		   if( length < LARGEST_OPT_SIZE )
		   {
			memcpy(tmp, pt, length);
			pt += length;
		   }
		   else
		   {
			memcpy(tmp, pt, LARGEST_OPT_SIZE);
			pt += length;
			length = LARGEST_OPT_SIZE;
		   }

		   if( opcode == DHCP_SERVER_ID )
		   {
                        ds_ptr->dhcp_server_id[0] = tmp[0];
                        ds_ptr->dhcp_server_id[1] = tmp[1];
                        ds_ptr->dhcp_server_id[2] = tmp[2];
                        ds_ptr->dhcp_server_id[3] = tmp[3];
		   }

		   if( opcode == DHCP_NETMASK )
		   {
                        ds_ptr->dhcp_net_mask[0] = tmp[0];
                        ds_ptr->dhcp_net_mask[1] = tmp[1];
                        ds_ptr->dhcp_net_mask[2] = tmp[2];
                        ds_ptr->dhcp_net_mask[3] = tmp[3];
		   }

                   if( ds_ptr->dhcp_optfunc != 0 )
		   {
			/* call user callback function */
                        ret = ds_ptr->dhcp_optfunc(dv_name, opcode, length, tmp);
			switch( ret )
			{
			    case NEXT_OPTION:       /* continue, no flags set */
						   break;
			    case STOP_PROCESSING:
						   found = ACCEPT_BIT;
						   break;
			    default:                /* error, decline server's offer */
						   found = DECLINE_BIT;
						   ext_loop++;
						   break;
			}
		   }
	      }
	 }
		if( ext_loop )
			break;
    } while( found == 0 );

    NU_Deallocate_Memory(inbuf);

    return(found);
}                                               /* end DHCP_Process_Packets */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      DHCP_Process_ACK                                4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function handles processing a DHCP returned ACK packet.     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Dhcp                 Creates a DHCP request packet and sends  */
/*                              to the DHCP server to obtain an IP       */
/*                              address for the specified named device.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_TCP_Log_Error        Log a TCP error.                         */
/*      NU_FD_Init              Sets all bits in a bitmap to zero.       */
/*      NU_FD_Set               Sets a bit in a bitmap.                  */
/*      NU_Select               Check for data on multiple sockets.      */
/*      NU_FD_Check             Checks to see if a particular bit has    */
/*                              been set in a bitmap.                    */
/*      NU_Recv_From            Receive Data from a socket descriptor.   */
/*      memcpy                  Copies one section of memory to another. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      socketd                 Socket Descriptor to retrieve data from. */    
/*      ds_ptr                  Pointer to DHCP Structure that contains  */
/*                              data that can be obtained at the         */
/*                              application layer.                       */
/*      timeout                 Timeout value used for NU_Select.        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      found                   Set to a 1 if Acknowledgement found, else*/
/*                              retruns a 0.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles        11-02-1997      Created initial version 1.0   */
/*      Donald R. Sharer   03-20-1998      Modified for Net 4.0          */
/*                                                                       */
/*************************************************************************/

int DHCP_Process_ACK(int16 socketd, DHCP_STRUCT *ds_ptr, uint16 timeout)
{
    int         ret;
    int16       flen;
    int ext_loop = 0;
    char inbuf[1500];
    int16 found = 0;
    uint8 dhcp_cookie[4] = DHCP_COOKIE;
    uint32 vend_cookie;
    uint8 *pt;
    ulint local_xid;
    DHCPLAYER *inbp;                    /* DHCP uses a Bootp type struct */
    FD_SET readfs, writefs, exceptfs;
    struct addr_struct fromaddr;
    static uint8 opcode = DHCP_PAD;
    static int16 length = 0;
    static uint8 tmp[256];
    do 
    {
	 NU_FD_Init(&readfs);
	 NU_FD_Set(socketd, &readfs);

	 ret = NU_Select(1, &readfs, &writefs, &exceptfs,
			 (timeout * TICKS_PER_SECOND));

	 if( ret == NU_NO_DATA )
	       break;

	 if(NU_FD_Check(socketd, &readfs) == NU_FALSE)
	       break;

	 fromaddr.family = NU_FAMILY_IP;
	 fromaddr.port = 0;
	 ret = NU_Recv_From(socketd, inbuf, IOSIZE,0,&fromaddr, &flen);

	 if( ret < 0 )
	 {
	       NU_Tcp_Log_Error( DHCP_RECV_FAILED, TCP_FATAL,
				 __FILE__, __LINE__);
	       break;
	 }

	 /* get pointer to DHCP packet. */
	 inbp = (DHCPLAYER *)inbuf;

	 memcpy(&local_xid, &inbp->dp_xid, sizeof(ulint) );

	 /* see if packet is the returning response to the packet I sent */
	 if( local_xid != dhcp_xid )
	       continue;

	 memcpy( &vend_cookie, inbp->dp_vend, 4);

	 /* test to see if cookie is a vendor cookie */
	 if( vend_cookie == *(uint32 *)dhcp_cookie &&
             ds_ptr->dhcp_optfunc != 0 )
	 {
	       /* Now loop thur vendor options, passing them to user call */
	       /* back function. */
	       pt = (uint8 *)inbp->dp_vend;
	       pt += 4;                                                        /* move past cookie */
	       for( ;*pt != DHCP_END; )
	       {
		    if( *pt == DHCP_PAD )          /* move past PAD bytes */
		    {
			 pt++;
			 continue;
		    }
				
		    opcode = *pt++;                /* save opcode */
		    length = *pt++;                                 /* save length */
		    if( length < LARGEST_OPT_SIZE )
		    {
			 memcpy(tmp, pt, length);
			 pt += length;
		    }
		    else
		    {
			 memcpy(tmp, pt, LARGEST_OPT_SIZE);
			 pt += length;
			 length = LARGEST_OPT_SIZE;
		    }

		    if( opcode == DHCP_MSG_TYPE )
		    {
			 if( tmp[0] == DHCPACK )
			 {
			      found = 1;
			      break;
			 }
			 else
			 {
			      ext_loop++;
			      found = 2;
			      break;
			 }
		    }
	       }
	 }
	 if( ext_loop )
	      break;
    } while( found == 0 );

    return(found);
}                                               /* end DHCP_Process_ACK */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      DHCP_init                                       4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function handles the initing of the DHCP packet.            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Dhcp                 Creates a DHCP request packet and sends  */
/*                              to the DHCP server to obtain an IP       */
/*                              address for the specified named device.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      memset                  Set memory to a particular value.        */
/*      memcpy                  Copies one section of memory to another. */
/*      strlen                  Returns the length of the specified      */
/*                              string.                                  */
/*      n_clicks                Retrives number of clock ticks.          */
/*      memcmp                  Compares memory at two addresses of a    */
/*                              specified length.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pkt                     Pointer to the DHCP layer packet.        */
/*      ds_ptr                  Pointer to DHCP Structure that contains  */
/*                              data that can be obtained at the         */
/*                              application layer.                       */
/*      dv_name                 Pointer to Devices name.                 */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      len                     The Length of the DHCP packet.           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/

static uint16 DHCP_init(DHCPLAYER *pkt, DHCP_STRUCT *ds_ptr, CHAR *dv_name)
{
    int32 i, len = 0;
    uint8 rfc1048_cookie[4] = DHCP_COOKIE;
    uint8 *pt;

    dhcp_xid = (ulint)n_clicks();
    memset(pkt, 0, sizeof(DHCPLAYER) );

    pkt->dp_op      = BOOTREQUEST;          /* opcode, 1 octet */
    pkt->dp_htype   = HARDWARE_TYPE;        /* hardware type, 1 octet */
    pkt->dp_hlen    = DADDLEN;                      /* hardware address length, 1 octet */
    pkt->dp_xid     = dhcp_xid;
    pkt->dp_secs    = 0;
    pkt->dp_flags   = 0x0001;                       /* set broadcast flag. */
	
    memcpy(pkt->dp_chaddr, ds_ptr->dhcp_mac_addr, MACSIZE);

    len = sizeof(pkt->dp_op) +
	  sizeof(pkt->dp_htype) +
	  sizeof(pkt->dp_hlen) +
	  sizeof(pkt->dp_hops) +
	  sizeof(pkt->dp_xid) +
	  sizeof(pkt->dp_secs) +
	  sizeof(pkt->dp_flags) +
	  sizeof(pkt->dp_ciaddr) +
	  sizeof(pkt->dp_yiaddr) +
	  sizeof(pkt->dp_siaddr) +
	  sizeof(pkt->dp_giaddr) +
	  sizeof(pkt->dp_chaddr) +
	  sizeof(pkt->dp_sname) +
	  sizeof(pkt->dp_file);
	
    pt = &pkt->dp_vend[0];
    *pt++ = rfc1048_cookie[0];
    *pt++ = rfc1048_cookie[1];
    *pt++ = rfc1048_cookie[2];
    *pt++ = rfc1048_cookie[3];
    *pt++ = DHCP_MSG_TYPE;
    *pt++ = 1;
    *pt++ = DHCPDISCOVER;
    *pt++ = DHCP_CLIENT_CLASS_ID;
    *pt++ = 7;
    *pt++ = HARDWARE_TYPE;
    *pt++ = ds_ptr->dhcp_mac_addr[0];
    *pt++ = ds_ptr->dhcp_mac_addr[1];
    *pt++ = ds_ptr->dhcp_mac_addr[2];
    *pt++ = ds_ptr->dhcp_mac_addr[3];
    *pt++ = ds_ptr->dhcp_mac_addr[4];
    *pt++ = ds_ptr->dhcp_mac_addr[5];
    *pt++ = DHCP_REQUEST_IP;
    *pt++ = 4;
    len += 18;

    /* if the ip_addr is null then use a broadcast address */
    if( memcmp(ds_ptr->dhcp_ip_addr, "\0\0\0\0", IP_ADDR_LEN) == 0 )
    {
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
    }
    else
    {
         *pt++ = ds_ptr->dhcp_ip_addr[0];
         *pt++ = ds_ptr->dhcp_ip_addr[1];
         *pt++ = ds_ptr->dhcp_ip_addr[2];
         *pt++ = ds_ptr->dhcp_ip_addr[3];
    }
    len += 4;
    i = strlen(dv_name);
    if( i > 0 )
    {
	 len += i;
	 *pt++ = DHCP_HOSTNAME;
	 *pt++ = (uint8)i;
         memcpy(pt, dv_name, i);
	 pt += i;
    }
    if( ds_ptr->dhcp_opts )
    {
         memcpy(pt, ds_ptr->dhcp_opts, ds_ptr->dhcp_opts_len);
         pt += ds_ptr->dhcp_opts_len;
         len += ds_ptr->dhcp_opts_len;
    }
    *pt++ = DHCP_END;
    len++;

    return((uint16)len);
}                                               /* end DHCP_init */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      DHCP_request                                    4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a DHCPREQUEST packet.                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Dhcp                 Creates a DHCP request packet and sends  */
/*                              to the DHCP server to obtain an IP       */
/*                              address for the specified named device.  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      n_clicks                Retrives number of clock ticks.          */
/*      memcmp                  Compares memory at two addresses of a    */
/*                              specified length.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pkt                     Pointer to the DHCP layer packet.        */
/*      ds_ptr                  Pointer to DHCP Structure that contains  */
/*                              data that can be obtained at the         */
/*                              application layer.                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      len                     The Length of the DHCP Request packet.   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*      Donald R. Sharer 3-20-1998      Modified for Net 4.0             */
/*                                                                       */
/*************************************************************************/

static uint16 DHCP_request(DHCPLAYER *pkt, DHCP_STRUCT *ds_ptr)
{
    uint16 len = 0;
    uint8 rfc1048_cookie[4] = DHCP_COOKIE;
    uint8 *pt;

    dhcp_xid = (ulint)n_clicks();

    pkt->dp_op      = BOOTREQUEST;          /* opcode, 1 octet */
    pkt->dp_htype   = HARDWARE_TYPE;        /* hardware type, 1 octet */
    pkt->dp_hlen    = DADDLEN;              /* hardware address length, 1 octet */
    pkt->dp_xid     = dhcp_xid;
    pkt->dp_secs    = 0;
    pkt->dp_flags   = 0x0001;                       /* set broadcast flag. */

    len = sizeof(pkt->dp_op) +
	  sizeof(pkt->dp_htype) +
	  sizeof(pkt->dp_hlen) +
	  sizeof(pkt->dp_hops) +
	  sizeof(pkt->dp_xid) +
	  sizeof(pkt->dp_secs) +
	  sizeof(pkt->dp_flags) +
	  sizeof(pkt->dp_ciaddr) +
	  sizeof(pkt->dp_yiaddr) +
	  sizeof(pkt->dp_siaddr) +
	  sizeof(pkt->dp_giaddr) +
	  sizeof(pkt->dp_chaddr) +
	  sizeof(pkt->dp_sname) +
	  sizeof(pkt->dp_file);
       
    /* memcpy(pkt->dp_chaddr, mac, MACSIZE); */
	
    pt = &pkt->dp_vend[0];
    *pt++ = rfc1048_cookie[0];
    *pt++ = rfc1048_cookie[1];
    *pt++ = rfc1048_cookie[2];
    *pt++ = rfc1048_cookie[3];
    *pt++ = DHCP_MSG_TYPE;
    *pt++ = 1;
    *pt++ = DHCPREQUEST;
    *pt++ = DHCP_CLIENT_CLASS_ID;
    *pt++ = 7;
    *pt++ = HARDWARE_TYPE;
    *pt++ = ds_ptr->dhcp_mac_addr[0];
    *pt++ = ds_ptr->dhcp_mac_addr[1];
    *pt++ = ds_ptr->dhcp_mac_addr[2];
    *pt++ = ds_ptr->dhcp_mac_addr[3];
    *pt++ = ds_ptr->dhcp_mac_addr[4];
    *pt++ = ds_ptr->dhcp_mac_addr[5];
    *pt++ = DHCP_REQUEST_IP;
    *pt++ = 4;
    len +=  18;
    if( memcmp(ds_ptr->dhcp_yiaddr, "\0\0\0\0", IP_ADDR_LEN) == 0 )
    {
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
    }
    else
    {
         *pt++ = ds_ptr->dhcp_yiaddr[0];
         *pt++ = ds_ptr->dhcp_yiaddr[1];
         *pt++ = ds_ptr->dhcp_yiaddr[2];
         *pt++ = ds_ptr->dhcp_yiaddr[3];
    }
    len += 4;
    /* tell all servers that responded to the DHCPDISCOVER broadcast */
    /* that the following server has been selected. */
    *pt++ = DHCP_SERVER_ID;
    *pt++ = 4;
    *pt++ = ds_ptr->dhcp_server_id[0];
    *pt++ = ds_ptr->dhcp_server_id[1];
    *pt++ = ds_ptr->dhcp_server_id[2];
    *pt++ = ds_ptr->dhcp_server_id[3];
    *pt++ = DHCP_END;
    len += 7;

    return(len);
}                                               /* end DHCP_request */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      DHCP_release                                    4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a DHCPRELEASE packet.                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Kelly Wiles, Xact Inc.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*     NU_Dhcp_Release          Releases the address obtained by the DHCP*/
/*                              process.                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*    memcmp                    Compares memory at two addresses of a    */
/*                              specified length.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pkt                     Pointer to the DHCP layer packet.        */
/*      ds_ptr                  Pointer to DHCP Structure that contains  */
/*                              data that can be obtained at the         */
/*                              application layer.                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      len                     The Length of the DHCP release packet.   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Kelly Wiles     11-02-1997      Created initial version 1.0      */
/*      Donald R. Sharer 3-20-1998      Modified for Net 4.0             */
/*                                                                       */
/*************************************************************************/

static uint16 DHCP_release(DHCPLAYER *pkt, DHCP_STRUCT *ds_ptr)
{
    uint16 len = 0;
    uint8 rfc1048_cookie[4] = DHCP_COOKIE;
    uint8 *pt;

    dhcp_xid = (ulint)n_clicks();

    pkt->dp_op              = BOOTREQUEST;          /* opcode, 1 octet */
    pkt->dp_htype   = HARDWARE_TYPE;        /* hardware type, 1 octet */
    pkt->dp_hlen    = DADDLEN;                      /* hardware address length, 1 octet */
    pkt->dp_xid     = dhcp_xid;
    pkt->dp_secs    = 0;
    pkt->dp_flags   = 0x0001;                       /* set broadcast flag. */

    len = sizeof(pkt->dp_op) +
	  sizeof(pkt->dp_htype) +
	  sizeof(pkt->dp_hlen) +
	  sizeof(pkt->dp_hops) +
	  sizeof(pkt->dp_xid) +
	  sizeof(pkt->dp_secs) +
	  sizeof(pkt->dp_flags) +
	  sizeof(pkt->dp_ciaddr) +
	  sizeof(pkt->dp_yiaddr) +
	  sizeof(pkt->dp_siaddr) +
	  sizeof(pkt->dp_giaddr) +
	  sizeof(pkt->dp_chaddr) +
	  sizeof(pkt->dp_sname) +
	  sizeof(pkt->dp_file);
	
    pt = &pkt->dp_vend[0];
    *pt++ = rfc1048_cookie[0];
    *pt++ = rfc1048_cookie[1];
    *pt++ = rfc1048_cookie[2];
    *pt++ = rfc1048_cookie[3];
    *pt++ = DHCP_MSG_TYPE;
    *pt++ = 1;
    *pt++ = DHCPRELEASE;
    *pt++ = DHCP_CLIENT_CLASS_ID;
    *pt++ = 7;
    *pt++ = HARDWARE_TYPE;
    *pt++ = ds_ptr->dhcp_mac_addr[0];
    *pt++ = ds_ptr->dhcp_mac_addr[1];
    *pt++ = ds_ptr->dhcp_mac_addr[2];
    *pt++ = ds_ptr->dhcp_mac_addr[3];
    *pt++ = ds_ptr->dhcp_mac_addr[4];
    *pt++ = ds_ptr->dhcp_mac_addr[5];
    *pt++ = DHCP_REQUEST_IP;
    *pt++ = 4;
    len += 18;
    if( memcmp(ds_ptr->dhcp_yiaddr, "\0\0\0\0", IP_ADDR_LEN) == 0 )
    {
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
	 *pt++ = 0x0;
    }
    else
    {
         *pt++ = ds_ptr->dhcp_yiaddr[0];
         *pt++ = ds_ptr->dhcp_yiaddr[1];
         *pt++ = ds_ptr->dhcp_yiaddr[2];
         *pt++ = ds_ptr->dhcp_yiaddr[3];
    }
    *pt++ = DHCP_END;
    len += 5;
    return(len);
}                                               /* end DHCP_release */

/***************************************************************************/
/* FUNCTION                                           VERSION              */
/*                                                                         */
/*  dhcp_Validate                                       4.0                */
/*                                                                         */
/* DESCRIPTION                                                             */
/*                                                                         */
/*  Function decides if a DHCPOFFER packet should be accepted.             */
/*                                                                         */
/* AUTHOR                                                                  */
/*                                                                         */
/*  Kelly Wiles, Xact Inc.                                                 */
/*                                                                         */
/* CALLED BY                                                               */
/*                                                                         */
/*    DHCP_Process_Packets      Processes incoming DHCP packets.           */
/*                                                                         */
/* CALLS                                                                   */
/*                                                                         */
/*    memcmp                    Compares memory at two addresses of a      */
/*                              specified length.                          */
/*                                                                         */
/* INPUTS                                                                  */
/*                                                                         */
/*    ds_ptr                    DHCP struct pointer.                       */
/*    dv_name                   Device name  of the current network card.  */
/*    dp                        DHCPLAYER structure pointer of current     */
/*                              DHCPOFFER packet.                          */
/*                                                                         */
/* OUTPUTS                                                                 */
/*                                                                         */
/*   If TRUE is returned by this function then NU_Dhcp function will accept*/
/*   the current DHCPOFFER packet, otherwise FALSE will be returned to the */
/*   NU_Dhcp function.                                                     */
/*                                                                         */
/* HISTORY                                                                 */
/*                                                                         */
/*      NAME                            DATE            REMARKS            */
/*                                                                         */
/*      Kelly Wiles         11/02/97    Initial version.                   */
/*      Donald R. Sharer    3-20-1998   Modified for Net 4.0               */
/*                                                                         */
/***************************************************************************/

#if(DHCP_VALIDATE_CALLBACK)

int dhcp_Validate(DHCP_STRUCT *ds_ptr, CHAR *dv_name, const DHCPLAYER *dp)
{
    uint8 your_subnet[3] = {204, 181, 204};
    uint8 serv_subnet[3] = {204, 181, 204};

    /* stops compiler from complaining about unused varaibles. */
    if( ds_ptr == 0 || dv == 0 || dp == 0 )
	 return(FALSE);

    /* Other items could be checked here but the user would have to */
    /* parse the vendor options field of the DHCPOFFER packet.  See */
    /* RFC 2132. */
    /*** Note: As of RFC 2132 the vendor option field is now variable ***/
    /*** length and not 64 bytes in size. Your must look for a DHCP_END. **/

    if( memcmp(your_subnet, dp->dp_yiaddr.is_ip_addrs, 3) == 0 )
	 return(TRUE);
    else
	 return(FALSE);
}
#endif

/**************************************************************************/
/* FUNCTION                                           VERSION             */
/*                                                                        */
/*  dhcp_Vendor_Options                                 4.0               */
/*                                                                        */
/* DESCRIPTION                                                            */
/*                                                                        */
/*  Function handles all vendor options returned by the DHCP server.      */
/*                                                                        */
/* AUTHOR                                                                 */
/*                                                                        */
/*  Kelly Wiles, Xact Inc.                                                */
/*                                                                        */
/* CALLED BY                                                              */
/*                                                                        */
/*    DHCP_Process_Packets      Processes incoming DHCP packets.          */
/*                                                                        */
/* CALLS                                                                  */
/*                                                                        */
/*    None.                                                               */
/*                                                                        */
/* INPUTS                                                                 */
/*                                                                        */
/*    dv_name                   Device name  of the current network card. */
/*    opc                       Opcode for the vendor option.             */
/*    len                       Length of vendor option in bytes.         */
/*    val                       Pointer to vendor option value.           */
/*                                                                        */
/* OUTPUTS                                                                */
/*                                                                        */
/*   If TRUE is returned by this function then NU_Dhcp function will stop */
/*   processing vendor options, otherwise -1 is returned.                 */
/*                                                                        */
/* HISTORY                                                                */
/*                                                                        */
/*      NAME                            DATE            REMARKS           */
/*                                                                        */
/*  Kelly Wiles         11/02/97    Initial version.                      */
/*  Donald R. Sharer    3/20/98     Modified for Net 4.0                  */
/*                                                                        */
/**************************************************************************/

#if(DHCP_VENDOR_OPTS_CALLBACK)

int dhcp_Vendor_Options( CHAR *dv_name, uint8 opc, uint16 len, uint8 *val)
{
    int i, ret = 0;

    /* if device name is not defined then return, telling NU_Dhcp to */
    /* stop processing vendor oprions. */
    if( dv_name == 0 || dv_name[0] == '\0' )
	  return(-1);

    /* As of RFC2131 and RFC2132 */
    /* This switch contains all opcodes known as of the above RFC's. */

    /* none or all opcodes could be returned by the DHCP server. */

    switch( opc )
    {
	  case DHCP_NETMASK:
				 break;
	  case DHCP_TIME_OFFSET:
				 break;
	  case DHCP_ROUTE:
				 break;
	  case DHCP_TIME:
				 break;
	  case DHCP_NAME_SERVER:
				 break;
	  case DHCP_DNS:
				 break;
	  case DHCP_LOG_SERVER:
				 break;
	  case DHCP_COOKIE_SERVER:
				 break;
	  case DHCP_LPR_SERVER:
				 break;
	  case DHCP_IMPRESS_SERVER:
				 break;
	  case DHCP_RESOURCE_SERVER:
				 break;
	  case DHCP_HOSTNAME:
				 break;
	  case DHCP_BOOT_FILE_SIZE:
				 break;
	  case DHCP_MERIT_DUMP_FILE:
				 break;
	  case DHCP_DOMAIN_NAME:
				 break;
	  case DHCP_SWAP_SERVER:
				 break;
	  case DHCP_ROOT_PATH:
				 break;
	  case DHCP_EXTENSIONS_PATH:
				 break;
				  /* IP Layer Parameters per Host. */
	  case DHCP_IP_FORWARDING:
				 break;
	  case DHCP_NL_SOURCE_ROUTING:
				 break;
	  case DHCP_POLICY_FILTER:
				 break;
	  case DHCP_MAX_DARAGRAM_SIZE:
				 break;
	  case DHCP_IP_TIME_TO_LIVE:
				 break;
	  case DHCP_MTU_AGING_TIMEOUT:
				 break;
	  case DHCP_MTU_PLATEAU_TABLE:
				 break;
				/* IP Layer Parameters per Interface. */
	  case DHCP_INTERFACE_MTU:
				 break;
	  case DHCP_ALL_SUBNETS:
				 break;
	  case DHCP_BROADCAST_ADDR:
				 break;
	  case DHCP_MASK_DISCOVERY:
				 break;
	  case DHCP_MASK_SUPPLIER:
				 break;
	  case DHCP_ROUTER_DISCOVERY:
				 break;
	  case DHCP_ROUTER_SOLICI_ADDR:
				 break;
	  case DHCP_STATIC_ROUTE:
				 break;
				/* Link Layer Parameters per Interface. */
	  case DHCP_TRAILER_ENCAP:
				 break;
	  case DHCP_ARP_CACHE_TIMEOUT:
				 break;
	  case DHCP_ETHERNET_ENCAP:
				 break;
			      /* TCP Parameters. */
	  case DHCP_TCP_DEFAULT_TTL:
				 break;
	  case DHCP_TCP_KEEPALIVE_TIME:
				 break;
	  case DHCP_TCP_KEEPALIVE_GARB:
				 break;
			     /* Application and Service Parameters. */
	  case DHCP_NIS_DOMAIN:
				 break;
	  case DHCP_NIS:
				 break;
	  case DHCP_NTP_SERVERS:
				 break;
	  case DHCP_VENDOR_SPECIFIC:
				 break;
	  case DHCP_NetBIOS_NAME_SER:
				 break;
	  case DHCP_NetBIOS_DATA_SER:
				 break;
	  case DHCP_NetBIOS_NODE_TYPE:
				 break;
	  case DHCP_NetBIOS_SCOPE:
				 break;
	  case DHCP_X11_FONT_SERVER:
				 break;
	  case DHCP_X11_DISPLAY_MGR:
				 break;

	  case DHCP_NIS_PLUS_DOMAIN:
				 break;
	  case DHCP_NIS_PLUS_SERVERS:
				 break;
	  case DHCP_MOBILE_IP_HOME:
				 break;
	  case DHCP_SMTP_SERVER:
				 break;
	  case DHCP_POP3_SERVER:
				 break;
	  case DHCP_NNTP_SERVER:
				 break;
	  case DHCP_WWW_SERVER:
				 break;
	  case DHCP_FINGER_SERVER:
				 break;
	  case DHCP_IRC_SERVER:
				 break;
	  case DHCP_STREETTALK_SERVER:
				 break;
	  case DHCP_STDA_SERVER:
				 break;
			   /* DHCP Extensions */
	  case DHCP_REQUEST_IP:
				 break;
	  case DHCP_IP_LEASE_TIME:
				 break;
	  case DHCP_OVERLOAD:
				 break;
	  case DHCP_MSG_TYPE:
				 break;
	  case DHCP_SERVER_ID:
				 ret = ACCEPT_BIT;
				 break;
	  case DHCP_REQUEST_LIST:
				 break;
	  case DHCP_MESSAGE:
				 break;
	  case DHCP_MAX_MSG_SIZE:
				 break;
	  case DHCP_RENEWAL_T1:
				 break;
	  case DHCP_REBINDING_T2:
				 break;
	  case DHCP_VENDOR_CLASS_ID:
				 break;
	  case DHCP_CLIENT_CLASS_ID:
				 break;
	  default:
				 return(-1);
	}

	return(ret);
}
#endif
#define NRAND_MAX   32767
/**************************************************************************/
/* FUNCTION                                           VERSION             */
/*                                                                        */
/*  NU_rand                                             4.0               */
/*                                                                        */
/* DESCRIPTION                                                            */
/*                                                                        */
/*  Function handles generating a random number.                          */
/*                                                                        */
/* AUTHOR                                                                 */
/*                                                                        */
/*  Accelerated Technology Inc.                                           */
/*                                                                        */
/* CALLED BY                                                              */
/*                                                                        */
/*    NU_Dhcp                                  Processes DHCP protocol.   */
/*    NU_DHCP_Release                          Releases DHCP IP address.  */
/*    NU_Bootp                                 Processes BOOTP protocol.  */
/*                                                                        */
/* CALLS                                                                  */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/* INPUTS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/* OUTPUTS                                                                */
/*                                                                        */
/*    Returns the Random Number.                                          */
/*                                                                        */
/* HISTORY                                                                */
/*                                                                        */
/*      NAME                            DATE            REMARKS           */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

int16 NU_rand(void)
{
	next = next * 1103515245 + 12345;
	return (uint)(next/65536) % (((uint) NRAND_MAX)+1); 
}        
