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
// File: CmbHwIntf.cpp
//
// Description:
//    Card Management Bus MIPS DDM / Atmel microcontroller interface code.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbHwIntf.cpp $
// 
// 22    2/15/00 11:30a Eric_wedel
// Added low-level AVR interface tracing support.  [For tracking
// DFCT12059]
// 
// 21    2/08/00 7:02p Eric_wedel
// Fixed a bug with sequencing of requests queued during callback
// processing.  Cleaned out some old cruft.  Added some extra debug code
// to help track mysterious SendContext exhaustion.
// 
// 20    1/20/00 4:19p Eric_wedel
// Fixed so callbacks are stored as instance/pfn pairs, since we can't
// rely on target pfn's always being in our host DDM (they may be in
// "smart cookies" like message send wrapper).
// Added some more debug support too.
// 
// 19    11/24/99 6:18p Ewedel
// Added watchdog timer on transmits.
// 
// 16    9/03/99 5:14p Ewedel
// Simplified send context's packet def.
// 
// 15    8/19/99 6:19p Jlane
// [ewx]  Made send buffer pool x32 also, to match receive (need at least
// one per slot which gives us 18, the rest are slop).
// 
// 14    8/19/99 3:13p Jlane
// Use new receive buffer mask.
// 
// 13    8/19/99 1:59p Ewedel
// Boosted number of unsolicited input buffers reserved, and added csect
// protection around access to ISR's shared rx buffers.
// 
// 12    7/30/99 5:14p Jlane
// [ewx]  Fixed doofy bug in choice of message pointer which is sent along
// to low-level CMB interface driver.
// (Also added debugging helper hook.)
// 
// 11    7/21/99 7:53p Ewedel
// Tweaked CCmbHwIntf::HandleCmbIrqSignal() so that it recognizes incoming
// commands as unsolicited packets, regardless of their destination value.
// 
// 10    7/19/99 5:03p Rkondapalli
// [ewx]  Fixed little memcpy() bug in
// CCmbHwIntf::QueueUnsolicitedPacket().
// 
// 9     7/15/99 4:23p Ewedel
// Added support for unsolicited input response packets (still needs
// tweaks for unsolicited request packets).
// 
// 8     6/28/99 5:47p Jlane
// [ewx] Fixed various bugs in case where commands are queued, or when
// callback does send(s) of its own.
// 
// 7     6/18/99 10:53a Rkondapalli
// Various fixes to make things work.  [ew/at]
// 
// 6     6/17/99 1:55p Ewedel
// Added hooks into real code in CmbIsr.cpp.  Unsolicited input (and
// non-callback Send()s) not supported yet.
// 
// 5     5/14/99 4:02p Ewedel
// Added routine IsRealCmbPresent() to allow client code to determine
// whether the CMB / interface is usable, or should be faked.
// 
// 4     5/14/99 12:06p Ewedel
// Changed to use CmbPacket instead of void for message buffers.
// Also changed comment inside HandleCmbIrqSignal() to better reflect its
// expected usage.
// 
// 3     5/11/99 11:22a Ewedel
// Added new member EnableInput().
// 
// 2     5/05/99 7:03p Ewedel
// Fixed weird Metrowerks function parameter warning.
// 
// 1     5/04/99 5:32p Ewedel
// Initial revision - just a hollow shell of its future self.
//
/*************************************************************************/


#include  "CmbHwIntf.h"

#include  "CmbIsr.h"       // access to the real CMB interface code

#include  "CtEvent.h"      // status codes

#include  "Critical.h"     // CHAOS critical section support

#include  <assert.h>

#include  "Odyssey_Trace.h"   // for Tracef()...

#include  "CmbDebugHack.h"    //BUGBUG - debug hack

#include  "RqOsTimer.h"       //BUGBUG - for our trial 1sec ticker


//BUGBUG - allow access to our single live instance under the debugger
CCmbHwIntf  * pCmbHwIntf;

//BUGBUG - our global debug info
CCmbDebugInfo CmbDebugInfo;

//  we now define our free packet lists as arrays, that they may be
//  more easily inspected under the debugger:

const int   cSendPktMax  =  32;     // how many sends can be outstanding
const int   cRecvPktMax  =  32;     // how many rx/unsolicited can be pending
                                    // (enough for notifies on all 18 slots
                                    //  plus a few spares)

//  send packet bufs
CCmbHwIntf::CSendContext      aSendCtxPool [cSendPktMax];

//  unsolicited packet receive bufs
CCmbHwIntf::CUnsolicitedPkt   aUnsolicitedPktPool [cRecvPktMax];

//  here is our low-level AVR interface tracking ring
CCmbDebugHwIntfLog   CmbDebugHwIntfLog;


//
//  CCmbHwIntf::CCmbHwIntf (pHostDdm, nSigCmbMsgRdy, nSigCmbIrq)
//
//  Description:
//    Our constructor.
//
//    This routine is responsible for establishing the identity of
//    our instance's host DDM, and the values of two signal codes
//    used to communicate with our host DDM and with our own CMB ISR.
//
//    Each DDM has its own signal code space, so the DDM which contains
//    our instance is responsible for allocating two signal codes from
//    its signal code space and reserving them for our use.  Note that
//    our host DDM dispatches one of the codes, while we dispatch the
//    other.  We signal both codes at the appropriate times.
//
//    ** Note that we will not send any signals to nSigCmbMsgRdy until
//       our member EnableInput() has been called with an arg of TRUE.
//       See EnableInput's function header for more info.
//
//  Inputs:
//    pHostDdm - Points to the DDM instance which contains our instance.
//                This value is used to keep our DdmOsServices base
//                class happy, and to provide a target for our own
//                signalling.
//    nSigCmbMsgRdy - A signal code reserved for us by our host DDM.
//                We use this code to signal our host DDM instance
//                when we have received a complete message from the
//                AVR.  The host DDM should call our member ReadMsg()
//                when it "receives" (dispatches) one of these signals.
//    nSigCmbIrq - A signal code reserved for us by our host DDM.
//                We use this code internally to communicate from our
//                ISR context to our "DDM-level" instance code.
//                ** We dispatch this signal internally -- our host
//                   DDM must not dispatch it.
//
//  Outputs:
//    none
//

CCmbHwIntf::CCmbHwIntf (DdmServices *pHostDdm,
                        SIGNALCODE nSigCmbMsgRdy, SIGNALCODE nSigCmbIrq)
               :  CCtTimedService (pHostDdm),
                  m_pHostDdm (pHostDdm),
                  m_nSigCmbMsgRead (nSigCmbMsgRdy),
                  m_nSigCmbIrq (nSigCmbIrq)
{

int         i;


   //BUGBUG - allow access to our single live instance under the debugger
	pCmbHwIntf = this;

   //  initialize our send-context tracking info
   m_pCurrentSendCtx     = NULL;
   m_PendingSendCtx      = NULL;
   m_PendingReplySendCtx = NULL;

   //  build up a finite list of available send context instances
   CSendContext *pSend;
   m_pFreeSendCtx = pSend = aSendCtxPool;
   for (i = 1;  i < cSendPktMax;  i ++)
      {
      //  thread first n-1 entries into linked list
      pSend->pNext = pSend + 1;

      pSend ++;
      }

   //  set last entry's Next link to naught
   pSend->pNext = NULL;

   //  similarly, we have no outstanding unsolicited packets yet
   m_pUnsolicitedPktList = NULL;

   //  and build up a finite list of available unsolicited pkt carriers
   CUnsolicitedPkt *pRcv;
   m_pUnsolicitedPktFreeList = pRcv = aUnsolicitedPktPool;
   for (i = 1;  i < cRecvPktMax;  i ++)
      {
      //  thread first n-1 entries into linked list
      pRcv->pNext = pRcv + 1;

      pRcv ++;
      }

   //  set last entry's Next link to naught
   pRcv->pNext = NULL;

   //  initialize our real CMA hardware's interface code
   CMB_Initialize ();

   //  claim our IRQ signal code's dispatches
   DispatchSignal(m_nSigCmbIrq,
                  (DdmServices::SignalCallback) &HandleCmbIrqSignal);

   //  set global variables, for use by our high-level ISR (HISR)

   //  the signal to announce receipt of either solicited or un- packets
   nSigCmbHwIrq = nSigCmbIrq;

   //  pointer to our host DDM, so we can signal it (signals are DDM-wide,
   //  so it's ok to use *pHostDdm instead of *this)
   pHwHostDdm = pHostDdm;

   //BUGBUG - establish an overly simplistic timer tick, so that we can
   //  try to work around possible watchdog problems.
   RqOsTimerStart  * pmsgStart;

   //  note that a "1 sec" tick is too short, since our timer is actually
   //  running at double speed.  So we do "2 sec", which is roughly 1 real sec.
   pmsgStart = new RqOsTimerStart (2000000, 2000000); // recurring ~1sec ticks

   //  save away start message's refnum, as this is our key to the
   //  new timer for its lifetime:

   //  (we create a holding place for the refnum since it is an I64, and
   //   Metrowerks doesn't guarantee us that our instance data will be
   //   suitably aligned)
   m_prefPeriodicTimer = new REFNUM (pmsgStart->refnum);

   //  send off message to create & start timer:
   Send (pmsgStart, REPLYCALLBACK(CCmbHwIntf, PeriodicTimerCallback));

   return;

}  /* end of CCmbHwIntf::CCmbHwIntf */

//
//  CCmbHwIntf::~CCmbHwIntf ()
//
//  Description:
//    Our destructor.
//
//    Undoes whatever our constructor did.
//
//  Inputs:
//    none
//
//  Outputs:
//    none
//

CCmbHwIntf::~CCmbHwIntf ()
{


   //..  call whatever "undo dispatch signal" routine Tom invents.
//   UnDispatchSignal(m_nSigCmbIrq);

//BUGBUG - should free all CSendContext instances -- what do we do
//         if one is presently outstanding?

   return;

}  /* end of CCmbHwIntf::~CCmbHwIntf */


//
//  CCmbHwIntf::IsRealCmbPresent ()
//
//  Description:
//    This is a silly little routine.  It exists to allow DDMs and
//    other clients of the hardware interface code to tell whether
//    a real or stub version of the interface is in use.
//
//    This routine should only return TRUE when real interface code
//    is present and CMB hardware appears to be present / usable.
//
//  Inputs:
//    none
//
//  Outputs:
//    CCmbHwIntf::IsRealCmbPresent - Returns TRUE if real CMB hardware
//                is present, and necessary class code is implemented
//                to permit its use.
//

BOOL  CCmbHwIntf::IsRealCmbPresent (void) const
{

   //BUGBUG - we need some intelligent way to decide whether the
   //  CMB hardware is present & functional.  Otherwise, we might
   //  say "true", and then not work when the CMB DDM asks us for things.

   //BUGBUG - for now, always claim working hardware.  Ha!
   return (TRUE);

}  /* end of CCmbHwIntf::IsRealCmbPresent */

//
//  CCmbHwIntf::EnableInput (fEnableInput)
//
//  Description:
//    This routine is called to enable or disable operation of our
//    CMB interface instance.
//
//    In particular, this routine is provided separately from our
//    constructor/destructor, so that an enclosing class may complete
//    its own initialization or teardown independent of when we are
//    enabled.  For example, as a member of a containing DDM, our
//    constructor would complete before the containing class' does.
//    If we enabled in our constructor, then we would be attempting
//    to send signals to our DDM's m_nSigCmbMsgRead before the DDM
//    had a chance to set a dispatch handler for this signal.
//
//  Inputs:
//    fEnableInput - Set to TRUE to enable input signals to our
//                host DDM, or to FALSE to disable them.
//
//  Outputs:
//    none
//

void  CCmbHwIntf::EnableInput (BOOL fEnableInput)
{

#pragma unused (fEnableInput)

}  /* end of CCmbHwIntf::EnableInput */

//
//  CCmbHwIntf::ReadMsg (MsgBuf, cbBufMax)
//
//  Description:
//    Read the next AVR-sent message from our input queue.
//
//    Call this routine when unsolicited messages are sent to the MIPS by
//    the AVR.  Specifically, our host DDM should call this routine when
//    it receives the "CmbMsgRead" signal as specified to our constructor.
//
//  Inputs:
//    MsgBuf - Buffer to place message into.
//    cbBufMax - One-based count of bytes that can fit into MsgBuf.
//
//  Outputs:
//    MsgBuf - Loaded with message, if one is ready.  Undefined otherwise.
//    cbMsg - Set to one-based count of bytes actually written to MsgBuf,
//                or required to hold it, if the buffer MsgBuf is too small.
//    CCmbHwIntf::ReadMsg - Returns CTS_SUCCESS if the read goes well,
//                or something else if there's no message to read or the
//                supplied buffer is too small.
//

STATUS  CCmbHwIntf::ReadMsg (CmbPacket& MsgBuf, int cbMsgBufMax)
{

U32               cbMsg;         // actual packet size
CUnsolicitedPkt * pCarrier;


   //  are there any packets waiting to return?
   if (m_pUnsolicitedPktList == NULL)
      {
      return (CTS_CMB_NO_UNSOLICITED_MSG);
      }

   //  got a message, figure out how big it is
   cbMsg = m_pUnsolicitedPktList->pkt.Size ();

   assert (cbMsgBufMax >= cbMsg);

   if (cbMsgBufMax < cbMsg)
      {
      //  whoops, packet too big for caller's buffer
      return (CTS_CMB_BUFFER_TOO_SMALL);
      }

   //  message fits in caller's buffer, so return it

   //  unlink message first
   pCarrier = m_pUnsolicitedPktList;
   m_pUnsolicitedPktList = m_pUnsolicitedPktList->pNext;

   //  return its data
   memcpy (&MsgBuf, &pCarrier->pkt, cbMsg);

   //  and return carrier instance to our free pool
   pCarrier->pNext = m_pUnsolicitedPktFreeList;
   m_pUnsolicitedPktFreeList = pCarrier;
//BUGBUG - debug info
   CmbDebugInfo.cCUnsolicitedPktsInUse --;
   assert (CmbDebugInfo.cCUnsolicitedPktsInUse >= 0);
//BUGBUG - end it

   return (CTS_SUCCESS);
   
}  /* end of CCmbHwIntf::ReadMsg */

//
//  CCmbHwIntf::SendMsg (Message, pCallbackInst, fnCallback,
//                       pvCookie, cusReplyTimeout)
//
//  Description:
//    Sends a message to the AVR, and optionally specifies a callback
//    to be signalled when a reply is received from the AVR.
//
//  Inputs:
//    Message - Message to send to AVR.
//    pCallbackInst - Instance pointer of DdmServices descendent in whose
//                context fnCallback is to be invoked.
//    fnCallback - Routine to call at completion of message send
//                (or when a fatal send error happens).
//                ** This must be a method of instance *pCallbackInst. **
//    pvCookie - Value to pass through as argument to fnCallback().
//    cusReplyTimeout - Timeout value before we return a NAK due to
//                no response from CMB.  lsb is one microsecond (1 us).
//                Our default of zero means "no timeout", though the AVR
//                generally implements its own timeout anyway, so we still
//                might receive a nak(timeout) response from the AVR.
//
//  Outputs:
//    CCmbHwIntf::SendMsg - Returns CTS_SUCCESS if the send initiates
//                properly, else something else.  We typically don't
//                report the final result of the send, since that involves
//                waiting until the AVR replies to us (a lengthy proposition).
//                If you wish to know the results, then specify a callback
//                parameter.
//

STATUS  CCmbHwIntf::SendMsg (const CmbPacket& Message,
                             DdmServices *pCallbackInst,
                             MsgCallback fnCallback,
                             void *pvCookie /* = NULL */ ,
                             U32 cusReplyTimeout /* = 0 */ )
{


   //  use our common sender, specifying the standard "pending send"
   //  queue in case of transmitter backup:
   return (InternalSendMsg (Message, pCallbackInst, fnCallback,
                            pvCookie, cusReplyTimeout,
                            m_PendingSendCtx));

}  /* end of CCmbHwIntf::SendMsg */

//
//  CCmbHwIntf::SendReplyToUnsolicitedCmd (pktReply)
//
//  Description:
//    Sends a reply to an unsolicited command packet back to the AVR.
//    We don't provide any callback for signalling completion.
//
//    We also use a special queue, in the event that our transmitter
//    is busy, to give precedence to these reply sends over ordinary
//    packets.
//
//  Inputs:
//    pktReply - CMB response packet to send to AVR.
//
//  Outputs:
//    CCmbHwIntf::SendReplyToUnsolicitedCmd - Returns CTS_SUCCESS if the send
//                initiates properly, else something else.  We typically don't
//                report the final result of the send, since that involves
//                waiting until the AVR replies to us (a lengthy proposition).
//                If you wish to know the results, then specify a callback
//                parameter.
//

STATUS  CCmbHwIntf::SendReplyToUnsolicitedCmd (const CmbPacket& pktReply)
{


   //  use our common sender, specifying the "pending reply" queue
   //  in case of transmitter backup:
   return (InternalSendMsg (pktReply,
                            NULL, NULL,      // no callback!
                            NULL, 0, m_PendingReplySendCtx));

}  /* end of CCmbHwIntf::SendReplyToUnsolicitedCmd */

//
//  CCmbHwIntf::HandleCmbIrqSignal (nSignal, pPayload)
//
//  Description:
//    Handles a CMB IRQ, as signalled by our CMB ISR.
//
//    This routine exists to do any DDM-level processing necessary
//    to prepare for signalling our host DDM of receipt of a complete
//    message from the CMB.
//
//    It is possible that this routine won't be needed at all, if our
//    ISR can do all the necessary setup work to prepare for our host
//    DDM to call ReadMsg().  Then our ISR could directly signal to
//    m_nSigCmbMsgRead instead, and this routine can go away.
//
//  Inputs:
//    nSignal - Signal code which is being dispatched to us.
//                Should always be m_nSigCmbIrq.
//    pPayload - Points to stuff fed to us by our ISR. As we use this,
//                it is actually a BOOL masquerading as a (void *).
//                The meaning of the boolean is "message ready."
//                When TRUE, we should read a message from the
//                driver's data area.  When FALSE, we should *not*;
//                the driver is merely notifying us that the transmitter
//                is ready for the next packet to be sent.
//
//  Outputs:
//    CCmbHwIntf::HandleCmbIrq - Returns CTS_SUCCESS if all goes well,
//                or else an error code.
//                Are we actually permitted to return an error here?
//

STATUS  CCmbHwIntf::HandleCmbIrqSignal (SIGNALCODE nSignal, void *pPayload)
{

CMB_CONTROL_BLOCK  * pCMB_CB;
CmbPacket            pktNew;     // intermediate buffer
U32                  iPacket;
const BOOL           bMessageReady  =  (BOOL) pPayload;
STATUS               sRet;


#pragma unused (pPayload)

   assert (nSignal == m_nSigCmbIrq);

   if (bMessageReady)
      {
      //  got a message, fish it out from our C code..

      //  first, be sure that interrupts are disabled so that we have
      //  clear access to data struct shared with our (low-level) ISR.
      Critical  csect;        // (construction asserts the csect)

      pCMB_CB = &CMB_ControlBlock;

      iPacket = pCMB_CB->CMB_RprocessX;      // grab packet index
      iPacket *= 32;                         // scale to byte index

      //  make a copy of packet, so we can release our lock on the
      //  ISR's receive buffer
      pktNew = *(CmbPacket *) (pCMB_CB->CMB_Rbuffer + iPacket);

      //  advance "read" queue pointer to next slot
      pCMB_CB->CMB_RprocessX++;
      pCMB_CB->CMB_RprocessX &= CMB_RECEIVE_BUFFER_INDEX_MASK;

      //  and now it's safe to release our lock on the shared data area
      csect.Leave ();

      //  process the received packet as appropriate
   
      if ((pktNew.Hdr.bDestAddr == CMB_NOTIFY) ||
          ((pktNew.Hdr.bStatus & CmbStatCmd) != 0))
         {
         //  a real unsolicited notification received, or a request from
         //  some other MIPS CPU.  Queue it & signal our host DDM:
         QueueUnsolicitedPacket (&pktNew);
         }
      else
         {
         //  got a response to a command, notify caller
         NotifySenderAndFreeContext (&pktNew, CTS_SUCCESS);
         }
      }
   else
      {
      //  we're being signalled that we completed sending a reply packet.
      //  Right now, we don't support callbacks on this sort of send,
      //  but we certainly do need to pull the reply carrier out of
      //  our "current" context, to mark our transmitter as "free".
      assert ((m_pCurrentSendCtx != NULL) &&
              (m_pCurrentSendCtx->fHasPacket) &&
              ((m_pCurrentSendCtx->Pkt.Hdr.bStatus & CmbStatCmd) == 0));

      if (m_pCurrentSendCtx != NULL)
         {
         //  free reply send context
         FreeContext (m_pCurrentSendCtx);
         m_pCurrentSendCtx = NULL;
         }
      }

   //  we might have received a reply which freed the transmitter,
   //  or we might have finished sending a reply.  Either way,
   //  we're ready to send our next packet.

   sRet = SendNextQueuedPacket ();

   //  always return success, signal callback caller doesn't know what
   //  to do with error codes (their meaning here is "reserved").
   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::HandleCmbIrqSignal */

//
//  CCmbHwIntf::StartSend (pPacketQueue)
//
//  Description:
//    Grabs a packet carrier from the specified queue, links it in
//    as our current packet, and starts it running across our low-
//    level CMB interface.
//
//  Inputs:
//    pPacketQueue - Points to head ptr of queue from which we extract
//                packet.
//
//  Outputs:
//    pPacketQueue - Updated to point to next packet on queue, if any.
//    m_pCurrentSendCtx - Set to point to packet which was previously
//                   at the head of pPacketQueue's queue.
//    CCmbHwIntf::StartSend - Returns CTS_SUCCESS, or some sort of code
//                   indicating why the transmit failed.
//

STATUS  CCmbHwIntf::StartSend (CSendContext *& pPacketQueue)
{

STATUS   sRet;


   assert (pPacketQueue != NULL);
   assert (m_pCurrentSendCtx == NULL);

   //  (for sync, must flag command active before sending it)
   m_pCurrentSendCtx = pPacketQueue;
   pPacketQueue = pPacketQueue -> pNext;

   sRet = CMB_SendCommand (&m_pCurrentSendCtx->Pkt);

   //  hardware should have been ready, but check anyway
   assert (sRet == CTS_SUCCESS);
   if (sRet == CTS_SUCCESS)
      {
      //  packet is winging its way to wherever.  Just to be on the safe side,
      //  we start a watchdog timer on it.
      if (WatchdogIsRunning ())
         {
         //  watchdog already running, just restart it.
         //  Note that we use current send context as our cookie.
         ResetWatchdogTimer (WDTIMEROPCALLBACK (CCmbHwIntf, WatchdogOpCallback),
                             m_pCurrentSendCtx);
         }
      else
         {
         //  watchdog not running, so start it.
         StartWatchdogTimer (2 * 1000000,    // 2e6 us = 2 second timeout
                             WDTIMEOUTCALLBACK (CCmbHwIntf, WatchdogTimeoutCallback),
                             WDTIMEROPCALLBACK (CCmbHwIntf, WatchdogOpCallback),
                             m_pCurrentSendCtx);
         }

      //  cool, context already moved so we're done.
      }
   else
      {
      //  hmm, couldn't send pending packet.
      //  We're hosed, nothing else will ever trigger a re-send,
      //  unless we get some sort of unsolicited input message
      //  from the AVR.
      //  In short, our own IOP is now toast.

      //  let's be sure to undo the context move which we did above
      m_pCurrentSendCtx->pNext = pPacketQueue;
      pPacketQueue = m_pCurrentSendCtx;
      m_pCurrentSendCtx = NULL;

      assert (FALSE);
      }

   return (sRet);

}  /* end of CCmbHwIntf::StartSend */

//
//  CCmbHwIntf::WatchdogTimeoutCallback (pvTimeoutCookie)
//
//  Description:
//    Called by our watchdog timer base class, when a timer started
//    by StartSend() has expired.  This should only happen when our
//    AVR interface has somehow lost the packet we were sending.
//
//    We use this timeout as an occasion to fail the send operation.
//    Higher-level code (e.g., CCmbMsgSender) may retry the operation
//    if desired.  Note that the retry may result in a reordering of
//    CMB packets, but this same effect would occur of the HBC AVR
//    itself timed out a response (with a NAK[timeout] dummy reply).
//
//  Inputs:
//    pvTimeoutCookie - Points to cookie fed in by StartSend().
//                Right now, we make this be m_pCurrentSendCtx, so
//                we can verify whether a timeout applies to what
//                we're processing now.
//
//  Outputs:
//    none
//

void  CCmbHwIntf::WatchdogTimeoutCallback (void *pvTimeoutCookie)
{


   Tracef ("CCmbHwIntf::WatchdogTimeoutCallback: got a local AVR timeout!\n");

   //  does this timeout notification apply to the packet which
   //  we think we're still sending?
   if (pvTimeoutCookie != m_pCurrentSendCtx)
      {
      //  nope, this timeout notification is ancient history.  Ignore it.
      CmbDebugInfo.cCmbHwIntfUnmatchedTimeoutCallbacks ++;
      Tracef ("CCmbHwIntf::WatchdogTimeoutCallback: ignoring unmatched timeout\n");
      return;
      }

   //  timeout is applicable, so do it to it.

   //BUGBUG - give us a little history
   CmbDebugHwIntfLog.DumpRing ();
   //BUGBUG - endit

   CmbDebugInfo.cCmbHwIntfMatchedTimeoutCallbacks ++;
   Tracef ("    pktReq cmd=0x%02X, dst=0x%02X, stat=0x%02x, cbDat=%d, Dat[0]=0x%02x\n",
           m_pCurrentSendCtx->Pkt.Hdr.bCommand,
           m_pCurrentSendCtx->Pkt.Hdr.bDestAddr,
           m_pCurrentSendCtx->Pkt.Hdr.bStatus,
           m_pCurrentSendCtx->Pkt.Hdr.cbData,
           m_pCurrentSendCtx->Pkt.abTail[0]);

   //  first, call caller's callback.
   NotifySenderAndFreeContext (NULL, CTS_CMB_LOCAL_AVR_TIMEOUT);

   //  then, start up next packet, if appropriate.
   SendNextQueuedPacket ();

   return;

}  /* end of CCmbHwIntf::WatchdogTimeoutCallback */

//
//  CCmbHwIntf::NotifySenderAndFreeContext (pReplyPkt, sResult)
//
//  Description:
//    A utility routine, which provides a common point for handling
//    "Send-complete" notifications to the sender.
//
//    Note that we might be saying "send complete" but returning
//    some sort of an error.  In the extreme case, we might even
//    be reporting that we failed to communicate with our local AVR,
//    in which case we have no reply packet to supply.
//
//  Inputs:
//    pReplyPkt - Packet to feed to sender's callback as reply to their send.
//                If sResult != CTS_SUCCESS, then pReplyPkt may be NULL.
//    sResult - Result of send operation.  CTS_SUCCESS == ok, anything else
//                is bad.
//    m_pCurrentSendCtx - [Instance var]  Context whose send operation
//                we are completing.
//
//  Outputs:
//    m_pCurrentSendCtx - Set to NULL, since we never initiate the send
//                of another packet.  Use SendNextQueuedPacket() for that,
//                after calling us.
//

void  CCmbHwIntf::NotifySenderAndFreeContext (const CmbPacket *pReplyPkt,
                                              STATUS sResult)
{


   assert ((pReplyPkt != NULL) || (sResult != CTS_SUCCESS));

   //  (we should always have a current context in this case)
   assert (m_pCurrentSendCtx != NULL);

   if (m_pCurrentSendCtx != NULL)
      {
      //  got a caller to notify, let's do it

      //  (response should match command send)
      if (pReplyPkt != NULL)
         {
         assert (pReplyPkt->Hdr.bCommand == m_pCurrentSendCtx->Pkt.Hdr.bCommand);
         }

      //  Note that we process callback *before* pulling our current send
      //  context off of "current".  This way, if our callback initiates
      //  any new sends, they will be queued after any other sends which
      //  are already pending.  This FIFO behavior hopefully will avoid
      //  starvation of simple CMB requests by CMB-intensive requests like
      //  environmental polling.

      //  figure out how sender wants to be notified
      if (m_pCurrentSendCtx->fnCallback != NULL)
         {
         //  caller wants a callback, watch close :-)
         ((m_pCurrentSendCtx->pCallbackInst)->*(m_pCurrentSendCtx->fnCallback))
               (m_pCurrentSendCtx->pvCookie, sResult, pReplyPkt);
         }
      else
         {
         //  caller didn't want a callback, so we add this to our
         //  "unsolicited" input queue (also does Signal() call).
         if (pReplyPkt != NULL)
            {
            //  got a reply packet, queue it
            QueueUnsolicitedPacket (pReplyPkt);
            }
         else
            {
            //  no packet, which right now means a local-side timeout.
            //  So we synthesize a NAK packet.
            CmbPacket   pktTimeout;

            pktTimeout.MakeReply (m_pCurrentSendCtx->Pkt, FALSE);
            pktTimeout.abTail[0] = k_eCmbNRCmaTimeout;

            //NOTE:  k_eCmbNRCmaTimeout is not quite the right value,
            //       but it conveys the general spirit.  And this code
            //       path is seldom used (if ever).

            //  queue synthetic packet for requestor's later perusal:
            QueueUnsolicitedPacket (&pktTimeout);
            }
         }

      //  either way, we're done with the caller's context info,
      //  so move it out of "current" and free it.
      Critical section;    // shouldn't be needed, but just in case..

      FreeContext (m_pCurrentSendCtx);
      m_pCurrentSendCtx = NULL;
      }

   return;

}  /*  end of CCmbHwIntf::NotifySenderAndFreeContext */

//
//  CCmbHwIntf::QueueUnsolicitedPacket (pCmbPacket)
//
//  Description:
//    This is a helper routine.  We allocate a carrier object, queue the
//    given CMB packet, and then post a signal to our host DDM to announce
//    the newly-queued packet.
//
//  Inputs:
//    pCmbPacket - Points to packet data to queue.  We make a copy of
//                this data, so the original need not live longer than
//                the duration of this call.
//
//  Outputs:
//    none
//

void  CCmbHwIntf::QueueUnsolicitedPacket (const CmbPacket *pCmbPacket)
{

CUnsolicitedPkt    * pUnsolPkt;
CUnsolicitedPkt    * pListTail;


   //  allocate a carrier for message, and queue it
   pUnsolPkt = m_pUnsolicitedPktFreeList;
   assert (pUnsolPkt != NULL);

//BUGBUG - debug info
   CmbDebugInfo.cCUnsolicitedPktsInUse ++;
//BUGBUG - end it

   if (pUnsolPkt == NULL)
      {
      //  whoops, we got some bad failure here..  drop the packet.
      //  (our caller can't handle errors, so we don't return any)
      return;
      }

   //  got a carrier, unlink it and save packet
   m_pUnsolicitedPktFreeList = pUnsolPkt->pNext;
   memcpy (&pUnsolPkt->pkt, pCmbPacket, pCmbPacket->Size());
   pUnsolPkt->pNext = NULL;

   //  link carrier at end of pending-unsolicited list
   if (m_pUnsolicitedPktList == NULL)
      {
      m_pUnsolicitedPktList = pUnsolPkt;
      }
   else
      {
      //  find end of queue
      for (pListTail = m_pUnsolicitedPktList;
           pListTail->pNext != NULL;
           pListTail = pListTail->pNext)
         {}

      //  and stick our new carrier on it
      pListTail->pNext = pUnsolPkt;
      }

   //  let our host DDM know that something came in
   m_pHostDdm->Signal(m_nSigCmbMsgRead);

   return;

}  /* end of CCmbHwIntf::QueueUnsolicitedPacket */

//
//  CCmbHwIntf::SendNextQueuedPacket ()
//
//  Description:
//    Checks to see that our transmitter is free, and if so finds and
//    sends our first queued packet.  Calling us when our transmitter
//    is busy is not any error, but results in a no-op here.
//
//    Note that when choosing among our queues, we give absolute
//    priority to packets on the "pending reply" queue, since the AVR
//    will normally hang while waiting for such replies.
//
//    Also, despite our juxtaposition, we're not related to
//    QueueUnsolicitedPacket(), which is concerned with received packet
//    queuing, not transmit packets.
//
//  Inputs:
//    none
//
//  Outputs:
//    CCmbHwIntf::SendNextQueuedPacket - Returns CTS_SUCCESS unless our
//                hardware is out of order.  [We even return CTS_SUCCESS
//                if a send is already in progress.]
//

STATUS  CCmbHwIntf::SendNextQueuedPacket (void)
{

STATUS   sMyRet;


   //  if there's a pending send get it started, unless somebody beat us to it:
   if (m_pCurrentSendCtx == NULL)
      {
      //  look for pending replies to unsolicited commands, first!
      //  These must always be given precedence over ordinary
      //  "send" packets, since the CMA will block while waiting
      //  for the former, and drop the latter on the floor (with NAK).
      if (m_PendingReplySendCtx != NULL)
         {
         //  got an unsolicited command reply packet to send
         sMyRet = StartSend (m_PendingReplySendCtx);
         }
      else if (m_PendingSendCtx != NULL)
         {
         //  got a regular packet to send, get to it
         sMyRet = StartSend (m_PendingSendCtx);
         }
      else
         {
         //  nothing to do, we're perfectly happy with this.
         sMyRet = CTS_SUCCESS;

         //  Let's turn off our watchdog timer.
         //  We might be invoked via HandleCmbIrqSignal(), in response
         //  to an unsolicited packet receive.  In this case, it is
         //  perfectly ok if the watchdog is not running, so let's
         //  protect ourselves from illegal watchdog ops:
         if (WatchdogIsRunning ())
            {
            StopWatchdogTimer (WDTIMEROPCALLBACK (CCmbHwIntf,
                                                  WatchdogOpCallback));
            }
         }
      }

   return (sMyRet);

}  /* end of CCmbHwIntf::SendNextQueuedPacket */

//
//  CCmbHwIntf::InternalSendMsg (Message, pCallbackInst, fnCallback,
//                               pvCookie, cusReplyTimeout, pSendDeferQueue)
//
//  Description:
//    Sends a message to the AVR, and optionally specifies a callback
//    to be signalled when a reply is received from the AVR.
//
//  Inputs:
//    Message - Message to send to AVR.
//    pCallbackInst - Instance pointer of DdmServices descendent in whose
//                context fnCallback is to be invoked.
//    fnCallback - Routine to call at completion of message send
//                (or when a fatal send error happens).
//                ** This must be a method of instance *pCallbackInst. **
//                This pointer may be NULL.  In this case, when we have a
//                response to return we will send a "packet ready"
//                notification to the client as if it were an unsolicited
//                packet.  The client then reads the packet using
//                ReadMsg(), as for unsolicited packets.
//    pvCookie - Value to pass through as argument to fnCallback().
//    cusReplyTimeout - Timeout value before we return a NAK due to
//                no response from CMB.  lsb is one microsecond (1 us).
//                Our default of zero means "no timeout", though the AVR
//                generally implements its own timeout anyway, so we still
//                might receive a nak(timeout) response from the AVR.
//    pSendDeferQueue - Head of queue in which we place this request if
//                our transmitter is already busy.
//
//  Outputs:
//    CCmbHwIntf::SendMsg - Returns CTS_SUCCESS if the send initiates
//                properly, else something else.  We typically don't
//                report the final result of the send, since that involves
//                waiting until the AVR replies to us (a lengthy proposition).
//                If you wish to know the results, then specify a callback
//                parameter.
//

STATUS  CCmbHwIntf::InternalSendMsg (const CmbPacket& Message,
                                     DdmServices *pCallbackInst,
                                     MsgCallback fnCallback, void *pvCookie,
                                     U32 cusReplyTimeout,
                                     CSendContext *& pSendDeferQueue)
{

CSendContext * pCtx;
STATUS         ulMyRet;

#pragma unused(cusReplyTimeout)		// for now, we rely on AVR

   //  first off, we need to build a context holder for this message
   if (! GetContext (pCtx))
      {
      //  whoops, out of resources
      return (CTS_OUT_OF_MEMORY);
      }

   //  load current message into context holder
   pCtx->Initialize (Message, pCallbackInst, fnCallback, pvCookie);

//BUGBUG - pass debug info into context too.  It just so happens that
//         all the calls to SendMsg() set pvCookie to our send wrapper.
   pCtx->pSendWrapper = (CCmbMsgSender *) pvCookie;
//BUGBUG - endit

   //  ok, now we're ready to attempt the send

   //  practice safe critical sectioning
   Critical section;

   //  attempt to send message
   if (m_pCurrentSendCtx == NULL)
      {
      //  no command in process, so try the send
      
      //  (note:  this normally copies pCtx to m_pCurrentSendCtx, and
      //   then makes pCtx == NULL.  It thinks pCtx is a queue head.)
      ulMyRet = StartSend (pCtx);
      
      if (ulMyRet != CTS_SUCCESS)
         {
         //  whoops, submit failed so clear out command.
         //BUGBUG - do something with ex-command carrier *pCtx
         assert (FALSE);
         }
      }
   else
      {
      //  CMB hardware interface is busy.  Since the CMB can only do
      //  one thing at a time, we queue this message.  It will be
      //  de-queued in our response signal handler, since that is where
      //  the CMB signals that it is done with whatever it was doing.

      if (pSendDeferQueue == NULL)
         {
         //  this is first item on list..
         pSendDeferQueue = pCtx;
         }
      else
         {
         //  stick this item on end of list
         CSendContext * p;
         for (p = pSendDeferQueue;  p->pNext != NULL;  p = p->pNext)
            {};

         assert ((p != NULL) && (p->pNext == NULL));

         p->pNext = pCtx;
         }

      assert (pCtx->pNext == NULL);    // (defined by Initialize() above)

      //  flag that we're really happy
      ulMyRet = CTS_SUCCESS;
      }

   return (ulMyRet);

}  /* end of CCmbHwIntf::InternalSendMsg */

//
//  CCmbHwIntf::PeriodicTimerCallback (pReply)
//
//  Description:
//    Called whenever we receive a tick from the timer DDM.
//    These tick replies are in response to an RqOsTimerStart message
//    which we sent in our constructor.
//
//    We use these ticks to see if it looks like our current send
//    context has gotten stale.  If so, we force a timeout of it.
//
//    Yes, this is what our base Watchdog class is supposed to do.
//    However, there are indications that it isn't working properly,
//    so we're here to provide a fallback.
//
//  Inputs:
//    pReply - Reply from timer DDM reporting another tick gone by.
//
//  Outputs:
//    CCmbHwIntf::PeriodicTimerCallback - Returns CTS_SUCCESS, always,
//                like a good little reply handler should.
//

STATUS  CCmbHwIntf::PeriodicTimerCallback (Message *pReply)
{


   //  keep track of time of last received tick
   CmbDebugInfo.ulTimeLastTickReceived = Time();

   //  keep count of total ticks received (== elapsed time in secs)
   CmbDebugInfo.cTotalTicksReceived ++;

   //  do we even have a "current" to worry about?
   if (m_pCurrentSendCtx != NULL)
      {
      //  yup, bump its ticks-gone-by counter
      m_pCurrentSendCtx->cTicksGoneBy ++;

      if (m_pCurrentSendCtx->cTicksGoneBy > 1)
         {
         //  whoops, at least one full tick has elapsed (maybe two).
         //  Since our ticks are one second, this is enough to time out.

         //BUGBUG - give us a little history
         CmbDebugHwIntfLog.DumpRing ();
         //BUGBUG - endit

         CmbDebugInfo.cCmbHwIntfTickInducedTimeouts ++;

         Tracef ("CCmbHwIntf::PeriodicTimerCallback: local AVR timeout!\n");
         Tracef ("    pktReq cmd=0x%02X, dst=0x%02X, stat=0x%02x, cbDat=%d, Dat[0]=0x%02x\n",
                 m_pCurrentSendCtx->Pkt.Hdr.bCommand,
                 m_pCurrentSendCtx->Pkt.Hdr.bDestAddr,
                 m_pCurrentSendCtx->Pkt.Hdr.bStatus,
                 m_pCurrentSendCtx->Pkt.Hdr.cbData,
                 m_pCurrentSendCtx->Pkt.abTail[0]);

         //  first, call caller's callback.
         NotifySenderAndFreeContext (NULL, CTS_CMB_LOCAL_AVR_TIMEOUT);

         //  then, start up next packet, if appropriate.
         SendNextQueuedPacket ();
         }
      }
   else
      {
      //  no current, nothing to do.  But let's make sure that we don't
      //  have any orphan pending contexts:
      assert (m_PendingSendCtx == NULL);
      }

   //  dispose of timer's reply message
   delete pReply;

   //  return our stock value
   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::PeriodicTimerCallback */


