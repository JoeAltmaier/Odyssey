/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CmbRecvLogEntry.cpp
//
// Description:
//    CMB DDM module.  Contains member routines and a helper class
//    used for receiving event log entries relayed from IOPs via CMB.
//    (We reassemble the fragments of each entry and, once a complete
//     entry is received, we forward it on to the event log.  This
//     code is only expected to run on the master HBC.)
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbRecvLogEntry.cpp $
// 
// 3     11/24/99 3:38p Ewedel
// Added watchdog's timeout cookie.
// 
// 2     10/18/99 4:30p Ewedel
// Changed DiscardReply() to DiscardOkReply().
// 
/*************************************************************************/

//#include  "DdmCMB.h"          // we define a member routine for DdmCmb

#include  "CmbEventRecv.h"    // our helper class defs

#include  "CtEvent.h"         // standard status codes

#include  "LogMasterMessages.h"     // for sending to event logger

#include  "Odyssey_Trace.h"

#include  <assert.h>



//  how long we allow before timing out receipt of the remaining
//  fragments of an event entry:
const U32   CCmbEventRecv::CEventAssy::m_cusTimeout  =  60 * 1000000;   // 60 sec



//
//  CCmbEventRecv::ProcessFragment (pktFragment)
//
//  Description:
//    Accepts a fragment of an event log entry, and gathers it together
//    with whatever other fragments of the same log entry, from the same
//    IOP, which we have already collected.
//
//    When we are passed a fragment which finally completes a given
//    event entry, we submit the event entry for processing by the
//    event log DDM, using message MsgAddLogEntry().
//
//    We maintain a timer on each event entry for which we're
//    accumulating fragments.  If a given entry remains incomplete
//    and doesn't receive any additional fragments within a set timeout
//    period, then we assume that the other fragments got lost (or that
//    the sending IOP bit the dust) and we dispose of the partial entry.
//
//    Since the event entry fragment sender always sends fragments in
//    order (and since the first fragment contains extra header data)
//    if we receive a non-first fragment for an event entry which we
//    are not accumulating already, we simply dispose of the fragment.
//    This also serves to clean up after any entries which might have
//    timed out (see previous paragraph).
//
//    Due to possible CMB noise or spurious naks, we are careful to
//    recognize and dispose of duplicate fragments, either expressly
//    or implicitly (per the preceding paragraph) once a full event
//    entry has been assembled.  This allows us to gracefully tolerate
//    excess retries submitted by an IOP.
//    
//  Inputs:
//    pktFragment - CMB command packet containing event entry fragment
//                which we're to collect / process.
//
//  Outputs:
//    none
//

void  CCmbEventRecv::ProcessFragment (const CmbPacket& pktFragment)
{

CEventAssy   * pEvent;


   assert (pktFragment.Hdr.bCommand == k_eCmbCmdSendLogEntry);
   assert ((pktFragment.Hdr.bStatus & CmbStatCmd) != 0);
   assert (pktFragment.Hdr.cbData > 2);   // our meta-header, plus data

   //  first, let's see if we are already accumulating fragments
   //  for this event log entry
   pEvent = m_pEventList;
   while ((pEvent != NULL) && ! pEvent->IsMine (pktFragment))
      {
      //  not this event's fragment, keep looking
      pEvent = pEvent->m_pNext;
      }

   if (pEvent == NULL)
      {
      //  fragment not merged into an existing event assembler,
      //  see if it's a first-fragment:
      if (pktFragment.abTail[offCmbSendEvtHdrFragSeqNum] ==
                  CmbSendEvtFirstFragSeqNum)
         {
         //  yup, got a first fragment.  Make a new event assembler,
         //  and add it to our list:
         pEvent = new CEventAssy (m_pParentDdm, pktFragment,
                                  EventCompletionCallback, this);

         //  just add new one to head -- we don't worry about any
         //  fancy sorting or other containers for now..
         pEvent->m_pNext = m_pEventList;
         m_pEventList = pEvent;
         }
      else
         {
         //  whoops, got a non-first fragment with no matching
         //  event assembler.  Blow it off.
         Tracef ("CCmbEventRecv::ProcessFragment: dropping unmatched fragment\n"
                 "                    SRC=%02d, evt seq=%d, frag seq=%d\n",
                 pktFragment.Hdr.bSrcAddr,
                 pktFragment.abTail[offCmbSendEvtHdrEventSeqNum],
                 pktFragment.abTail[offCmbSendEvtHdrFragSeqNum]);
         }
      }
   else
      {
      //  got a match with an existing event assembler, feed it our packet
      pEvent->AddFragment (pktFragment);
      }

   //  all done
   return;

}  /* end of CCmbEventRecv::ProcessFragment */

//
//  CCmbEventRecv::EventCompletionCallback (pCompletedEvent, pvMyInstance)
//
//  Description:
//    Called by an event assembler instance, when it has completed
//    its processing.  When we receive this call, the assembler has
//    already forwarded the event to the event log, and is ready to
//    be deleted.
//
//    We remove the event assembler instance from our list of outstanding
//    assemblers, and delete it.
//    
//  Inputs:
//    pCompletedEvent - Points to event assembler instance which has
//                completed its activity.
//    pvMyInstance - Veiled pointer to our own instance, which contains
//                *pCompletedEvent in its list of outstanding events.
//
//  Outputs:
//    none
//

/* static */
void  CCmbEventRecv::EventCompletionCallback (CEventAssy *pCompletedEvent,
                                              void *pvMyInstance)
{

CCmbEventRecv   * const This = (CCmbEventRecv *) pvMyInstance;
CEventAssy      * pEvent;


   //  we may get called twice for a given event assembler, so
   //  see if completed event is still in our container (list):

   if (pCompletedEvent == This->m_pEventList)
      {
      //  yup, it's right there at the head of our list.  Remove it.
      This->m_pEventList = This->m_pEventList->m_pNext;

      //  and dispose of event assembler
      delete pCompletedEvent;
      }
   else
      {
      //  not at head of our list, so do a search

      pEvent = This->m_pEventList;

      while ((pEvent->m_pNext != NULL) &&
             (pEvent->m_pNext != pCompletedEvent))
         {
         //  advance to next entry
         pEvent = pEvent->m_pNext;
         }

      if (pEvent->m_pNext == pCompletedEvent)
         {
         //  found it in our list, remove it.
         pEvent->m_pNext = pEvent->m_pNext->m_pNext;

         //  and dispose of event assembler
         delete pCompletedEvent;
         }
      else
         {
         //  whoops, event assembler not found.  This should never happen.
         assert (pEvent->m_pNext == pCompletedEvent);
         }
      }

   //  event unlinked and deleted, if it was present in our list
   return;

}  /* end of CCmbEventRecv::EventCompletionCallback */

//
//  CEventAssy::CEventAssy (pParentDdm, pktFirstFrag, 
//                          pfnCompletionCallback, pvCompletionCookie)
//
//  Description:
//    Our only constructor.
//
//    Builds an instance of CEventAssy, and populates it with the first
//    fragment of the event it is to assemble.
//
//    Of course, we're also careful to start a watchdog timer running
//    in case we don't get any more fragments of this event.
//    
//  Inputs:
//    pParentDdm - Points to instance of our parent DDM.  Our base class
//                cares about this.
//    pktFirstFragment - Fragment whose receipt triggered our creation.
//                We save a copy of the fragment, and initialize our
//                instance based on its parameters.
//    pfnCompletionCallback - Points to simple (static) callback function
//                used to notify our container when our assembly
//                operation is completed.
//    pvCompletionCookie - "Cookie" value which is passed to
//                *pfnCompletionCallback when we invoke it.  This is
//                typically an instance pointer of our container.
//
//  Outputs:
//    none
//

CCmbEventRecv::
CEventAssy::CEventAssy (DdmServices *pParentDdm, const CmbPacket& pktFirstFrag,
                        CompletionCallback pfnCompletionCallback,
                        void *pvCompletionCookie) :
               m_eOriginIop ((TySlot) pktFirstFrag.Hdr.bSrcAddr),
               m_bEventSeqNum (pktFirstFrag.abTail[offCmbSendEvtHdrEventSeqNum]),
               m_cFragsTotal (pktFirstFrag.abTail[offCmbSendEvtHdrFragCount]),
               m_pfnCompletionCallback (pfnCompletionCallback),
               m_pvCompletionCookie (pvCompletionCookie),
               CCtTimedService (pParentDdm)
{


   //  check a few little details..
   assert (pktFirstFrag.Hdr.bCommand == k_eCmbCmdSendLogEntry);
   assert (pktFirstFrag.Hdr.cbData > sizeof (CmbSendEvtHdr));
   assert (pktFirstFrag.abTail[offCmbSendEvtHdrFragSeqNum] ==
                                          CmbSendEvtFirstFragSeqNum);
   assert (pktFirstFrag.abTail[offCmbSendEvtHdrFragCount] > 0);

   //  ok, build up frag holder for our first fragment
   m_pFrags = new CFrag (pktFirstFrag);

   //  only the frag we just added..
   m_cFragsSoFar = 1;

   m_cbFragDataSoFar = pktFirstFrag.Hdr.cbData - sizeof (CmbSendEvtHdrFirst);

   //  now establish a watchdog timer on remaining fragments
   StartWatchdogTimer (m_cusTimeout, WDTIMEOUTCALLBACK (CEventAssy, TimeoutCB),
                       WDTIMEROPCALLBACK (CEventAssy, InertTimerCallback));

   return;

}  /* end of CEventAssy::CEventAssy */


//  This here continuation routine merely exists to catch our watchdog's
//  callback signifying that the timer is (re-) started.
//  This is a WDTIMEROPCALLBACK()-conformant callback.
//
//  Inputs:
//    sResult - Indicates whether watchdog timer was started successfully.
//
//  Ouputs:
//    none
//

void  CCmbEventRecv::CEventAssy::InertTimerCallback (STATUS sResult)
{

   #pragma unused(sResult)
   
   return;

}  /* end of CEventAssy::InertTimerCallback */

//
//  CEventAssy::AddFragment (pktFragment)
//
//  Description:
//    Adds the given fragment to our collection.
//
//    If this completes the event we've been assembling, then we
//    send it off to the event log, and signal our container that
//    we're completed.
//    
//  Inputs:
//    pktFragment - Fragment to add to our collection.  This might be
//                a duplicate of one we already have, in which case
//                we ignore it.
//
//  Outputs:
//    none
//

void  CCmbEventRecv::CEventAssy::AddFragment (const CmbPacket& pktFragment)
{

CFrag  * pFrag;
U8       bNewFragSeq;
U8       bNextEntryFragSeq;


   assert (pktFragment.Hdr.bCommand == k_eCmbCmdSendLogEntry);
   assert (pktFragment.Hdr.cbData > sizeof (CmbSendEvtHdr));
   assert (IsMine (pktFragment));
   assert (pktFragment.abTail[offCmbSendEvtHdrFragSeqNum] < m_cFragsTotal);

   //  we should never already have a complete set of fragments..
   assert (m_cFragsSoFar < m_cFragsTotal);

   //  now then, zip through our list checking to see if frag seqnum is there
   bNewFragSeq = pktFragment.abTail[offCmbSendEvtHdrFragSeqNum];

   //  did we get a re-send of fragment 0?
   if (bNewFragSeq == CmbSendEvtFirstFragSeqNum)
      {
      //  yup, it's a duplicate.  Simply ignore it.
      return;
      }

   //  all other fragments have higher seqnums than the list head
   assert (bNewFragSeq > CmbSendEvtFirstFragSeqNum);

   //  we keep our list sorted by ascending frag seqnum..

   pFrag = m_pFrags;
   while (pFrag->pNext != NULL)
      {
      //  we use pFrag as a sort of "pPrev" value.  So grab "pCur"'s
      //  sort key value:
      bNextEntryFragSeq =
               pFrag->pNext->pktFrag.abTail[offCmbSendEvtHdrFragSeqNum];

      if (bNewFragSeq <= bNextEntryFragSeq)
         {
         //  found a duplicate, or our insertion point
         break;
         }

      //  not yet, keep traversing list
      pFrag = pFrag->pNext;
      }

   if (pFrag->pNext == NULL)
      {
      //  normal, easy case.  Our fragment isn't a duplicate, and it goes
      //  on the end of the list:

      //  (fragments should always come in sequence:)
      assert ((pFrag->pktFrag.abTail[offCmbSendEvtHdrFragSeqNum] + 1)
                           == bNewFragSeq);

      pFrag->pNext = new CFrag (pktFragment);
      }
   else
      {
      //  the "new >= list_entry" test hit -- do we have a duplicate?
      if (bNewFragSeq == bNextEntryFragSeq)
         {
         //  yup, it's a duplicate.  Ignore it.
         return;
         }
      else
         {
         //  got a non-duplicate to insert.. (actually, this "insert in middle"
         //  case should never happen, due to the sender's structure.  But we
         //  handle it just in case.)
         assert (bNewFragSeq >
                        pFrag->pktFrag.abTail[offCmbSendEvtHdrFragSeqNum]);
         assert (bNewFragSeq < bNextEntryFragSeq);

         //  link new one in between pFrag and pFrag->pNext:
         pFrag->pNext = new CFrag (pktFragment, pFrag->pNext);
         }
      }

   //  if we get here, we added a fragment somewhere.  Track it,
   //  and see if we're done yet.

   m_cFragsSoFar ++;

   //  (also keep track of event size)
   m_cbFragDataSoFar += pktFragment.Hdr.cbData - sizeof (CmbSendEvtHdr);

   if (m_cFragsSoFar == m_cFragsTotal)
      {
      //  yow!  We've got a complete event entry, let's send it!
      SubmitEvent ();

      //  tell our watchdog timer to shut down.
      StopWatchdogTimer (WDTIMEROPCALLBACK (CEventAssy, AddFragment2));

      //  (our continuation routine will take care of our container)
      }
   else
      {
      //  we're not complete yet, but we did add a fragment.
      //  So let's restart our watchdog timer.
      //
      //  IMPORTANT:  we don't want to do this until we're done
      //  with our list update, since we might have a multi-"thread"
      //  intertwining while the timer code transfers us to our
      //  continuation routine.
      ResetWatchdogTimer (WDTIMEROPCALLBACK (CEventAssy, InertTimerCallback));
      }

   //  all done
   return;

}  /* end of CEventAssy::AddFragment */


//  This continuation routine catches our timer shutdown result.
//
//  We don't so much care what it is, but must wait until the timer
//  is shut down, to ensure that we don't get into a situation where
//  we signal completion, and then our timeout callback is called
//  and also signals completion.
//
//  By definition of CCtTimedService, once we get to this callback
//  we are assured that we will not receive a timeout callback.
//  However, our instance must exist until the time of this
//  callback's invocation, since it may be required for various
//  operations / callbacks internal to CCtTimedService.  And of course,
//  for this callback itself.  :-)
//
//  Inputs:
//    sResult - Result of stopping watchdog timer.
//
//  Ouputs:
//    none
//

void  CCmbEventRecv::CEventAssy::AddFragment2 (STATUS sResult)
{

   #pragma unused(sResult)
   
   //  now that our timer is shut off, let our container know that we're finis.
   DoCompletionCallback ();

   return;

}  /* end of CEventAssy::AddFragment2 */

//
//  CEventAssy::SubmitEvent ()
//
//  Description:
//    A little helper routine.  Bundles up all the logic for assembling
//    and sending an event, once we have received all of its fragments.
//    
//  Inputs:
//    none
//
//  Outputs:
//    none
//

void  CCmbEventRecv::CEventAssy::SubmitEvent ()
{

U8              * pbEventBuf;
U8              * pbCur;
U32               cbFrag;
CFrag           * pFrag;
Event           * pEvent;
MsgAddLogEntry  * pmsgAddLogEntry;


   //  first, gather all fragments into one contiguous buffer, which
   //  is the packed form of our event:
   pbEventBuf = new U8 [m_cbFragDataSoFar];

   //  copy first fragment, whose size is a bit different
   cbFrag = m_pFrags->pktFrag.Hdr.cbData - sizeof (CmbSendEvtHdrFirst);
   memcpy (pbEventBuf,
           m_pFrags->pktFrag.abTail + sizeof (CmbSendEvtHdrFirst),
           cbFrag);

   pbCur = pbEventBuf + cbFrag;
   pFrag = m_pFrags->pNext;

   //  do all non-first fragments:
   while (pFrag != NULL)
      {
      cbFrag = pFrag->pktFrag.Hdr.cbData - sizeof (CmbSendEvtHdr);
      memcpy (pbCur, pFrag->pktFrag.abTail + sizeof (CmbSendEvtHdr), cbFrag);

      pbCur += cbFrag;
      pFrag = pFrag->pNext;
      }

   //  now convert event to unpacked form (does a copy):
   pEvent = new Event (pbEventBuf);

   //  all done with our own packed event buffer, so let's zap it
   delete [] pbEventBuf;

   //  now build up log-add message (also does a copy):
   pmsgAddLogEntry = new MsgAddLogEntry (pEvent);

   //  sneak in a little debug output, since this is nice to know:
   Tracef ("CCmbEventRecv::CEventAssy::SubmitEvent():  submitting an event log entry!\n"
           "         event code = %X, param cnt = %d,\n"
           "         src slot = %d, src DID = %d\n",
           pEvent->GetEventCode(), pEvent->GetParameterCount(),
           pEvent->GetSlot(), pEvent->GetDID());

   //  and dispose of event instance
   delete pEvent;

   //  finally, send off add message:
   Send (pmsgAddLogEntry, NULL, REPLYCALLBACK (CEventAssy, DiscardOkReply));

   return;

}  /* end of CEventAssy::SubmitEvent */

//
//  CEventAssy::TimeoutCB (pvTimeoutCookie)
//
//  Description:
//    This routine is called when our watchdog timer expires.
//    If this happens, it means that whoever was sending us event
//    fragments seems to have given up the ghost.
//
//    Rather than sitting around using up memory, we now go ahead
//    and tell our container that we're through (and should be
//    disposed of).
//
//    If the event sender does come back to life, its event data
//    will be lost, unless it resends the first fragment.
//    Life is that way, sometimes.
//    
//  Inputs:
//    pvTimeoutCookie - Generic cookie, which we don't use.
//
//  Outputs:
//    none
//

void  CCmbEventRecv::CEventAssy::TimeoutCB (void * /* pvTimeoutCookie */ )
{

   //  our timer's already stopped, so let our container know we're done.
   DoCompletionCallback ();

   return;

}  /* end of CEventAssy::TimeoutCB */


