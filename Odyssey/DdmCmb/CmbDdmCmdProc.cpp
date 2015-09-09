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
// File: CmbDdmCmdProc.cpp
//
// Description:
//    Card Management Bus interface DDM.
//
//    This file contains the member routines for processing commands sent
//    to our DDM via the CmdSender/CmdServer inter-DDM comm machinery.
//    See CDdmCMB::CmdReady() below for more info.
//
// $Log: /Gemini/Odyssey/DdmCmb/CmbDdmCmdProc.cpp $
// 
// 19    1/20/00 4:11p Eric_wedel
// Fixed callback target signatures.
// 
// 18    1/03/00 6:46p Ewedel
// Fixed CmbStateToIopStatus() so that all overloads clean up raw state
// value before conversion [least common denominator does it now].  Fix
// for jfl.
// 
// 17    12/14/99 8:04p Ewedel
// Updated CmbStateToIopStatus() to reflect current low-level CMB state
// defs.
// 
// 16    10/08/99 11:57a Ewedel
// Updated CDdmCMB::CmbStateToIopStatus() to properly map all currently
// defined states.
// 
// 14    9/03/99 5:13p Ewedel
// Removed poll-environment command request, it's now handled via PHS
// interface.
// 
// 13    8/24/99 8:06p Ewedel
// Changed m_iContigEvcN to m_aiContigEvc[], and fixed jfl TS interface
// bug.
// 
// 12    8/15/99 12:05p Jlane
// Added parameters for new TS interface changes.
// 
// 11    8/11/99 7:53p Ewedel
// Added a bunch more request processors.
// 
// 10    8/02/99 7:29p Ewedel
// Added environmental polling request hook (k_eCmbCtlPollEnv).
// 
// 9     7/23/99 1:27p Rkondapalli
// [ewx]  Fixed CDdmCMB::SetIopPtsStateAndReply() so that it updates CMB
// DDM's cached state info before sending PTS update (doing it in the PTS
// reply handler is too late!).
// 
// 8     7/20/99 4:57p Rkondapalli
// No more replies ffrom Send on PTS objects.
// 
// 7     7/15/99 4:15p Ewedel
// Updated for CmbPacket def changes.
// 
// 6     6/24/99 7:21p Ewedel
// Updated to latest CmdQueue interface, changed some constant names
// around, and updated for redefined CMB power control command.
// 
// 5     6/18/99 1:16p Ewedel
// Added funny little hack to power-on processing, so that power-on
// command will not report status to sender until the target IOP moves
// into the "awaiting boot" state.  This is a TEMPORARY hack.  Long-term,
// we'll get unsolicited status updates and pass them into the IOP Status
// table, and our power-on command will return as soon as it gets a reply
// from the target CMA.
// 
// 4     6/16/99 8:24p Ewedel
// Added ControlIopBoot support, and various tweaks to better support
// robust IOP state tracking.
// 
// 3     6/15/99 7:09p Ewedel
// Updated for latest command queue interface.
// 
// 2     6/15/99 12:31a Ewedel
// Updated to current queue interface level.
//
// 1     6/15/99 12:04a Ewedel
// First cut.
//
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CmbDdmCommands.h"

#include  "CtEvent.h"         // standard status codes

#include  "Fields.h"

#include  "Odyssey_Trace.h"

#include  <assert.h>



//  a little something for translating fan RPM to CMB packet encoding
static inline U8  CmbFanSpeedCode (U32 ulFanRpm)
      {
         return (0 * ulFanRpm);	//BUGBUG - conversion unknown!
      };

//  little helper for issuing prompts
void  IssuePrompt (const char *pszPromptFmt, ...);



//
//  CDdmCMB::CmdReady (hRequest, pRequest)
//
//  Description:
//    Called when CmdSender::csndrExecute() is used to send us a request.
//    We dispatch the request for appropriate processing.  After processing
//    completes (may span multiple DDM messages) we issue a suitable reply.
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    CDdmCMB::CmdReady - Returns CTS_SUCCESS (will become void later on).
//

void  CDdmCMB::CmdReady (HANDLE hRequest, CmbCtlRequest *pRequest)
{

U32               iContigSlot;
CDdmCmdCarrier  * pReqHolder;


//   Tracef("CDdmCMB::CmdReady(), result = %d\n", ulStatus);

   assert (pRequest != NULL);

//   //  what we do to report status is call
//   m_CmdServer.csrvReportStatus (hRequest, SQ_COMMAND_STATUS, CTS_SUCCESS,
//                                 NULL,    // (our result data is in req pkt)
//                                 pRequest);  // pRequest can have short life


   if ((pRequest->eCommand == k_eCmbCtlDriveLockCtl) ||
       (pRequest->eCommand == k_eCmbCtlDriveBypassCtl))
      {
      //  got a command destined for a DDH -- no generic params here.
      iContigSlot = 0;
      }
   else
      {
      //  got a command destined for one (or more) IOPs.

      //  figure out our ContigSlot value for the target IOP
      if (! ValidIopSlot (pRequest->eSlot, iContigSlot))
         {
         Tracef ("CDdmCMB::CmdReady: invalid slot ID in row record: %d\n",
                 pRequest->eSlot);

         //  report failure right now.
         m_CmdServer.csrvReportCmdStatus (hRequest, 
                                          CTS_CMB_INVALID_PARAMETER,
                                          NULL,    // (our result data is in req pkt)
                                          pRequest);  // pRequest can have short life

         return;
         }
      //  got target IOP's contig slot, now do what we're told
      }

   //  ok, build up a carrier for tracking this request
   pReqHolder = new CDdmCmdCarrier (iContigSlot, hRequest, *pRequest);

   switch (pRequest->eCommand)
      {
      case k_eCmbCtlPower:
         // params are CCmbCtlPower
         ControlIopPower (pReqHolder);
         break;

      case k_eCmbCtlPciWindow:
         // params are CCmbCtlPciWindow
         ControlIopPciWindow (pReqHolder);
         break;

      case k_eCmbCtlCmbCtlBoot:
         // params are CCmbCtlBoot
         ControlIopBoot (pReqHolder);
         break;

      case k_eCmbCtlIopLockCtl:
         // params are CCmbCtlIopLock
         ControlIopLock (pReqHolder);
         break;

      case k_eCmbCtlDriveLockCtl:
         // params are CCmbCtlDriveLock
         ControlDriveBayLock (pReqHolder);
         break;

      case k_eCmbCtlDriveBypassCtl:
         // params are CCmbCtlDriveBypass
         ControlDriveBypass (pReqHolder);
         break;

      case k_eCmbCtlFanSpeed:
         // params are CCmbCtlSetFanSpeed
         ControlFanSpeed (pReqHolder);
         break;

      case k_eCmbCtlOneChassisSN:
         // params are CCmbCtlSetChassisSN
         ControlOneChassisSN (pReqHolder);
         break;

      case k_eCmbCtlPciBusAccess:
         // params are CCmbCtlPciBusEnable
         ControlPciBusAccess (pReqHolder);
         break;

      case k_eCmbCtlSpiResetEnable:
         // params are CCmbCtlSpiResetEnable
         ControlSpiResetEnable (pReqHolder);
         break;

      case k_eCmbCtlHbcMaster:
         // params are CCmbCtlHbcMaster
         ControlHbcMaster (pReqHolder);
         break;

      default:
         Tracef ("CDdmCMB::CmdReady: unknown command code %d\n",
                 pRequest->eCommand);

         //  dispose of unneeded request cookie
         delete pReqHolder;

         //  report unknown command code to sender
         m_CmdServer.csrvReportCmdStatus (hRequest,
                                          CTS_CMB_INVALID_PARAMETER,
                                          NULL,
                                          pRequest);
      }

   //  for now, we return happiness.  Later, we'll have no return value at all.
   return;

}  /* end of CDdmCMB::CmdReady */

//
//  CDdmCMB::ControlIopPower (pCmd)
//
//  Description:
//    Called when we receive CMB DDM command packet for controlling
//    power to an IOP's MIPS CPU.
//
//    We take care of tweaking power at the IOP, updating the IOP's
//    state in the IOP Status table, and replying to the command
//    packet.
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlIopPower (CDdmCmdCarrier *pCmd)
{

char  achPrompt [128];


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlPower);

   //  behave according to the MIPS CPU's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_UNKNOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_UNRESPONSIVE);
         break;

      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
         break;

      case IOPS_POWERED_DOWN:
         if (pCmd->m_Req.u.Power.bPowerOn)
            {
            //  caller wants us to power IOP on, so let's do so

            if (m_CmbHwIntf.IsRealCmbPresent())
               {
               //  got real CMB hardware, so send a message to it
               SendCmbMsg (pCmd, k_eCmbCmdPowerControl, pCmd->m_Req.eSlot,
                           CMBCALLBACK (CDdmCMB, ControlIopPower2),
                           1, k_eCmbPowerOn);

               //  [callback is invoked even in error cases, and takes
               //   care of cleanup]
               }
            else
               {
               //  no CMB hardware, so fake it by asking user to flip a switch
               MakeSlotMessage (achPrompt, "Please turn on power to",
                                pCmd -> m_iContigSlot);
               IssuePrompt ("%s", achPrompt);

               //  powered up IOP, now update its state in PTS
               SetIopPtsStateAndReply (pCmd, IOPS_POWERED_ON);

               //  and report success
               ReportCmdStatus (pCmd, CTS_SUCCESS);
               }
            }
         else
            {
            //  "IOP" already powered down, so do nothing but be happy
            ReportCmdStatus (pCmd, CTS_SUCCESS);
            }
         break;

      default:
         //  IOP is in some powered-up condition

         if (pCmd->m_Req.u.Power.bPowerOn)
            {
            //  caller wants us to power it on, so just be happy
            ReportCmdStatus (pCmd, CTS_SUCCESS);
            }
         else
            {
            //  need to power down IOP, let's do it
            if (m_CmbHwIntf.IsRealCmbPresent())
               {
               //  got real CMB hardware, so send a message to it
               SendCmbMsg (pCmd, k_eCmbCmdPowerControl, pCmd->m_Req.eSlot,
                           CMBCALLBACK (CDdmCMB, ControlIopPower2),
                           1, k_eCmbPowerOff);

               //  [callback is invoked even in error cases, and takes
               //   care of cleanup]
               }
            else
               {
               //  no CMB hardware, so fake it by asking user to flip a switch
               MakeSlotMessage (achPrompt, "Please turn off power to",
                                pCmd -> m_iContigSlot);
               IssuePrompt ("%s", achPrompt);

               //  powered down IOP, now update its state in PTS
               SetIopPtsStateAndReply (pCmd, IOPS_POWERED_DOWN);
               }
            }

      }

   return;

}  /* end of CDdmCMB::ControlIopPower */


//  We're called back with the results of sending a "power control" command
//  to some IOP's CMA.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for this request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.
//

void  CDdmCMB::ControlIopPower2 (void *pvCmd, STATUS ulStatus,
                                 const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;
IopState          eState;


   //  digest NAK packet, if received
   if (ulStatus == CTS_SUCCESS)
      {
      //  got a reply packet, so map its status
      CmbReplyToStatus (pReply, ulStatus);
      }

   if (ulStatus == CTS_SUCCESS)
      {
      //  the command went through, so update PTS for the IOP
      //  (this is our initial powered-up state)

      //  convert CMB state to IOP Status table state
      CmbStateToIopStatus (pReply, eState);

//*disable kludge code, as full-path update via unsolicited notifies
//*seems to now be working.
//*      //BUGBUG - here we introduce a little kludge, to facilitate the
//*      //  initial "cmb boot" demo.  Rather than simply returning the
//*      //  IOP's current state, if we powered it on we also wait until
//*      //  the IOP indicates that it is in "awaiting boot" state.
//*      if ((pCmd->m_Req.eCommand == k_eCmbCtlPower) &&
//*          (pCmd->m_Req.u.Power.bPowerOn) &&
//*          (eState != IOPS_AWAITING_BOOT))
//*         {
//*         //  yup, run our funny little kludge
//*         ControlIopPower3 (pCmd, ulStatus, pReply);
//*         }
//*      else
         {
         //  do normal PTS state update, reply & cleanup
         SetIopPtsStateAndReply (pCmd, eState);
         }
      }
   else
      {
      //  whoops, CMB command didn't go through, report error to requestor
      ReportCmdStatus (pCmd, ulStatus);
      }

   return;

}  /* end of CDdmCMB::ControlIopPower2 */


//  We're called back with the results of sending either "power control"
//  command or a "slot poll" command to the target IOP's CMA.  If the
//  resulting IOP MIPS state is not yet "awaiting boot", then we sleep
//  for a little while and send another poll to the CMA.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for the original
//                "power on" request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.
//

void  CDdmCMB::ControlIopPower3 (void *pvCmd, STATUS ulStatus,
                                 const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;
IopState          eState;


   //  digest NAK packet, if received
   if (ulStatus == CTS_SUCCESS)
      {
      //  got a reply packet, so map its status
      CmbReplyToStatus (pReply, ulStatus);
      }

   if (ulStatus == CTS_SUCCESS)
      {
      //  the command went through, so see if we've got the desired state

      //  convert CMB state to IOP Status table state
      CmbStateToIopStatus (pReply, eState);

Tracef ("CDdmCMB::ControlIopPower3: state is %d, we want %d\n", eState,
        IOPS_AWAITING_BOOT);

      if (eState != IOPS_AWAITING_BOOT)
         {
         //  not there yet, so do another poll

         //  Note that it should be safe to call NU_Sleep here, since we
         //  are running on our CmbDdm thread, processing a signal sent
         //  to us (or our CMB Hw Intf code).  We'll put our whole DDM
         //  to sleep, but so what?  This is only a hack.
         NU_Sleep (100);      // 1 second: per JoeA, 1 tick = 10ms
                              // (at least on most board types)

         //  now do another direct status poll
         SendCmbMsg (pCmd, k_eCmbCmdStatusPoll, pCmd->m_Req.eSlot,
                     CMBCALLBACK (CDdmCMB, ControlIopPower3));
         }
      else
         {
         //  got our target IOP state, so do normal PTS state update,
         //  reply & cleanup
         SetIopPtsStateAndReply (pCmd, eState);
         }
      }
   else
      {
      //  whoops, CMB command didn't go through, report error to requestor
      ReportCmdStatus (pCmd, ulStatus);
      }

   return;

}  /* end of CDdmCMB::ControlIopPower3 */

//
//  CDdmCMB::ControlIopPciWindow (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    PCI window parameters for an IOP board.
//
//    We verify that the target IOP should be in a receptive state
//    and then send along the parameters to the IOP.
//    We defer our response until we receive a response from the IOP
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlIopPciWindow (CDdmCmdCarrier *pCmd)
{

CCmbMsgSender   * pCmbMsg;
char              achPrompt [128];


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlPciWindow);
   assert (pCmd->m_Req.u.PciWindow.ulPciWinSize != 0);

   //  behave according to the MIPS CPU's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_UNKNOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_UNRESPONSIVE);
         break;

      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
         break;

      case IOPS_POWERED_DOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_POWERED_DOWN);
         break;

      case IOPS_POWERED_ON:
         //  IOP is freshly powered on, and has not yet announced that
         //  it is ready for PCI window data -- it will signal this
         //  by setting state "IOPS_AWAITING_BOOT"
         ReportCmdStatus (pCmd, CTS_CMB_IOP_MIPS_NOT_READY);
         break;

      case IOPS_AWAITING_BOOT:
      default:
         //  IOP is in some powered-up condition; we don't know if it
         //  is really ready to receive PCI window command, but it's
         //  close enough.

         if (m_CmbHwIntf.IsRealCmbPresent())
            {
            //  got real CMB hardware

            //  build up message to send (we must do this "by hand", as it
            //  exceeds the ability of our simplified SendCmbMsg() helper
            pCmbMsg = new CCmbMsgSender (this, k_eCmbCmdSetPciWindow,
                                         pCmd->m_Req.eSlot | CmbAddrMips,
                                         3 * sizeof (U32));
            pCmbMsg -> AddParam (0, pCmd->m_Req.u.PciWindow.ulPciWinSize);
            pCmbMsg -> AddParam (4, pCmd->m_Req.u.PciWindow.ulPciWinPciBase);
            pCmbMsg -> AddParam (8, pCmd->m_Req.u.PciWindow.ulPciWinIopBase);

            //  and send command off to CMB
            pCmbMsg -> Send (m_CmbHwIntf, pCmd,
                             CMBCALLBACK (CDdmCMB, ReportControlResult));

            //  [callback is invoked even in error cases, and takes
            //   care of cleanup]
            }
         else
            {
            //  no CMB hardware, so just report what we would have done
            //  on the console
            MakeSlotMessage (achPrompt, "Setting PCI window for",
                             pCmd -> m_iContigSlot);
            Tracef ("%s\n   size/pci base/iop base:  0x%08X / 0x%08X / 0x%08X\n",
                    achPrompt, 
                    pCmd->m_Req.u.PciWindow.ulPciWinSize,
                    pCmd->m_Req.u.PciWindow.ulPciWinPciBase,
                    pCmd->m_Req.u.PciWindow.ulPciWinIopBase);

            //  and report success
            ReportCmdStatus (pCmd, CTS_SUCCESS);
            }
         break;
      }

   return;

}  /* end of CDdmCMB::ControlIopPciWindow */

//
//  CDdmCMB::ControlIopBoot (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    boot parameters for an IOP board's MIPS CPU.
//
//    We verify that the target IOP should be in a receptive state
//    and then send along the parameters to the IOP.
//    We defer our response until we receive a response from the IOP
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlIopBoot (CDdmCmdCarrier *pCmd)
{

CCmbMsgSender   * pCmbMsg;
char              achPrompt [128];


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlCmbCtlBoot);
   assert ((pCmd->m_Req.u.Boot.eAction == CCmbCtlBoot::k_eDiagnostic) ||
           (pCmd->m_Req.u.Boot.eAction == CCmbCtlBoot::k_ePCI) ||
           (pCmd->m_Req.u.Boot.eAction == CCmbCtlBoot::k_eCrashDump));

   //  behave according to the MIPS CPU's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_UNKNOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_UNRESPONSIVE);
         break;

      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
         break;

      case IOPS_POWERED_DOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_POWERED_DOWN);
         break;

      case IOPS_POWERED_ON:
         //  IOP is freshly powered on, and has not yet announced that
         //  it is ready for PCI window data -- it will signal this
         //  by setting state "IOPS_AWAITING_BOOT"
         ReportCmdStatus (pCmd, CTS_CMB_IOP_MIPS_NOT_READY);
         break;

      case IOPS_AWAITING_BOOT:
      default:
         //  IOP is in some powered-up condition; we don't know if it
         //  is really ready to receive boot command, but it's
         //  close enough.

         if (m_CmbHwIntf.IsRealCmbPresent())
            {
            //  got real CMB hardware

            //  build up message to send (we must do this "by hand", as it
            //  exceeds the ability of our simplified SendCmbMsg() helper
            pCmbMsg = new CCmbMsgSender (this, k_eCmbCmdSetBootParams,
                                         pCmd->m_Req.eSlot | CmbAddrMips,
                                         2 * sizeof (U32) + sizeof (U8));
            pCmbMsg -> AddParam (0, (U8) pCmd->m_Req.u.Boot.eAction);
            pCmbMsg -> AddParam (1, pCmd->m_Req.u.Boot.ulImageOffset);
            pCmbMsg -> AddParam (5, pCmd->m_Req.u.Boot.ulParamOffset);

            //  and send command off to CMB
            pCmbMsg -> Send (m_CmbHwIntf, pCmd,
                             CMBCALLBACK (CDdmCMB, ReportControlResult));

            //  [callback is invoked even in error cases, and takes
            //   care of cleanup]
            }
         else
            {
            //  no CMB hardware, so just report what we would have done
            //  on the console
            MakeSlotMessage (achPrompt, "Setting boot params for",
                             pCmd -> m_iContigSlot);
            Tracef ("%s\n   action = %d, offImage = %08X, offParams = %08X\n",
                    achPrompt, 
                    pCmd->m_Req.u.Boot.eAction,
                    pCmd->m_Req.u.Boot.ulImageOffset,
                    pCmd->m_Req.u.Boot.ulParamOffset);

            //  and report success
            ReportCmdStatus (pCmd, CTS_SUCCESS);
            }
         break;
      }

   return;

}  /* end of CDdmCMB::ControlIopBoot */

//
//  CDdmCMB::ControlIopLock (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    that an IOP's locking solenoid should be engaged or released.
//
//    We verify that the operation is legal for the IOP in its
//    present state and, if so, we perform it.
//
//    We defer our response until we receive a response from the IOP
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlIopLock (CDdmCmdCarrier *pCmd)
{


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlIopLockCtl);
   assert ((pCmd->m_Req.u.IopLock.eAction == CCmbCtlIopLock::k_eLockIop) ||
           (pCmd->m_Req.u.IopLock.eAction == CCmbCtlIopLock::k_eUnlockIop));

   //  this code just won't tolerate a visibly non-CMB environment:
   assert (m_CmbHwIntf.IsRealCmbPresent());


   //  Note that we generally don't respect the IOP's state, except
   //  for a very specific inhibit of unlocks sent to IOPs flagged
   //  as running OS-level code.
   //  We deliberately don't check state too much, in case the IOP
   //  is unable to properly report its own state.  It would be rather
   //  silly to be unable to unlock an IOP (in order to replace it)
   //  because the IOP's state-reporting machinery was broken.

   switch (pCmd->m_Req.u.IopLock.eAction)
      {
      case CCmbCtlIopLock::k_eLockIop:
         //  locking is permitted for an IOP in any MIPS state..
         SendCmbMsg (pCmd, k_eCmbCmdIopSlotLockCtl, pCmd->m_Req.eSlot,
                     CMBCALLBACK (CDdmCMB, ReportControlResult),
                     1, k_eCmbLockEnable);
         break;

      case CCmbCtlIopLock::k_eUnlockIop:
         //  is IOP running?
         if (m_aIopStatusImage[pCmd->m_iContigSlot].eState == IOPS_OPERATING)
            {
            //  unlocking is not permitted for a running IOP:
            ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_IN_USE);
            }
         else
            {
            //  not running OS-level code, so enable IOP card extraction
            SendCmbMsg (pCmd, k_eCmbCmdIopSlotLockCtl, pCmd->m_Req.eSlot,
                        CMBCALLBACK (CDdmCMB, ReportControlResult),
                        1, k_eCmbLockDisable);
            }

      default:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_IN_USE);
      }

   return;

}  /* end of CDdmCMB::ControlIopLock */

//
//  CDdmCMB::ControlDriveBayLock (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    whether a given drive bay should have its locking solenoid
//    enabled or not.
//
//    We verify that the target DDH should be in a receptive state
//    and then send the request off to it.
//    We defer our response until we receive a response from the DDH
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlDriveBayLock (CDdmCmdCarrier *pCmd)
{

U32      iDdhBay;          // zero-based DDH-relative bay number
STATUS   sRet;


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlDriveLockCtl);
   assert ((pCmd->m_Req.u.DriveLock.eAction == CCmbCtlDriveLock::k_eLockDrive) ||
           (pCmd->m_Req.u.DriveLock.eAction == CCmbCtlDriveLock::k_eUnlockDrive));

   assert (m_CmbHwIntf.IsRealCmbPresent());


   //  figure out drive bay's indices

   sRet = ParseDriveBayId (pCmd->m_Req.u.DriveLock.ulDriveBay,
                           iDdhBay, pCmd->m_Req.eSlot, pCmd->m_iContigSlot);

   assert ((pCmd->m_Req.u.DriveLock.ulDriveBay & ~0x1F) == 0);
   if (sRet != CTS_SUCCESS)
      {
      //  whoops, invalid drive bay ID
      ReportCmdStatus (pCmd, sRet);
      return;
      }

   //  ok, behave according to the DDH's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_DDH_NOT_PRESENT);
         break;

      case IOPS_POWERED_ON:
         //  this is the normal operating state for a DDH, so let's
         //  send caller's the request.
         SendCmbMsg (pCmd, k_eCmbCmdSetDriveLock, pCmd->m_Req.eSlot,
                     CMBCALLBACK (CDdmCMB, ReportControlResult),
                     1,
                     (pCmd->m_Req.u.DriveLock.eAction ==
                                       CCmbCtlDriveLock::k_eLockDrive) ?
                                    k_eCmbLockEnable : k_eCmbLockDisable);
         break;

      default:
         //  all others report as "DDH not usable"
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_DDH_NOT_USABLE);
      }

   return;

}  /* end of CDdmCMB::ControlDriveBayLock */

//
//  CDdmCMB::ControlDriveBypass (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    whether a given drive should be bypassed at the DDH, which
//    would effectively isolate the drive from the system.
//
//    We verify that the target DDH should be in a receptive state
//    and then send the request off to it.
//    We defer our response until we receive a response from the DDH
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlDriveBypass (CDdmCmdCarrier *pCmd)
{

U32      iDdhBay;          // zero-based DDH-relative bay number
STATUS   sRet;


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlDriveBypassCtl);
   assert ((pCmd->m_Req.u.DriveBypass.eAction ==
                                 CCmbCtlDriveBypass::k_eBypassDrive) ||
           (pCmd->m_Req.u.DriveBypass.eAction ==
                                 CCmbCtlDriveBypass::k_eEnableDrive));

   assert (m_CmbHwIntf.IsRealCmbPresent());

   //  figure out drive bay's indices

   sRet = ParseDriveBayId (pCmd->m_Req.u.DriveLock.ulDriveBay,
                           iDdhBay, pCmd->m_Req.eSlot, pCmd->m_iContigSlot);

   if (sRet != CTS_SUCCESS)
      {
      //  whoops, invalid drive bay ID
      ReportCmdStatus (pCmd, sRet);
      return;
      }

   //  ok, behave according to the DDH's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_DDH_NOT_PRESENT);
         break;

      case IOPS_POWERED_ON:
         //  this is the normal operating state for a DDH, so let's
         //  send caller's request.
         SendCmbMsg (pCmd, k_eCmbCmdSetPortBypass, pCmd->m_Req.eSlot,
                     CMBCALLBACK (CDdmCMB, ReportControlResult),
                     1,
                     (pCmd->m_Req.u.DriveBypass.eAction ==
                                       CCmbCtlDriveBypass::k_eBypassDrive) ?
                                    k_eCmbDriveBypass : k_eCmbDriveEnable);
         break;

      default:
         //  all others report as "DDH not usable"
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_DDH_NOT_USABLE);
      }

   return;

}  /* end of CDdmCMB::ControlDriveBypass */

//
//  CDdmCMB::ControlFanSpeed (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    the desired main system intake / exhaust fan speeds.
//
//    We do our best to push the target settings to both EVCs,
//    if two are operating.
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlFanSpeed (CDdmCmdCarrier *pCmd)
{

const U32   ulFanUpperLimit  =  2000;     // max permissible RPMs


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlFanSpeed);

   //  verify that target fan speeds are legal
   if ((pCmd->m_Req.u.FanSpeed.ulIntake  > ulFanUpperLimit) ||
       (pCmd->m_Req.u.FanSpeed.ulExhaust > ulFanUpperLimit) ||
       ((! pCmd->m_Req.u.FanSpeed.fUpdateIntake) &&
        (! pCmd->m_Req.u.FanSpeed.fUpdateExhaust)))
      {
      //  whoopsie, invalid param[s]
      ReportCmdStatus (pCmd, CTS_CMB_INVALID_PARAMETER);
      return;
      }

   assert (m_CmbHwIntf.IsRealCmbPresent());

   //  send off intake set command to first EVC, if present

   if ((m_aIopStatusImage[m_aiContigEvc[0]].eState == IOPS_POWERED_ON) &&
       pCmd->m_Req.u.FanSpeed.fUpdateIntake)
      {
      //  EVC 0 is present, and we're to update intake fan speed:
      SendCmbMsg (pCmd, k_eCmbCmdSetFanSpeed, CMB_EVC0,
                  CMBCALLBACK (CDdmCMB, ControlFanSpeed2),
                  2, k_eCmbFanPairIntake,
                  CmbFanSpeedCode (pCmd->m_Req.u.FanSpeed.ulIntake));
      }
   else
      {
      //  no available EVC0, so claim this set cmd went ok
      ControlFanSpeed2 (pCmd, CTS_SUCCESS, NULL);
      }

   return;

}  /* end of CDdmCMB::ControlFanSpeed */


//  we're called back in response to the first fan speed set command
//  sent to EVC 0.  We continue with the second fan speed, etc.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for this request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.  NULL if we didn't talk to the CMB;
//             NULL does not necessarily imply an error here.
//

void  CDdmCMB::ControlFanSpeed2 (void *pvCmd, STATUS ulStatus,
                                 const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;


   if ((ulStatus == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, ulStatus);
      }

   if (ulStatus != CTS_SUCCESS)
      {
      //BUGBUG - should log this, somehow.
      Tracef ("CDdmCMB::ControlFanSpeed2: failed to set EVC0 intake speed, "
              "ret = %X\n",  ulStatus);
      }

   //  set EVC1's intake speed

   if ((m_aIopStatusImage[m_aiContigEvc[1]].eState == IOPS_POWERED_ON) &&
       pCmd->m_Req.u.FanSpeed.fUpdateIntake)
      {
      //  EVC 1 is present, and we're to update intake fan speed:
      SendCmbMsg (pCmd, k_eCmbCmdSetFanSpeed, CMB_EVC1,
                  CMBCALLBACK (CDdmCMB, ControlFanSpeed3),
                  2, k_eCmbFanPairIntake,
                  CmbFanSpeedCode (pCmd->m_Req.u.FanSpeed.ulIntake));
      }
   else
      {
      //  no available EVC0, so claim this set cmd went ok
      ControlFanSpeed3 (pCmd, CTS_SUCCESS, NULL);
      }

   return;

}  /* end of CDdmCMB::ControlFanSpeed2 */


//  we're called back in response to the first fan speed set command
//  sent to EVC 1.  We continue with the next fan speed, etc.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for this request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.  NULL if we didn't talk to the CMB;
//             NULL does not necessarily imply an error here.
//

void  CDdmCMB::ControlFanSpeed3 (void *pvCmd, STATUS ulStatus,
                                 const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;


   if ((ulStatus == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, ulStatus);
      }

   if (ulStatus != CTS_SUCCESS)
      {
      //BUGBUG - should log this, somehow.
      Tracef ("CDdmCMB::ControlFanSpeed3: failed to set EVC1 intake speed, "
              "ret = %X\n",  ulStatus);
      }

   //  set EVC0's exhaust speed

   if ((m_aIopStatusImage[m_aiContigEvc[0]].eState == IOPS_POWERED_ON) &&
       pCmd->m_Req.u.FanSpeed.fUpdateExhaust)
      {
      //  EVC 0 is present, and we're to update exhaust fan speed:
      SendCmbMsg (pCmd, k_eCmbCmdSetFanSpeed, CMB_EVC0,
                  CMBCALLBACK (CDdmCMB, ControlFanSpeed4),
                  2, k_eCmbFanPairExhaust,
                  CmbFanSpeedCode (pCmd->m_Req.u.FanSpeed.ulExhaust));
      }
   else
      {
      //  no available EVC0, so claim this set cmd went ok
      ControlFanSpeed4 (pCmd, CTS_SUCCESS, NULL);
      }

   return;

}  /* end of CDdmCMB::ControlFanSpeed3 */


//  we're called back in response to the second fan speed set command
//  sent to EVC 0.  We continue with the next fan speed, etc.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for this request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.  NULL if we didn't talk to the CMB;
//             NULL does not necessarily imply an error here.
//

void  CDdmCMB::ControlFanSpeed4 (void *pvCmd, STATUS ulStatus,
                                 const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;


   if ((ulStatus == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, ulStatus);
      }

   if (ulStatus != CTS_SUCCESS)
      {
      //BUGBUG - should log this, somehow.
      Tracef ("CDdmCMB::ControlFanSpeed4: failed to set EVC0 exhaust speed, "
              "ret = %X\n",  ulStatus);
      }

   //  set EVC1's exhaust speed

   if ((m_aIopStatusImage[m_aiContigEvc[1]].eState == IOPS_POWERED_ON) &&
       pCmd->m_Req.u.FanSpeed.fUpdateExhaust)
      {
      //  EVC 1 is present, and we're to update exhaust fan speed:
      SendCmbMsg (pCmd, k_eCmbCmdSetFanSpeed, CMB_EVC1,
                  CMBCALLBACK (CDdmCMB, ReportControlResult),
                  2, k_eCmbFanPairExhaust,
                  CmbFanSpeedCode (pCmd->m_Req.u.FanSpeed.ulExhaust));
      }
   else
      {
      //  no available EVC1, so claim this set cmd went ok
      ReportControlResult (pCmd, CTS_SUCCESS, NULL);
      }

   return;

}  /* end of CDdmCMB::ControlFanSpeed4 */

//
//  CDdmCMB::ControlOneChassisSN (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    the new chassis serial number for a given IOP/EVC/DDH.
//
//    We verify that the target CMA should be in a receptive state
//    and then send along the parameters to it.
//    We defer our response until we receive a response from the CMA
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlOneChassisSN (CDdmCmdCarrier *pCmd)
{

CCmbMsgSender   * pCmbMsg;


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlOneChassisSN);

   //  behave according to the MIPS CPU's current state
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_UNKNOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_UNRESPONSIVE);
         break;

      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
         break;

      case IOPS_POWERED_DOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_POWERED_DOWN);
         break;

      case IOPS_POWERED_ON:
      default:
         //  IOP is in some powered-up condition; it should be fine
         //  for receiving a new chassis serial number.

         assert (m_CmbHwIntf.IsRealCmbPresent());

         //  build up message to send (we must do this "by hand", as it
         //  exceeds the ability of our simplified SendCmbMsg() helper
         pCmbMsg = new CCmbMsgSender (this, k_eCmbCtlOneChassisSN,
                                      pCmd->m_Req.eSlot,
                                      sizeof (pCmd->m_Req.u.ChassisSN.strSerialNumber));
         memcpy (pCmbMsg->PacketPtr()->abTail,
                 pCmd->m_Req.u.ChassisSN.strSerialNumber,
                 sizeof (pCmd->m_Req.u.ChassisSN.strSerialNumber));

         //  and send command off to CMB
         pCmbMsg -> Send (m_CmbHwIntf, pCmd,
                          CMBCALLBACK (CDdmCMB, ReportControlResult));

         //  [callback is invoked even in error cases, and takes
         //   care of cleanup]
      }

   return;

}  /* end of CDdmCMB::ControlOneChassisSN */

//
//  CDdmCMB::ControlPciBusAccess (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet selecting
//    whether an IOP should be able to access the PCI bus.
//
//    What this packet does is enable or disable the "quickswitches"
//    used to isolate each IOP from its attached PCI bus(es).
//
//    We verify that the target CMA should be in a receptive state
//    and then send along the parameters to it.
//    We defer our response until we receive a response from the CMA
//    (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlPciBusAccess (CDdmCmdCarrier *pCmd)
{


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlPciBusAccess);
   assert ((pCmd->m_Req.u.PciBus.eAction ==
                        CCmbCtlPciBusEnable::k_eCtlPciBusEnable) ||
           (pCmd->m_Req.u.PciBus.eAction ==
                        CCmbCtlPciBusEnable::k_eCtlPciBusIsolate));

   //  behave according to the target CMA's current state:
   switch (m_aIopStatusImage[pCmd->m_iContigSlot].eState)
      {
      case IOPS_UNKNOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_UNRESPONSIVE);
         break;

      case IOPS_EMPTY:
      case IOPS_BLANK:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
         break;

      case IOPS_POWERED_DOWN:
         ReportCmdStatus (pCmd, CTS_CMB_REQUESTED_IOP_POWERED_DOWN);
         break;

      case IOPS_POWERED_ON:
      default:
         //  IOP is in some powered-up condition; it should be safe
         //  to tinker with its PCI bus access

         assert (m_CmbHwIntf.IsRealCmbPresent());

         //  build up message to send
         SendCmbMsg (pCmd, k_eCmbCmdPciBusAccessCtl, pCmd->m_Req.eSlot,
                     CMBCALLBACK (CDdmCMB, ReportControlResult),
                     1, (pCmd->m_Req.u.PciBus.eAction ==
                                    CCmbCtlPciBusEnable::k_eCtlPciBusEnable) ?
                              k_eCmbPciAccessEnable : k_eCmbPciAccessIsolate);

         //  [callback is invoked even in error cases, and takes
         //   care of cleanup]
         break;
      }

   return;

}  /* end of CDdmCMB::ControlPciBusAccess */

//
//  CDdmCMB::ControlSpiResetEnable (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet selecting
//    whether our current HBC should be able to assert reset to
//    one or more AVR devices.
//
//    This is a little funky:  this packet requests that our local
//    AVR enable our use of the reset lines.  But the MIPS CPU must
//    then access a MIPS-writable register to actually assert the
//    SPI resets.
//
//    As always, we defer our response until we receive a response
//    from the CMA (via CMB, of course).
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlSpiResetEnable (CDdmCmdCarrier *pCmd)
{


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCtlSpiResetEnable);
   assert ((pCmd->m_Req.u.SpiResetEnable.eAction ==
                              CCmbCtlSpiResetEnable::k_eCtlSpiEnableReset) ||
           (pCmd->m_Req.u.SpiResetEnable.eAction ==
                              CCmbCtlSpiResetEnable::k_eCtlSpiDisableReset));

   //  this command always goes to our local CMA:

   assert (m_CmbHwIntf.IsRealCmbPresent());

   //  build up message to send
   SendCmbMsg (pCmd, k_eCmbCmdSpiResetEnable, CMB_SELF,
               CMBCALLBACK (CDdmCMB, ReportControlResult),
               1, (pCmd->m_Req.u.SpiResetEnable.eAction ==
                              CCmbCtlSpiResetEnable::k_eCtlSpiEnableReset) ?
                     k_eCmbSpiResetEnable : k_eCmbSpiResetDisable);

   //  [callback is invoked even in error cases, and takes
   //   care of cleanup]

   return;

}  /* end of CDdmCMB::ControlSpiResetEnable */
//
//  CDdmCMB::ControlHbcMaster (pCmd)
//
//  Description:
//    Called when we receive a CMB DDM command packet specifying
//    that we should change HBC mastery over to the other HBC.
//
//    We forward the request to the EVC(s) for processing.
//    As always, we don't return until we see a response.  In this
//    case, the response of interest is a change in state of the
//    "HBC master" line made available to us via the "get board info"
//    CMB query.
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to process.
//
//  Outputs:
//    none
//

void  CDdmCMB::ControlHbcMaster (CDdmCmdCarrier *pCmd)
{


   assert (pCmd != NULL);
   assert (pCmd->m_Req.eCommand == k_eCmbCmdChangeHbcMaster);

   //  this command goes to one or more EVCs.  How do we know which one
   //  is dominant?  And how do we handle an HBC master switchover before
   //  we've sent a request to the second EVC?
   //  behave according to the target CMA's current state:

   //BUGBUG - do what here?
   //  (for now, report failure)
   ReportControlResult (pCmd, CTS_NOT_IMPLEMENTED, NULL);

   return;

}  /* end of CDdmCMB::ControlHbcMaster */


//
//  CDdmCMB::ReportControlResult (pCmd, ulStatus, pReply)
//
//  Description:
//    We're called back with the results of some CMB operation
//    initiated in response to a Control*() member.
//
//    We extract the response status, report it to the original
//    requestor, and generally tidy things up.
//
//    Note that, by definition, we do not report a result to the
//    requestor until after the CMB ack/nak has been received,
//    so that the requested CMB operation has completed before
//    the requestor sees a response.
//
//  Inputs:
//    pvCmd - Points to our command info carrier for this request.
//    ulStatus - Result of CMB request message send.
//    pReply - Reply from CMB.
//

void  CDdmCMB::ReportControlResult (void *pvCmd, STATUS ulStatus,
                                    const CmbPacket *pReply)
{

CDdmCmdCarrier  * pCmd  =  (CDdmCmdCarrier *) pvCmd;

#pragma unused(pReply)


   if ((ulStatus == CTS_SUCCESS) && (pReply != NULL))
      {
      //  got a reply packet, so map its status
      CmbReplyToStatus (pReply, ulStatus);
      }

   //  all we do is report the result of the CMB communication
   ReportCmdStatus (pCmd, ulStatus);

   return;

}  /* end of CDdmCMB::ReportControlResult */

//
//  CDdmCMB::SetIopPtsStateAndReply (pCmd, eIopState)
//
//  Description:
//    Called to update an IOP's "current state" field in the PTS IOP Status
//    table, and then send a reply back to the requestor which initiated
//    our present operation.
//
//    We are careful to always send some sort of status back to the
//    requestor, even if we can't update PTS.
//
//  Inputs:
//    iContigSlot - Contiguous slot index of IOP to update.
//    eIopState - New "current" state value to set for IOP.
//
//  Outputs:
//    none
//

void  CDdmCMB::SetIopPtsStateAndReply (CDdmCmdCarrier *pCmd,
                                       IopState eIopState)
{

TSModifyField      * pmsgPtsUpd;
STATUS               ulRet;


   //  put the new IOP State value someplace where it will persist
   //  for the duration of our PTS operation:
   pCmd->m_eIopState = eIopState;

   //  make basic "field update" message instance
   pmsgPtsUpd = new TSModifyField;
   assert (pmsgPtsUpd != NULL);

   //  build up our "field update" for changing IOP's state.  Note that we
   //  use our (carefully contrived) request cookie as the field update
   //  cookie also.
   ulRet = pmsgPtsUpd -> Initialize (this, CT_IOPST_TABLE_NAME,
                                     CT_PTS_RID_FIELD_NAME,
                                     &m_aIopStatusImage[pCmd->m_iContigSlot].rid,
                                     sizeof (m_aIopStatusImage[
                                                   pCmd->m_iContigSlot].rid),
                                     CT_IOPST_IOPCURRENTSTATE,
                                     &pCmd->m_eIopState,
                                     sizeof (pCmd->m_eIopState),
                                     1,            // modify just one row
                                     NULL,         // ignore rows-modified ret
                                     NULL, 0,      // ignore rowID return
                                     (pTSCallback_t) &SetIopPtsStateAndReply2,
                                     pCmd);

   assert (ulRet == CTS_SUCCESS);

   //  We also maintain local cached copies of some IOP Status values.
   //  While we might update our cache in SetIopPtsStateAndReply2(), this
   //  may be too late since we might process other messages from our
   //  queue in the interim (before SetIopPtsStateAndReply2() is called).
   //  So we update our cached data right here.
   //  *sigh*  Cache coherency with PTS is non-trivial.
   m_aIopStatusImage [pCmd->m_iContigSlot].eState = eIopState;

   //  send message off to do exciting things..
   pmsgPtsUpd -> Send ();
   return;

}  /* end of CDdmCMB::SetIopPtsStateAndReply */


void  CDdmCMB::SetIopPtsStateAndReply2 (void *pClientContext, STATUS ulStatus)
{

CDdmCmdCarrier  * pCmd;


   assert (pClientContext != NULL);
   
   pCmd = (CDdmCmdCarrier *) pClientContext;

   if (ulStatus != CTS_SUCCESS)
      {
      Tracef ("CDdmCMB::SetIopPtsStateAndReply2: IOP slot %d PTS cur state update"
              " failed, ret = %d\n",
              m_aeContigToSlot [pCmd->m_iContigSlot], ulStatus);
      }
   else
      {
      //  PTS updated ok.  We already updated our local cache, so we're done.
      }

   //  whatever happened, let requester know (also cleans up request)
   ReportCmdStatus (pCmd, ulStatus);

   return;

}  /* end of CDdmCMB::SetIopPtsStateAndReply2 */

//
//  CDdmCMB::ReportCmdStatus (pCmd, ulStatus)
//
//  Description:
//    A little helper for responding to a DDM command which we received
//    via the standard command queue machinery.
//
//    We take as input our customary request carrier object, and we
//    generate a reply using the supplied request packet and
//    status value.  We then delete the carrier object.
//
//  Inputs:
//    pCmd - Standard command carrier, containing the command
//             to reply to.
//    ulStatus - Status to report in response.
//
//  Outputs:
//    none
//

void  CDdmCMB::ReportCmdStatus (CDdmCmdCarrier *pCmd, STATUS ulStatus)
{


   assert (pCmd != NULL);

   m_CmdServer.csrvReportCmdStatus (pCmd->m_hRequest,
                                    ulStatus,
                                    NULL,    // (no separate result data)
                                    &pCmd->m_Req);

   delete pCmd;

   return;

}  /* end of CDdmCMB::ReportCmdStatus */

//
//  CDdmCMB::CmbStateToIopStatus (pCmbPkt, eState)
//
//  Description:
//    A little helper for mapping from the CMB packet's "STATUS" byte
//    "mips state" field into our DDM-ish IopState coding.
//
//  Inputs:
//    pCmbPkt - Points to packet containing CMB status byte to start from.
//
//  Outputs:
//    eState - Loaded with equivalent of pCmbPkt->bStatus etc.
//

void  CDdmCMB::CmbStateToIopStatus (const CmbPacket *pCmbPkt, IopState& eState)
{


   assert (pCmbPkt != NULL);

   //  run on our standard packet status field
   CmbStateToIopStatus (pCmbPkt->Hdr.bStatus, eState);

   return;

}  /* end of CDdmCMB::CmbStateToIopStatus(pCmbPkt, eState) */


void  CDdmCMB::CmbStateToIopStatus (U8 bCmbState, IopState& eState)
{


   //  strip off any high-order status bits:
   bCmbState &= CmbStatMipsState;

   switch (bCmbState)
      {
      case k_eCmbMStNotFound:
         //BUGBUG - we don't know whether to map to "empty" or "blank".
         //         For now, assume "blank" (i.e., filler panel in place)
         eState = IOPS_BLANK;
         break;

      case k_eCmbMStPoweredOff:
         eState = IOPS_POWERED_DOWN;
         break;

      case k_eCmbMStPoweredOn:
         eState = IOPS_POWERED_ON;
         break;

      case k_eCmbMStAwaitingBoot:
         eState = IOPS_AWAITING_BOOT;
         break;

      case k_eCmbMStRunningDiags:
      case k_eCmbMStAwaitingDiagCmd:
         eState = IOPS_DIAG_MODE;
         break;

      case k_eCmbMStBootingPCIImage:
         eState = IOPS_BOOTING;
         break;

      case k_eCmbMStLoadingOSImage:
         eState = IOPS_LOADING;
         break;

      case k_eCmbMStRunningOSImage:
         eState = IOPS_OPERATING;
         break;

      case k_eCmbMStOSImageSuspended:
         eState = IOPS_SUSPENDED;
         break;

      case k_eCmbMStImageCorrupt:
         eState = IOPS_IMAGE_CORRUPT;
         break;

      case k_eCmbMStInsufficientRam:
         eState = IOPS_INSUFFICIENT_RAM;
         break;

      case k_eCmbMStUpdatingBootRom:
         eState = IOPS_UPDATING_BOOT_ROM;
         break;

      case k_eCmbMStAttentionNeeded:
         //  this is a pseudo-state.  It is presently only reported by
         //  DDH and EVC CMAs.  So we can simply convert it to the
         //  "normal" state expected of these CMAs.  If we ever see this
         //  state, we will clear it, which will trigger an unsolicited
         //  status update.  Thus, if our guess here is wrong, it will
         //  at least be short-lived.  :-)
         eState = IOPS_POWERED_ON;
         break;

      default:
         Tracef ("CDdmCMB::CmbStateToIopStatus(): unknown CMB state: %d\n",
                 bCmbState);
         //  now fall through into k_eCmbMStUnknown case:

      case k_eCmbMStUnknown:
         eState = IOPS_UNKNOWN;
      }

   return;

}  /* end of CDdmCMB::CmbStateToIopStatus(bCmbState, eState) */

//
//  CDdmCMB::CmbReplyToStatus (pCmbPkt, sStatus) 
//
//  Description:
//    A little helper for mapping from a CMB response packet's "STATUS" byte
//    flags and (if appropriate) NAK code, into a standard STATUS event code.
//
//  Inputs:
//    pCmbPkt - Points to packet containing CMB status byte to start from.
//
//  Outputs:
//    sStatus - Loaded with equivalent value, in DDM-speak.
//

void  CDdmCMB::CmbReplyToStatus (const CmbPacket *pReply, STATUS& sStatus)
{


   assert (pReply != NULL);
   assert ((pReply->Hdr.bStatus & CmbStatCmd) == 0);

   if ((pReply->Hdr.bStatus & CmbStatAck) != 0)
      {
      //  cool, response signifies happiness
      sStatus = CTS_SUCCESS;
      }
   else
      {
      //  whoops, got a NAK packet, translate the NAK status byte.
      assert (pReply->Hdr.cbData >= 1);   // (may be extra; first is NAK status)

      switch (pReply->abTail[0])
         {
         case k_eCmbNRCmdUnknown:
            sStatus = CTS_CMB_CMA_UNKNOWN_CMD;
            break;

         case k_eCmbNRBadParam:
            sStatus = CTS_CMB_CMA_BAD_PARAM;
            break;

         case k_eCmbNRCmaPktPreempt:
            sStatus = CTS_CMB_CMA_BUSY_UNSOL;
            break;

         case k_eCmbNRCmaTimeout:
            sStatus = CTS_CMB_REQUESTED_IOP_UNRESPONSIVE;
            break;

         case k_eCmbNRMipsTimeout:
            sStatus = CTS_CMB_CMA_MIPS_TIMEOUT;
            break;

         default:
            sStatus = CTS_CMB_CMA_UNKNOWN_NAK;
         }
      }

   return;

}  /* end of CDdmCMB::CmbReplyToStatus */

//
//  CDdmCMB::ParseDriveBayId (ulRawDriveBay, iDdhBay, eDdhSlot, iDdhContigSlot)
//
//  Description:
//    A private helper routine for issuing a prompt string and waiting
//    for the user to respond.  We only accept a simple carriage-return
//    input, we don't do any command line stuff.
//
//  Inputs:
//    ulRawDriveBay - Standard Odyssey internal drive ID number.  This
//                is the drive's SCSI ID, as documented in
//                "Odyssey Hardware Parameters" (Odyssey_hw_params.doc).
//
//  Outputs:
//    iDdhBay - Set to zero-based index of drive bay with respect to
//                its controlling DDH.  I.e., this number is always
//                in the range of 0 . . 4.
//    eDdhSlot - CMB slot ID of DDH which controls bay ID ulRawDriveBay.
//    iDdhContigSlot - CMB DDM internal contiguous slot index corresponding
//                to eDdhSlot.  As returned by ValidIopSlot().
//    CDdmCMB::ParseDriveBayId - Returns CTS_SUCCESS if ulRawDriveBay is
//                a valid drive bay ID, else a descriptive error code.
//

STATUS  CDdmCMB::ParseDriveBayId (U32 ulRawDriveBay, U32& iDdhBay,
                                  TySlot& eDdhSlot, U32& iDdhContigSlot)
{

const U32   cDrivesPerDdh  =  5;
U32         iDdh;
BOOL        fRet;
STATUS      sMyRet;


   assert ((ulRawDriveBay & ~0x1F) == 0);

   //  which bay within DDH's range?
   iDdhBay = ulRawDriveBay & 0x7;

   assert ((0 <= iDdhBay) && (iDdhBay < cDrivesPerDdh));

   if (((ulRawDriveBay & ~0x1F) == 0) && (iDdhBay < cDrivesPerDdh))
      {
      //  which DDH are we asked to talk to?
      iDdh = (ulRawDriveBay >> 3) & 0x3;

      //  make DDH's CMB slot address
      eDdhSlot = (TySlot) (iDdh + CMB_DDH0);

      //  and map it to a contig index
      fRet = ValidIopSlot (eDdhSlot, iDdhContigSlot);
      assert (fRet);

      sMyRet = CTS_SUCCESS;
      }
   else
      {
      //  whoops, invalid drive bay #
      sMyRet = CTS_CMB_INVALID_PARAMETER;
      }

   return (sMyRet);

}  /* end of CDdmCMB::ParseDriveBayId */

//
//  IssuePrompt (pszPromptFmt, ...)
//
//  Description:
//    A private helper routine for issuing a prompt string and waiting
//    for the user to respond.  We only accept a simple carriage-return
//    input, we don't do any command line stuff.
//
//  Inputs:
//    pszPromptFmt - Prompt string, which may contain printf()-style
//                format characters.
//    ... - Any additional arguments required by *pszPromptFmt.
//
//  Outputs:
//    none
//

void  IssuePrompt (const char *pszPromptFmt, ...)
{

va_list  pArgs;


   //  grab our printf-ish arg tail
   va_start (pArgs, pszPromptFmt);

   //  issue prompt
   printf("\a* * *\a ");
   vprintf(pszPromptFmt, pArgs);
   printf(  "\n       (press <return> when ready)\n");

   //  dispose of arg tail
   va_end (pArgs);

   //  flush any type-ahead
   while (kbhit ())
      {
      getchar ();
      }

   //  now wait for a CR
   while (getch () != '\r')
      {}
   printf("\n");

   //  all done, pretend whatever we prompted for has happened.
   return;

}  /* end of IssuePrompt */

