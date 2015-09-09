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
// File: CmbEventSend.h
// 
// Description:
//  This file defines class CCmbEventSend.  This class is used to
//  perform the autonomous submission of an event log entry from
//  an IOP to the current master HBC.
//
//  Because CCmbEventSend performs autonomous CMB interface calls,
//  it is defined as a member of the DdmServices OS-level base class.
//
// 
// $Log: $
// 
/*************************************************************************/


#ifndef _CmbEventSend_h_
#define _CmbEventSend_h_


#ifndef __DdmOsServices_h
# include  "DdmOsServices.h"
#endif

#ifndef _CmbHwIntf_h_
# include  "CmbHwIntf.h"
#endif

#ifndef _CmbMsgSendWrapper_h_
# include  "CmbMsgSendWrapper.h"
#endif



class CCmbEventSend : public DdmServices
{
public:

   //  our only constructor, sets up what we need (we consume *pEvent!)
   CCmbEventSend (DdmServices *pParentDdm, CCmbHwIntf& m_CmbHwIntf,
                  Event *pEvent);

private:

   //  reference to CMB's hardware interface instance
   CCmbHwIntf& m_CmbHwIntf;

   //  event in "packed" form for transmission
   char   * m_pchPackedEvent;

   //  one-based count of bytes in packed form
   U32      m_cchPackedEvent;

   //  one-based count of bytes remaining to send
   U32      m_cchRemaining;

   //  points to beginning of fragment we're sending right now
   char   * m_pchRemaining;

   //  one-based count of bytes in fragment we're sending right now
   U32      m_cchCurSend;

   //  CMB packet sender in which we place the current fragment we're sending
   CCmbMsgSender  m_cmdFragSend;

   //  count of send retries on this fragment
   U32      m_cCurFragRetries;

   //  zero-based index of fragment we're sending now (embedded in packet)
   U8       m_iFrag;

   //  the sequence number (unique within our IOP) assigned to this event
   //  (used to reassemble fragments at the HBC)
   //  A byte is sufficient here, since we control sequencing internally
   //  to avoid overrunning the CmbHwIntf's buffers.  The result is that
   //  we should send a single event log entry all the way through before
   //  moving on to the next.
   const U8    m_bSeqNum;

   //  our source of IOP-global unique sequence numbers
   static U8   m_bThisIopSeqNum;

   //  how big our own per-fragment message overhead is
   static const U32  m_cbMyHeader;

   //  queue of outstanding event send requests
   static CCmbEventSend   * m_pCurSender;

   //  link for queuing outstanding send requests
   CCmbEventSend   * m_pNextSender;


   //  send next fragment off to HBC (deletes our instance if
   //  all fragments have been sent)
   void  SendNextFragment (void);

   //  callback used to continue send of a given event entry
   void  FragmentCallback (void *pvCookie, STATUS sStatus,
                           const CmbPacket *pReply);

};  /* end of class CCmbEventSend */

#endif  /* #ifndef _CmbEventSend_h_ */


