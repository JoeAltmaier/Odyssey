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
// File: CmbMsgSendWrapper.cpp
//
// Description:
//    Contains the implementation of a helper class, CCmbMsgSender, which
//    simplifies tracking of messages (requests) sent from the MIPS to
//    the CMB, and their replies.  For further info, please refer to our
//    header file (CmbMsgSendWrapper.h) or the function headers below.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbMsgSendWrapper.cpp $
// 
// 10    1/20/00 4:31p Eric_wedel
// Updated for revised hw intf callback form, and added some debug
// tracking, 
// 
// 9     12/22/99 6:28p Ewedel
// Oops, retry helper routine's return value was backwards.  Fixed now.
// 
// 8     11/24/99 5:20p Ewedel
// Moved timeout retry logic into DoTimeoutRetry(), added support for
// local CMA timeouts (via CmbHwIntf's new watchdog support).  Also added
// port-C packet dumping, presently compiled out, for BobC.
// 
// 6     9/03/99 5:21p Ewedel
// Added standard reflection of packet response status into status code
// passed to user's callback.
// 
// 5     8/04/99 1:12p Ewedel
// Added send-retry support when CMB timeout (on-bus only) is seen.
// 
// 4     7/15/99 4:15p Ewedel
// Updated for CmbPacket def changes.
// 
// 3     6/16/99 8:23p Ewedel
// Added support for defining valid packet source addresses (after get
// board info has been called, natch).
// 
// 2     6/14/99 11:58p Ewedel
// Cleaned up & brought closer to reality.
// 
// 1     6/03/99 7:19p Ewedel
// Initial revision.
//
/*************************************************************************/


#include  "CmbMsgSendWrapper.h"

#include  "DdmCmb.h"             // for some helper members
#include  <string.h>

#include  <assert.h>

#include  "CtEvent.h"      // message compiler-generated status codes

#include  "Odyssey_Trace.h"      // for Tracef()...


//  define the following symbol to enable packet-level debug output on port C
//  (This is for the hardware guys to use, mostly.)
#define  ENABLE_PORT_C_TRACE   0
//*#define  ENABLE_PORT_C_TRACE   1


#if  ENABLE_PORT_C_TRACE
//# include  "types.h"       // needed by tty.h
  typedef int  I32;
  extern "C" {
# include  "tty.h"         // what we use to touch port C
  }
#endif



//  right now, we permit up to two send retries (only on CMB timeout)
const U32   CCmbMsgSender::m_cSendRetriesMax  =  2;


//  what our standard packet source address is
TySlot      CCmbMsgSender::m_eMyPacketSrc  =  (TySlot) 0;


#if ENABLE_PORT_C_TRACE
//  helper for dumping packet data out onto TTY port C
void  DumpPacketToPortC (const char *pszLabel, const CmbPacket& Pkt);
#endif


//
//  CCmbMsgSender::CCmbMsgSender (pHostDdm, bCommand, bDest, cbData)
//
//  Description:
//    Our constructor.  Initializes our CMB-bound message packet,
//    and sundry other instance data.
//
//    Note that we initialize the packet's status to indicate that
//    it is a command packet, not a response.
//
//    *  An instance created using this constructor will attempt to
//       destroy itself after completion of the requestor's callback.
//       To avoid this behavior, call Initialize() on the instance.
//
//  Inputs:
//    pHostDdm - For CHAOS message-sending bookkeeping.
//    bCommand - CMB command code for our packet.
//    bDest - CMB destination code for our packet.
//    cbData - One-based count of data bytes in our packet's tail,
//             *not* counting the trailing CRC.
//
//  Outputs:
//    none
//

CCmbMsgSender::CCmbMsgSender (DdmOsServices *pHostDdm, U8 bCommand, U8 bDest,
                              U8 cbData /* = 0 */ ) :
                        DdmOsServices (pHostDdm),
                        m_pHostDdm (pHostDdm)
{

//BUGBUG - debug info
   CmbDebugInfo.cMsgSendWrappersExtant ++;
//BUGBUG - endit

   Initialize (bCommand, bDest, cbData);

   //  override Initialize()'s default behavior, because of the flavor
   //  of constructor we are:
   m_fKeepInstance = FALSE;

   return;

}  /* end of CCmbMsgSender::CCmbMsgSender */

//
//  CCmbMsgSender::Initialize (bCommand, bDest, cbData)
//
//  Description:
//    Initializes our instance.  Mostly, initializes our CMB-bound
//    message packet.
//
//    Note that we initialize the packet's status to indicate that
//    it is a command packet, not a response.
//
//    *  This routine marks the current instance as *not* auto-delete,
//       so that it will persist until expressly deleted.  Of course,
//       this is also useful for instances which are members of other
//       classes.  :-)
//
//  Inputs:
//    bCommand - CMB command code for our packet.
//    bDest - CMB destination code for our packet.
//    cbData - One-based count of data bytes in our packet's tail,
//             *not* counting the trailing CRC.
//
//  Outputs:
//    none
//

void  CCmbMsgSender::Initialize (U8 bCommand, U8 bDest, U8 cbData /* = 0 */ )
{

int   cbPacket;      // size we initialize, not caller's size


   assert (cbData <= CmbPacket::PayloadMaxSize());

   m_pPacket = &m_LittlePacket;

   cbPacket = sizeof (m_LittlePacket);

   //  ok, got a packet, now fill it in

   memset (m_pPacket, 0, cbPacket);

   m_pPacket->Hdr.bDestAddr = bDest;
   m_pPacket->Hdr.bSrcAddr  = m_eMyPacketSrc;    // standard source address
   m_pPacket->Hdr.bCommand  = bCommand;
   m_pPacket->Hdr.bStatus   = CmbStatCmd;  // default packet to "command" type
   m_pPacket->Hdr.cbData    = cbData;

   //  flag that instance should *not* self-destruct
   m_fKeepInstance = TRUE;

   return;

}  /* end of CCmbMsgSender::Initialize */

//
//  CCmbMsgSender::AddParam (ibParamOffset, bData)
//  CCmbMsgSender::AddParam (ibParamOffset, nData)
//
//  Description:
//    Adds a parameter value to our packet.
//
//    We check to verify that the given parameter size, placed at the
//    given offset, will not exceed the declared size of the packet.
//
//    Note that the specified offset is from start of packet data,
//    and *not* from the start of the packet header.
//
//    Per CMB convention, we store multi-byte integers in network
//    byte order (big-endian).
//
//  Inputs:
//    ibParamOffset - Zero-based byte offset, *from start of payload area*,
//                to start of first byte of parameter.  For the first parameter,
//                this value is zero, etc.
//    bData - Byte-sized parameter value to stick in packet.
//    nData - Integer-sized (four byte) parameter value to stick in packet.
//                Per CMB convention, we store multi-byte integers in network
//                byte order (big endian).  We perform the conversion here.
//
//  Outputs:
//    none
//

void  CCmbMsgSender::AddParam (U8 ibParamOffset, U8 bData)
{

   assert (m_pPacket != NULL);
   assert (ibParamOffset < m_pPacket->Hdr.cbData);

   if (ibParamOffset < m_pPacket->Hdr.cbData)
      {
      m_pPacket->abTail[ibParamOffset] = bData;
      }

   return;

}  /* end of CCmbMsgSender::AddParam (U8 ibParamOffset, U8 bData) */


void  CCmbMsgSender::AddParam (U8 ibParamOffset, U32 nData)
{

   assert (m_pPacket != NULL);
   assert ((m_pPacket->Hdr.cbData >= sizeof (nData)) &&
           (ibParamOffset <= (m_pPacket->Hdr.cbData - sizeof (nData))));

   if ((m_pPacket->Hdr.cbData >= sizeof (nData)) &&
       (ibParamOffset <= (m_pPacket->Hdr.cbData - sizeof (nData))))
      {
      //  store integer in big-endian form, in a CPU-independent way
      m_pPacket->abTail[ibParamOffset + 0] = (U8) (nData >> 24);
      m_pPacket->abTail[ibParamOffset + 1] = (U8) (nData >> 16);
      m_pPacket->abTail[ibParamOffset + 2] = (U8) (nData >>  8);
      m_pPacket->abTail[ibParamOffset + 3] = (U8)  nData;
      }

   return;

}  /* end of CCmbMsgSender::AddParam (U8 ibParamOffset, U32 nData) */

//
//  CCmbMsgSender::Send (CmbIntf, pvCookie, pReplyCallback)
//
//  Description:
//    Sends our packet content off to the CMB hardware interface.
//    The reply will come back via the supplied callback entry point.
//
//    ** We are careful to always return status via the our callback
//       parameter, even if we have to call it right in this routine.
//
//  Inputs:
//    CmbIntf - Instance of CMB hardware interface which we send packet to.
//                [There should be only one, but we have no a priori knowledge
//                 of its location.]
//    pvCookie - A (void *) of stuff which only our caller cares about.
//                This value is fed back to *pReplyCallback when it is called.
//    pReplyCallback - Points to routine which is to be called when the
//                packet's reply comes back, or when an error occurs.
//                This callback is *always* called, even if our transmit
//                attempt fails miserably right away (in which case, the
//                callback is called right away also).
//
//  Outputs:
//    none
//

void  CCmbMsgSender::Send (CCmbHwIntf& CmbIntf, void *pvCookie,
                           CmbCallback pReplyCallback)
{

STATUS   sRet;


   assert (pReplyCallback != NULL);
   assert (m_pPacket != NULL);
   assert ((m_pPacket->Hdr.bDestAddr == CMB_SELF) ||
           (m_pPacket->Hdr.bSrcAddr != 0));

   //  save caller's cookie inside our own
   m_pvCallersCookie = pvCookie;

   //  similarly, save caller's callback
   m_CallersCallback = pReplyCallback;

   //  also save caller's CMB hw intf, in case we need to re-try the send
   m_pCmbIntf = &CmbIntf;

   //  note that we haven't done any retries yet
   m_cSendRetries = 0;

#if ENABLE_PORT_C_TRACE
   DumpPacketToPortC ("Send CMB req: ", *m_pPacket);
#endif

   //  attempt the send..
   sRet = CmbIntf.SendMsg (*m_pPacket, this,
                           CMBCALLBACK(CCmbMsgSender, MyCmbCallback), this);

   if (sRet != CTS_SUCCESS)
      {
      //  whoops, trigger callback function right now (with no reply packet)

      //  to avoid undue stack depth (the caller might make a whole chain
      //  of failed Cmb sends), we use DdmServices::Action() to restore
      //  a relatively flat stack, and then make the immediate callback
      //  from there.

      //  (save error status for ImmediateCallback())
      m_sRetError = sRet;

      Action(ACTIONCALLBACK(CCmbMsgSender, ImmediateCallback));
      }
   else
      {
      //  our instance will exist until after MyCmbCallback has run
      }

   return;

}  /* end of CCmbMsgSender::Send (CmbIntf, pvCookie, pReplyCallback) */

//
//  CCmbMsgSender::MyCmbCallback (pvCookie, sRet, pReply)
//
//  Description:
//    Our internal callback routine.  This is where the CMB hardware
//    interface calls us to announce receipt (or otherwise) of a
//    CMB packet.
//
//    Note that we are called back regardless of whether our CMB
//    packet was sent.  Thus, this routine is always responsible
//    for calling the user's (our creator's) callback routine.
//
//  Inputs:
//    pvCookie - Per our usage (see Send()), a pointer to the CCmbMsgSender
//                instance whose send triggered this operation.
//    sRet - Final status of operation.
//    pReply - Points to CMB response packet, if any.  This is a read-only
//             value, and is NULL if the send operation failed.
//
//  Outputs:
//    none
//

void  CCmbMsgSender::MyCmbCallback (void *pvCookie, STATUS sRet,
                                    const CmbPacket *pReply)
{

CCmbMsgSender   * This;    // (left over from when we used to be static)
BOOL              fDoCallbackNow;


   assert (pvCookie != NULL);

   //  recover our veiled instance pointer
   This = (CCmbMsgSender *) pvCookie;

   //  "This" is left over from old implementation --- keep it honest:
   assert (This == this);

   //  assume we'll do the callback and expire, unless we find otherwise
   fDoCallbackNow = TRUE;


#if ENABLE_PORT_C_TRACE
   DumpPacketToPortC ("Got CMB resp: ", *pReply);
#endif

   if (This != NULL)
      {
      //  check to see if we got a CMB timeout..  if so, we'll retry the send
      if ((sRet == CTS_SUCCESS) && (pReply != NULL) &&
          ((pReply->Hdr.bStatus & CmbStatAck) == 0))
         {
         //  got a NAK packet, see if it's one we'll retry
         switch (pReply->abTail[0])
            {
            case k_eCmbNRCmaTimeout:
               //  we got a CMB bus timeout -- this might be due to bus
               //  noise or other such nonsense, so let's retry the send.
               fDoCallbackNow = ! This->DoTimeoutRetry ();
               break;

            case k_eCmbNRCmaPktPreempt:
               //  our packet got preempted by the CMA, which has sent
               //  us an unsolicited command packet, and is waiting
               //  exclusively for the response to that packet.
               //
               //  We simply retry the send.  Our interface code should
               //  know that an unsolicited command is outstanding, and
               //  leave our packet in the queue until after the desired
               //  response to the unsolicited command has been sent.
               //  attempt the send..

               //  Note that we have no retry limit here, unlike
               //  our timeout handler.
               sRet = This->m_pCmbIntf->SendMsg (*This->m_pPacket, this,
                                                 CMBCALLBACK(CCmbMsgSender,
                                                             MyCmbCallback),
                                                 This);

               if (sRet == CTS_SUCCESS)
                  {
                  //  send went ok, so defer caller's callback until after
                  //  we receive another response
                  fDoCallbackNow = FALSE;
                  }
               break;

            default:
               //  for other NAK types, we simply pass them through
               //  to the requestor, with no special retry logic.
               assert (fDoCallbackNow);
            }
         }
      else if (sRet == CTS_CMB_LOCAL_AVR_TIMEOUT)
         {
         //  got a local MIPS/AVR interface timeout.  Yikes!
         //  This should never happen, but if it does we takes its
         //  retries from the same pool used for CMB bus timeouts.
         fDoCallbackNow = ! This->DoTimeoutRetry ();
         }

      //  unless we decided otherwise, call caller's callback now
      if (fDoCallbackNow)
         {
         //  call caller's real callback.
         assert (This->m_CallersCallback != NULL);

         if (This->m_CallersCallback != NULL)
            {
            //  going to do the callback, first transmute packet's status
            if (sRet == CTS_SUCCESS)
               {
               //  see what packet's result is
               CDdmCMB::CmbReplyToStatus (pReply, sRet);
               }

            //  now call requestor's callback hook
            ((This->m_pHostDdm)->*(This->m_CallersCallback))
                      (This->m_pvCallersCookie, sRet, pReply);
            }

         //  all done with this wrapper instance, so dispose of it.
         if (! This->m_fKeepInstance)
            {
            delete This;
            }
         }
      }

   return;

}  /* end of CCmbMsgSender::MyCmbCallback */

//
//  CCmbMsgSender::ImmediateCallback (pvCookie)
//
//  Description:
//    This is another callback, used as a target of DdmServices::Action().
//    We use this routine iff a CMB Send() fails, to guarantee that
//    the requestor always gets a callback.
//
//    Rather than directly calling the caller's callback, we transfer
//    control here, where we have a fresh stack context, and then
//    call the caller's routine.  We do this in order to avoid stack
//    overruns.
//
//    *  Note that, in keeping with standard CCmbMsgSender usage, once
//       the caller's callback completes we delete our own instance,
//       unless the caller specified that we should persist.
//
//  Inputs:
//    pvCookie - Not used.
//
//  Outputs:
//    none
//

STATUS  CCmbMsgSender::ImmediateCallback (void *pvCookie)
{

#pragma unused(pvCookie)


   //  all we do is make this call, albeit in a fresh stack context:
   (m_pHostDdm->*m_CallersCallback) (m_pvCallersCookie,
                                     m_sRetError, NULL);

   if (! m_fKeepInstance)
      {
      //  our instance's lifetime is ended, so tidy it up
      delete this;
      }

   //  action callbacks always report happiness, in the CHAOS way
   return (CTS_SUCCESS);

}  /* end of CCmbMsgSender::ImmediateCallback */

//
//  CCmbMsgSender::DoTimeoutRetry ()
//
//  Description:
//    This routine is called to attempt a timeout-instigated send retry.
//
//    We first qualify the retry, to see if the current packet has used up
//    all of its retry attempts.  If it has more to go, we attempt the send
//    again.
//
//    We report whether or not we started another send of the packet.
//
//  Inputs:
//    none
//
//  Outputs:
//    CCmbMsgSender::DoTimeoutRetry - Returns TRUE if we initiated another
//                send of our packet, else FALSE.  We might return FALSE
//                due to the packet's retry limit being exceeded, or due to
//                errors reported by our underlying interface code.
//

BOOL  CCmbMsgSender::DoTimeoutRetry ()
{

STATUS   sRet;
BOOL     fWeDidResend;


   //  default to no resend done
   fWeDidResend = FALSE;

   //  see if packet has any more lives
   if (m_cSendRetries < m_cSendRetriesMax)
      {
      //  haven't retried the send too much, so let's retry now.

      Tracef ("CCmbMsgSender::DoTimeoutRetry: retrying CMB send...\n");

      //  attempt the send..
      sRet = m_pCmbIntf->SendMsg (*m_pPacket, this,
                                  CMBCALLBACK(CCmbMsgSender,
                                              MyCmbCallback),
                                  this);

      //  keep track of our retries..
      m_cSendRetries ++;

      if (sRet == CTS_SUCCESS)
         {
         //  send went ok, so defer caller's callback until after
         //  we receive another response
         fWeDidResend = TRUE;
         }
      }

   return (fWeDidResend);

}  /* end of CCmbMsgSender::DoTimeoutRetry */

//
//  DumpPacketToPortC (pszLabel, Pkt)
//
//  Description:
//    This is a debug helper for displaying a CMB packet (request or reply)
//    on TTY port C.
//
//    Right now, we just dump the packet in hex.
//
//  Inputs:
//    pszLabel - Points to label string to prepend to packet data.
//    Pkt - Packet to dump.
//
//  Outputs:
//    none
//

#if ENABLE_PORT_C_TRACE

static void  NibbleToPortC (U8 bNibble)
{
char  chNibble;

   chNibble = bNibble & 0xF;
   if (chNibble < 10)
      chNibble += '0';
   else
      chNibble += 'A';

   ttyC_out (chNibble);
}  /* end of NibbleToPortC */

inline void  ByteToPortC (U8 bData)
{
   NibbleToPortC (bData >> 4);
   NibbleToPortC (bData);
}  /* end of ByteToPortC */

static void  StringToPortC (const char *pszString)
{
   assert (pszString != NULL);

   while (*pszString != 0)
      {
      ttyC_out (*pszString);
      pszString ++;
      }
}  /* end of StringToPortC */

void  DumpPacketToPortC (const char *pszLabel, const CmbPacket& Pkt)
{

const U8  * pbPkt;
int         i;

static BOOL fPortCInited = FALSE;


   if (! fPortCInited)
      {
      ttyC_init (115200);     // customary baud rate these days..
      fPortCInited = TRUE;
      }

   //  dump caller's packet label first
   StringToPortC (pszLabel);

   //  dump packet's header data
   pbPkt = (const U8 *) &Pkt;
   for (i = 0;  i < sizeof (Pkt.Hdr);  i ++)
      {
      ByteToPortC (*pbPkt);
      ttyC_out (' ');

      pbPkt ++;
      }

   //  dump packet's payload, if any
   if (Pkt.Hdr.cbData > 0)
      {
      ttyC_out ('[');
      for (i = 0;  i < Pkt.Hdr.cbData; i ++)
         {
         ByteToPortC (Pkt.abTail[i]);
         ttyC_out (' ');
         }
      ttyC_out (']');
      }

   //  all done, send an eol as well
   ttyC_out ('\r');
   ttyC_out ('\n');

   return;

}  /* end of DumpPacketToPortC */

#endif  // #if ENABLE_PORT_C_TRACE

