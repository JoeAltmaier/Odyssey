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
// File: CmbEventRecv.h
// 
// Description:
//  This file defines class CCmbEventRecv.  This class is used to
//  perform semi-autonomous recovery of event entries sent to us
//  via the CMB.  We are fed a succession of k_eCmbCmdSendLogEntry
//  CMB command packets.  Each contains a fragment of an event entry,
//  from some IOP.
//
//  We group the fragments according to their IOP of origin, and their
//  event sequence number.  Once all fragments have been received, we
//  extract the full event entry, and submit it to the event log for
//  recording.
//
//  * This class is meant only to operate on the master HBC, where
//    the event log lives.
//
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbEventRecv.h $
// 
// 2     11/24/99 3:38p Ewedel
// Added watchdog's timeout cookie.
// 
/*************************************************************************/


#ifndef _CmbEventRecv_h_
#define _CmbEventRecv_h_


#ifndef _CtTimeGuard_h_
# include  "CtTimeGuard.h"
#endif

#ifndef _CmbHwIntfMsgs_h_
# include  "CmbHwIntfMsgs.h"
#endif

#ifndef _CmbSendEventDefs_h_
# include  "CmbSendEventDefs.h"
#endif



//  the interface to the fragment reassembler; there is one instance of
//  this class, declared as a private member of DdmCmb
class CCmbEventRecv
{
public:

   //  our constructor, sets up what we need
   CCmbEventRecv (DdmServices *pParentDdm) : m_pParentDdm (pParentDdm)
            {  m_pEventList = NULL;  };

   //  submit an event fragment to us for reassembly
   void  ProcessFragment (const CmbPacket& pktFragment);

   //  what we use to represent a fragment of an event entry
   class CFrag
   {
   public:
      CFrag     * pNext;
      CmbPacket   pktFrag;

      CFrag (const CmbPacket& pktSrc, CFrag *_pNext = NULL)
      {  pktFrag = pktSrc;
         pNext = _pNext;  };
   };


private:

   //  what we use to track one event entry
   class CEventAssy : public CCtTimedService
   {
   public:

      //  we define a simple, static callback which our container uses
      //  to remove us @ completion of our task (normal or timeout):
      typedef  void (*CompletionCallback) (CEventAssy *pCompletedInstance,
                                           void *pvCompletionCookie);

      //  our constructor, starts off a new assembly operation
      CEventAssy (DdmServices *pParentDdm, const CmbPacket& pktFirstFrag,
                  CompletionCallback pfnCompletionCallback,
                  void *pvCompletionCookie);

      //  test to see if a given fragment matches our event
      //  (we don't worry about duplicates here)
      inline BOOL  IsMine (const CmbPacket& pktFrag)  const
      {
         return ((pktFrag.Hdr.bSrcAddr == m_eOriginIop) &&
                 (pktFrag.abTail[offCmbSendEvtHdrEventSeqNum] == m_bEventSeqNum));
      }

      //  do the work of adding a fragment to our instance
      void  AddFragment (const CmbPacket& pktFragment);

      //  we provide a simple pointer for linking us into a list
      CEventAssy   * m_pNext;

   private:

      //  we keep a list of received fragments
      CFrag  * m_pFrags;

      //  count of fragments received so far
      U8       m_cFragsSoFar;

      //  count of bytes in non-duplicate fragments received so far
      //  (count excludes our meta-header data: this is event size only)
      U32      m_cbFragDataSoFar;

      //  origin of our event entry
      const TySlot   m_eOriginIop;

      //  IOP-relative sequence number of our event entry
      const U8       m_bEventSeqNum;

      //  count of fragments required to complete event
      const U8       m_cFragsTotal;

      //  completion callback address
      const CompletionCallback   m_pfnCompletionCallback;

      //  completion callback cookie value
      void * const   m_pvCompletionCookie;

      //  helper to actually invoke our container's completion callback
      //  (probably returns to us with our instance deleted!!)
      inline void  DoCompletionCallback (void)
      {
         if (m_pfnCompletionCallback != NULL)
            {
            m_pfnCompletionCallback (this, m_pvCompletionCookie);
            }
      };

      //  here's how long we allow before timing out receipt
      //  of the remaining fragments of an event entry:
      static const U32  m_cusTimeout;     // lsb = 1us

      //  dummy callback for various timer operations
      void  InertTimerCallback (STATUS sResult);

      //  continuation routine for AddFragment()
      void  AddFragment2 (STATUS sResult);

      //  helper for submitting an event to the logger
      void  SubmitEvent (void);

      //  our watchdog timer's timeout callback
      void  TimeoutCB (void *pvTimeoutCookie);


   };  /* end of class CEventAssy */
     
   //  resume class CCmbEventRecv:

   //  we keep a copy of our parent DDM instance ptr, since it is needed
   //  by each CEventAssy which we instantiate
   DdmServices  * const m_pParentDdm;

   //  in main CCmbEventRecv helper class, we keep a list of outstanding
   //  event assembly operations:
   CEventAssy   * m_pEventList;

   //  helper for processing event assembly completion callbacks
   static void  EventCompletionCallback (CEventAssy *pCompletedEvent,
                                         void *pvMyInstance);

};  /* end of class CCmbEventRecv */

#endif  /* #ifndef _CmbEventRecv_h_ */


