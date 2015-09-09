/****************************************************************************/
/*                                                                          */
/*    CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.               */
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
/*  Bootp                                                      4.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This file will contain all the BOOTP routines.                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Accelerated Technology Inc.                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      NU_Bootp                        Processes BOOTP function.           */
/*      BOOTP_init                      Initializes the BOOTP Request       */                                                                                                       
/*                                      Packet.                             */
/*      BOOTP_Process_Packet            Processes the BOOTP Reply Packet.   */              
/*      NU_rand                         Produces a random timeout delay.    */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      BOOTP.H                         Holds the defines necessary for     */  
/*                                      BOOTp to function properly.         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Craig L. Meredith       04/10/93        Initial version.            */
/*  Maiqi Qian      12/06/96        Fixed the time wrap around (spr0229)    */
/*  Maiqi Qian      12/12/96        Added a buffer for receiving reply.     */
/*  Don Sharer      03/10/98        Added Net 4.0 Implementation.           */
/*                                                                          */
/****************************************************************************/

#ifdef PLUS
  #include "nucleus.h"
#else
  #include "nu_defs.h"
  #include "nu_extr.h"
#endif
#include "protocol.h"
#include "socketd.h"
#include "data.h"
#include "bootp.h"
#include "externs.h"
#include "data.h"
#include "tcp_errs.h"
#include "target.h"
#include "tcp.h"
#include "net_extr.h"
#include "dhcp.h"    
#include "rtab.h"
#include "rip2.h"

static ulint bootp_xid;

/* Local functions */




static uint16 BOOTP_init (BOOTPLAYER *pkt, BOOTP_STRUCT  *out_bp);
int BOOTP_Process_Packets(int16 socketd, BOOTP_STRUCT *bp, uint16 timeout);
extern int16  NU_rand(void);

/* This list is used to hold on to the BOOTP request that will be sent. By using
   this list the need to build the request each time it is transmitted is
   avoided. Instead the request is built once and then reused if
   retransmissions are necessary. */
NET_BUFFER_HEADER BOOTP_List;

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_Bootp                                    Version 4.0                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This routine will do the main processing of bootp lookup request    */
/*      calls.  The function will build the entire packet from the mac layer*/ 
/*      to the BOOTP layer. It will send a bootp request packet, and process*/      
/*      the BOOTP REPLY that is sent from the BOOTP Server.                 */
/*      The code will also handle retries.                                  */    
/*                                                                          */    
/* AUTHOR                                                                   */
/*                                                                          */                                                                            
/*      Donald R. Sharer, Accelerated Technology Inc.                       */    
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      Main application Function that is written by the user.              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      Bootp_init                     Initializes BOOTP request Packet.    */
/*      DEV_Init_Devices               Initailizes Devices.                 */
/*      NU_Socket                      Creates a Socket.                    */
/*      NU_TCP_Log_Error               Logs Errors.                         */
/*      NU_Bind                        Binds a Socket.                      */
/*      NU_Close_Socket                Closes a Socket.                     */
/*      UTL_Zero                       Zeroes out a Variable.               */
/*      DEV_Get_DEV_By_Name            Gets a Device structure by name.     */
/*      memcpy                         Copies a specified memory location   */
/*                                     to another.                          */
/*      memcmp                         Compares a specified memory location */
/*                                     with another.                        */
/*      MEM_Buffer_Chain_Dequeue       Dequeues any incoming buffers.       */
/*      intswap                        Swaps high low byte of a word.       */
/*      tcpcheck                       Calculates checksum for UDP/TCP      */
/*                                     packets.                             */
/*      ipcheck                        Calculates the checksum of the IP    */
/*                                     packet.                              */
/*      DEV_Attach_IP_to_Device        Attaches the IP address to a         */
/*                                     particular device.                   */
/*      NU_rand                        Random Number Generator.             */
/*      comparen                       Compares two values of a specified   */
/*                                     length.                              */
/*      BOOTP_Process_Packet           Processes the Incoming BOOTP Reply.  */
/*      *(int_face->dev_start)         Transmits Packet.                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*      dv_name                        Pointer to Devices name.             */
/*      bp_ptr                         Pointer to BOOTP structure.          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      retval                         Retruns 0 if successful. Returns     */
/*                                     value less than 1 if unsuccessful.   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Donald R. Sharer                03/11/98        Net 40 version.     */
/*                                                                          */
/****************************************************************************/
STATUS NU_Bootp ( CHAR  *dv_name, BOOTP_STRUCT *bp_ptr)
{
    uint16              delay, delay_mask = 3;
    int                 i;
    int                 ret;
    BOOTPLAYER          *bootp_ptr;
    IPLAYER             *ip_ptr;
    UDPLAYER            *udp_pkt;
    NET_BUFFER          *buf_ptr;
    NET_BUFFER          *next_buf=0;
    struct pseudotcp    tcp_chk;
    int16               failed = 0,retval=0;
    DV_DEVICE_ENTRY     *int_face=0;
    int16               socketd;
    uint8               *buffer;
    struct addr_struct  clientaddr;
    uint16              nbytes;
    STATUS              status;
    int16               hlen = sizeof (IPLAYER);
    INT                 length;
    int32               total_size, current_size;
    int                 found = 0;    
    int32               flags;
    SCK_SOCKADDR        sa;
    int32               temp_data_len,temp_total_data_len;
    DLAYER              *ether_header;

	/* Create a socket and bind to it, so that we can receive packets. */
    socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, NU_NONE);
    if( socketd < 0 )
    {
        NU_Tcp_Log_Error( BOOTP_SOCKET, TCP_FATAL, __FILE__, __LINE__);
        return (NU_BOOTP_INIT_FAILED);
    }

    /* build local address and port to bind to. */
    clientaddr.family = NU_FAMILY_IP;
    clientaddr.port   = IPPORT_BOOTPC;
    clientaddr.id.is_ip_addrs[0] = (uint8) 0;
    clientaddr.id.is_ip_addrs[1] = (uint8) 0;
    clientaddr.id.is_ip_addrs[2] = (uint8) 0;
    clientaddr.id.is_ip_addrs[3] = (uint8) 0;
    clientaddr.name = "ATI";

    /*  Allocate memory for the buffer space */
    status = NU_Allocate_Memory(&System_Memory,(VOID **) &buffer,sizeof(BOOTPLAYER), NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
            return (NU_BOOTP_INIT_FAILED);


    ret = NU_Bind(socketd, &clientaddr, 0);
    if( ret != socketd )
    {
        NU_Tcp_Log_Error( BOOTP_SOCKET_BIND, TCP_FATAL,
                    __FILE__, __LINE__);
        NU_Close_Socket(socketd);
        return (NU_BOOTP_INIT_FAILED);
    }

    UTL_Zero(bp_ptr->bp_mac_addr, sizeof(bp_ptr->bp_mac_addr));
    UTL_Zero(bp_ptr->bp_siaddr,sizeof(bp_ptr->bp_siaddr));
    UTL_Zero(bp_ptr->bp_giaddr,sizeof(bp_ptr->bp_giaddr));
    UTL_Zero(bp_ptr->bp_sname,sizeof(bp_ptr->bp_sname));
    UTL_Zero(bp_ptr->bp_file,sizeof(bp_ptr->bp_file));
	
    /*  Get the device by name to be used on BOOTP for this iteration. */
 
    int_face = DEV_Get_Dev_By_Name(dv_name);

    if (int_face->dev_mtu < BOOTP_MAX_HEADER_SIZE)
        return (NU_BOOTP_INIT_FAILED);

    /* copy the hardware address from the device structure */
    memcpy(bp_ptr->bp_mac_addr, int_face->dev_mac_addr, DADDLEN);

    /*  make sure that the MAC address is a good.  */
    if ( memcmp(bp_ptr->bp_mac_addr, "\0\0\0\0\0\0", 6) == 0)
        return (NU_BOOTP_INIT_FAILED);

    buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, BOOTP_MAX_HEADER_SIZE);

    if (buf_ptr == NU_NULL)
        return (NU_NO_BUFFERS);

    buf_ptr->mem_dlist = &BOOTP_List;


    UTL_Zero(buf_ptr->mem_parent_packet, sizeof(NET_PARENT_BUFFER_SIZE));
    
    
    if (buf_ptr->next_buffer != NU_NULL)
    {
        next_buf= buf_ptr->next_buffer;
    }
    
     UTL_Zero(next_buf->mem_packet, sizeof(NET_MAX_BUFFER_SIZE));





    /* Set up pointers to each of the headers that make up a BOOTP Packet.  */
    bootp_ptr = (BOOTPLAYER *)(buf_ptr->mem_parent_packet + (BOOTP_MAX_HEADER_SIZE - (sizeof(BOOTPLAYER))));
    udp_pkt  = (UDPLAYER *) (((char *)bootp_ptr) - sizeof(UDPLAYER));
    ip_ptr   = (IPLAYER *) (((char *)udp_pkt) - sizeof(IPLAYER));

    /*  Build the BOOTP Request Packet  */

    nbytes = BOOTP_init((BOOTPLAYER *)buffer, bp_ptr);

    /*  Initialize the local and foreign port numbers  */

    udp_pkt->source = intswap(IPPORT_BOOTPC);
    udp_pkt->dest   = intswap(IPPORT_BOOTPS);

    /* Set up the UDP Header  */

    udp_pkt->length = intswap ((uint16) (nbytes + sizeof(UDPLAYER)));

    udp_pkt->check = 0;



    buf_ptr->data_ptr = buf_ptr->mem_parent_packet + NET_MAX_UDP_HEADER_SIZE;
    total_size = nbytes;

    /*  Chain the Bootp Request Packet */

    if (total_size > NET_PARENT_BUFFER_SIZE)
    {
       current_size = NET_PARENT_BUFFER_SIZE - sizeof(IPLAYER) - NET_ETHER_HEADER_OFFSET_SIZE;
       total_size =   total_size - current_size;
    }
    else
    {
       current_size =  total_size;
    }

    /*  Copy Bootp Packet into first Buffer */
    memcpy(buf_ptr->data_ptr , buffer, current_size);

    /*  Set the Data Length to the Size of bytes copied.  */
    buf_ptr->data_len = current_size;

    /*  Set the Total data length to the Number of bytes in a Bootp Packet. */
    buf_ptr->mem_total_data_len = nbytes;

    /*  Increment Bootp Buffer to be at the number of bytes copied.  */
    buffer = buffer + current_size;

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


   /*  Set up the IP header  */
    ip_ptr->versionandhdrlen = (((hlen >> 2)) | (IP_VERSION << 4));

    /*  Set the IP header to no fragments. */

    ip_ptr->frags = 0;

    /* Set the IP packet ID.  */

    ip_ptr->ident = 0;

    /* Set the type of service. */

    ip_ptr->service = 0;

    length = nbytes +(uint16)sizeof(UDPLAYER);

    /* Set the total length( data and IP header) for this packet. */

    ip_ptr->tlen = intswap((int16)(length + hlen));

    /*  Set the time to live. */

    ip_ptr->ttl = IP_TIME_TO_LIVE;

    /*  Set the protocol. */

    ip_ptr->protocol = IP_UDP_PROT;


    /* We are doing a broadcast, so we do not need this fields. */

    memset(ip_ptr->ipsource,0,4);
    memcpy(ip_ptr->ipdest,"\377\377\377\377",4);


    /*  Compute the IP checksum. */
    ip_ptr->check = 0;
    ip_ptr->check = ipcheck((uint16 *)ip_ptr, (uint16)(hlen >> 1));

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

    /*  Set initial Delay for Processing packets.  */

    delay= (delay_mask & NU_rand()) + 1;

    /*  Innitialize the ethernet header.  */

    ether_header = (DLAYER *)sa.sck_data;
    memcpy(ether_header->dest, NET_Ether_Broadaddr, DADDLEN);
    memcpy(ether_header->me, bp_ptr->bp_mac_addr, DADDLEN);
    ether_header->type = EIP;

    sa.sck_family = SK_FAM_UNSPEC;
    sa.sck_len = sizeof(sa);

    /*  Transmit the packet  */

    for(i=0; i< BOOTP_RETRIES; i++)
    {
        
        /* Grab the semaphore because we are about to change the interface
           that the BOOTP request will be sent over. */
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

        if( status != NU_SUCCESS)
        {
            NU_Tcp_Log_Error(BOOTP_SEND_FAILED, TCP_FATAL, __FILE__,__LINE__);
            break;
        }

        /*  Look at each packet on the buffer list to see if it is my reply */
        found = BOOTP_Process_Packets(socketd, bp_ptr, delay);

        if (found)
            break;                           /*  Found the Packet. */

        delay_mask++;


        delay = (delay_mask & NU_rand()) + 1;

        /*  Get the the packet off the Trasmitted list and retrasnmit again. */
        buf_ptr= BOOTP_List.head;
        buf_ptr->data_ptr = BOOTP_List.head->mem_parent_packet + NET_ETHER_HEADER_OFFSET_SIZE;
        buf_ptr->data_len = temp_data_len;
        buf_ptr->mem_total_data_len = temp_total_data_len;
    }  /*  End For Loop */


    if( found == 1 )
    {
        status = DEV_Attach_IP_To_Device( dv_name, bp_ptr->bp_yiaddr,
                                          bp_ptr->bp_net_mask);
        if( status != NU_SUCCESS )
        {
            NU_Tcp_Log_Error( BOOTP_ATTACH_IP, TCP_FATAL,
                  __FILE__, __LINE__);
            failed++;
        }

    }
	
    retval= failed + retval;
	   
    /* Cleanup */
    NU_Close_Socket(socketd);

    /*  Free the Buffer Chain.  */
    MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    NU_Deallocate_Memory(buffer);
    if(!found)
        retval= BOOTP_RECV_FAILED;
    
    return(retval);
    
}  /* NU_Bootp */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*      BOOTP_init                              Version 4.0                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This routine will handle the initing of the bootp packet.           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Accelerated Technology Inc.                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*      NU_Bootp                Application level function for Bootp.       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      memset                  Set memory to a particular value.           */
/*      memcpy                  Copies one section of memory to another.    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*      pkt                     Packet to be transmitted.                   */
/*      out_bp                  The structure that conatins global data.    */
/*                              filename to be returned.                    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      len                     The Length of the BOOTP Request packet.     */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Craig L. Meredith       04/10/93        Initial version.            */
/*      Donald R. Sharer        03/20/98        Modified for Net 4.0        */
/*                                                                          */
/****************************************************************************/

static uint16 BOOTP_init (BOOTPLAYER *pkt, BOOTP_STRUCT  *out_bp)
{
    /* Initially, there are no buffers on the BOOTP_List. */
    BOOTP_List.head = NU_NULL;
    BOOTP_List.tail = NU_NULL;

    /* get a unique transaction ID */
    bootp_xid = (ulint)n_clicks();

    /*  Set the Packet to all 0's */
    memset(pkt, 0, sizeof(BOOTPLAYER) );

    /* This is a bootp request. */
    pkt->bp_op = BOOTREQUEST;

    /* Initialize the hardware dependencies */
    pkt->bp_htype = HARDWARE_TYPE;
    pkt->bp_hlen = DADDLEN;        /* Hardware Address 1 octet */

    /* Initialize the unique ID. */
    pkt->bp_xid = bootp_xid;

    /* The number of seconds the bootp client has been running. */
    pkt->bp_secs = 1;
    memcpy(pkt->bp_chaddr, out_bp->bp_mac_addr,DADDLEN);

    /* Initialize the server's ip address to the broadcast address.
     *  The server will fill in his correct address. */
    memcpy(pkt->bp_siaddr.is_ip_addrs, IP_Brd_Cast, IP_ADDR_LEN);

    return(sizeof(*pkt));

}  /* BOOTP_init */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  BOOTP_Process_Packets                       Version 4.0                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This routine will handle the parseing of the incomming bootp        */
/*      packets.                                                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Donald R. Sharer, Accelerated Technology Inc.                       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*  NU_Bootp                    Application level function for Bootp.       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  NU_Allocate_Memory          Allocate Memory.                            */
/*  NU_Deallocate_Memory        Deallocates Memory.                         */
/*  NU_TCP_Log_Error            Log a TCP error.                            */
/*  NU_FD_Init                  Sets all bits in a bitmap to zero.          */
/*  NU_FD_Set                   Sets a bit in a bitmap.                     */
/*  NU_Select                   Check for data on multiple sockets.         */
/*  NU_FD_Check                 Checks to see if a particular bit has been  */
/*                              set in a bitmap.                            */
/*  NU_Recv_From                Receive Data from a socket descriptor.      */
/*  memcpy                      Copies one section of memory to another.    */
/*  comparen                    Compares one string to another string.      */
/*  strncpy                     Copy a string of one particular size to     */
/*                              another string.                             */
/* INPUTS                                                                   */
/*                                                                          */
/*      bp                      Pointer to the bootp packet that we are     */
/*                              to process.                                 */
/*      socketd                 Socket Descriptor to retrieve data from.    */    
/*      timeout                 Timeout value used for NU_Select.           */    
/*                                                                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      found                   Returns a 0 if the packet was not found.    */
/*                              Returns a 1 if the packet was found.        */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*   Don R. Sharer                   03/10/98        Modified for Net 4.0   */
/*                                                                          */
/****************************************************************************/

int BOOTP_Process_Packets(int16 socketd, BOOTP_STRUCT *bp, uint16 timeout)
{
    int                 ret;
    sint                x, items, len;
    int16               flen;
    char                *inbuf;
    int16               found = 0;
    uint8               bootp_cookie[4] = BOOTP_COOKIE;
    uint32              vend_cookie;
    ulint               local_xid;
    BOOTPLAYER          *bootp_ptr;          /* Bootp struct pointer*/
    FD_SET              readfs, writefs, exceptfs;
    STATUS              status;
    struct addr_struct  fromaddr;
    uchar               *ptr;
    uint8 message[80];

    /*   Allocate Memory for input Buffer */

    status = NU_Allocate_Memory(&System_Memory, (VOID **)&inbuf, IOSIZE,
				(UNSIGNED)NU_NO_SUSPEND);

    /*   Check if an error occured during NU_Allocate_Memory */
    if (status != NU_SUCCESS)
    {
        NU_Tcp_Log_Error( DHCP_NO_MEMORY, TCP_FATAL, __FILE__, __LINE__);
        return (NU_NO_SOCK_MEMORY);
    }

    /*   Do While to process received data */
    do 
    { /*  do while */
	 
        NU_FD_Init(&readfs);
        NU_FD_Set(socketd, &readfs);

        ret = NU_Select(NSOCKETS, &readfs, &writefs, &exceptfs,
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
              NU_Tcp_Log_Error( BOOTP_RECV_FAILED, TCP_FATAL,
                   __FILE__, __LINE__);
              break;
        }

        /* get pointer to BOOTP packet. */
        bootp_ptr = (BOOTPLAYER *)inbuf;

        /*  Retrieve the unique ID from the packet  */
        memcpy(&local_xid, &bootp_ptr->bp_xid, sizeof(ulint) );


        /* see if packet is the returning response to the packet I sent */
        if( local_xid != bootp_xid )
           continue;


        /* Accept the very first packet */

        /*  Get the Server address */
        bp->bp_siaddr[0] = bootp_ptr->bp_siaddr.is_ip_addrs[0];
        bp->bp_siaddr[1] = bootp_ptr->bp_siaddr.is_ip_addrs[1];
        bp->bp_siaddr[2] = bootp_ptr->bp_siaddr.is_ip_addrs[2];
        bp->bp_siaddr[3] = bootp_ptr->bp_siaddr.is_ip_addrs[3];

        /*  Get the gateway address */
        bp->bp_giaddr[0] = bootp_ptr->bp_giaddr.is_ip_addrs[0];
        bp->bp_giaddr[1] = bootp_ptr->bp_giaddr.is_ip_addrs[1];
        bp->bp_giaddr[2] = bootp_ptr->bp_giaddr.is_ip_addrs[2];
        bp->bp_giaddr[3] = bootp_ptr->bp_giaddr.is_ip_addrs[3];

        /*  Get my IP address */
        bp->bp_yiaddr[0] = bootp_ptr->bp_yiaddr.is_ip_addrs[0];
        bp->bp_yiaddr[1] = bootp_ptr->bp_yiaddr.is_ip_addrs[1];
        bp->bp_yiaddr[2] = bootp_ptr->bp_yiaddr.is_ip_addrs[2];
        bp->bp_yiaddr[3] = bootp_ptr->bp_yiaddr.is_ip_addrs[3];

        /*  Get the Server Name */
        memcpy(bp->bp_sname, bootp_ptr->bp_sname, sizeof(bp->bp_sname) );

        /* Get the Bootp file name.  */
        memcpy(bp->bp_file, bootp_ptr->bp_file, sizeof(bp->bp_file) );

        /*  Get the Vnedor specific cookie.  */
        memcpy( &vend_cookie, bootp_ptr->bp_vend, 4);

        /* test to see if cookie is a vendor cookie */
        if (comparen (bootp_ptr->bp_vend, VM_RFC1048,4))
        {

            if( vend_cookie == *(uint32 *)bootp_cookie )
            {
             /* Now loop thur vendor options, passing them to user */
            /* callback function. */
            ptr = bootp_ptr->bp_vend + 4;
            found = 1;
            while ((*ptr != 255) && ((ptr - bootp_ptr->bp_vend) < 64))
            {
                 switch (*ptr)
                 {
                  case BOOTP_PAD:           /* nop padding, used to align fields to word */
                          ptr++;    /* boundries.                                */
                          break;

                  case BOOTP_SUBNET:       /* subnet mask */
                          len = (*(ptr + 1));
                          ptr += 2;
                          bp->bp_net_mask[0] = (*ptr);
                          bp->bp_net_mask[1] = (*(ptr + 1));
                          bp->bp_net_mask[2] = (*(ptr + 2));
                          bp->bp_net_mask[3] = (*(ptr + 3));
                          ptr += len;
                          break;

                  case BOOTP_TIMEOFF:       /* time offset */
                          ptr += 3;
                          break;

                  case BOOTP_GATEWAY:       /* gateways  */
                          len = (*(ptr + 1));
                          items = len/4;
                          ptr += 2;
                          for (x = 0; x < items; x++)
                          {
                              ptr += 4;
                          }
                          break;

                  case BOOTP_TIMESERV:             /* time servers */
                  case BOOTP_NAMESERV:            /* IEN = 116 name server */
                          ptr += 3;
                          break;

                  case BOOTP_DOMNAME:             /* domain name server */
                          len = (*(ptr + 1));
                          items = len / 4;
                          ptr += 2;
                          for (x = 0; x < items; x++)
                          {

                              ptr += 4;
                          }
                          break;

                  case BOOTP_LOGSERV:       /* log server */
                                /* Place your code here. */
                  case BOOTP_COOKSRV:       /* cookie server */
                                /* Place your code here. */
                  case BOOTP_LPRSRV:        /* lpr server */
                                /* Place your code here. */
                  case BOOTP_IMPRSRV:       /* impress server */
                                /* Place your code here. */
                  case BOOTP_RLPSRV:        /* rlp server */
                                /* Place your code here. */
                          ptr += 3;
                          break;

                  case BOOTP_HOSTNAME:      /* client host name */
                          len = (*(ptr + 1));
                          strncpy ((int8 *)message, (const int8 *)ptr + 2, (uint16)len);
                          message[len] = 0;
                          ptr += len+2;
                          break;

                  case BOOTP_BFILSZ:        /* Bootp File Size */
                          ptr += 2;
                          break;

                  case BOOTP_END:
                          break;

                  default:
                          ptr += 3;
                          break;
                 }    /* end switch */
            }             /* end while    */
            }
        }

    } while( found == 0 );

    NU_Deallocate_Memory(inbuf);

    return(found);
}   /* end BOOTP_Process_Packets */


