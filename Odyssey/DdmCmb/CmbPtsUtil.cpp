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
// File: CmbPtsUtil.cpp
//
// Description:
//    CMB DDM helper members for handling PTS updates.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbPtsUtil.cpp $
// 
// 7     9/03/99 5:21p Ewedel
// Updated for Temp -> Temp1 param name changes.
// 
// 6     8/24/99 8:06p Ewedel
// Fixed jfl TS interface bugs.
// 
// 5     8/15/99 12:05p Jlane
// Added parameters for new TS interface changes.
// 
// 4     7/21/99 8:19p Ewedel
// Fixes for TS interface changes.
// 
// 3     7/20/99 4:57p Rkondapalli
// No more replies ffrom Send on PTS objects.
// 
// 2     7/20/99 2:11p Rkondapalli
// [ewx]  Fixed UpdatePtsIopRow() and FinishIopRowModify() so that Reply()
// is only called when a req is provided.
// 
// 1     7/15/99 4:26p Ewedel
// Initial revision.
//
/*************************************************************************/

#include  "DdmCMB.h"

#include  "Fields.h"     // PTS stuff



//
//  CDdmCMB::UpdatePtsIopRow (pSlotCtx)
//
//  Description:
//    This routine is called to finish off an IOP row-related operation.
//    We take a standard "slot context" instance, which must in turn
//    contain a ready-to-write IOP Status table row.
//
//    We conclude the PTS update with a callback to FinishIopRowModify(),
//    which will take care of notifying the original requestor that our
//    processing is completed.  FinishIopRowModify() also tidies up the
//    various memory allocations.
//
//    *  Note that we ignore everything in *pSlotCtx except for the
//       IOP Row pointer, contiguous slot index, and the original request
//       message.
//       In particular, we do *not* copy the various IOP parameters
//       from *pSlotCtx into the row record: the record must be finished
//       and ready to write when we are called.
//
//  Inputs:
//    pSlotCtx - A standard IOP slot context struct.  The members
//                pIopRow, iContigSlot and pReq must be valid.
//
//  Outputs:
//    *pSlotCtx - Deleted, along with *pSlotCtx->pIopRow.
//

void CDdmCMB::UpdatePtsIopRow (SlotContext *pSlotCtx)
{

TSModifyRow  * pModifyRow;
STATUS         sRet;


   assert ((pSlotCtx != NULL) && (pSlotCtx->pIopRow != NULL));
   assert (m_aeContigToSlot[pSlotCtx -> iContigSlot] ==
           pSlotCtx -> pIopRow -> Slot);

   //  build our action message, a row modification
   pModifyRow = new TSModifyRow;

   sRet = pModifyRow -> Initialize (
      this,
      CT_IOPST_TABLE_NAME,
      CT_PTS_RID_FIELD_NAME,     // standard PTS-defined rowID field
      &(m_aIopStatusImage[pSlotCtx->iContigSlot].rid),
      sizeof (m_aIopStatusImage[pSlotCtx->iContigSlot].rid),
      pSlotCtx -> pIopRow,    // this is *not* consumed by TSModifyRow
      sizeof (IOPStatusRecord),
      1,                      // modify just one row
      NULL,                   // ignore rows-modified return
      &pSlotCtx->ridNewRow,
      sizeof(pSlotCtx->ridNewRow),
      (pTSCallback_t) &FinishIopRowModify,
      pSlotCtx);
   assert (sRet == CTS_SUCCESS);

   //  We also maintain local cached copies of some IOP Status values.
   //  While we might update our cache in FinishIopRowModify(), this
   //  may be too late since we might process other messages from our
   //  queue in the interim (before FinishIopRowModify() is called).
   //  So we update our cached data right here.
   //  *sigh*  Cache coherency with PTS is non-trivial.
   m_aIopStatusImage [pSlotCtx->iContigSlot].eState =
               pSlotCtx->pIopRow->eIOPCurrentState;
   m_aIopStatusImage [pSlotCtx->iContigSlot]. TempNormThreshold =
               pSlotCtx->pIopRow->TempNormThreshold;
   m_aIopStatusImage [pSlotCtx->iContigSlot].TempHiThreshold =
               pSlotCtx->pIopRow->TempHiThreshold;
   memcpy (m_aIopStatusImage [pSlotCtx->iContigSlot].ChassisSerialNumber,
           pSlotCtx->pIopRow->ChassisSerialNumber,
           sizeof (m_aIopStatusImage [pSlotCtx->iContigSlot].ChassisSerialNumber));

   //  do the row update
   pModifyRow -> Send();

   return;

}  /* end of CDdmCMB::UpdatePtsIopRow */

//
//  CDdmCMB::FinishIopRowModify (pClientContext, status)
//
//  Description:
//    Called when a PTS row-modify operation has completed for some
//    row in the IOP Status table.  This is not a listen callback,
//    but rather is explicitly asked for in the various places which
//    do IOP row-modify operations in our DDM.
//
//    We tidy up after the row-modify, and report the final status
//    to our requestor.
//
//  Inputs:
//    pClientContext - Aliased reference to pSlotCtx, "slot context" for
//                   IOP Status row which we are updating.
//    status - Result of PTS modify-row operation.
//
//  Outputs:
//    CDdmCMB::FinishIopRowModify - Returns CTS_SUCCESS.
//

STATUS CDdmCMB::FinishIopRowModify (void *pClientContext, STATUS status)
{

SlotContext           * pSlotCtx;         // our slot operation context


//*   Tracef("CDdmCMB::FinishIopRowModify(), status = %d\n", status);

   //  up-cast our slot-context
   pSlotCtx = (SlotContext *) pClientContext;

   if (status == CTS_SUCCESS)
      {
      //  if the row update succeeded, record slot's new row ID
      //  (we had to write the new ID to someplace other than our
      //   main rid array, or else it might have trashed the old rid
      //   which we used as a key value for the operation)

      m_aIopStatusImage[pSlotCtx -> iContigSlot].rid = pSlotCtx -> ridNewRow;
      }

   //  let requestor know how it went
   if (pSlotCtx->pReq != NULL)
      {
      Reply (pSlotCtx -> pReq, status);
      }

   //  free up putative new row record, and context holder
   delete pSlotCtx -> pIopRow;
   delete pSlotCtx;

   //  tell CHAOS that we're happy (though request originator might not be)
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::FinishIopRowModify */

//
//  CDdmCMB::UpdatePtsIopState (iContigSlot, eNewState)
//
//  Description:
//    This routine is called to finish off an IOP state update operation.
//
//    We set the given IOP's state in the IOP Status table to the provided
//    state value.
//
//    We expect to be called in response to an unsolicited message from
//    our local CMA, so we have no provision for calling a "user" callback
//    when our update is completed.
//
//  Inputs:
//    iContigSlot - Contiguous index of IOP whose state we're to update.
//                This value is as returned by ValidateIopSlot().
//    eNewState - New State value to set as IOP's current state.
//
//  Outputs:
//    none
//

void CDdmCMB::UpdatePtsIopState (U32 iContigSlot, IopState eNewState)
{

TSModifyField      * pModifyField;
ModifyStateContext * pModifyStateCtx;


   //  build our action message, a field modification
   pModifyField = new TSModifyField;

   pModifyStateCtx = new ModifyStateContext (eNewState);

   pModifyField -> Initialize (
      this,
      CT_IOPST_TABLE_NAME,
      CT_PTS_RID_FIELD_NAME,     // standard PTS-defined rowID field
      &(m_aIopStatusImage[iContigSlot].rid),
      sizeof (m_aIopStatusImage[iContigSlot].rid),
      CT_IOPST_IOPCURRENTSTATE,
      &pModifyStateCtx->eState,
      sizeof (pModifyStateCtx->eState),
      1,                         // modify just one row
      NULL,                      // ignore rows-modified return
      &pModifyStateCtx->ridNewRow,
      sizeof (pModifyStateCtx->ridNewRow),
      (pTSCallback_t) &UpdatePtsIopState2,
      pModifyStateCtx);

   //  We also maintain local cached copies of some IOP Status values.
   //  While we might update our cache in UpdatePtsIopState2(), this
   //  may be too late since we might process other messages from our
   //  queue in the interim (before UpdatePtsIopState2() is called).
   //  So we update our cached data right here.
   //  *sigh*  Cache coherency with PTS is non-trivial.
   m_aIopStatusImage [iContigSlot].eState = eNewState;

   //  do the row update
   pModifyField -> Send();
   return;

}  /* end of CDdmCMB::UpdatePtsIopState */

//
//  CDdmCMB::UpdatePtsIopState2 (pModifyStateCtx, sStatus)
//
//  Description:
//    This routine is called to finish off an IOP Status table
//    state field update.
//
//    We merely tidy up the context carrier which UpdatePtsIopState()
//    allocated to give a long enough lifetime for the field data and
//    returned rowID.
//
//  Inputs:
//    pModifyStateCtx - The context carrier which we need to release.
//    sStatus - Result of the state field PTS update.
//
//  Outputs:
//    CDdmCMB::UpdatePtsIopState2 - We always return CTS_SUCCESS.
//

STATUS  CDdmCMB::UpdatePtsIopState2 (ModifyStateContext *pModifyStateCtx,
                                     STATUS sStatus)
{

#pragma unused(sStatus)


//*   Tracef ("CDdmCMB::UpdatePtsIopState2: sStatus = %X\n", sStatus);

   //  merely free up the context carrier, and we're done
   delete pModifyStateCtx;

   //  our return code is not a meaningful value, since we're called
   //  back from class TSModifyField.  So we play it safe:
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::UpdatePtsIopState2 */



