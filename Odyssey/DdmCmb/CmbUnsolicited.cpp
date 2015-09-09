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
// File: CmbUnsolicited.cpp
//
// Description:
//    CMB DDM member routines for handling unsolicited CMB input packets.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbUnsolicited.cpp $
// 
// 13    1/20/00 4:22p Eric_wedel
// Fixed callback target signatures.
// 
// 12    12/17/99 7:58p Ewedel
// Enabled "attention needed" event processing.
// 
// 11    12/14/99 8:14p Ewedel
// Fixed bug in targeting of "get attn status" query.  Also disabled bulk
// of status processing, pending resolution of difficulty with AVR
// firmware.
// 
// 10    12/13/99 2:03p Ewedel
// Added support for EVC & DDH "attention needed" pseudo-states.
// 
// 9     10/18/99 4:30p Ewedel
// Changed DiscardReply() to DiscardOkReply().
// 
// 7     9/03/99 5:22p Ewedel
// Updated for IOP Status record changes.
// 
// 6     8/13/99 9:46a Ewedel
// Changed form of small message's Send() to properly dispose of reply.
// 
// 5     8/12/99 7:46p Ewedel
// Made k_eCmbCmdDdmMessage request message handling remotely real.  Still
// need to iron out kinks in CMB reply though.
// 
// 4     8/11/99 7:50p Ewedel
// Prepped for handling incoming messages (inter-DDM and event logging).
// 
// 3     7/20/99 2:11p Rkondapalli
// [ewx]  Fixed HandleSlotStatusUpdate() to retrieve state value from
// proper slot in tail of CmbPacket.
// 
// 2     7/16/99 10:12a Ewedel
// Added gate flags to suppress unsolicited packet handling during early
// startup.  Also added some temp debug output.
// 
// 1     7/15/99 4:26p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "EnvDdmMsgs.h"      // stuff for sending notification to env ddm

#include  <assert.h>

#include  "Odyssey_Trace.h"



//
//  CDdmCMB::CmbMsgReceived (nSignal, pPayload)
//
//  Description:
//    Handler for our eSigAtmelMsgWaiting CHAOS signal code.
//
//    This signal is sent to us by the Atmel hardware interface object
//    when a complete CMB message has been received and is ready to read.
//
//    We read said CMB message and process it.
//
//    Note that we are normally only called for "unsolicited" CMB traffic
//    since replies to our own requests are handled via callbacks specified
//    during request submission (see CCmbHwIntf::SendMsg()).
//
//  Inputs:
//    nSignal - Signal code for which we're invoked.  Should always be
//                eAtmelMsgWaiting.
//    pPayload - Not used by this handler.
//
//  Outputs:
//    CDdmCMB::CmbMsgReceived - Always returns CTS_SUCCESS -- other values
//                are reserved for future use by CHAOS.
//

STATUS CDdmCMB::CmbMsgReceived (SIGNALCODE nSignal, void *pPayload)
{

#pragma unused(pPayload)

CmbPacket   pkt;
STATUS      ulRet;


   assert (nSignal == eSigAtmelMsgWaiting);

   //  grab message, and see what to do about it
   ulRet = m_CmbHwIntf.ReadMsg (pkt, sizeof (pkt));
   if (ulRet != CTS_SUCCESS)
      {
      //  tsk, a spurious signal I guess
      assert (ulRet == CTS_CMB_NO_UNSOLICITED_MSG);
      return (CTS_SUCCESS);
      }

   //  got a message, but are we permitted to handle it yet?
   if (! m_fAcceptUnsolicitedCmbInput)
      {
      //  nope, too early in our bringup.  Ignore this message.
      return (CTS_SUCCESS);
      }

   //  got a message, do stuff with it
   if ((pkt.Hdr.bCommand == k_eCmbCmdCmbSlotStatus) &&
       ((pkt.Hdr.bStatus & CmbStatCmd) == 0))
      {
      //  got a slot status update, handle it
      HandleSlotStatusUpdate (pkt);
      }
   else if (((pkt.Hdr.bCommand == k_eCmbCmdDdmMessage) ||
             (pkt.Hdr.bCommand == k_eCmbCmdSendLogEntry)) &&
            ((pkt.Hdr.bStatus & CmbStatCmd) != 0))
      {
      //  got a DDM-level message of some sort, handle it too
      HandleDdmMessage (pkt);
      }
   else if ((pkt.Hdr.bStatus & CmbStatCmd) != 0)
      {
      //  got an unrecognized command, return a NAK
      SendCmbNak (pkt);
      }
   else
      {
      //  got an unrecognized response, just drop it on the floor
      assert (FALSE);
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::CmbMsgReceived */

//
//  CDdmCMB::HandleSlotStatusUpdate (pkt)
//
//  Description:
//    This routine is called to take care of a CMB Slot Status message
//    (response, actually) sent to us from our local CMA.  Only the HBC
//    flavor of DDM receives this message.
//
//    The master HBC's CMA sends us a slot status notification when
//    the state of some other CMA has changed since it was last polled.
//    Normally we simply propagate this state information into the
//    IOP Status table row for the affected CMB slot.  However, the EVC
//    and DDH CMAs may declare themselves in a special "attention needed"
///   state.
//
//    Although it is passed through the standard status polling logic, this
//    "attention needed" is not a real state, but instead is a pseudo-state
//    used to indicate that the reporting CMA detected some event of interest
//    to the system.  As such, this state value is not propagated to the
//    IOP Status table.  Instead, when it is detected we initiate special
//    state query processing which, among other things, will result in the
//    CMA reverting its state to a "normal" value.
//
//    If a regular (not to say "normal") IOP state is reported, we convert
//    the reported IOP state into an IOP Status table state, and update the
//    appropriate IOP Status table row.
//
//    If a new IOP has been inserted, we also update the various other
//    fields like manufacturer name, IOP type, etc.
//
//  Inputs:
//    pkt - Packet containing slot status message.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleSlotStatusUpdate (const CmbPacket& pkt)
{

IopState                eNewState;
TySlot                  eSlot;
U32                     iContigSlot;   // contiguous index of updated IOP's slot
PollContext           * pPollCtx;
MsgCmbUpdateIopStatus * pmsgUpdateIop;


   assert (pkt.Hdr.bCommand == k_eCmbCmdCmbSlotStatus);
   assert ((pkt.Hdr.bStatus & CmbStatCmd) == 0);    // packet is response
   assert ((pkt.Hdr.bStatus & CmbStatAck) != 0);    // packet is ack, not nak


   //  if we get an attention state, go off and handle it pronto:
   if (pkt.abTail[1] == k_eCmbMStAttentionNeeded)
      {
      HandleAttentionState (pkt);
      return;
      }

   //  got a regular state change notification.
   //  Have we begun our first slot poll yet?
   if (! m_fSlotPollBegun)
      {
      //  nope, so ignore this slot status update.  We don't want to
      //  disturb the current state of PTS until Boot Manager or
      //  whoever tells us that they've finished reading it.

      Tracef ("CDdmCMB::HandleSlotStatusUpdate: ignoring status update pkt\n");

      return;
      }

   //  fish out CMB state code
   CmbStateToIopStatus (pkt.abTail[1], eNewState);

   //  we should never get a status update with state "unknown"
   assert (eNewState != IOPS_UNKNOWN);

   //  also grab slot of updated IOP / other FRU (might be DDH or EVC)

   eSlot = (TySlot) pkt.abTail[0];
   if (! ValidIopSlot (eSlot, iContigSlot))
      {
      //  oh great, CMB fed us an invalid slot #.  Log this & otherwise
      //  ignore it.
      assert (FALSE);
      //BUGBUG - log bad CMB-supplied slot ID here..

      //  ignore this packet.
      return;
      }

   Tracef ("CDdmCMB::HandleSlotStatusUpdate: slot status update, slot = %d\n",
           eSlot);
   Tracef ("                                 old state = %d, new state = %d\n",
           m_aIopStatusImage[iContigSlot].eState,  eNewState);


   //  is board present, but did it used to not be?  If so, board is newly
   //  inserted, and we need to pick up all of its various params:
   if ((eNewState != IOPS_EMPTY) && (eNewState != IOPS_BLANK))
      {
      //  new state says there's a board there..  did there used to not be?
      if ((m_aIopStatusImage[iContigSlot].eState == IOPS_EMPTY) ||
          (m_aIopStatusImage[iContigSlot].eState == IOPS_BLANK))
         {
         //  yup, got a newly-inserted something-or-other.
         //  Acquire new board's params here, using our standard
         //  single-IOP poll logic:
         pPollCtx = new PollContext;

         //  [since we were signalled, we have no context to save]
         pPollCtx->pReq = NULL;

         //  allocate the request message we'll use for all IOPs
         pmsgUpdateIop = new MsgCmbUpdateIopStatus;

         //  set our target slot #
         pmsgUpdateIop->eTargetSlot = eSlot;

         //  let continuation routine do the rest
         Send (pmsgUpdateIop, pPollCtx,
               REPLYCALLBACK (CDdmCMB, HandleSlotStatusUpdate2));
         }
      else
         {
         //  nope, just a state change on an existing board.  Record it
         //  and we're done.
         UpdatePtsIopState (iContigSlot, eNewState);
         }
      }
   else
      {
      //  new state says there's no board there, let's clear out its
      //  old record in the IOP Status table

      //  make an empty record
      SlotContext     * pSlotCtx;
      IOPStatusRecord * pIopRow;

      pIopRow = new IOPStatusRecord;
      pIopRow->Slot = eSlot;
      pIopRow->eIOPCurrentState = eNewState;

      //  make up funny context carrier which row updater wants
      pSlotCtx = new SlotContext (iContigSlot, pIopRow);

      //  change slot's IOP Status table entry to reflect non-existence
      UpdatePtsIopRow (pSlotCtx);
      }

   return;

}  /* end of CDdmCMB::HandleSlotStatusUpdate */

//
//  CDdmCMB::HandleSlotStatusUpdate2 (pReply)
//
//  Description:
//    This routine is called to take care of a CMB Slot Status message
//    (response, actually) sent to us from our local CMA.  Only the HBC
//    flavor of DDM receives this message.
//
//    We convert the reported IOP state into an IOP Status table state,
//    and update the IOP status table.
//
//    If a new IOP has been inserted, we also update the various other
//    fields like manufacturer name, IOP type, etc.
//
//  Inputs:
//    pkt - Packet containing slot status message.
//
//  Outputs:
//    CDdmCMB::HandleSlotStatusUpdate2 - Returns CTS_SUCCESS, always.
//

STATUS  CDdmCMB::HandleSlotStatusUpdate2 (Message *pmsgReply)
{

MsgCmbUpdateIopStatus * pReply  =  (MsgCmbUpdateIopStatus *) pmsgReply;
PollContext           * pPollCtx;

   //  unveil our update context
   pPollCtx = (PollContext *) pReply->GetContext();
   assert (pPollCtx != NULL);

   //  dispose of memory, & we're done
   delete pPollCtx;
   delete pReply;

   //  only safe reply, thanks CHAOS..
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::HandleSlotStatusUpdate2 */

//
//  CDdmCMB::HandleAttentionState (pkt)
//
//  Description:
//    This routine is called when we receive a CMB Slot Status reply
//    indicating that some CMA has declared itself in the "attention
//    needed" state.
//
//    We query the CMA to discover its attention condition(s), and then
//    perform the appropriate notifications / updates within DDM-land.
//
//    Note that at present we don't provide automated responses for
//    any detected conditions.  That is the responsibility of whoever
//    listens for the notifications which we send out.
//
//  Inputs:
//    pkt - Packet containing "attention needed" slot status message.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionState (const CmbPacket& pkt)
{


   assert (pkt.Hdr.bCommand == k_eCmbCmdCmbSlotStatus);
   assert ((pkt.Hdr.bStatus & CmbStatCmd) == 0);    // packet is response
   assert ((pkt.Hdr.bStatus & CmbStatAck) != 0);    // packet is ack, not nak
   assert (pkt.abTail[1] == k_eCmbMStAttentionNeeded);   // pkt is "attn needed"

   //  first off, query CMA for its attention bits
   //  [we have no context yet, beyond the identity of the squittering CMA]

   //  note that we must use the address contained in the reply's payload,
   //  and not the reply's own source address, which is always "hbc cma".
   SendCmbMsg (NULL, k_eCmbCmdGetAttnStatus, pkt.abTail[0],
               CMBCALLBACK(CDdmCMB, HandleAttentionState2));

   return;

}  /* end of CDdmCMB::HandleAttentionState */

//
//  CDdmCMB::HandleAttentionState2 (pvCookie, sStatus, pReply)
//
//  Description:
//    We're called back with the results of a "get attention status flags"
//    command sent to some CMA which had entered the "attention needed"
//    state.
//
//    We take a look at the flags, and the type of device, and take it
//    from there.
//
//  Inputs:
//    pvCookie - Not used here.
//    sStatus - Result of requested operation.
//    pReply - Reply from CMB machinery.  Only guaranteed valid when
//             sStatus == CTS_SUCCESS.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionState2 (void * /* pvCookie */, STATUS sStatus,
                                      const CmbPacket *pReply)
{


   if ((sStatus != CTS_SUCCESS) || (pReply == NULL))
      {
      //  whoops, naught to do
      return;
      }

   assert ((pReply->Hdr.bCommand == k_eCmbCmdGetAttnStatus) &&
           ((pReply->Hdr.bStatus & CmbStatAck) != 0) &&
           (pReply->Hdr.cbData > 0));

   if ((pReply->Hdr.bCommand != k_eCmbCmdGetAttnStatus) ||
       ((pReply->Hdr.bStatus & CmbStatAck) == 0) ||
       (pReply->Hdr.cbData == 0))
      {
      //  some weird illegal condition; our assert above flagged it
      return;
      }

   //  got status bits, process them according to responding CMA type:
   switch (pReply->Hdr.bSrcAddr)
      {
      case CMB_EVC0:
      case CMB_EVC1:
         HandleAttentionEvc (*pReply);
         break;

      case CMB_DDH0:
      case CMB_DDH1:
      case CMB_DDH2:
      case CMB_DDH3:
         HandleAttentionDdh (*pReply);
         break;

      default:
         Tracef ("CDdmCMB::HandleAttentionState2: \"attention needed\" "
                 "from unexpected CMA,\n"
                 "                                CMA slot = %d\n",
                 pReply->Hdr.bSrcAddr);
         assert (FALSE);
      }

   //  all done
   return;

}  /* end of CDdmCMB::HandleAttentionState2 */

//
//  CDdmCMB::HandleAttentionEvc (pktReply)
//
//  Description:
//    We're called to handle a "get attention status flags" response
//    which originated in an EVC-type CMA.
//
//    We do whatever sort of notification is called for.
//
//  Inputs:
//    pktReply - "Get attention status flags" response from an EVC.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionEvc (const CmbPacket& pktReply)
{


   assert (pktReply.Hdr.cbData > 0);

   //  (right now, we only use one byte of flags; if there are more,
   //   we ignore them)

   if ((pktReply.abTail[0] & CmbAttnFlagEvcKey) != 0)
      {
      //  got a keyswitch position change, read new position
      SendCmbMsg (NULL, k_eCmbCmdGetLastValue, pktReply.Hdr.bSrcAddr,
                  CMBCALLBACK(CDdmCMB, HandleAttentionEvcKey),
                  1, k_eCmbParamEvcKeyPosition);

      //  (will continue after read reply comes back; check other bits now)
      }

   if ((pktReply.abTail[0] & CmbAttnFlagEvcBattery) != 0)
      {
      //  got a battery current change, read & update raw params
      SendCmbMsg (NULL, k_eCmbCmdGetLastValue, pktReply.Hdr.bSrcAddr,
                  CMBCALLBACK(CDdmCMB, HandleAttentionEvcBattery),
                  1, k_eCmbParamEvcBattFuseDrops);

      //  (will continue after read reply comes back; check other bits now)
      }

   //  we should be done with all possible status bits now..
   assert ((pktReply.abTail[0] & ~(CmbAttnFlagEvcKey | CmbAttnFlagEvcBattery))
                  == 0);

   return;

}  /* end of CDdmCMB::HandleAttentionEvc */

//
//  CDdmCMB::HandleAttentionEvcKey (pvCookie, sStatus, pReply)
//
//  Description:
//    We're called back with an EVC's response to a "get key position"
//    query.  We use this response to construct & send a key-change
//    notification message.
//
//  Inputs:
//    pvCookie - Not used here.
//    sStatus - Result of requested operation.
//    pReply - Reply from CMB machinery.  Only guaranteed valid when
//             sStatus == CTS_SUCCESS.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionEvcKey (void * /* pvCookie */, STATUS sStatus,
                                    const CmbPacket *pReply)
{

MsgEnvKeyswitch * pmsgKeyPos;
U32               iEvc;


   if ((sStatus != CTS_SUCCESS) || (pReply == NULL))
      {
      assert (FALSE);
      return;
      }

   //  got revised keyswitch position info; save it in our raw params cache.
   //  This way, we'll return consistent answers between our notification
   //  message and any outstanding environmental poll request(s).
   SetKeyPos ((TySlot) pReply->Hdr.bSrcAddr,
              (CT_EVC_KEYPOS) (pReply->abTail[0] & 3));

   //  build keyswitch position change notification, and send it.

   iEvc = pReply->Hdr.bSrcAddr - CMB_EVC0;
   pmsgKeyPos = new MsgEnvKeyswitch (m_EvcEnvCache[iEvc].eKeyPosition, iEvc);

   //  we don't care about what happens to the message, so we use
   //  DiscardOkReply() to take care of the callback & reply delete
   Send (pmsgKeyPos, NULL, REPLYCALLBACK (DdmServices, DiscardOkReply));

   return;

}  /* end of CDdmCMB::HandleAttentionEvcKey */

//
//  CDdmCMB::HandleAttentionEvcBattery (pvCookie, sStatus, pReply)
//
//  Description:
//    We're called back with an EVC's response to a "get battery fuse drops"
//    query.  We use this response to construct & send a battery voltage
//    change notification message.
//
//  Inputs:
//    pvCookie - Not used here.
//    sStatus - Result of requested operation.
//    pReply - Reply from CMB machinery.  Only guaranteed valid when
//             sStatus == CTS_SUCCESS.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionEvcBattery (void * /* pvCookie */, STATUS sStatus,
                                          const CmbPacket *pReply)
{

MsgEnvPowerEvent   * pmsgBattery;
U32                  iEvc;


   if ((sStatus != CTS_SUCCESS) || (pReply == NULL))
      {
      assert (FALSE);
      return;
      }

   //  got revised battery fuse voltage drop info; save it in our
   //  raw params cache.
   //  This way, we'll return consistent answers between our notification
   //  message and any outstanding environmental poll request(s).
   SetFuseDrops ((TySlot) pReply->Hdr.bSrcAddr, *pReply);

   //  build battery current direction change notification, and send it

   iEvc = pReply->Hdr.bSrcAddr - CMB_EVC0;

   pmsgBattery = new MsgEnvPowerEvent (m_EvcEnvCache[iEvc].ausBattFuseDrops[0],
                                       m_EvcEnvCache[iEvc].ausBattFuseDrops[1],
                                       iEvc);

   //  we don't care about what happens to the message, so we use
   //  DiscardOkReply() to take care of the callback & reply delete
   Send (pmsgBattery, NULL, REPLYCALLBACK (DdmServices, DiscardOkReply));

   return;

}  /* end of CDdmCMB::HandleAttentionEvcBattery */

//
//  CDdmCMB::HandleAttentionDdh (pktReply)
//
//  Description:
//    We're called to handle a "get attention status flags" response
//    which originated in a DDH-type CMA.
//
//    We do whatever sort of notification is called for.
//
//  Inputs:
//    pktReply - "Get attention status flags" response from an DDH.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionDdh (const CmbPacket& pktReply)
{


   assert (pktReply.Hdr.cbData > 0);

   //  (right now, we only use one byte of flags; if there are more,
   //   we ignore them)

   if ((pktReply.abTail[0] & CmbAttnFlagDdhBay) != 0)
      {
      //  got a drive insertion or removal, take a look:
      SendCmbMsg (NULL, k_eCmbCmdGetDdhStatus, pktReply.Hdr.bSrcAddr,
                  CMBCALLBACK(CDdmCMB, HandleAttentionDdh2));
      }

   //  we should be all done with status bits now
   assert ((pktReply.abTail[0] & ~k_eCmbCmdGetDdhStatus) == 0);

   return;

}  /* end of CDdmCMB::HandleAttentionDdh */

//
//  CDdmCMB::HandleAttentionDdh2 (pvCookie, sStatus, pReply)
//
//  Description:
//    We're called to handle a "get DDH status" reply from a DDH
//    which had issued an "attention needed" state with the
//    CmbAttnFlagDdhBay flag set.
//
//    We update the ??? PTS table to reflect whatever changes we see
//    in drives inserted / removed.
//
//  Inputs:
//    pktReply - "Get DDH status" response from an DDH.
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleAttentionDdh2 (void * /* pvCookie */, STATUS sStatus,
                                    const CmbPacket *pReply)
{


   if ((sStatus != CTS_SUCCESS) || (pReply == NULL))
      {
      assert (FALSE);
      return;
      }

   //BUGBUG - got DDH status - now what do we do with it?

   return;

}  /* end of CDdmCMB::HandleAttentionDdh2 */

//
//  CDdmCMB::HandleDdmMessage (pkt)
//
//  Description:
//    This routine is called to take care of a DDM message, veiled inside
//    a CMB packet(s).
//
//    Note that the various messages all come in inside CMB command packets.
//    To keep the CMB happy, we send acks of these packets back to the CMA
//    as soon as we receive them.  We do not report a meaningful result
//    in the ack, but merely inticate that we received the given packet.
//    
//  Inputs:
//    pkt - Packet containing DDM message [fragment].
//
//  Outputs:
//    none
//

void  CDdmCMB::HandleDdmMessage (const CmbPacket& pkt)
{

REQUESTCODE    eRequest;      // request code for embedded CHAOS message
Message      * pMsg;
CmbPacket      pktReply;
STATUS         sRet;


   assert ((pkt.Hdr.bCommand == k_eCmbCmdDdmMessage) ||
           (pkt.Hdr.bCommand == k_eCmbCmdSendLogEntry));

   assert ((pkt.Hdr.bStatus & CmbStatCmd) != 0);


   switch (pkt.Hdr.bCommand)
      {
      case k_eCmbCmdDdmMessage:
         //  got a simple "small" message to relay to somebody
         //  on our current IOP

         assert (pkt.Hdr.cbData >= sizeof (U32));

         if (pkt.Hdr.cbData >= sizeof (U32))
            {
            //  extract mandatory request code
            eRequest = (REQUESTCODE) ntohl (*(U32 *) pkt.abTail);

            //  build requested message
            pMsg = new Message (eRequest);

            if (pkt.Hdr.cbData > sizeof (U32))
               {
               //  got a payload, add it on
               pMsg->AddPayload((void *) (pkt.abTail + sizeof (U32)),
                                pkt.Hdr.cbData - sizeof (U32));
               }

            //  message all ready, now send it
            //  (DiscardOkReply does just that, since we don't report
            //   the results of the message back to the requestor.
            //   Far as I know, it just burps up an error message
            //   if a non-ok reply is received.)
            sRet = Send (pMsg, REPLYCALLBACK(CDdmCMB, DiscardOkReply));

            //  if we do ACK command packets at this level, then we can
            //  return a response packet whose payload contains the
            //  STATUS code sRet returned by Send().  A little more info
            //  for the requestor (HBC, presumably).
            // (continue here)
            }
         break;

      case k_eCmbCmdSendLogEntry:
         //  should only receive log entry fragments on the HBC
         assert (m_WeAreMasterHbc);

         //  received an event log fragment, submit it to our reassembler
         m_EventAssembler.ProcessFragment (pkt);
         break;

//      default:
         //  tsk, some unsupported message
      }

   //  all done with unsolicited command packet, now let's send a reply.
   pktReply.MakeReply (pkt);     // makes an ACK reply..

   m_CmbHwIntf.SendReplyToUnsolicitedCmd(pktReply);

   return;

}  /* end of CDdmCMB::HandleDdmMessage */

//
//  CDdmCMB::SendCmbNak (pkt)
//
//  Description:
//    This routine is called to send a NAK reply to a CMB packet.
//
//    ***  We must be careful to force our reply to the head of the
//         outbound CMB packet queue.  This is because the whole CMB
//         bus will stall until we send some sort of reply to the
//         received packet.
//
//    This routine is presently TBD.
//
//  Inputs:
//    pkt - Packet containing message which we're supposed to NAK.
//
//  Outputs:
//    none
//

void  CDdmCMB::SendCmbNak (const CmbPacket& pkt)
{

#pragma unused(pkt)


   assert (FALSE);

   return;

}  /* end of CDdmCMB::SendCmbNak */


