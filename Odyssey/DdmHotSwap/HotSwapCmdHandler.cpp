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
// File: HotSwapCmdHandler.cpp
//
// Description:
//    Contains implementation of CHotSwapCmdHandler "smart cookie".
//    For more info, see HotSwapCmdHandler.h, or the function headers below.
//
//
// $Log: /Gemini/Odyssey/DdmHotSwap/HotSwapCmdHandler.cpp $
// 
// 4     1/11/00 7:30p Mpanas
// new PathDescriptor table changes
// - use new rid for DiskDescriptor table
// 
// 3     12/22/99 6:38p Ewedel
// Added host DDM instance ptr param to our constructor, as this must be
// passed through to our DdmServices base class.  Also restored proper
// simple form of DiskDescriptor::RqReadRow() instantiation.
// 
// 2     12/16/99 4:00a Dpatel
// [ewx] Added [partial?] workaround for apparent rid alignment bug in hot
// swap request struct.
// 
// 1     12/16/99 1:12a Ewedel
// Initial revision.
//
/*************************************************************************/


#include  "Simple.h"

#include  "Odyssey.h"         // disk type codes

#include  "HotSwapCmdHandler.h"

#include  "StorageRollCallTable.h"     // for checking drive's "free-ness"

#include  "DdmCmbMsgs.h"      // how we do things to the DDHes

#include  "CtEvent.h"         // status codes

#include  "Odyssey_trace.h"   // debug stuff
#include  <assert.h>          // debug stuff


//
//  CDdmHotSwap::CDdmHotSwap (did)
//
//  Description:
//    Our constructor.
//
//  Inputs:
//    did - CHAOS "Device ID" of the instance we are constructing.
//
//  Outputs:
//    none
//

CHotSwapCmdHandler::CHotSwapCmdHandler (DdmServices *pHostDdm, const CHotSwapIntf& req, HANDLE hReq,
                                        CmdServer& HotSwapCmdQueue) :
                                          DdmServices (pHostDdm),
                                          m_req (req),
                                          m_hReq (hReq),
                                          m_HotSwapCmdQueue (HotSwapCmdQueue)
{

DiskDescriptor::RqReadRow   * pReadRow;


   //  note that we don't yet have a disk descriptor query reply
   m_pDiskDescrReply = NULL;

   //  first off, we need to retrieve some relevant data.

   //  Do that according to our request type:
   //  (only disk operations, right now)

   switch (m_req.eCommand)
      {
      case CHotSwapIntf::k_eReleaseDriveBay:
         //  enable a drive to be removed

      case CHotSwapIntf::k_eRestoreDriveBay:
         //  lock drive into bay, place it in FC loop?

         //  first, read the drive's disk descriptor entry: the action
         //  continues in HandleDiskCommand()
         pReadRow = new DiskDescriptor::RqReadRow (m_req.ridDiskDescr);
         Send (pReadRow, NULL, REPLYCALLBACK (CHotSwapCmdHandler,
                                              HandleDiskCommand));

         break;

      default:
         //  unknown / unsupported request code
         Tracef ("CHotSwapCmdHandler::CmdReady: unknown command code %d\n",
                 m_req.eCommand);

         //  stop right here.
         ReportErrorAndSelfDestruct (CTS_HSW_INVALID_PARAMETER);

         //  (can't do anything more to our instance, as
         //   we no longer have an instance)
      }

   return;

}  /* end of CHotSwapCmdHandler::CHotSwapCmdHandler() */

//
//  CHotSwapCmdHandler::HandleDiskCommand (pmsgRawReply)
//
//  Description:
//    We're called back with the results of a query to the Disk
//    Descriptor table to obtain the row specified by our requestor.
//
//    Assuming that we got a valid row, we use its data to derive
//    the raw CMB-level parameters which we need to perform the
//    requested operation.
//
//    If a drive release is requested, we first ensure that the
//    drive is "free", and thus eligible to be released.
//
//  Inputs:
//    pmsgRawReply - Base class form of reply to our PTS query.
//
//  Outputs:
//    CHotSwapCmdHandler::HandleDiskCommand - Returns CTS_SUCCESS
//                always, like a good little reply callback.
//

STATUS  CHotSwapCmdHandler::HandleDiskCommand (Message *pmsgRawReply)
{

StorageRollCallRecord::RqReadRow  * pReadRow;
MsgCmbDdhControl                  * pDdhCtl;
STATUS                              sRet;


   m_pDiskDescrReply = (DiskDescriptor::RqReadRow *) pmsgRawReply;
   assert (m_pDiskDescrReply != NULL);
   m_pDiskDescr = m_pDiskDescrReply->GetRowPtr();

   sRet = m_pDiskDescrReply->Status();
   if ((sRet != CTS_SUCCESS) || (m_pDiskDescrReply->GetRowCount() != 1))
      {
      //  whoops, we're toast.
      if (sRet == CTS_SUCCESS)
         {
         //  hmm, should never happen, but if it does claim an invalid param
         sRet = CTS_HSW_INVALID_PARAMETER;
         }
      ReportErrorAndSelfDestruct (sRet);

      //  (our destructor cleans up reply for us)
      return (CTS_SUCCESS);
      }

   //  got the row data, do what we wish with it.

   //  first, check to see that our disk is an internal unit:
#ifdef WEIRD_CW_BUG_FIXED
//BUGBUG - for some reason, the compiler refuses to recognize TypeFcDisk,
//         even though its header, Odyssey.h, is included.  Arghh!
   if (m_pDiskDescr->DiskType != TypeFcDisk)
      {
      //  not an internal Odyssey (DDH-connected) drive, so fail.
      ReportErrorAndSelfDestruct (CTS_HSW_INVALID_PARAMETER);

      //  (our destructor cleans up reply for us)
      return (CTS_SUCCESS);
      }
#endif  // #ifdef WEIRD_CW_BUG_FIXED

   //  So far, so good.
   //  Do different stuff depending upon our request type (we should
   //  only ever be called [back] for disk-related operations).

   switch (m_req.eCommand)
      {
      case CHotSwapIntf::k_eReleaseDriveBay:
         //  enable a drive to be removed.  First, we must verify that
         //  it is ok to remove this drive.  Find storage rollcall table
         //  entry for our drive's disk descriptor:
         pReadRow = new StorageRollCallRecord::RqReadRow
                           (fdSRC_DESC_RID,
                            &m_pDiskDescr->rid,
                            sizeof (m_pDiskDescr->rid));
         Send (pReadRow, NULL, REPLYCALLBACK (CHotSwapCmdHandler,
                                              HandleDiskRelease));

         //  action will resume in HandleDiskRelease()
         break;

      case CHotSwapIntf::k_eRestoreDriveBay:
         //  lock drive into bay, & place it in FC loop.
         //  This one, we can do as long as a drive is present.
         //  And if we have a disk descriptor, then the drive is there.

         //  set up message with proper params
         pDdhCtl = new MsgCmbDdhControl (MsgCmbDdhControl::SetBayLockState);
         SetDdhDriveId (pDdhCtl);
         pDdhCtl->m_Payload.u.fLockBay = TRUE;

         //  send it off to the CMB DDM
         Send (pDdhCtl, NULL, REPLYCALLBACK (CHotSwapCmdHandler,
                                             HandleDiskRestore));

         //  action will resume in HandleDiskRestore()
         break;

      default:
         //  another "should never happen"
         assert (FALSE);
         ReportErrorAndSelfDestruct (CTS_HSW_INVALID_PARAMETER);
      }

   //  report success, as reply callbacks do.
   return (CTS_SUCCESS);

}  /* end of CHotSwapCmdHandler::HandleDiskCommand */

//
//  CHotSwapCmdHandler::HandleDiskRelease (pmsgRawReply)
//
//  Description:
//    We're called back with the results of a query to the Storage
//    RollCall table for the entry corresponding to our disk drive.
//
//    If we get the entry (failing to is an error) then we check to
//    see if the drive is free or in use.  If in use, we stop and
//    return an error.  Otherwise, we proceed to release the drive's
//    locking solenoid.
//
//  Inputs:
//    pmsgRawReply - Base class form of reply to our PTS query.
//
//  Outputs:
//    CHotSwapCmdHandler::HandleDiskRelease - Returns CTS_SUCCESS
//                always, like a good little reply callback.
//

STATUS  CHotSwapCmdHandler::HandleDiskRelease (Message *pmsgRawReply)
{

StorageRollCallRecord::RqReadRow  * pReadRow;
MsgCmbDdhControl                  * pDdhCtl;
STATUS                              sRet;


   pReadRow = (StorageRollCallRecord::RqReadRow *) pmsgRawReply;

   sRet = pReadRow->Status ();
   if ((sRet != CTS_SUCCESS) || (pReadRow->GetRowCount() != 1))
      {
      //  whoops, we're toast.
      if (sRet == CTS_SUCCESS)
         {
         //  hmm, should never happen, but if it does claim an invalid param
         sRet = CTS_HSW_INVALID_PARAMETER;
         }
      ReportErrorAndSelfDestruct (sRet);

      //  dispose of reply & we're very done.
      delete pReadRow;
      return (CTS_SUCCESS);
      }

   //  got row data, how about it?  Is the drive in use?
   if (pReadRow->GetRowPtr()->fUsed)
      {
      //  whoops, drive is in use.  Return our very own, customized
      //  and ever-so-stylish error response:
      ReportErrorAndSelfDestruct (CTS_HSW_DRIVE_IN_USE);

      //  dispose of reply & we're done.
      delete pReadRow;
      return (CTS_SUCCESS);
      }

   //  drive is free, so go ahead and unlock its solenoid

   //  set up message with proper params
   pDdhCtl = new MsgCmbDdhControl (MsgCmbDdhControl::SetBayLockState);
   SetDdhDriveId (pDdhCtl);
   pDdhCtl->m_Payload.u.fLockBay = FALSE;

   //  send msg on its way.  Note that we use standard callback handler
   //  to report results of our operation, since this is our last step.
   Send (pDdhCtl, NULL, REPLYCALLBACK (CHotSwapCmdHandler,
                                       ReportResults));

   //  dispose of reply, since we're through with it
   delete pReadRow;

   //  per reply callback custom:
   return (CTS_SUCCESS);

}  /* end of CHotSwapCmdHandler::HandleDiskRelease */

//
//  CHotSwapCmdHandler::HandleDiskRestore (pmsgRawReply)
//
//  Description:
//    We're called back with the results of sending a MsgCmbDdhControl
//    message to the CMB to lock our drive in its bay.
//
//    Assuming that much went ok, we continue by requesting that the
//    CMB enable FC port access to the bay (just in case it had
//    previously been turned off).
//
//  Inputs:
//    pmsgRawReply - Base class form of reply to our MsgCmbDdhControl msg.
//
//  Outputs:
//    CHotSwapCmdHandler::HandleDiskRestore - Returns CTS_SUCCESS
//                always, like a good little reply callback.
//

STATUS  CHotSwapCmdHandler::HandleDiskRestore (Message *pmsgRawReply)
{

MsgCmbDdhControl   * pDdhCtl;


   pDdhCtl = (MsgCmbDdhControl *) pmsgRawReply;

   if (pDdhCtl->Status() != CTS_SUCCESS)
      {
      //  whoops, DDH control operation wiped out, so we're done.
      ReportErrorAndSelfDestruct (pDdhCtl->Status());

      //  dispose of reply & we're out of here
      delete pDdhCtl;
      return (CTS_SUCCESS);
      }

   //  we're done with reply, so dump it
   delete pDdhCtl;

   //  drive is locked in bay, now ensure that its FC loop access is enabled
   pDdhCtl = new MsgCmbDdhControl (MsgCmbDdhControl::SetDriveBypass);
   SetDdhDriveId (pDdhCtl);
   pDdhCtl->m_Payload.u.aBypassFlags[0].fBypassPort = FALSE;
   pDdhCtl->m_Payload.u.aBypassFlags[0].fChangeThisPort = TRUE;
   pDdhCtl->m_Payload.u.aBypassFlags[1].fBypassPort = FALSE;
   pDdhCtl->m_Payload.u.aBypassFlags[1].fChangeThisPort = TRUE;

   //  send msg on its way.  Note that we use standard callback handler
   //  to report results of our operation, since this is our last step.
   Send (pDdhCtl, NULL, REPLYCALLBACK (CHotSwapCmdHandler, ReportResults));

   //  as per usual
   return (CTS_SUCCESS);

}  /* end of CHotSwapCmdHandler::HandleDiskRestore */

//
//  CHotSwapCmdHandler::ReportResults (pmsgReply)
//
//  Description:
//    This is our catch-all reply callback for when we're running
//    the last step of some operation.
//
//    We simply report the final op status back to the requestor,
//    dispose of our reply and our instance, and we're done.
//
//  Inputs:
//    pmsgReply - Reply to some operation or other.  We use this reply's
//             status as our final operation status.
//
//  Outputs:
//    CHotSwapCmdHandler::ReportResults - Returns CTS_SUCCESS, always.
//             Why do reply callbacks even have return codes?
//

STATUS  CHotSwapCmdHandler::ReportResults (Message *pmsgReply)
{


   //  as luck would have it, we can use ReportErrorAndSelfDestruct()
   //  to do most of our work for us, although of course we aren't
   //  necessarily reporting an error.
   ReportErrorAndSelfDestruct (pmsgReply->Status());

   //  dispose of reply message, and we're out of here
   //  (our instance data is already gone)
   delete pmsgReply;

   return (CTS_SUCCESS);

}  /* end of CHotSwapCmdHandler::ReportResults */

//
//  CHotSwapCmdHandler::ReportErrorAndSelfDestruct (sRet)
//
//  Description:
//    Reports an error status back to our requestor, and then
//    disposes of our instance.
//
//    Note that this "smart cookie" class is designed for
//    instances to self-destruct when they complete.
//
//    ** Since this routine deletes our instance, all callers must
//       be careful not to touch any instance data after calling
//       this routine.
//
//  Inputs:
//    sRet - Status code to report to requestor.
//
//  Outputs:
//    none
//

void  CHotSwapCmdHandler::ReportErrorAndSelfDestruct (STATUS sRet)
{


   //  send an error reply back to requestor
   m_HotSwapCmdQueue.csrvReportCmdStatus
                           (m_hReq,
                            sRet,
                            NULL,      // (our result data is in req pkt)
                            (void *) &m_req);   // (short lived var is ok)

   //  after the command queue op completes, we can safely dispose
   //  of our own instance.
   delete this;

   return;

}  /* end of CHotSwapCmdHandler::ReportErrorAndSelfDestruct */

//
//  CHotSwapCmdHandler::SetDdhDriveId (pDdhCtl)
//
//  Description:
//    Fills in the DDH / bay number parameters of the supplied message,
//    working from our instance's cached disk descriptor record.
//
//  Inputs:
//    pDdhCtl - Points to message which we're supposed to fill in.
//
//  Outputs:
//    *pDdhCtl - DDH index and drive bay number are populated.
//

void  CHotSwapCmdHandler::SetDdhDriveId (MsgCmbDdhControl * pDdhCtl)  const
{


   assert (pDdhCtl != NULL);
   assert (m_pDiskDescr != NULL);
   if ((pDdhCtl != NULL) && (m_pDiskDescr != NULL))
      {
      //  slot ID is a simple contiguous index, so we can decompose it
      //  into DDH id and relative bay number easily:
      pDdhCtl->m_Payload.eDdh = (TySlot) ((m_pDiskDescr->SlotID / 5) + CMB_DDH0);
      pDdhCtl->m_Payload.iBay = m_pDiskDescr->SlotID % 5;
      }

   return;

}  /* end of CHotSwapCmdHandler::SetDdhDriveId */

