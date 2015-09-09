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
/***************************************************************************
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
****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  TOOLS.C                                                   4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  dequeue                                                                 */
/*  enqueue                                                                 */
/*  normalize_ptr                                                           */
/*  rmqueue                                                                 */
/*  netsleep                                                                */
/*  TL_Put_Event                                                            */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*   nucleus.h                                                              */
/*   nu_defs.h                                                              */
/*   nu_extr.h                                                              */
/*   protocol.h                                                             */
/*   data.h                                                                 */
/*   sockext.h                                                              */
/*   externs.h                                                              */
/*   tcpdefs.h                                                              */
/*   tcp_errs.h                                                             */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/*  Glen Johnson      10/26/94      Modified enqueue, rmqueue, dequeue,     */
/*                                  to handle the new buffer                */
/*                                  pointer management.                     */
/*  Maiqi Qian      12/06/96           Fixed the time wrap around (spr0229) */
/*  Uriah T. Pollock  11/18/97      Fixed bug in TL_Pur_Event - spr 380     */
/*                                                                          */
/****************************************************************************/

#ifdef PLUS
  #include "nucleus.h"
#else   /* !PLUS */
  #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
  #include "nu_extr.h"
#endif  /* !PLUS */

#include "target.h"
#include "protocol.h"
#include "net_extr.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "tcpdefs.h"
#include "tcp_errs.h"
#include "tcp.h"
#ifdef NORM_PTR
    #include <dos.h>
#endif


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      netsleep                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Sleep, while demuxing packets, so we don't miss anything            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      timer_task                                                       */
/*      netxopen                                                         */
/*      NU_EventsDispatcher                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      n_clicks                                                         */
/*      NET_Demux                                                        */
/*      tcp_sendack                                                      */
/*                                                                       */
/*************************************************************************/
#if !(defined INTERRUPT) || (defined PACKET_DRV)

int16 CDECL netsleep (int16 n)
{
    uint16 u;
    int16 nmux, redir;
    int32 t, gt, start;
    PORT *prt, **q;
    uint8 *pc;

    redir = 0;
    start = n_clicks ();

    if (n)
        t = start + n * TICKSPERSEC;
    else
        t = start;

    do
    {
        nmux = NET_Demux ();                /* demux all packets */

        /*
        *  if there were packets in the incoming packet buffer, then more
        *  might have arrived while we were processing them.  This gives
        *  absolute priority to packets coming in from the network.
        */
        if (nmux)
            continue;
        /*
         *  Check each port to see if action is necessary.
         *  This now sends all Ack packets, due to prt->lasttime being set to 0L.
         *  Waiting for nmux==0 for sending ACKs makes sure that the network
         *  has a much higher priority and reduces the number of unnecessary
         *  ACKs.
         */
        gt = n_clicks ();
        q = &portlist[0];
        for (u = 0; u < NPORTS; u++, q++)
        {
            prt = *q;
            if ((prt != NU_NULL) && (prt->state > SLISTEN))
            {
                /*  NFH - may be a bug, or we will never get here under normal
                 *  circumstances.  However, in transq, if sendsize is 0 we
                 *  spin in an infinite loop.  I added the code && p->sendsize
                 *  > 0 to prevent this.
                 */
                if ((!prt->out.lasttime) && (prt->sendsize > 0) &&
                    (prt->state == SEST))
                {
                     tcp_sendack(prt);               /* takes care of all ACKs */
                }
                if ((INT32_CMP((prt->out.lasttime+POKEINTERVAL), gt) < 0) &&
                     (prt->state == SEST))
                {
                     tcp_sendack(prt);
                     prt->out.lasttime = n_clicks();
                }

            } /* end if */
        } /* end for */

        redir = 0;              /* reset flag for next demux */
    } while ((t>n_clicks ()) && (n_clicks() >= start));  /* allow for wraparound of timer */

    return (nmux);             /* will demux once, even for sleep(0) */
}
#endif  /* !INTERRUPT */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      enqueue                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Add a packet to a TCP queue.  Used by both 'write()' and            */
/*   TCP_Interpret().   WINDOWSIZE is the size limitation of the         */
/*   advertised window.                                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*      estab1986                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*      MEM_Buffer_Update_Lists                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*************************************************************************/
uint16 enqueue (struct sock_struct *sock_ptr)
{
    NET_BUFFER *item_ptr;

    /* Remove the packet from the buffer_list and place it in the window list. */
    item_ptr = MEM_Update_Buffer_Lists(&MEM_Buffer_List, &sock_ptr->s_recvlist);

    /* Increment the number of bytes and the number of packets this
     * window contains.
     */
    sock_ptr->s_recvbytes += item_ptr->mem_total_data_len;           
    sock_ptr->s_recvpackets++;

    return ((uint16) item_ptr->mem_total_data_len);
} /* enqueue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      dequeue                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Used by netread, this copies data out of the queue and then         */
/*   deallocates it from the queue.                                      */
/*   rmqueue is very similar and is to be used by tcpsend                */
/*   to store unacknowledged data.                                       */
/*                                                                       */
/*   Returns the number of bytes removed from the queue.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*      netread                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*      MEM_Buffer_Chain_Free                                            */
/*                                                                       */
/*************************************************************************/
int32 dequeue (struct sock_struct *sock_ptr, char *buffer, uint32 nbytes)
{
    int32       actual_bytes;
    NET_BUFFER  *buf_ptr, *work_ptr;
    uint32      bytes_copied, offset = 0;

    /* If there is no data in the window, return. */
    if ((sock_ptr->s_recvbytes == 0) || (sock_ptr->s_recvlist.head == NU_NULL))
        return(0);

    /* If we don't have as much data as the user wants then give him what
     * we have. */
    if ((sock_ptr->s_recvbytes) < (uint)nbytes)
			nbytes = sock_ptr->s_recvbytes;

    /* because nbytes has its value changed below, actual_bytes is used
     * to preserve the number that will be returned.
     */
    actual_bytes = nbytes;

    /* While there is more data to be moved and buffers exist from which to
     * move it, continue.
     */
    while ( (nbytes) && (sock_ptr->s_recvlist.head != NU_NULL) )
    {
        bytes_copied = 0;

        /* Check to see if the current packet buffer contains more than
         * enough data.  If it does, then simply move the data and update
         * the data structures.  If it doesn't then all data in the current
         * buffer is moved, and that buffer is deallocated.
         */
        if (sock_ptr->s_recvlist.head->mem_total_data_len > nbytes)
        {

           /* Move the data into the caller's buffer, copy it from the 
              buffer chain. */

           /* Get a pointer to the data */
           buf_ptr = sock_ptr->s_recvlist.head;

           /* Is there more than enough data in the parent buffer. If 
              so then copy what is needed. */
           if (buf_ptr->data_len >= nbytes)
           {
               /* Copy only what is requested. */
               memcpy(buffer + offset, buf_ptr->data_ptr, nbytes);

               /* Update the bytes copied. */
               bytes_copied = nbytes;

               /* Update what is in this buffer and in the chain. */
               buf_ptr->data_ptr           += nbytes;
               buf_ptr->data_len           -= nbytes;
               buf_ptr->mem_total_data_len -= nbytes;
           }
           /* Is there any data in this buffer? */
           else if (buf_ptr->data_len > 0)
           {
               /* Copy all of it */
               memcpy (buffer + offset, buf_ptr->data_ptr, 
                   buf_ptr->data_len);

               /* Update the offset to copy more data too and the 
                  number of bytes just copied. */
               offset          += buf_ptr->data_len;
               bytes_copied    += buf_ptr->data_len;

               /* Update what is in this buffer and in the chain. */
               buf_ptr->mem_total_data_len -= buf_ptr->data_len;
               buf_ptr->data_len            = 0;
           }

           /* Get a work pointer. */
           work_ptr = buf_ptr;

           /* Loop through the chain if needed and copy all buffers. */
           while ((work_ptr->next_buffer != NU_NULL) &&
                   (bytes_copied < nbytes))
           {
               /* Move to the next buffer in the chain */
               work_ptr = work_ptr->next_buffer;

               /* Is there more than enough data in the buffer. If 
                  so then copy what is needed. */
               if (work_ptr->data_len >= (nbytes - bytes_copied))
               {
                   /* Copy only what is requested. */
                   memcpy(buffer + offset, work_ptr->data_ptr, 
                       (nbytes - bytes_copied));
                   
                   /* Update what is in this buffer and in the chain. */
                   work_ptr->data_ptr          += (nbytes - bytes_copied);
                   work_ptr->data_len          -= (nbytes - bytes_copied);
                   buf_ptr->mem_total_data_len -= (nbytes - bytes_copied);

                   /* Update the bytes copied. */
                   offset       += (nbytes - bytes_copied);
                   bytes_copied += (nbytes - bytes_copied);

               }
               /* Is there any data in this buffer? */
               else if (work_ptr->data_len > 0)
               {
                   /* Copy all of it */
                   memcpy (buffer + offset, work_ptr->data_ptr, 
                       work_ptr->data_len);

                   /* Update the offset to copy more data too and the 
                      number of bytes just copied. */
                   offset          += work_ptr->data_len;
                   bytes_copied    += work_ptr->data_len;

                   /* Update what is in this buffer and in the chain. */
                   buf_ptr->mem_total_data_len -= work_ptr->data_len;
                   work_ptr->data_len           = 0;
               }

            } /* end while there are buffers in the chain */

            /* Update the number of bytes in the window. */
            sock_ptr->s_recvbytes -= nbytes;

            /* Indicate that the caller's buffer has been filled. */
            nbytes -= nbytes;
        }
        else   /* there is not enough data in this buffer */
        {

           /* Move the data into the caller's buffer, copy it from the 
              buffer chain. */

           /* Get a pointer to the data */
           buf_ptr = sock_ptr->s_recvlist.head;

           /* If there is data in the parent buffer then copy it. */
           if (buf_ptr->data_len)
           {

               /* Do the parent buffer first */
               memcpy(buffer + offset, buf_ptr->data_ptr, 
                   buf_ptr->data_len);

               /* Update the bytes copied. */
               bytes_copied = buf_ptr->data_len;
           }
           else
               bytes_copied = 0;

           /* Loop through the chain if needed and copy all buffers. */
           while (buf_ptr->next_buffer != NU_NULL)
           {

               /* Move to the next buffer in the chain */
               buf_ptr = buf_ptr->next_buffer;

               /* If there is data in this buffer then copy it. */
               if (buf_ptr->data_len)
               {
                   /* Copy the data */
                   memcpy(buffer + offset + bytes_copied, 
                       buf_ptr->data_ptr, buf_ptr->data_len);

                   /* Update the bytes copied. */
                   bytes_copied += buf_ptr->data_len;
               }

           } /* end while there are buffers in the chain */


            /* Update the number of bytes left before the caller's request
             * has been filled. */
            nbytes -= sock_ptr->s_recvlist.head->mem_total_data_len;

            /* Update the number of bytes in the window. */
            sock_ptr->s_recvbytes -= sock_ptr->s_recvlist.head->mem_total_data_len;

            /* Update the number of packets that are in the window. */
            sock_ptr->s_recvpackets--;

            offset += (int16) sock_ptr->s_recvlist.head->mem_total_data_len;

            /* Put the empty buffer back onto the free list. */
            MEM_Buffer_Chain_Free (&sock_ptr->s_recvlist, 
                &MEM_Buffer_Freelist);
        }
    }

    return(actual_bytes);
} /* end dequeue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      rmqueue                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Does the queue deallocation once an ack is received.                */
/*   rmqueue of WINDOWSIZE or greater bytes will empty the queue         */
/*                                                                       */
/* CALLED BY                                                             */
/*      ackcheck                                                         */
/*      tcp_retranmit                                                    */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*      MEM_Buffer_Chain_Free                                            */
/*                                                                       */
/*************************************************************************/
uint16 rmqueue (struct TCP_Window *wind, int32 acked)
{
    uint16      bytes_removed = 0;
    NET_BUFFER  *buf_ptr;

    buf_ptr = wind->packet_list.head;

    while(buf_ptr)
    {
        /* If the current packet was acknowledged and the current packet is not
         * the next one to be transmitted, then remove it.  The sequence number
         * for the next packet to be transmitted has not yet been filled in and
         * there for contains an arbitrary number that maay be less than the
         * received ack. */
        if( (INT32_CMP(acked, (buf_ptr->mem_seqnum+(int32)buf_ptr->mem_tcp_data_len)) >= 0) &&
            (buf_ptr != wind->nextPacket) )
        {
            /* Update the number of bytes removed so far. */
            bytes_removed += buf_ptr->mem_tcp_data_len;

            /* Update the number of bytes contained in this window. */
            wind->contain -= buf_ptr->mem_tcp_data_len;  

            /* Update the number of packets contained in this window. */
            wind->num_packets--;

            /* Look at the next buffer. */
            buf_ptr = (NET_BUFFER *)buf_ptr->next;

            /* Place the acknowledged buffer back on the free list. */
            MEM_Buffer_Chain_Free (&wind->packet_list, &MEM_Buffer_Freelist);
        }
        else
            break;
    }

	/*	Return the number of bytes removed.  */
    return (bytes_removed);
}   /* end rmqueue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TL_Put_Event                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Add an event to the queue.                                          */
/*   Will probably get the memory for the entry from the free list.      */
/*   Returns 0 if there was room, 1 if an event was lost.                */
/*                                                                       */
/* CALLED BY                                                             */
/*      ARP_Resolve                                                      */
/*      tcpdo                                                            */
/*      UDP_Append                                                       */
/*      UTL_Timerset                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send_To_Queue                                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME                DATE        REMARKS                              */
/*                                                                       */
/*  Uriah T. Pollock    11/18/97     Changed the send to queue service   */
/*                                     to a none suspending call. A      */
/*                                     suspend in this routine could     */
/*                                     cause deadlock. Removed the sema- */
/*                                     phore release and obtain around   */
/*                                     the send to queue, since they are */
/*                                     no longer needed.                 */
/*                                                                       */
/*************************************************************************/
STATUS TL_Put_Event (uint16 event, UNSIGNED dat)
{
#ifdef PLUS
    STATUS status;
    STATUS return_status;
#else   /* !PLUS */
    int16  status;                        /* status of memory allocation */
    int16  return_status;
#endif  /* !PLUS */

    /*  Send_Message points to a single structure to be placed in the
     *  event queue. The index 3 signals that the structure is 3 words
     *  (or 6 bytes) long and is formatted as follows:
     *  struct
     *  {
     *      8 bits: the msg_class
     *      8 bits: the event
     *      16 bits: pointer to the next event on the queue
     *      16 bits: the data field
     *  };
     */

#ifdef PLUS
    UNSIGNED Send_Message[3];
    /* Send a message to the event queue.  Note that if
		 the queue is full this task suspends until space
		 becomes available.  */
    Send_Message[0] = (UNSIGNED) event;
    Send_Message[1] = (UNSIGNED) NU_NULL;
    Send_Message[2] = (UNSIGNED) dat;


    status =  NU_Send_To_Queue(&eQueue, &Send_Message[0], (UNSIGNED)3,
                              (UNSIGNED)NU_NO_SUSPEND);

#else   /* !PLUS */
    unsigned int Send_Message[3];
    /* Send a message to the event queue.  Note that if
     *   the queue is full this task suspends until space
     *   becomes available.  */
	Send_Message[0] = event;
	Send_Message[1] = NU_NULL;
	Send_Message[2] = dat;

    NU_Release_Resource(TCP_Resource);
    status = NU_Send_Item (eQueue, (unsigned int *)Send_Message,
                           NU_WAIT_FOREVER);
    NU_Request_Resource (TCP_Resource, NU_WAIT_FOREVER);
#endif  /* !PLUS */

	/* Determine if the message was sent successfully.	*/
	if (status == NU_SUCCESS)
        return_status = NU_SUCCESS;
	else
		return_status = 1;

	return (return_status);
} /* TL_Put_Event */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      normalize_pointer                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function takes a pointer for segmented memory architetures  */
/*      like the 80386 in real mode and normalizes that pointer.  It     */
/*      is not necessary on a flat memory model architecture.            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      UTL_Timerset                                                     */
/*      NU_Listen                                                        */
/*      SCK_Create_Socket                                                */
/*      NU_Init_Net                                                      */
/*      makeuport                                                        */
/*      makeport                                                         */
/*      MEM_Init                                                         */ 
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*       FP_SEG                                                          */
/*       FP_OFF                                                          */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
void *normalize_ptr(void *ptr)
{

#ifdef NORM_PTR
        unsigned long temp_address;

        temp_address = FP_SEG(ptr);
        temp_address = (temp_address << 4) + FP_OFF(ptr);
        FP_SEG(ptr) =  temp_address >> 4;
        FP_OFF(ptr) =  temp_address & 0xf;
#endif /* NORM_PTR */

        return (ptr);

}
