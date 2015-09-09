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
// File: CmbMsgSendWrapper.h
// 
// Description:
//    Defines a helper class, CCmbMsgSender, which simplifies tracking
//    of messages (requests) sent from the MIPS to the CMB, and their
//    replies.
//
//    The main motivation for this class is the need to persist messages
//    across DDM method invocations.  It functions in a manner analogous
//    to (and inspired by :-) the PTS interface wrappers (TS* classes).
//
//    Note that, unlike the TS* classes, CCmbMsgSender instances may
//    *optionally* self-destruct after completion of the caller's
//    callback routine.  The choice of self-destruct is dictated by
//    how an instance is created and initialized: refer to the routine
//    descriptions below or in our .cpp file for details.
//    [Making self-destruct optional allows inclusion of CCmbMsgSender
//     instances as members within other classes.]
// 
// $Log: /Gemini/Odyssey/DdmCmb/CmbMsgSendWrapper.h $
// 
// 8     1/20/00 4:32p Eric_wedel
// Added some more debug tracking, and made CMBCALLBACK() macro
// type-strong.
// 
// 7     11/24/99 5:18p Ewedel
// Added DoTimeoutRetry().
// 
// 5     8/11/99 7:51p Ewedel
// Added direct access to embedded CMB packet, because sometimes you just
// need it.
// 
// 4     8/04/99 1:10p Ewedel
// Added send-retry support when CMB timeout (on-bus only) is seen.
// 
// 3     6/16/99 8:23p Ewedel
// Added support for defining valid packet source addresses (after get
// board info has been called, natch).
// 
// 2     6/14/99 11:57p Ewedel
// Cleaned up & brought closer to reality.
// 
// 1     6/03/99 7:19p Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _CmbMsgSendWrapper_h_
#define _CmbMsgSendWrapper_h_


#ifndef _CmbHwIntf_h_
# include  "CmbHwIntf.h"
#endif

#ifndef __DdmOsServices_h
# include  "DdmOsServices.h"
#endif

#ifndef _CmbDebugHack_h_
# include  "CmbDebugHack.h"      //BUGBUG - debug hack
#endif


class CCmbMsgSender : public DdmServices
{
public:

   //  our constructor - makes a ready-to-use packet, in the simple case
   //  (packet is flagged for self-destruct @ completion of callback)
   CCmbMsgSender (DdmServices *pCallbackInst, U8 bCommand, U8 bDest,
                  U8 cbData = 0);

   //  another constructor, for use in conjunction with Initialize()
   //  (packet is flagged to *not* self-destruct @ completion of callback)
   inline CCmbMsgSender (DdmServices *pHostDdm) : DdmServices(pHostDdm),
                                                  m_pHostDdm (pHostDdm)
               {  CmbDebugInfo.cMsgSendWrappersExtant ++;
                  Initialize (0, 0);  };

   //  destructor, strictly for debug porpoises
   ~CCmbMsgSender ()
      {
//BUGBUG - debug info
      CmbDebugInfo.cMsgSendWrappersExtant --;
//BUGBUG - endit
      //  set a marker to indicate we're destroyed
      m_pCmbIntf = NULL;
      };

   //  (re-) initialize an instance
   //  (clears param tail, and suppresses self-destruct behavior)
   void  Initialize (U8 bCommand, U8 bDest, U8 cbData = 0);

   //  add parameter values to the packet
   void  AddParam (U8 ibParamOffset, U8 bData);    // byte-sized param
   void  AddParam (U8 ibParamOffset, U32 nData);   // integer-sized param

   //  allow direct access to packet for those difficult setup cases
   inline CmbPacket *  PacketPtr (void)
                     {  return (m_pPacket);  };


   //  *** add other message editing helpers here ***

   //  what our send completion callback looks like
   typedef void  (DdmServices::*CmbCallback) (void *pvCookie, STATUS status,
                                              const CmbPacket *pReply);

   //  the guy which starts things off:  send this message to our local CMA.
   //  [Status always comes back via *pReplyCallback()]
   void  Send (CCmbHwIntf& CmbIntf, void *pvCookie, CmbCallback pReplyCallback);

   //  tell us what SRC address to use for new packets
   static inline void  SetSourceAddr (TySlot eMySource)
                        {  m_eMyPacketSrc = (TySlot) (eMySource | CmbAddrMips);  };

private:

   //  our host DDM
   DdmOsServices   * const m_pHostDdm;

   //  our host DDM's CMB hardware interface instance
   CCmbHwIntf      * m_pCmbIntf;

   //  points to the packet which we actually send (may be m_LittlePacket,
   //  or may be allocated from the heap)
   CmbPacket * m_pPacket;

   //  our packet data.  We used to choose between this and a dynamically
   //  allocated packet, but now raw CmbPacket is as big as packets are
   //  allowed to get, so we always use it.
   CmbPacket   m_LittlePacket;

   //  here's where we keep the caller's cookie, while we're using
   //  the cookie slot for our own instance ptr
   void      * m_pvCallersCookie;

   //  and here's the caller's callback
   CmbCallback m_CallersCallback;

   //  count of retries made so far on sending our current packet
   U32         m_cSendRetries;

   //  holder for error status "passed" to ImmediateCallback() (see below)
   STATUS      m_sRetError;

   //  flag indicating whether our instance should self-destruct
   //  after completing requestor's callback
   BOOL        m_fKeepInstance;     // false == self-destruct @ completion

   //  maximum permitted send retries
   static const U32  m_cSendRetriesMax;

   //  what our standard source address is
   static TySlot     m_eMyPacketSrc;

   //  intermediate callback which we feed to CmbIntf..
   void  MyCmbCallback (void *pvCookie, STATUS sRet,
                        const CmbPacket *pReply);

   //  another intermediate callback, used with Action() to report errors
   STATUS  ImmediateCallback (void *pvCookie);

   //  helper for encapsulating send timeout-retry logic
   BOOL  DoTimeoutRetry (void);

};  /* end of CCmbMsgSender */


//  here's a type-safe function pointer cast helper template.
//  As with other such critters, this one takes a parameter whose
//  signature is identical to its return value's, except for being
//  in the native class specified by the caller.
template<class Caller> inline CCmbMsgSender::CmbCallback CmbCallbackCast
                  (void (Caller::*pCmbCallback) (void *pvCookie, STATUS status,
                                                 const CmbPacket *pReply))
   {  return ((CCmbMsgSender::CmbCallback) pCmbCallback);  };

//  here's a little macro helper to take the address of a member function,
//  regardless of the compiler environment we're using.
#if defined(__ghs__)    // Green Hills
#define  CMBCALLBACK(clas, method)     (CmbCallbackCast<clas> (&clas::method))
#else       // Metrowerks
#define  CMBCALLBACK(clas, method)     (CmbCallbackCast<clas> (&method))
#endif


#endif  // #ifndef _CmbMsgSendWrapper_h_


