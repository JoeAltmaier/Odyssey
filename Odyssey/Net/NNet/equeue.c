/****************************************************************************/
/*                                                                          */
/*       CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.            */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  equeue.c                                                   4.0            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  Event queue manager                                                       */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*  NU_EventsDispatcher                                                       */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*  No other file dependencies                                                */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*  NAME                DATE        REMARKS                                   */
/*                                                                            */
/* Barbara Harwell      10/14/92    Created initial version.                  */
/* Barbara Harwell      10/26/92    Modified to include queue services.       */
/* Glen Johnson         04/30/96    Made some changes based on                */
/*                                  recommendations of K. Goto                */
/*                                  of Hitachi.                               */
/*                                                                            */
/******************************************************************************/

/* telnet includes */
#ifdef PLUS
  #include "nucleus.h"
#else   /* !PLUS */
  #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
  #include "nu_extr.h"
#endif  /* !PLUS */

#include "target.h"
#include "netevent.h"
#include "protocol.h"
#include "socketd.h"
#include "arp.h"
#include "externs.h"   /* include prototypes */
#include "data.h"
#include "tcpdefs.h"
#include "sockext.h"
#include "igmp.h"

#ifdef NU_PPP
#include "ppp.h"
extern struct           lcp_state_struct    ncp_state;
extern struct           lcp_state_struct    lcp_state;
extern NU_TIMER         LCP_Restart_Timer;
#endif

#ifndef PLUS
void NU_EventsDispatcher(void);
#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                              VERSION         */
/*                                                                       */
/*      NU_Events_Dispatcher                             4.0             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for dispatching events from the        */
/*   event queue to the appropriate handling routines.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/* History:                                                              */
/*       created - 10/14/92, bgh                                         */
/*       modified - 10/26/92, bgh - to include Nucleus queue services    */
/*                                                                       */
/*************************************************************************/

#ifdef PLUS
VOID NU_EventsDispatcher(UNSIGNED argc, VOID *argv)
#else   /* !PLUS */
VOID NU_EventsDispatcher(VOID)
#endif  /* !PLUS */
{
    struct uport        *uprt;
    struct port         *prt;
	struct sock_struct  *sock_ptr = NU_NULL;  /* Socket pointer. */
#ifdef PLUS
    STATUS              status;
    UNSIGNED            waittime;
#else   /* !PLUS */
    int16               status,
                        waittime;
#endif  /* !PLUS */
    int16               tmoflag;
    UNSIGNED            event = 0,
                        dat = 0;
    tqe_t               *duptqe,
                        *tqe_wait;           /* Timer queue entry */
#ifdef NU_PPP
    DV_DEVICE_ENTRY     *dev_ptr;
#endif


	/*******************************************************************
       Receive_Message points to a single occurence in the event queue.
       The index 3 signals that the structure is 3 words (or 6 bytes)
       long and is formatted as follows:

       struct
       {
          8 bits: the msg_class
          8 bits: the event
          16 bits: pointer to the next event on the queue
          16 bits: the data field
       }
    *******************************************************************/
#ifdef PLUS
    UNSIGNED    Receive_Message[3] = {0, 0, 0};
    UNSIGNED    actual_size;
#else   /* !PLUS */
    unsigned int      Receive_Message[3];
#endif  /* !PLUS */
    tqe_wait = (tqe_t *)0;

#ifdef PLUS
  /*  Remove compilation warnings for unused parameters.  */
  status = (STATUS) argc + (STATUS) argv;
#endif

    while (1)
    {

          /* Retrieve a message from the event queue.  Note that if the source
             queue is empty this task suspends until something becomes
             available. */
#ifdef PLUS
          NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else   /* !PLUS */
          NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif  /* !PLUS */

          /*  If someone is waiting on the timer list, then we need to
              calculate the next hit time for that timer.  */

          if (!tqe_wait)
            tqe_wait = tcp_timerlist.flink;

          if (tqe_wait) {
#ifdef PLUS
            if (tqe_wait->duetime > NU_Retrieve_Clock())
                waittime = ((UNSIGNED) tqe_wait->duetime - NU_Retrieve_Clock());
            else
                waittime = NU_NO_SUSPEND;
#else   /* !PLUS */
            waittime = (tqe_wait->duetime - NU_Read_Time());
            if (waittime <= 0)
              waittime = (-1);
#endif  /* !PLUS */
          }
          else {
            tqe_wait = (tqe_t *)0;
#ifdef PLUS
            waittime = NU_SUSPEND;
#else   /* !PLUS */
            waittime = NU_WAIT_FOREVER;
#endif  /* !PLUS */
          }

#ifdef PLUS
          /*  If waittime is not NU_SUSPEND then there is a timeout value. */
          if (waittime != NU_NO_SUSPEND) {
            NU_Release_Semaphore(&TCP_Resource);
#else   /* !PLUS */
          if (waittime >= 0) {
            NU_Release_Resource(TCP_Resource);
#endif  /* !PLUS */

#ifdef PLUS
            status = NU_Receive_From_Queue(&eQueue, &Receive_Message[0],
                                          (UNSIGNED)3, &actual_size, waittime);

            NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else   /* !PLUS */
            status = NU_Retrieve_Item(eQueue, Receive_Message, waittime);

            NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif  /* !PLUS */

          }
          else

#ifdef PLUS
            status = NU_TIMEOUT;
#else   /* !PLUS */
            status = NU_QUEUE_TIMEOUT;
#endif  /* !PLUS */

#ifndef INTERRUPT
        netsleep(0);
#endif  /* !INTERRUPT */
		/* Determine if the message was received successfully.	*/

#ifdef PLUS
        if (status == NU_TIMEOUT)
#else   /* !PLUS */
        if (status == NU_QUEUE_TIMEOUT)
#endif  /* !PLUS */
		{
          if (tqe_wait == tcp_timerlist.flink) {

             /*  Take off the head of the list. */
             dll_dequeue((tqe_t *) &tcp_timerlist);

             /*  Place any duplicate entries on to the timer list. */
             duptqe = (tqe_t *)dll_dequeue(&tqe_wait->duplist);
             while (duptqe)
             {
                tqpost((tqe_t *) &tcp_timerlist, duptqe);
                duptqe = (tqe_t *)dll_dequeue(&tqe_wait->duplist);
             }

             /*  Place the dequeued entry on the free list. */
             dll_enqueue(&tcptimer_freelist, tqe_wait);

             tmoflag = 1;

             /* Take care of event TCPRETRANS here...other events are
                handled by the CASE statement below... */

             if (tqe_wait->tqe_event == TCPRETRANS)
             {
                /*  Get a pointer to the porlist entry.  */
                prt = portlist[tqe_wait->tqe_data];

                /*  If there is still data to be transmitted and we
                    still have a connection, update the retransmission
                    timeout value.  */
                if ((prt->out.num_packets > 0) || (prt->state > SEST))
                {
                    /*  If a retransmission timeout occurs, exponential
                     *  back-off.  This number returns toward the correct
                     *  value by the RTT measurement code in ackcheck. */
                    if (prt->rto < MAXRTO)
                        prt->rto <<= 1;      /* double it */
                }

                tcp_retransmit (prt, tqe_wait->tqe_ext_data);
                tqe_wait = (tqe_t *)0;
#ifdef PLUS
                NU_Release_Semaphore(&TCP_Resource);
#else   /* !PLUS */
                NU_Release_Resource(TCP_Resource);
#endif  /* !PLUS */
                continue;
             }
             else
             {
               event = tqe_wait->tqe_event;
               dat = tqe_wait->tqe_data;
               status = NU_SUCCESS;
             }
          }
          else
          {
            tqe_wait = (tqe_t *)0;
#ifdef PLUS
            NU_Release_Semaphore(&TCP_Resource);
#else   /* !PLUS */
            NU_Release_Resource(TCP_Resource);
#endif  /* !PLUS */
            continue;
          }
        }
		else
		{
			tmoflag = 0;
            tqe_wait = (tqe_t *)NU_NULL;
        }

        /* Determine if the message was received successfully.  */
        if (status == NU_SUCCESS)
        {
			if (!tmoflag)
			{
                event = (int16)Receive_Message[0];
                dat   = (uint16)Receive_Message[2];
			}

            /* switch on the msg_class/event combination */
            switch (event)
            {
                default:
                     break;

                case CONNULL:
                     break;

                /***********  CONNECTION CLASS  **********************/
                case CONFAIL:  /* connection attempt failed */
                case CONOPEN:  /* successful connect attempt */

					/* Make sure the socket is not NULL, this is possible if a 
					   TCP connection is made and immediately RESET by the 
					   foreign host. */
					if ((sock_ptr = socket_list[dat]) != NU_NULL)
					{

						/* return control to the waiting task */
#ifdef PLUS
						if (sock_ptr->s_TXTask != NU_NULL)
							NU_Resume_Task(sock_ptr->s_TXTask);
#else   /* !PLUS */
						if (sock_ptr->s_TXTask != -1)
							NU_Start(sock_ptr->s_TXTask);
#endif  /* !PLUS */
					}

                    break;

                case TCPACK:
                    /* An ack needs to be sent. */

                    /* Get a pointer to the port. */
                    prt = portlist[dat];

                    /* Clear the ACK timer flag in the port. */
                    prt->portFlags &= (~ACK_TIMER_SET);

                    /* Send the ack. */
                    tcp_sendack (prt);

                    break;


                case CONTX:

                      /*  get a pointer into the port list table at the
                          entry pointed to by dat in the event queue */
                      prt = portlist[dat];

                      if ( (prt->xmitFlag == NU_SET) &&
                           (prt->out.nextPacket != NU_NULL) )
                      {
                          prt->tcpout.flags |= TPUSH;

                          tcp_xmit(prt, prt->out.nextPacket);

                          prt->out.nextPacket = (NET_BUFFER *)prt->out.nextPacket->next;

                          prt->xmitFlag = NU_CLEAR;

                      }
                      break;

                case WINPROBE:

                     /* Get a pointer to the socket. */
					if ((sock_ptr = socket_list[dat]) != NU_NULL)
					{

						/* restart the waiting task */
#ifdef PLUS
						if (sock_ptr->s_TXTask != NU_NULL)
							NU_Resume_Task(sock_ptr->s_TXTask);
#else
						if (sock_ptr->s_TXTask != -1)
							NU_Start(sock_ptr->s_TXTask);
#endif
					}
                    break;


                case SELECT:

                     NU_Resume_Task((NU_TASK *)dat);
                     break;


                case ARPRESOLVE:
                case RARP_REQUEST:

                     ARP_Event((uint16)dat);

                     break;

                case CONRX:

                      NET_Demux();
                      break;

                /**********************  USER CLASS *************************/
                case UDPDATA:
                    /* get a pointer into the port list table at the
                       entry pointed to by dat in the event queue */
                    uprt = uportlist[dat];

                    if (uprt != NU_NULL)
                    {
                        /* return control to the waiting task */
#ifdef PLUS
                        if (uprt->RXTask != NU_NULL)
                            NU_Resume_Task(uprt->RXTask);
#else   /* !PLUS */
                        if (uprt->RXTask != -1)
                            NU_Start(uprt->RXTask);
#endif  /* !PLUS */
                    }
                    break;

#if INCLUDE_IP_MULTICASTING
                case EV_IGMP_REPORT :
                    
                    /* Send an IGMP multicast group membership report. */
                    IGMP_REPORT_EVENT(dat);
                    break;

#endif /* INCLUDE_IP_MULTICASTING */

                case EV_IP_REASSEMBLY :

                    /* Clear the fragment list. The whole datagram has not
                       yet been received. */
                    IP_Reassembly_Event((IP_QUEUE_ELEMENT *)dat);
                    break;

               /*********************** PPP CLASS ************************/
#ifdef NU_PPP
                case LCP_RESEND:

                    /* Remove the PPP header that was added the first time
                       this packet was sent. */
                    lcp_state.negotiation_pkt->data_ptr += sizeof (PPP_HEADER);
                    lcp_state.negotiation_pkt->data_len -= sizeof (PPP_HEADER);
                    lcp_state.negotiation_pkt->mem_total_data_len
                                                        -= sizeof (PPP_HEADER);

                    /* Send the packet again. */
                    PPP_TX_Packet (lcp_state.negotiation_pkt->mem_buf_device,
                        lcp_state.negotiation_pkt);

                    break;

                case LCP_SEND_CONFIG:

                    /* Send a new configure request packet */
                    LCP_Send_Config_Req ();

                    break;

                case HANGUP_MDM:

                    MDM_Hangup();

                    break;

                case LCP_ECHO_REQ:

                    /* Send a echo request packet */
                    LCP_Send_Echo_Req ((DV_DEVICE_ENTRY *) dat);

                    break;

                case NCP_RESEND:

                    /* Remove the PPP header that was added the first time
                       this packet was sent. */
                    ncp_state.negotiation_pkt->data_ptr += sizeof (PPP_HEADER);
                    ncp_state.negotiation_pkt->data_len -= sizeof (PPP_HEADER);
                    ncp_state.negotiation_pkt->mem_total_data_len
                                                        -= sizeof (PPP_HEADER);

                    /* Send the packet again. */
                    PPP_TX_Packet (ncp_state.negotiation_pkt->mem_buf_device,
                        ncp_state.negotiation_pkt);

                    break;

                case NCP_SEND_CONFIG:


                    /* Get a pointer to the device structure for this device
                       so that the IP address of this device can be found. */
                    dev_ptr = (DV_DEVICE_ENTRY *)dat;

                    /* Send it to the host */
                    NCP_IP_Send_Config_Req (dev_ptr->dev_addr.dev_ip_addr);

                    break;

                case LCP_CLOSE_LINK:

                    /* Get a pointer to the device structure for this device
                       so that the IP address of this device can be found. */
                    dev_ptr = (DV_DEVICE_ENTRY *)dat;

                    /* Close the network down. */
                    PPP_Kill_All_Open_Sockets(dev_ptr);

                    /* Detach the IP address from this device. */
                    DEV_Detach_IP_From_Device (dev_ptr->dev_net_if_name);

                    /* Send a terminate request */
                    LCP_Send_Terminate_Req();

                    /* Start the timer */
                    NU_Reset_Timer (&LCP_Restart_Timer, LCP_Timer_Expire,
                       LCP_TIMEOUT_VALUE, LCP_TIMEOUT_VALUE, NU_ENABLE_TIMER);

                    break;

                case PAP_SEND_AUTH:

                    /* Retransmit the authentication pkt */
                    PAP_Send_Authentication();

                    break;

                case CHAP_RESEND:

                    /* Remove the PPP header that was added the first time
                       this packet was sent. */
                    lcp_state.negotiation_pkt->data_ptr += sizeof (PPP_HEADER);
                    lcp_state.negotiation_pkt->data_len -= sizeof (PPP_HEADER);
                    lcp_state.negotiation_pkt->mem_total_data_len
                                                        -= sizeof (PPP_HEADER);

                    /* Resend the last sent chap packet */
                    PPP_TX_Packet (lcp_state.negotiation_pkt->mem_buf_device,
                        lcp_state.negotiation_pkt);

                    break;

                case CHAP_CHALL:

                    /* Send the CHAP challenge */
                    CHAP_Send_Challenge();

                    break;

#endif /* NU_PPP */


            } /* end switch on msg_class/event combination */
        } /* end if status is NU_SUCCESS */

		/* added 11/3/92 - during ATI mods */
#ifdef PLUS
        NU_Release_Semaphore(&TCP_Resource);
#else   /* !PLUS */
        NU_Release_Resource(TCP_Resource);
#endif  /* !PLUS */

    } /* end while */
}  /* end NU_EventDispatcher */
