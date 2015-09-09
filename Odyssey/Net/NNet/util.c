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
*    TCP/IP kernel for NCSA Telnet                                          *
*     by Tim Krauskopf                                                      *
*                                                                           *
*     National Center for Supercomputing Applications                       *
*     152 Computing Applications Building                                   *
*     605 E. Springfield Ave.                                               *
*     Champaign, IL  61820                                                  *
*/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  util.c                                                     2.3          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Session interface routines                                              */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  NU_TCP_Time                                                             */
/*  UTL_Checksum                                                            */
/*  UTL_Clear_Matching_Timer                                                */
/*  UTL_Timerset                                                            */
/*  UTL_Timerunset                                                          */
/*  UTL_Zero                                                                */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*	NAME				DATE		REMARKS 								*/
/*                                                                          */
/* Maiqi Qian      12/06/96         Fixed the time wrap up (spr0229)        */
/*                                                                          */
/****************************************************************************/

/*
 *   Includes
 */
#ifdef PLUS
	#include "nucleus.h"
#else
	#include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
	#include "nu_extr.h"
#endif
#include "target.h"
#include "net_extr.h"
#include "protocol.h"
#include "externs.h"
#include "tcp.h"
#include "tcpdefs.h"
#include "tcp_errs.h"
#include "netevent.h"
#include "data.h"

#ifdef PLUS
  extern NU_TASK           NU_EventsDispatcher_ptr;
#endif

uint my_current_time = 55555555;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UTL_Timerset(int16, int16, int16, int16, int32)                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Set an async timer, and when time elapses sticks an event in the    */
/*   network event queue.                                                */
/*                                                                       */
/*   class, event, dat is what gets posted when howlong times out.       */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_EventsDispatcher                                              */
/*      tcpsend                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_dequeue                                                      */
/*      NU_Allocate_Memory                                               */
/*      n_clicks                                                         */
/*      TL_Put_Event                                                     */
/*                                                                       */
/*************************************************************************/
STATUS UTL_Timerset (UNSIGNED event, UNSIGNED dat, UNSIGNED howlong, 
                  int32 seq_number)
{
    tqe_t *tqe;
#ifdef PLUS
    STATUS  status;
#else
    sint status;              /* status of memory allocation */
#endif
    uint *return_ptr;

    /*  Get an entry from the freelist.  */
    tqe = (tqe_t *) dll_dequeue ((tqe_t *) &tcptimer_freelist);

    /*  Check to see if an entry was found.  If one was not found, need
        to allocate a new one. */

    if (!tqe)
	{
        /*  Get some memory for the new entry.  */
#ifdef PLUS
        status = NU_Allocate_Memory(&System_Memory, (void **) &return_ptr,
                                (UNSIGNED)sizeof(tqe_t),
                                (UNSIGNED)NU_NO_SUSPEND);
#else
        status = NU_Alloc_Memory (sizeof(tqe_t),
                              (unsigned int **)&return_ptr, NU_WAIT_FOREVER);
#endif

        /* check status of memory allocation */
		if (status == NU_SUCCESS)
        {
            return_ptr = (uint *)normalize_ptr(return_ptr);
            tqe = (tqe_t *)return_ptr;
        }
		else
		{
            NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_RECOVERABLE,
                                __FILE__, __LINE__);
			return (-1);
		}
	}

    /*  Set up the new entry.  */
	tqe->tqe_event = event;
	tqe->tqe_data = dat;
    tqe->tqe_ext_data = seq_number;
    tqe->duetime = n_clicks() + (UNSIGNED) howlong;

    /*  Place the new entry on the timerlist.  */
    tqpost ((tqe_t *) &tcp_timerlist, tqe);

    /* Check to see if the current task is the  events dispatcher.  If it
     * is we do not want to place an item onto the event queue.  If the queue
     * is full the events dispatcher will suspend on the queue, and since
     * the events dispatcher is the only task that removes items from the
     * queue, deadlock will occur.
     */
#ifdef PLUS
    if (NU_Current_Task_Pointer() == &NU_EventsDispatcher_ptr)
#else
    if (NU_Current_Task_ID() == NU_EventsDispatcherID)
#endif
    {
        return(NU_SUCCESS);
    }

    /*  Wake up the event dispatcher from an indefinite wait */
    /*  iff this is the first entry on the timer list        */
    if (tcp_timerlist.flink == tqe)
      TL_Put_Event(CONNULL, dat);

    return (NU_SUCCESS);
} /* UTL_Timerset */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UTL_Timerunset                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Remove all timer events from the queue that match the               */
/*   class/event/dat.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*      tcpdo                                                            */
/*      ackcheck                                                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      UTL_Clear_Matching_Timer                                         */
/*                                                                       */
/*************************************************************************/
STATUS UTL_Timerunset (UNSIGNED event, UNSIGNED dat, int32 ack_num)
{
   return (UTL_Clear_Matching_Timer (&tcp_timerlist, event, dat, ack_num));
}  /* UTL_Timerunset */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UTL_Clear_Matching_Timer                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Remove all timer events from the queue that match the               */
/*   class/event/dat.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*      UTL_Timerunset                                                   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_dequeue                                                      */
/*      dll_insert                                                       */
/*      dll_remove                                                       */
/*      dll_enqueue                                                      */
/*                                                                       */
/*************************************************************************/
STATUS UTL_Clear_Matching_Timer (struct tqhdr *tlist, UNSIGNED event, UNSIGNED dat, 
                             int32 ack_num)
{
    tqe_t *ent,      /* Points to the current entry in the queue. */
          *tmpent,   /* Points to the item being promoted from the duplist. */
          *savent;   /* Preserves our position in the queue. */

    /* Search the list for matching timers. */
    for (ent = tlist->flink; ent; )
    {
      /* Does this match the entry we are looking for. */
      if ( (ent->tqe_event == event) && (ent->tqe_data == dat) && 
           (INT32_CMP(ack_num, ent->tqe_ext_data) > 0) )
      {
          /* We have found a matching entry.  Preserve a pointer to the next
           * entry. */
          savent = ent->flink;

          /* If this entry contains a duplist we need to search it to. */
          if (ent->duplist.flink)
          {
             /* Search the duplist for a match. */
             UTL_Clear_Matching_Timer(&ent->duplist, event, dat, ack_num);

             /* Pull the first item, if one exists, off the duplist. */
             tmpent = (tqe_t *)dll_dequeue(&ent->duplist);

             /* If the duplist still contained an item after we cleared it,
              * we want to promote one. */
             if(tmpent)
             {
                 /* Promote this item to the position that was held by the item
                  * we are removing. */
                 tmpent->duplist.flink = ent->duplist.flink;
                 tmpent->duplist.blink = ent->duplist.blink;
                 if (tmpent)
                     dll_insert((tqe_t *)tlist, tmpent, ent);
             }
          }

          /* Remove the item. */
          dll_remove((tqe_t *)tlist, ent);

          /* Place the item back on the free list. */
          dll_enqueue((tqe_t *)&tcptimer_freelist, ent);

          /* Point to the next item to be checked. */
          ent = savent;
      }
      else
      {
          /* We did not find a match.  Clear the duplist if it exists. */
          if (ent->duplist.flink)
          {
              UTL_Clear_Matching_Timer(&ent->duplist, event, dat, ack_num);
          }

          /* Point to the next item to be checked. */
          ent = ent->flink;
      }
    }

  return (NU_SUCCESS);
}  /* UTL_Clear_Matching_Timer */


#ifndef MSC
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_TCP_Time                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*      init_time                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
long    NU_TCP_Time(uint32 *current_time)
{
    long    tcp_current_time;

    tcp_current_time = my_current_time;

    if (current_time != NU_NULL)
        *current_time = my_current_time;

    return(tcp_current_time);
}
#endif /* MSC */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UTL_Zero                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function clears an area of memory to all zeros.             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      This function is called from many places.                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      <Function Called>                   <Description>                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ptr         Pointer to starting address.                         */
/*      size        The number of bytes of memory to zero.               */
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
VOID UTL_Zero(VOID *ptr, uint32 size)
{
    uint32      i;
    CHAR        *work_ptr = (CHAR *)ptr;

    for (i = 0; i < size; work_ptr[i++] = NU_NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      UTL_Checksum                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Calculate a TCP checksum.                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Send_SYN_FIN                                                     */
/*      tcp_sendack                                                      */
/*      tcpsend                                                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      tcpcheck                                                         */
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
uint16 UTL_Checksum (NET_BUFFER *buf_ptr, uint8 *source, 
                     uint8 *dest, uint8 protocol)
{
    struct pseudotcp     tcp_chk;

    tcp_chk.source  = *(uint32 *)source;
    tcp_chk.dest    = *(uint32 *)dest;
    tcp_chk.z       = 0;
    tcp_chk.proto   = protocol;
    tcp_chk.tcplen  = intswap((uint16)buf_ptr->mem_total_data_len);

    return(tcpcheck( (uint16 *) &tcp_chk, buf_ptr));

} /* UTL_Checksum */
