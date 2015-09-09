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
// File: DdmCMBTest.cpp
// 
// Description:
//    This file contains a test DDM used to exercise the CMB DDM.
// 
// $Log: /Gemini/Odyssey/DdmCmb/TestDdm/DdmCMBTest.cpp $
// 
// 22    10/21/99 6:56p Ewedel
// Corrected environmental some units displays.
// 
// 21    10/19/99 7:52p Ewedel
// Changed to report on new EVC Raw Params member eEvcMasterSlot.
// 
// 20    10/08/99 12:06p Ewedel
// Added exercise of new MsgCmbSetMipsState request, and updated state
// display table (it's now a little class StateNameMap :-).
// 
// 18    9/13/99 6:59p Agusev
// [ewx] Added type decoding for EVC and DDH, and replaced serial number
// display with AVR firmware version (most useful!).
// 
// 17    9/03/99 6:52p Ewedel
// Updated for PHS header change.
// 
// 16    9/03/99 5:34p Ewedel
// Modified to exercise CMB environmental reporting via new PHS Reporter
// interface.
// 
// 15    8/11/99 7:45p Ewedel
// Updated to reflect IOP Status table change (no more Name field).
// 
// 14    7/27/99 6:41p Ewedel
// Updated for new TS interface, and removed old printf() test call.
// 
// 13    7/16/99 7:22p Ewedel
// Added listen on IOP Status table, to hear when changes are made.  Also
// moved IOP Status table row printing into its own routine,
// PrintIopStatusRow() (surprise!).
// 
// 12    7/15/99 4:14p Ewedel
// Changed IOPTY_NIC_RAC to replacement type IOPTY_NAC.
// 
// 11    6/24/99 7:19p Ewedel
// Cleaned out old cruft, updated to latest CmdQueues interface.
// 
// 10    6/18/99 10:55a Rkondapalli
// Oops, put auto-slot finder into routine which is actually being used.
// [ew]
// 
// 9     6/16/99 8:20p Ewedel
// Added support for dynamically determining which IOP slot to play
// power-on games with (it picks the first which has initial state
// "powered down").
// Also worked around status queue bug (can't be zero size now).
// 
// 8     6/15/99 7:10p Ewedel
// Updated for latest command queue interface.
// 
// 7     6/15/99 12:29a Ewedel
// Updated to current queue interface level.
// 
// 6     6/15/99 12:03a Ewedel
// Added command queue interface exercise stuff.
// 
// 5     5/12/99 4:07p Ewedel
// Updated for trace.h -> Odyssey_Trace.h system change, and to clean up a
// few DDM style bugs.  Also revised to account for updated IOP status
// table structure.
// 
// 4     4/07/99 6:28p Ewedel
// Changed to request power-on of a card which is present per DdmCmb's
// present static card table.  Also changed status messages to use hex for
// readability.
// 
// 3     3/30/99 6:24p Ewedel
// Updated to work with latest CHAOS changes.
// 
// 2     3/24/99 1:47p Ewedel
// Added test of MsgCmbIopControl message.  Also added some printf() test
// code.
// 
// 1     3/19/99 7:37p Ewedel
// Initial checkin.
//
/*************************************************************************/


#include  "DdmCmbTest.h"

#include  "DdmCMBMsgs.h"

#include  "DdmCmb.h"          // for just a few publics..

#include  "CmbDdmCommands.h"

#include  "IopTypes.h"

#include  "BuildSys.h"

#include  "ReadTable.h"

#include  "Fields.h"

#include  "Odyssey_Trace.h"

#include  "EnvStatusReport.h" // for PHS_RETURN_STATUS payload (sgl)

#include  "RqDdmReporter.h"   // for PHS' SGL ID

#include  "stdio.h"

#include  "OsHeap.h"          // heap debug support (from ..\msl)

#include  <assert.h>

   
//  Class Link Name used by Buildsys.cpp
CLASSNAME(CDdmCMBTest, SINGLE);



//  we pretend that we're the PHS reporter, in order to exercise that intf.
//  in the CMB DDM
SERVELOCAL (CDdmCMBTest, PHS_START);


class StateNameMap
{
public:
   static const char *GetName (IopState eState)
         {  return (m_aIopStateDescr [eState - IOPS_UNKNOWN]);  };

private:

   //  a helper for displaying IOP state -- indexed by IopState
   static const char *m_aIopStateDescr [];

};  /* end of class StateNameMap */

const char *StateNameMap::m_aIopStateDescr [] = {
   "unknown",           // IOPS_UNKNOWN
   "empty",             // IOPS_EMPTY
   "blank",             // IOPS_BLANK
   "powered on",        // IOPS_POWERED_ON
   "awaiting boot",     // IOPS_AWAITING_BOOT
   "diag mode",         // IOPS_DIAG_MODE
   "booting",           // IOPS_BOOTING
   "loading",           // IOPS_LOADING
   "operating",         // IOPS_OPERATING
   "suspended",         // IOPS_SUSPENDED
   "failing over",      // IOPS_FAILING
   "failed [over]",     // IOPS_FAILED
   "quiescent",         // IOPS_QUIESCENT
   "powered down",      // IOPS_POWERED_DOWN
   "unlocked",          // IOPS_UNLOCKED
   "image corrupt",     // IOPS_IMAGE_CORRUPT
   "out of memory"      // IOPS_INSUFFICIENT_RAM
};


//  helper for dumping rows from the IOP Status table
static void
PrintIopStatusRow (const IOPStatusRecord *pIopRow, BOOL fShowHeader = FALSE);

//  helper for dumping an EVC Raw Parameter table row
static void  PrintEvcRaw (const CtEVCRawParameterRecord& Rec);

//  helper for dumping an env status reply's IOP slot data
static
void  PrintEnvCmbSlot (TySlot eSlot,
                       const ENV_STATUS_RECORD::CMB_SLOT_RECORD& SlotInfo);


static void Test_Printf(void);

static void Test_Printf()
{
 
U32 value;
I64 dword_value;


   value = 0X0A06DEDA0;
   Tracef("value = 0X0A06DEDA0 = %%X %X\n", value);

   value = 0X80000000;
   Tracef("value = 0X80000000 = %%X %X\n", value);

   dword_value = 0X0A06DEDA0A06DEDA0;
   Tracef("dword_value =  0x0A06DEDA0A06DEDA0 = %%LX %LX\n", dword_value);
   Tracef("dword_value = 11560157092672040352 = %%Lu %Lu\n", dword_value);
   Tracef("dword_value = -6886586981037511264 = %%Ld %Ld\n", dword_value);

}  /* end of Test_Printf */



// Ctor -- Create ourselves --------------------------------------------CDdmCMBTest-
//
Ddm *CDdmCMBTest::Ctor (DID did)
{
   
   return (new CDdmCMBTest(did));

}  /* end of CDdmCMBTest::Ctor */


// CDdmCMBTest -- Constructor -------------------------------------------CDdmCMBTest-
//
CDdmCMBTest::CDdmCMBTest (DID did) : Ddm(did),
                           m_CmbDdmIntf (CMB_CONTROL_QUEUE,
                                         CMB_CONTROL_COMMAND_SIZE,
                                         CMB_CONTROL_STATUS_SIZE,
                                         this)
{

   Tracef("CDdmCMBTest::CDdmCMBTest()\n");
//   SetConfigAddress(&config,sizeof(config)); // tell Ddm:: where my config area is

   //  initialize our simple member data
   m_pIopRowsRead       = NULL;
   m_cIopRowsRead       = 0;
   m_pStateListen       = NULL;
   m_pStateListenType   = NULL;
   m_pStateUpdRow       = NULL;
   m_cbStateUpdRow      = 0;
   m_eFirstPoweredDownSlot = (TySlot) 0;

   //  do some printf() exercises for JimF
//   Test_Printf ();

}  /* end of CDdmCMBTest::CDdmCMBTest */


// Enable -- Start-it-up -----------------------------------------------CDdmCMBTest-
//
STATUS CDdmCMBTest::Enable(Message *pMsg)
{

STATUS   status;


   Tracef("CDdmCMBTest::Enable()\n");

   //  make our PHS emulator available to the CMB DDM
   //  (might better be done in Initialize(), which we don't have)
   DispatchRequest(PHS_START,
                   REQUESTCALLBACK (CDdmCMBTest, ReqPhsStart));

   //  send our first message off to the CMB DDM
   status = WakeUpCmbDdm ();

   Tracef("CDdmCMBTest::WakeUpCmbDdm() status = %u.\n", status);

   //  report successful enable
   Reply (pMsg, CTS_SUCCESS);
   return (CTS_SUCCESS);

}  /* end of CDdmCMBTest::Enable */

//
//  CDdmCMBTest::WakeUpCmbDdm ()
//
//  Description:
//    Called by our Enable() routine.  We send the first message to the
//    CMB DDM.  This will (once CHAOS is finished) cause the CMB DDM to
//    be faulted-in.
//
//    It also causes the CMB DDM to create & populate the IOP Status and
//    EVC Status tables in PTS.
//
//    Our full routine operation, bridging across several callbacks, is
//       o  Send a MsgCmbPollAllIops message to the CMB DDM.
//       o  Upon reply from that message, enumerate the IOP Status table
//          in PTS.
//
//  Inputs:
//    none
//
//  Outputs:
//    CDdmCMBTest::WakeUpCmbDdm* - Returns result of most recent OS operation
//             (Send, etc.).
//

STATUS  CDdmCMBTest::WakeUpCmbDdm (void)
{

STATUS               ulRet;


   //  initialize our control interface, in a synchronous way
   ulRet = m_CmbDdmIntf.csndrInitialize(INITIALIZECALLBACK (CDdmCMBTest,
                                                            WakeUpCmbDdm1));

   assert (ulRet == CTS_SUCCESS);
   
   return (CTS_SUCCESS);

}  /* end of CDdmCMBTest::WakeUpCmbDdm */


//  We are called back from m_CmdServer.csrvInitialize(), in Initialize().
//
//  Inputs:
//    ulStatus - Result of control interface initialize operation.
//

void  CDdmCMBTest::WakeUpCmbDdm1 (STATUS ulStatus)
{

STATUS   sRet;


   //  as a stub, all we can do is flag if init went bad, no init message
   //  available to reply to here.
   assert (ulStatus == CTS_SUCCESS);

   //  now let's hang a listen on the IOP Status table, looking for
   //  state field changes.  [Since the CMB DDM is now system-start,
   //  and we're not, we can safely assume that the whole table is
   //  built before we ever begin execution.]

   m_pStateListen = new TSListen;

   //  we need to say how big our row return buffer needs to be.
   m_cbStateUpdRow = sizeof (IOPStatusRecord);

   sRet = m_pStateListen->Initialize(
                              this,
                              ListenOnModifyAnyRowAnyField,
                              CT_IOPST_TABLE_NAME,
                              NULL, NULL, 0,    // no key field value
                              NULL, NULL, 0,    // no field name value
                              ReplyContinuous | ReplyWithRow,  // return row data
                              NULL, NULL,       // no table data ret
                              NULL,             // no listener ID ret
                              &m_pStateListenType,
                              &m_pStateUpdRow,
                              &m_cbStateUpdRow,
                              (pTSCallback_t) &WakeUpCmbDdm1a,
                              NULL);            // no cookie needed

   assert (sRet == CTS_SUCCESS);

   m_pStateListen->Send();

   //  all done, wait for CDdmCMBTest::WakeUpCmbDdm1a to be called.
   return;

}  /* end of CDdmCMBTest::WakeUpCmbDdm1 */


//  We are called back from m_pStateListen->Send(), in WakeUpCmbDdm1().
//  This callback both indicates listen post completion, and is called
//  to signify a listen callback.
//
//  Inputs:
//    ulStatus - Result of PTS listen post request.
//

void  CDdmCMBTest::WakeUpCmbDdm1a (void *pvCookie, STATUS sStatus)
{

MsgCmbPollAllIops  * pPollReq;
STATUS               sRet;

#pragma unused(pvCookie)


   if ((*m_pStateListenType & ListenInitialReply) != 0)
      {
      //  we're being called to indicate that our listen request was
      //  posted.  This is just a continuation of our basic initialization
      //  sequence.
      Tracef ("CDdmCMBTest::WakeUpCmbDdm1a: listen post status = %X\n", sStatus);

      //  allocate a poll request message (must come from PCI space?)
      pPollReq = new MsgCmbPollAllIops;
     
      //  there isn't any body, just send the message
     
      sRet = Send (pPollReq, (ReplyCallback) &WakeUpCmbDdm2);
     
      if (sRet != OK)
         {
         //  whoops, didn't send message, so we'd better delete it here
         delete pPollReq;
         }
      }
   else
      {
      //  egad!  We're being called to signify a row change in the
      //  IOP Status table.  Announce this to our user.
      Tracef ("CDdmCMBTest::WakeUpCmbDdm1a: listen callback, status = %X\n",
              sStatus);

      if (sStatus == CTS_SUCCESS)
         {
         //  did a happy listen, so let's report our row data.
         assert (m_cbStateUpdRow >= sizeof (*m_pStateUpdRow));

         PrintIopStatusRow (m_pStateUpdRow, TRUE);
         }
      }

   return;

}  /* end of CDdmCMBTest::WakeUpCmbDdm1a */


//
//  Inputs:
//    pReply - Points to message allocated in WakeUpCmbDdm(), which is
//             returning from the CMB DDM with neat news (we hope :-).
//

STATUS  CDdmCMBTest::WakeUpCmbDdm2 (MessageReply *pReply)
{

TSReadTable     * pReadIops;


   //  got reply back to our earlier send.

   //  Let's see what CMB DDM said in response to our "poll all" request
   Tracef("CDdmCMBTest::WakeUpCmbDdm2() status = %u.\n",
          pReply->DetailedStatusCode);
  
   //  Per TRN, we are supposed to delete our own messages, so let's do it:
   delete pReply;

   //... now enumerate rows in the IOP Status table
   pReadIops = new TSReadTable;

   //  set table read parameters
   pReadIops->Initialize (this,
                          CT_IOPST_TABLE_NAME,   // table name
                          &m_pIopRowsRead,
                          &m_cIopRowsRead,
                          (pTSCallback_t) &WakeUpCmbDdm3,
                          NULL);

   //  make some safe defaults..
   m_pIopRowsRead = NULL;
   m_cIopRowsRead = 0;

   //  do the send
   pReadIops -> Send ();

   return (CTS_SUCCESS);

}  /* end of CDdmCMBTest::WakeUpCmbDdm2 */

//
//  Inputs:
//    pClientContext - Not used.
//    ulResult - Result of read-rows PTS request.
//    m_pIopRowsRead - Points to buffer containing IOP Status table data.
//    m_cIopRowsRead - One-based count of IOP Status table rows read.
//

STATUS  CDdmCMBTest::WakeUpCmbDdm3 (void *pClientContext, STATUS ulResult)
{

#pragma unused (pClientContext)

IOPStatusRecord * pSlot;
CmbCtlRequest     strCmbReq;
U32               ulRet;
U32               i;


   printf("CDdmCMBTest::WakeUpCmbDdm3: ulResult = %u, m_cIopRows = %u\n",
          ulResult, m_cIopRowsRead);

   if (ulResult != CTS_SUCCESS)
      {
      printf("CDdmCMBTest::WakeUpCmbDdm3: quitting due to enumeration failure\n");
      return (CTS_SUCCESS);
      }

   if (m_cIopRowsRead == 0)
      {
      printf("CDdmCMBTest::WakeUpCmbDdm3: quitting due to no IOP data read\n");
      return (CTS_SUCCESS);
      }

   //  zip through returned array, reporting on each slot

   printf("CDdmCMBTest::WakeUpCmbDdm3: %d slots enumerated\n", m_cIopRowsRead);
   
   m_eFirstPoweredDownSlot = (TySlot) 0;

   pSlot = m_pIopRowsRead;
   for (i = 0;  i < m_cIopRowsRead;  i ++)
      {
      PrintIopStatusRow (pSlot, i == 0);     // (print header, first row only)
      
      //  also keep an eye out for the first powered-down IOP we see
      if ((pSlot->eIOPCurrentState == IOPS_POWERED_DOWN) &&
          (m_eFirstPoweredDownSlot == (TySlot) 0))
         {
         //  yes, we have a winner!
         m_eFirstPoweredDownSlot = pSlot->Slot;
         }

      //  all done with this IOP Status slot, move on to next one
      pSlot ++;
      }

   //  all done listing IOP slots, now tell a card to turn on.
   //  For now, we just always ask the IOP in slot IOP_RAC0
   //  (see DdmCMB.cpp, array aIopSlot[]) to power up.

   //  set up various parameters for request (note that parameter
   //  struct need only live for duration of csndrExecute(), below)
   strCmbReq.eCommand = k_eCmbCtlPower;
   strCmbReq.eSlot    = m_eFirstPoweredDownSlot;
   strCmbReq.u.Power.bPowerOn = TRUE;     // power-up, not power-down

   OsHeap::CheckHeap();

   ulRet = m_CmbDdmIntf.csndrExecute (&strCmbReq, 
                                      CMD_COMPLETION_CALLBACK (CDdmCMBTest,
                                                               WakeUpCmbDdm4),
                                      NULL);    // we have no cookie today

   OsHeap::CheckHeap();

   assert (ulRet == CTS_SUCCESS);

   Tracef("CDdmCMBTest::WakeUpCmbDdm3() powerup send status = 0x%X.\n", ulRet);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMBTest::WakeUpCmbDdm3 */


//
//  Inputs:
//    sStatus - Status of command requested by csndrExecute(), above.
//    pResultData - Not used with the CMB DDM interface.
//    pCmdData - Points to command data submitted to csndrExecute().
//    pvCookie - Points to cookie submitted to csndrExecute().
//

void  CDdmCMBTest::WakeUpCmbDdm4 (STATUS sStatus, void *pResultData,
                                  CmbCtlRequest *pCmdData, void *pvCookie)
{

#pragma unused(pCmdData)
#pragma unused(pvCookie)


   //  got reply back to our earlier send.

   assert (pResultData == NULL);

   //  Let's see what the CMB DDM said to our "power on" request
   Tracef("CDdmCMBTest::WakeUpCmbDdm4() \"power on\" request result = %X\n",
          sStatus);

   //  send off an "update status" message to move our IOP into "operating" state
   MsgCmbSetMipsState * pmsgSetMipsState;

   pmsgSetMipsState = new MsgCmbSetMipsState (MsgCmbSetMipsState::SetOperating);
   Send (pmsgSetMipsState, NULL, REPLYCALLBACK (DdmServices, DiscardOkReply));

   //  now, let's pretend we're an IOP, and forward an event log entry
   //  to the "HBC".
   //  [Will this work as-is on a real Odyssey?  Maybe..  but it should
   //   work well on an eval crate.]

   if (Address::iSlotHbcMaster != Address::iSlotMe)
      {
      //  we're not the HBC master, so send it an event

      //  make up our event instance
      Event  * pEvent;

      pEvent = new Event (CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);   // any code will do..

      //  submit event instance to CMB DDM, which consumes it
      //  [This mechanism is expected on the IOP, but would never be used
      //   on an HBC, since we are only a backup comm channel.]
      sStatus = CDdmCMB::ForwardEventLogEntry (pEvent);

      Tracef("CDdmCMBTest::WakeUpCmbDdm4() event log forward request result = %X\n",
             sStatus);
      }

   //  and that's it, we don't do anything else right now.

   //  [Of course, the CMB DDM should send us a PHS_START message,
   //   since we're acting as a dummy PHS Reporter.  We'll respond
   //   to that message with a PHS_RETURN_STATUS request, and the
   //   CMB DDM should then reply with a complete environmental sweep.]
   return;

}  /* end of CDdmCMBTest::WakeUpCmbDdm4 */

//
//  CDdmCMBTest::ReqPhsStart (pReqMsg)
//
//  Description:
//    Called when somebody sends us a PHS_START request message
//    message (class MsgCmbUpdateIopStatus).
//
//    We refresh the specified IOP's row in the IOP Status table.
//
//  Inputs:
//    pReqMsg - The request which we're to process.
//
//  Outputs:
//    CDdmCMB::ReqUpdateIopStatus - Returns OK, or a moderately
//             interesting error code.
//

STATUS CDdmCMBTest::ReqPhsStart (RqDdmReporter *pReqMsg)
{

Message   * pQueryMsg;


   TRACE_PROC (CDdmCMBTest::ReqPhsStart);

   assert (pReqMsg != NULL);
   assert (pReqMsg->reqCode == PHS_START);
   assert (pReqMsg->ReporterCode == PHS_EVC_STATUS);

   //  save CMB DDM's DID in our instance data, just because.
   m_didCmbDdm = pReqMsg->did;

   //  and, immediately ping the CMB DDM with a "return status" message
   pQueryMsg = new Message (PHS_RETURN_STATUS);

   //  Add the return Table data SGL item:
   //    NULL pb indicates data buffer to be alloc'd by transport.
   //    NULL cb will tell transport to get allocate size when the
   //    PTS calls GetSGL.  This requires SGL_DYNAMIC_REPLY too.
   pQueryMsg->AddSgl(DDM_REPLY_DATA_SGI, NULL, NULL, SGL_DYNAMIC_REPLY);

   Send (m_didCmbDdm, pQueryMsg, NULL,
         REPLYCALLBACK (CDdmCMBTest, ShowCmbPhsReply));

   return (CTS_SUCCESS);

}  /* end of CDdmCMBTest::ReqPhsStart */

//
//  CDdmCMBTest::ShowCmbPhsReply (pReply)
//
//  Description:
//    Called back when the CMB DDM replies to our PHS_RETURN_STATUS
//    query message.
//
//    We take care of dumping the CMB DDM's response to the console.
//
//  Inputs:
//    pReply - Reply message containing ENV_STATUS_RECORD data.
//
//  Outputs:
//    CDdmCMBTest::ShowCmbPhsReply - Always returns CTS_SUCCESS,
//             because that's what reply callbacks do.
//

STATUS CDdmCMBTest::ShowCmbPhsReply (Message *pReply)
{

ENV_STATUS_RECORD  * pEnv;
U32                  cbEnv;
TySlot               eSlot;
U32                  i;


   //  Let's see what the CMB DDM said to our "poll environment" PHS request
   Tracef("CDdmCMBTest::ShowCmbPhsReply() env poll result status = %X\n",
          pReply->Status());

   assert (pReply->Status() == CTS_SUCCESS);

   if (pReply->Status() == CTS_SUCCESS)
      {
      //  got a good reply, extract payload & make sure it looks ok
      cbEnv = 0;
      pReply->GetSgl(DDM_REPLY_DATA_SGI, &pEnv, &cbEnv);

      assert (cbEnv == sizeof (*pEnv));

      if (cbEnv == sizeof (*pEnv))
         {
         //  got an env record, display it
         Tracef ("  Evc0 Raw Parameters:\n");
         PrintEvcRaw (pEnv->EVCRawParameters[0]);

         Tracef ("  Evc1 Raw Parameters:\n");
         PrintEvcRaw (pEnv->EVCRawParameters[1]);

         //  display each CMB bus slot also
         for (i = 0;  i < CDdmCMB::m_cContigSlots; i ++)
            {
            eSlot = CDdmCMB::m_aeContigToSlot[i];
            PrintEnvCmbSlot (eSlot, pEnv->CmbSlotInfo[eSlot]);
            }
         }
      }

   delete pReply;
   
   return (CTS_SUCCESS);

}  /* end of CDdmCMBTest::ShowCmbPhsReply */

//
//  PrintIopStatusRow (pIopRow, fShowHeader)
//
//  Description:
//    A little helper routine for printing the contents of an IOP Status
//    table row.  We can optionally be told to print the standard table
//    display header (column labels, etc.) as well.
//
//  Inputs:
//    pIopRow - Points to row whose contents we display.
//    fShowHeader - If TRUE, we display the 
//
//  Outputs:
//    none
//

static
void  PrintIopStatusRow (const IOPStatusRecord *pIopRow, BOOL fShowHeader)
{

const char   * psz;
char           achBuf [32];      // for little message sections


   if (fShowHeader)
      {
      //  display IOP Status table print header
      printf("Type     Slot Manufacturer                        Name     AVR Rev   State\n");
//*      printf("Type     Slot Manufacturer                        Name     Serial #  State\n");
      }

   //  verify that slot entry looks roughly ok
   if (pIopRow->size != sizeof (*pIopRow))
      {
      printf("PrintIopStatusRow: !! slot size mismatch, actual = %d, expected = %d\n",
             pIopRow->size, sizeof (*pIopRow));
      }

   //Type     Slot Manufacturer                        Name     AVR Rev   State
   //ttttttt   ss  mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm  nnnnnnn  nn / nn   ssssssssssssssss
   //1234567   12  1234567890123456789012345678901234  1234567  12345678  1234567890123456

   //  figure out type value
   switch (pIopRow->IOP_Type)
      {
      case IOPTY_HBC:
         psz = "HBC";
         break;

      case IOPTY_NIC:
         psz = "NIC";
         break;

      case IOPTY_RAC:
         psz = "RAC";
         break;

      case IOPTY_SSD:
         psz = "SSD";
         break;

      case IOPTY_NAC:
         psz = "NAC";
         break;
         
      case IOPTY_EVC:
         psz = "EVC";
         break;
         
      case IOPTY_DDH:
         psz = "DDH";
         break;

      default:
         psz = achBuf;
         sprintf (achBuf, "? %03d ?", pIopRow->IOP_Type);
      }
   printf("%-7s   ", psz);

   //Type     Slot Manufacturer                        Name     AVR Rev   State
   //ttttttt   ss  mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm  nnnnnnn  nn / nn   ssssssssssssssss
   //1234567   12  1234567890123456789012345678901234  1234567  12345678  1234567890123456

   printf("%2d  %-34s  %-7s  %02d / %02d ",
          pIopRow->Slot, pIopRow->Manufacturer, /* pIopRow->Name */ "",
          pIopRow->ulAvrSwVersion, pIopRow->ulAvrSwRevision);
//*   printf("%2d  %-34s  %-7s  %-8s",
//*          pIopRow->Slot, pIopRow->Manufacturer, /* pIopRow->Name */ "",
//*          pIopRow->SerialNumber);

   //  translate state to text & print it

   if (pIopRow->eIOPCurrentState <= IOPS_POWERED_DOWN)
      {
      //  cool, it's one we know
      psz = StateNameMap::GetName (pIopRow->eIOPCurrentState);
      }
   else
      {
      //  whoops, one we don't know
      psz = achBuf;
      sprintf (achBuf, " ?%d?", pIopRow->eIOPCurrentState);
      }

   printf("  %s\n", psz);

}  /* end of PrintIopStatusRow */

//
//  PrintEvcRaw (Rec)
//
//  Description:
//    Prints one EVC Raw Parameter table record to the console.
//
//  Inputs:
//    Rec - The EVC raw record to display.
//
//  Outputs:
//    none
//

static
void  PrintEvcRaw (const CtEVCRawParameterRecord& Rec)
{

const char   * sz;
const char   * pszLeader  =  "      ";


   switch (Rec.EvcSlotId)
      {
      case CMB_EVC0:
         sz = "EVC0";
         break;

      case CMB_EVC1:
         sz = "EVC1";
         break;

      default:
         sz = "EVC???";
      }

   Tracef ("%s%s, reachable = %s\n", pszLeader, sz,
           Rec.fEvcReachable ? "True" : "False");

   if (! Rec.fEvcReachable)
      {
      //  nothing else to display..
      return;
      }

   //  temperature & related info
   Tracef ("%sExit air temp = %d, fan up thresh = %d, fan norm = %d\n",
           pszLeader, Rec.ExitAirTemp, Rec.ExitTempFanUpThresh,
           Rec.ExitTempFanNormThresh);
   Tracef ("%sFan speed [0] = %d, [1] = %d, [2] = %d, [3] = %d\n",
           pszLeader,
           Rec.FanSpeed[0], Rec.FanSpeed[1], Rec.FanSpeed[2], Rec.FanSpeed[3]);
   Tracef ("%sFan speed set [upper] = %d, [lower] = %d\n",
           pszLeader, Rec.FanSpeedSet[0], Rec.FanSpeedSet[1]);

   //  primary power supply values
   Tracef ("%sPrimary supply status (0 / 1 / 2)\n", pszLeader);
   Tracef ("%s  InputOk = %d / %d / %d, OutputOk = %d / %d / %d\n",
           pszLeader, Rec.fInputOK[0], Rec.fInputOK[1], Rec.fInputOK[2],
                      Rec.fOutputOK[0], Rec.fOutputOK[1], Rec.fOutputOK[2]);
   Tracef ("%s  Fail/overtemp = %d / %d / %d, Enable = %d / %d / %d\n",
           pszLeader, Rec.fFanFailOrOverTemp[0], Rec.fFanFailOrOverTemp[1],
           Rec.fFanFailOrOverTemp[2], Rec.fPrimaryEnable[0],
           Rec.fPrimaryEnable[1], Rec.fPrimaryEnable[2]);

   //  aux supply status
   Tracef ("%sAux supply status\n", pszLeader);
   Tracef ("%s  currents: 3.3v = %dma, 5v = %dma, 12v[A/B/C] = %d / %d / %d ma\n",
           pszLeader, Rec.DCtoDC33Current, Rec.DCtoDC5Current,
           Rec.DCtoDC12ACurrent, Rec.DCtoDC12BCurrent, Rec.DCtoDC12CCurrent);

   Tracef ("%s  DC-to-DC converter temperatures (degrees C):\n", pszLeader);
   Tracef ("%s    3.3v = %d, 5v = %d, 12v[A/B/C] = %d / %d / %d\n",
           pszLeader, Rec.DCtoDC33Temp, Rec.DCtoDC5Temp,
           Rec.DCtoDC12ATemp, Rec.DCtoDC12BTemp, Rec.DCtoDC12CTemp);
   Tracef ("%s  Aux Enable[0] = %d, Enable[1] = %d\n",
           pszLeader, Rec.fDCtoDCEnable[0], Rec.fDCtoDCEnable[1]);

   Tracef ("%sMidplane (SMP) voltages:\n",
           pszLeader);
   Tracef ("%s  48v = %d (mv), 3.3v = %d mv, 5v = %d (mv), 12v = %d (mv)\n",
           pszLeader, Rec.SMP48Voltage, Rec.DCtoDC33Voltage,
                      Rec.DCtoDC5Voltage, Rec.DCtoDC12Voltage);
   switch (Rec.KeyPosition)
      {
      case CT_KEYPOS_SERVICE:
         sz = "\"service\" mode (e.g., diags)";
         break;

      case CT_KEYPOS_OFF:
         sz = "power off";
         break;

      case CT_KEYPOS_ON:
         sz = "normal operation";
         break;

      case CT_KEYPOS_SECURITY:
         sz = "security override";
         break;

      default:
         sz = "?? unknown keyswitch setting ??";
      }
   Tracef ("%sKeyswitch position: %s\n", pszLeader, sz);

   switch (Rec.eEvcMasterSlot)
      {
      case CMB_EVC0:
         sz = "evc 0";
         break;

      case CMB_EVC1:
         sz = "evc 1";
         break;

      default:
         sz = "?? unrecognized ??";
         assert (FALSE);
         break;
      }

   Tracef ("%sEVC Master selection: %s\n", pszLeader, sz);

   Tracef ("%sBatteries (0 / 1):  temperature = %d / %d deg C,\n",
           pszLeader, Rec.BatteryTemperature[0], Rec.BatteryTemperature[1]);
   Tracef ("%s                    current = %d / %d ma\n",
           pszLeader, Rec.BatteryCurrent[0], Rec.BatteryCurrent[1]);

   return;

}  /* end of PrintEvcRaw */

//
//  PrintEnvCmbSlot (eSlot, SlotInfo)
//
//  Description:
//    Prints info contained in one EnvStatusRecord CmbSlot instance.
//
//  Inputs:
//    eSlot - Slot which SlotInfo describes.
//    SlotInfo - Misc stuff about a CMB slot (eSlot).
//
//  Outputs:
//    none
//

static
void  PrintEnvCmbSlot (TySlot eSlot,
                       const ENV_STATUS_RECORD::CMB_SLOT_RECORD& SlotInfo)
{

const char   * psz;
char           szBuf [16];


   //  print out a table header, if we're doing the first slot
   if (eSlot == IOP_HBC0)
      {
      Tracef ("   Slot     Present  Temp  HiThresh  NormThresh\n");
      Tracef ("                     (.1C)  (0.1C)    (0.1C)\n");
      //       sssss (nn)    sss     ttt    ttt        ttt
      //                1         2         3         4         5
      //       12345678901234567890123456789012345678901234567890
//              HBC0 IOPxx EVC0 DDH0 ???
      }

   assert ((eSlot >= IOP_HBC0) && (eSlot < CMB_ADDRESS_MAX));

   //  build up slot name
   switch (eSlot)
      {
      case IOP_HBC0:
         psz = "HBC0";
         break;

      case IOP_HBC1:
         psz = "HBC1";
         break;

      case CMB_EVC0:
         psz = "EVC0";
         break;

      case CMB_EVC1:
         psz = "EVC1";
         break;

      case CMB_DDH0:
         psz = "DDH0";
         break;

      case CMB_DDH1:
         psz = "DDH1";
         break;

      case CMB_DDH2:
         psz = "DDH2";
         break;

      case CMB_DDH3:
         psz = "DDH3";
         break;

      default:
         //  better be a plain old IOP slot
         sprintf (szBuf, "IOP%02d", eSlot - IOP_SSDU0);
         psz = szBuf;
      }

   //  now dump the slot's info (omit temp stuff if slot is empty)
   //     ("   Slot     Present  Temp  HiThresh  NormThresh\n");
   //       sssss (nn)    sss     ttt    ttt        ttt
   //                1         2         3         4         5
   //       12345678901234567890123456789012345678901234567890

   Tracef ("%-5s (%02d)    %-3s", psz, eSlot, SlotInfo.fPresent ? "yes" : " no");
   if (SlotInfo.fPresent)
      {
      //  got something in slot, so dump other params too
      Tracef ("     %3d    %3d        %3d",
              SlotInfo.Temperature,
              SlotInfo.TemperatureHiThreshold,
              SlotInfo.TemperatureNormThreshold);
      }

   Tracef ("\n");

   return;

}  /* end of PrintEnvCmbSlot */

