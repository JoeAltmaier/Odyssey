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
/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      DLL.C                                           NET 4.0          */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Linked list routines used by tcp/ip                                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*     dll_insert                       Address resolution               */
/*     dll_enqueue                      Send out an arp request packet   */
/*     dll_dequeue                      Interpret ARP Packets            */
/*     dll_remove                       Request local IP number          */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     externs.h         External definitions for functions in NCSA      */
/*                       Telnet.                                         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*
 * These routines all reference a linked-list header, used to point to
 * a particular list.  Each entry in the list is prefixed by two pointer
 * values, namely a forward pointer and a backward pointer:
 *
 *               struct *flink,*blink;
 *               <Application-specific structure fields>
 *                           .
 *                           .
 *                           .
 *               <end of structure>
 *
 * Internally, the linked list routines operate on only the first two
 * entries in the structure, the "flink" or forward link, and the "blink"
 * the backward link.
 *
 * A linked list header that identifies the beginning of a particular list
 * is a single pointer value.  This pointer value contains a pointer to
 * the first entry ("head") on the list.  The "blink" value of the first entry
 * on the list points to the last entry on the list.
 */

#include "protocol.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "data.h"
#include "tcp_errs.h"
#include "externs.h"

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      dll_insert                                      4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Insert an item into a linked list just before lpos.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*      tqpost                          Insert a timer queue entry into  */
/*                                      a delta list.                    */
/*      UTL_Clear_Matching _Timer       Remove all timer events from the */
/*                                      queue that match.                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/*************************************************************************/
void *dll_insert(void *h, void *i, void *l)
{
    tqe_t   *hdr = (tqe_t *)h;
    tqe_t   *item = (tqe_t *)i;
    tqe_t   *lpos = (tqe_t *)l;


  /* Make item's flink point to lpos */
  item->flink = lpos;

  /* Set item's blink to point to the node that currently precedes lpos */
  item->blink = lpos->blink;

  /*  If there is already a node in front of lpos, we want its flink to
   *  point to item.
   */
  if (lpos->blink)
    lpos->blink->flink = item;
  
  /* Set lpos's blink to point at item */
  lpos->blink = item;

  /* If lpos was the first node in the linked list.  We need to make
   * hdr's flink point to item, which is the new first node.
   */
  if (lpos == hdr->flink)
    hdr->flink = item;

  return(item);
} /* dll_insert */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      dll_enqueue                                     4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Insert an item at the end of a linked list.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_EventsDispatcher             Dispatches events from the event */
/*                                      queue.                           */
/*      tqpost                          Insert a timer queue entry into  */
/*                                      a delta list.                    */
/*      UTL_Clear_Matching _Timer       Remove all timer events from the */
/*                                      queue that match.                */
/*      ARP_RARP                        Processes RARP.                  */
/*      ARP_Resolve                     Resolve an ethernet hardware     */
/*                                      address.                         */
/*      DNS_Initialize                  Initializes the DNS component.   */
/*      DNS_Add_Host                    Adds a new host to the host file.*/
/*      DEV_Init_Devices                Initializes all devices in       */
/*                                      Nucleus Net.                     */
/*      IP_Reassembly                   Reassemble IP fragments into a   */
/*                                      complete datagram.               */
/*      Net_Ether_Send                  Hardware layer transmission      */
/*                                      function fo ethernet.            */
/*      RTE_Init_Route_Table            Initializes the routing table.   */

/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Control_Interrupts           Turns interrupts on or off.      */
/*                                                                       */
/*************************************************************************/
void *dll_enqueue(void *h, void *i)
{
    tqe_t *hdr = (tqe_t *)h;
    tqe_t *item = (tqe_t *)i;
#ifdef PLUS
    INT old_level;
#else
    int old_level;
#endif

    /*  Temporarily lockout interrupts to protect the global buffer variables. */
    old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Set item's flink to point at NULL */
    item->flink = (tqe_t *)NU_NULL;

    /*  If there is currently a node in the linked list, we want add
     *  item after that node
    */
    if (hdr->flink) {
        /* Make the last node's flink point to item */
        hdr->blink->flink = item;

        /* Make item's blink point to the old last node */
        item->blink = hdr->blink;

        /* Make hdr's blink point to the new last node, item */
        hdr->blink = item;
    }
    /*  if the linked list was empty, we want both the hdr's flink and
     *  the hdr's blink to point to item.  Both of item's links will
     *  point to NULL, as there are no other nodes in the list
    */
    else {
        hdr->flink = hdr->blink = item;
        item->blink = (tqe_t *)NU_NULL;
    }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

    return(item);
} /* end dll_enqueue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      dll_dequeue                                     4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Remove and return the first node in a linked list.                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_EventsDispatcher             Dispatches events from the event */
/*                                      queue.                           */
/*      UTL_Clear_Matching _Timer       Remove all timer events from the */
/*                                      queue that match.                */
/*      UTL_Timerset                    Sets an async timer and when time*/
/*                                      elapses it sticks an event in the*/
/*                                      network event queue.             */
/*      dll_remove                      Request local IP number          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Control_Interrupts           Turns interrupts on or off.      */
/*                                                                       */
/*************************************************************************/
void *dll_dequeue(void *h)
{
   tqe_t *hdr = (tqe_t *)h;
   tqe_t *ent;
#ifdef PLUS
    INT old_level;
#else
    int old_level;
#endif

   /*  Temporarily lockout interrupts to protect the global buffer variables. */
   old_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

  /* Create a pointer to the first node in the linked list */
  ent = hdr->flink;

  /* If there is a node in the list we want to remove it. */
  if (ent) {
      /* Make the hdr point the second node in the list */
      hdr->flink = ent->flink;

      /*  If there was a second node, we want that node's blink to at 0. */
      if (hdr->flink)
          hdr->flink->blink = (tqe_t *)0;

      /* Clear the next and previous pointers.*/
      ent->flink = ent->blink = NU_NULL;

  }

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(old_level);

  /* Return a pointer to the removed node */
  return(ent);
}  /* end dll_dequeue */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                           VERSION            */
/*                                                                       */
/*      dll_remove                                      4.0              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Remove a node and return a pointer to the succeeding node.          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ARP_RARP                        Processes RARP.                  */
/*      ARP_Interpret                   Interprets ARP packets.          */
/*      ARP_Event                       Process an ARP Timer Event.      */
/*      IP_Reassembly                   Reassemble IP fragments into a   */
/*                                      complete datagram.               */
/*      IP_Free_Queue_Element           Free a fragment reassembly header*/
/*                                      and any attched fragments.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_dequeue                     Remove and return the first node */
/*                                      in a linked list.                */
/*                                                                       */
/*************************************************************************/
void *dll_remove(void *h, void *i)
{
    tqe_t     *hdr = (tqe_t *)h;
    tqe_t     *item = (tqe_t *)i;
    tqe_t     *ent, *retval;


  /*  Search the linked list until item is found or the end of the list
   *  is reached.
   */
  for (ent = hdr->flink;( (ent) && (ent != item) ); ent = ent->flink)
    ;

  /*  If item was not before reaching the end of the list, return a
   *  pointer to 0.
   */
  if (!ent) {
      return( (tqe_t *)0 );
  }

  /* Keep a pointer to the node to be returned */
  retval = item->flink;

  /* If we're deleting the list head, this is just a dequeue operation */
  if (hdr->flink == item)
      dll_dequeue(hdr);
  else
      /*  If we are deleting the list tail, we need to reset the tail pointer
       *  and make the new tail point forward to 0.
       */
      if (hdr->blink == item) {
          hdr->blink = item->blink;
          hdr->blink->flink = (tqe_t *)0;
      }
      else  /* We're removing this entry from the middle of the list */
      {
          item->blink->flink = item->flink;
          item->flink->blink = item->blink;
      }

  /* return a pointer to the removed node */
  return(retval);
} /* dll_remove */

