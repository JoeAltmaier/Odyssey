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
// File: ReqOther.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    incoming DDM request messages.
//
// $Log: /Gemini/Odyssey/DdmCmb/ReqOther.cpp $
// 
// 11    1/20/00 4:12p Eric_wedel
// Fixed callback target signatures.
// 
// 10    12/13/99 2:34p Ewedel
// Removed obsolete Env Reporter wakeup call (thanks, Vuong!).  [VN]
// 
// 9     11/19/99 6:36p Ewedel
// Moved ReqEvcControl() et al off into their own file.
// 
// 8     10/27/99 12:19p Ewedel
// Substantially revised ReqEvcControl() in making it real.  Moved
// ReqIopControl() off into new file ReqIopControl.cpp.
// 
// 7     10/18/99 4:31p Ewedel
// Enabled RqDdmReporter() message send to announce our readiness for PHS
// stuff.
// 
// 6     10/08/99 12:04p Ewedel
// Added CDdmCMB::ReqSetMipsState() et al, to handle new message
// MsgCmbSetMipsState.
// 
// 5     9/07/99 5:00p Jlane
// [ewx] Disabled PHS Reporter registration, since real-world EVCs don't
// support some (all?) of the queried parameters.
// 
// 4     9/03/99 5:33p Ewedel
// Modified to actually do requested EVC updates (was old skeletal code),
// and to support new separate env ctl table.
// 
// 3     8/24/99 8:09p Ewedel
// Removed now-obsolete CDdmCMB::ReqUpdateEvcStatus().  Fixed various jfl
// TS interface bugs.  Added standard ReportOsRunningToCma() completion
// routine.
// 
// 2     8/15/99 12:05p Jlane
// Added parameters for new TS interface changes.
// 
// 1     8/11/99 7:56p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CtEvent.h"         // standard status codes


#include  <assert.h>



//
//  CDdmCMB::ReqPollAllIops (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_POLL_ALL_IOPS request
//    message (class MsgCmbPollAllIops).
//
//    We update the current data for all rows in the IOP Status table.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbIopControl, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqPollAllIops - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqPollAllIops (Message *pReqMsg)
{

PollContext           * pPollCtx;
MsgCmbUpdateIopStatus * pmsgUpdateIop;


//*   Tracef("CDdmCMB::ReqPollAllIops()\n");

   if (pReqMsg->reqCode != CMB_POLL_ALL_IOPS)
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // must *always* return success
      }

   //  looks good, so note that we've begin slot polling.
   m_fSlotPollBegun = TRUE;      // enable slot status updates

   //  ok, allocate poll context, and let our standard continuation
   //  routine start the first poll
   pPollCtx = new PollContext;

   //  save away our original poll request msg
   pPollCtx->pReq = pReqMsg;

   //  flag that we've polled one IOP row (what we start below)
   pPollCtx->cRowsPolled = 1;

   //  allocate the request message we'll use for all IOPs
   pmsgUpdateIop = new MsgCmbUpdateIopStatus;

   //  and launch the first IOP update
   pmsgUpdateIop->eTargetSlot = m_aeContigToSlot [0];    // first IOP's slot

   //  let continuation routine do the rest
   Send (pmsgUpdateIop, pPollCtx, REPLYCALLBACK (CDdmCMB, ReqPollAllIops2));

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqPollAllIops */

//
//  Inputs:
//    pReply - Points to reply (from ourself) to most recent IOP update call.
//    pReply->
//    pClientContext - Points to PollContext instance tracking our
//                current polling sequence.
//    status - Result of last IOP polled.
//

STATUS CDdmCMB::ReqPollAllIops2 (Message *pReply)
{

PollContext           * pPollCtx;
MsgCmbUpdateIopStatus * pmsgUpdateIop;


//*   Tracef("CDdmCMB::ReqPollAllIops2(), reply status = %x\n",
//*          pReply->DetailedStatusCode);

   //  upcast our reply msg
   pmsgUpdateIop = (MsgCmbUpdateIopStatus *) pReply;

   //  unveil our polling context
   pPollCtx = (PollContext *) pReply->GetContext();

   //  the two should definitely line up
   assert (m_aeContigToSlot [pPollCtx->cRowsPolled - 1] ==
           pmsgUpdateIop->eTargetSlot);

   //  for now, we ignore status, and try to poll as many IOP rows as possible

   if (pPollCtx->cRowsPolled < CT_IOPST_MAX_IOP_SLOTS)
      {
      //  still got more to poll, do the next one
      pmsgUpdateIop->eTargetSlot = m_aeContigToSlot [pPollCtx->cRowsPolled];

      //  pre-increment row count, since it is also our "loop" counter
      pPollCtx->cRowsPolled ++;

      //  and go for next IOP row update
      Send (pmsgUpdateIop, pPollCtx, REPLYCALLBACK (CDdmCMB, ReqPollAllIops2));
      }
   else
      {
      //  All done with poll.  Let CMA know it is now safe to start sending
      //  unsolicited status updates to us, and report completion to
      //  poll requestor.

      assert ((pPollCtx->pReq != NULL) &&
              (pPollCtx->pReq->reqCode == CMB_POLL_ALL_IOPS));


      //  tell the CMA that it can send us unsolicited stuff
      //  (takes care of sending Reply() when it's done, for good or ill)
      ReportOsRunningToCma (pPollCtx->pReq);

      //  dispose of other things which we don't need any more
      delete pPollCtx;        // our polling context
      delete pmsgUpdateIop;   // the msg we used for probing individual IOPs
      }

   //  all done
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqPollAllIops2 */

//
//  CDdmCMB::ReqSendSmallMsg (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_SEND_SMALL_MSG request
//    message (class MsgCmbSendSmallMsg).
//
//    We send the supplied message along to the given target IOP.
//    Our response merely indicates whether we think the message
//    arrived.  It does *not* indicate whether the message was
//    successfully processed by the far end.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbSendSmallMsg, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqSendSmallMsg - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqSendSmallMsg (Message *pReqMsg)
{

MsgCmbSendSmallMsg    * pmsgSendSmallMsg;
MsgCmbSendSmallMsg::CPayload   * pPayload;
CCmbMsgSender         * pCmbMsg;


//*   Tracef("CDdmCMB::ReqPollAllIops()\n");

   if ((! MsgUpcast (pReqMsg, pmsgSendSmallMsg)) ||
       (! pmsgSendSmallMsg->GetPayload (pPayload)))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // must *always* return success
      }

   assert (pPayload->m_cbTargetPayload <= 20);  // bytes avail after req code

   //  got our request message, let's send it out onto the CMB:

   //  build up CMB command message
   pCmbMsg = new CCmbMsgSender (this, k_eCmbCmdDdmMessage,
                                pPayload->m_eTargetIop | CmbAddrMips,
                                sizeof (U32) + pPayload->m_cbTargetPayload);

   //  add target request code (how message will be routed at destination)
   pCmbMsg -> AddParam (0, pPayload->m_eTargetRequest);

   //  and if there's any requestor-supplied payload, send that along too
   if (pPayload->m_cbTargetPayload > 0)
      {
      memcpy (pCmbMsg->PacketPtr()->abTail + sizeof (U32),
              pPayload->m_abTargetPayload, pPayload->m_cbTargetPayload);
      }

   //  send command off to CMB (we simply use the request msg as our cookie):
   pCmbMsg -> Send (m_CmbHwIntf, pmsgSendSmallMsg,
                    CMBCALLBACK (CDdmCMB, ReqSendSmallMsg2));

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqSendSmallMsg */


//  We're called back when the CMB hw intf has a reply for our
//  k_eCmbCmdDdmMessage command.  Normally, this contains an ack
//  of our message submission to the remote MIPS CPU.
//
//  Inputs:
//    pvSendSmallMsg - Original request which triggered our processing.
//    sStatus - Indicates whether *submission* to CMB succeeded.  We
//                don't get a meaningful reply back from the destination,
//                at least at the low CMB level.
//    pReply - Points to reply from destination's MIPS CPU, acknowledging
//                receipt of our small message.
//

void  CDdmCMB::ReqSendSmallMsg2 (void *pvSendSmallMsg,
                                 STATUS sStatus, const CmbPacket *pReply)
{

MsgCmbSendSmallMsg * pmsgSendSmallMsg  =  (MsgCmbSendSmallMsg *) pvSendSmallMsg;


   assert ((sStatus != CTS_SUCCESS) || (pReply != NULL));

   if (sStatus == CTS_SUCCESS)
      {
      CmbReplyToStatus (pReply, sStatus);
      }

   //  simply report results of send to our requestor.
   //  !! Note that we only report the results of the CMB message send!!
   //     Of the message's final fate at the destination MIPS, we have
   //     no clue.
   Reply (pmsgSendSmallMsg, sStatus);

   return;

}  /* end of CDdmCMB::ReqSendSmallMsg2 */

//
//  CDdmCMB::ReqSetMipsState (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_SET_MIPS_STATE request
//    message (class MsgCmbSetMipsState).
//
//    We send the equivalent state value to our local CMA (AVR)
//    to advertise as our current MIPS State.  If we are the current
//    HBC Master, we also directly update our row in the IOP status
//    table, since that doesn't seem to happen properly in response
//    to our CMA update.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbSetMipsState, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqSetMipsState - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqSetMipsState (Message *pReqMsg)
{

MsgCmbSetMipsState    * pmsgSetMipsState;
CCmbMsgSender         * pCmbMsg;
CmbMipsState            eCmaState;


   if (! MsgUpcast (pReqMsg, pmsgSetMipsState))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // must *always* return success
      }

   assert ((pmsgSetMipsState->m_Payload.eAction ==
                     MsgCmbSetMipsState::SetOperating) ||
           (pmsgSetMipsState->m_Payload.eAction ==
                     MsgCmbSetMipsState::SetSuspended));

   //  translate caller's state code into a CMA state code
   switch (pmsgSetMipsState->m_Payload.eAction)
      {
      case MsgCmbSetMipsState::SetOperating:
         eCmaState = k_eCmbMStRunningOSImage;
         break;

      case MsgCmbSetMipsState::SetSuspended:
         eCmaState = k_eCmbMStOSImageSuspended;
         break;

      default:
         //  tsk, some unrecognized request
         Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
         return (CTS_SUCCESS);      // must *always* return success
      }


   //  got desired state code, send it off to our CMA:

   //  build up CMB command message
   pCmbMsg = new CCmbMsgSender (this, k_eCmbCmdSetMipsState,
                                CMB_SELF, 1);

   //  add target request code (how message will be routed at destination)
   pCmbMsg -> AddParam (0, (U8) eCmaState);

   //  send command off to CMB (we simply use the request msg as our cookie):
   pCmbMsg -> Send (m_CmbHwIntf, pmsgSetMipsState,
                    CMBCALLBACK (CDdmCMB, ReqSetMipsState2));

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqSetMipsState */


//  We're called back when the CMB hw intf has a reply for our
//  k_eCmbCmdSetMipsState command.  This should always contain
//  an ACK of our Set Mips State request.
//
//  If we're HBC Master, we then proceed to update our row in the
//  IOP Status table, otherwise we're done.
//
//  Inputs:
//    pmsgSendSmallMsg - Original request which triggered our processing.
//    sStatus - Indicates how our "Set Mips State" request fared.
//                [It should always work.]
//    pReply - Points to reply from our CMA.
//

void  CDdmCMB::ReqSetMipsState2 (void *pmsgReq,
                                 STATUS sStatus, const CmbPacket *pReply)
{

MsgCmbSetMipsState * pmsgSetMipsState = (MsgCmbSetMipsState *) pmsgReq;
IopState                         eNewState;
IOPStatusRecord::RqModifyField * pModifyField;
U32                              iContigSlotMe;


   assert ((sStatus != CTS_SUCCESS) || (pReply != NULL));

   if (m_WeAreMasterHbc)
      {
      //  ok, we also want to update the IOP status table.

      switch (pmsgSetMipsState->m_Payload.eAction)
         {
         case MsgCmbSetMipsState::SetOperating:
            eNewState = IOPS_OPERATING;
            break;

         case MsgCmbSetMipsState::SetSuspended:
            eNewState = IOPS_SUSPENDED;
            break;

         default:
            //  tsk, some unrecognized request
            assert (FALSE);
            Reply (pmsgSetMipsState, CTS_CMB_INVALID_PARAMETER);
            return;
         }

      //  update our cached row info first, so we have a persistent copy
      //  of our new state code
      ValidIopSlot (m_eMyIopSlot, iContigSlotMe);
      m_aIopStatusImage[iContigSlotMe].eState = eNewState;

      //  now build up field modify message, using newly updated state
      //  cache as source value (must persist across message send)
      pModifyField = new IOPStatusRecord::RqModifyField
                              (m_aIopStatusImage[iContigSlotMe].rid,
                               CT_IOPST_IOPCURRENTSTATE,
                               &m_aIopStatusImage[iContigSlotMe].eState,
                               sizeof (m_aIopStatusImage[iContigSlotMe].eState));

      //  (our reply callback will notify requestor of completion)
      Send (pModifyField, pmsgReq, REPLYCALLBACK (CDdmCMB, ReqSetMipsState3));
      }
   else
      {
      //  we're all done, let caller know.
      Reply (pmsgSetMipsState, sStatus);
      }

   return;

}  /* end of CDdmCMB::ReqSetMipsState2 */


//  We're called back when PTS has finished processing our modify-field
//  request.  The PTS reply should always report success.
//
//  We then report success to the original "set MIPS state" requestor.
//
//  Inputs:
//    pmsgReply - Reply from PTS.
//

STATUS CDdmCMB::ReqSetMipsState3 (Message *pmsgReply)
{

MsgCmbSetMipsState * pmsgSetMipsState =
            (MsgCmbSetMipsState *) pmsgReply->GetContext();


   assert (pmsgReply != NULL);
   assert (pmsgReply->DetailedStatusCode == CTS_SUCCESS);
   assert ((pmsgSetMipsState != NULL) &&
           (pmsgSetMipsState->reqCode == MsgCmbSetMipsState::MyRequestCode()));

   //  simply let requestor know that all is coolness
   Reply (pmsgSetMipsState, CTS_SUCCESS);

   //  dispose of our PTS modify-field message
   delete pmsgReply;

   //  and report completion
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqSetMipsState3 */

