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
// File: DdmCmb.cpp
//
// Description:
//    Card Management Bus interface DDM.
//
// $Log: /Gemini/Odyssey/DdmCmb/DdmCMB.cpp $
// 
// 42    1/20/00 4:27p Eric_wedel
// Added support for CMB_EVC_POWER_CONTROL, and fixed callback target
// signatures.  Removed a little cruft, and added (minimal) support for
// Triton HBC board ID.
// 
// 41    11/19/99 6:35p Ewedel
// Changed fan speed setting to % (default is 25%) of full speed, and
// added cache of current settings.
// 
// 40    11/17/99 11:41a Ewedel
// Removed last vestiges of PHS support.
// 
// 39    11/16/99 6:12p Ewedel
// Added DDH control command, and changed env reporting from PHS message
// to special CMB message.
// 
// 38    11/05/99 6:21p Ewedel
// Updated CDdmCMB::ValidIopSlot() to support pseudo-slot ID "CMB_SELF."
// 
// 37    11/03/99 1:16p Ewedel
// Made all DispatchRequest() calls always be performed.  This is
// necessary for HBC failover support.  Of course, we now need to detect
// and fail master-only request messages sent to (& dispatched by)
// non-master CMB DDM instances.
// 
// 36    10/27/99 12:23p Ewedel
// Changed so that we now write our EnvControl table's settings to the
// EVCs.  This way we know they're in sync, and any previously-saved
// settings will be restored properly.
// 
// 35    10/08/99 12:02p Ewedel
// Added hooks for supporting new request message CMB_SET_MIPS_STATE.
// Changed CDdmCMB::ReportOsRunningToCma() et al to report our final state
// as "loading" on up-rev HBC AVRs, and to also update our PTS state, if
// we're the master HBC (where the normal AVR state update notification
// loop doesn't work).
// 
// 34    9/30/99 6:37p Ewedel
// Fixed HBC AVR version mismatch hack: correct latest downrev revision is
// 0x10.
// 
// 31    9/03/99 5:29p Ewedel
// Added support for PHS Reporter interface, calved off env control PTS
// table, added nice EVC param update helper routine (sends messages to
// all operating EVCs transparently).
// 
// 30    8/24/99 8:11p Ewedel
// Removed CMB_UPDATE_EVC_STATUS request.  Did substantial overhaul of PTS
// table init, to support failover & raw params.
// 
// 29    8/11/99 7:48p Ewedel
// Various changes, including migration of some code into DdmCmb.h, and
// new files ReqOther.cpp, ReqUpdateIopStatus.cpp.
// 
// 28    8/02/99 2:41p Ewedel
// Updated to work with latest EVC Status record defs.
// 
// 27    7/27/99 6:42p Ewedel
// Updated for new EVC Status record, and to work around buggy Odyssey ROM
// (doesn't set MIPS state properly on HBC).
// 
// 26    7/21/99 8:19p Ewedel
// Fixes for TS interface changes.
// 
// 25    7/19/99 5:04p Rkondapalli
// [ewx]  Fixed CDdmCMB::Initialize5() to that it looks for the [now]
// correct k_eCmbMStPoweredOn state.  [Left over bug from old cmd
// ordering.]
// 
// 24    7/16/99 10:13a Ewedel
// Added flags to gate processing of unsolicited input packets.
// 
// 23    7/15/99 4:18p Ewedel
// Flipped order of "get board info" and "set mips state" command sends:
// "get board info" now happens first.  Moved CmbMsgReceived() into new
// file CmbUnsolicited.cpp.  Updated for CmbPacket def changes.
// 
// 22    7/08/99 5:11p Ewedel
// Removed Initialize0a(), which was a patch to overcome temporary
// problems with "system start" DDM access to PTS.
// 
// 21    6/30/99 5:03p Ewedel
// Split Initialize() off into Initialize() and Initialize0a(), to make
// room for "PTS ping" call needed by new CHAOS release.
// 
// 20    6/24/99 7:25p Ewedel
// Updated to latest CmdQueue interface and revised some private constant
// names.
// 
// 19    6/16/99 8:27p Ewedel
// Added support for "get board info" and "set boot params".  Corrected
// caching of IOP state params.  Temporarily disabled extended IOP param
// queries, since CMB firmware doesn't support them (we now just ask for
// state & IOP type).
// 
// 18    6/15/99 7:08p Ewedel
// Updated for latest command queue interface.
// 
// 17    6/15/99 12:32a Ewedel
// Updated to current queue interface level.
// 
// 16    6/15/99 12:02a Ewedel
// Various updates, cleanup, support added for listen mode and command
// queue interface.  Added code to really talk to the CMB, when hardware
// is present.
// 
// 15    6/03/99 7:17p Ewedel
// Updated for revised TySlot enum def (see address.h) and for latest IOP
// and EVC status table changes.  Also added beginnings of CMB hw Intf
// class testing.
// 
// 14    5/14/99 12:07p Ewedel
// Removed old trace.h.
// 
// 13    5/12/99 4:14p Ewedel
// Added printf() that got lost during previous rev's merge.  Thanks, vss.
// 
// 12    5/12/99 4:12p Ewedel
// Various updates for revised IOP status structure.  Also changed kludge
// code from getchar() to getch(), and changed from trace.h to
// Odyssey_Trace.h per system-wide update.
// 
// 11    4/29/99 3:13p Jlane
// In ReqIOPControl2 compare keys against '\n' not '\r'.
// 
// 10    4/07/99 6:31p Ewedel
// Improved prompt display using new helper routine MakeSlotMessage(), and
// fixed  ReqIopControl2() so it won't permit actions to be performed on
// an empty IOP slot.  Further state transition intelligence is left to
// the System Master.  :-)
// 
// 9     4/05/99 7:07p Ewedel
// Rearranged aIopSlot[] yet again - this one matches hardware for now.
// :-)
// 
// 8     4/05/99 5:41p Ewedel
// Corrected size specifications for CDdmCMB::m_aeContigToSlot[] and
// aIopSlot[] so that they better reflect reality as presently understood.
// Also fixed CDdmCMB::Initialize2() so that it works off of the actual
// size of aIopSlot[].
// 
// 7     4/05/99 4:16p Ewedel
// Moved RAC to different slot, since IOP_RAC0 seems to have hardware
// issues.
// 
// 6     4/05/99 3:35p Ewedel
// Updated aIopSlot[] dummy slot description table to match the hardware
// that Jerry is working with.
// 
// 5     4/03/99 6:23p Jlane
// In ReqUpdateEvcStatus save incoming message pointer in conbtext pReq
// for later reply.
// 
// 4     4/02/99 11:08a Ewedel
// Changed Initialize() so that it always replies to its trigger message.
// It just returns a non-success detail status when there are problems.
// 
// 3     3/30/99 6:22p Ewedel
// Updated to work with latest PTS & CHAOS changes.
// 
// 2     3/24/99 1:45p Ewedel
// Removed printf() test code.
// 
// 1     3/19/99 7:37p Ewedel
// Initial checkin.
//
/*************************************************************************/

#include  "DdmCMB.h"
#include  "BuildSys.h"

#include  "DdmCMBMsgs.h"      // what we mostly traffic in
#include  "CmbMsgSendWrapper.h"  // a helper for sending CMB messages
#include  "IopTypes.h"        // what are the possible IOP card types

#include  "CmbDdmCommands.h"  // command/status queue interface defs

#include  "RqOsDdmManager.h"  // for funny "ping" message we must send to system

#include  <ansi\stdio.h>      // [ansi\ forces use of our local stdio.h]

#include  "OsHeap.h"          // heap debug support (from ..\msl)

#include  <assert.h>          // debug stuff

//*//  more debug stuff:  for tPCI to really be tSMALL,
//*//  so "placed" new()s will be included in the heap tester
//*#undef tPCI
//*#define tPCI  tSMALL

   
CLASSNAME (CDdmCMB, SINGLE);  // Class Link Name used by Buildsys.cpp --
                              // must match CLASSENTRY() name formal (4th arg)
                              // in buildsys.cpp

//  statically declare which request codes our DDM will serve:
//  (we declare ours as usable for board-local comms only)
//  *** These defs must match those in Initialize() et all, below ***

//  master HBC-only requests
SERVELOCAL (CDdmCMB, CMB_UPDATE_IOP_STATUS);
SERVELOCAL (CDdmCMB, CMB_IOP_CONTROL      );
SERVELOCAL (CDdmCMB, CMB_EVC_CONTROL      );
SERVELOCAL (CDdmCMB, CMB_EVC_POWER_CONTROL);
SERVELOCAL (CDdmCMB, CMB_DDH_CONTROL      );
SERVELOCAL (CDdmCMB, CMB_POLL_ALL_IOPS    );
SERVELOCAL (CDdmCMB, CMB_POLL_ENVIRONMENT );

//  requests supported by all CMB DDM instances
SERVELOCAL (CDdmCMB, CMB_SEND_SMALL_MSG   );
SERVELOCAL (CDdmCMB, CMB_SET_MIPS_STATE   );


//  special table for mapping from TySlot to "contiguous slot" index

const TySlot  CDdmCMB::m_aeContigToSlot [CT_IOPST_MAX_IOP_SLOTS]  =  {
   IOP_HBC0,  IOP_HBC1,
   IOP_SSDU0, IOP_SSDU1, IOP_SSDU2, IOP_SSDU3, 
   IOP_SSDL0, IOP_SSDL1, IOP_SSDL2, IOP_SSDL3, 
   IOP_RAC0,  IOP_APP0,  IOP_APP1,  IOP_NIC0,
   IOP_RAC1,  IOP_APP2,  IOP_APP3,  IOP_NIC1,
   CMB_EVC0,  CMB_EVC1,
   CMB_DDH0,  CMB_DDH1,  CMB_DDH2,  CMB_DDH3 
};

//  how many entries there are (or should be) in m_aeContigToSlot
const U32     CDdmCMB::m_cContigSlots  =  CT_IOPST_MAX_IOP_SLOTS;



//  *** here is some dummy data for describing what's in the various IOP slots
//      (with no CMB hardware, we don't have any easy way of discovering the
//       real data)

//  array must have as many entries as m_aeContigToSlot[]

typedef struct {

   IopState    eState;
   IopType     eType;
   const char *pszSerialNo;
} IopSlotEntry;

//  this array is the master, of which PTS is only a shadow.  [3/31/99 only]
//  Of course, in the real Odyssey, PTS will be the master copy.
//  This array is indexed by "contiguous slot ID", as returned by ValidIopSlot()
static IopSlotEntry   aIopSlot [CT_IOPST_MAX_IOP_SLOTS]  =  {
   {  IOPS_BOOTING,        IOPTY_HBC,     "HBC.123"  },  // IOP_HBC0
   {  IOPS_BLANK  },                                     // IOP_HBC1
//   {  IOPS_POWERED_DOWN,   IOPTY_SSD,     "SSD.122"  },  // IOP_SSDU0
   {  IOPS_BLANK  },                                     // IOP_SSDU0
   {  IOPS_BLANK  },                                     // IOP_SSDU1
   {  IOPS_BLANK  },                                     // IOP_SSDU2
   {  IOPS_BLANK  },                                     // IOP_SSDU3
   {  IOPS_BLANK  },                                     // IOP_SSDL0
   {  IOPS_BLANK  },                                     // IOP_SSDL1
   {  IOPS_BLANK  },                                     // IOP_SSDL2
   {  IOPS_BLANK  },                                     // IOP_SSDL3
   {  IOPS_POWERED_DOWN,   IOPTY_RAC,     "RAC.003"   }, // IOP_RAC0
   {  IOPS_BLANK  },                                     // IOP_APP0
   {  IOPS_BLANK  },                                     // IOP_APP1
   {  IOPS_BLANK  },                                     // IOP_NIC0
   {  IOPS_BLANK  },                                     // IOP_RAC1
   {  IOPS_BLANK  },                                     // IOP_APP2
   {  IOPS_BLANK  },                                     // IOP_APP3
   {  IOPS_POWERED_DOWN,   IOPTY_NIC,     "NIC.006"   }, // IOP_NIC1
   {  IOPS_BLANK  },                                     // CMB_EVC0
   {  IOPS_BLANK  },                                     // CMB_EVC1
   {  IOPS_BLANK  },                                     // CMB_DDH0
   {  IOPS_BLANK  },                                     // CMB_DDH1
   {  IOPS_BLANK  },                                     // CMB_DDH2
   {  IOPS_BLANK  },                                     // CMB_DDH3
};


//  here's a another bit of dummy data, for easing prompt generation
//  in our 3/31 incarnation.  This table is accessed using our
//  contiguous slot index convention.

static const char * apszSlotName []  =  {
   "HBC0",     // IOP_HBC0
   "HBC1",     // IOP_HBC1
   "SSDU0",    // IOP_SSDU0
   "SSDU1",    // IOP_SSDU1
   "SSDU2",    // IOP_SSDU2
   "SSDU3",    // IOP_SSDU3 
   "SSDL0",    // IOP_SSDL0
   "SSDL1",    // IOP_SSDL1
   "SSDL2",    // IOP_SSDL2
   "SSDL3",    // IOP_SSDL3 
   "RAC0",     // IOP_RAC0
   "APP0",     // IOP_APP0
   "APP1",     // IOP_APP1
   "NIC0",     // IOP_NIC0
   "RAC1",     // IOP_RAC1
   "APP2",     // IOP_APP2
   "APP3",     // IOP_APP3
   "NIC1",     // IOP_NIC1
   "EVC0",     // CMB_EVC0
   "EVC1",     // CMB_EVC1
   "DDH0",     // CMB_DDH0
   "DDH1",     // CMB_DDH1
   "DDH2",     // CMB_DDH2
   "DDH3",     // CMB_DDH3 
};


//  a (private) globally accessible copy of our instance
CDdmCMB   * CDdmCMB::m_pLocalCmbDdm  =  NULL;


//
//  CDdmCMB::CDdmCMB (did)
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

CDdmCMB::CDdmCMB (DID did) : Ddm (did),
                             m_CmbHwIntf (this, eSigAtmelMsgWaiting,
                                                eSigAtmelReserved),
                             m_CmdServer (CMB_CONTROL_QUEUE,
                                          CMB_CONTROL_COMMAND_SIZE,
                                          CMB_CONTROL_STATUS_SIZE,
                                          this,
                                          CMDCALLBACK (CDdmCMB, CmdReady)),
                             m_EventAssembler (this)
{


   Tracef("CDdmCMB::CDdmCMB()\n");
//   SetConfigAddress(&config,sizeof(config)); // tell Ddm:: where my config area is

   //  save away our instance pointer in a global place
   //  (there should only ever be one instance of us on a given board)
   m_pLocalCmbDdm = this;

   //  we don't want to process unsolicited CMB input until we're all set up
   m_fAcceptUnsolicitedCmbInput = FALSE;

   //  and we especially don't want to handle slot status updates yet
   m_fSlotPollBegun = FALSE;

   m_eMyIopSlot = (TySlot) 0;    // (invalid, for now)
   m_WeAreMasterHbc = TRUE;      //BUGBUG - always, for now

//*   m_pIopStateListen = NULL;
   m_pUpdatedIopRow  = NULL;
   m_cbUpdatedIopRow = 0;

   memset (m_aIopStatusImage, 0, sizeof (m_aIopStatusImage));

   //  set up contiguous indices for EVCs
   ValidIopSlot (CMB_EVC0, m_aiContigEvc[0]);
   ValidIopSlot (CMB_EVC1, m_aiContigEvc[1]);

   return;

}  /* end of CDdmCMB::CDdmCMB (DID did) */

//
//  CDdmCMB::Ctor (did)
//
//  Description:
//    Our static, standard system-defined helper function.
//    This routine is called by CHAOS when it wants to create
//    an instance of CDdmCMB.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    CDdmCMB::Ctor - Returns a pointer to the new instance, or NULL.
//

/* static */
Ddm *CDdmCMB::Ctor (DID did)
{

   return (new CDdmCMB (did));

}  /* end of CDdmCMB::Ctor */

//
//  CDdmCMB::Initialize (pInitMsg)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    CDdmCMB::Enable().
//
//    If we're the HBC master, we create the PTS tables which we "own":
//    the IOP Status table and the EVC Status table.  We also populate
//    them with initial dummy rows, so that we have valid row IDs for
//    all rows.  The initial IOP row data is all given "unknown" status.
//    The EVC data is given valid dummy values.
//
//    For all boards' CMB DDM instances, we then let the CMB know that
//    we're open for business by settings our "mips state" to
//    "running OS."
//
//    Finally, we call our base class' Initialize(), after we complete
//    our local functionality.
//
//  Inputs:
//    pInitMsg - Points to message which triggered our DDM's fault-in.
//          This is always an "initialize" message.
//
//  Outputs:
//    CDdmCMB::Initialize - Returns OK if all is cool, else an error.
//

/* virtual */
STATUS CDdmCMB::Initialize (Message *pInitMsg)
{


   //  verify that code's checksum is ok
   OsHeap::CheckOs();

   //  similarly, do a baseline check on the heap as well
   OsHeap::CheckHeap();

   //  first thing we need to know is whether we're on the master HBC

   //  of course, in order to do that we have to make sure that our
   //  CMB interface code is properly wired up:

   //  here's where our CMB "message ready" signals go
   DispatchSignal(eSigAtmelMsgWaiting, SIGNALCALLBACK (CDdmCMB, CmbMsgReceived));

   //  (our interface should always report a CMB present, even if simulated)
   assert (m_CmbHwIntf.IsRealCmbPresent ());


   //  now send the "get board info" message to get params,
   //  including the master flag
   SendCmbMsg (pInitMsg, k_eCmbCmdGetBoardInfo, CMB_SELF, 
               CMBCALLBACK (CDdmCMB, CDdmCMB::Initialize2), 0);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMB::Initialize () */


//  We're called back with the results of sending a "get board info" command
//  to our local CMA.
//
//  Inputs:
//    pInitMsg - Original message parameter to Initialize().  We pass this
//             along from routine to routine as our "cookie" value.
//    status - Result of CMB request message send
//    rReply - Reply from CMB
//

void  CDdmCMB::Initialize2 (void *pvCookie, STATUS status,
                            const CmbPacket *pReply)
{

Message      * pInitMsg =  (Message *) pvCookie;
STATUS         ulRet;


   Tracef("CDdmCMB::Initialize2() status=%u\n", status);

   //  the following bitfield checks should be done by our caller, but the
   //  pointer might be null, if we never sent the command...
   assert ((status != CTS_SUCCESS) ||
           ((pReply != NULL) &&
            ((pReply->Hdr.bStatus & CmbStatCmd) == 0) &&
            ((pReply->Hdr.bStatus & CmbStatAck) != 0)));

   assert (m_CmbHwIntf.IsRealCmbPresent ());

   //  check various basics, and also verify that state is in our pre-
   //  "set mips state" condition.
   //BUGBUG - our state should always be k_eCmbMStBootingPCIImage.
   //  However, the Odyssey boot ROM is presently leaving the HBC's
   //  state set to k_eCmbMStPoweredOn, so we accept that too (for now).
   if ((status == CTS_SUCCESS) &&
       (pReply->Hdr.bCommand == k_eCmbCmdGetBoardInfo) &&
       (((pReply->Hdr.bStatus & CmbStatMipsState) == k_eCmbMStPoweredOn) ||
        ((pReply->Hdr.bStatus & CmbStatMipsState) == k_eCmbMStBootingPCIImage)))
      {
      //  good, got a healthy reply.
      assert (pReply->Hdr.cbData == 3);

      //  all instances of the CMB DDM need to know their CMB slot number
      m_eMyIopSlot = (TySlot) pReply->abTail[1];

      //  (late-breaking development: the boot ROM should already have
      //   left this info in Address::)
      assert (m_eMyIopSlot == Address::iSlotMe);

      //  let our CMB message-sending helper know about this too..
      CCmbMsgSender::SetSourceAddr (m_eMyIopSlot);

      //  both master and non-master instances may need to support
      //  send-small-message:
      //  (this def must have a matching SERVELOCAL() macro def
      //   at the head of this source file!)
      DispatchRequest(CMB_SEND_SMALL_MSG,
                      REQUESTCALLBACK (CDdmCMB, ReqSendSmallMsg));

      //  similarly, each board's virtual master must be able to change our
      //  state to Operating, and when a suspend comes along, we need to
      //  mark that too (somebody in the failover dept. does it, I guess)
      DispatchRequest (CMB_SET_MIPS_STATE,
                       REQUESTCALLBACK (CDdmCMB, ReqSetMipsState));


      //  declare the master HBC-only request codes which we service:
      //  (these defs must have matching SERVELOCAL() macro defs
      //   at the head of this source file!).  These defs are always
      //  These dispatches are always done, since we never know when
      //  we might suddenly find ourselves the new master due to
      //  HBC failover.
      DispatchRequest(CMB_UPDATE_IOP_STATUS,
                      REQUESTCALLBACK (CDdmCMB, ReqUpdateIopStatus));
      DispatchRequest(CMB_IOP_CONTROL,
                      REQUESTCALLBACK (CDdmCMB, ReqIopControl));
      DispatchRequest(CMB_EVC_CONTROL,
                      REQUESTCALLBACK (CDdmCMB, ReqEvcControl));
      DispatchRequest(CMB_EVC_POWER_CONTROL,
                      REQUESTCALLBACK (CDdmCMB, ReqEvcPowerControl));
      DispatchRequest(CMB_DDH_CONTROL,
                      REQUESTCALLBACK (CDdmCMB, ReqDdhControl));
      DispatchRequest(CMB_POLL_ALL_IOPS,
                      REQUESTCALLBACK (CDdmCMB, ReqPollAllIops));
      DispatchRequest(CMB_POLL_ENVIRONMENT,
                      REQUESTCALLBACK (CDdmCMB, ReqPollEnvironment));

      //  and we *really* need to know if we're on the master HBC
      m_WeAreMasterHbc = ((pReply->abTail[2] & 1) == 1);

      if (m_WeAreMasterHbc)
         {
         assert ((pReply->abTail[0] == IOPTY_HBC) ||
                 (pReply->abTail[0] == IOPTY_HBC_TRI));

         assert ((pReply->abTail[1] == IOP_HBC0) ||
                 (pReply->abTail[1] == IOP_HBC1));


         //  this is where incoming "forward an event log entry" signals go
         //BUGBUG - this dispatch should only really be available on non-
         //  HBCs, but we make it available on HBCs for CmbTest debug help
         DispatchSignal (eSigSendEventLogEntry,
                         SIGNALCALLBACK (CDdmCMB, SendEventLogEntry));

         //  initialize our control interface..
         ulRet = m_CmdServer.csrvInitialize (INITIALIZECALLBACK (CDdmCMB,
                                                                 Initialize3));
         if (ulRet != CTS_SUCCESS)
            {
            assert (ulRet == CTS_SUCCESS);

            //  we're toast, stop right here
            Reply (pInitMsg, ulRet);

            return;
            }
         else
            {
            //  got our command queue interface ready, now let's
            //  go for our PTS setup (IOP Status table is first.)
            InitializeIopPts (pInitMsg);
            }
         }
      else
         {
         //  we're not running on the master HBC.  This means that we
         //  do want to receive the "forward a log entry" signal:

         //  this is where incoming "forward an event log entry" signals go
         DispatchSignal (eSigSendEventLogEntry,
                         SIGNALCALLBACK (CDdmCMB, SendEventLogEntry));

         //  since we're not the HBC master, we're mostly done with init.

         //  so now, let the CMB know that we're running OS-level code.
         //  (also handles Reply() to our init message)

         ReportOsRunningToCma (pInitMsg);
         }
      }
   else
      {
      //BUGBUG - what do we do if we can't talk to the CMB?
      //  How can we force a fault on the current board (IOP or HBC)?
      assert (FALSE);

      if (status == CTS_SUCCESS)
         {
         //  we're definitely not feeling successful today
         status = CTS_CMB_CMA_REQ_FAILED;
         }

      //  let init requestor know that thing's done busted.
      Reply (pInitMsg, status);
      }

   //  being a DDM callback, we're always supposed to return success.
   //  Why do we bother with return codes?
   return;

}  /* end of CDdmCMB::Initialize2 */


//  We are called back from m_CmdServer.csrvInitialize(), in Initialize2().
//  Note that since there is no cookie parameter to csrvInitialize, we
//  treat this callback as a "stub" rather than part of our main flow
//  of initialization.
//
//  Inputs:
//    ulStatus - Result of initialize operation.
//

void  CDdmCMB::Initialize3 (STATUS ulStatus)
{


   //  as a stub, all we can do is flag if init went bad, no init message
   //  available to reply to here.
   assert (ulStatus == CTS_SUCCESS);

   return;

}  /* end of CDdmCMB::Initialize3 */

//
//  CDdmCMB::InitializeIopPts (pInitMsg)
//
//  Description:
//    Called by our main initialization code, iff we're running on
//    the current master HBC.  This routine and its successors
//    take care of initializing the CMB DDM's various PTS tables
//    (IOP Status, EVC Status, EVC Raw Parameters).  We are careful
//    to perform the initialization in a failover-friendly way, not
//    blasting any data which may already be in place.
//
//    When we successfully complete PTS initialization, we then send
//    our local CMA the "mips state" change message, flagging that
//    we are fully initialized.  In turn, this enables the CMA to
//    send us gobs and gobs of "unsolicited" notifications about the
//    status of CMB slots and the modules found in them.
//
//    Finally, we call our base class' Initialize(), after we complete
//    our local functionality.
//
//  Inputs:
//    pInitMsg - Original message parameter to Initialize().  We pass this
//             along from routine to routine as our "cookie" value.
//
//  Outputs:
//    none
//

void  CDdmCMB::InitializeIopPts (Message *pInitMsg)
{

fieldDef     * pciIopStatusTable_FieldDefs;  // copy of defs which is sendable
TSDefineTable* pDefineTable;


   assert (m_WeAreMasterHbc);

   //  on initialize, we always try to create our IOP status table.
   //  Once the 'P' in PTS is true, this will mostly fail.  But it's
   //  a harmless (or even useful) failure.

   //  allocate a buffer which can be sent across IOP card boundaries
   pciIopStatusTable_FieldDefs = (fieldDef*)
                     new (tPCI) char [cbIopStatusTable_FieldDefs];

   OsHeap::CheckHeap();

   //  copy PTS field defs into sendable buffer
   memcpy (pciIopStatusTable_FieldDefs, aIopStatusTable_FieldDefs,
           cbIopStatusTable_FieldDefs);

   OsHeap::CheckHeap();

   //  build up a full PTS "define table" message
   pDefineTable = new TSDefineTable;

   //  (it is claimed that we'll never get a null pointer back from the heap;
   //   instead our host board would fail-away all by itself)

   //  load stuff into our define-table message
   pDefineTable -> Initialize (
               this,                      // DdmServices *
               CT_IOPST_TABLE_NAME,
               pciIopStatusTable_FieldDefs,
               cbIopStatusTable_FieldDefs,
               CT_IOPST_MAX_IOP_SLOTS,    // (num PTS rows to reserve)
               TRUE,                      // yes, we want persistence
               (pTSCallback_t) &InitializeIopPts2,  // where Send() reply goes
               pInitMsg);                 // preserve pInitMsg for our successor(s)

   //  send message, and we're done for now..
   //  [m_pDefineTable should take care of destroying itself inside Send()]
   pDefineTable -> Send();

   return;

}  /* end of CDdmCMB::InitializeIopPts */


//  We are called back from InitializeIopPts()'s define-table call
//  for the IOP Status table.  If things are ok, we light off our
//  row initialization loop
//
//  Inputs:
//    pInitMsg - Original message parameter to Initialize().  We pass this
//             along from routine to routine as our "cookie" value.
//    status - Result of PTS define-table operation.
//

STATUS CDdmCMB::InitializeIopPts2 (Message *pInitMsg, STATUS status)
{

const U32         cIopSlotInitRows  =  sizeof (aIopSlot) / sizeof (*aIopSlot);
IopInitCtx      * pInitCtx;


//*   Tracef("CDdmCMB::InitializeIopPts2() status=%u, crowIop = %u\n",
//*          status, m_cIopRowsCreated);

   //  verify that code's checksum is ok
//   OsHeap::CheckOs();

   //  similarly, do a baseline check on the heap as well
   OsHeap::CheckHeap();

   //  got response from PTS, go on to the next step

   //  if we get some sort of error status, then what?
   //  Note that the table may already exist, in fact once PTS
   //  is actually persistent, the table most likely will.
   //  So we're perfectly happy to see a "table exists" error.
   if ((status != CTS_SUCCESS) && (status != ercTableExists))
      {
      //  our attempt to define the IOP Status table has failed.

      //  report that we failed initialization, if anybody cares
      Reply (pInitMsg, status);

      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  got our table defined, now let's launch into querying for rows
   //  and adding those which aren't there yet (this covers the failover
   //  and persistence cases).

   pInitCtx = new IopInitCtx (pInitMsg);

   return (InitializeIopPts3 (pInitCtx, CTS_SUCCESS));

}  /* end of CDdmCMB::InitializeIopPts2 */


//  We are called back variously from InitializeIopPts4()'s PTS calls,
//  and from InitializeIopPts2() and InitializeIopPts3() when another row
//  is in need of creation.
//
//  We verify that the most recent IOP Status table row operation
//  went ok, and then move along to testing the next row for existence.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during IOP Status table building.
//             We pass this around as our cookie for a while.
//    status - Result of PTS read-row or insert-row operation.
//

STATUS CDdmCMB::InitializeIopPts3 (IopInitCtx *pInitCtx, STATUS status)
{

TSReadRow * pTsRead;
STATUS      sRet;


   //  see whether previous operation succeeded
   if (status == CTS_SUCCESS)
      {
      //  cool, let's move on to see if next row in table exists.

      if (pInitCtx->iContigSlot < m_cContigSlots)
         {
         //  got more rows to do.

         //  find current row's CMB slot ID
         pInitCtx->eSlot = m_aeContigToSlot [pInitCtx->iContigSlot];

         //  first see if current row already exists:  we'll read up to
         //  two rows, just to see if more than one is defined for a
         //  given slot key value.  [Should never happen.]
         pTsRead = new TSReadRow;
         sRet = pTsRead->Initialize(this, CT_IOPST_TABLE_NAME,
                                    CT_IOPST_SLOT, &pInitCtx->eSlot,
                                    sizeof (pInitCtx->eSlot),
                                    pInitCtx->u.aIopRec,
                                    sizeof (pInitCtx->u.aIopRec),
                                    &pInitCtx->cRowsRead,
                                    (pTSCallback_t) &InitializeIopPts4,
                                    pInitCtx);

         assert (sRet == CTS_SUCCESS);

         //  send off request (always calls callback & deletes *pTsRead)
         pTsRead->Send();
         }
      else
         {
         //  all done with IOP Status table, now move on to EVC Status:
         InitializeEvcPts (pInitCtx);
         }
      }
   else
      {
      //  whoops, couldn't insert new row.  (We're only called for
      //  read-row when it has succeeded).
      assert (status == CTS_SUCCESS);

      //  abort initialization as best we may
      Reply (pInitCtx->pInitMsg, status);

      //  and dispose of context cookie
      delete pInitCtx;

      //  fall into standard return():
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeIopPts3 */


//  We are called back from InitializeIopPts3()'s read-row test operation
//  on the IOP Status table.  If the current row doesn't yet exist,
//  then we initiate its insertion into the table.  Otherwise, we
//  call InitializeIopPts3() again to proceed with the next row's
//  validation.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during IOP Status table building.
//             We pass this around as our cookie for a while.
//    status - Result of PTS read-row operation.
//

STATUS CDdmCMB::InitializeIopPts4 (IopInitCtx *pInitCtx, STATUS status)
{

IOPStatusRecord * pNewIopStatusRow;
TSInsertRow     * pInsertRow;
STATUS            sMyRet;


   //  did read-row find an extant row for our current slot?
   if ((status == CTS_SUCCESS) && (pInitCtx->cRowsRead > 0))
      {
      //  yup, read-row found row(s) already in status table
      assert (pInitCtx->cRowsRead == 1);

      //  grab first (should be only) row's RID as our access key
      m_aIopStatusImage[pInitCtx->iContigSlot].rid =
                  pInitCtx->u.aIopRec[0].rid;

      //  bump our row count, and move on to next row
      pInitCtx->iContigSlot ++;

      //  test next row to see if it exists, etc.
      InitializeIopPts3 (pInitCtx, status);
      }
   else
      {
      //  current target row doesn't exist yet, so try creating it:

      //  define standard IOP Status table row
      pNewIopStatusRow = BuildCurrentIopStatus (pInitCtx->iContigSlot);

      //  copy new row data into our working context
      pInitCtx->u.aIopRec[0] = *pNewIopStatusRow;

      //  & dispose of buffer just allocated by BuildCurrentIopStatus().
      //  Yes, this is silly, but it works best given historical issues
      //  (and only happens during DDM initialization).
      delete pNewIopStatusRow;

      //  create a new InsertRow message, and load it up with our row stuff
      pInsertRow = new TSInsertRow;

      //  add our new-row data to the add-row message
      sMyRet = pInsertRow -> Initialize (
         this,
         CT_IOPST_TABLE_NAME,
         pInitCtx->u.aIopRec,                // row data
         sizeof (pInitCtx->u.aIopRec[0]),    // its size
         &(m_aIopStatusImage[pInitCtx->iContigSlot].rid),   // where to stash
                                                            // new row's rowID
         (pTSCallback_t) &InitializeIopPts3, // our standard loop target
         pInitCtx);                          // keep init context available
      assert (sMyRet == CTS_SUCCESS);

      //  bump our "rows created" count, even though we don't know for sure
      //  that the new row will be created.
      //  (we need to increment it -- it is our "loop counter")
      pInitCtx->iContigSlot ++;

      //  all ready, send off the message object
      pInsertRow -> Send ();
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeIopPts4 */

//
//  CDdmCMB::InitializeEvcPts (pInitCtx)
//
//  Description:
//    Called by our IOP PTS initialization code, when it has finished
//    with the IOP Status table.
//
//    We take care of the EVC Raw Parameters table initialization,
//    along with our successor routines.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during EVC PTS table building.
//             We pass this around as our cookie for a while.
//
//  Outputs:
//    none
//

void  CDdmCMB::InitializeEvcPts (IopInitCtx *pInitCtx)
{

TSDefineTable   * pDefineTable;
STATUS            sMyRet;


   //  first, let's do the EVC Raw Parameters table
   //  [we now assume that we're located on the same IOP as PTS,
   //   so we don't need to allocate from any magic memory pool]

   //  first, reset our rows-processed counter (this table has
   //   two rows)
   pInitCtx->eSlot = CMB_EVC0;

   //  build up a full PTS "define table" message -- we always
   //  attempt the table definition.  If the table already exists,
   //  then this will fail harmlessly.
   pDefineTable = new TSDefineTable;

   //  load stuff into our define-table message
   sMyRet = pDefineTable -> Initialize (
               this,                      // DdmServices *
               CT_EVC_RAW_PARAM_TABLE,
               CtEVCRawParameterRecord::FieldDefs (),
               CtEVCRawParameterRecord::FieldDefsSize (),
               2,                         // (num PTS rows to reserve)
               FALSE,                     // table is not persistent
               (pTSCallback_t) &InitializeEvcPts2,
               pInitCtx);                 // preserve pInitCtx
   assert (sMyRet == CTS_SUCCESS);

   //  send message, and we're done for now..
   //  [*pDefineTable should take care of destroying itself inside Send()]
   pDefineTable -> Send();

   //  all done with whatever we were going to do
   return;
    
}  /* end of CDdmCMB::InitializeEvcPts */


//  We are called back from InitializeEvcPts()'s define-table call
//  for the EVC Raw Params table.  If things are ok, we light off our
//  row initialization loop
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during EVC PTS table building.
//             We pass this around as our cookie for a while.
//    status - Result of PTS define-table operation.
//

STATUS CDdmCMB::InitializeEvcPts2 (IopInitCtx *pInitCtx, STATUS status)
{


//*   Tracef("CDdmCMB::InitializeEvcPts2() status=%u, crowIop = %u\n",
//*          status, m_cIopRowsCreated);

   //  verify that code's checksum is ok
//   OsHeap::CheckOs();

   //  similarly, do a baseline check on the heap as well
   OsHeap::CheckHeap();

   //  got response from PTS, go on to the next step

   //  Note that the table may already exist, in fact once PTS
   //  is actually persistent, the table most likely will.
   //  So we're perfectly happy to see a "table exists" error.
   if ((status != CTS_SUCCESS) && (status != ercTableExists))
      {
      //  our attempt to define the EVC Raw Params table has failed.

      //  report that we failed initialization, if anybody cares
      Reply (pInitCtx->pInitMsg, status);

      //  and dispose of context cookie
      delete pInitCtx;

      return (CTS_SUCCESS);      // (must *always* return success)
      }

   //  got our table defined, now let's launch into querying for rows
   //  and adding those which aren't there yet (this covers the failover
   //  and persistence cases).

   return (InitializeEvcPts3 (pInitCtx, CTS_SUCCESS));

}  /* end of CDdmCMB::InitializeEvcPts2 */


//  We are called back variously from InitializeEvcPts4()'s PTS calls,
//  and from InitializeEvcPts2() and InitializeEvcPts3() when another row
//  is in need of creation.
//
//  We verify that the most recent EVC Raw Params table row operation
//  went ok, and then move along to testing the next row for existence.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during EVC PTS table building.
//             We pass this around as our cookie for a while.
//    status - Result of PTS read-row or insert-row operation.
//

STATUS CDdmCMB::InitializeEvcPts3 (IopInitCtx *pInitCtx, STATUS status)
{

TSReadRow * pTsRead;
STATUS      sRet;


   //  see whether previous operation succeeded
   if (status == CTS_SUCCESS)
      {
      //  cool, let's move on to see if next row in table exists.

      if (pInitCtx->eSlot <= CMB_EVC1)
         {
         //  got more rows to do.

         //  first see if current row already exists:  we'll read up to
         //  two rows, just to see if more than one is defined for a
         //  given slot key value.  [Should never happen.]
         pTsRead = new TSReadRow;
         sRet = pTsRead->Initialize(this, CT_EVC_RAW_PARAM_TABLE,
                                    CT_EVCRP_EVCSLOTID,
                                    &pInitCtx->eSlot,
                                    sizeof (pInitCtx->eSlot),
                                    pInitCtx->u.aEvcRaw,
                                    sizeof (pInitCtx->u.aEvcRaw),
                                    &pInitCtx->cRowsRead,
                                    (pTSCallback_t) &InitializeEvcPts4,
                                    pInitCtx);

         assert (sRet == CTS_SUCCESS);

         //  send off request (always calls callback & deletes *pTsRead)
         pTsRead->Send();
         }
      else
         {
         //  all done with EVC Raw Params table

         //  now move on to Env Control table
         InitializeEnvCtlPts (pInitCtx);
         }
      }
   else
      {
      //  whoops, couldn't insert new row.  (We're only called for
      //  read-row when it has succeeded).
      assert (status == CTS_SUCCESS);

      //  abort initialization as best we may
      Reply (pInitCtx->pInitMsg, status);

      //  and dispose of context cookie
      delete pInitCtx;

      //  fall into standard return():
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeEvcPts3 */


//  We are called back from InitializeEvcPts3()'s read-row test operation
//  on the EVC Raw Params table.  If the current row doesn't yet exist,
//  then we initiate its insertion into the table.  Otherwise, we
//  call InitializeEvcPts3() again to proceed with the next row's
//  validation.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during EVC PTS table building.
//             We pass this around as our cookie for a while.
//    status - Result of PTS read-row operation.
//

STATUS CDdmCMB::InitializeEvcPts4 (IopInitCtx *pInitCtx, STATUS status)
{

TSInsertRow  * pInsertRow;
STATUS         sMyRet;


   //  did read-row find an extant row for our current slot?
   if ((status == CTS_SUCCESS) && (pInitCtx->cRowsRead > 0))
      {
      //  yup, read-row found row(s) already in status table
      assert (pInitCtx->cRowsRead == 1);

      //  bump our row count (unlike IOP init, we only use eSlot)
      pInitCtx->eSlot = (TySlot) (pInitCtx->eSlot + 1);

      //  test next row to see if it exists, etc.
      InitializeEvcPts3 (pInitCtx, status);
      }
   else
      {
      //  current target row doesn't exist yet, so try creating it:

      //  define a positively minimal EVC Raw Params table row
      //  (it will be filled in properly once the environment
      //   reporter starts asking us to poll the environment)
      pInitCtx->u.aEvcRaw[0].EvcSlotId = pInitCtx->eSlot;

      //  create a new InsertRow message, and load it up with our row stuff
      pInsertRow = new TSInsertRow;

      //  add our new-row data to the add-row message
      sMyRet = pInsertRow -> Initialize (
         this,
         CT_EVC_RAW_PARAM_TABLE,
         pInitCtx->u.aEvcRaw,                // row data
         sizeof (pInitCtx->u.aEvcRaw[0]),    // its size
         &pInitCtx->ridDummy,                // someplace to put new rows rid
         (pTSCallback_t) &InitializeEvcPts3, // our standard loop target
         pInitCtx);                          // keep init context available
      assert (sMyRet == CTS_SUCCESS);

      //  bump our "rows created" count, even though we don't know for sure
      //  that the new row will be created.
      //  (we need to increment it -- it is our "loop counter")
      pInitCtx->eSlot = (TySlot) (pInitCtx->eSlot + 1);

      //  all ready, send off the message object
      pInsertRow -> Send ();
      }

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeEvcPts4 */

//
//  CDdmCMB::InitializeEnvCtlPts (pInitCtx)
//
//  Description:
//    Called by our EVC PTS initialization code, when it has finished
//    with the EVC Raw Parameter table.
//
//    We take care of the Environment Control Table initialization,
//    with a little help from our successor routines.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer.
//             We pass this around as our cookie for a while.
//
//  Outputs:
//    none
//

void  CDdmCMB::InitializeEnvCtlPts (IopInitCtx *pInitCtx)
{

CtEnvControlRecord::RqDefineTable * preqDefTab;
STATUS   sRet;


   //  attempt table definition, in case we're dealing with a fresh PTS
   preqDefTab = new CtEnvControlRecord::RqDefineTable (Persistant_PT,
                                                       1);     // only 1 row

   sRet = Send (preqDefTab, pInitCtx,
                REPLYCALLBACK (CDdmCMB, InitializeEnvCtlPts2));

   assert (sRet == CTS_SUCCESS);

   return;

}  /* end of CDdmCMB::InitializeEnvCtlPts */


//  We are called back from InitializeEnvCtlPts() when our define table
//  operation for the Env Control Table has completed.
//
//  We pretty much ignore the define table results, and proceed to check
//  whether any row is already defined in the table (which might itself
//  already have existed, thus our disdain for define table's response).
//
//  Inputs:
//    pReply - Reply to our define table request.
//

STATUS  CDdmCMB::InitializeEnvCtlPts2 (Message *pReply)
{

IopInitCtx   * pInitCtx;
CtEnvControlRecord::RqEnumTable   * preqEnumTab;
STATUS         sRet;


   assert (pReply != NULL);

   pInitCtx = (IopInitCtx *) pReply->GetContext();

   assert (pInitCtx != NULL);

   //  simply dispose of define-table evidence
   delete pReply;

   //  build up enum-table request, asking for two rows (should only be one)

   preqEnumTab = new CtEnvControlRecord::RqEnumTable (
                                    0,       // start at first row
                                    2);      // ask for two rows

   sRet = Send (preqEnumTab, pInitCtx,
                REPLYCALLBACK (CDdmCMB, InitializeEnvCtlPts3));

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeEnvCtlPts2 */


//  We are called back from InitializeEnvCtlPts2() when our enum table
//  operation for the Env Control Table has completed.
//
//  We check to see whether any rows were enumerated: we expect either
//  zero or one row.  If one, we save away its RowId and we're done.
//  If zero, we proceed to create a new row in the table, with safe defaults.
//
//  Inputs:
//    pmsgReply - Reply to our enumerate table request.
//

STATUS  CDdmCMB::InitializeEnvCtlPts3 (Message *pmsgReply)
{

CtEnvControlRecord::RqEnumTable   * pReply;
IopInitCtx   * pInitCtx;
U32            cRows;
CtEnvControlRecord::RqInsertRow   * preqInsert;
STATUS         sRet;


   pReply = (CtEnvControlRecord::RqEnumTable *) pmsgReply;
   assert (pReply != NULL);

   pInitCtx = (IopInitCtx *) pReply->GetContext();
   assert (pInitCtx != NULL);

   assert (pReply->Status() == CTS_SUCCESS);

   if (pReply->Status() != CTS_SUCCESS)
      {
      //  whoopsie, we're toast
      Reply (pInitCtx->pInitMsg, pReply->Status());
      delete pReply;
      delete pInitCtx;
      return (CTS_SUCCESS);
      }

   cRows = pReply->GetRowCount();

   assert ((cRows == 0) || (cRows == 1));

   if (cRows > 0)
      {
      //  already got a row; save it's RowId, and whole record for EVC init
      m_ridEnvCtl = pReply->GetRowPtr()->rid;

      pInitCtx->EnvCtlRec = *pReply->GetRowPtr();

      //  [also save configured fan speeds in our local cache]
      m_aulFanSpeedSet[0] = pReply->GetRowPtr()->FanSpeedSet[0];
      m_aulFanSpeedSet[1] = pReply->GetRowPtr()->FanSpeedSet[1];

      //  now go off and load the env ctl values to the EVCs
      InitializeEnvCtlPtsDone (pInitCtx);
      }
   else
      {
      //  no row yet, so let's make one up

      assert (pReply->GetRowDataSize() == 0);        // should be no data

      //  set up nice, safe defaults for record
      pInitCtx->EnvCtlRec.FanSpeedSet[0] = 25;        // % of full speed
      pInitCtx->EnvCtlRec.FanSpeedSet[1] = 25;        // (i.e., 1/4 speed)

      pInitCtx->EnvCtlRec.ExitTempFanUpThresh = 60 * 2;  // degrees C, 0.5C lsb
      pInitCtx->EnvCtlRec.ExitTempFanNormThresh = 55 * 2;

      //  save away a copy of fan speed settings in our cache too
      m_aulFanSpeedSet[0] = pInitCtx->EnvCtlRec.FanSpeedSet[0];
      m_aulFanSpeedSet[1] = pInitCtx->EnvCtlRec.FanSpeedSet[1];

      //  make up an insert msg
      preqInsert = new CtEnvControlRecord::RqInsertRow (&pInitCtx->EnvCtlRec,
                                                        1);

      sRet = Send (preqInsert, pInitCtx,
                   REPLYCALLBACK (CDdmCMB, InitializeEnvCtlPts4));
      assert (sRet == CTS_SUCCESS);
      }

   //  either way, all done with our own reply, so dump it
   delete pReply;

   return (CTS_SUCCESS);      // (what good little callbacks always say)

}  /* end of CDdmCMB::InitializeEnvCtlPts3 */


//  We are called back from InitializeEnvCtlPts3() when our insert row
//  operation for the Env Control Table has completed.
//
//  We verify that the insert went ok, and then save away the new row's
//  RowId.
//
//  Inputs:
//    pmsgReply - Reply to our insert row request.
//

STATUS  CDdmCMB::InitializeEnvCtlPts4 (Message *pmsgReply)
{

CtEnvControlRecord::RqInsertRow   * pReply;
IopInitCtx   * pInitCtx;
STATUS         sRet;
rowID        * pRid;


   pReply = (CtEnvControlRecord::RqInsertRow *) pmsgReply;
   assert (pReply != NULL);

   pInitCtx = (IopInitCtx *) pReply->GetContext();

   assert (pInitCtx != NULL);

   sRet = pReply->Status();
   if ((sRet != CTS_SUCCESS) ||
       (pReply->GetRowIdDataCount() != 1))
      {
      //  whoopsie, we're toast
      if (sRet == CTS_SUCCESS)
         {
         //  wasn't really successful; PTS merely claims that it
         //  successfully didn't insert our row.  Gagh.
         sRet = CTS_CMB_PTS_INSERT_FAILED;
         }
      Reply (pInitCtx->pInitMsg, sRet);
      delete pReply;
      delete pInitCtx;
      return (CTS_SUCCESS);
      }

   //  so far so good, now save away new record's RowId
   pRid = pReply->GetRowIdDataPtr ();
   assert (pRid != NULL);

   m_ridEnvCtl = *pRid;

   //  we've saved data, now dispose of reply
   delete pReply;

   //  and do our common cleanup
   InitializeEnvCtlPtsDone (pInitCtx);

   return (CTS_SUCCESS);

}  /* end of CDdmCMB::InitializeEnvCtlPts4 */


//  We are called from InitializeEnvCtlPts3() or InitializeEnvCtlPts4(),
//  when they realize that we are done with the Env Control Table init.
//
//  We send our new settings off to the EVC, and then report back to our
//  original requestor.
//
//  Inputs:
//    pInitCtx - Carries our original Initialize() message pointer, and
//             some stuff useful during EVC PTS table building.
//             We pass this around as our cookie for a while.
//

void  CDdmCMB::InitializeEnvCtlPtsDone (IopInitCtx *pInitCtx)
{


   assert (pInitCtx != NULL);
   assert ((pInitCtx->EnvCtlRec.ExitTempFanUpThresh & ~0xFF) == 0);

   //  send fan-up threshold to both EVCs:
   SendEvcMsg (pInitCtx, k_eCmbCmdSetThreshold,
               CMBCALLBACK(CDdmCMB, InitializeEnvCtlPtsDone2),
               2, k_eCmbFanTempThreshFanUp,
               pInitCtx->EnvCtlRec.ExitTempFanUpThresh);

   return;

}  /* end of CDdmCMB::InitializeEnvCtlPtsDone */


//  We are called from InitializeEnvCtlPtsDone() when SendEvcMsg() has
//  finished sending the "fan up" EVC temperature.
//
//  We send the "fan restore" EVC command.
//
//  Inputs:
//    pvCookie - An aliased form of IopInitCtx, our cookie data.
//    sStatus - Results of SendEvcMsg().
//    pReply - Final result from SendEvcMsg; not especially meaningful.
//

void  CDdmCMB::InitializeEnvCtlPtsDone2 (void *pvCookie, STATUS sStatus,
                                         const CmbPacket *pReply)
{

#pragma unused(sStatus)
#pragma unused(pReply)

IopInitCtx   * pInitCtx = (IopInitCtx *) pvCookie;


   assert (pInitCtx != NULL);
   assert (sStatus == CTS_SUCCESS);

   //  got fan-up threshold set, now take care of fan-restore

   //  send fan-restore threshold to both EVCs:
   SendEvcMsg (pInitCtx, k_eCmbCmdSetThreshold,
               CMBCALLBACK(CDdmCMB, InitializeEnvCtlPtsDone3),
               2, k_eCmbFanTempThreshFanRestore,
               pInitCtx->EnvCtlRec.ExitTempFanNormThresh);

   return;

}  /* end of CDdmCMB::InitializeEnvCtlPtsDone2 */


//  We are called from InitializeEnvCtlPtsDone2() when SendEvcMsg() has
//  finished sending the "fan restore" EVC temperature.
//
//  We do final cleanup and let the requestor know what happens.
//
//  Inputs:
//    pvCookie - An aliased form of IopInitCtx, our cookie data.
//    sStatus - Results of SendEvcMsg().
//    pReply - Final result from SendEvcMsg; not especially meaningful.
//

void  CDdmCMB::InitializeEnvCtlPtsDone3 (void *pvCookie, STATUS sStatus,
                                         const CmbPacket *pReply)
{

#pragma unused(sStatus)
#pragma unused(pReply)

IopInitCtx   * pInitCtx = (IopInitCtx *) pvCookie;


   //  we're as done as we're going to be.
   //  Report results to original initialize requestor.

   //NOTE:  We do *not* set MIPS State to "running OS."  Doing this enables
   //    the CMA to send us unsolicited slot status updates, which in turn
   //    causes us to update the IOP Status table.  And we're not supposed
   //    to do that until somebody sends us a "poll all IOPs" message.
   //    So we do "set MIPS State" inside poll all IOPs instead.

   Reply (pInitCtx->pInitMsg, CTS_SUCCESS);

   //  and dispose of context cookie
   delete pInitCtx;

   return;

}  /* end of CDdmCMB::InitializeEnvCtlPtsDone2 */

//
//  CDdmCMB::Quiesce (pMsg)
//
//  Description:
//    Called when our DDM is supposed to enter the quiescent state.
//    We do any necessary tidying up, and then reply to acknowledge
//    the quiesce request.
//
//    ** Per the DDM model, we cannot receive any request messages
//       once we have entered our quiesce processing.  All new requests
//       are deferred until quiesce is "undone" by an enable message.
//
//    ** Note that replies are never blocked (or "deferred") -- they
//       will always come into the DDM as soon as they work their way
//       through the DDM instance's message queue.
//
//  Inputs:
//    pMsg - Points to quiesce message which triggered this call.
//             Must be replied to in order to signal quiesce completion.
//
//  Outputs:
//    CDdmCMB::Quiesce - Returns OK, or a highly descriptive error code.
//

STATUS CDdmCMB::Quiesce (Message *pMsg)
{ 


   Tracef("CDdmCMB::Quiesce() entered.\n");
   
   //  * * *  do local Quiesce stuff here.  * * *

   //  signal CHAOS that our DDM instance is finished quiescing.
   Reply (pMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMB::Quiesce */

//
//  CDdmCMB::Enable (pMsg)
//
//  Description:
//    Called during our DDM instance's initial fault-in [after Initialize()
//    has been called], and also called when our DDM is being reenabled
//    after a Quiesce() call had been made.
//
//    We read any configuration data which we ourselves need from PTS, since
//    it might have changed since we last ran.
//
//    We do whatever else we need to to render ourselves ready for business.
//    Then we signal CHAOS that we're done enable()ing, and ready to process
//    new request messages.
//
//    ** Per the DDM model, our instance will not be able to receive any
//       new request messages until after signalling completion of our
//       enable processing (by replying to our input message).
//
//    ** Note that we may receive reply messages at any time, so that our
//       initialize and/or enable processing may involve submitting requests
//       to other DDMs and waiting for their replies.
//
//  Inputs:
//    pMsg - Points to "enable" message which triggered this call.
//          Must be replied to in order to signal completion of Enable()
//          processing, and our willingness to accept new request messages.
//
//  Outputs:
//    CDdmCMB::Ctor - Returns a pointer to the new instance, or NULL.
//

STATUS CDdmCMB::Enable(Message *pMsg)
{ 


   Tracef("CDdmCMB::Enable() entered.\n");

   //  let base class know that we're done with enable phase
   //  (allows new requests to be processed by our DDM instance --
   //   until this call, CHAOS was holding them off from us to let
   //   us finish initialization / reply processing)
   Reply (pMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmCMB::Enable */

//
//  CDdmCMB::ForwardEventLogEntry (pEventEntry)
//
//  Description:
//    Used by non-CMB DDMs to relay a message to the Odyssey system's
//    master HBC, where it can be recorded in the master event log.
//
//    This routine uses the CHAOS Signal() mechanism to hand the event
//    instance to the CMB DDM instance which resides on the caller's IOP.
//    The CMB DDM then does the actual processing.
//
//    **  This routine hands off the Event instance to the local board's
//        CMB DDM instance.  It is only meant to be used on non-HBC IOPs.
//
//    *** This routine uses the CMB to transfer the packet.  This means
//        that it is spectacularly low-performance, and should only be
//        used as a last resort.
//
//  Inputs:
//    pEventEntry - Points to event log entry instance which we're
//          to relay to the master HBC.  The CMB DDM deletes this
//          instance after it has sent the event data to the HBC.
//
//  Outputs:
//    CDdmCMB::ForwardEventLogEntry - Returns CTS_SUCCESS, or some hint
//          as to why we failed.
//

/* static */
STATUS CDdmCMB::ForwardEventLogEntry (Event *pEventEntry)
{

STATUS   sMyRet;


//  (can't do this check, unless m_WeAreMasterHbc is static)
//   assert (! m_WeAreMasterHbc);

   assert (pEventEntry != NULL);

   if (pEventEntry != NULL)
      {
      //  got an event instance, assume it's valid

      //  hand the event instance off to the CMB DDM via Signal():
      m_pLocalCmbDdm -> Signal (eSigSendEventLogEntry, pEventEntry);

      sMyRet = CTS_SUCCESS;
      }
   else
      {
      //  tsk, NULL pointers are not part of the deal
      sMyRet = CTS_CMB_INVALID_PARAMETER;
      }

   return (sMyRet);

}  /* end of CDdmCMB::ForwardEventLogEntry */

//
//  CDdmCMB::ReportOsRunningToCma (pMsg)
//
//  Description:
//    Called when the CMB DDM has finished initialization, and is ready
//    to receive unsolicited inputs from the local CMA.
//
//    We send a "set MIPS state" message to our local CMA indicating
//    that we are now running OS-level code.  When that message completes,
//    successfully or otherwise, we then issue a Reply() to the caller's
//    given message to complete the message's processing.
//
//    Note that if we're running on the master HBC, we aren't called
//    until after the IOP Manager (or whoever) has called us to perform
//    our first IOP slot sweep.  We defer this call in order to prevent
//    unsolicited CMB slot status updates from causing us to alter the
//    IOP Status table before the IOP Manager has had a chance to capture
//    its prior state.
//
//  Inputs:
//    pMsg - Original message parameter to request which we are finishing.
//             We cart this along as our cookie until our CMB request
//             has completed, and then we send a reply to it reporting
//             our final status.
//
//  Outputs:
//    none
//

void  CDdmCMB::ReportOsRunningToCma (Message *pMsg)
{


   //  shut off our internal filtering of unsolicited messages
   m_fAcceptUnsolicitedCmbInput = TRUE;

//BUGBUG - here commences a major hack.  We do a direct request to our
//  own HBC's CMA ('cause it's easier than going to PTS) to find out
//  what its firmware revision is.  If it's down-rev, then we send the
//  old, obsolete "enable unsoliciteds" state, otherwise we send the
//  current, cool "enable unsoliciteds" state.  :-)
   if (m_WeAreMasterHbc)
      {
      //  we're on an HBC, so send it the request
//*      SendCmbMsg (pMsg, k_eCmbCmdGetLastValue, CMB_SELF,
      SendCmbMsg (pMsg, k_eCmbCmdGetLastValue, Address::iSlotMe,
                  CMBCALLBACK (CDdmCMB, ReportOsRunningToCma1),
                  1, k_eCmbParamCmaFirmwareInfo);
      }
   else
      {
      //  we're not an HBC, so directly call our dummy callback:
      ReportOsRunningToCma1 (pMsg, CTS_SUCCESS, NULL);
      }

   return;

}  /* dummy end of ReportOsRunningToCma() */


//BUGBUG - dummy routine split for workaround of AVR MIPS state mismatch
void  CDdmCMB::ReportOsRunningToCma1 (void *pvCookie, STATUS status,
                                      const CmbPacket *pReply)
{

CmbMipsState   eState;


//BUGBUG - the following is a HACK done to permit operation with
//         either old AVRs unaware of the (new) k_eCmbMStRunningOSImage
//         state, or new AVRs which do support it.
//         fLateModelHbcAvr should not exist, and the real value for
//         MIPS State here should always be k_eCmbMStLoadingOSImage.
const U8  bLastDownrevHbcAvrRevision = 0x10;
   if ((status == CTS_SUCCESS) && (pReply != NULL) &&
       (pReply->abTail[1] > bLastDownrevHbcAvrRevision))
      {
      //  got an up-rev HBC AVR, so give it the real state code.
      //  Of course, the real state code is supposed to be "loading",
      //  so this whole complex batch of code can just go away now.  :-)
//      eState = k_eCmbMStRunningOSImage;
      eState = k_eCmbMStLoadingOSImage;
      }
   else
      {
      //  got a non-HBC, or a down-rev AVR, so give the old bogus code
      eState = k_eCmbMStLoadingOSImage;
      }
//BUGBUG - endit (eState should always be == k_eCmbMStLoadingOSImage)

   //  send the "set mips state" message to announce that we're now up
   //  and running OS-level code (i.e., us :-)
   SendCmbMsg (pvCookie, k_eCmbCmdSetMipsState, CMB_SELF, 
               CMBCALLBACK (CDdmCMB, ReportOsRunningToCma2),
               1, eState);

   //  the rest happens in our callback

   return;

}  /* end of CDdmCMB::ReportOsRunningToCma */


//  We're called back with the results of sending a "set MIPS state" command
//  to our local CMA.
//
//  Inputs:
//    pReqMsg - Points to original message parameter (e.g., Initialize)
//             whose receipt triggered our eventual invocation.
//    status - Result of CMB request message send
//    rReply - Reply from CMB
//

void  CDdmCMB::ReportOsRunningToCma2 (void *pvCookie, STATUS status,
                                      const CmbPacket *pReply)
{

#pragma unused(pReply)

Message                        * pReqMsg  =  (Message *) pvCookie;
IOPStatusRecord::RqModifyField * pmsgModifyField;
U32                              iContigSlotMe;


   //  if we're the master HBC, we must also update our row in the
   //  IOP Status table, since the AVR doesn't loop notifications for
   //  our own state changes back to us.
   if (m_WeAreMasterHbc)
      {
      //  yup, do the update

      //  update our cached row info first, so we have a persistent copy
      //  of our new state code
      ValidIopSlot (m_eMyIopSlot, iContigSlotMe);
      m_aIopStatusImage[iContigSlotMe].eState = IOPS_LOADING;

      //  now build up field modify message, using newly updated state
      //  cache as source value (must persist across message send)
      pmsgModifyField = new IOPStatusRecord::RqModifyField
                              (m_aIopStatusImage[iContigSlotMe].rid,
                               CT_IOPST_IOPCURRENTSTATE,
                               &m_aIopStatusImage[iContigSlotMe].eState,
                               sizeof (m_aIopStatusImage[iContigSlotMe].eState));

      //  (our reply callback will notify requestor of completion)
      Send (pmsgModifyField, pReqMsg,
            REPLYCALLBACK (CDdmCMB, ReportOsRunningToCma3));
      }
   else
      {
      //  simply let original requestor know how things went
      Reply (pReqMsg, status);
      }

   return;

}  /* end of CDdmCMB::ReportOsRunningToCma2 */


//  We're called back with the results of sending a "modify field" command
//  to the PTS to update our own HBC's state.
//
//  Inputs:
//    pReply - Reply message from PTS, really an occulted version of
//             IOPStatusRecord::RqModifyField.
//

STATUS  CDdmCMB::ReportOsRunningToCma3 (Message *pReply)
{

Message   * pReqMsg;


   assert (pReply != NULL);

   //  recover original request message
   pReqMsg = (Message *) pReply->GetContext();
   assert (pReqMsg != NULL);

   //  send PTS' status back as our own (for lack of any better status)
   Reply (pReqMsg, pReply->Status());

   //  dispose of our PTS request message
   delete pReply;

   //  and we're done.
   return (CTS_SUCCESS);      // what reply callbacks always say

}  /* end of CDdmCMB::ReportOsRunningToCma3 */

//
//  CDdmCMB::SendCmbMsg (pvCookie, eCommand, bDest, pCmbCallback,
//                       cParams, bParam1, bParam2)
//
//  Description:
//    Called to send a message to the CMB interface.
//
//    We take care of all the stuff about setting up and sending the
//    message.  Our caller just needs to supply a standard CMBCALLBACK
//    entry point which we return control to after the operation.
//
//    Note that, in keeping with our underlying CCmbMsgSender worker,
//    we always call the user's callback routine, even in the event of
//    errors before reaching the local CMA.  Thus, this routine has
//    no return value.
//
//    ** Some error conditions might result in our directly calling
//       *pCmbCallback.  So the caller should not perform any
//       operations following this call (in the calling routine)
//       with the assumption that they will precede the invocation
//       of *pCmbCallback: they may in fact follow it.
//
//  Inputs:
//    pvCookie - Any old value, passed through to pCmbCallback() when
//                it is invoked.
//    eCommand - CMB command code for the message we send.
//    bDest - Standard CMB packet destination value (i.e., a TySlot code
//                with optional (CmbAddrMips) flag ORed in).
//    pCmbCallback - Callback routine which is invoked when the message
//                processing is complete (normal or error).
//    cParams - Count of parameter bytes in message.  Presently, this
//                routine only supports values 0 .. 2.
//    bParam1 - First param value.
//    bParam2 - Second param value.
//
//  Outputs:
//    none
//

void  CDdmCMB::SendCmbMsg (void *pvCookie, CmbHwCmdCode eCommand, U8 bDest,
                           CCmbMsgSender::CmbCallback pCmbCallback,
                           U8 cParams, U8 Param1 /*= 0*/ , U8 Param2 /*= 0*/)
{

CCmbMsgSender   * pCmbMsg;


   assert ((0 <= cParams) && (cParams <= 2));

   //  build up CMB command message
   pCmbMsg = new CCmbMsgSender (this, eCommand, bDest, cParams);

   if (cParams > 0)
      {
      pCmbMsg -> AddParam (0, Param1);

      if (cParams > 1)
         {
         pCmbMsg -> AddParam (1, Param2);
         }
      }

   //  send command off to CMB
   pCmbMsg -> Send (m_CmbHwIntf, pvCookie, pCmbCallback);

   return;

}  /* end of CDdmCMB::SendCmbMsg */

//
//  CDdmCMB::SendEvcMsg (pvCookie, eCommand, pCmbCallback,
//                       cParams, bParam1, bParam2)
//
//  Description:
//    Called to send a message to however many EVCs are presently
//    operating in the current Odyssey.  We send the same message
//    to both EVCs.  We will not call the supplied callback until
//    we have received answers from as many EVCs as are operating
//    in our system.
//
//    We report success via the callback only if all responding
//    EVCs issue positive acknowledges to our message.
//
//    We take care of all the stuff about setting up and sending the
//    message.  Our caller just needs to supply a standard CMBCALLBACK
//    entry point which we return control to after the operation.
//
//    Note that, in keeping with our underlying CCmbMsgSender worker,
//    we always call the user's callback routine, even in the event of
//    errors before reaching the local CMA.  Thus, this routine has
//    no return value.
//
//    ** Some error conditions might result in our directly calling
//       *pCmbCallback.  So the caller should not perform any
//       operations following this call (in the calling routine)
//       with the assumption that they will precede the invocation
//       of *pCmbCallback: they may in fact follow it.
//
//  Inputs:
//    pvCookie - Any old value, passed through to pCmbCallback() when
//                it is invoked.
//    eCommand - CMB command code for the message we send.
//    pCmbCallback - Callback routine which is invoked when the message
//                processing is complete (normal or error).
//    cParams - Count of parameter bytes in message.  Presently, this
//                routine only supports values 0 .. 2.
//    bParam1 - First param value.
//    bParam2 - Second param value.
//
//  Outputs:
//    none
//

void  CDdmCMB::SendEvcMsg (void *pvCookie, CmbHwCmdCode eCommand, 
                           CCmbMsgSender::CmbCallback pCmbCallback,
                           U8 cParams /* = 0 */,
                           U8 Param1 /* = 0 */, U8 Param2 /* = 0 */ )
{

CEvcSendContext * pSendCtx;
CCmbMsgSender   * pCmbMsg;
const U8          bDest = CMB_EVC0;


   assert ((0 <= cParams) && (cParams <= 2));

   //  make yet another context cookie, for tracking caller's command stuff
   pSendCtx = new CEvcSendContext (eCommand, cParams, Param1, Param2,
                                   pCmbCallback, pvCookie);

   //  we get to send to EVC0.  Is one present?
   if (m_aIopStatusImage[m_aiContigEvc[bDest - CMB_EVC0]].eState ==
                            IOPS_POWERED_ON)
      {
      //  yup, EVC is there and functional.  Scoot the message out to it.

      //  build up CMB command message
      pCmbMsg = new CCmbMsgSender (this, eCommand, bDest, cParams);

      if (cParams > 0)
         {
         pCmbMsg -> AddParam (0, Param1);

         if (cParams > 1)
            {
            pCmbMsg -> AddParam (1, Param2);
            }
         }

      //  send command off to CMB
      pCmbMsg -> Send (m_CmbHwIntf, pSendCtx,
                       CMBCALLBACK (CDdmCMB, SendEvcMsg2));
      }
   else
      {
      //  no EVC0 to talk to, so call our callback directly
      SendEvcMsg2 (pSendCtx, CTS_SUCCESS, NULL);
      }

   return;

}  /* CDdmCMB::SendEvcMsg */


//
//  We're called back with the response from EVC0, or with a dummy
//  success code if there is no EVC0 in our system.
//
//  Inputs:
//    pvSendCtx - Our send context cookie (carries caller's cookie in it).
//    sStatus - Status of send to EVC0.
//    pReply - Points to EVC0's response packet, or NULL if no EVC0.
//
void  CDdmCMB::SendEvcMsg2 (void *pvSendCtx, STATUS sStatus,
                            const CmbPacket *pReply)
{

CEvcSendContext * pSendCtx  =  (CEvcSendContext *) pvSendCtx;
CCmbMsgSender   * pCmbMsg;
const U8          bDest = CMB_EVC1;


   assert (pSendCtx != NULL);

   //  Note whether we got an ok response
   pSendCtx->m_sEvcResult = sStatus;

   if (pReply != NULL)
      {
      //  got a real reply, save it
      pSendCtx->m_pktEvcResult = *pReply;
      }

   //  we get to send to EVC1.  Is one present?
   if (m_aIopStatusImage[m_aiContigEvc[bDest - CMB_EVC0]].eState ==
                            IOPS_POWERED_ON)
      {
      //  yup, EVC is there and functional.  Scoot the message out to it.

      //  build up CMB command message
      pCmbMsg = new CCmbMsgSender (this, pSendCtx->m_eCommand, bDest,
                                   pSendCtx->m_cParams);

      if (pSendCtx->m_cParams > 0)
         {
         pCmbMsg -> AddParam (0, pSendCtx->m_bParam1);

         if (pSendCtx->m_cParams > 1)
            {
            pCmbMsg -> AddParam (1, pSendCtx->m_bParam2);
            }
         }

      //  send command off to CMB
      pCmbMsg -> Send (m_CmbHwIntf, pSendCtx,
                       CMBCALLBACK (CDdmCMB, SendEvcMsg3));
      }
   else
      {
      //  no EVC0 to talk to, so call our callback directly
      SendEvcMsg3 (pSendCtx, CTS_SUCCESS, NULL);
      }

   return;

}  /* end of CDdmCMB::SendEvcMsg2 */


//
//  We're called back with the response from EVC1, or with a dummy
//  success code if there is no EVC1 in our system.
//
//  Inputs:
//    pvSendCtx - Our send context cookie (carries caller's cookie in it).
//    sStatus - Status of send to EVC1.
//    pReply - Points to EVC1's response packet, or NULL if no EVC1.
//
void  CDdmCMB::SendEvcMsg3 (void *pvSendCtx, STATUS sStatus,
                            const CmbPacket *pReply)
{

CEvcSendContext * pSendCtx  =  (CEvcSendContext *) pvSendCtx;
const U8          bDest = CMB_EVC1;


   assert (pSendCtx != NULL);

   //  merge results of two EVC accesses
   if (pSendCtx->m_sEvcResult == CTS_SUCCESS)
      {
      pSendCtx->m_sEvcResult = sStatus;

      if (pReply != NULL)
         {
         //  got a real reply, save it
         pSendCtx->m_pktEvcResult = *pReply;
         }
      }

   //  pass results on to our caller
   (this->*pSendCtx->m_pCmdCallback) (pSendCtx->m_pvCookie,
                                      pSendCtx->m_sEvcResult,
                                      &pSendCtx->m_pktEvcResult);

   //  dispose of our cookie, and we're done
   delete pSendCtx;

   return;

}  /* end of CDdmCMB::SendEvcMsg3 */

//
//  CDdmCMB::ValidIopSlot (eSlot, iContigSlot)
//
//  Description:
//    Called to verify that a supplied "TySlot" enum value is
//    actually a valid IOP slot, as we understand it.
//
//    When we recognize a valid slot, we also return its "contiguous
//    index", which is a value we use internally to index various
//    IOP slot-specific data structures.  We don't like to use the
//    raw TySlot enum because its values are rather sparse, and so
//    would consume a lot of extra memory.
//
//  Inputs:
//    eSlot - IOP slot value to validate.
//
//  Outputs:
//    iContigSlot - Returns zero-based "contiguous index" equivalent
//             of our input eSlot value.  Only valid when this routine
//             returns TRUE.
//    CDdmCMB::ValidIopSlot - Returns TRUE when the supplied eSlot value
//             is valid, else returns FALSE.
//

BOOL CDdmCMB::ValidIopSlot (TySlot eSlot, U32& iContigSlot)  const
{

U32            i;
const U32      ceContigToSlot  =  sizeof (m_aeContigToSlot) /
                                    sizeof (*m_aeContigToSlot);
const TySlot * pSlot;
BOOL           fMyRet;


   //  a pessimistic default
   fMyRet = FALSE;

   //  some strange users (hi, Craig!) want to use CMB_SELF,
   //  so let's make it intelligible here:
   if (eSlot == CMB_SELF)
      {
      eSlot = Address::iSlotMe;
      }

   //  scoot through our list of known Slot IDs, looking for a match
   pSlot = m_aeContigToSlot;

   for (i = 0;  i < ceContigToSlot;  i ++)
      {
      if (eSlot == *pSlot)
         {
         //  found the proper slot, return its "contiguous index"
         iContigSlot = i;
         fMyRet = TRUE;
         break;
         }

      //  no match yet, move on to next slot
      pSlot ++;
      }

   //  all done, report what happened
   return (fMyRet);

}  /* end of CDdmCMB::ValidIopSlot */

//
//  CDdmCMB::BuildCurrentIopStatus (iContigSlot, ulPlacement)
//
//  Description:
//    Given a "contiguous index" reference to an IOP slot, we build up
//    an IOP Status table row entry with current values for the slot.
//
//    We also copy the row entry values into our internal table image,
//    m_aIopStatusImage[].
//
//    *  We return a freshly new-ed heap block containing the requested
//       slot data.  It is the caller's responsibility to free it using
//       C++ "delete" when done with it.
//
//    *** We produce different sorts of records, based on whether we
//        believe CMB hardware is available or not.  When the CMB appears
//        to be operational, we create proto-records whose states are
//        mostly "unknown."  In this case, we expect that later on we'll
//        get notifications from the CMB which will trigger updates to
//        the IOP records with real data.
//        When the CMB does not appear to be available, we produce
//        sheerest bogusness, driven off of a dummy IOP slot table which
//        we maintain for just this purpose.
//
//  Inputs:
//    iContigSlot - Contiguous 0-based index of IOP slot to return data for.
//                This index value is as returned by CDdmCMB::ValidIopSlot().
//    ulPlacement - Controls which heap we allocate the row record from.
//                Normally tSMALL (for local heap), this may be set to tPCI
//                or other new "placement" syntax argument(s) to specify
//                use of an alternate (i.e., transport-friendly) heap.
//
//  Outputs:
//    m_aIopStatusImage[iContigSlot] - Loaded with parameters from status
//                record which we return.
//    CDdmCMB::BuildCurrentIopStatus - Returns a pointer to a newly-allocated
//             IOPStatusRecord containing the desired slot's info.
//             Returns NULL if heap allocation fails.
//

IOPStatusRecord *CDdmCMB::BuildCurrentIopStatus (U32 iContigSlot,
                                                 U32 ulPlacement /* = tSMALL */ )
{

IopSlotEntry    * pIopSlot;
IOPStatusRecord * pMyRet;


   assert (iContigSlot < CT_IOPST_MAX_IOP_SLOTS);

   //  allocate the memory, firstly (constructor does standard setup)
   pMyRet = new (ulPlacement) IOPStatusRecord;
   if (pMyRet == NULL)
      {
      //  whoops
      return (pMyRet);
      }

   if (m_CmbHwIntf.IsRealCmbPresent ())
      {
      //  got real CMB hardware, so make a null record which we'll
      //  fill in later as we get slot update notifications from the CMB.
      pMyRet -> Slot = m_aeContigToSlot [iContigSlot];   // uses TySlot value

      pMyRet -> eIOPCurrentState = IOPS_UNKNOWN;

      if ((pMyRet -> Slot == IOP_HBC0) || (pMyRet -> Slot == IOP_HBC1))
         {
         //  board is an HBC, so its initial target state is "up":
//         pMyRet -> eIOPDesiredState = IOPDS_BOOTED;
         }
      else
         {
         //  all other IOPs have an initial target of "powered down":
//         pMyRet -> eIOPDesiredState = IOPDS_POWERED_DOWN;
         }
      }
   else
      {
      //  no CMB hardware to talk to, so produce a fake record using
      //  our dummy slot table

      //  make convenient ptr to our slot data
      pIopSlot = aIopSlot + iContigSlot;

      //  current state is as given in our dummy table (IOPS_BLANK / IOPS_NEW)
      pMyRet -> eIOPCurrentState = pIopSlot -> eState;

      //  hmm..  for now, we claim that the target state of non-present boards
      //  is powered down, all others is booted (crude, and usurps the 
      //  boot manager's prerogative)   BUGBUG!
      if ((pIopSlot -> eState == IOPS_BLANK) ||
          (pIopSlot -> eState == IOPS_EMPTY))
         {
         //  nothing there, so say it's to be powered down
//         pMyRet -> eIOPDesiredState = IOPDS_POWERED_DOWN;
         }
      else
         {
         //  got a board, pick a desired state so that only the HBC
         //  is running, all others are powered off
         if (pIopSlot -> eType == IOPTY_HBC)
            {
            //  got an HBC, flag its desired state as "booted" (which it is..)
//            pMyRet -> eIOPDesiredState = IOPDS_BOOTED;
            }
         else
            {
            //  some other IOP, so flag its initial target state as "powered down"
//            pMyRet -> eIOPDesiredState = IOPDS_POWERED_DOWN;
            }
         }

      pMyRet -> Slot = m_aeContigToSlot [iContigSlot];   // uses TySlot value

      if (pMyRet -> eIOPCurrentState != IOPS_BLANK)
         {
         //  slot is non-empty, so populate other fields in the record
         pMyRet -> IOP_Type = pIopSlot -> eType;
         strcpy (pMyRet -> SerialNumber, (char *) pIopSlot -> pszSerialNo);

         pMyRet -> ulAvrSwVersion  = 1;
         pMyRet -> ulAvrSwRevision = 0;
         strcpy (pMyRet -> strHwPartNo, "xxyyyyyzz");
         pMyRet -> ulHwRevision    = 1;
         pMyRet -> ulHwMfgDate     = 12345;

   //      pMyRet -> RedundantSlot = ...
   //      pMyRet -> Image1Status = OK;        // whatever..
   //      pMyRet -> Image2Status = ...
   //      pMyRet -> Installation = DateTime value..
         strcpy (pMyRet -> Manufacturer, "ConvergeNet Tech., Inc.");
//                                        12345678901234567890123         


         pMyRet -> Temp = 35;    // temp near CPU -- on all IOPs
         pMyRet -> TempHiThreshold = 55;     // degrees C
         pMyRet -> TempNormThreshold = 45;   // degrees C

   //      pMyRet -> WhichImageIsActive = 1;    // primary image active
         }
      }

   //  got record built, now copy it into our table image
//   m_aIopStatusImage[iContigSlot].eIOPDesiredState =
//                                                pMyRet -> eIOPDesiredState;
   m_aIopStatusImage[iContigSlot].TempHiThreshold =
                                                pMyRet -> TempHiThreshold;
   m_aIopStatusImage[iContigSlot].TempNormThreshold =
                                                pMyRet -> TempNormThreshold;
   strcpy (m_aIopStatusImage[iContigSlot].ChassisSerialNumber,
                                                pMyRet -> ChassisSerialNumber);

   //  all done, return resultant IOP Status row entry
   return (pMyRet);

}  /* end of CDdmCMB::BuildCurrentIopStatus */

//
//  CDdmCMB::MakeSlotMessage (pchMsgBuf, pszMsgPrefix, iContigSlot)
//
//  Description:
//    This is a little helper routine.  It builds a normal-format prompt
//    message using the supplied custom prefix text and slot ID.
//
//    The message text is written into the supplied buffer, which had
//    better be "big enough."  This highly lame code is only temporary,
//    so don't worry too much about the lack of error checking.  :-)
//
//  Inputs:
//    pchMsgBuf - Points to buffer to receive our generated message.
//    pszMsgPrefix - Points to custom text to place in start of buffer.
//    iContigSlot - "Contiguous slot" index of slot whose data we're
//                   to display in *pchMsgBuf, after *pszMsgPrefix.
//
//  Outputs:
//    *pchMsgBuf - Loaded with an ever-so-pretty message.
//

void  CDdmCMB::MakeSlotMessage (char *pchMsgBuf, const char *pszMsgPrefix,
                                int iContigSlot)
{

   sprintf (pchMsgBuf, "%s IOP slot %s (%u)", pszMsgPrefix,
            apszSlotName [iContigSlot], m_aeContigToSlot [iContigSlot]);

}  /* end of CDdmCMB::MakeSlotMessage */

