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
// File: ReqEvcControl.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    EVC Control request message.
//
//
// $Log: /Gemini/Odyssey/DdmCmb/ReqEvcControl.cpp $
// 
// 5     2/11/00 4:20p Eric_wedel
// [DFCT13005] Fixed reply leak in ReqEvcControl4().
// 
// 4     1/20/00 4:14p Eric_wedel
// Added support for "set boot conditions" and "EVC power control"
// requests.
// 
// 3     12/13/99 2:42p Ewedel
// Removed obsolete EVC Master stuff, and added commands for setting V54
// trim value and aux supply enable flags.  [VN]
// 
// 2     12/09/99 1:47a Iowa
// 
// 1     11/19/99 6:39p Ewedel
// Calved off from ReqOther.cpp.  Also updated fan speed info to work in
// units of %, and to keep our new fan speed setting cache current.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CtEvent.h"         // standard status codes


#include  <assert.h>



//
//  CDdmCMB::ReqEvcControl (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_EVC_CONTROL request
//    message (class MsgCmbEvcControl).
//
//    We perform the requested action, and then refresh the Env Control
//    table's (one & only) row.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbEvcControl, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqEvcControl - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqEvcControl (Message *pReqMsg)
{

MsgCmbEvcControl   * pReq;
CtEnvControlRecord::RqReadRow  * preqReadRow;
STATUS               sRet;


   if (! MsgUpcast (pReqMsg, pReq))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // (must *always* return success)
      }


   //  grab current env control settings first

   preqReadRow = new CtEnvControlRecord::RqReadRow(m_ridEnvCtl);

   sRet = Send (preqReadRow, pReqMsg,
                REPLYCALLBACK (CDdmCMB, ReqEvcControl2));
   assert (sRet == CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMB::ReqEvcControl */


//  We are called back in response to ReqEvcControl()'s read of
//  the current Env Control table value.
//
//  We perform whatever operation our original request sender
//  asked for.
//
//  Inputs:
//    pRaw - Points to veiled reply to our read-table operation.
//

STATUS CDdmCMB::ReqEvcControl2 (Message *pRaw)
{

CtEnvControlRecord::RqReadRow *pReply = (CtEnvControlRecord::RqReadRow *) pRaw;
MsgCmbEvcControl   * pReq;
EnvCtlContext      * pEnvCtx;
STATUS               sRet;
const U32            k_iFanPair  =  0;    // fan pair whose speeds we set
U8                   bAuxEnable;


   assert (pReply != NULL);

   pReq = (MsgCmbEvcControl *) pReply->GetContext();

   assert (pReq != NULL);

   sRet = pReply->Status();
   if (sRet != CTS_SUCCESS)
      {
      //  whoopsie, we're toast
      Reply (pReq, sRet);
      delete pReply;
      return (CTS_SUCCESS);
      }

   assert (pReply->GetRowCount() == 1);

   //  so far so good, now perform requested operations

   //  (pause to make a little context cookie)
   pEnvCtx = new EnvCtlContext;

   //  save original control request message
   pEnvCtx->pReq = pReq;

   //  and save current value of Env Control record
   pEnvCtx->EnvRec = *pReply->GetRowPtr();

   //  done with our read-row reply, so dispose of it
   delete pReply;


   //  do whatever changes are requested by our MsgCmbEvcControl input:
   switch (pReq->m_Payload.eAction)
      {
      case MsgCmbEvcControl::SetIntakeFanRpm:
         // set target RPMs of intake fan pair

         assert (pReq->m_Payload.u.ulTargetFanSpeed <= 100);

         if (pReq->m_Payload.u.ulTargetFanSpeed <= 100)
            {
            //  feed request to EVCs
            SendEvcMsg (pEnvCtx, k_eCmbCmdSetFanSpeed,
                        CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                        2,          // two bytes of params
                        k_eCmbFanPairIntake,
                        pReq->m_Payload.u.ulTargetFanSpeed);

            //  save it in our PTS record
            pEnvCtx->EnvRec.FanSpeedSet[k_eCmbFanPairIntake] =
                              pReq->m_Payload.u.ulTargetFanSpeed;

            //  .. and in our local cache
            m_aulFanSpeedSet[k_eCmbFanPairIntake] =
                              pReq->m_Payload.u.ulTargetFanSpeed;
            }
         else
            {
            //  invalid parameter; hit our callback in a no-op way:
            ReqEvcControl3 (pEnvCtx, CTS_CMB_INVALID_PARAMETER, NULL);
            }
         break;

      case MsgCmbEvcControl::SetExhaustFanRpm:
         // set target RPMs of exhaust fan pair

         assert (pReq->m_Payload.u.ulTargetFanSpeed <= 100);


         if (pReq->m_Payload.u.ulTargetFanSpeed <= 100)
            {
            //  feed request to EVCs
            SendEvcMsg (pEnvCtx, k_eCmbCmdSetFanSpeed,
                        CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                        2,          // two bytes of params
                        k_eCmbFanPairExhaust,
                        pReq->m_Payload.u.ulTargetFanSpeed);

            //  save it in our PTS record
            pEnvCtx->EnvRec.FanSpeedSet[k_eCmbFanPairExhaust] =
                              pReq->m_Payload.u.ulTargetFanSpeed;

            //  .. and in our local cache
            m_aulFanSpeedSet[k_eCmbFanPairExhaust] =
                              pReq->m_Payload.u.ulTargetFanSpeed;
            }
         else
            {
            //  invalid parameter; hit our callback in a no-op way:
            ReqEvcControl3 (pEnvCtx, CTS_CMB_INVALID_PARAMETER, NULL);
            }
         break;

      case MsgCmbEvcControl::SetFanUpThresh:
         // set auto fan up / restore temp thresholds

         assert (pReq->m_Payload.u.ulFanUpThresh < 100);

         if (pReq->m_Payload.u.ulFanUpThresh < 100)
            {
            //  set threshold (scale from 1.0 degree to 0.5 degree lsb
            SendEvcMsg (pEnvCtx, k_eCmbCmdSetThreshold,
                        CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                        2,          // two bytes of params
                        k_eCmbFanTempThreshFanUp,
                        pReq->m_Payload.u.ulFanUpThresh * 2);

            //  save it in our PTS record (saved in 0.5 deg scaled form)
            pEnvCtx->EnvRec.ExitTempFanUpThresh =
                           pReq->m_Payload.u.ulFanUpThresh * 2;
            }
         else
            {
            //  invalid parameter; hit our callback in a no-op way:
            ReqEvcControl3 (pEnvCtx, CTS_CMB_INVALID_PARAMETER, NULL);
            }
         break;

      case MsgCmbEvcControl::SetFanRestoreThresh:
         // set fan-restore temp threshold

         assert (pReq->m_Payload.u.ulFanRestoreThresh < 100);

         if (pReq->m_Payload.u.ulFanRestoreThresh < 100)
            {
            //  set threshold (scale from 1.0 degree to 0.5 degree lsb
            SendEvcMsg (pEnvCtx, k_eCmbCmdSetThreshold,
                        CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                        2,          // two bytes of params
                        k_eCmbFanTempThreshFanRestore,
                        pReq->m_Payload.u.ulFanRestoreThresh * 2);

            //  save it in our PTS record (saved in 0.5 deg scaled form)
            pEnvCtx->EnvRec.ExitTempFanNormThresh =
                           pReq->m_Payload.u.ulFanRestoreThresh * 2;
            }
         else
            {
            //  invalid parameter; hit our callback in a no-op way:
            ReqEvcControl3 (pEnvCtx, CTS_CMB_INVALID_PARAMETER, NULL);
            }
         break;

      case MsgCmbEvcControl::SetSmpV54Adjust:
         // set EVCs' 54v supply trimming values

         //  (we have no idea what valid values are, so we'll
         //   pass through anything right now)

         //  set EVCs' 54v adjust byte
         SendEvcMsg (pEnvCtx, k_eCmbCmdSetSmpV54,
                     CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                     1,          // one byte of params
                     pReq->m_Payload.u.bSmpV54Adjust);

         //  Note that we *don't* save the v54 trim value in our
         //  control record.  It must be rederived manually each
         //  time the system comes up.  This is a safety issue,
         //  since we really don't want to over-charge the batteries.
         //  The EVC sets a "safe" default for this value, until we
         //  tell it different.
         break;

      case MsgCmbEvcControl::SetAuxSupplyEnable:
         //  control aux power supply enables.  Note that both EVCs
         //  control each aux supply, even though the aux supplies
         //  are located on individual EVCs.

         //  build up aux supply enable CMB control parameter byte:
         bAuxEnable = 0;
         if (pReq->m_Payload.u.aAuxSupplyEnable[0].fChangeEnable)
            {
            bAuxEnable = 0x10;
            if (pReq->m_Payload.u.aAuxSupplyEnable[0].fEnableState)
               {
               bAuxEnable |= 1;
               }
            }
         if (pReq->m_Payload.u.aAuxSupplyEnable[1].fChangeEnable)
            {
            bAuxEnable = 0x20;
            if (pReq->m_Payload.u.aAuxSupplyEnable[1].fEnableState)
               {
               bAuxEnable |= 2;
               }
            }

         //  got control byte built up, now send aux enable command
         SendEvcMsg (pEnvCtx, k_eCmbCmdSetAuxSupplyEna,
                     CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                     1,          // one byte of params
                     bAuxEnable);

         //  Note that we *don't* save the aux enable settings in our
         //  control record.  We always default to all supplies enabled
         //  until told otherwise.
         break;

      case MsgCmbEvcControl::SetBootConditions:
         //  set "boot condition" flags.  Right now there is only one,
         //  and it controls what the EVCs will read as a valid boot
         //  request:  key in non-"off" position, or only in "service"
         //  position.

         //  build up flag byte:
         bAuxEnable = pReq->m_Payload.u.BootConditions.fServiceOnly ? 1 : 0;

         //  got flag byte built up, now send "set boot conditions" command
         SendEvcMsg (pEnvCtx, k_eCmbCmdSetBootCondit,
                     CMBCALLBACK (CDdmCMB, ReqEvcControl3),
                     1,          // one byte of params
                     bAuxEnable);

         //  (note that we use our standard ReqEvcControl3() callback.
         //   This will do a gratuitous update of our EVC control rec
         //   in PTS, but is basically harmless.)
         break;

      default:
         //  unrecognized parameter, so fail the request:
         ReqEvcControl3 (pEnvCtx, CTS_CMB_INVALID_PARAMETER, NULL);
      }

   return (CTS_SUCCESS);      // as a REPLYCALLBACK() always does..

}  /* end of CDdmCMB::ReqEvcControl2 */


//  We're called back when our helper has updated as many EVCs
//  as it can find.  If the update was successful, we roll the
//  revised EVC control record back to PTS.  Otherwise, we just
//  blast things right here.
//
//  Inputs:
//    pCtx - Veiled context of our current request processing.
//    sStatus - Status of operation - success if at least one EVC
//                was updated successfully.
//    pReply - Reply packet, uncertain significance (which EVC, etc.).
//

void CDdmCMB::ReqEvcControl3 (void *pCtx, STATUS sStatus,
                              const CmbPacket *pReply)
{

#pragma unused(pReply)

EnvCtlContext *pEnvCtx = (EnvCtlContext *) pCtx;
CtEnvControlRecord::RqModifyRow   * pUpdate;


   assert (pEnvCtx != NULL);

   //  save final reported status in our cookie
   pEnvCtx->sResult = sStatus;

   //  if we were successful, update our Env Ctl record in PTS
   if (sStatus == CTS_SUCCESS)
      {
      pUpdate = new CtEnvControlRecord::RqModifyRow (pEnvCtx->EnvRec);

      Send (pUpdate, pEnvCtx, REPLYCALLBACK (CDdmCMB, ReqEvcControl4));
      }
   else
      {
      //  failed, so just stop here & clean things up.
      Reply (pEnvCtx->pReq, sStatus);
      delete pEnvCtx;
      }

   return;

}  /* end of CDdmCMB::ReqEvcControl3 */


//  We're called back when our env control table has been updated.
//  We simply tidy up and report EVC update status to the requestor.
//
//  Inputs:
//    pReply - Reply to our PTS row modify request.
//

STATUS CDdmCMB::ReqEvcControl4 (Message *pReply)
{

EnvCtlContext *pEnvCtx = (EnvCtlContext *) pReply->GetContext();


   assert (pEnvCtx != NULL);
   assert (pReply->Status() == CTS_SUCCESS);

   //  report EVC operation result to caller (caller doesn't care about
   //  our PTS activity, really..)
   Reply (pEnvCtx->pReq, pEnvCtx->sResult);

   //  dispose of our cookie
   delete pEnvCtx;

   //  and our PTS reply
   delete pReply;

   return (CTS_SUCCESS);      // as a REPLYCALLBACK always does

}  /* end of CDdmCMB::ReqEvcControl4 */

//
//  CDdmCMB::ReqEvcPowerControl (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_EVC_POWER_CONTROL request
//    message (class MsgCmbEvcPowerControl).
//
//    We perform the requested action, and then refresh the Env Control
//    table's (one & only) row.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbEvcPowerControl, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqEvcPowerControl - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqEvcPowerControl (Message *pReqMsg)
{

MsgCmbEvcPowerControl * pReq;


   if (! MsgUpcast (pReqMsg, pReq))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  no additional data needed, simply examine message's payload:
   switch (pReq->m_Payload.eAction)
      {
      case MsgCmbEvcPowerControl::SetAllPowerOff:
         //  yowsa!  This is it, let's tell the EVCs to shut us off.
         //  We do this by simply telling them to turn off all
         //  primary supplies.  They recognize this to mean that
         //  we want *everything* turned off.
         SendEvcMsg (pReqMsg, k_eCmbCmdSetPriSupplyEna,
                     CMBCALLBACK (CDdmCMB, ReqEvcPowerControl2),
                     1, 0x70);
         break;

      default:
         //  hmm.. unrecognized action code
         Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      }
   
   //  all done, for now
   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMB::ReqEvcPowerControl */


//  We're called back when our helper has updated as many EVCs
//  as it can find.  Normally, we won't be called back, since
//  the EVCs will have turned off power.  But just in case,
//  we report the results of the EVC message send (may have failed
//  for some reason) to our caller.
//
//  Inputs:
//    pvReqMsg - Veiled request message which started this all.
//    sStatus - Status of operation - success if at least one EVC
//                was updated successfully.
//    pReply - Reply packet, uncertain significance (which EVC, etc.).
//

void CDdmCMB::ReqEvcPowerControl2 (void *pvReqMsg, STATUS sStatus,
                                   const CmbPacket *pReply)
{

#pragma unused(pReply)

MsgCmbEvcPowerControl * pReq;


   pReq = (MsgCmbEvcPowerControl *) pvReqMsg;
   assert (pReq != NULL);

   //  let requestor know what happened, as best we understand it
   Reply (pReq, sStatus);

   //  and that's all we do.
   return;

}  /* end of CDdmCMB::ReqEvcPowerControl2 */

