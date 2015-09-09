/*************************************************************************/
/*                                                                       */
/*    CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.            */
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
/*      TIMER.C                                           4.0            */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Timer event queue routines used by tcp/ip                           */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*     tqpost                           Insert a timer event into the    */
/*                                      timer queue                      */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     externs.h         External definitions for functions in NCSA      */
/*                       Telnet.                                         */
/*     protocol.h        External structure definitions for each type    */
/*                       of prtocol this program handles.                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#include "protocol.h"
#include "externs.h"

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      tqpost                                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Insert a timer queue entry into a delta list.  The delta list       */
/*   queue is ordered by increasing due time.                            */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_EventsDispatcher                                              */
/*      UTL_Timerset                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      dll_enqueue                                                      */
/*      dll_insert                                                       */
/*                                                                       */
/*************************************************************************/
int16 tqpost(tqe_t *tqlist, tqe_t *tqe)
{
    tqe_t    *tqpos;
#ifdef PLUS
    UNSIGNED duetime = tqe->duetime;   /*  Pick up the duetimer for the new entry. */
#else /* PLUS */
    int16    duetime = tqe->duetime;
#endif /* PLUS */


  /*  Clear out the new entry's duplist. */
  tqe->duplist.flink = (tqe_t *)NU_NULL;
  tqe->duplist.blink = (tqe_t *)NU_NULL;

  /*  Search to see if this timer needs to be inserted into the list or
      if it is a duplicate entry.  */
  for (tqpos = tqlist->flink; tqpos; tqpos = tqpos->flink)
    if ( (duetime == tqpos->duetime) || (duetime < tqpos->duetime) )
      break;

  /*  If we found something, the new item needs to be added to a duplist or
      it needs to be inserted.  */
  if (tqpos)
  {

    /*  It needs to be added to a duplist. */
    if (tqe->duetime == tqpos->duetime)
    {
      dll_enqueue((tqe_t *) &tqpos->duplist, tqe);
    }
    else
    {
      /*  It needs to be inserted. */
      dll_insert((tqe_t *) tqlist, tqe, tqpos);
    }
  }
  else
    /*  If we did not find that the new entry has an equal time (duplicate) or
        needed to be inserted, then add it to the end of the list.  */
    dll_enqueue((tqe_t *) tqlist, tqe);

  return(NU_SUCCESS);

} /* tqpost */
