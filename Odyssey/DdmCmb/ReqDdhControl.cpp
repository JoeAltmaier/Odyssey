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
// File: ReqDdhControl.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    DDH Control request message.
//
//
// $Log: /Gemini/Odyssey/DdmCmb/ReqDdhControl.cpp $
// 
// 1     11/16/99 7:12p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CtEvent.h"         // standard status codes


#include  <assert.h>



//
//  CDdmCMB::ReqDdhControl (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_DDH_CONTROL request
//    message (class MsgCmbDdhControl).
//
//    We send the specified control operation on to the specified DDH.
//    We don't reply to the message sender until the operation has
//    completed, and the DDH has replied to us.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbDdhControl, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqDdhControl - Returns CTS_SUCCESS, as request handlers
//             are supposed to do.
//

STATUS CDdmCMB::ReqDdhControl (Message *pReqMsg)
{

MsgCmbDdhControl   * pReq  =  (MsgCmbDdhControl *) pReqMsg;
U32                  iContigSlot;
IopState             eCurState;
U8                   bBypassMask;


   if ((! MsgUpcast (pReqMsg, pReq)) ||
       (pReq->m_Payload.iBay > MsgCmbDdhControl::iDdhBayNumberMax) ||
       (pReq->m_Payload.eDdh < CMB_DDH0) || (pReq->m_Payload.eDdh > CMB_DDH3))
      {
      //  whoops, invalid message
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  see if targeted DDH is present in system

   //  map from requested DDH slot number to contig slot #
   if (! ValidIopSlot (pReq->m_Payload.eDdh, iContigSlot))
      {
      Reply (pReq, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);
      }

   //  grab our current state, as cached for DDH:
   eCurState = m_aIopStatusImage [iContigSlot].eState;

   //  by the way, is there even a DDH in the requested slot?
   if ((eCurState == IOPS_EMPTY) || (eCurState == IOPS_BLANK))
      {
      //  oopsie
      Reply (pReq, CTS_CMB_REQUESTED_DDH_SLOT_EMPTY);
      return (CTS_SUCCESS);
      }

   //  got a DDH, now do what the request says

   switch (pReq->m_Payload.eAction)
      {
      case MsgCmbDdhControl::SetBayLockState:
         SendCmbMsg (pReq, k_eCmbCmdSetDriveLock,
                     pReq->m_Payload.eDdh,
                     CMBCALLBACK (CDdmCMB, ReqDdhControl2),
                     2, pReq->m_Payload.iBay,
                     pReq->m_Payload.u.fLockBay ? 1 : 0);
         break;

      case MsgCmbDdhControl::SetDriveBypass:
         //  build up bitmask for command
         bBypassMask = 0;
         if (pReq->m_Payload.u.aBypassFlags[0].fChangeThisPort)
            {
            bBypassMask |= 0x01;    // enable bypass A setting change
            bBypassMask |= pReq->m_Payload.u.aBypassFlags[0].fBypassPort ?
                                    0x04 : 0;
            }
         if (pReq->m_Payload.u.aBypassFlags[1].fChangeThisPort)
            {
            bBypassMask |= 0x02;    // enable bypass B setting change
            bBypassMask |= pReq->m_Payload.u.aBypassFlags[1].fBypassPort ?
                                    0x08 : 0;
            }

         SendCmbMsg (pReq, k_eCmbCmdSetPortBypass,
                     pReq->m_Payload.eDdh,
                     CMBCALLBACK (CDdmCMB, ReqDdhControl2),
                     2, pReq->m_Payload.iBay, bBypassMask);
         break;

      case MsgCmbIopControl::NOP:
      default:
         //  whoopsie..
         Reply (pReq, CTS_NOT_IMPLEMENTED);
         return (CTS_SUCCESS);
      }

   //  (our callback will do whatever needs doing)
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqDdhControl */

//
//  We're called back when the requested DDH operation completes.
//  ** BUGBUG - we need to define DDH status table, so we can then
//     update it with the results of this operation.
//
//  We then reply to the requestor to let them know how things went.
//
//  Inputs:
//    pvReqMsg - Veiled form of our original request message.
//    sStatus - Result of requested operation.
//    pReply - Reply from CMB machinery.  Only guaranteed valid when
//             sStatus == CTS_SUCCESS.
//    

void  CDdmCMB::ReqDdhControl2 (void *pvReqMsg, STATUS sStatus,
                               const CmbPacket *pReply)
{

MsgCmbDdhControl   * pReq  =  (MsgCmbDdhControl *) pvReqMsg;
#pragma unused(pReply)


   assert (pReq != NULL);
   assert (sStatus == CTS_SUCCESS);

   //BUGBUG - until we have a DDH Status table to update, simply report
   //  status back to our requestor:
   Reply (pReq, sStatus);

   return;

}  /* end of CDdmCMB::ReqDdhControl2 */

