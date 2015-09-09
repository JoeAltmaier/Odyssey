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
// File: ReqUpdateIopStatus.cpp
//
// Description:
//    CMB DDM module.  Contains member routines used for processing
//    a single IOP Status Update request message.
//
// $Log: /Gemini/Odyssey/DdmCmb/ReqUpdateIopStatus.cpp $
// 
// 14    1/20/00 4:13p Eric_wedel
// Fixed callback target signatures, and added DDH id patch.
// 
// 13    12/14/99 8:06p Ewedel
// Added proper hooks for "attention needed" pseudo-state.
// 
// 12    10/18/99 4:29p Ewedel
// Added check of "board bits" hardware register, to discriminate between
// "empty" slots and slots with filler panels in them.
// 
// 11    9/13/99 6:58p Agusev
// [ewx] Reenabled full IOP interrogation sequence, removed some assert()s
// which no longer apply, and added slot number to debug messages.
// 
// 10    9/11/99 7:43p Tnelson
// [ewx]  Added silly kludge to work around further AVR bugs when
// attempting to read board info from own board (HBC master AVR).
// 
// 9     9/10/99 5:36p Ewedel
// Fixed CDdmCMB::ReqUpdateIopStatus3() so that it should handle unusual
// state returned for own-slot properly.
// 
// 8     9/07/99 5:01p Jlane
// [ewx] Fixed board-type patch so that it only applies to IOPs, not EVCs
// or DDHs.  Also truncated IOP read sequence again, since EVC (02) can't
// respond properly to "get chassis serial #".
// 
// 7     9/07/99 1:16p Jlane
// Changed Tracef output of status to use hex and also commented out
// assert in ReqUpdateIopStatus4.
// 
// 6     9/03/99 5:33p Ewedel
// Modified to support updated IOPStatusRecord.
// 
// 5     8/25/99 6:20p Ewedel
// Removed old hack.  We now (once again) read the full complement of
// parameters from the target IOP.
// 
// 4     8/13/99 3:18p Ewedel
// Added major hack to suppress various query commands which confuse the
// AVR no end.  (Seem to crash it, in fact.)  TEMPORARY!
// 
// 3     8/13/99 2:28p Jlane
// [ewx] Added mask patch to strip off erroneous board type bits returned
// by AVR.
// 
// 2     8/13/99 10:29a Jlane
// [ewx] Enhanced robustness, add patches to work around incomplete AVR
// firmware.
// 
// 1     8/11/99 7:56p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMB.h"

#include  "CtEvent.h"         // standard status codes


#include  <assert.h>

#include  "Odyssey_Trace.h"


//  here's a little helper for reading the "board present" HBC register.
//  Bits in the addressed register are defined as shown in amSlotBoardBit[];
//  a zero bit means something is present in the bit's slot.
static U32 const * const pulBoardPresentBits = (U32 *) 0xBF010000;


//  and here's an array of masks for checking "board present" bits:
//  (an all-zero mask means no board bit for that "slot")
static U32  amSlotBoardBit [CT_IOPST_MAX_IOP_SLOTS]  =  {

   0x100000,      // IOP_HBC0 (board bit really means "other HBC")
   0x100000,      // IOP_HBC1 (ditto)

   0x000010,      // IOP_SSDU0 (B1)
   0x000020,      // IOP_SSDU1 (B2)
   0x000040,      // IOP_SSDU2 (B3)
   0x000080,      // IOP_SSDU3 (B4)
   0x001000,      // IOP_SSDL0 (D1)
   0x002000,      // IOP_SSDL1 (D2)
   0x004000,      // IOP_SSDL2 (D3)
   0x008000,      // IOP_SSDL3 (D4)
   0x000008,      // IOP_RAC0  (A4)
   0x000001,      // IOP_APP0  (A1)
   0x000002,      // IOP_APP1  (A2)
   0x000004,      // IOP_NIC0  (A3)
   0x000800,      // IOP_RAC1  (C4)
   0x000100,      // IOP_APP2  (C1)
   0x000200,      // IOP_APP3  (C2)
   0x000400,      // IOP_NIC1  (C3)
   0,             // CMB_EVC0
   0,             // CMB_EVC1
   0x010000,      // CMB_DDH0
   0x020000,      // CMB_DDH1
   0x040000,      // CMB_DDH2
   0x080000,      // CMB_DDH3 
};


//
//  CDdmCMB::ReqUpdateIopStatus (pReqMsg)
//
//  Description:
//    Called when somebody sends us a CMB_UPDATE_IOP_STATUS request
//    message (class MsgCmbUpdateIopStatus).
//
//    We refresh the specified IOP's row in the IOP Status table.
//
//  Inputs:
//    pReqMsg - Aliased pointer to MsgCmbUpdateIopStatus, the request
//             which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqUpdateIopStatus - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMB::ReqUpdateIopStatus (Message *pReqMsg)
{

MsgCmbUpdateIopStatus * pReq;
U32                     iContigSlot;      // contiguous index of requested slot
SlotContext           * pSlotCtx;         // our slot operation context
TSReadRow             * pReader;
STATUS                  sMyRet;


//*   Tracef("CDdmCMB::ReqUpdateIopStatus()\n");

   //  first off, upcast request message to proper type:
   if (! MsgUpcast (pReqMsg, pReq))
      {
      //  tsk, unsupported msg
//      assert (FALSE);
      Reply (pReqMsg, CTS_CMB_INVALID_PARAMETER);     // sort of..
      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  validate requested IOP slot number
   if (! ValidIopSlot (pReq -> eTargetSlot, iContigSlot))
      {
      //  tsk, let the requestor know they screwed up
      Reply (pReq, CTS_CMB_INVALID_PARAMETER);
      return (CTS_SUCCESS);
      }

   //  allocate a little context holder for this request's processing
   pSlotCtx = new SlotContext;
   //BUGBUG - no heap errors, right?

   //  fill in all our salient context
   pSlotCtx -> eSlot = pReq -> eTargetSlot;
   pSlotCtx -> iContigSlot = iContigSlot;
   pSlotCtx -> pReq  = pReq;

   if (m_CmbHwIntf.IsRealCmbPresent())
      {
      //  hmm, got real CMB interface.  First, we need to read the IOP's
      //  current row from the IOP Status table

      //  allocate & save buffer to read IOP slot's row into
      pSlotCtx -> pIopRow = new IOPStatusRecord;

      //  make up our PTS read-row message
      pReader = new TSReadRow;
     
      sMyRet = pReader->Initialize(this,
                          CT_IOPST_TABLE_NAME,
                          CT_PTS_RID_FIELD_NAME,
                          &(m_aIopStatusImage[iContigSlot].rid),
                          sizeof (m_aIopStatusImage[iContigSlot].rid),
                          pSlotCtx -> pIopRow,     // row buffer to read into
                          sizeof (IOPStatusRecord),
                          &(pSlotCtx -> cRowsRead),
                          (pTSCallback_t) &ReqUpdateIopStatus2,
                          pSlotCtx);
      assert (sMyRet == CTS_SUCCESS);
     
      //  send request off to PTS
      pReader->Send ();
      }
   else
      {
      //  we're bogus and don't have a CMB to talk to.  So return
      //  a totally fake IOP status.
      pSlotCtx -> pIopRow = BuildCurrentIopStatus (iContigSlot);

      //BUGBUG - IopStatus will never come back NULL, 'cause heap never fails :-)

      UpdatePtsIopRow (pSlotCtx);

      //  (UpdatePtsIopRow() takes care of all cleanup & reply -- we're done)
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqUpdateIopStatus */


//  We're called back when PTS answers with the IOP's row from IOP Status table.
//
//  Inputs:
//    pClientContext - Aliased reference to pSlotCtx, "slot context" for
//                   IOP whose status we're checking.
//    status - Result of PTS ReadRow command.
//
STATUS CDdmCMB::ReqUpdateIopStatus2 (void *pClientContext, STATUS status)
{

SlotContext     * pSlotCtx;         // our slot operation context


   assert (m_CmbHwIntf.IsRealCmbPresent());

   //  unmask our slot context
   pSlotCtx = (SlotContext *) pClientContext;

   //  did we get the IOP Status row back?
   if (status != CTS_SUCCESS)
      {
      //  whoops, stop now
      Tracef ("CDdmCMB::ReqUpdateIopStatus2: PTS row read status = %x\n",
              status);

      Reply (pSlotCtx->pReq, status);
      return (CTS_SUCCESS);
      }

   //  ok, got IOP's current record.

   //  Now ask CMB for IOP slot's present status

   //  send Status Query command off to CMB
   SendCmbMsg (pSlotCtx, k_eCmbCmdCmbSlotStatus, CMB_SELF,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus3),
               1, pSlotCtx -> eSlot);

   //  no status returned at this time, and CMB callback may be immediate,
   //  so exit right now.
   return (CTS_SUCCESS);

}  /* end of CDdmCMB::ReqUpdateIopStatus2 */

//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdCmbSlotStatus command.
//
//  Inputs:
//    pvCookie - Aliased reference to pSlotCtx, "slot context" for
//                   IOP whose status we're checking.
//    status - Result of CMB "slot status" command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus3 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

IopState          eState;
SlotContext     * pSlotCtx;         // our slot operation context
IOPStatusRecord * pIopRow;
U32               mBoardPresentBits;
U32               mCurSlotBit;


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert ((pReply == NULL) || (status == CTS_SUCCESS));

   assert (pvCookie != NULL);

   //  unmask our cookie
   pSlotCtx = (SlotContext *) pvCookie;

   //  check our reply -- is it ok?
   if (status == CTS_SUCCESS)
      {
      //  cool, got some status back for IOP
      assert (pReply->Hdr.cbData == 2);   // two bytes, ID and Status
      assert (pReply->Hdr.bCommand == k_eCmbCmdCmbSlotStatus);

      //  returned state info might be our pseudo-state, "attn needed."
      //  If so, run it through special processing
      if (pReply->abTail[1] == k_eCmbMStAttentionNeeded)
         {
         //  yup, take care of "attention needed" pseudo-state
         HandleAttentionState (*pReply);
         }

      //  now map raw state into IOP Status table-style of state code
      //  (if state code is a pseudo-state, it will be fixed up)
      CmbStateToIopStatus (pReply->abTail[1], eState);

      //  save present state in IOP slot's status record
      pSlotCtx -> pIopRow -> eIOPCurrentState = eState;

      //  if we have an empty slot, then stop right here.
      if ((eState == IOPS_UNKNOWN) ||
          (eState == IOPS_EMPTY) ||
          (eState == IOPS_BLANK))
         {
         //  no IOP, so just update state field and we're done.

         //  first, figure out whether slot is truly empty, or has
         //  a filler panel:
         mBoardPresentBits = *pulBoardPresentBits;
         mCurSlotBit = amSlotBoardBit[pSlotCtx->iContigSlot];

         if ((mBoardPresentBits & mCurSlotBit) != 0)
            {
            //  got a bit defined for this slot, and it says "not present":
            eState = IOPS_EMPTY;
            }
         else if (pSlotCtx->eSlot >= CMB_DDH0)
            {
            //  whoa, here's a little kludge for Andrey:
            if (eState == IOPS_BLANK)
               {
               //  for some reason, the "board present bits" aren't
               //  reliable for DDHs on all FP3 hardware (notably,
               //  Jerry's machine).  So, we ignore the hardware
               //  claiming that a non-talkative DDH is present,
               //  and just force state to "empty":
               eState = IOPS_EMPTY;
               }
            }
         else
            {
            //  leave eState at its default translation of AVR's value:
            //  "blank" or "unknown"
            }

         //  build an empty IOP slot record (use buffer we allocated for read)
         pIopRow = pSlotCtx -> pIopRow;
         pIopRow -> Clear ();
         pIopRow -> Slot = pSlotCtx -> eSlot;
         pIopRow -> eIOPCurrentState = eState;

         //  and update IOP slot's row in PTS (takes care of all cleanup)
         UpdatePtsIopRow (pSlotCtx);
         }
      else
         {
         //  got a real IOP, let's read some more params from it to fill out
         //  our final IOP Status row

         SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
                     CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus4),
                     1, k_eCmbParamIopType);
         }
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus3: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);

      //  rather than saving our partial results, we fail the request.
      //  BUGBUG - this is rather non-robust, we're not even retrying.
      Reply (pSlotCtx->pReq, status);

      //  free memory, we're all done.
      delete pSlotCtx->pIopRow;
      delete pSlotCtx;
      }
   
   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus3 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_CmbParamIopType] command.
//
//  Inputs:
//    pvCookie - Aliased reference to pSlotCtx, "slot context" for
//                   IOP whose status we're checking.
//    status - Result of CMB "slot status" command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus4 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext     * pSlotCtx;         // our slot operation context


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

//   assert ((pReply == NULL) || (status == CTS_SUCCESS));

   assert (pvCookie != NULL);

   //  unmask our cookie
   pSlotCtx = (SlotContext *) pvCookie;

   //  check our reply -- is it ok?
   if (status == CTS_SUCCESS)
      {
      //  cool, got some status back for IOP, save it first.
      assert (pReply->Hdr.cbData == 1);    // one byte, IOP type

      //  save IOP type in IOP slot's row record
      pSlotCtx->pIopRow->IOP_Type = (IopType) (pReply->abTail[0]);
      
      //BUGBUG - upper bits of type are bogus; mask them away
      if (pSlotCtx->eSlot < CMB_EVC0)
         {
         //  IOPs have need for bogus slot bit stripping
         pSlotCtx->pIopRow->IOP_Type = 
            (IopType) (pSlotCtx->pIopRow->IOP_Type & 3);
         }

//*      //BUGBUG - at this time, the real firmware doesn't support
//*      //         things like chassis serial number.  So we stop here
//*      //         and file what we've got.  (just IOP type & state)
//*
//*      //  and update IOP slot's row in PTS (takes care of all cleanup)
//*      UpdatePtsIopRow (pSlotCtx);
//*      return;
//*BUGBUG - endit

      //  now ask for IOP's chassis ("system") serial number
      SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
                  CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus5),
                  1, k_eCmbParamChassisSerNum);
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus4: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);

      //  rather than saving our partial results, we fail the request.
      //  BUGBUG - this is rather non-robust, we're not even retrying.
      Reply (pSlotCtx->pReq, status);

      //  free memory, we're all done.
      delete pSlotCtx->pIopRow;
      delete pSlotCtx;
      }

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus4 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_CmbParamChassisSerNum] command.
//
//  Inputs:
//    pvCookie - Aliased reference to pSlotCtx, "slot context" for
//                   IOP whose status we're checking.
//    status - Result of CMB "slot status" command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus5 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext     * pSlotCtx;         // our slot operation context


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pvCookie != NULL);

   //  unmask our cookie
   pSlotCtx = (SlotContext *) pvCookie;

   //  check our reply -- is it ok?
   if (status == CTS_SUCCESS)
      {
      //  cool, got some status back for IOP, save it first.
      assert (pReply->Hdr.cbData <=
              sizeof (pSlotCtx->pIopRow->ChassisSerialNumber));

      //  save IOP's chassis serial number in IOP slot's row record

      memset (pSlotCtx->pIopRow->ChassisSerialNumber, 0, 
              sizeof (pSlotCtx->pIopRow->ChassisSerialNumber));
      memcpy (pSlotCtx->pIopRow->ChassisSerialNumber, pReply->abTail,
              sizeof (pSlotCtx->pIopRow->ChassisSerialNumber));
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus5: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  now ask for IOP's own board serial number
   SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus6),
               1, k_eCmbParamBoardSerNum);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus5 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_CmbParamBoardSerNum] command.
//
//  Inputs:
//    pvCookie - Aliased reference to pSlotCtx, "slot context" for
//                   IOP whose status we're checking.
//    status - Result of CMB "slot status" command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus6 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext     * pSlotCtx;         // our slot operation context


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pvCookie != NULL);

   //  unmask our cookie
   pSlotCtx = (SlotContext *) pvCookie;

   //  check our reply -- is it ok?
   if (status == CTS_SUCCESS)
      {
      //  cool, got some status back for IOP, save it first.
      assert (pReply->Hdr.cbData <=
              sizeof (pSlotCtx->pIopRow->SerialNumber));

      //  save IOP's board serial number in IOP slot's row record

      memset (pSlotCtx->pIopRow->SerialNumber, 0, 
              sizeof (pSlotCtx->pIopRow->SerialNumber));
      memcpy (pSlotCtx->pIopRow->SerialNumber, pReply->abTail,
              sizeof (pSlotCtx->pIopRow->SerialNumber));
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus6: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  done with serial numbers, now see about AVR firmware version info
   SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus7),
               1, k_eCmbParamCmaFirmwareInfo);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus6 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_eCmbParamCmaFirmwareInfo] command.
//
//  Inputs:
//    pSlotCtx - "Slot context" for IOP whose status we're checking.
//    status - Result of CMB command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus7 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext  * pSlotCtx  =  (SlotContext *) pvCookie;


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pSlotCtx != NULL);

   if ((status == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, status);
      }

   //  check our reply -- is it ok?
//*BUGBUG - AVR is returning bogus response, so let's be more specific
//*   about what we treat as "success":
   if ((status == CTS_SUCCESS) && (pReply != NULL) &&
       (pReply->Hdr.cbData == 2))
//*   if (status == CTS_SUCCESS)
      {
      assert ((pReply != NULL) && (pReply->Hdr.cbData == 2));

      //  save IOP's AVR firmware version / revision
      pSlotCtx->pIopRow->ulAvrSwVersion  = pReply->abTail[0];
      pSlotCtx->pIopRow->ulAvrSwRevision = pReply->abTail[1];
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus7: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  now see about reading board's hardware build info
   SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus8),
               1, k_eCmbParamBoardHwBuildInfo);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus7 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_eCmbParamBoardHwBuildInfo] command.
//
//  Inputs:
//    pSlotCtx - "Slot context" for IOP whose status we're checking.
//    status - Result of CMB command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus8 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext  * pSlotCtx  =  (SlotContext *) pvCookie;


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pSlotCtx != NULL);

   if ((status == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, status);
      }
      
   //  check our reply -- is it ok?
//*BUGBUG - AVR is returning bogus response, so let's be more specific
//*   about what we treat as "success":
   if ((status == CTS_SUCCESS) && (pReply != NULL) &&
       (pReply->Hdr.cbData == 22))
//*   if (status == CTS_SUCCESS)
      {
      assert ((pReply != NULL) && (pReply->Hdr.cbData == 22));

      //  save IOP's part number info
      memcpy (pSlotCtx->pIopRow->strHwPartNo, pReply->abTail,
              sizeof (pSlotCtx->pIopRow->strHwPartNo));
      pSlotCtx->pIopRow->ulHwRevision =
                        ntohs (*(U16 *) (pReply->abTail + 16));
      pSlotCtx->pIopRow->ulHwMfgDate =
                        ntohl (*(U32 *) (pReply->abTail + 18));
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus8: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  read board's IOP boot params also (just because)
   SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus9),
               1, k_eCmbParamIopMipsHwParams);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus8 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_eCmbParamIopMipsHwParams] command.
//
//  Inputs:
//    pSlotCtx - "Slot context" for IOP whose status we're checking.
//    status - Result of CMB command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus9 (void *pvCookie, STATUS status,
                                    const CmbPacket *pReply)
{

SlotContext  * pSlotCtx  =  (SlotContext *) pvCookie;


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pSlotCtx != NULL);

   if ((status == CTS_SUCCESS) && (pReply != NULL))
      {
      CmbReplyToStatus (pReply, status);
      }

   //  check our reply -- is it ok?
//*BUGBUG - AVR is returning bogus response, so let's be more specific
//*   about what we treat as "success":
   if ((status == CTS_SUCCESS) && (pReply != NULL) &&
       (pReply->Hdr.cbData == 6))
//*   if (status == CTS_SUCCESS)
      {
      assert ((pReply != NULL) && (pReply->Hdr.cbData == 6));

      //  save IOP's part number info
      pSlotCtx->pIopRow->ulIopEpldRevision =
                     ntohs (*(U16 *) (pReply->abTail));
      pSlotCtx->pIopRow->ulIopMipsSpeed =
                     ntohs (*(U16 *) (pReply->abTail + 2));
      pSlotCtx->pIopRow->ulIopPciSpeed =
                     ntohs (*(U16 *) (pReply->abTail + 4));
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus9: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  grab board's manufacturer name field
   SendCmbMsg (pSlotCtx, k_eCmbCmdGetLastValue, pSlotCtx -> eSlot,
               CMBCALLBACK (CDdmCMB, ReqUpdateIopStatus10),
               1, k_eCmbParamBoardMfgName);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus9 */


//  We're called back when the CMB hw intf has a reply for our
//  k_CmbCmdGetLastValue[k_eCmbParamBoardMfgName] command.
//
//  Inputs:
//    pSlotCtx - "Slot context" for IOP whose status we're checking.
//    status - Result of CMB command processing.
//    pReply - CMB's reply packet.  Only valid when status == CTS_SUCCESS.
//

void  CDdmCMB::ReqUpdateIopStatus10 (void *pvCookie, STATUS status,
                                     const CmbPacket *pReply)
{

SlotContext  * pSlotCtx  =  (SlotContext *) pvCookie;


   //  we should only ever get here when we have a live CMB interface
   assert (m_CmbHwIntf.IsRealCmbPresent());

   assert (pSlotCtx != NULL);

   //  check our reply -- is it ok?
   if (status == CTS_SUCCESS)
      {
      assert ((pReply != NULL) &&
              (pReply->Hdr.cbData > 0) &&
              (pReply->Hdr.cbData < sizeof (pSlotCtx->pIopRow->Manufacturer)));

      //  save IOP's manufacturer name
      memset (pSlotCtx->pIopRow->Manufacturer, 0,
              sizeof (pSlotCtx->pIopRow->Manufacturer));
      memcpy (pSlotCtx->pIopRow->Manufacturer, pReply->abTail,
              pReply->Hdr.cbData);
      }
   else
      {
      //  whoops, didn't get a valid answer back from the CMB

      //BUGBUG - should throw an alarm here
      Tracef ("CDdmCMB::ReqUpdateIopStatus10: CMB cmd failed, status = %X, slot = %d\n",
              status, pSlotCtx->eSlot);
      }

   //  all done, now send updated row back to PTS
   UpdatePtsIopRow (pSlotCtx);

   return;

}  /* end of CDdmCMB::ReqUpdateIopStatus10 */


