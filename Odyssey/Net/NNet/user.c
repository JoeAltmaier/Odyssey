/*************************************************************************/
/*                                                                       */
/*       CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.         */
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
/*****************************************************************************
*                                                                           *
*     part of:                                                              *
*     TCP/IP kernel for NCSA Telnet                                         *
*     by Tim Krauskopf                                                      *
*                                                                           *
*     National Center for Supercomputing Applications                       *
*     152 Computing Applications Building                                   *
*     605 E. Springfield Ave.                                               *
*     Champaign, IL  61820                                                  *
*/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  user.c                                                     2.3            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  Network library interface routines                                        */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*  doconnect                                                                 */
/*  netclose                                                                  */
/*  netinit                                                                   */
/*  netlisten                                                                 */
/*  netread                                                                   */
/*  netsend                                                                   */
/*  netwrite                                                                  */
/*  netxopen                                                                  */
/*  Send_SYN_FIN                                                              */
/*  windowprobe                                                               */
/*                                                                            */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*  No other file dependencies                                                */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*  NAME                DATE        REMARKS                                   */
/*                                                                            */
/*  Glen Johnson      10/26/94      Updated netwrite for release 1.0.G1.E     */
/*  Glen Johnson      04/30/96      Made some changes based on recommedations */
/*                                  of K. Goto of Hitachi.                    */
/*  Maiqi Qian        12/06/96      Fixed the problem in spr0229.             */
/*  Maiqi Qian        12/11/96      Moved the assignment of retransmits value */
/*                                  into tcpsend() from netwrite().           */
/*  Maiqi Qian     12/12/96     Modified windowprobe to probe forever.        */
/*                                                                            */
/******************************************************************************/
#ifdef PLUS
  #include "nucleus.h"
#else   /* !PLUS */
  #include "nu_defs.h" 
  #include "nu_extr.h"
#endif  /* !PLUS */

#include "target.h"
#include "protocol.h"
#include "tcp.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "netevent.h"
#include "tcp_errs.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif
#include "arp.h"
#include "sockext.h"
#include "net_extr.h"

extern NU_TASK           NU_EventsDispatcher_ptr;
extern NU_TASK           timer_task_ptr;

#define	abs(a)	(a < 0) ? (-a) : (a)
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netread (uint16, char *, uint16)                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Read data from the connection buffer (specified by *pnum*) into the */
/*   user buffer for up to *n* bytes.                                    */
/*   Returns number of bytes read, or <0 if error.  Does not block.      */
/*                                                                       */
/* CALLED BY                                                             */
/*          NU_Recv                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      tcp_sendack                                                      */
/*      dequeue                                                          */
/*      NU_GetPnum                                                       */
/*                                                                       */
/*************************************************************************/
int16 netread(struct sock_struct *sock_ptr, char *buffer, uint16 nbytes)
{
    int16           howmany;
    uint16          i, lowwater;
    int16		    pnum;    
    struct port     *prt;

  	/* As long as there is data return it.  It is possible for the connection to 
	   have been closed in the event of a RESET.  However, if the data is here 
	   then it has been validated and is acceptable. So the data will be given to 
	   the application even if the connection has been closed.
	*/
	if (sock_ptr->s_recvbytes > 0)
	{
        /* Retrieve the queue data. */
        howmany = (int16) dequeue(sock_ptr, buffer, nbytes);
		
		if (sock_ptr->s_state & SS_ISCONNECTED)
		{
			/* Get the look up the index of the port structure. */
			if ((pnum = NU_GetPnum (sock_ptr)) >= 0)
			{
				prt = portlist[pnum];
				i = prt->in.size;                     /* how much before? */
		        prt->in.size += (uint16)howmany;      /* increment leftover room  */

			    lowwater = prt->credit>>1;
				if((i<lowwater) && ((prt->in.size) >= (uint)lowwater))
					tcp_sendack(prt);
			}
			else
			{
				howmany = NU_NOT_CONNECTED;
			}
		}

	}
	else
	{
		/* There was no queued data to receive. So, check to see if the 
		   connection still exists. */
		if (sock_ptr->s_state & SS_ISCONNECTED)
			howmany = 0;
		else
			howmany = NU_NOT_CONNECTED;
	}

    return ((int16)howmany);

} /* netread */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netwrite                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Write data into the output queue (specified by pnum).               */
/*   As long as there is buffer space and the foreign host's receive     */
/*   window has not been filled, data will be sent.  The number of bytes */
/*   actually sent is returned ( a number between 0 and nbytes).         */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Send                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Chain_Dequeue                                         */
/*      netsend                                                          */
/*      NU_GetPnum                                                       */
/*      UTL_Timerset                                                     */
/*      UTL_Timerunset                                                   */
/*      windowprobe                                                      */
/*                                                                       */
/* INPUTS                                                                */
/*      sockptr         Pointer to a socket on which to send the data.   */
/*      buffer          Pointer to the data to be sent.                  */
/*      nbytes          The number of bytes of data to send.             */
/*      status          Indicates why all data could not be sent.        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes that were sent.                              */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        04/13/95    Modified to make use of window      */
/*                                   probes and the Nagle alogorithm.    */
/*     G. Johnson        11/10/95    The new field buf_ptr->retransmits  */
/*                                   is initialized here.                */
/*     Maiqi Qian        12/06/96    Added sent_by_wp to windowprobe     */
/*                                   problem in spr0229.                 */
/*  Maiqi Qian        12/11/96      Moved the assignment of retransmits  */
/*                                  value into tcpsend().                */
/*                                                                       */
/*************************************************************************/
int16 netwrite(struct sock_struct *sockptr, uint8 *buffer, uint16 nbytes, 
               int16 *status)
{
    uint16      nsent=0, s;
    int32       sent_by_wp=0;
    struct      port *prt;
    struct      TCP_Window *wind;
    NET_BUFFER  *buf_ptr, *work_buf;
    int32       bytes_to_move = MAX_SEGMENT_LEN;
    int32       numbytes, bytes_left, bc;
    int16       pnum;

    /* Assume a successful operation until we are proven wrong. */
    *status = NU_SUCCESS;

    if ((pnum = NU_GetPnum(sockptr)) < 0)
    {
        *status = NU_INVALID_PORT;
        return(0);
    }

    if((prt = portlist[pnum]) == NU_NULL)
    {
        *status = NU_INVALID_PORT;
        return(0);
    }

    /* must be established connection */
    if((prt->state != SEST) && (prt->state != SCWAIT))
    {
        *status = NU_NOT_CONNECTED;
        return(0);
    }

    wind = &prt->out;

    numbytes = nbytes;


    /*  Find out what their advertised window size is.  If we want to
        send more bytes than they are advertising, then decrease the
        amount of bytes we want to send. */
    if ((wind->size - wind->contain) < numbytes)
    {
        numbytes = (wind->size - wind->contain);
    }

    if (numbytes <= 0)
    {
        /* Check to see if a window probe is needed.  A probe is only needed if
           the foreign port has advertised a window of 0 and there is no
           unacknowledged data. */
        if ( (wind->size == 0) && (wind->nxt == wind->ack) )
        {
            if ((sent_by_wp=windowprobe(prt, nbytes, buffer)) == NU_NOT_ESTAB)
            {
                *status = NU_NOT_CONNECTED;
                return(0);
            }

            /* Advance the buffer pointer beyond the data that has already been
               sent. */
            buffer += sent_by_wp;

            numbytes = nbytes - sent_by_wp;
            if (numbytes > (wind->size - wind->contain))
                numbytes = wind->size - wind->contain;
        }
        else
        {
            *status = NU_WINDOW_FULL;
            return(0);
        }
    }

    /* Are there any packets that contain data but have not yet been sent.
       Should never be more than one of these. */
    if (wind->nextPacket != NU_NULL)
    {

        /* Get a pointer to the start of the buffer chain. */
        buf_ptr = wind->nextPacket;

        /* Calculate the amount of data to be put into this buffer chain. */
        bytes_to_move = (numbytes < (uint16)(MAX_SEGMENT_LEN - buf_ptr->mem_total_data_len))
            ?  numbytes : (MAX_SEGMENT_LEN - buf_ptr->mem_total_data_len);

        bytes_left = bytes_to_move;

        /* Find the end of this buffer chain. */
        for ( work_buf = buf_ptr; 
              work_buf->next_buffer != NU_NULL; 
              work_buf = work_buf->next_buffer);

        if (work_buf == wind->nextPacket)
        {
            /* How much data should be added to this buffer. */
            bc = (NET_PARENT_BUFFER_SIZE - NET_MAX_TCP_HEADER_SIZE) 
                   - work_buf->data_len;

            bc = (bytes_left < bc) ? bytes_left : bc;
        }
        else
        {
            /* How much data should be added to this buffer. */
            bc = NET_MAX_BUFFER_SIZE - work_buf->data_len;
            bc = (bytes_left < bc) ? bytes_left : bc;
        }

        memcpy (work_buf->data_ptr + work_buf->data_len, buffer, bc);
        work_buf->data_len += bc;

        /* Bump up the total count for this buffer chain. */
        buf_ptr->mem_total_data_len += bc;

        bytes_left -= bc;
        buffer += bc;

        while (bytes_left)
        {
            /* Make sure there are enough buffers left before we take 
               another one. We must leave some for RX. */
            if ((MAXBUFFERS - MEM_Buffers_Used) <= NET_FREE_BUFFER_THRESHOLD)
            {
                /* Set the counter to reflect that we could not complete
                   sending all the data. */
                bytes_to_move -= bytes_left;

                /* Set bytes_left to zero so we can get out of this loop. */
                bytes_left = 0;
    
                /* Set the status. */
                *status = NU_NO_BUFFERS;

                continue;
            }

            work_buf->next_buffer = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);
            
            work_buf = work_buf->next_buffer;

            bc = (bytes_left < NET_MAX_BUFFER_SIZE) ? 
                    bytes_left : NET_MAX_BUFFER_SIZE;

            /* Copy the data. */
            memcpy (work_buf->mem_packet, buffer, bc);

            /* Set the data pointer. */
            work_buf->data_ptr = work_buf->mem_packet;

            /* Store the number of bytes in this buffer. */
            work_buf->data_len = bc;

            /* Bump up the total count for this buffer chain. */
            buf_ptr->mem_total_data_len += bc;

            bytes_left -= bc;

            buffer += bc;
        }


        /* Update various conters and statistics. */
        prt->out.contain += (uint16) bytes_to_move;

        nsent += (uint16) bytes_to_move;

        numbytes -= bytes_to_move;

        /* Update the number of actual TCP data held in this packet. This will
           only be used if this packet is retransmitted. */
        buf_ptr->mem_tcp_data_len = (uint16) buf_ptr->mem_total_data_len;

        /* Now that the data has been broken up into packets, send it.  If some 
           data was sent and there was a timer event set to transmit data, then 
           clear the event.  The data that the event was intended for has just 
           been sent. */
        if ( ((s = netsend(prt, buf_ptr)) > 0) && (prt->xmitFlag) )
        {
            UTL_Timerunset (CONTX, prt->pindex, (int32)1);
            prt->xmitFlag = NU_CLEAR;
        
            /* Clear the next packet pointer since this packet was sent. */
            wind->nextPacket = NU_NULL;
        }
    }

    /* While there is more data to send break it up into individual packets 
       and send it. */
    while (numbytes > 0)
    {
        /* If there is not enough data left to fill a complete buffer,
           update bytes_to_move with the number of bytes left. */
        if (numbytes <= MAX_SEGMENT_LEN)
           bytes_left = numbytes;
        else
            bytes_left = MAX_SEGMENT_LEN;

        /* Make sure there are enough buffers left before we take 
           another one. We must leave some for RX. */
        if ((MAXBUFFERS - MEM_Buffers_Used) <= NET_FREE_BUFFER_THRESHOLD)
        {
            /* Get out of the loop. */
            numbytes = 0;

            /* Set the status. */
            *status = NU_NO_BUFFERS;

            continue;
        }

        /* Allocate a buffer chain to stick the data in. */
        buf_ptr = MEM_Buffer_Chain_Dequeue (&MEM_Buffer_Freelist, 
            (bytes_left + NET_MAX_TCP_HEADER_SIZE));

        /* Compute the data size with the TCP header. */
        buf_ptr->mem_total_data_len = bytes_left;

        buf_ptr->mem_tcp_data_len = (uint16) bytes_left;

        work_buf = buf_ptr;

        /* Copy the data into the buffer chain. */
        while (bytes_left && work_buf)
        {
            /* Check to see if the current buffer is the first one in the chain.
               The first one is treated differntly from the rest. */
            if (work_buf == buf_ptr)
            {
                /* Set the number of bytes to copy to the minimum of the space 
                   available or the bytes left. */ 
                bc = (bytes_left < (NET_PARENT_BUFFER_SIZE - NET_MAX_TCP_HEADER_SIZE))
                    ? bytes_left : (NET_PARENT_BUFFER_SIZE - NET_MAX_TCP_HEADER_SIZE);
                
                /* Point to where the data will begin. */
                work_buf->data_ptr = (work_buf->mem_parent_packet + NET_MAX_TCP_HEADER_SIZE);
            }
            else
            {
                /* Set the number of bytes to copy to the minimum of the space 
                   available or the bytes left. */
                bc = (bytes_left < NET_MAX_BUFFER_SIZE) ? bytes_left : NET_MAX_BUFFER_SIZE;
                
                /* Point to where the data will begin. */
                work_buf->data_ptr = work_buf->mem_packet;
            }

            /* Copy the data into the buffer. */
            memcpy(work_buf->data_ptr, buffer, bc);
            
            /* update the number of bytes still to be copied. */
            bytes_left -= bc;

            /* Initialize the number of bytes of data in this buffer. */
            work_buf->data_len = bc;

            /* Advance the pointer to the userd data. */
            buffer += bc;

            /* Point to the next buffer in the chain. */
            work_buf = work_buf->next_buffer;
        }

        /* Initialize the list that this packet will be deallocated to when
           transmission is complete.
        */
        buf_ptr->mem_dlist = &prt->out.packet_list;

        /* Initialize the data length of this buffer. */
        buf_ptr->mem_option_len = 0;

        /* Update the amount of data in this port. */
        prt->out.contain += (uint16) buf_ptr->mem_total_data_len;

        /* Update the number of packets in this port. */
        prt->out.num_packets++;

        /* Increment the number of bytes we have sent so far. */
        nsent += (uint16) buf_ptr->mem_total_data_len;

        /* Decrease the amount of data left before the caller's request has
         * been filled. */
        numbytes -= (uint16) buf_ptr->mem_total_data_len;

        /* Now that the data has been broken up into packets, send it.  If some 
           data was sent and there was a timer event set to transmit data, then 
           clear the event.  The data that the event was intended for has just 
           been sent. */
        if ( ((s = netsend(prt, buf_ptr)) > 0) && (prt->xmitFlag) )
        {
            UTL_Timerunset (CONTX, prt->pindex, (int32)1);
            prt->xmitFlag = NU_CLEAR;

            /* Clear the next packet pointer since this packet was sent. */
            wind->nextPacket = NU_NULL;
        }
        else if (s == 0)
            wind->nextPacket = buf_ptr;

        if(wind->nextPacket != NU_NULL)
        {
            prt->xmitFlag = NU_SET;

            UTL_Timerset(CONTX, prt->pindex, SWSOVERRIDE, (int32)0);
        }

    }

    /* If the other side has sent a FIN then indicate that the connection is
       closing. */
    if (prt->state == SCWAIT)
        *status = NU_CLOSING;

    return((int16)(nsent+sent_by_wp));

} /* end netwrite */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netlisten(uint16)                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Listen to a TCP port number and make the connection automatically   */
/*   when the SYN packet comes in.  The TCP layer will notify the higher */
/*   layers with a CONOPEN event.  Save the port number returned to      */
/*   refer to this connection.                                           */
/*                                                                       */
/*   example usage : portnum=netlisten ( service )                       */
/*                                                                       */
/*   Returns<0 if error                                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      IP_Find_Route                                                    */
/*      makeport                                                         */
/*      n_clicks                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*      SCK_Create_Socket                                                */
/*                                                                       */
/*************************************************************************/
STATUS  netlisten(uint16 serv, struct pseudotcp *tcp_check)
{
    int16   pnum,i,pos;
    struct port *prt;
    int32   tval=0,tmp,tmnc;
    SCK_SOCKADDR_IP *dest;

      pos = 0;
      if((pnum=makeport())<0)
      {
	   if (pnum == -1)
	   {	       
	       tmnc = n_clicks();
	       for(i=0; (!(i >= NPORTS) && (portlist[i]!= NU_NULL)); i++)
	       {
		    if(portlist[i]->state == STWAIT)
		    {	    
		        tmp= abs(INT32_CMP(tmnc, portlist[i]->out.lasttime));
		        if (tmp > tval)
		        {
		           tval= tmp;
		           pos = i;		        
                    }
		    }
	       }
	       if(pos >= NPORTS)
             {
                  NU_Tcp_Log_Error (TCP_NO_TCP_PORTS, TCP_RECOVERABLE,
                                  __FILE__, __LINE__);
		    	return(-2);
             }
	       pnum= pos;
	   }
      }


    if(NU_NULL==(prt=portlist[pnum]))
         return(-2);

    /* Create the socket that will be used by the application that accepts 
	   this connection. */
    if ( (prt->p_socketd = SCK_Create_Socket(NU_PROTO_TCP)) < 0)
    {
        prt->state = SCLOSED;
        return (-2);
    }

    /* Point to the destination. */
    dest = (SCK_SOCKADDR_IP *) &prt->tp_route.rt_ip_dest;
    dest->sck_family = SK_FAM_IP;
    dest->sck_len = sizeof (*dest);
    dest->sck_addr = tcp_check->source;

    IP_Find_Route(&prt->tp_route);

    if (prt->tp_route.rt_route == NU_NULL)
    {
        prt->state = SCLOSED;
        return -1;
    }

    memcpy(prt->tcp_laddr, &tcp_check->dest,IP_ADDR_LEN);
           
    prt->in.port=serv;
    prt->out.port=0;                     /* accept any outside port #*/
    prt->in.lasttime=n_clicks();         /* set time we started */


    prt->state  = SLISTEN;
    prt->credit = WINDOWSIZE;            /* default value until changed */
    prt->tcpout.source = intswap(serv);  /* set service here too */

    return(pnum);

} /* netlisten */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netxopen(uint8 *, uint16, uint16, uint16, uint16, uint16)        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Open a network socket for the user to *machine* using               */
/*   port *service*.  The rest of the parameters are self-explanatory.   */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Connect                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      doconnect                                                        */
/*      IP_Find_Route                                                    */
/*      makeport                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  Maiqi Qian  12/05/96  Fixed the problem of SOR0229                   */
/*   1. Move makeport() and related lines after netdlayer successed.     */
/*   2. Move assignment of SSYNS to port->state to doconnect() after     */
/*     Send_SYN_FIN().                                                   */
/*                                                                       */
/*************************************************************************/
STATUS netxopen (uint8 *machine, uint16 service, INT socketd)
{
    struct port     *prt;
    int16           pnum;
    SCK_SOCKADDR_IP *dest;
    DV_DEVICE_ENTRY *device;

	/*
	*  check the IP number and don't allow broadcast addresses
	*/
	if (machine[3] == 255)
	{
		NU_Tcp_Log_Error (TCP_NO_BOARD_ADDRS, TCP_RECOVERABLE,
						  __FILE__, __LINE__);
		return (-4);
	}

#ifndef INTERRUPT
    netsleep (0);  /* make sure no waiting packets */
#endif

	/*
	*  get the hardware address for that host, or use the one for the
	*  gateway all handled by 'netdlayer' by ARPs.
	*/

    if ((pnum = makeport()) < 0)         /* set up port structure and packets */
        return (NU_NO_PORT_NUMBER);

	prt = portlist[pnum]; 			/* create a new port */

    prt->p_socketd = socketd;

    /* Determine which device will be used for communication.  This will allow
       us to decide which IP address to use on the local side. */
    /* The TCP port structure includes a route field that used to cache a route 
       to the foreign host.  Fill it in here. This will save us the trouble of 
       looking up the route at the IP layer for every packet sent.
    */
    /* Point to the destination. */
    dest = (SCK_SOCKADDR_IP *) &prt->tp_route.rt_ip_dest;
    dest->sck_family = SK_FAM_IP;
    dest->sck_len = sizeof (*dest);
    dest->sck_addr = *(uint32 *)machine;

    IP_Find_Route(&prt->tp_route);

    if (prt->tp_route.rt_route == NU_NULL)
    {
        prt->state = SCLOSED;

        /* Increment the number of packets that could not be delivered. */
        SNMP_ipOutNoRoutes_Inc;
        return -1;
    }

    device = prt->tp_route.rt_route->rt_device;

    memcpy (prt->tcp_laddr, device->dev_addr.dev_ip_addr, IP_ADDR_LEN);

    /* make a copy of the ip number that we are trying for */
    memcpy (prt->tcp_faddr, machine, IP_ADDR_LEN);

	/*
	 *	Make the connection, if you can, we will get an event notification
	 *	later if it connects.  Timeouts must be done at a higher layer.
	 */

    /* If the connection was made return the index into the portlist of 
       the new port.  Else return failure. */
    if ( doconnect (pnum, service) == NU_SUCCESS)
	    return (pnum);
    else
        return -1;

} /* netxopen */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      doconnect                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This routine sends the actual packet out to try and establish a     */
/*   connection.                                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*      netxopen                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ARP_Find_Entry                                                   */
/*      intswap                                                          */
/*      Send_SYN_FIN                                                     */
/*      SCK_Suspend_Task                                                 */
/*      NU_Current_Task_Pointer                                          */
/*      UTL_Timerset                                                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  Maiqi Qian  12/05/96  Fixed the problem of SOR0229                   */
/*   1. Move assignment of SSYNS to port->state after Send_SYN_FIN().    */
/*                                                                       */
/*************************************************************************/
STATUS doconnect(int16 pnum, uint16 service)
{
    struct port     *prt;
    STATUS          stat;
    SCK_SOCKADDR_IP dest;
    NU_TASK         *task_ptr;

    /* Get a pointer to the port. */
    prt = portlist[pnum];

    prt->tcpout.dest = intswap(service);      /* for example, telnet=23 */
    prt->out.port = service;                  /* service same as port num */
    prt->tcpout.flags = TSYN;                 /* want to start up sequence */
    prt->tcpout.ack = 0;                      /* beginning has no ACK */
    prt->state = SSYNS;                       /* syn sent */

    stat = Send_SYN_FIN(prt, 4);

    prt->out.nxt++;                       /* ack should be for next byte */

    /* If the hardware address of the remote host was unknown then suspend here.  
       The current task will be resumed when either the hardware address has 
       been resolved or the ARP resolution times out.
    */
    if (stat == NU_UNRESOLVED_ADDR)
    {
         task_ptr = NU_Current_Task_Pointer();
       
        if ( (task_ptr != &NU_EventsDispatcher_ptr) && 
             (task_ptr != &timer_task_ptr) )
        {

            /* Suspend pending resolution of the hardware address. */
            SCK_Suspend_Task(task_ptr);

            /* We need to know if the hardware address was successfully resolved so 
               look up the IP address of the next hop. If the next hop is a gateway 
               then see if the address for the gateway was resolved.  Else see if 
               the address for the remote host was resolved. */
            if (prt->tp_route.rt_route->rt_flags & RT_GATEWAY)
                dest.sck_addr = prt->tp_route.rt_route->rt_gateway.sck_addr;
            else
                dest.sck_addr = *(uint32 *)prt->tcp_faddr;

            /* Check the ARP Cache to see if the hardware address was resolved. */
            if (ARP_Find_Entry(&dest) != NU_NULL)
            {
                stat = NU_SUCCESS;

                /* If the transmit was a success, set a retransmit event for this
                   packet.
                */
                UTL_Timerset (TCPRETRANS, prt->pindex, prt->rto, prt->out.nxt);

                /* Increment the number of TCP segments transmitted. */
                SNMP_tcpOutSegs_Inc;
            }
        }
    }

    if (stat == NU_SUCCESS)
    {

        /* Increment the number active connections attempted. */
        SNMP_tcpActiveOpens_Inc;
    }
    else
        prt->state = SCLOSED;                 /* syn sent */

    return(stat);

}   /* doconnect */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      Send_SYN_FIN                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This routine is responsible for sending a packet containing either  */
/*   a SYN or FIN bit.                                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*      doconnect                                                        */
/*      netclose                                                         */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      IP_Send                                                          */
/*      longswap                                                         */
/*      MEM_Buffer_Dequeue                                               */
/*      MEM_One_Buffer_Chain_Free                                        */
/*      tcp_update_headers                                               */
/*      UTL_Checksum                                                     */
/*      UTL_Timerset                                                     */
/*                                                                       */
/*************************************************************************/
STATUS Send_SYN_FIN(PORT *prt, int16 options)
{
    TCPLAYER    *tcp_ptr;
    NET_BUFFER  *buf_ptr;
    int16       tcp_hlen;
    STATUS      stat;
    uchar       HUGE *options_ptr;

    /* One buffer will be large enough for the SYN FIN */
    buf_ptr = MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

    if(buf_ptr == NU_NULL)
        return -1;

    /* Init the pointers */
    buf_ptr->next           = NU_NULL;
    buf_ptr->next_buffer    = NU_NULL;

    /* Initialize each field in the buffer. */
    /* Point the data pointer at an offset into the buffer large enough to leave
       room for the IP and MAC layer headers. */
    buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + 
            (NET_MAX_TCP_HEADER_SIZE - sizeof (TCPLAYER)));

    /* There are 4 bytes of option data in the SYN packet. */
    buf_ptr->data_len = buf_ptr->mem_total_data_len = (sizeof (TCPLAYER) + options);
    buf_ptr->mem_option_len = options;

    /* Set the TCP data length to one. This is used for ack and seqnum 
       comparison. A SYN or FIN only bumps the seqnum by 1. */
    buf_ptr->mem_tcp_data_len = 1;

    /* Store the sequence number of this packet. */
    buf_ptr->mem_seqnum = prt->out.nxt;

    /* Initialize the list that this packet will be deallocated to when
       transmission is complete.
    */
    buf_ptr->mem_dlist = &prt->out.packet_list;

    /* Initialize the number of times this packet has been retransmitted. */
    buf_ptr->mem_retransmits = MAX_RETRANSMITS;

    /* Update the number of packets in this port. */
    prt->out.num_packets++;

    /* Update the amount of data in this port. */
    prt->out.contain++;

    /* Compute the sizeof the TCP header in words */
    tcp_hlen = ((sizeof(TCPLAYER) + options - 1) / 4) + 1;

    /* Update the header information. */
    tcp_update_headers(prt, longswap(buf_ptr->mem_seqnum), tcp_hlen);

    /* Move the header information into the packet. */
    memcpy (buf_ptr->data_ptr, &prt->tcpout, (sizeof(TCPLAYER) + options) );

    options_ptr = (buf_ptr->data_ptr + sizeof (TCPLAYER));
    tcp_ptr     = (TCPLAYER *) buf_ptr->data_ptr;

    /* Should option data be included. */
    if (options)
    {
        /* Add the TCP mac segment size option to this packet. */
        options_ptr[0] = 2;
        options_ptr[1] = 4;
        *(int16 *)&options_ptr[2] = intswap(MAX_SEGMENT_LEN);
    }

    /* Compute and fill in the checksum. */
    tcp_ptr->check = UTL_Checksum(buf_ptr, prt->tcp_laddr, prt->tcp_faddr, 
                                  IP_TCP_PROT);

    /* Send this packet. */
    stat = IP_Send(buf_ptr, &prt->tp_route, *(uint32 *)prt->tcp_faddr,
                   *(uint32 *)prt->tcp_laddr, 0, IP_TIME_TO_LIVE, 
                   IP_TCP_PROT, 0, NU_NULL);

    if (stat == NU_SUCCESS)
    {
        /* If the transmit was a success, set a retransmit event for this
           packet.
        */
        UTL_Timerset (TCPRETRANS, prt->pindex, prt->rto, buf_ptr->mem_seqnum);

        /* Increment the number of TCP segments transmitted. */
        SNMP_tcpOutSegs_Inc;
    }
    else if (stat != NU_UNRESOLVED_ADDR)
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated later by TCP. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    }

    return (stat);

}  /* Send_SYN_FIN */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netclose                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Start the closing process on port pnum.                             */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Close_Socket                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*      MEM_Buffer_Cleanup                                               */
/*      n_clicks                                                         */
/*      Send_SYN_FIN                                                     */
/*      tcp_xmit                                                         */
/*      UTL_Timerunset                                                   */
/*                                                                       */
/* INPUTS                                                                */
/*      pnum            Number of the port to close.                     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Success or failure.                                              */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        06/13/95      Modified SCWAIT case of the switch*/
/*                                     statment to fix a bug.            */
/*                                                                       */
/*************************************************************************/
int16 netclose (int16 pnum, struct sock_struct *sock_ptr)
{
    PORT *prt;
    int16         return_status = -1;

	if ((pnum < 0) || (pnum > NPORTS))		   /* is a valid port? */
		return (-1);

    if ((prt = portlist[pnum]) != NU_NULL)
	{		 /* something there */
        switch (prt->state)
		{
            case SLISTEN:             /* we don't care anymore */
			case SSYNS:

                 /* Increment the number of connection failures. */
                 SNMP_tcpAttemptFails_Inc;

                 prt->state = SCLOSED;
				 break;

			case SEST:					  /* must initiate close */
                 /* Send FIN only if all data has been transmitted. */
                 if (prt->out.nextPacket == NU_NULL)
                 {
                    if(prt->portFlags & ACK_TIMER_SET)
                    {
                        /*  Delete the ACK timeout timer.  */
                        UTL_Timerunset(TCPACK, prt->pindex, (int32)1);

                        /* Clear the ACK timer flag in the port. */
                        prt->portFlags &= (~ACK_TIMER_SET);
                    }

                    prt->tcpout.flags = TACK | TFIN;
                    Send_SYN_FIN(prt, 0);
                    prt->state = SFW1;           /* wait for ACK of FIN */
                    return_status = NU_SUCCESS;
                 }
                 else
                 {
                    if (prt->xmitFlag == NU_SET)
                    {
                         UTL_Timerunset (CONTX, prt->pindex, (int32)1);

                         prt->tcpout.flags = TPUSH | TACK | TFIN;

                         tcp_xmit(prt, prt->out.nextPacket);

                         prt->out.nextPacket = prt->out.nextPacket->next;

                         prt->xmitFlag = NU_CLEAR;
                    }

                    prt->state = SFW1;           /* wait for ACK of FIN */

                    prt->closeFlag = NU_SET;
                    return_status = -1;

                 }
                 break;                   /* do nothing for now ?*/


			case SCWAIT:			  /* other side already closed */
                 /* Send the FIN. */
                 if (prt->out.nextPacket == NU_NULL)
                 {
                     prt->tcpout.flags = TFIN | TACK;
                     Send_SYN_FIN(prt, 0);

                     /* Update the connection state. */
                     prt->state = SLAST;

                     /* Deallocate any date that is in the in window. */
                     if (sock_ptr->s_recvbytes)
                     {
                        MEM_Buffer_Cleanup(&sock_ptr->s_recvlist);
                     }

                     if(prt->out.contain)
                     {
                        MEM_Buffer_Cleanup(&prt->out.packet_list);
                        prt->out.nextPacket = NU_NULL;
                     }

                     return_status = -1;
                 }
                 else
                 {
                    if (prt->xmitFlag == NU_SET)
                    {
                         UTL_Timerunset (CONTX, prt->pindex, (int32)1);

                         prt->tcpout.flags = TPUSH | TACK | TFIN;

                         tcp_xmit(prt, prt->out.nextPacket);

                         prt->out.nextPacket = prt->out.nextPacket->next;

                         prt->xmitFlag = NU_CLEAR;
                    }

                    prt->state = SLAST;           /* wait for ACK of FIN */

                    prt->closeFlag = NU_SET;
                    return_status = -1;
                }

                 break;

			case STWAIT:			  /* time out yet? */
				 if ((portlist[pnum]->out.lasttime + WAITTIME) < n_clicks ())
				 {
                     prt->state = SCLOSED;
				 }
                 
                 return_status = NU_SUCCESS;
				 
                 break;

            case SLAST:
                 /* If the state is SLAST, then a FIN has already been sent.  We
                    are waiting for an ACK.  If one is not received the
                    retransmit timeout will eventually close this connection.
                 */
                 return_status = -1;

                 break;

            case SCLOSING:
            case SCLOSED:
                 return_status = NU_SUCCESS;
                 break;

			default:
				 break;
		}
	}
	else
	{
		return (1);
	}
    return (return_status);
}  /* end netclose */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netinit(void)                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Handles all the initialization to bring up the network connection.  */
/*   Assumes that the configuration file has already been read up.       */
/*                                                                       */
/*   Returns 0 on successful initialization.                             */
/*                                                                       */
/* CALLED BY                                                             */
/*          NU_Init_Net                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      init_time                                                        */
/*      MEM_Init                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*      protinit                                                         */
/*                                                                       */
/*************************************************************************/
STATUS netinit (void)
{
    int16 ret;

    /* initialize the system clock */
    init_time ();

    /*
     *   Initializes all buffers and hardware for data link layer.
     *   Machine/board dependent.
     */

    if ( (ret = MEM_Init()) != NU_SUCCESS )
	{
		NU_Tcp_Log_Error (TCP_HRDWR_INIT, TCP_RECOVERABLE,
						  __FILE__, __LINE__);
		return (ret);
	} /* end if */

	/*
	*  initialize the template packets needed for transmission
	*/
	protinit ();			 /* set up empty packets */

    return (NU_SUCCESS);
} /* netinit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      windowprobe                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Time out for three seconds.  Then if the window has not been updated*/
/*   send a 1 byte packet(windowprobe).                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*          netwrite                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Dequeue                                               */
/*      tcp_retransmit                                                   */
/*      tcp_xmit                                                         */
/*      UTL_Timerset                                                     */
/*      UTL_Timerunset                                                   */
/*      NU_Current_Task_Pointer                                          */
/*      NU_Release_Semaphore                                             */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*                                                                       */
/* INPUTS                                                                */
/*      prt             Pointer to a port.                               */
/*      nbytes          The number of bytes in the buffer parameter.     */
/*      buffer          Pointer to a data buffer.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes that were sent in window probes.             */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        04/13/95             Created initial version.   */
/*  Maiqi Qian     12/12/96     Modified windowprobe to probe forever.   */
/*                                                                       */
/*************************************************************************/
int32 windowprobe(PORT *prt, uint16 nbytes, uint8 *buffer)
{
    uint16      nsent = 0;
    NET_BUFFER  *buf_ptr;

    /* Set the probe flag. */
    prt->probeFlag = NU_SET;
    prt->rto = MINRTO;

    /* Set a timer event to resume this task. */
    UTL_Timerset(WINPROBE, prt->p_socketd, PROBETIMEOUT, 0);

    /* Probe the foreign host while he is advertising a window size of 0. */
    while(prt->out.size == 0)
    {

#ifdef PLUS
        /* Preserve the the task pointer. */
        socket_list[prt->p_socketd]->s_TXTask = NU_Current_Task_Pointer();

        /* Suspend this task. */
        SCK_Suspend_Task(socket_list[prt->p_socketd]->s_TXTask);

        socket_list[prt->p_socketd]->s_TXTask = NU_NULL;

#else /* RTX */

        /* Preserve the the task pointer. */
        socket_list[prt->p_socketd]->s_TXTask = NU_Current_Task_ID();

        /* Suspend this task. */
        SCK_Suspend_Task(socket_list[prt->p_socketd]->s_TXTask);

        socket_list[prt->p_socketd]->s_TXTask = -1;
#endif

        /* If a close was initiated while suspension was in effect abort
           the window probe. */
        if (prt->state != SEST)
            return(NU_NOT_ESTAB);

        /* if the windowsize is still zero and no unacknowledged data has been
         * sent and the buffer has untransmitted data, then send a window
         * probe.  We depend on the retransmit logic to resend the window probe
         * until either the probe is acknowledged or the window is opened. */

        if( (prt->out.size == 0) && (prt->out.contain == 0) && (nsent < nbytes))
        {
            buf_ptr = (NET_BUFFER *)MEM_Buffer_Dequeue(&MEM_Buffer_Freelist);

            if (buf_ptr != NU_NULL)
            {
               /* Point the data pointer at an offset into the buffer large
                  enough to leave room for the header information. */
               buf_ptr->data_ptr = buf_ptr->mem_parent_packet + 
                   NET_MAX_TCP_HEADER_SIZE;

               /* move the data into the packet buffer */
               memcpy(buf_ptr->data_ptr, buffer, 1);

               /* Initialize the data length of this buffer. */
               buf_ptr->data_len            = 1;
               buf_ptr->mem_tcp_data_len    = 1;
               buf_ptr->mem_total_data_len  = 1; 
               buf_ptr->mem_option_len      = 0;

               /* Initialize the list that this packet will be deallocated to when
                  transmission is complete.
               */
               buf_ptr->mem_dlist = &prt->out.packet_list;

               /* Update the amount of data in this port. */
               prt->out.contain++;

               /* Update the number of packets in this port. */
               prt->out.num_packets++;

               /* Increment the number of bytes we have sent so far. */
               nsent++;

               /* Move the location from which we are copying forward. */
               buffer++;

               /* Send the completed Packet. */
               tcp_xmit(prt, buf_ptr);
            }
        }
        else if (prt->out.size > 0)
            break;


        /* Set a timer event to resume this task just before the retransmits
           is about finishing. */
        UTL_Timerset(WINPROBE, prt->p_socketd, prt->rto*(MAX_RETRANSMITS-1),0);
        
        /* reset retransmits number, make the probing forever. MQ  */
        buf_ptr->mem_retransmits = MAX_RETRANSMITS;
    }

    /* At this point if there is a packet on the outgoing packet list.  Then
     * that means the foreign host opened up his window without
     * acknowledging the window probe.  Since the probe contains the next byte
     * expected by the foreign host, go ahead and retransmit it.  Otherwise, an
     * out of order packet situation will occur and it will take longer to fully
     * recover from the 0 windowsize. */

    if (prt->out.packet_list.head != NU_NULL)
    {
        /*  Clear the timer that will retransmit the window probe.  */
        UTL_Timerunset(WINPROBE, prt->p_socketd, (int32)1);

        tcp_retransmit(prt, prt->out.packet_list.head->mem_seqnum);
    }

    /* Clear the probe flag. */
    prt->probeFlag = NU_CLEAR;

    return(nsent);
} /* end windowprobe */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netsend                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function examines the ports outwindow and sends the data that  */
/*   is queued in the windows packet_list.  All of the data may not be   */
/*   sent immediately.  The decision to send data immediately or to hold */
/*   off is based on the Nagle Algorithm (RFC 1122, page 98).            */
/*                                                                       */
/* CALLED BY                                                             */
/*          netwrite                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      tcp_xmit                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*      prt             Pointer to a port.                               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes that were sent.                              */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        04/13/95             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
uint16 netsend(PORT *prt, NET_BUFFER *buf_ptr)
{
    uint16          nsent = 0;
    TCP_WINDOW          *wind;

    wind = &prt->out;

    /* Check to see if the PUSH flag has been set (i.e., the Nagle Algorithm
       has been disabled).  If so send every packet immediately with the
       push flag set. */
    if(wind->push)
    {
        prt->tcpout.flags |= TPUSH;

        /* Update the number of data bytes sent so far. */
        nsent += (uint16) buf_ptr->mem_total_data_len;

        /* Send the packet. */
        tcp_xmit(prt, buf_ptr);

    }

    /* Should the next packet be sent?  This decision is based on the Nagle
     * Algorithm (RFC 1122, page 98).  Following are the four cases when a
     * packet should be sent:
     * 1)  If a maximum size packet can be sent.
     * 2)  If there is no unacknowledged data and the data is pushed and
     *     all queued data can be sent.  Note that at this point all queued
     *     data can be sent, else it would not be queued.
     * 3)  If there is no unacknowledged data and some specified fraction of
     *     the maximum windowsize can be sent.  In this case we send if at
     *     least half the max windowsize can be sent.
     * 4)  If the data is pushed and an override timeout occurs.
    */
    else if( (buf_ptr->mem_total_data_len >= prt->sendsize) ||
            (wind->nxt == wind->ack) ||
            ((wind->nxt == wind->ack) && (buf_ptr->mem_total_data_len >=
                                         (prt->maxSendWin >> 1))) )
    {
        /* Update the number of data bytes sent so far. */
        nsent += (uint16) buf_ptr->mem_total_data_len;

        /* Send the packet. */
        tcp_xmit(prt, buf_ptr);

    }

    /* Return the number of data bytes that were sent. */
    return(nsent);
}  /* netsend */
