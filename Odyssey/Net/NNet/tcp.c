/*************************************************************************/
/*                                                                       */
/*     Copyright (c) 1993 - 1996 Accelerated Technology, Inc.            */
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
*      TCP/UDP/ICMP/IP Network kernel for NCSA Telnet                      *
*      by Tim Krauskopf                                                    *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*
*/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  TCP                                                        4.0            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  TCP level routines                                                        */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*  TCP_Interpret                                                             */
/*  tcpdo                                                                     */
/*  checkmss                                                                  */
/*  tcpresetfin                                                               */
/*  tcpsend                                                                   */
/*  ackcheck                                                                  */
/*  estab1986                                                                 */
/*  checkfin                                                                  */
/*  tcp_xmit                                                                  */
/*  tcp_retransmit                                                            */
/*  tcp_ooo_packet                                                            */
/*  check_ooo_list                                                            */
/*  NU_FindEmptyPort                                                          */
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
/*  Glen Johnson      04/30/96      Made some changes based on recommedations */
/*                                  of K. Goto of Hitachi.                    */
/* Maiqi Qian      12/06/96           Fixed the time wrap up (spr0229)        */
/* Maiqi Qian      12/11/96         Modified tcpsend() and tcp_retransmit().  */
/*                                                                            */
/******************************************************************************/

#undef OPTIMIZED_ACK
/*
 *  Includes
 */
#ifdef PLUS
  #include "nucleus.h"
#endif

#include "protocol.h"
#include "net_extr.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "externs.h"
#include "data.h"
#include "target.h"
#include "sockext.h"
#include "netevent.h"
#include "tcp_errs.h"
#include "tcp.h"
#include "arp.h"
#if SNMP_INCLUDED
#include "snmp_g.h"
#endif

/* Local Prototypes */
static  void checkfin (struct port *, TCPLAYER *);
static  int16 estab1986 (struct port *, NET_BUFFER *, uint16, uint16);
static  int16 ackcheck (struct port *, TCPLAYER *);
static  void checkmss (struct port *, TCPLAYER *, uint16);
static  int16 tcpresetfin (TCPLAYER *, struct pseudotcp *, int16);
static  int16 tcpdo (struct port *, NET_BUFFER *, uint16, struct pseudotcp *, int16);
static  int16 NU_FindEmptyPort (struct TASK_TABLE_STRUCT *);
int16 checkListeners(int16);

/*
 *  Semi-Global Vars
 */
static uint16 pnum;                                /* port number */

extern NU_TASK           NU_EventsDispatcher_ptr;
extern NU_TASK           timer_task_ptr;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCP_Interpret                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Called when a packet comes in and passes the IP checksum and is     */
/*   of TCP protocol type.  Check to see if we have an open connection   */
/*   on the appropriate port and stuff it in the right buffer            */
/*                                                                       */
/* CALLED BY                                                             */
/*      IP_Interpret                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      tcpcheck                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*      MEM_Buffer_Chain_Free                                            */
/*      intswap                                                          */
/*      tcpdo                                                            */
/*      checkListeners                                                   */
/*      tcpresetfin                                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      10-12-1995      Modified so hosts with different */
/*                                      IP's but same port #'s could be  */
/*                                      distinguished.                   */
/*                                                                       */
/*************************************************************************/
int16 TCP_Interpret (NET_BUFFER *buf_ptr, struct pseudotcp *tcp_chk)
{
    struct       port *prt;
    uint16      *temp_tcp;
    uint16      i, myport, hlen, hisport;
    TCPLAYER    *p;

    /* Increment the number of TCP segments received. */
    SNMP_tcpInSegs_Inc;

    /* NFH - We need to add the padding back in so that the TCP-Packet
       will be interpreted properly.  */
    temp_tcp = (uint16 *) p;
    temp_tcp--;
    
    /* Grab a pointer to the tcp layer */
    p = (TCPLAYER *)buf_ptr->data_ptr;

    /*
    *  checksum
    *    First, fill the pseudo header with its fields, then run our
    *  checksum to confirm it.
    *
    */
    if(p->check)
    {
        /* compute the checksum */
        if (tcpcheck( (uint16 *) tcp_chk, buf_ptr))
        {
            NU_Tcp_Log_Error (TCP_BAD_CKSUM, TCP_RECOVERABLE,
                  __FILE__, __LINE__);

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Increment the number of TCP packets received with errors. */
            SNMP_tcpInErrs_Inc;

            return (2);
        }  /* end if, for compute of the checksum */
    } /* end if, we need to do the checksum */

    /*
     *  find the port which is associated with the incoming packet
     *  First try open connections, then try listeners
     */
    myport = intswap (p->dest);
    hisport = intswap (p->source);

    /* bytes offset to data */
    hlen = p->hlen >> 2;

    /* Set the option len for this packet. */
    buf_ptr->mem_option_len = (sizeof (TCPLAYER) - hlen);

    for (i = 0; i < NPORTS; i++)
    {
        prt=portlist[i];
        if((prt != NU_NULL) && (prt->in.port == myport) &&
           (prt->out.port == hisport) &&
             comparen(prt->tcp_faddr, &tcp_chk->source, IP_ADDR_LEN))
        {
            pnum = i;
            return (tcpdo (prt, buf_ptr, hlen, tcp_chk, prt->state));
        } /* end if, this is the port that we want */
    } /* end for, i < NPORTS */

    /*
     *  If we got this far, then the current state is either SLISTEN or SCLOSED.
     *  First check for listeners.
     */
    if (checkListeners(myport) == NU_TRUE) 
    {
        return( tcpdo (prt, buf_ptr, hlen, tcp_chk, SLISTEN));
    }

    /*
    *  no matching port was found to handle this packet, reject it
    */

    /* Send a reset. */
    tcpresetfin (p, tcp_chk, (int16)(buf_ptr->mem_total_data_len - hlen));

    /* If we have reached this point, nobody processed the packet.  Therefore
       we need to drop it by placing it back on the buffer_freelist. */
    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

    /* no error message if it is a SYN */
    if(!(p->flags & TSYN))
    {
        /* Increment the number of TCP packets received with errors. */
        SNMP_tcpInErrs_Inc;

        NU_Tcp_Log_Error (TCP_BAD_PACKET_PORT, TCP_RECOVERABLE,
                  __FILE__, __LINE__);
    } /* end if t.flags & TSYN */

    /* return on port matches */
    return (1);
}  /* TCP_Interpret() */

/******************************************************************************/
/*                                                                            */
/* FUNCTION                                                                   */
/*                                                                            */
/*      tcpdo                                                                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   Deliver the incoming packet.                                             */
/*                                                                            */
/* CALLED BY                                                                  */
/*      TCP_Interpret                                                         */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*      intswap                                                               */
/*      longswap                                                              */
/*      tcp_sendack                                                           */
/*      TL_Put_Event                                                          */
/*      checkmss                                                              */
/*      ackcheck                                                              */
/*      estab1986                                                             */
/*      UTL_Timerunset                                                        */
/*      MEM_Buffer_Chain_Free                                                 */
/*      NU_FindEmptyPort                                                      */
/*      netlisten                                                             */
/*      Send_SYN_FIN                                                          */
/*      tcpresetfin                                                           */
/*      NU_Tcp_Log_Error                                                      */
/*      TCP_Cleanup                                                           */
/*      SCK_Connected                                                         */
/*      rmqueue                                                               */
/*      NU_SearchTaskList                                                     */
/*                                                                            */
/* NAME            DATE            REMARKS                                    */
/*                                                                            */
/* G. Johnson      05-24-1996      Fixed a bug in state SSYNS.                */
/*                                                                            */
/******************************************************************************/
static int16 tcpdo(struct port *prt, NET_BUFFER *buf_ptr, uint16 hlen, 
                   struct pseudotcp *tcp_chk, int16 state)
{
    struct TASK_TABLE_STRUCT    *Task_Entry;
    uint16                      myport;
    int16                       tasklist_num, portlist_num;
    unsigned char               ii;
    TCPLAYER                    *p;
    uint16                      tlen;

    /* Get the total length on the packet. Including the TCP header. */
    tlen = (int16) buf_ptr->mem_total_data_len;

    /* Get a pointer to the TCP header. */
    p = (TCPLAYER *)buf_ptr->data_ptr;

    /* Strip off the TCP header from the data sizes and data ptr. */
    buf_ptr->data_ptr           += (sizeof (TCPLAYER) + buf_ptr->mem_option_len);
    buf_ptr->data_len           -= (sizeof (TCPLAYER) + buf_ptr->mem_option_len);
    buf_ptr->mem_total_data_len -= (sizeof (TCPLAYER) + buf_ptr->mem_option_len);
    
    /*  Enter the state machine.  Determine the current state and perform
        the appropriate function.  Completed I/O will invoke this
        routine and consequently advance the state.  */

    switch (state)
    {
    case SLISTEN:  /*  Waiting for a remote connection.  */

        /*  If a reset is received, ignore it since we have not
	        yet established a connection.  */
	    if (p->flags & TRESET)
        {
            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            break;
        }

	    /*  Received a SYN, someone is starting a connection.  */
	    if(p->flags & TSYN)
	    {

            Task_Entry = Task_Head;
            myport = intswap (p->dest);

            /* verify that the list is not empty */
            while (Task_Entry != NU_NULL)
            {
                /* search for a task table structure that matches myport */
                if (Task_Entry->local_port_num == myport)
                    break;

                /* continue checking the next structure */
                Task_Entry = Task_Entry->next;
            } /* end while Task_Entry != NU_NULL */

            /* verify that there is a task table port entry free */
            if(NU_IGNORE_VALUE < (tasklist_num = NU_FindEmptyPort (Task_Entry)))
            {
                /* establish a portlist entry */
                if((portlist_num = netlisten (myport, tcp_chk)) < 0)
                {
                    /* Drop the packet by placing it back on the buffer_freelist. */
                    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

                    return (1);
                }  /* end if establish a portlist entry */

                /* store the portlist entry number in the task table */
                Task_Entry->port_entry[tasklist_num] = portlist_num;

                /* send that port structure to tcpdo */
                prt = portlist[portlist_num];

                /* store the task ID in the port table */
                socket_list[prt->p_socketd]->s_TXTask = Task_Entry->Task_ID;
       
            }  /* end if for no port num back from nuke */
            else
            {
                /* Drop the packet by placing it back on the buffer_freelist. */
                MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

                return(1);
            }

            /* Preserve the Task_Entry information so we can cleanup in the case
               where the connection is aborted before being accepted.
            */
            prt->task_entry = Task_Entry;
            prt->task_num = tasklist_num;

            /*  remember anything important from the incoming TCP header */
            prt->out.size = intswap(p->window);
            prt->out.port = intswap(p->source);
            prt->in.nxt = longswap(p->seq)+1;

            /*  set the necessary fields in the outgoing TCP packet  */
            prt->tcpout.dest = p->source;
            prt->tcpout.ack = longswap(prt->in.nxt);
            prt->tcpout.flags = TSYN | TACK;

            prt->tcpout.hlen = 24 << 2;

            /*  initialize all of the low-level transmission stuff
                (IP and lower) */
            memcpy (prt->tcp_faddr, &tcp_chk->source, IP_ADDR_LEN);

            /*  Send the SYN/ACK packet to the client.  */
            Send_SYN_FIN (prt, 4);

            /*  Indicate that a SYN has been received so get ready
                to get an ACK.  */
            prt->state = SSYNR;

            /* Increment the number of passive opens. */
            SNMP_tcpPassiveOpens_Inc;

        } /* end if */
        else
        {
            /* Send a reset if this is not a SYN packet. */
            tcpresetfin (p, tcp_chk, (int16) buf_ptr->mem_total_data_len);
        }

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free(&MEM_Buffer_List, &MEM_Buffer_Freelist);

        break;

    case SSYNR: 
  
        /*  In the SYN Received State we expect that we got an ACK.
		    If we did, then we need to send an ACK and move on to
		    the connected state.  */

          if (p->flags & TRESET)
          {
              NU_Tcp_Log_Error (TCP_NO_HOST_RESET, TCP_RECOVERABLE,
                        __FILE__, __LINE__);

              /* Clean up this port. */
              TCP_Cleanup(prt);

              /* Return to the closed state */
              prt->state = SCLOSED;

              /* Increment the number of connection failures. */
              SNMP_tcpAttemptFails_Inc;

              /* Drop the packet by placing it back on the buffer_freelist. */
              MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

              return (1);
          } /* end if */

          /*   We are expecting an ACK, if we did not get it then send
           *   the SYN/ACK back.  */
          if(!(p->flags & TACK))
          {
               prt->tcpout.flags = TACK | TSYN;
               tcp_sendack (prt);

               /* Preserve a pointer to the packet buffer just processed so we
                * can deallocate it.
                */

               /* Drop the packet by placing it back on the buffer_freelist. */
               MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
               break;
          } /* end if */

          /*  Update the header length for the TCP header.  */
          prt->tcpout.hlen = 20 << 2;

          /*  Set up for the timeout timer.  */
          prt->out.lasttime = n_clicks();

          /*  Indicate that a SYN has been sent.  */
          prt->out.nxt++;

          /*  Starting ACK value */
          prt->out.ack = longswap(p->ack);

          /*  Accept his window size.  */
          prt->out.size = intswap(p->window);
          prt->maxSendWin = prt->out.size;

          /*  Set up to send an ACK back.  */
          prt->tcpout.flags = TACK;

          /*  Move on to established state.  */
          prt->state = SEST;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_ADD, SEST, prt->tcp_laddr,
                                prt->in.port, prt->tcp_faddr,
                                prt->out.port);
#endif

          /* Mark the socket as connected. */
          SCK_CONNECTED(prt->p_socketd);	
	
          /*  Delete the timeout timer.  */
          UTL_Timerunset(TCPRETRANS, prt->pindex, prt->out.ack);

          /*  Remove the SYN packet from the window.  */
          rmqueue(&prt->out, prt->out.ack);

          /* Get the task entry.  The -1 indicates that we don't care about the
             state as long as port number and port index match. */
          Task_Entry = NU_SearchTaskList(Task_Entry, 
                                        socket_list[prt->p_socketd]->s_TXTask,
                                        prt->in.port, -1, prt->pindex);

          /*  Indicate the connection is complete.  This one can be accepted.*/
          Task_Entry->stat_entry[Task_Entry->current_idx] = SEST;

          /* If there is a task waiting to accept a connection, resume him. */
          if(Task_Entry->acceptFlag)
          {
              /*  Send an event to wake up the suspended server.  */
              TL_Put_Event (CONOPEN, prt->p_socketd);
          }

          /*  Detemermine if the sender has specified a new maximum
              message size.  */
          checkmss (prt, p, hlen);

          /* fall through */

    case SEST:    /* normal data transmission */
        
        /*  See if the last packet acknowledged one that we sent.  If
	        so, ackcheck will update the buffer and begin transmission
	        on the next part of the data if any is left in the buffer. */

        if(!ackcheck(prt, p))
        {
            /* If this port is in the process of closing, then resume the task. */
            if(prt->closeFlag)
            {
                prt->closeFlag = NU_CLEAR;
                TCP_Cleanup(prt);
            }
        }

        estab1986 (prt, buf_ptr, tlen, hlen);

	    return (0);

    case SSYNS: /* check to see if the ACK is for our SYN */

        /* remember that tcpout is pre-set-up */
        if (p->flags & TACK)
	    {	                                /* It is ACKING us */

            /* Is the ACK for the SYN that was sent?  If not we have a half open
               connection (i.e., the foreign host believes there is already a
               connection open on this port.  So send a reset and drop the packet.
            */
            if ((uint32)longswap (p->ack) != prt->out.nxt)
            {

                /* Send a reset. */
                tcpresetfin (p, tcp_chk, (int16) buf_ptr->mem_total_data_len);

                NU_Tcp_Log_Error (TCP_ACK_INV, TCP_RECOVERABLE,__FILE__,
                              __LINE__);

                /* Drop the packet by placing it back on the buffer_freelist. */
                MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

                return (1);
            } /* end if */
	    } /* end if */

	    if (p->flags & TRESET)
	    {
            NU_Tcp_Log_Error (TCP_NO_HOST_RESET, TCP_RECOVERABLE,
                  __FILE__, __LINE__);

            /* Cleanup after ourselves. */
            TCP_Cleanup(prt);

            prt->state = SCLOSED;

            /* Increment the number of connection failures. */
            SNMP_tcpAttemptFails_Inc;

            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
       
            return (1);
	    } /* end if */

        if(p->flags & TSYN)                 /* need to send ACK */
	    {
            prt->tcpout.flags = TACK;
            prt->in.nxt = longswap(p->seq) + 1;
            prt->tcpout.ack = longswap(prt->in.nxt);
            prt->out.ack = longswap(p->ack);
            prt->out.size = intswap(p->window);  /* credit window */
            prt->maxSendWin = prt->out.size;
       
            tcp_sendack (prt);
       
            if (p->flags & TACK)
            {
                prt->state = SEST;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_ADD, SEST, prt->tcp_laddr,
                                prt->in.port, prt->tcp_faddr,
                                prt->out.port);
#endif

                /* Mark the socket as connected. */
                SCK_CONNECTED(prt->p_socketd);	
	
                /*  Delete the timeout timer.  */
                UTL_Timerunset(TCPRETRANS, prt->pindex, prt->out.ack);

                /*  Remove it from the window.  */
                rmqueue(&prt->out, prt->out.ack);

                TL_Put_Event (CONOPEN, prt->p_socketd);
          
                checkmss (prt, p, hlen);
            } /* end if */
            else
            {
                prt->state=SSYNR;           /* syn received */
            }
	    } /* end if */

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

        break;

    case SCWAIT:
        
        ackcheck (prt, p);

        if (!prt->in.contain)
        {
            prt->tcpout.flags = TFIN | TACK;

            Send_SYN_FIN(prt, 0);

            prt->state = SLAST;
        } /* end if */
      
        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
        break;

    case SLAST:    /* check ack of FIN, or reset to see if we are done */

        if (ackcheck (prt, p) == NU_SUCCESS)
        {
            TCP_Cleanup(prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif

            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            break;
        }

        if ((p->flags & TRESET)
            || ((uint32)longswap (p->ack) == (prt->out.nxt + 1)))
        {
            TCP_Cleanup(prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif

        }

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
      break;

    case SFW1:                              /* waiting for ACK of FIN */

        /* throw away data */
        ackcheck (prt, p);
    
        prt->in.nxt = longswap (p->seq) + tlen - hlen;
      
        if (p->flags & TRESET)
        {
            TCP_Cleanup(prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif

        }
        else if ((uint32)longswap (p->ack) != (prt->out.nxt + 1))
	    {
            if (p->flags & TFIN)            /* got FIN, no ACK for mine */
            {
                prt->in.nxt++;              /* account for FIN byte */
                prt->tcpout.ack = longswap (prt->in.nxt);
                prt->tcpout.flags = TACK;   /* final byte has no FIN flag */

                tcp_sendack (prt);

                prt->state = SCLOSING;

            } /* end if */
            else
            {
                prt->tcpout.ack = longswap (prt->in.nxt);
                prt->tcpout.flags = TACK | TFIN;

                /* Since we are already in SFW1 a FIN has already been sent.  So
                   don't call Send_SYN_FIN here.  That would create another packet
                   containing a FIN that would have a retransmit event associated
                   with it. */
                tcp_sendack (prt);
            } /* end else */
	    } /* end if */

        else if (p->flags & TFIN)	/* ACK and FIN */
	    {
            prt->in.nxt++;                  /* account for his FIN flag */
            prt->out.nxt++;                 /* account for my FIN */
            prt->tcpout.ack = longswap (prt->in.nxt);
            prt->tcpout.flags = TACK;       /* final byte has no FIN flag */
            tcp_sendack (prt);

            TCP_Cleanup(prt);
            prt->state = STWAIT;            /* we are done */

#if SNMP_INCLUDED
            SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                    prt->in.port, prt->tcp_faddr,
                                    prt->out.port);
#endif

        } /* end if */
	    else
	    {                                   /* got ACK, no FIN */
            prt->out.nxt++;                 /* account for my FIN byte */
            prt->tcpout.flags = TACK;       /* final pkt has no FIN flag */
            prt->state = SFW2;
	    } /* end else */
        
        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
        break;
      
    case SFW2:

        /* want FIN */
        ackcheck (prt, p);
        
        prt->in.nxt=longswap(p->seq)+tlen-hlen;
        
        if(p->flags&TRESET)
        {
            TCP_Cleanup(prt);
            prt->state=SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif
        }
        else if(p->flags&TFIN)               /* we got FIN */
        {
            prt->in.nxt++;                  /* count his FIN byte */
            prt->tcpout.ack=longswap(prt->in.nxt);
            tcp_sendack (prt);

            TCP_Cleanup(prt);
            prt->state=STWAIT;

#if SNMP_INCLUDED
            SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                    prt->in.port, prt->tcp_faddr,
                                    prt->out.port);
#endif

        } /* end if */

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
        break;

    case SCLOSING:                          /* want ACK of FIN */

        if(p->flags&TRESET)
        {
            TCP_Cleanup(prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif
        }
        else if(!ackcheck(prt, p))
        {
            prt->out.nxt++;                 /* account for my FIN byte */
            TCP_Cleanup(prt);
            prt->state = STWAIT;            /* time-wait state next */

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif
        } /* end if */
      
        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
        break;

    case STWAIT:                            /* ack FIN again? */
      
        if(p->flags&TRESET)
        {
            TCP_Cleanup(prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif
        }
        
        if(prt->out.lasttime &&
            (INT32_CMP(prt->out.lasttime+WAITTIME, n_clicks()) < 0)
            || p->flags&TFIN)               /* only if he wants it */
        {
            tcp_sendack (prt);
            prt->state = SCLOSED;

#if SNMP_INCLUDED
          SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif
        }

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
      
        break;

    case SCLOSED:
        
        prt->in.port = prt->out.port = 0;
        
        for(ii = 0; ii < 4; ii++)
           prt->tcp_faddr[ii] = 0;

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        
        break;

    default:

        NU_Tcp_Log_Error (TCP_UNKOWN_STATE, TCP_RECOVERABLE,
                         __FILE__, __LINE__);

        /* Drop the packet by placing it back on the buffer_freelist. */
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

        break;
  } /* end switch */

  return(0);
}   /* end tcpdo() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      checkmss                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Look at incoming SYN,ACK packet and check for the options field      */
/*  containing a TCP Maximum segment size option.  If it has one,        */
/*  then set the port's internal value to make sure that it never        */
/*  exceeds that segment size.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*                                                                       */
/*************************************************************************/
static void checkmss(struct port *prt,TCPLAYER *p, uint16 hlen)
{
    uint16 i;
    uint8  *data_ptr;
    
    
    /* Get a pointer to data past the TCP header. */
    data_ptr = (uint8 *)(p + sizeof (TCPLAYER));

    /*  Check header for maximum segment size option.  */
    if(hlen > 20 && data_ptr[0] == 2 && data_ptr[1] == 4)
    {

        /* Extract the MSS */
        i = intswap(*(uint16 *)&data_ptr[2]);

        /* we have our own limits too */
        if(i < (uint16) (prt->sendsize))
            prt->sendsize=i;

    } /* end if */
}   /* end checkmss() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcpresetfin                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Send a reset packet back to sender                                   */
/*  Use the packet which just came in as a template to return to         */
/*  sender.  Fill in all of the fields necessary and send it back.       */
/*                                                                       */
/* CALLED BY                                                             */
/*      TCP_Interpret                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      longswap                                                         */
/*      ipcheck                                                          */
/*      MEM_Buffer_Dequeue                                               */
/*      MEM_Buffer_Enqueue                                               */
/*      IP_Send                                                          */
/*                                                                       */
/*************************************************************************/
static int16 tcpresetfin(TCPLAYER *t, struct pseudotcp *tcp_chk, int16 dlen)
{
    uint16      tport;
    NET_BUFFER  *buf_ptr;
    TCPLAYER    *out_tcp;
    STATUS      stat;
      
    if(t->flags&TRESET)                     /* don't reset a reset */
        return(1);

    /* We need to build a new packet to transmit this reset back to the
       sender.  Therefore, we must get the packet itself and we must acquire
       a header packet for the transmit operation (see below).  */

    /* Allocate a buffer to place the arp packet in. */
    buf_ptr = MEM_Buffer_Dequeue(&MEM_Buffer_Freelist);

    if(buf_ptr == NU_NULL)
    {
        return (NU_NULL);
    }

    /* Initialize each field in the allocated buffer. */
    buf_ptr->data_len   = buf_ptr->mem_total_data_len = sizeof (TCPLAYER);
    buf_ptr->data_ptr   = buf_ptr->mem_parent_packet +
        (NET_MAX_TCP_HEADER_SIZE - sizeof (TCPLAYER));
    buf_ptr->mem_seqnum = 0;
    buf_ptr->mem_dlist  = &MEM_Buffer_Freelist;

    /* Set up a pointer to the packet. */
    out_tcp = (TCPLAYER *)(buf_ptr->data_ptr);

    /*  Now we start building the packet itself.  We move the data from
        the TCP packet that we received into our newly allocated packet.  */

    /*  Swap TCP layer portions for sending back. */
    if(t->flags&TACK)
    {
        out_tcp->seq    = t->ack;           /* ack becomes next seq # */
        out_tcp->ack    = 0L;               /* ack # is 0 */
        out_tcp->flags  = TRESET|TFIN;
    } /* end if */
    else
    {
        out_tcp->seq = 0L;
        
        if(t->flags & TSYN)
            out_tcp->ack = longswap (longswap(t->seq) + dlen + 1);
        else
            out_tcp->ack = longswap (longswap(t->seq) + dlen);

        out_tcp->flags = TRESET | TACK | TFIN;
    } /* end else */

    tport           = t->source;            /* swap port #'s */
    out_tcp->source = t->dest;
    out_tcp->dest   = tport;
    out_tcp->hlen   = (sizeof (TCPLAYER) << 2); /* header len */
    out_tcp->window = 0;
    out_tcp->urgent = t->urgent;
    out_tcp->check = 0;

    /* Note that the source and destination IP addresses in tcp_chk need to be 
       reversed. The destination is our source and the source is our 
       destination. */
    out_tcp->check = UTL_Checksum (buf_ptr, (uint8 *)&tcp_chk->dest, 
                     (uint8 *)&tcp_chk->source, IP_TCP_PROT);

    /* Send this packet. */
    /* Note that the IP source addresses and IP destination addresses are
       swapped.*/
    stat = IP_Send(buf_ptr, NU_NULL, tcp_chk->source, tcp_chk->dest, 0,
                    IP_TIME_TO_LIVE, IP_TCP_PROT, IP_TYPE_OF_SERVICE, NU_NULL);

    if (stat == NU_SUCCESS)
    {
        /* Increment the number of TCP segments transmitted. */
        SNMP_tcpOutSegs_Inc;

        /* Increment the number of TCP resets sent. */
        SNMP_tcpOutRsts_Inc;
    }
    else if (stat != NU_UNRESOLVED_ADDR)
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated later by TCP. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    }

    return (0);
}   /* end tcpresetfin() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_update_headers                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Update the fields in the IP  and TCP headers that change from    */
/*      packet to packet.  Depending on the type parameter, call         */
/*      either tcpsend or tcp_sendack.                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Send_SYN_FIN                                                     */
/*      tcp_xmit                                                         */
/*      tcp_sendack                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      longswap                                                         */
/*      NU_Tcp_Log_Error                                                 */
/*                                                                       */
/*************************************************************************/
int16 tcp_update_headers (struct port *pport, uint32 seq_num, int16 tcp_hlen)
{
    struct port          *prt;

    prt = pport;

    if (prt == NU_NULL)
	{
		NU_Tcp_Log_Error (TCP_INV_PORT, TCP_RECOVERABLE,
						  __FILE__, __LINE__);
		return (-1);
	} /* end if */

	/* do TCP header */
    prt->tcpout.seq = seq_num;         /* byte swapped */

    /* Update the ack value that will be sent. */
    prt->tcpout.ack = longswap (pport->in.nxt);

    /* Update the tcp header len */
    prt->tcpout.hlen = (tcp_hlen << 4);

   /*
	*  if the port has some credit limit, use it instead of large
	*  window buffer.  Generally demanded by hardware limitations.
	*/
    if ((uint)(prt->credit) < (prt->in.size))
	{
        prt->tcpout.window = intswap (prt->credit);
	}
	else
	{
        prt->tcpout.window = intswap (prt->in.size);  /* window size */
	}

    /* get the checksum */
    prt->tcpout.check = 0;

    return NU_SUCCESS;

} /* end tcp_update_headers.  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_sendack                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Transmit an ack packet or a packet that contains options.        */
/*      Acks work this way so that we don't have to allocate a buffer    */
/*      for a packet that will contain no data.                          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      estab1986                                                        */
/*      checkfin                                                         */
/*      TCP_Ackit                                                        */
/*      netsleep                                                         */
/*      netread                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      tcp_update_headers                                               */
/*      MEM_Buffer_Dequeue                                               */
/*      UTL_Checksum                                                     */
/*      IP_Send                                                          */
/*      MEM_Buffer_Enqueue                                               */
/*                                                                       */
/*************************************************************************/
int16 tcp_sendack(struct port *pport)
{
    TCPLAYER    *tcp_ptr;
    NET_BUFFER  *buf_ptr;
    STATUS      stat;

    /*  Clear the PUSH flag if no data to be sent. */
    if (!pport->out.contain)
        pport->tcpout.flags &= ~TPUSH;

    /* Update the IP and TCP headers with the latest info.  The TCP header is
       always 5 words in size for an ACK packet. */
    tcp_update_headers(pport, longswap(pport->out.nxt), 5);

    /* Allocate a buffer to place the ack packet in. */
    buf_ptr = MEM_Buffer_Dequeue(&MEM_Buffer_Freelist);

    if(buf_ptr == NU_NULL)
    {
        return (NU_NULL);
    }

    /* Initialize each field in the allocated buffer, adding on the TCP layer. */
    buf_ptr->mem_total_data_len = sizeof (TCPLAYER);
    buf_ptr->data_len           = sizeof (TCPLAYER);
    buf_ptr->next               = NU_NULL;
    buf_ptr->next_buffer        = NU_NULL;
    buf_ptr->mem_seqnum         = 0;
    buf_ptr->mem_dlist          = &MEM_Buffer_Freelist;

    /* Point to the location within the packet where the TCP header begins. */
    buf_ptr->data_ptr = (buf_ptr->mem_parent_packet + 
        (NET_MAX_TCP_HEADER_SIZE - sizeof (TCPLAYER)));

    /* Move the TCP header into the packet. */
    memcpy (buf_ptr->data_ptr, &pport->tcpout, sizeof(TCPLAYER));

    /* Point at the TCP header. */
    tcp_ptr = (TCPLAYER *)buf_ptr->data_ptr;

    /* Compute and fill in the checksum. */
    tcp_ptr->check = UTL_Checksum(buf_ptr, pport->tcp_laddr, pport->tcp_faddr, 
                                  IP_TCP_PROT);

    /* Send this packet. */
    stat = IP_Send((NET_BUFFER *)buf_ptr, &pport->tp_route, *(uint32 *)pport->tcp_faddr,
                   *(uint32 *)pport->tcp_laddr, 0, IP_TIME_TO_LIVE, 
                   IP_TCP_PROT, IP_TYPE_OF_SERVICE, NU_NULL);

    if (stat == NU_SUCCESS)
    {
        /* Increment the number of TCP segments transmitted. */
        SNMP_tcpOutSegs_Inc;
    }
    else if (stat != NU_UNRESOLVED_ADDR)
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated when transmission is complete. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    }

    return(NU_SUCCESS);
}   /* end tcp_sendack */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcpsend                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send a tcp packet.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      tcp_xmit                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Timerunset                                                   */
/*      UTL_Checksum                                                     */
/*      IP_Send                                                          */
/*      UTL_Timerset                                                     */
/*      MEM_One_Buffer_Chain_Free                                        */   
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/* Maiqi Qian      12/11/96  Added assignment of retransmits value.      */
/*                                                                       */
/*************************************************************************/
STATUS tcpsend(struct port *pport, NET_BUFFER *buf_ptr)
{
    TCPLAYER        *tcp_ptr;
    STATUS          stat;

    /* Check to see if there is an ACK timer event set.  If so clear  it out
       because the data packet that is about to be sent will include the ack. */
    if(pport->portFlags & ACK_TIMER_SET)
    {
        /*  Delete the ACK timeout timer.  */
        UTL_Timerunset(TCPACK, pport->pindex, (int32)1 );

        /* Clear the ACK timer flag in the port. */
        pport->portFlags &= (~ACK_TIMER_SET);
    }

    /* Now add the TCP header and options to the size and update the data
       pointer to point to the header. */
    buf_ptr->data_ptr           -= (sizeof (TCPLAYER) + buf_ptr->mem_option_len);
    buf_ptr->data_len           += (sizeof (TCPLAYER) + buf_ptr->mem_option_len);
    buf_ptr->mem_total_data_len += (sizeof (TCPLAYER) + buf_ptr->mem_option_len);

    /* Move the header information into the packet. */
    memcpy ( buf_ptr->data_ptr, &pport->tcpout,
             (sizeof(TCPLAYER) + buf_ptr->mem_option_len) );

    /* Point to the beginning of the TCP header. */
    tcp_ptr = (TCPLAYER *)buf_ptr->data_ptr;

    /* Compute and fill in the checksum. */
    tcp_ptr->check = UTL_Checksum(buf_ptr, pport->tcp_laddr, pport->tcp_faddr, 
                                  IP_TCP_PROT);

    /* Initialize the number of times this packet has been retransmitted. */
    buf_ptr->mem_retransmits = MAX_RETRANSMITS;

    /* Send this packet. */
    stat = IP_Send((NET_BUFFER *)buf_ptr, &pport->tp_route, *(uint32 *)pport->tcp_faddr,
                    *(uint32 *)pport->tcp_laddr, 0, IP_TIME_TO_LIVE, 
                    IP_TCP_PROT, IP_TYPE_OF_SERVICE, NU_NULL);

    if (stat == NU_SUCCESS)
    {
        /* Set a retransmit event for this packet. */
        UTL_Timerset (TCPRETRANS, pport->pindex, pport->rto,
                   (uint32)buf_ptr->mem_seqnum + buf_ptr->mem_tcp_data_len);

        /* Increment the number of TCP segments transmitted. */
        SNMP_tcpOutSegs_Inc;
    }
    else if (stat != NU_UNRESOLVED_ADDR)
    {
        /* The packet was not sent.  Dealocate the buffer.  If the packet was
           transmitted it will be deallocated later by TCP. */
        MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
    }

    return(stat);

}   /* end tcpsend() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ackcheck                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Take an incoming packet and see if there is an ACK for the outgoing */
/*   side.  Use that ACK to dequeue outgoing data.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      longswap                                                         */
/*      NU_Resume_Task                                                   */
/*      TCP_Cleanup                                                      */
/*      SCK_Disconnecting                                                */
/*      NU_Tcp_Log_Error                                                 */
/*      INT32_CMP                                                        */
/*      UTL_Timerunset                                                   */
/*      rmqueue                                                          */
/*      n_clicks                                                         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME             DATE        REMARKS                                 */
/*                                                                       */
/*  G. Johnson      11/10/95     Added logic to deallocate all buffers   */
/*                               and clear all retransmit events if a    */
/*                               RST is received.                        */
/*                                                                       */
/*************************************************************************/
static int16 ackcheck(struct port *p, TCPLAYER *tcp_pkt)
{
    int32     ak;
    int32     rttl;
    sint      i;

    /*  We received a reset and the sequence number is within the current window
        then close the connection.  */
    if((tcp_pkt->flags & TRESET) &&
        (INT32_CMP(longswap(tcp_pkt->seq), p->in.nxt) >= 0)
        && (INT32_CMP(longswap(tcp_pkt->seq), (p->in.nxt+p->in.size)) <= 0))
    {

        /*  Indicate that we got a reset.  */
		NU_Tcp_Log_Error (TCP_RESET_HOST, TCP_RECOVERABLE,
						  __FILE__, __LINE__);

#if SNMP_INCLUDED
        if ((p->state == SCWAIT) || (p->state == SEST))
            SNMP_tcpEstabResets_Inc;

        SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, p->tcp_laddr,
                                p->in.port, p->tcp_faddr,
                                p->out.port);
#endif
        /* Mark the socket as disconnecting. */
        SCK_DISCONNECTING(p->p_socketd);

        /* Cleanup the port structure. */
        TCP_Cleanup(p);

        /*  Indicate that the connection is closed.  */
        p->state = SCLOSED;

        return (1);
    } /* end if */

    /*  If we did not get an ACK then return.  */
    if(!(tcp_pkt->flags & TACK))
        return(1);

    /*  Pick-up the other side's maximum transmission size and agree with
        him. */
    p->out.size = intswap(tcp_pkt->window);

    /* If the window size > 0 and the local host is currently probing the
       foreign host then restart the probing task. */
    if( (p->out.size > 0) && (p->probeFlag == NU_SET) )
    {
        /* Clear the window probe timer event. */
        UTL_Timerunset(WINPROBE, p->p_socketd, (int32)1);

        /* Clear the probe flag. */
        p->probeFlag = NU_CLEAR;

        /* Restart the probing task. */
#ifdef PLUS
        if (socket_list[p->p_socketd]->s_TXTask != NU_NULL)
            NU_Resume_Task(socket_list[p->p_socketd]->s_TXTask);
#else
        if (socket_list[p->p_socketd]->s_TXTask != -1)
            NU_Start(socket_list[p->p_socketd]->s_TXTask);
#endif

        /* Clear the the transmitting task pointer. */
        socket_list[p->p_socketd]->s_TXTask = NU_NULL;
    }

    /*  rmqueue any bytes which have been ACKed, update p->out.nxt to the
     *  new next seq number for outgoing.  Update send window.  */

    /*  Pick up the other side's ack number.  */
    ak = longswap(tcp_pkt->ack);

    /*  If ak is not increasing (above p->out.ack) then we should assume
     *  that it is a duplicate packet or a keepalive packet that 4.2 sends out.  
     */
    if ( INT32_CMP(ak, p->out.ack) > 0 ) 
    {
        /*  Delete the timeout timer.  */
        UTL_Timerunset(TCPRETRANS, p->pindex, longswap(tcp_pkt->ack) );

        /*  Remove it from the window.  */
        rmqueue(&p->out, ak);

        p->out.ack = ak;

        /*  Check to see if this acked our most recent transmission.  If so,
            adjust the RTO value to reflect the newly measured RTT.  This
            formula reduces the RTO value so that it gradually approaches the
            most recent round trip measurement.  When a packet is retransmitted,
            this value is doubled (exponential backoff).  */
        rttl = n_clicks() - p->out.lasttime;

        /*  If there is not data in the output buffer, and the amount of
            time since the last transmission is less than the maximum
            retry timeout value and the current retry timeout value is
            greater than the minimum retry timeout value, then update
            the retry timeout value to reflect the amount of time that
            it took for the packet to be acknowledged.  */

        if (!p->out.contain && rttl < (long) (MAXRTO) && p->rto >= MINRTO)
        {
              /*  Convert the acknowledge time to an integer value.  */
              i = (int16) rttl;

              /*  Use a smoothing function to update the retry timeout
                  value.  */
              i=((p->rto - MINRTO) * 3 + i + 1) >> 2;

              /*  Update the timeout value.  */
              p->rto = (uint16) i+MINRTO;

        } /* end if - need to adjust the RTO.  */

        if (tasks_waiting_to_send > 0)
        {
#ifdef PLUS
            if (socket_list[p->p_socketd]->s_TXTask != NU_NULL)
            {
                /* One of the suspended tasks will be started, so decrement the
                   number of tasks waiting. */
                tasks_waiting_to_send--;

                NU_Resume_Task(socket_list[p->p_socketd]->s_TXTask);

                socket_list[p->p_socketd]->s_TXTask = NU_NULL;
            }

#else /* RTX */
            if (socket_list[p->p_socketd]->s_TXTask != -1)
            {
                /* One of the suspended tasks will be started, so decrement the
                   number of tasks waiting. */
                tasks_waiting_to_send--;

                NU_Resume_Task(socket_list[p->p_socketd]->s_TXTask);

                socket_list[p->p_socketd]->s_TXTask = -1;
            }
#endif /* PLUS */

        }

        /*  Indicate that the operation was successful.  */
        return(NU_SUCCESS);
    } /* end if */

    return(1);
}   /* end ackcheck() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      estab1986                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Take a packet which has arrived for an established connection and   */
/*   put it where it belongs.                                            */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Chain_Free                                            */
/*      checkfin                                                         */
/*      longswap                                                         */
/*      INT32_CMP                                                        */
/*      tcp_ooo_packet                                                   */
/*      TCP_Ackit                                                        */
/*      enqueue                                                          */
/*      check_ooo_list                                                   */
/*      NU_Release_Semaphore                                             */
/*      NU_Resume_Task                                                   */
/*      Nu_Obtain_Semaphore                                              */
/*                                                                       */
/*************************************************************************/
static int16 estab1986(struct port *prt, NET_BUFFER *buf_ptr, uint16 tlen, uint16 hlen)
{
    uint16      dlen;
    uint32      sq, want;
    STATUS      status;
    TCPLAYER    *pkt = 0;
      
    /*  Calculate the length of the data received.  */
    buf_ptr->mem_tcp_data_len = dlen = tlen - hlen;

    if(((MAXBUFFERS - MEM_Buffers_Used) <= 4) && (dlen >0))
    {
        /*  Before dropping this packet check the FIN bit to see if this
            connection is closing. */
        checkfin (prt, pkt);
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
        return(0);
    }

    /* Get a pointer to the TCP header. */
    pkt = (TCPLAYER *)(buf_ptr->data_ptr - hlen);

    /*  See if we want this packet, or is it a duplicate?  */

    /*  Get the sequence number.  */
    sq = longswap(pkt->seq);

    /*  See what we were expecting.  */
    want = prt->in.nxt;

    /*  If the sequence number is not what we expected, then they may
     *  have still sent something, just less than we had hoped for.
     */
    if (sq != want)
    {
        /*  If the sequence number is less than what was expected,
         *  but the sequence number plus data length is greater than expected,
         *  then extract that data which is new.   */
        if ( (INT32_CMP(sq, want) < 0) && (INT32_CMP((sq+dlen), want) > 0) )
        {
            /*  Pick up where we want to begin taking the data from.  */
            hlen += (uint16)(want - sq);

            /*  Only take what we want, not what we have already received.  */
            dlen -= (uint16)(want - sq);

        } /* end if */
        /* If this is not the packet expected, but it is within the the current
           window, then place it in the out of order list. */
        else if ((INT32_CMP(sq, want) > 0) &&
            (INT32_CMP((sq+dlen), (want+prt->in.size)) <= 0) )
        {
            checkfin(prt, pkt);

            /* Only insert this packet in the out of order list if it contains
               data.
             */
            if (dlen)
            {
                status = tcp_ooo_packet(prt, sq);
                if (status != NU_SUCCESS)
                    MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
            }
            else    /* Release the buffer space. */
                MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            return (0);
        }
        else
        {
            /*  Somehow the other side missed the ACK that we have already
                sent for this packet.  RFC-1122 recommends sending an ACK
                in this case.  */
            TCP_Ackit(prt, 1);

            /* Discard the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            return(-1);

        } /* end else */

    } /* end if */
    else  /* Sequence number is equal to what we expect.  Check for data. */
    {
        /*  If we did not receive any data, then check for a FIN bit.  If
         *  we did not receive data, we will just throw the rest away, it
         *  was probably an ACK.
         */
        if (dlen == 0)
        {
            /* See if he sent us a FIN bit. */
            checkfin(prt,pkt);

            /* Discard the packet */
            /* Drop the packet by placing it back on the buffer_freelist. */
            MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

            /* Get out since there is no data to process.  */
            return(0);
        } /* end if */
    }


    /*  If we have room in the window, update the ACK field values.  */
    /*****************************************************************/
    /*                                                               */
    /*    Added code based on Dave Badger, Texas Memory, suggestion. */
    /*                12/16/94  NFH                                  */
    /*                                                               */
    /*****************************************************************/
    if ((prt->in.size >= (uint)dlen) && ((uint)dlen > 0) )
    {
        /*  Calculate the new ack value.  */
        prt->in.nxt += dlen;

        /*  Update the window size.  */
        prt->in.size -= dlen;

        /*  Put the data into the input TCP buffer for the user. */
        /* In most cases the hlen - 20 in the call below will evaluate to zero.
         * The one exeception will be when a packet is received that contains
         * both old and new data.  In this case hlen has been incremented by the
         *  number of old bytes in the packet up above. */

        enqueue (socket_list[prt->p_socketd]);

        /* Are there any packets in the out of order list. */
        if(prt->in.ooo_list.head != NU_NULL)
        {
            check_ooo_list(prt);
        }

        /* Ack the data. */
        TCP_Ackit(prt, 0);

        /*  Send a message to the dispatcher to let the user know
            there is data.  */
        if (dlen > 0)
        {

            if(socket_list[prt->p_socketd]->s_RXTask)
            {
#ifdef PLUS
                NU_Release_Semaphore(&TCP_Resource);
                NU_Resume_Task(socket_list[prt->p_socketd]->s_RXTask);
                NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else   /* !PLUS */
                NU_Release_Resource(TCP_Resource);
                NU_Start(socket_list[prt->p_socketd]->s_RXTask);
                NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif  /* !PLUS */
            }
        }
        /*  Update the last time that we received something from the other
            side. */
        prt->in.lasttime = n_clicks ();
    } /* end if */
    else if (dlen > 0)    /*  We have no room to receive data.  */
    {

        /*  Need an ACK to be sent anyway.  */
        /************************************/
        /*                                  */
        /* NFH:  THIS CODE IS SUSPECT.  WHY */
        /* DO WE SEND AN ACK???  WE HAVE    */
        /* NOT RECEIVED ANY DATA.           */
        /************************************/

        tcp_sendack (prt);

        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);

    } /* no more room in input buffer */
    else
        MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
 
    /*  Check the FIN bit to see if this connection is closing. */
    checkfin (prt, pkt);

    /*  Got here, so everything is OK. */
    return (0);

}   /* end estab1986() */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      checkfin                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Check the FIN bit of an incoming packet to see if the connection    */
/*   should be closing, ACK it if we need to.                            */
/*   Half open connections immediately, automatically close.  We do      */
/*   not support them.  As soon as the incoming data is delivered, the   */
/*   connection will close.                                              */
/*                                                                       */
/* CALLED BY                                                             */
/*      estab1986                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      longswap                                                         */
/*      SCK_DISCONNECTING                                                */
/*      tcp_sendack                                                      */
/*      UTL_Timerunset                                                   */
/*      NU_Resume_Task                                                   */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-29-1995      Changed so that the semaphore is */
/*                                      no longer released.              */
/*                                                                       */
/*************************************************************************/
static void checkfin(struct port *prt,TCPLAYER *pkt)
{
    if(pkt->flags&TFIN)
    {
        /* fin bit found */
        prt->in.nxt++;                              /* couNt the FIN byte */
        prt->state=SCWAIT;                          /* close-wait */
        prt->tcpout.ack=longswap(prt->in.nxt);    /* set ACK in packet */
        prt->credit=0;

        /* Mark the socket state as disconnecting. */
        SCK_DISCONNECTING(prt->p_socketd);

        /* If necessary clear out the Task_Entry structure. */
        if (prt->task_entry)
        {
            prt->task_entry->port_entry[prt->task_num] = NU_IGNORE_VALUE;
            prt->task_entry->stat_entry[prt->task_num] = NU_IGNORE_VALUE;
        }

        /*  At this point, we know that we have received all data that the
         *  other side is allowed to send.  Some of that data may still be in
         *  the incoming queue.  As soon as that queue empties, finish off the
         *  TCP close sequence.  We are not allowing the user to utilize a half-
         *  open connection, but we cannot close before the user has received
         *  all of the data from the incoming queue.
         */

        /* Acknowledge the FIN */
        tcp_sendack (prt);

        /* Check to see if a window probe is in progress.  If so clear this
           event and resume the waiting task. */
        if (prt->probeFlag == NU_SET)
        {
            prt->probeFlag = NU_CLEAR;

            /*  Clear the timer that will retransmit the window probe.  */
            UTL_Timerunset(WINPROBE, prt->p_socketd, (int32)1);
#ifdef PLUS
            if (socket_list[prt->p_socketd]->s_TXTask != NU_NULL)
                NU_Resume_Task(socket_list[prt->p_socketd]->s_TXTask);
#else   /* !PLUS */
            if (socket_list[prt->p_socketd]->s_TXTask != -1)
                NU_Start(socket_list[prt->p_socketd]->s_TXTask);
#endif  /* !PLUS */
        }

        /* If there is a task waiting to receive data resume him. */
        if(socket_list[prt->p_socketd]->s_RXTask)
        {
#ifdef PLUS
            NU_Resume_Task(socket_list[prt->p_socketd]->s_RXTask);
#else   /* !PLUS */
            NU_Start(socket_list[prt->p_socketd]->s_RXTask);
#endif  /* !PLUS */
        }

    } /* end if */
}   /* end checkfin() */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_FindEmptyPort                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Searches a single task table structure to see if there is a port     */
/*  list entry that is empty - this indicates that the server is         */
/*  willing to accept an additional connection request                   */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
static int16 NU_FindEmptyPort(struct TASK_TABLE_STRUCT *Task_Entry)
{
      int16 tempIndex;  /* to traverse the port entries in the task table */
      uint16 counter;    /* tracks number of entries checked */
      
      /* start counter equal to no entries checked */
      counter = Task_Entry->total_entries;
      
      /* begin the search at the oldest entry - current_idx */
      tempIndex = (int16)Task_Entry->current_idx;
      
      while (counter > 0)
      {
          /* we're looking for an empty port entry in the task table */
          if (Task_Entry->port_entry[tempIndex] == NU_IGNORE_VALUE)
               /* we want to return the index of the empty port entry */
               return(tempIndex);
               
               /* increment index but check for wraparound */
          if ((uint16)tempIndex == (Task_Entry->total_entries-1))
               tempIndex = 0;
          else
               tempIndex++;
          
          /* decrement number of entries left to check */
          counter--;
      }
      
      /* if we get this far, there was no empty entry in this structure */
      return(NU_IGNORE_VALUE);
} /* NU_FindEmptyPort */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_retransmit                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Send a tcp packet.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Events_Dispatcher                                             */
/*      windowprobe                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Buffer_Remove                                                */
/*      NU_Tcp_Error_Log                                                 */
/*      tcp_sendack                                                      */
/*      SCK_Disconnected                                                 */
/*      TCP_Cleanup                                                      */
/*      IP_Send                                                          */
/*      UTL_Timerset                                                     */
/*      MEM_One_Buffer_Chain_Free                                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME                DATE        REMARKS                              */
/*                                                                       */
/*  G. Johnson      11/10/95        Added logic to abort connection after*/
/*                                  a defined number of retransmits.     */
/* Maiqi Qian      12/11/96         Replaced tcpsend() by NET_Send() and */
/*                                  UTL_Timerset().                      */
/*                                                                       */
/*************************************************************************/
int16 tcp_retransmit(struct port *prt, int32 seq_num)
{
    int32       acked;
    NET_BUFFER  *buf_ptr;
    STATUS      stat;

    /* Get the latest value that was acked. */
    acked = prt->out.ack;

    /* make sure an ack has not been received for the packet that will be
     * retransmitted. */
    if( (seq_num - acked) < 0)
    {
        /* Somehow an ACKed packet was not removed.  Remove it now. */
        rmqueue(&prt->out, acked);
        return 0;
    }

    /* Search the packet list for the packet that needs to be retransmitted. */
    buf_ptr = prt->out.packet_list.head;

    while(buf_ptr)
    {
        /* Is this the packet we are looking for. */
        if(((uint32)buf_ptr->mem_seqnum + buf_ptr->mem_tcp_data_len) == (uint32)seq_num)
            break;

        /* Look at the next packet. */
        buf_ptr = buf_ptr->next;
    }

    MEM_Buffer_Remove (&prt->out.packet_list, buf_ptr);

    /* If the packet to be retransmitted was found, send it. */
    if(buf_ptr)
    {
        /* When retransmits reaches 0, this packet has been retransmitted the
           max number of times.  So abort the connection. */
        if(!buf_ptr->mem_retransmits)
        {
            NU_Tcp_Log_Error (TCP_NO_HOST_RESET, TCP_RECOVERABLE,
                      __FILE__, __LINE__);

#if SNMP_INCLUDED
            if ((prt->state == SCWAIT) || (prt->state == SEST))
                SNMP_tcpEstabResets_Inc;

            SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif

            /* Send a reset just in case the other side is still up. */
            prt->tcpout.flags = TRESET;
            tcp_sendack(prt);

            /* Mark the socket as disconnecting. */
            SCK_DISCONNECTING(prt->p_socketd);
            
            /* The connection is  closed.  Cleanup. */
            TCP_Cleanup(prt);

            /* Abort this connection. */
            prt->state = SCLOSED;

            return (-1);
        }

        /*
         Since the retransmision is caused by the timer, which was set by
         tcpsend(), where the buffer was installed, the following lines
         are redundant.
         1. tcpsend() is replaced by NET_Send(),
         2. UTL_Timerset() is necessarily set again, here.
        */
        /* Decrement the number of times this packet can be retransmitted. */
        buf_ptr->mem_retransmits--;

        /* Reset the data lengths */
        buf_ptr->mem_total_data_len = buf_ptr->mem_tcp_data_len + 
            buf_ptr->mem_option_len + sizeof(TCPLAYER);

        /* If the total data length is small enough to fit in the
           parent buffer then these lengths are the same. */
        if (buf_ptr->mem_tcp_data_len <= (NET_PARENT_BUFFER_SIZE - 
                (NET_MAX_TCP_HEADER_SIZE + buf_ptr->mem_option_len)))
            buf_ptr->data_len = buf_ptr->mem_total_data_len;
        else
            buf_ptr->data_len = (NET_PARENT_BUFFER_SIZE - 
                (NET_MAX_TCP_HEADER_SIZE - 
                (sizeof (TCPLAYER) + buf_ptr->mem_option_len)));

        /* Reset the data pointer. */
        buf_ptr->data_ptr           = buf_ptr->mem_parent_packet + 
            (NET_MAX_TCP_HEADER_SIZE - (sizeof (TCPLAYER) + buf_ptr->mem_option_len));

        /* Send this packet. */
        stat = IP_Send((NET_BUFFER *)buf_ptr, &prt->tp_route, *(uint32 *)prt->tcp_faddr,
                        *(uint32 *)prt->tcp_laddr, 0, IP_TIME_TO_LIVE, 
                        IP_TCP_PROT, IP_TYPE_OF_SERVICE, NU_NULL);

        if (stat == NU_SUCCESS)
        {
            /* If the transmit was a success, set a retransmit event for this
               packet.
            */
            /* Set a retransmit event for this packet. */
            UTL_Timerset (TCPRETRANS, prt->pindex, prt->rto, seq_num);

            /* Increment the number of retransmitted segments. */
            SNMP_tcpRetransSegs_Inc;
        }
        else if (stat != NU_UNRESOLVED_ADDR)
        {
            /* The packet was not sent.  Dealocate the buffer.  If the packet was
               transmitted it will be deallocated later by TCP. */
            MEM_One_Buffer_Chain_Free (buf_ptr, &MEM_Buffer_Freelist);
        }

        return(NU_SUCCESS);
    }
    else
       return( -1);

}  /* end tcp_retransmit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCP_Ackit                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for deciding when an ACK should be sent.*/
/*  An ack should be delayed to 1) make it possible to ack several       */
/*  packets at once  2) If a response is sent immediately by the receiver*/
/*  allow an ack to be sent with the response.  However, when data is    */
/*  being received very rapidly the ack can't be delayed for too long.   */
/*  This is the reason for checking when the last ack was sent, and      */
/*  sending an ack immediately if the specified threshold is reached.    */
/*                                                                       */
/* CALLED BY                                                             */
/*      estab1986                                                        */
/*      NU_EventsDispatcher                                              */
/*                                                                       */
/* CALLS                                                                 */
/*      UTL_Timerset                                                     */
/*      UTL_Timerunset                                                   */
/*      tcp_sendack                                                      */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
VOID TCP_Ackit(struct port *prt, INT force)
{

    /* If the caller wishes to force an immediate ACK or there has been enough
       data received to fill up half the window since the last ack was sent,
       then send an ack. */
    if ((force) ||
        (int32)(prt->in.nxt - longswap(prt->tcpout.ack)) >= (prt->in.size >> 1))
    {
        /* Check to see if there is a timer event to transmit an ack.  If
           so clear the timer, it is no longer needed.  */
        if(prt->portFlags & ACK_TIMER_SET)
        {
            /*  Delete the ACK timeout timer.  */
            UTL_Timerunset(TCPACK, prt->pindex, (int32)1 );

            /* Clear the ACK timer flag in the port. */
            prt->portFlags &= (~ACK_TIMER_SET);
        }

        /*  Make sure that the ack goes out.  */
        tcp_sendack (prt);
    }
    /* If a timer event to send an ack has not been created then create one. */
    else if (!(prt->portFlags & ACK_TIMER_SET) )
    {
        /* Set up the ACK timer. */
        UTL_Timerset(TCPACK, prt->pindex, ACKTIMEOUT, (int32)0);

        /* Set the flag to indicate a timer has been set. */
        prt->portFlags |= ACK_TIMER_SET;
    }

} /* TCP_Ackit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_ooo_packet                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for placing out of order TCP packets    */
/*  into their proper place in the out of order list.  This is called    */
/*  whenever a packet that is not expected but is within the window is   */
/*  received.  The list of out of order list will later be used when the */
/*  expected arrives.  All data (the expected packet and those retrived  */
/*  from this list) will then be acknowledged at one time.               */
/*                                                                       */
/* CALLED BY                                                             */
/*      estab1986                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MEM_Update_Buffer_Lists                                          */
/*      INT32_CMP                                                        */
/*      MEM_Buffer_Dequeue                                               */
/*      MEM_Buffer_Insert                                                */
/*                                                                       */
/* INPUTS                                                                */
/*      prt             Pointer to a port.                               */
/*      buffer          Pointer to to where data starts in a packet      */
/*                      buffer.                                          */
/*      dlen            The amount of data in the packet.                */
/*      seq             Sequence number of the packet.  Used to place    */
/*                      the packet in its proper place in the list.      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        03/25/95             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
int16 tcp_ooo_packet(PORT *prt, int32 seq)
{
    TCP_WINDOW      *wind;
    NET_BUFFER  *buf_ptr;
    NET_BUFFER  *curr_buf, *prev_buf;

    wind = &prt->in;
    curr_buf = wind->ooo_list.head;

    /* First check to see if there are currently any packets in the out of order
       list.  If not then add this one and return.  Else search for the position
       at which this buffer should be inserted. */
    if (curr_buf == NU_NULL)
    {
        /* Move the buffer from the buffer_list to the out of order list. */
        buf_ptr = MEM_Update_Buffer_Lists (&MEM_Buffer_List, &wind->ooo_list);
    }
    else
    {
        /* Search for the first packet that has a sequence number that is not
           less than the sequence number of the packet that is being inserted. 
         */
        while( (curr_buf != NU_NULL) && (INT32_CMP(seq, curr_buf->mem_seqnum) > 0) )
        {
            prev_buf = curr_buf;
            curr_buf = (NET_BUFFER *)curr_buf->next;
        }

        /* If a packet with a larger sequence number could not be found then add
           the new packet to the end of the list. */
        if (curr_buf == NU_NULL)
        {
            /* Move the buffer from the buffer_list to the out of order list. */
            buf_ptr = MEM_Update_Buffer_Lists (&MEM_Buffer_List, &wind->ooo_list);
        }

        /* If a packet with this sequence number is already in the list, then
           return a failure status. */
        else if (INT32_CMP(seq, curr_buf->mem_seqnum) == 0)
		{
            return (-1);
		}
        else
        {
            /* Insert this buffer into the list. */
            buf_ptr = (NET_BUFFER *)MEM_Buffer_Dequeue(&MEM_Buffer_List);

            MEM_Buffer_Insert(&wind->ooo_list, buf_ptr, curr_buf, prev_buf);
        }
    }

    /* Initialize the sequence field in the buffer. */
    buf_ptr->mem_seqnum = seq;

    return (NU_SUCCESS);
} /* tcp_ooo_packet */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      check_ooo_list                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  Checks the out of order list for the packet that is expected next.   */
/*  If the next packet is found it is placed in the packet list and      */
/*  the next ack sent will acknowledge this packet.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*      estab1986                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*      MEM_Update_Buffer_Lists                                          */
/*      INT32_CMP                                                        */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*      prt             Pointer to a port.                               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NONE                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        03/25/95             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
VOID check_ooo_list(PORT *prt)
{

    int32 nxt;
    NET_BUFFER *curr_buf;
    NET_BUFFER *buf_ptr;

    struct sock_struct *sock_ptr = socket_list[prt->p_socketd];

    nxt = prt->in.nxt;
    curr_buf = prt->in.ooo_list.head;

    /* First check to see if there are any packets on the ooo (out of order)
       list that are older than the one we expect to receive next.  If there are
       any such packets then deallocate them. */
    while( (curr_buf != NU_NULL) && (INT32_CMP(nxt, curr_buf->mem_seqnum) > 0) )
    {
        MEM_Update_Buffer_Lists(&prt->in.ooo_list, &MEM_Buffer_Freelist);
        curr_buf = prt->in.ooo_list.head;
    }

    /* Make sure we are still pointing at the head. */
    curr_buf = prt->in.ooo_list.head;

    /* As long as the packet that is expected next is on the ooo list get it and
       place it on the packet list. */
    while( (curr_buf != NU_NULL) && (INT32_CMP(nxt, curr_buf->mem_seqnum) == 0) )
    {
        /* Place the packet onto the packet list. */
        buf_ptr = MEM_Update_Buffer_Lists(&prt->in.ooo_list, &sock_ptr->s_recvlist);

        /* Update the expected sequence number. */
        prt->in.nxt += buf_ptr->mem_tcp_data_len;

        /* Decrease the amount of space in the receive window. */
        prt->in.size -= buf_ptr->mem_tcp_data_len;

        /* Increase the number of bytes that are buffered in the window. */
        sock_ptr->s_recvbytes += buf_ptr->mem_tcp_data_len;
        sock_ptr->s_recvpackets++;

        nxt = prt->in.nxt;

        /* Set the current pointer equal to the new head of the OOO list.  The
           old head was removed by MEM_Update_Buffer_Lists() above.  Doing the 
           following will not work: "curr_buf = curr_buf->next;".  This is because 
           the call to MEM_Update_Buffer_Lists() above places the current buffer 
           at the tail of the in window's packet list and in the process sets the 
           next pointer to NULL.  So curr_buf would then be set to NULL 
        */
        curr_buf = prt->in.ooo_list.head;

    }

}  /* check_ooo_list */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tcp_xmit                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for sending data.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*      windowprobe                                                      */
/*      netsend                                                          */
/*      NU_EventsDispatcher.                                             */
/*      netclose                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*      tcp_update_headers                                               */
/*      tcpsend                                                          */
/*      SCK_Suspend_Task                                                 */
/*      UTL_Timerset                                                     */
/*                                                                       */
/* INPUTS                                                                */
/*      prt             Pointer to a port.                               */
/*      buf_ptr         Pointer to a packet buffer.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes sent.                                        */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        04/13/95             Created Initial version.   */
/* Maiqi Qian      12/11/96         Added buf->option_len for tcpsend(). */
/*                                                                       */
/*************************************************************************/
STATUS tcp_xmit(PORT *prt, NET_BUFFER *buf_ptr)
{
    STATUS          stat;
    int16           tcp_hlen;
    SCK_SOCKADDR_IP dest;
    NU_TASK         *task_ptr;

    /* Store the sequence number of this packet. */
    buf_ptr->mem_seqnum = prt->out.nxt;

    /* Calculate the next sequence number to be sent. */
    prt->out.nxt += buf_ptr->mem_tcp_data_len;

    /* Compute the TCP header size in words. */
    tcp_hlen = ((sizeof(TCPLAYER) + buf_ptr->mem_option_len - 1) / 4) + 1;

    /* Update the header information. */
    tcp_update_headers(prt, longswap(buf_ptr->mem_seqnum), tcp_hlen);

    /* Send the packet. */
    stat = tcpsend( prt, buf_ptr);

    if (stat == NU_UNRESOLVED_ADDR)
    {
        task_ptr = NU_Current_Task_Pointer();
       
        if ( (task_ptr != &NU_EventsDispatcher_ptr) && 
             (task_ptr != &timer_task_ptr) )
        {
            SCK_Suspend_Task(task_ptr);

            dest.sck_addr = *(uint32 *)prt->tcp_faddr;

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

    /* Clear the PUSH bit. It may or may not have been sent. */
    prt->tcpout.flags &= ~TPUSH;

    return(stat);

} /* end tcp_xmit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      checkListeners                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function checks for listeners on a port.                        */
/*                                                                       */
/* CALLED BY                                                             */
/*      TCP_Interpret                                                    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*      portnum         Port number to check for listeners.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_TRUE if a listener is found, NU_FALSE otherwise.              */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        05/15/95             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
int16 checkListeners(int16 portnum)
{
    struct TASK_TABLE_STRUCT *Task_Entry;

    Task_Entry = Task_Head;

    while (Task_Entry != NU_NULL)
    {
         /* Search for a task that is listening on portnum */
         if (Task_Entry->local_port_num == portnum)
         {
            return(NU_TRUE);
         }  /* end if myport matches local port num */

         /* continue checking the next structure */
         Task_Entry = Task_Entry->next;
    } /* end while Task_Entry != NU_NULL */

    return(NU_FALSE);
} /* checkListeners */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCP_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     This function cancels pending events for a connection when it     */
/*     closes.  It also clears out the various packet lists associated   */
/*     with a particular connection.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      tcpdo                                                            */
/*      ackcheck                                                         */
/*      tcp_retransmit                                                   */
/*      NU_Abort                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Timerunset                                                   */
/*      MEM_Buffer_Cleanup                                               */
/*      NU_Resume_Task                                                   */
/*      NU_Deallocate_Memory                                             */
/*      RTAB_Free                                                        */
/*                                                                       */
/* INPUTS                                                                */
/*      struct port *:  Pointer to the port structure which describes    */
/*                      the closing connection.                          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS always.                                               */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        11/14/96             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
STATUS TCP_Cleanup(struct port *prt)
{
	struct sock_struct	*sockptr;

    /* Clear all retransmit events for this port. */
    UTL_Timerunset(TCPRETRANS, prt->pindex, prt->out.nxt );

    /* Clear all lists of packet buffers. */
    MEM_Buffer_Cleanup (&prt->in.ooo_list);
    MEM_Buffer_Cleanup (&prt->out.packet_list);

    /* Is there an event set to transmit a partial packet. If so clear that 
       event. */
	if (prt->xmitFlag == NU_SET)
    {
		UTL_Timerunset (CONTX, prt->pindex, (int32)1);
        prt->xmitFlag = NU_CLEAR;
    }

    /* Get a pointer to the socket. */
	sockptr = socket_list[prt->p_socketd];

	if (sockptr)
	{
		/* restart the waiting task */
		if (sockptr->s_RXTask != NU_NULL)
		{
           NU_Resume_Task(sockptr->s_RXTask);
		   sockptr->s_RXTask = NU_NULL;
		}

        /* restart the waiting task */
        if (sockptr->s_TXTask != NU_NULL)
		{
           NU_Resume_Task(sockptr->s_TXTask);
		   sockptr->s_TXTask = NU_NULL;
		}
	}
    
	/* If necessary clear out the Task_Entry structure. */
    if (prt->task_entry)
    {
        prt->task_entry->port_entry[prt->task_num] = NU_IGNORE_VALUE;
        prt->task_entry->stat_entry[prt->task_num] = NU_IGNORE_VALUE;

		/* If we get here it means that a port was closed before it was ever 
		   accepted by an application.  That means we have clear the socket as
		   well. 
		*/
          
		if (sockptr)
		{
			/* Clear out the receive list. */
			MEM_Buffer_Cleanup (&sockptr->s_recvlist);
		
			/* release the memory used by this socket */
			NU_Deallocate_Memory( (VOID *)sockptr);

			/* clear this socket pointer for future use */
			socket_list[prt->p_socketd] = NU_NULL;
		}
    }

    /* Free the route that was being used by this connection. */
    RTAB_Free(prt->tp_route.rt_route);

    return (NU_SUCCESS);

}/* end TCP_Cleanup */

