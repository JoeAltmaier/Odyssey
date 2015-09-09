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
// File: ReqIopControl.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    IOP Control DDM request message.
//
//
// $Log: /Gemini/Odyssey/DdmCmb/ReqIopControl.cpp $
// 
// 1     10/27/99 12:25p Ewedel
// Split off from ReqOther.cpp, and enhanced for current message def.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CtEvent.h"         // standard status codes


#include  <assert.h>



//
//  CDdmCMB::ReqIopControl (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_IOP_CONTROL request
//    message (class MsgCmbIopControl).
//
//    We refresh the specified IOP's row in the IOP Status table, once
//    we have completed the requested operation.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbIopControl, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqIopControl - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqIopControl (Message *pReqMsg)
{

MsgCmbIopControl   * pReq;
U32                  iContigSlot;
IopState             eCurState;


   if (! MsgUpcast (pReqMsg, pReq))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  grab IOP's current status first

   //  map from requested IOP slot number to contig slot #
   if ((! ValidIopSlot (pReq->m_Payload.eTargetSlot, iContigSlot)) ||
       (pReq->m_Payload.eTargetSlot >= NSLOT))
      {
      Reply (pReq, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);
      }


   //  grab our current state, as cached for IOP:
   //  (default to no state change)
   eCurState = m_aIopStatusImage [iContigSlot].eState;

   //  by the way, is there even a card in the requested slot?
   if ((eCurState == IOPS_EMPTY) || (eCurState == IOPS_BLANK))
      {
      //  oopsie
      Reply (pReq, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
      return (CTS_SUCCESS);
      }

   //  got current IOP state, and an IOP, now do what the request says
   //BUGBUG - we should be smarter about validating IOP state transitions!

   switch (pReq->m_Payload.eAction)
      {
      case MsgCmbIopControl::PowerOn:
         SendCmbMsg (pReq, k_eCmbCmdPowerControl,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbPowerOn);
         break;

      case MsgCmbIopControl::PowerOff:
         SendCmbMsg (pReq, k_eCmbCmdPowerControl,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbPowerOff);
         break;

      case MsgCmbIopControl::EnablePCI:
         SendCmbMsg (pReq, k_eCmbCmdPciBusAccessCtl,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbPciAccessEnable);
         break;

      case MsgCmbIopControl::DisablePCI:
         SendCmbMsg (pReq, k_eCmbCmdPciBusAccessCtl,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbPciAccessIsolate);
         break;

      case MsgCmbIopControl::EnableRemoval:
         if (eCurState != IOPS_POWERED_DOWN)
            {
            //  whoops, say "can't do that"
            ReqIopControl2 (pReq, CTS_CMB_REQUESTED_IOP_IN_USE, NULL);
            }
         else
            {
            //  give requested op a shot
            SendCmbMsg (pReq, k_eCmbCmdIopSlotLockCtl,
                        pReq->m_Payload.eTargetSlot,
                        CMBCALLBACK (CDdmCMB, ReqIopControl2),
                        1, k_eCmbLockDisable);
            }
         break;

      case MsgCmbIopControl::DisableRemoval:
         SendCmbMsg (pReq, k_eCmbCmdIopSlotLockCtl,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbLockEnable);
         break;

      case MsgCmbIopControl::Reset:
         SendCmbMsg (pReq, k_eCmbCmdResetCpu,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2),
                     1, k_eCmbCpuStrobeReset);
         break;

      case MsgCmbIopControl::NMI:
         //  (no params to NMI: it always strobes NMI to the MIPS CPU
         SendCmbMsg (pReq, k_eCmbCmdNmiCpu,
                     pReq->m_Payload.eTargetSlot,
                     CMBCALLBACK (CDdmCMB, ReqIopControl2));
         break;

      case MsgCmbIopControl::NOP:
      default:
         //  whoopsie..
         Reply (pReq, CTS_NOT_IMPLEMENTED);
         return (CTS_SUCCESS);
      }

   //  (our callback will do whatever needs doing)
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqIopControl */

//
//  We're called back when the requested CMB operation completes.
//  If the operation was successful, we update our cached state
//  to reflect the changed state.  In either case, when then reply
//  to the requestor to let them know how things went.
//
//  Inputs:
//    pvReqMsg - Veiled form of our original request message.
//    sStatus - Result of requested operation.
//    pReply - Reply from CMB machinery.  Only guaranteed valid when
//             sStatus == CTS_SUCCESS.
//    

void  CDdmCMB::ReqIopControl2 (void *pvReqMsg, STATUS sStatus,
                               const CmbPacket *pReply)
{

MsgCmbIopControl   * pReq  =  (MsgCmbIopControl *) pvReqMsg;
IopState             eNewState;


   assert (pReq != NULL);

   if (sStatus != CTS_SUCCESS)
      {
      //  whoops, request failed.  Stop right here.
      Reply (pReq, sStatus);
      return;
      }

   //  got an update, so let's refresh our status table.
   CmbStateToIopStatus (pReply, eNewState);

   //  roll new state into IOP Status table, and then reply to requestor
   SetIopPtsStateAndReply (pReq->m_Payload.eTargetSlot, eNewState, pReq);

   return;

}  /* end of CDdmCMB::ReqIopControl2 */

//
//  CDdmCMB::SetIopPtsStateAndReply (eIopSlot, eIopState, pRequest)
//
//  Description:
//    Called to update an IOP's "current state" field in the PTS IOP Status
//    table, and then send a reply back to the requestor which initiated
//    our present operation.
//
//    *  We expect to be called only when the requested operation was
//       completed successfully.
//
//    We are careful to always send some sort of status back to the
//    requestor, even if we can't update PTS.
//
//  Inputs:
//    eIopSlot - Slot of IOP whose state we're to update.
//    eIopState - New state of IOP (may be unchanged from current state).
//    pRequest - Request message which triggered the operation we report on.
//
//  Outputs:
//    none
//

void  CDdmCMB::SetIopPtsStateAndReply (TySlot eIopSlot, IopState eIopState,
                                       Message *pRequest)
{

U32                              iContigSlot;
IOPStatusRecord::RqModifyField * pModifyField;


   assert (pRequest != NULL);

   //  grab contiguous index for slot
   if (! ValidIopSlot (eIopSlot, iContigSlot))
      {
      //  oh, great.
      assert (FALSE);

      //  just let caller know something is wrong, even thought
      //  their requested operation may have already completed
      Reply (pRequest, CTS_CMB_INVALID_PARAMETER);
      return;
      }

   //  ok, see if state has changed from what we already have cached:
   if (eIopState == m_aIopStatusImage [iContigSlot].eState)
      {
      //  nope, no change.  Just report success & we're done.
      Reply (pRequest, CTS_SUCCESS);
      return;
      }

   //  got a delta in state values.  Update our cache, and then do the
   //  IOP's PTS row too.
   m_aIopStatusImage [iContigSlot].eState = eIopState;

   pModifyField = new IOPStatusRecord::RqModifyField
                           (m_aIopStatusImage [iContigSlot].rid,
                            CT_IOPST_IOPCURRENTSTATE,
                            &m_aIopStatusImage [iContigSlot].eState,
                            sizeof (m_aIopStatusImage [iContigSlot].eState));

   Send (pModifyField, pRequest,
         REPLYCALLBACK (CDdmCMB, SetIopPtsStateAndReply2a));

   return;

}  /* end of CDdmCMB::SetIopPtsStateAndReply */


//  We're called back when PTS has finished processing our modify-field
//  request.  The PTS reply should always report success.
//
//  We then report success to the original requestor.
//
//  Inputs:
//    pmsgReply - Reply from PTS.
//

STATUS  CDdmCMB::SetIopPtsStateAndReply2a (Message *pmsgReply)
{

Message   * pmsgRequest;


   assert ((pmsgReply != NULL) && (pmsgReply->Status() == CTS_SUCCESS));

   //  (we otherwise ignore the PTS result code)

   //  let requestor know we're all done, and it's cool.
   pmsgRequest = (Message *) pmsgReply->GetContext();
   assert (pmsgRequest != NULL);
   Reply (pmsgRequest, CTS_SUCCESS);

   //  tidy up after our own PTS update message
   delete pmsgReply;

   return (CTS_SUCCESS);      // as every good REPLYCALLBACK() does..

}  /* end of CDdmCMB::SetIopPtsStateAndReply2 */


