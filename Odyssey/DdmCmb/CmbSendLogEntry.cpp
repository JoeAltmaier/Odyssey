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
// File: CmbSendLogEntry.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    event log entry relay requests.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbSendLogEntry.cpp $
// 
// 5     1/20/00 4:11p Eric_wedel
// Fixed callback target signatures.
// 
// 2     8/24/99 8:07p Ewedel
// Changed EventLogEntrySignalCode() to eSigSendEventLogEntry -- no longer
// publicly exposed.
// 
// 1     8/11/99 7:56p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMB.h"          // we define a member routine for DdmCmb

#include  "CmbEventSend.h"    // our helper class defs

#include  "CmbSendEventDefs.h"   // some common CMB event sending defs

#include  "CtEvent.h"         // standard status codes

#include  <stdlib.h>          // for min()


#include  "Odyssey_Trace.h"

#include  <assert.h>


//  rolling sequence number used to identify all fragments of a given
//  event log entry.  Note that this sequence number is unique to the
//  IOP it is stored on.  The HBC must factor packet source into its
//  fragment reassembly logic, in addition to this seqnum.
U8    CCmbEventSend::m_bThisIopSeqNum  =  1;

//  count of bytes used in our per-fragment header data:
//    first byte is IOP-specific event seqnum, second byte is intra-event
//    fragment seqnum
const U32  CCmbEventSend::m_cbMyHeader  =  sizeof (CmbSendEvtHdr);



//  helper for queuing event sender instances, so we don't overrunb
//  the lower-level CMB interface support.
CCmbEventSend   * CCmbEventSend::m_pCurSender  =  NULL;


//
//  CDdmCMB::SendEventLogEntry (nSignal, pvEvent)
//
//  Description:
//    Handler for our eSigSendEventLogEntry CHAOS signal code.
//
//    This signal is sent to our IOP-based incarnation(s) when some DDM
//    (usually just the event log's IOP-based agent DDM) wishes to record
//    an event log entry, but can't presently access the HBC via
//    conventional channels.
//
//    This signal is originated when some other DDM on the IOP calls
//    our static member CDdmCMB::ForwardEventLogEntry().  This member
//    in turn sends a signal to our DDM instance, passing along the
//    supplied Event instance as a parameter.  And then, here we are.  :-)
//
//    We pick up the supplied Event instance, obtain its compact binary
//    representation, and send it across the CMB to the master HBC.
//    If necessary, we will fragment the log entry to ensure that it fits
//    within our rather modest CMB packet payload size.
//
//    ** Each fragment of a log entry is sent on a best-effort datagram
//       basis.  This means there is a finite (but small) chance that
//       a given log entry will not be successfully delivered to the HBC.
//    
//  Inputs:
//    nSignal - Signal code for which we're invoked.  Should always be
//                eSigSendEventLogEntry.
//    pvEvent - Points to an instance of class Event (include\oos\event.h)
//                containing the record we're to forward to the HBC.
//
//  Outputs:
//    CDdmCMB::SendEventLogEntry - Always returns CTS_SUCCESS -- other
//                values are reserved for future use by CHAOS.
//

STATUS CDdmCMB::SendEventLogEntry (SIGNALCODE nSignal, void *pvEvent)
{

Event     * pEvent  =  (Event *) pvEvent;
CmbPacket   pkt;


   assert (nSignal == eSigSendEventLogEntry);

//BUGBUG - the following code is correct, but impedes single-CPU debug:
//*   if (m_WeAreMasterHbc)
//*      {
//*      //  we should never be called when *we* are the master HBC:
//*      assert (! m_WeAreMasterHbc);
//*
//*      //  better just bail out now.  (We assert()ed this above.)
//*      return (CTS_SUCCESS);
//*      }
//BUGBUG - endit

   //  got to crack open our payload, figure out how many messages
   //  it takes, and send it across to the HBC.

   //  we have a little helper class which does all this for us.
   //  Launch an instance and it'll do the rest:
   //  (consumes pEvent for us)
   //  [for now, use the easy CHAOS-maintained master slot ID
   //   as our event destination]
   new CCmbEventSend (this, m_CmbHwIntf, pEvent);
   
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::SendEventLogEntry */

//
//  CCmbEventSend::CCmbEventSend (pParentDdm, CmbHwIntf, pEvent)
//
//  Description:
//    Constructs an instance of CCmbEventSend, and starts it working
//    on sending the specified Event instance off to the current master
//    HBC.
//
//    Our newly-constructed instance then handles the remainder of the
//    event log entry transmission all by itself (with help from the
//    CMB's hardware interface).  The CMB DDM is mostly only involved
//    in creating our instance.  Thereafter we do it by ourself.
//
//    Since event log entries, even in packed form, presently exceed
//    the size of a CMB packet, we perform necessary fragmentation and
//    sequencing in sending them across to the HBC.
//
//    In order to avoid overrunning our underlying CMB transport
//    interface, we serialize CCmbEventSend instances within a board.
//    When a new instance is created, it normally starts to send
//    itself right away.  However, if it discovers that some other
//    instance is already sending, the new instance will instead
//    queue itself pending completion of the in-transit instance.
//
//    ** Each fragment of a log entry is sent on a best-effort datagram
//       basis.  This means there is a finite (but small) chance that
//       a given log entry will not be successfully delivered to the HBC.
//    
//  Inputs:
//    pParentDdm - Points to DDM in whose context we receive callbacks.
//                In our case, this is always the CMB DDM.
//    CmbHwIntf - Reference to CMB hardware interface instance.  What we
//                use to talk to the CMB.
//    pEvent - Points to an instance of class Event (include\oos\event.h)
//                containing the record we're to forward to the HBC.
//
//  Outputs:
//    none
//

CCmbEventSend::CCmbEventSend (DdmServices *pParentDdm, CCmbHwIntf& CmbHwIntf,
                              Event *pEvent) :
                           DdmServices (pParentDdm),
                           m_CmbHwIntf (CmbHwIntf),
                           m_cmdFragSend (this, 0, 0),
                           m_bSeqNum (m_bThisIopSeqNum ++)
{

CCmbEventSend   * pEnd;


   assert (pEvent != NULL);
   if (pEvent == NULL)
      {
      //  hmm, nothing to send.
      delete this;         //BUGBUG - is this legal in our constructor?!?
      return;
      }

   //  extract a packed copy of the event entry - this is what we send
   m_cchPackedEvent = pEvent->GetEventData(&m_pchPackedEvent);
   assert ((m_cchPackedEvent > 0) && (m_pchPackedEvent != NULL));

   //  got packed version of event, it's now safe to dispose of original form
   delete pEvent;

   //  now then, initialize other bits of our context
   m_cchRemaining    = m_cchPackedEvent;
   m_pchRemaining    = m_pchPackedEvent;
   m_cchCurSend      = 0;
   m_cCurFragRetries = 0;
   m_iFrag           = CmbSendEvtFirstFragSeqNum;

   m_pNextSender     = NULL;

   //  our instance is all primed and ready to send.  But first,
   //  is some other instance presently sending?
   Critical section;

   if (m_pCurSender == NULL)
      {
      //  nope, nobody sending, so we'll start.
      m_pCurSender = this;

      //  let our continuation routine light off the first send
      SendNextFragment ();
      }
   else
      {
      //  somebody else already sending, so simply queue ourself.
      pEnd = m_pCurSender;
      while (pEnd->m_pNextSender != NULL)
         {
         pEnd = pEnd->m_pNextSender;
         }

      pEnd->m_pNextSender = this;
      m_pNextSender = NULL;
      }

   return;

}  /* end of CCmbEventSend::CCmbEventSend */

//
//  CCmbEventSend::SendNextFragment ()
//
//  Description:
//    Sends the next fragment of our event data.  If we are sending
//    the first fragment, we use the modified header data which includes
//    fragment count.
//
//    When we detect that all fragments have been sent, we delete
//    our own instance -- there is presently no need for any signal
//    back to the CMB DDM, which instantiated us in the first place.
//    
//  Inputs:
//    none
//
//  Outputs:
//    none
//

void  CCmbEventSend::SendNextFragment ()
{

U32   cbThisHeader;     // this fragment's header size
U32   cbNormPayload;    // normal fragment payload size
U32   cFragsTotal;

   
   //  if there are any more fragments, send the next one
   if (m_cchRemaining > 0)
      {
      //  yup, got more stuff to send.

      //  figure out just how much more..

      cbNormPayload = CmbPacket::PayloadMaxSize() - sizeof (CmbSendEvtHdr);

      if (m_cchRemaining == m_cchPackedEvent)
         {
         //  this is first fragment of event; reserve an extra byte for
         //  fragment count
         cbThisHeader = sizeof (CmbSendEvtHdrFirst);
         }
      else
         {
         cbThisHeader = sizeof (CmbSendEvtHdr);
         }

      m_cchCurSend = CmbPacket::PayloadMaxSize() - cbThisHeader;

      //  if we have fewer bytes remaining than we have space, adjust for it
      if (m_cchRemaining < m_cchCurSend)
         {
         m_cchCurSend = m_cchRemaining;
         }

      //  build up packet (using Initialize() ensures that object,
      //  which is a part of our member data, will not attempt to
      //  self-destruct).

      m_cmdFragSend.Initialize (k_eCmbCmdSendLogEntry,
                                Address::iSlotHbcMaster | CmbAddrMips,
                                m_cchCurSend + cbThisHeader);

      //  add our own header data
      m_cmdFragSend.AddParam (offCmbSendEvtHdrEventSeqNum, m_bSeqNum);
      m_cmdFragSend.AddParam (offCmbSendEvtHdrFragSeqNum,  m_iFrag);

      assert (m_cbMyHeader == sizeof (CmbSendEvtHdr));   // "normal" header

      if (m_cchRemaining == m_cchPackedEvent)
         {
         //  this is first fragment, so add in fragment count also
         //  (this is a standard ceiling function: x = (m + n - 1) / n;
         //   the +1 accounts for missing byte in first fragment)
         cFragsTotal = (m_cchPackedEvent + 1 + (cbNormPayload - 1)) /
                           cbNormPayload;
         assert (cFragsTotal < 256);

         //BUGBUG - what do we do if packed event is too big (> 5609 bytes)?

         //  set frag count in 1st frag only!
         m_cmdFragSend.AddParam (offCmbSendEvtHdrFragCount, (U8) cFragsTotal);
         }

      //  copy current fragment data into packet's payload, just past
      //  our own header data
      memcpy (m_cmdFragSend.PacketPtr()->abTail + cbThisHeader,
              m_pchRemaining, m_cchCurSend);

      //  send it off to current HBC master
      m_cmdFragSend.Send(m_CmbHwIntf,
                         NULL,            // our instance is our cookie
                         CMBCALLBACK (CCmbEventSend, FragmentCallback));

      //  regardless of outcome, we'll be called back to handle results
      }
   else
      {
      //  all done sending this event to the HBC.

      Critical section;

      //  grab the first pending event sender, if any
      m_pCurSender = m_pNextSender;

      if (m_pCurSender != NULL)
         {
         //  got another event to send, get it started
         m_pCurSender->SendNextFragment ();
         }

      section.Leave();

      //  Now we simply dispose of ourselves, and we're finished.
      delete this;
      }

   return;

}  /* end of CCmbEventSend::SendNextFragment */

//
//  CCmbEventSend::FragmentCallback (pvCookie, status, pReply)
//
//  Description:
//    Receives the response to our previous fragment send, and
//    continues by sending the next fragment.
//
//    When we detect that all fragments have been sent, we delete
//    our own instance -- there is presently no need for any signal
//    back to the CMB DDM, which instantiated us in the first place.
//
//    ** Each fragment of a log entry is sent on a best-effort datagram
//       basis.  This means there is a finite (but small) chance that
//       a given log entry will not be successfully delivered to the HBC.
//    
//  Inputs:
//    pvCookie - Not used, our instance data does nicely instead.
//    sStatus - Result of most recent fragment send.
//    pReply - Reply packet for most recent fragment send.
//
//  Outputs:
//    none
//

void  CCmbEventSend::FragmentCallback (void *pvCookie, STATUS sStatus,
                                       const CmbPacket *pReply)
{

#pragma unused(pvCookie)
#pragma unused(pReply)


   //  do something to make sure that send went ok..
   assert (sStatus == CTS_SUCCESS);

   //BUGBUG - if status reflects anything real here, we might be able
   //  to retry the previous send.  This should be doable, as long as
   //  we don't bump our seq number, and the fragment reassembler knows
   //  how to ignore duplicates (ack response might have got lost,
   //  rather than the actual fragment-containing packet).
   if (sStatus == CTS_SUCCESS)
      {
      //  last fragment sent ok, move on to next
      m_cchRemaining -= m_cchCurSend;
      m_pchRemaining += m_cchCurSend;

      m_iFrag ++;

      //  since we're moving on to a new fragment, we've done no retries
      m_cCurFragRetries = 0;

      //  now send off next fragment (deletes our instance if all fragments
      //  have been sent)
      SendNextFragment ();
      }
   else
      {
      Tracef("CCmbEventSend::SendNextFragment: ABORT status = %X, frag = %d\n",
             sStatus, m_iFrag);

      //  might do retry stuff here (see prior BUGBUG comment for details)
      //  but that would result in retries-squared, since our underlying
      //  CCmbMsgSender helper has transparent retry support.

      //  hit our limit on retries for this fragment.  Guess we'll just
      //  abandon the send.
      delete this;
      return;

      //Note:  In order to do retry, we need to set a time using DdmTimer,
      //  accessed using the message class RqOsTimerStart and friends.
      //  Note that if we set a one-shot timer, as we should, then the
      //  timer will delete itself when it expires.  Otherwise (i.e., in
      //  the normal case) we must kill the timer using a RqOsTimerStop
      //  message.  Be aware that the latter message's constructor requires
      //  a copy of (or pointer to) the original message, in order to find
      //  its refnum.  sigh.

      }

   return;

}  /* end of CCmbEventSend::FragmentCallback */


