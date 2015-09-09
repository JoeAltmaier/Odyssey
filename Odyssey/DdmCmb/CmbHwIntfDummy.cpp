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
// File: CmbHwIntfDummy.cpp
//
// Description:
//    Card Management Bus MIPS DDM / Atmel microcontroller interface code.
//
//    This is a dummy version, which simulates an HBC with working CMA by
//    returning dummy data in response to various CMB commands of interest.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbHwIntfDummy.cpp $
// 
// 11    1/20/00 4:21p Eric_wedel
// Got back into clean-compiling condition, but not tested since eval
// crate suppose is presently busted.
// 
// 10    10/08/99 11:59a Ewedel
// Changed k_eMyHbcSlot & friends to be in sync with Address::iSlotMe as
// reported on an eval crate (or default Odyssey slot 0 HBC).  Also
// changed some dummy slot data so that types fit within current 3 bit
// field constraint.
// 
// 8     9/03/99 5:20p Ewedel
// Added dummy EVC and DDH slot entries, and enhanced debug reporting to
// support full EVC queries (poll environment, etc.).
// 
// 7     8/11/99 7:52p Ewedel
// Tweaked to work with updated CMB command set, and filled in several
// more "get last value" response types.
// 
// 6     7/15/99 4:24p Ewedel
// Added test support for unsolicited packets, and updated for CmbPacket
// format change.
// 
// 5     6/24/99 7:24p Ewedel
// Updated for constant name changes, and revised power/PCI control CMB
// commands.
// 
// 4     6/18/99 1:12p Ewedel
// Added support for "status poll" command, and funny little hack to let
// powered-up IOPs gradually transition into "awaiting boot" state.  Also
// fixed a few bugs & warnings.
// 
// 3     6/17/99 1:54p Ewedel
// Changed DdmOsServices to DdmServices.
// 
// 2     6/16/99 8:22p Ewedel
// Fixed misc bugs, added more robust slot/state tracking.
// 
// 1     6/15/99 12:04a Ewedel
// Initial revision.
//
/*************************************************************************/


#include  "CmbHwIntf.h"

#include  "DdmCMB.h"       // for CT_IOPST_MAX_IOP_SLOTS def

#include  "CtEvent.h"      // status codes

#include  "Odyssey_Trace.h"

#include  <assert.h>


//  little helper for issuing prompts (lives in CmbDdmCmdProc.cpp)
extern void  IssuePrompt (const char *pszPromptFmt, ...);


//  lots of funky static data, since we don't want to tamper with the
//  class definition which we're emulating

//  actual storage allocated for our standard "reply" packet
static U8  abReplyPkt [32];     // as big as CMB packets ever get

//  and the typed pointer to our reply packet
static CmbPacket   * const ppktReply  =  (CmbPacket *) abReplyPkt;

//  here's the slot we report as our own HBC slot, in get board info
//  (for compatibility with Address::iSlotMe, we set slot 0)
//  NOTE:  be sure that the corresponding row in table aIopSlot[] (see
//         below) is defined as "booting PCI image".
//*const TySlot  k_eMyHbcSlot = IOP_HBC1;
const TySlot  k_eMyHbcSlot = IOP_HBC0;


//  pointer to the hardware interface instance which we're emulating
static CCmbHwIntf  * This;


//  special table for mapping from TySlot to "contiguous slot" index

static const TySlot  aeContigToSlot [CT_IOPST_MAX_IOP_SLOTS]  =  {
   IOP_HBC0,  IOP_HBC1,
   IOP_SSDU0, IOP_SSDU1, IOP_SSDU2, IOP_SSDU3, 
   IOP_SSDL0, IOP_SSDL1, IOP_SSDL2, IOP_SSDL3, 
   IOP_RAC0,  IOP_APP0,  IOP_APP1,  IOP_NIC0,
   IOP_RAC1,  IOP_APP2,  IOP_APP3,  IOP_NIC1,
   CMB_EVC0,  CMB_EVC1,
   CMB_DDH0,  CMB_DDH1,  CMB_DDH2,  CMB_DDH3 
};


//  *** here is some dummy data for describing what's in the various IOP slots
//      (with no CMB hardware, we don't have any easy way of discovering the
//       real data)

//  array must have as many entries as m_aeContigToSlot[]

typedef struct {
   CmbMipsState   eCmbState;
   IopType        eType;
   const char   * pszSerialNo;
   U32            cRepliesBeforeBoot;     // count of status replies from slot
                                          //  prior to boot
} IopSlotEntry;


//  this array is the master, of which PTS is only a shadow.  [3/31/99 only]
//  Of course, in the real Odyssey, PTS will be the master copy.
//  This array is indexed by "contiguous slot ID", as returned by ValidIopSlot()
static IopSlotEntry   aIopSlot [CT_IOPST_MAX_IOP_SLOTS]  =  {
   {  k_eCmbMStBootingPCIImage,  IOPTY_HBC,  "HBC.223"  },  // IOP_HBC0
   {  k_eCmbMStNotFound },                                  // IOP_HBC1
//   {  k_eCmbMStPoweredOff,       IOPTY_SSD,  "SSD.122"  },  // IOP_SSDU0
   {  k_eCmbMStNotFound },                                  // IOP_SSDU0
   {  k_eCmbMStNotFound },                                  // IOP_SSDU1
   {  k_eCmbMStNotFound },                                  // IOP_SSDU2
   {  k_eCmbMStNotFound },                                  // IOP_SSDU3
   {  k_eCmbMStNotFound },                                  // IOP_SSDL0
   {  k_eCmbMStNotFound },                                  // IOP_SSDL1
   {  k_eCmbMStNotFound },                                  // IOP_SSDL2
   {  k_eCmbMStNotFound },                                  // IOP_SSDL3
   {  k_eCmbMStPoweredOff,       IOPTY_NAC,  "NAC.003"   }, // IOP_RAC0
   {  k_eCmbMStNotFound },                                  // IOP_APP0
   {  k_eCmbMStNotFound },                                  // IOP_APP1
   {  k_eCmbMStNotFound },                                  // IOP_NIC0
   {  k_eCmbMStNotFound },                                  // IOP_RAC1
   {  k_eCmbMStNotFound },                                  // IOP_APP2
   {  k_eCmbMStNotFound },                                  // IOP_APP3
   {  k_eCmbMStPoweredOff,       IOPTY_SSD,  "SSD.006"   }, // IOP_NIC1
   {  k_eCmbMStNotFound },                                  // CMB_EVC0
   {  k_eCmbMStPoweredOn,        IOPTY_EVC,  "EVC.003"   }, // CMB_EVC1
   {  k_eCmbMStNotFound },                                  // CMB_DDH0
   {  k_eCmbMStNotFound },                                  // CMB_DDH1
   {  k_eCmbMStPoweredOn,        IOPTY_DDH,  "DDH.007"   }, // CMB_DDH2
   {  k_eCmbMStNotFound },                                  // CMB_DDH3
};


//  an arbitrary, empty slot which we eventually change to "populated"
const TySlot  k_eNewBoardSlot = IOP_RAC1;


//  here's the carrier object which we use for queueing unsolicited packets
typedef struct _UnsolicitedPkt {
   struct _UnsolicitedPkt   * pNext;
   CmbPacket                  pkt;
} UnsolicitedPkt;


//  this is the head of our list of outstanding unsolicited packets
static  UnsolicitedPkt   * pUnsolicitedPktList = NULL;


//  here's where we keep a list of outstanding event log fragment sends
//  (those which are awaiting reponses)
static CCmbHwIntf::CSendContext   * pEventLogSendCallbackQueue = NULL;


//  helper routines for processing various command packet types

//  request current status of a CMB slot, as reported by slot's AVR
static STATUS  DoStatusPoll (const CmbPacket& pktCommand,
                             CmbPacket& pktReply);

//  request status of a CMB slot, as cached by the CMB master AVR
//  get common info about a board, typically our local IOP (CMB_SELF)
static STATUS  DoGetBoardInfo (const CmbPacket& pktCommand,
                               CmbPacket& pktReply);

//  request status of a CMB slot, as cached by the CMB master AVR
static STATUS  DoCmbSlotStatus (const CmbPacket& pktCommand,
                                CmbPacket& pktReply);

//  set our own CMA's "mips state" value
static STATUS  DoSetMipsState (const CmbPacket& pktCommand,
                               CmbPacket& pktReply);

//  read most recently polled value from target IOP's CMA
static STATUS  DoGetLastValue (const CmbPacket& pktCommand,
                               CmbPacket& pktReply);

//  handle MIPS CPU power-on and power-off commands
static STATUS  DoIopPowerCtl (const CmbPacket& pktCommand,
                              CmbPacket& pktReply);

//  handle "set PCI window" command
static STATUS  DoSetPciWindow (const CmbPacket& pktCommand,
                               CmbPacket& pktReply);

//  handle "set boot params" command
static STATUS  DoSetBootParams (const CmbPacket& pktCommand,
                                CmbPacket& pktReply);

//  handle "send small DDM message" command
static STATUS  DoSendDdmMessage (const CmbPacket& pktCommand,
                                 CmbPacket& pktReply);

//  handle "send event log entry [fragment]" command
static STATUS  DoSendLogEntry (const CmbPacket& pktCommand,
                               CmbPacket& pktReply,
                               CCmbHwIntf::CSendContext *& pActCook);


//  various utility helper routines

//  make a proto-reply packet, starting from a source command packet
static void  MakeReplyFrame (U32 iContigSlot, const CmbPacket& pktCommand,
                             CmbPacket& pktReply, U8 cbParam = 0);

//  do an unsolicited slot status update for the given slot
static void  PostSlotNotify (U32 iContigSlot);

//  do an arbitrary unsolicited notification of the CMB DDM
static void  PostNotify (const CmbPacket& pktNotify);

//  validate a CMB "dest" value, and convert to our "contiguous slot index"
static STATUS  ValidIopSlot (TySlot eSlot, U32& iContigSlot);

//  format a slot-specific prompt string
static void  MakeSlotMessage (char *pchMsgBuf, const char *pszMsgPrefix,
                              U8 bDestSlot);


//  here's a little hook for network byte order conversion..
//  (MIPS and network byte order are both big-endian, so this is a no-op)
static inline U32 ntohl (U32 u32NetOrder)
         {  return (u32NetOrder);  }


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
//                This value is used to keep our DdmServices base
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


   //  verify that our own "HBC" slot is the same as Address:: thinks it is
   assert (k_eMyHbcSlot == Address::iSlotMe);

   //  mark that "transmit" interface to Atmel is idle
   m_fXmitToAtmelIsIdle = TRUE;

//*   //  claim our "IRQ" signal code's dispatches
//*   DispatchSignal(m_nSigCmbIrq,
//*                  SIGNALCALLBACK (CCmbHwIntf, HandleCmbIrqSignal));

   //  correctly set our private "This" pointer hack
   This = this;

   return;

}  /* end of CCmbHwIntf::CCmbHwIntf */

//
//  CCmbHwIntf::ReadMsg (MsgBuf, cbMsgBufMax)
//
//  Description:
//    Called by our host DDM to read an unsolicited AVR message, or a reply
//    to a Send() which didn't include a callback.
//
//    In general, this routine should be called in response to nSigCmbMsgRdy
//    signal dispatches in our host DDM.
//
//  Inputs:
//    MsgBuf - Where to place the returned message.
//    cbMsgBufMax - One-based count of bytes available in MsgBuf.
//
//  Outputs:
//    MsgBuf - Loaded with returned message, if buffer is big enough.
//             Loaded with truncated message otherwise.  Doing this permits
//             the caller to determine how large of a buffer is needed to
//             store the entire message.
//    CCmbHwIntf::ReadMsg - Returns CTS_SUCCESS if the whole message could
//             be returned, or an error message if either no message remains
//             to read, or the next message exceeds the caller's buffer size.
//

STATUS CCmbHwIntf::ReadMsg (CmbPacket& MsgBuf, int cbMsgBufMax)
{

U32               cbMsg;         // actual packet size
UnsolicitedPkt  * pCarrier;


   //  are there any packets waiting to return?
   if (pUnsolicitedPktList == NULL)
      {
      return (CTS_CMB_NO_UNSOLICITED_MSG);
      }

   //  got a message, figure out how big it is
   cbMsg = pUnsolicitedPktList->pkt.Size ();

   assert (cbMsgBufMax >= cbMsg);

   if (cbMsgBufMax < cbMsg)
      {
      //  whoops, packet too big for caller's buffer
      return (CTS_CMB_BUFFER_TOO_SMALL);
      }

   //  message fits in caller's buffer, so return it

   //  unlink message first
   pCarrier = pUnsolicitedPktList;
   pUnsolicitedPktList = pUnsolicitedPktList->pNext;

   //  return its data
   memcpy (&MsgBuf, &pCarrier->pkt, cbMsg);

   //  and free carrier instance
   delete pCarrier;

   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::ReadMsg */

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

   //  we're all about pretending to be real, so say "yes":
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

   #pragma unused(fEnableInput)

}  /* end of CCmbHwIntf::EnableInput */

//
//  CCmbHwIntf::SendMsg (pktMessage, pCallbackInst, fnCallback,
//                       pvCookie, cusReplyTimeout)
//
//  Description:
//    Dummy routine for emulating send of a message to the AVR.
//
//    We process the submitted message, and call the caller's callback
//    directly.  [Note that direct callback is also done by the "March build"
//    hardware interface, since it is polled.]
//    If no callback is specified, then we'll send a signal to the DDM,
//    again per standard CCmbHwIntf definition.
//
//  Inputs:
//    pktMessage - CMB message to process.  Must be a command packet
//                at present, replies are not supported.
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

STATUS  CCmbHwIntf::SendMsg (const CmbPacket& pktMessage,
                             DdmServices *pCallbackInst,
                             MsgCallback fnCallback,
                             void *pvCookie /* = NULL */ ,
                             U32 cusReplyTimeout /* = 0 */ )
{


   //  being as we're fake, we simply crack the packet and process
   //  those which we understand

   //  for now, we don't support sending reply packets
//*   assert ((pktMessage.Hdr.bStatus & CmbStatCmd) != 0);

   //  make sure that unless dest address is "self", we also have
   //  some sort of return address
   assert ((pktMessage.Hdr.bDestAddr == CMB_SELF) ||
           (pktMessage.Hdr.bSrcAddr == (k_eMyHbcSlot | CmbAddrMips)));

   //  and we require a callback, since our author is a lazy bum
   assert (fnCallback != NULL);

   //  we should always have our standard reply buffer available
   assert (ppktReply != NULL);

   //  do our common message handler (queue arg is bogus in dummy code)
   return (InternalSendMsg (pktMessage, pCallbackInst, fnCallback, pvCookie,
                            cusReplyTimeout, m_PendingSendCtx));

}  /* end of CCmbHwIntf::SendMsg */

//
//  CCmbHwIntf::SendReplyToUnsolicitedCmd (pktReply)
//
//  Description:
//    Dummy routine for emulating send of an unsolicited command reply
//    message to the AVR.
//
//    We find the matching outstanding command send (sent by the "other"
//    MIPS CPU), and release it to perform its callback operation.
//
//  Inputs:
//    pktReply - Reply packet to send along to originator of
//                unsolicited command.
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


   //  being as we're fake, we simply crack the packet and process
   //  those which we understand

   //  for now, we don't support sending reply packets
//*   assert ((pktMessage.Hdr.bStatus & CmbStatCmd) != 0);

   //  make sure that unless dest address is "self", we also have
   //  some sort of return address
   //  dest address should never be "self", and should have MIPS bit set:
   //BUGBUG - whilst debugging single-CPU, must disable this check:
//*   assert ((pktReply.Hdr.bDestAddr != CMB_SELF) &&
//*           ((pktReply.Hdr.bDestAddr & CmbAddrMips) != 0) &&
//*           ((pktReply.Hdr.bDestAddr & ~CmbAddrMips) != k_eMyHbcSlot));

   assert (((pktReply.Hdr.bStatus & CmbStatCmd) == 0) &&
           ((pktReply.Hdr.bStatus & CmbStatAck) != 0));

   //  do our common message handler (queue arg is bogus in dummy code)
   return (InternalSendMsg (pktReply, NULL, NULL,     // no callback!
                            NULL, 0, m_PendingReplySendCtx));

}  /* end of CCmbHwIntf::SendReplyToUnsolicitedCmd */

//
//  CCmbHwIntf::InternalSendMsg (pktMessage, pCallbackInst, fnCallback,
//                               pvCookie, cusReplyTimeout, pSendDeferQueue)
//
//  Description:
//    Dummy routine for emulating send of a message to the AVR.
//
//    We process the submitted message, and call the caller's callback
//    directly.  [Note that direct callback is also done by the "March build"
//    hardware interface, since it is polled.]
//    If no callback is specified, then we'll send a signal to the DDM,
//    again per standard CCmbHwIntf definition.
//
//  Inputs:
//    pktMessage - CMB message to process.  Must be a command packet
//                at present, replies are not supported.
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
//    pSendDeferQueue - Queue into which we would defer message.
//                But only real code does this, not our dummy stuff.
//
//  Outputs:
//    CCmbHwIntf::SendMsg - Returns CTS_SUCCESS if the send initiates
//                properly, else something else.  We typically don't
//                report the final result of the send, since that involves
//                waiting until the AVR replies to us (a lengthy proposition).
//                If you wish to know the results, then specify a callback
//                parameter.
//

STATUS  CCmbHwIntf::InternalSendMsg (const CmbPacket& pktMessage,
                                     DdmServices *pCallbackInst,
                                     MsgCallback fnCallback, void *pvCookie,
                                     U32 cusReplyTimeout,
                                     CSendContext *& pSendDeferQueue)
{

CSendContext * pActCook;
STATUS         sMyRet;

#pragma unused(cusReplyTimeout)		// for now, we rely on AVR
#pragma unused(pSendDeferQueue)


   //  being as we're fake, we simply crack the packet and process
   //  those which we understand


   //  create a default action cookie - some of our helpers might
   //  modify this (or even "steal" it, setting pActCook == NULL).
   pActCook = new CSendContext (pvCookie, pCallbackInst, fnCallback,
                                CTS_SUCCESS, NULL);


   //  process caller's packet
   switch (pktMessage.Hdr.bCommand)
      {
      case k_eCmbCmdStatusPoll:
         sMyRet = DoStatusPoll (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdGetBoardInfo:
         sMyRet = DoGetBoardInfo (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdCmbSlotStatus:
         //  request status of a CMB slot, as cached by the CMB master AVR
         sMyRet = DoCmbSlotStatus (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdSetMipsState:
         //  set our own CMA's "mips state" value
         sMyRet = DoSetMipsState (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdGetLastValue:
         //  read most recently polled value from target IOP's CMA
         sMyRet = DoGetLastValue (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdPowerControl:
         sMyRet = DoIopPowerCtl (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdSetPciWindow:
         sMyRet = DoSetPciWindow (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdSetBootParams:
         sMyRet = DoSetBootParams (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdDdmMessage:
         sMyRet = DoSendDdmMessage (pktMessage, *ppktReply);
         break;

      case k_eCmbCmdSendLogEntry:
         sMyRet = DoSendLogEntry (pktMessage, *ppktReply, pActCook);
         break;

      default:
         Tracef ("CCmbHwIntf::SendMsg [dummy]: unsupported command: %d\n",
                 pktMessage.Hdr.bCommand);
         assert (FALSE);

         //BUGBUG - could build up a dummy reply packet here
      }

   if (pActCook == NULL)
      {
      //  our action cookie got hijacked, so we do nothing now.
      //  [The routine which did the hijack has posted an unsolicited
      //   message to our CMB DDM: the DDM's response will reinstitute
      //   the reply which our action cookie would have performed.]
      }
   else
      {
      //  we still have an action cookie (though it might not be the
      //  one we created), so we're supposed to use it.

      pActCook->sResult = sMyRet;

      //  figure out what updates our cookie needs to reflect our result:
      if (sMyRet == CTS_SUCCESS)
         {
         //  all is coolness
         pActCook->SetPkt (*ppktReply);
         }

      //  now queue callback processing for a fresh stack context
      Action (ACTIONCALLBACK (CCmbHwIntf, DoCallbackAction), pActCook);
      }

   //  we always report "send" success to our caller
   //  [note that we're returning before calling the callback]
   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::InternalSendMsg */

//
//  CCmbHwIntf::DoCallbackAction (pActionCookie)
//
//  Description:
//    Handles an "action callback", yet another cool OS mechanism
//    brought to us by those elves in Iowa.
//
//    Actions are simply calls to ourself, which are queued through
//    the standard DDM message queuing machinery.  This means that
//    when we chain execution via an action, we start the action
//    with a fresh stack context.  This is nice way to limit stack
//    depth, as well as breaking up long operations (at an action
//    "boundary", we might relinquish the CPU to other waiting DDMs).
//
//    This here action routine exists simply to finish the dummy
//    request processing initiated by CCmbHwIntf::SendMsg() above.
//    We call the requestor's callback, and then tidy up.
//
//  Inputs:
//    pActionCookie - Stuff assembled by SendMsg().  These are the
//                parameters which we supply to the user's callback.
//
//  Outputs:
//    CCmbHwIntf::DoCallbackAction - Returns CTS_SUCCESS, always.
//

STATUS  CCmbHwIntf::DoCallbackAction (CSendContext *pActionCookie)
{

CmbPacket * ppkt;


   //  figure out whether we're supposed to supply a packet pointer
   if (pActionCookie->fHasPacket)
      {
      ppkt = &pActionCookie->Pkt;
      }
   else
      {
      ppkt = NULL;
      }

   //  we have a reply packet to send back, so let's do it
   ((pActionCookie->pCallbackInst)->*(pActionCookie->fnCallback))
                                       (pActionCookie->pvCookie,
                                        pActionCookie->sResult,
                                        ppkt);

   //  now dispose of our temp cookie
   delete pActionCookie;

   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::DoCallbackAction */

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
//    pPayload - Points to stuff fed to us by our ISR.  Probably not
//                needed, since we can share whatever we need through
//                our instance data.
//
//  Outputs:
//    CCmbHwIntf::HandleCmbIrq - Returns CTS_SUCCESS if all goes well,
//                or else an error code.
//                Are we actually permitted to return an error here?
//

STATUS  CCmbHwIntf::HandleCmbIrqSignal (SIGNALCODE nSignal, void *pPayload)
{


   //  do stuff here necessary to prep for host DDM's call to ReadMsg()
   // ...

   //  this routine may optionally be used as a handoff point when sending
   //  a signal to our host DDM.  It *Must* be used as a handoff point when
   //  calling a host DDM-specified callback function.  This is because as
   //  we transfer control to this routine by use of a signal (which CHAOS
   //  routes through our host DDM), CHAOS shifts our execution into our
   //  host DDM's thread context, where it should be in order to execute
   //  a callback supplied by the host DDM.

   //  if appropriate, signal host DDM of pending message from AVR
//   Signal (m_pHostDdm);    // (no payload sent)

   //  if appropriate, call a host DDM-supplied "response-received" callback
//   (*pHostCallback) (pvCookie, Status, pReply)

#pragma unused(nSignal)
#pragma unused(pPayload)

   return (CTS_SUCCESS);

}  /* end of CCmbHwIntf::HandleCmbIrqSignal */

//
//  DoStatusPoll (pktCommand, pktReply)
//
//  Description:
//    Handles a "poll status" command.
//
//    This command may be issued to any valid CMB slot.
//
//    Note: Our use of the standard MakeReplyFrame() method ensures
//          that after a couple of status poll calls, an IOP in the
//          "powered up" state will transition "by itself" into the
//          "awaiting boot" state.  See MakeReplyFrame() for the
//          actual trickery.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoGetBoardInfo - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//


static STATUS  DoStatusPoll (const CmbPacket& pktCommand,
                             CmbPacket& pktReply)
{

U32      iContigSlot;


   //  check some stuff that should always be so
   assert (pktCommand.Hdr.bCommand == k_eCmbCmdStatusPoll);
   assert (pktCommand.Hdr.cbData == 0);
   assert ((pktCommand.Hdr.bDestAddr & CmbAddrMips) == 0);

   //  find contig slot index for the slot we're claiming to be in
   ValidIopSlot ((TySlot) pktCommand.Hdr.bDestAddr, iContigSlot);
   
   //  build up our reply packet, and we're done (MakeReplyFrame does
   //  any necessary status jiggling).
   MakeReplyFrame (iContigSlot, pktCommand, pktReply);

   //  all done, report our result
   return (CTS_SUCCESS);

}  /* end of DoStatusPoll */

//
//  DoGetBoardInfo (pktCommand, pktReply)
//
//  Description:
//    Handles a "get board info" command.
//
//    We require that this command be issued to CMB_SELF, and then we
//    return suitable hard-wired information.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoGetBoardInfo - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoGetBoardInfo (const CmbPacket& pktCommand,
                               CmbPacket& pktReply)
{

U32      iContigSlot;


   //  check some stuff that should always be so
   assert (pktCommand.Hdr.bCommand == k_eCmbCmdGetBoardInfo);
   assert (pktCommand.Hdr.bDestAddr == CMB_SELF);
   assert (pktCommand.Hdr.cbData == 0);

   //  find contig slot index for the slot we're claiming to be in
   ValidIopSlot (k_eMyHbcSlot, iContigSlot);
   
   //  ok, let's build up our reply packet
   MakeReplyFrame (iContigSlot, pktCommand, pktReply, 3);

   pktReply.abTail[0] = IOPTY_HBC;
   pktReply.abTail[1] = k_eMyHbcSlot;
   pktReply.abTail[2] = 1;    // yes, we are the master HBC

   //  all done, report our result
   return (CTS_SUCCESS);

}  /* end of DoGetBoardInfo */

//
//  DoCmbSlotStatus (pktCommand, pktReply)
//
//  Description:
//    Handles a "CMB slot status" command.
//
//    We extract data for the requested slot from our dummy tables,
//    and return it.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoCmbSlotStatus - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoCmbSlotStatus (const CmbPacket& pktCommand,
                                CmbPacket& pktReply)
{

U32      iTargContigSlot;
U32      iMyContigSlot;
STATUS   ulMyRet;


   //  check some stuff that should always be so
   assert (pktCommand.Hdr.bCommand == k_eCmbCmdCmbSlotStatus);
   assert (pktCommand.Hdr.bDestAddr == CMB_SELF);
   assert (pktCommand.Hdr.cbData == 1);

   //  map our own slot, for state return
   ValidIopSlot (k_eMyHbcSlot, iMyContigSlot);

   //  extract target slot ID
   ulMyRet = ValidIopSlot ((TySlot) pktCommand.abTail[0], iTargContigSlot);
   if (ulMyRet == CTS_SUCCESS)
      {
      //  ok, got a good target, let's build up our reply packet
      MakeReplyFrame (iMyContigSlot, pktCommand, pktReply, 2);

      pktReply.abTail[0] = pktCommand.abTail[0];   // copy target slot ID
      pktReply.abTail[1] = aIopSlot[iTargContigSlot].eCmbState;
      }

   //  all done, report our result
   return (ulMyRet);

}  /* end of DoCmbSlotStatus */

//
//  DoSetMipsState (pktCommand, pktReply)
//
//  Description:
//    Handles a "set MIPS state" CMB command.
//
//    We update data for the requested slot in our dummy tables.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoSetMipsState - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoSetMipsState (const CmbPacket& pktCommand,
                               CmbPacket& pktReply)
{

U32      iContigSlot;
STATUS   ulMyRet;


   //  check some stuff that should always be so
   assert (pktCommand.Hdr.bCommand == k_eCmbCmdSetMipsState);
   assert (pktCommand.Hdr.bDestAddr == CMB_SELF);
   assert (pktCommand.Hdr.cbData == 1);
   assert ((k_eCmbMStUnknown <= pktCommand.abTail[0]) &&
           (pktCommand.abTail[0] <= k_eCmbMStRunningOSImage));

   //  define our own slot number
   ulMyRet = ValidIopSlot (k_eMyHbcSlot, iContigSlot);
   if (ulMyRet == CTS_SUCCESS)
      {
      //  update state in our dummy tables
      aIopSlot[iContigSlot].eCmbState = (CmbMipsState) pktCommand.abTail[0];

      //  and build up a happy reply packet
      MakeReplyFrame (iContigSlot, pktCommand, pktReply);
      }

   //  all done, report our result
   return (ulMyRet);

}  /* end of DoSetMipsState */

//
//  DoGetLastValue (pktCommand, pktReply)
//
//  Description:
//    Handles a "get last value" CMB command.
//
//    We either user our dummy tables or fish a value out of thin air.
//
//    ** Just to call attention to it, if somebody asks for a parameter
//       which we don't support yet, we'll throw an assert().
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoGetLastValue - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoGetLastValue (const CmbPacket& pktCommand,
                               CmbPacket& pktReply)
{

U32      iContigSlot;
STATUS   ulMyRet;


   //  check some stuff that should always be so
   assert (pktCommand.Hdr.bCommand == k_eCmbCmdGetLastValue);
   assert ((pktCommand.Hdr.bDestAddr & CmbAddrMips) == 0);
   assert (pktCommand.Hdr.cbData == 1);
//*   assert (pktCommand.Hdr.cbData == 2);

   //  map target slot number
   ulMyRet = ValidIopSlot ((TySlot) pktCommand.Hdr.bDestAddr, iContigSlot);
   if (ulMyRet == CTS_SUCCESS)
      {
      //  valid slot, make sure slot has a card in it
      assert (aIopSlot[iContigSlot].eCmbState != k_eCmbMStNotFound);
       
      //  now do different stuff based on which parameter is requested:
      switch (pktCommand.abTail[0])
         {
//*         case k_eCmbParamEvcPrimStatus:
//*            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 4);
//*            pktReply.abTail[0] = 7;    // all inputs (line voltage) ok
//*            pktReply.abTail[1] = 7;    // all outputs ok
//*            pktReply.abTail[2] = 2;    // primary 1 has fan fail/over temp alert
//*            pktReply.abTail[3] = 7;    // all primaries enabled
//*            break;
         case k_eCmbParamEvcPrimStatus:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 4);
            pktReply.abTail[0] = 6;    // line power to 1 & 2 (0 dead)
            pktReply.abTail[1] = 6;    // output status same (0 dead)
            pktReply.abTail[2] = 1;    // supply 0 failed
            pktReply.abTail[3] = 7;    // all enabled by EVC
            break;

         case k_eCmbParamEvcPrimVoltage:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 2);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

         case k_eCmbParamEvcAuxCurrents:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 10);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

         case k_eCmbParamEvcAuxTemperatures:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 10);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

         case k_eCmbParamEvcAuxVoltages:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 6);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

         case k_eCmbParamEvcAuxEnables:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 1);
            pktReply.abTail[0] = 3;    // both aux supplies enabled
            break;

         case k_eCmbParamEvcBattFuseDrops:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 4);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

         case k_eCmbParamEvcBattTemperatures:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 4);
//*            pktReply.abTail[  - values unknown, leave at zero
            break;

//*         case k_eCmbParamEvcFanSpeeds:
//*            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 8);
//*            pktReply.abTail[
//*            break;

         case k_eCmbParamEvcFanSpeeds:
            //  for now, make a NAK reply, as much to test the CMB DDM
            //  as out of laziness on our part
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 1);
            pktReply.Hdr.bStatus &= ~CmbStatAck;   // change ACK to NAK

            //  for a reason, claim we don't know this command
            pktReply.abTail[0] = k_eCmbNRCmdUnknown;
            break;

         case k_eCmbParamEvcKeyPosition:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 1);
            pktReply.abTail[0] = (U8) CT_KEYPOS_ON;   // key turned on (normal)
            break;

         case k_eCmbParamTemperature:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 2);
            pktReply.abTail[0] = 0;
            pktReply.abTail[1] = 35 * 2;     // 35 deg C, @ 0.5 deg C lsb
            break;

         case k_eCmbParamIopType:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 1);
            pktReply.abTail[0] = aIopSlot[iContigSlot].eType;
            break;

         case k_eCmbParamBoardHwBuildInfo:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 22);
            memset (pktReply.abTail, 0, 22);
            strcpy ((char *) pktReply.abTail, "12cdefg89"); // 9-dig part number
            pktReply.abTail[17] = 1;      // low byte of revision
            pktReply.abTail[20] = (1234 >> 8);  // byte[1] of build date
            pktReply.abTail[21] = 1234 & 0xFF;  // byte[0] of build date
            break;

         case k_eCmbParamCmaFirmwareInfo:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 2);
            pktReply.abTail[0] = 1;    // AVR firmware version
            pktReply.abTail[1] = 0;    // AVR firmware revision
            break;

         case k_eCmbParamIopMipsHwParams:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 6);
            memset (pktReply.abTail, 0, 6);
            //  who knows what these should be, so we leave them blank..
            break;

         case k_eCmbParamChassisSerNum:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 16);

            //  we use a common, totally bogus chassis serial number:
            strcpy ((char *) pktReply.abTail, "Chassis 01X1999");
            break;

         case k_eCmbParamBoardSerNum:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 16);

            //  our table includes board serial num, so just use it:
            strcpy ((char *) pktReply.abTail,
                    (char *) aIopSlot[iContigSlot].pszSerialNo);
            break;

         case k_eCmbParamBoardMfgName:
            MakeReplyFrame (iContigSlot, pktCommand, pktReply, 24);
            strcpy ((char *) pktReply.abTail, "ConvergeNet Tech., Inc.");
            //                        12345678901234567890123
            break;

         default:
            Tracef ("DoGetLastValue: unknown param type code (%d)\n",
                    pktCommand.abTail[0]);
            assert (FALSE);
         }
      }

   //  all done, report our result
   return (ulMyRet);

}  /* end of DoGetLastValue */

//
//  DoIopPowerCtl (pktCommand, pktReply)
//
//  Description:
//    Handles MIPS CPU "power on" and "power off" CMB commands.
//
//    Being as we're dummy code, we simply issue prompts to
//    effect the desired control operations.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoIopPowerCtl - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoIopPowerCtl (const CmbPacket& pktCommand,
                              CmbPacket& pktReply)
{

char     achPrompt [128];
U32      iContigSlot;
STATUS   ulMyRet;


   assert (pktCommand.Hdr.bCommand == k_eCmbCmdPowerControl);
   assert (pktCommand.Hdr.cbData == 1);
   assert ((pktCommand.abTail[0] == k_eCmbPowerOn) ||
           (pktCommand.abTail[0] == k_eCmbPowerOff));

   ulMyRet = ValidIopSlot ((TySlot) pktCommand.Hdr.bDestAddr, iContigSlot);

   if (ulMyRet == CTS_SUCCESS)
      {
      //  do the appropriate prompt, and update IOP state also
      if (pktCommand.abTail[0] == k_eCmbPowerOn)
         {
         MakeSlotMessage (achPrompt, "CMB: Please turn on power to",
                          pktCommand.Hdr.bDestAddr);
         aIopSlot[iContigSlot].eCmbState = k_eCmbMStPoweredOn;
         }
      else
         {
         MakeSlotMessage (achPrompt, "CMB: Please turn off power to",
                          pktCommand.Hdr.bDestAddr);
         aIopSlot[iContigSlot].eCmbState = k_eCmbMStPoweredOff;
         }

      IssuePrompt ("%s", achPrompt);

      //  all done, report happiness
      MakeReplyFrame (iContigSlot, pktCommand, pktReply);
      }

   return (ulMyRet);

}  /* end of DoIopPowerCtl */

//
//  DoSetPciWindow (pktCommand, pktReply)
//
//  Description:
//    Handles the "set PCI window" CMB command.
//
//    We simply display the requested PCI window parameters.
//    Note that we ensure that the message is targeted to the MIPS CPU,
//    and not to its CMA (this message must be sent pass-through).
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoSetPciWindow - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoSetPciWindow (const CmbPacket& pktCommand,
                               CmbPacket& pktReply)
{

U32      iContigSlot;
STATUS   ulMyRet;


   assert (pktCommand.Hdr.bCommand == k_eCmbCmdSetPciWindow);
   assert (pktCommand.Hdr.cbData == 3 * sizeof (U32));
   assert ((pktCommand.Hdr.bDestAddr & CmbAddrMips) == CmbAddrMips);

   //  window size must be greater than zero..
   assert ((*(U32 *) pktCommand.abTail) != 0);

   ulMyRet = ValidIopSlot ((TySlot) (pktCommand.Hdr.bDestAddr & ~CmbAddrMips),
                           iContigSlot);

   if (ulMyRet == CTS_SUCCESS)
      {
      Tracef ("CMB: setting IOP slot (%d) PCI window, size = 0x%08x * 64kB\n",
              pktCommand.Hdr.bDestAddr & CmbAddrMips,
              ntohl (*(U32 *) pktCommand.abTail));
      Tracef ("     PCI bus base = 0x%08X,  IOP base = 0x%08X\n",
              ntohl (*(U32 *) (pktCommand.abTail + 4)),
              ntohl (*(U32 *) (pktCommand.abTail + 8)));

      //  all done, report happiness
      MakeReplyFrame (iContigSlot, pktCommand, pktReply);
      }

   return (ulMyRet);

}  /* end of DoSetPciWindow */

//
//  DoSetBootParams (pktCommand, pktReply)
//
//  Description:
//    Handles the "set PCI window" CMB command.
//
//    We simply display the requested PCI window parameters.
//    Note that we ensure that the message is targeted to the MIPS CPU,
//    and not to its CMA (this message must be sent pass-through).
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoSetBootParams - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoSetBootParams (const CmbPacket& pktCommand,
                                CmbPacket& pktReply)
{

U32      iContigSlot;
STATUS   ulMyRet;


   assert (pktCommand.Hdr.bCommand == k_eCmbCmdSetBootParams);
   assert (pktCommand.Hdr.cbData == 2 * sizeof (U32) + 1);
   assert ((pktCommand.Hdr.bDestAddr & CmbAddrMips) == CmbAddrMips);

   ulMyRet = ValidIopSlot ((TySlot) (pktCommand.Hdr.bDestAddr & ~CmbAddrMips),
                           iContigSlot);

   if (ulMyRet == CTS_SUCCESS)
      {
      //  got IOP slot, now issue different message based on boot command:
      switch (pktCommand.abTail[0])
         {
         case k_eCmbBootNone:
            Tracef ("CMB: IOP (%d) boot type: none\n",
                    pktCommand.Hdr.bDestAddr & ~CmbAddrMips);
            break;
     
         case k_eCmbBootDiags:
            Tracef ("CMB: IOP (%d) boot type: Diags\n",
                    pktCommand.Hdr.bDestAddr & ~CmbAddrMips);
            break;
     
         case k_eCmbBootPCI:
            Tracef ("CMB: IOP (%d) boot type: PCI, image @ 0x%08X * 64kB, "
                    "params @ 0x%08X * 64kB\n",
                    pktCommand.Hdr.bDestAddr & ~CmbAddrMips,
                    ntohl (*(U32 *) (pktCommand.abTail + 1)),
                    ntohl (*(U32 *) (pktCommand.abTail + 5)));
            break;
   
         case k_eCmbBootCrashDump:
            Tracef ("CMB: IOP (%d) boot type: Crash Dump\n",
                    pktCommand.Hdr.bDestAddr & ~CmbAddrMips);
            break;

         default:
            Tracef ("CMB: IOP (%d) boot type unrecognized (%d)\n",
                    pktCommand.Hdr.bDestAddr & ~CmbAddrMips,
                    pktCommand.abTail[0]);
            assert (FALSE);
         }

      //  all done, report happiness
      MakeReplyFrame (iContigSlot, pktCommand, pktReply);
      }

   return (ulMyRet);

}  /* end of DoSetBootParams */

//
//  DoSendDdmMessage (pktCommand, pktReply)
//
//  Description:
//    Handles the "send small DDM message" CMB command.
//
//    Because we expect to only be used on a (single-CPU) eval crate,
//    we simply post the message as an unsolicited input to our own
//    CMB DDM.  Not perfect, but hopefully good enough for testing.
//
//    *  Note that this may be fraught with peril, depending on
//       how the messages are finally handled at the DDM level.
//       [Handshake, etc.]
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    DoSetBootParams - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoSendDdmMessage (const CmbPacket& pktCommand,
                                 CmbPacket& pktReply)
{

   assert (FALSE);

   return (CTS_NOT_IMPLEMENTED);

}  /* end DoSendDdmMessage */

//
//  DoSendLogEntry (pktCommand, pktReply, pActCook)
//
//  Description:
//    Handles "send event log entry [fragment]" CMB command / response.
//
//    "send event log entry" is set up to transfer an event log entry
//    from an IOP to the HBC, via the CMB.  There are thus two CMB DDMs
//    involved, on on the IOP and another on the HBC.  We emulate both
//    DDMs, so that testing may occur on a single-CPU eval crate.
//
//    When we receive a command packet, we assume we're running in
//    "IOP mode."  We translate it into an unsolicited notification
//    and post it back to the CMB DDM.  This step emulates the transfer
//    from IOP to HBC.  We also defer our standard immediate  callback
//    by "hijacking" the caller's action callback cookie.
//
//    When we receive a response packet, we assume we're running in
//    "HBC mode".  We locate the matching (IOP-side) command packet,
//    which we kept inside the action cookie hijacked in the preceding
//    paragraph, and post it back to continue the IOP caller's processing.
//    [Because we are processing a response packet, we have no callback
//     of our own to make.]
//
//    Note that this both-ends-in-one-box approach requires that the
//    CMB DDM be able to respond to both ends of the conversation.
//
//  Inputs:
//    pktCommand - Command packet to process.
//
//  Outputs:
//    pktReply - Loaded with suitable reply packet, when we return
//                CTS_SUCCESS.
//    pActCook - We set this action cookie to NULL when we receive a command
//                packet; we later restore the cookie during processing of
//                the response from the "other" board's CMB DDM.
//    DoSetBootParams - Returns CTS_SUCCESS, or some sort of status code
//                describing why we failed.
//

static STATUS  DoSendLogEntry (const CmbPacket& pktCommand,
                               CmbPacket& pktReply,
                               CCmbHwIntf::CSendContext *& pActCook)
{

CCmbHwIntf::CSendContext * pEnd;


   //  only defer callbacks when we're given the command-form packet;
   //  responses have no callback, ever.
   if ((pktCommand.Hdr.bStatus & CmbStatCmd) != 0)
      {
      //  got a command packet, turn it into a notification for the
      //  "other end" of the IOP -> HBC channel:
      PostNotify (pktCommand);

      //  and save caller's action cookie, deferring callback until
      //  we're called with a response to this command:

      //  (make sure we place cookie at *end* of queue)
      pActCook->pNext = NULL;

      if (pEventLogSendCallbackQueue == NULL)
         {
         pEventLogSendCallbackQueue = pActCook;
         }
      else
         {
         pEnd = pEventLogSendCallbackQueue;
         while (pEnd->pNext != NULL)
            {
            pEnd = pEnd->pNext;
            }

         pEnd->pNext = pActCook;
         }

      //  let caller know we're hijacking their action cookie
      pActCook = NULL;
      }
   else
      {
      //  response sender should not have a callback
      assert (pActCook->fnCallback == NULL);

      //  got a response packet.  This means that we dispose of the
      //  caller's action cookie, since they're sending a response
      //  which has no callback.
      delete pActCook;

      //  now, recover oldest command-side action cookie.  This way,
      //  we'll wake up the sender of the original command.
      assert (pEventLogSendCallbackQueue != NULL);

      pActCook = pEventLogSendCallbackQueue;
      pEventLogSendCallbackQueue = pActCook->pNext;

      //  save response packet in reply buffer, since this is what
      //  our caller expects
      pktReply = pktCommand;
      }

   return (CTS_SUCCESS);

}  /* end of DoSendLogEntry */

//
//  MakeReplyFrame (iContigSlot, pktCommand, pktReply, cbParam)
//
//  Description:
//    Given a CMB command packet, constructs an suitable CMB reply packet.
//    We specify the count of parameter bytes, but do not fill them in.
//    We do, however, zero the reply's parameter bytes array.
//
//  Inputs:
//    iContigSlot - Contiguous slot index of IOP which we're emulating.
//                This isn't always the DST/SRC value, so we provide it
//                explicitly.
//    pktCommand - Command packet to use as source of parameters for reply.
//
//  Outputs:
//    pktReply - Loaded with reply packet, whose header parameters are
//                derived from pktCommand.
//

static void  MakeReplyFrame (U32 iContigSlot, const CmbPacket& pktCommand,
                             CmbPacket& pktReply, U8 cbParam /* = 0 */ )
{

CmbPacket * pReply = &pktReply;     //BUGBUG - hack to view under debugger
const CmbPacket * pCmd   = &pktCommand;   //BUGBUG - ditto

#ifdef USE_FUNNY_HACK
U32         iNewBoardSlot;
#endif


   pktReply.Hdr.bDestAddr  = pktCommand.Hdr.bSrcAddr;
   pktReply.Hdr.bSrcAddr   = pktCommand.Hdr.bDestAddr;
   pktReply.Hdr.bCommand   = pktCommand.Hdr.bCommand;

#ifdef USE_FUNNY_HACK
   //  here we do a funny hack:  for IOP's in the powered-on state,
   //  we count the number of status replies (to any command).
   //  Once we've sent a certain number, we automagically change
   //  the IOP's state from powered-on to awaiting boot.  Thus,
   //  code which polls us will see the IOP appear to transition
   //  just as if it had a MIPS boot ROM doing stuff.
   if (aIopSlot[iContigSlot].eCmbState == k_eCmbMStPoweredOn)
      {
      //  we're in powered-on state, so keep a count of replies sent
      //  since entering this state:
      aIopSlot[iContigSlot].cRepliesBeforeBoot ++;

      if (aIopSlot[iContigSlot].cRepliesBeforeBoot > 3)
         {
         //  sent a reply to power-on message, and two other messages since.
         //  So it's time to shift IOP into "awaiting boot" state:
         aIopSlot[iContigSlot].eCmbState = k_eCmbMStAwaitingBoot;

         //  also, just for fun, let's send a couple of CMB slot status
         //  change notifications.  We do one for the state change we just
         //  fabricated, and one for a whole new phantom IOP (aren't they all?).
         PostSlotNotify (iContigSlot);

         //  find our "new" slot's contiguous form
         ValidIopSlot (k_eNewBoardSlot, iNewBoardSlot);

         //  shift its state from empty to "new"
         assert (aIopSlot[iNewBoardSlot].eCmbState == k_eCmbMStNotFound);

         aIopSlot[iNewBoardSlot].eCmbState   = k_eCmbMStPoweredOff;
         aIopSlot[iNewBoardSlot].eType       = IOPTY_RAC;
         aIopSlot[iNewBoardSlot].pszSerialNo = "00xyz";

         PostSlotNotify (iNewBoardSlot);

         //  and reset counter for next time this happens
         //[BUGBUG - assert() above will fire if we run twice]
         aIopSlot[iContigSlot].cRepliesBeforeBoot = 0;
         }
      
      }

   //  all done with IOP state jiggering, now build up status byte.
#endif  /* #ifdef USE_FUNNY_HACK */

   //  status contains the slot's "mips state" and the ACK flag
   //  ("response" is implied by absence of the cmb flag CmbStatCmd)
   pktReply.Hdr.bStatus    = aIopSlot[iContigSlot].eCmbState | CmbStatAck;

   pktReply.Hdr.cbData     = cbParam;
   pktReply.Hdr.bHeaderCRC = 0;

   //  actually, let's blast the packet's whole tail
   memset (pktReply.abTail, 0, sizeof (pktReply.abTail));

   return;

}  /* end of MakeReplyFrame */

//
//  PostSlotNotify (iContigSlot)
//
//  Description:
//    Called to generate an unsolicited slot status change notification.
//    We post the notification to our "unsolicited" message queue, and
//    then send a signal to our host CMB DDM.
//
//  Inputs:
//    iContigSlot - Contiguous index of slot whose status we are to
//                report in our slot status change notification.
//
//  Outputs:
//    none
//

static
void  PostSlotNotify (U32 iContigSlot)
{

CmbPacket         pktReply;


   //  build up slot status notification packet

   pktReply.Hdr.bDestAddr = CMB_NOTIFY;    // reply is a notification
   pktReply.Hdr.bSrcAddr  = CMB_SELF;      // comes from "own CMA"
   pktReply.Hdr.bCommand  = k_eCmbCmdCmbSlotStatus;

   //  status contains the slot's "mips state" and the ACK flag
   //  ("response" is implied by absence of the cmb flag CmbStatCmd)
   pktReply.Hdr.bStatus   = aIopSlot[iContigSlot].eCmbState | CmbStatAck;

   pktReply.Hdr.cbData    = 2;
   pktReply.abTail[0] = aeContigToSlot [iContigSlot];    // IOP which is updated
   pktReply.abTail[1] = aIopSlot [iContigSlot].eCmbState;   // IOP's state

   //  got notify packet built up, now submit it
   PostNotify (pktReply);

   return;

}  /* end of PostSlotNotify */

//
//  PostNotify (pktNotify)
//
//  Description:
//    Generates an unsolicited packet notification to the CMB DDM.
//    We also queue the supplied packet for later reading by the
//    CMB DDM via our dummy CCmbHwIntf::ReadMsg() implementation.
//
//    We post the notification to our "unsolicited" message queue, and
//    then send a signal to our host CMB DDM.
//
//  Inputs:
//    pktNotify - Packet to send as unsolicited notification.
//
//  Outputs:
//    none
//

static
void  PostNotify (const CmbPacket& pktNotify)
{

UnsolicitedPkt  * pCarrier;
UnsolicitedPkt  * pListTail;
STATUS            ulRet;


   //  got packet all ready, now stick it into queue
   pCarrier = new UnsolicitedPkt;
   pCarrier->pkt   = pktNotify;        // copy whole tail, needed or not
   pCarrier->pNext = NULL;

   if (pUnsolicitedPktList == NULL)
      {
      pUnsolicitedPktList = pCarrier;
      }
   else
      {
      //  find end of queue
      for (pListTail = pUnsolicitedPktList;
           pListTail->pNext != NULL;
           pListTail = pListTail->pNext)
         {}

      //  and stick our new carrier on it
      pListTail->pNext = pCarrier;
      }

   //  signal our host DDM that there's a new unsolicited packet
   ulRet = This->HostDdmPtr()->Signal (This->CmbMsgReadySigCode());

   assert (ulRet == CTS_SUCCESS);

   return;

}  /* end of PostNotify */

//
//  ValidIopSlot (eSlot, iContigSlot)
//
//  Description:
//    Called to verify that a supplied "TySlot" enum value is
//    actually a valid IOP slot, as we understand it.
//
//    When we recognize a valid slot, we also return its "contiguous
//    index", which is a value we use internally to index various
//    IOP slot-specific data structures.  We don't like to use the
//    raw TySlot enum because its values are rather sparse, and so
//    would consume a lot of extra memory.
//
//  Inputs:
//    eSlot - IOP slot value to validate.
//
//  Outputs:
//    iContigSlot - Returns zero-based "contiguous index" equivalent
//             of our input eSlot value.  Only valid when this routine
//             returns TRUE.
//    ValidIopSlot - Returns CTS_SUCCESS when all is cool, or some
//             meaningful error code (with assert()) otherwise.
//

STATUS  ValidIopSlot (TySlot eSlot, U32& iContigSlot)
{

U32            i;
const U32      ceContigToSlot  =  sizeof (aeContigToSlot) /
                                    sizeof (*aeContigToSlot);
const TySlot * pSlot;
STATUS         ulMyRet;


   //  a pessimistic default
   ulMyRet = CTS_CMB_INVALID_PARAMETER;

   if (eSlot == CMB_SELF)
      {
      //  for mapping purposes, use our "real" slot #
      eSlot = k_eMyHbcSlot;
      }

   //  scoot through our list of known Slot IDs, looking for a match
   pSlot = aeContigToSlot;

   for (i = 0;  i < ceContigToSlot;  i ++)
      {
      if (eSlot == *pSlot)
         {
         //  found the proper slot, return its "contiguous index"
         iContigSlot = i;
         ulMyRet = CTS_SUCCESS;
         break;
         }

      //  no match yet, move on to next slot
      pSlot ++;
      }

   //  We should never get a bogus TySlot value (not even CMB_SELF).
   //  Right now, we consider presence of the CmbAddrMips flag to be bad also;
   //  later that may change.
   assert (ulMyRet == CTS_SUCCESS);

   //  all done, report what happened
   return (ulMyRet);

}  /* end of ValidIopSlot */

//
//  MakeSlotMessage (pchMsgBuf, pszMsgPrefix, bDestSlot)
//
//  Description:
//    This is a little helper routine.  It builds a normal-format prompt
//    message using the supplied custom prefix text and slot ID.
//
//    The message text is written into the supplied buffer, which had
//    better be "big enough."  This highly lame code is only temporary,
//    so don't worry too much about the lack of error checking.  :-)
//
//  Inputs:
//    pchMsgBuf - Points to buffer to receive our generated message.
//    pszMsgPrefix - Points to custom text to place in start of buffer.
//    bDestSlot - Destination slot address, in CMB packet form (e.g.,
//                   bit 7 flags MIPS CPU, others are TySlot value.
//
//  Outputs:
//    *pchMsgBuf - Loaded with an ever-so-pretty message.
//

static
void  MakeSlotMessage (char *pchMsgBuf, const char *pszMsgPrefix,
                       U8 bDestSlot)
{

   sprintf (pchMsgBuf, "%s IOP slot (%u)", pszMsgPrefix, bDestSlot);

   return;

}  /* end of MakeSlotMessage */


