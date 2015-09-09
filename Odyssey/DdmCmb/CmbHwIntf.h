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
// File: CmbHwIntf.h
// 
// Description:
//    Card Management Bus MIPS DDM / Atmel microcontroller interface class.
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbHwIntf.h $
// 
// 14    2/08/00 7:04p Eric_wedel
// Removed a little cruft, added a good deal of debug code for tracking
// mysterious SendContext exhaustion.
// 
// 13    1/20/00 4:20p Eric_wedel
// Changed callback ptr to inst/ptr pair, added some more debug info, and
// did a little cleanup.
// 
// 12    11/24/99 6:18p Ewedel
// Oops, needed new member SendNextQueuedPacket(), too.
// 
// 11    11/24/99 3:39p Ewedel
// Changed to derive from CCtTimedService, which gives us watchdog timer
// support.
// 
// 9     9/03/99 5:15p Ewedel
// Enhanced send context with debug support, more modern packet format.
// Added debug hook to hw intf class.
// 
// 8     7/19/99 3:20p Rkondapalli
// Corrected CSendContext::Initialize()'s memcpy().
// 
// 7     7/15/99 4:22p Ewedel
// Added various items (including new class CUnsolicitedPkt) for
// unsolicited packet input support.
// 
// 6     6/17/99 1:53p Ewedel
// Added CSendContext helper class for tracking CMB send requests.
// 
// 5     6/12/99 1:29a Ewedel
// Various bits of cleanup to make callback stuff work.
// 
// 4     5/14/99 4:02p Ewedel
// Added routine IsRealCmbPresent() to allow client code to determine
// whether the CMB / interface is usable, or should be faked.
// 
// 3     5/14/99 12:05p Ewedel
// Changed to use CmbPacket instead of void for message buffers.
// 
// 2     5/11/99 11:19a Ewedel
// Added new member EnableInput().
// 
// 1     5/04/99 5:32p Ewedel
// Initial revision - just a hollow shell of its future self.
// 
/*************************************************************************/

#ifndef _CmbHwIntf_h_
#define _CmbHwIntf_h_


#include "Ddm.h"

#ifndef _CmbHwIntfMsgs_h_
# include  "CmbHwIntfMsgs.h"
#endif

#ifndef CTS_SUCCESS
# include  "CtEvent.h"
#endif

#ifndef _CtTimeGuard_h_
# include  "CtTimeGuard.h"    // watchdog "smart cookie" base class
#endif

#ifndef _CmbDebugHack_h_
# include  "CmbDebugHack.h"
#endif


class CCmbHwIntf : public CCtTimedService
{
public:

   //  our only constructor - specifies our dedicated signal codes
   //  (see function header comment for important details!)
   CCmbHwIntf (DdmServices *pHostDdm,
               SIGNALCODE nSigCmbMsgRdy, SIGNALCODE nSigCmbIrq);

   //  we've got lots of hairy internals to clean up -- our destructor does it
   ~CCmbHwIntf ();

   //  while we're still in development mode, it's useful to know
   //  whether a given build has a real CMB interface, or is bogus
   BOOL  IsRealCmbPresent (void) const;

   //  call this routine to actually enable signals to our host DDM
   void  EnableInput (BOOL fEnableInput);

   //  call this routine to read unsolicited AVR messages, or replies to
   //  sends which didn't include a callback.  Put another way, call this
   //  routine in response to nSigCmbMsgRdy dispatches in your DDM.
   STATUS  ReadMsg (CmbPacket& MsgBuf, int cbMsgBufMax);

   //  here's the callback routine prototype for receiving replies
   //  to messages we sent earlier
   typedef  void  (DdmServices::*MsgCallback) (void *pvCookie, STATUS status,
                                               const CmbPacket *pReply);

   //  call this routine to send a response to an unsolicited command
   //  back to the AVR
   STATUS  SendReplyToUnsolicitedCmd (const CmbPacket& pktReply);

   //  call this routine to send all other messages to the AVR
   STATUS  SendMsg (const CmbPacket& Message,
                    DdmServices *pCallbackInst, MsgCallback fnCallback,
                    void *pvCookie = NULL,
                    U32 cusReplyTimeout = 0);

   //  a couple helpers for our "dummy" version

   inline DdmServices * HostDdmPtr (void)  const
                     {  return (m_pHostDdm);  };

   inline SIGNALCODE CmbMsgReadySigCode (void) const
                     {  return (m_nSigCmbMsgRead);  };


   //  here's a little helper class for tracking Send() requests.
   //  This is also used for our debug-only Action callback.

   class CSendContext
   {
   public:

      CSendContext (void)
            {  pNext = NULL;  };

      CSendContext (void *pvCookie,
                    DdmServices *pCallbackInst, MsgCallback fnCallback,
                    STATUS sResult, CmbPacket *ppktResult)
      {
         this->pvCookie      = pvCookie;
         this->pCallbackInst = pCallbackInst;
         this->fnCallback    = fnCallback;
         this->sResult       = sResult;
         this->pNext         = NULL;
         if (ppktResult != NULL)
            {
            SetPkt (*ppktResult);
            }
         else
            {
            this->fHasPacket = FALSE;
            }
      }


      void  Initialize (const CmbPacket& Message,
                        DdmServices *pCallbackInst,
                        MsgCallback fnCallback, void *pvCookie,
                        STATUS sResult = CTS_SUCCESS)
      {
         this->Pkt           = Message;
         this->pCallbackInst = pCallbackInst;
         this->fnCallback    = fnCallback;
         this->pvCookie      = pvCookie;
         this->pNext         = NULL;
         this->sResult       = sResult;
         this->fHasPacket    = TRUE;
      };

      void  SetPkt (const CmbPacket& pkt)
      {
         this->Pkt        = pkt;
         this->fHasPacket = TRUE;
      }

      //  our member data, such as it is

      //  context in which to call fnCallback member func
      DdmServices  * pCallbackInst;

      //  what to call when reply to this message is received
      MsgCallback    fnCallback;

      //  a caller-defined value which we pass to m_fnCallback
      void         * pvCookie;

      //  the actual packet
      CmbPacket      Pkt;

      //  do we have a real packet value (CmbHwIntfDummy only)
      BOOL           fHasPacket;

      //  pointer to next context in whatever list we're in
      //  (not presently used in debug code)
      CSendContext * pNext;

      //  status of operation (only used by CmbHwIntfDummy code)
      STATUS         sResult;

//BUGBUG - debug info to track strange buffer exhaustion bug
      CCmbMsgSender *pSendWrapper;
      I64            ulGetTime;     //Time() at last GetContext() of us
      I64            ulGetDelta;    // delta from prior GetContext() to us
      U32            ulGetSeqNum;   // order of our getting by GetContext()
      U32            cTicksGoneBy;  // count of 1sec ticks while we're current
//BUGBUG - endit

   };  /* end of CSendContext */


private:    /* resuming class CCmbHwIntf */

   //  pointer to our host DDM, so we can signal it
   DdmServices     * const m_pHostDdm;

   //  signal code sent to our host DDM when we've received a complete
   //  message from the AVR to process.
   const SIGNALCODE  m_nSigCmbMsgRead;

   //  signal code from our host DDM's code space.  This signal is reserved
   //  for our internal use in propagating AVR-to-MIPS IRQs.
   const SIGNALCODE  m_nSigCmbIrq;

   //  here's the carrier object which we use for queueing unsolicited packets
   class CUnsolicitedPkt
   {
   public:
      CUnsolicitedPkt * pNext;
      CmbPacket         pkt;
   };  /* end of CUnsolicitedPkt */

   //  head of our list of outstanding unsolicited packets
   CUnsolicitedPkt * m_pUnsolicitedPktList;

   //  head of our free list of unsolicited packet carriers
   CUnsolicitedPkt * m_pUnsolicitedPktFreeList;


   //  helper for handling our ISR's callback into DDM-land
   //  -- might not be needed, if our ISR can directly set up
   //     ReadMsg() stuff and then do signal to host DDM
   //  (must conform to DdmServices::SignalCallback prototype)
   STATUS  HandleCmbIrqSignal (SIGNALCODE nSignal, void *pPayload);

   //  helper for queuing an unsolicited or otherwise unhandled packet
   //  (also posts Signal() annunciation to our host DDM)
   void  QueueUnsolicitedPacket (const CmbPacket *pCmbPacket);

   //  helper for allocating one of our free contexts
   BOOL  GetContext (CSendContext *& pNewCtx)
      {
//BUGBUG - debug info hack
      static I64  ulLastTime;
      static U32  ulGetSeqNum;
//BUGBUG - endit
      
      assert (m_pFreeSendCtx != NULL);
      if (m_pFreeSendCtx != NULL)
         {
//BUGBUG - debug info hack
         CmbDebugInfo.cCSendContextsInUse ++;
//BUGBUG - endit
         pNewCtx = m_pFreeSendCtx;
         m_pFreeSendCtx = pNewCtx->pNext;
//BUGBUG - debug info hack
         pNewCtx->ulGetTime = Time ();	// set abs time of this alloc
         pNewCtx->ulGetDelta = pNewCtx->ulGetTime - ulLastTime;
         ulLastTime = pNewCtx->ulGetTime;

         pNewCtx->ulGetSeqNum = ulGetSeqNum ++;

         pNewCtx->cTicksGoneBy = 0;
//BUGBUG - endit
         return (TRUE);
         }
      return (FALSE);
      };

   //  helper for freeing context allocated by GetContext()
   //  (context inst is assumed to already be unlinked)
   void  FreeContext (CSendContext *pCtx)
      {
//BUGBUG - debug info hack
         CmbDebugInfo.cCSendContextsInUse --;
         assert (CmbDebugInfo.cCSendContextsInUse >= 0);
//BUGBUG - endit
         pCtx->pNext = m_pFreeSendCtx;
         m_pFreeSendCtx = pCtx;
      };

   //  helper for sending next packet queued for transmit
   //  (not related to QueueUnsolicitedPacket(), which deals
   //   with received packets)
   STATUS  SendNextQueuedPacket (void);

   //  helper for doing standard send, with choice of defer queue
   STATUS  InternalSendMsg (const CmbPacket& Message,
                            DdmServices *pCallbackInst, MsgCallback fnCallback,
                            void *pvCookie, U32 cusReplyTimeout,
                            CSendContext *& pSendDeferQueue);

   //  helper for starting transmit of a packet (handles queuing too)
   STATUS  StartSend (CSendContext *& pPacketQueue);

   //  watchdog timer op callback -- we don't do anything now.
   void  WatchdogOpCallback (STATUS /* sResult */ )
                  {  return;  };

   //  watchdog timeout callback - fails current send operation
   void  WatchdogTimeoutCallback (void *pvTimeoutCookie);

   //  common routine used to finish off a send's business with caller
   void  NotifySenderAndFreeContext (const CmbPacket *pReplyPkt,
                                     STATUS sResult);

   //  we keep a list of free contexts, so our queue can't be unbounded
   CSendContext * m_pFreeSendCtx;

   //  here's the send context currently in transit
   CSendContext * m_pCurrentSendCtx;

   //  here's the head of the pending list of transmits
   CSendContext * m_PendingSendCtx;

   //  here's the head of the pending list of replies to unsolicited cmds
   //  (these replies get absolute priority over sends in *m_PendingSendCtx
   CSendContext * m_PendingReplySendCtx;


   //  private ACTIONCALLBACK() helper for CmbHwIntfDummy.cpp version:
   STATUS  DoCallbackAction (CSendContext *pActionCookie);

//BUGBUG - here's some hack stuff for circumventing possible watchdog issues

   //  refnum to our own timer start (RqOsTimerStart) message
   REFNUM * m_prefPeriodicTimer;

   //  callback entry point for our periodic timer
   STATUS  PeriodicTimerCallback (Message *pReply);

   //  helper for encapsulating "local AVR timeout" processing.
   void  TimeoutCurrentSend (void);

};  /* end of class CCmbHwIntf */


#endif  /* #ifndef _CmbHwIntf_h_ */

