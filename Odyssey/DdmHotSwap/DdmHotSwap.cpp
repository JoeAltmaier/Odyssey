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
// File: DdmHotSwap.cpp
//
// Description:
//    Hot Swap Master DDM.
//
//
// $Log: /Gemini/Odyssey/DdmHotSwap/DdmHotSwap.cpp $
// 
// 1     10/11/99 7:51p Ewedel
// First cut, really just a shell now.
//
/*************************************************************************/

#include  "DdmHotSwap.h"

#include  "DdmHotSwapCommands.h"

#include  "BuildSys.h"        // for our CLASS() def

#include  "CtEvent.h"

#include  <ansi\stdio.h>      // [ansi\ forces use of our local stdio.h]

#include  "OsHeap.h"          // heap debug support (from ..\msl)

#include  <assert.h>          // debug stuff

//*//  more debug stuff:  for tPCI to really be tSMALL,
//*//  so "placed" new()s will be included in the heap tester
//*#undef tPCI
//*#define tPCI  tSMALL

   
CLASSNAME (CDdmHotSwap, SINGLE); // Class Link Name used by Buildsys.cpp --
                                 // must match CLASSENTRY() "name" formal
                                 // (4th arg) in buildsys.cpp




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

CDdmHotSwap::CDdmHotSwap (DID did) : Ddm (did),
                             m_CmdServer (HSW_CONTROL_QUEUE,
                                          HSW_CONTROL_COMMAND_SIZE,
                                          HSW_CONTROL_STATUS_SIZE,
                                          this,
                                          CMDCALLBACK (CDdmHotSwap, CmdReady))
{


   TRACE_PROC(CDdmHotSwap::CDdmHotSwap);

   Tracef("CDdmHotSwap::CDdmHotSwap()\n");

   return;

}  /* end of CDdmHotSwap::CDdmHotSwap (DID did) */

//
//  CDdmHotSwap::Ctor (did)
//
//  Description:
//    Our static, standard system-defined helper function.
//    This routine is called by CHAOS when it wants to create
//    an instance of CDdmHotSwap.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    CDdmHotSwap::Ctor - Returns a pointer to the new instance, or NULL.
//

/* static */
Ddm *CDdmHotSwap::Ctor (DID did)
{

   return (new CDdmHotSwap (did));

}  /* end of CDdmHotSwap::Ctor */

//
//  CDdmHotSwap::Initialize (pInitMsg)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    CDdmHotSwap::Enable().
//
//    We simply turn on our command queue interface.
//
//    (We also need to post a listen on the EVC Status table, but that
//     must be deferred until at least Enable(), and maybe beyond since
//     its origin is somewhat ambiguous at present.)
//    nb:  We probably ought to just create / populate a dummy EVC Status
//    table if one doesn't already exist.
//
//  Inputs:
//    pInitMsg - Points to message which triggered our DDM's fault-in.
//          This is always an "initialize" message.
//
//  Outputs:
//    CDdmHotSwap::Initialize - Returns OK if all is cool, else an error.
//

/* virtual */
STATUS CDdmHotSwap::Initialize (Message *pInitMsg)
{

STATUS   sRet;


   Tracef ("CDdmHotSwap::Initialize()\n");
   TRACE_PROC(CDdmHotSwap::Initialize);

   //  verify that code's checksum is ok
   OsHeap::CheckOs();

   //  similarly, do a baseline check on the heap as well
   OsHeap::CheckHeap();



   //  initialize our control interface..
   sRet = m_CmdServer.csrvInitialize (INITIALIZECALLBACK (CDdmHotSwap,
                                                          Initialize2));
   if (sRet != CTS_SUCCESS)
      {
      assert (sRet == CTS_SUCCESS);

      //  we're toast, stop right here
      Reply (pInitMsg, sRet);

      //  (must always return CTS_SUCCESS, even when it wasn't)
      return (CTS_SUCCESS);
      }

   //  since there is no room for a cookie in csrvInitialize(),
   //  we stop right here.
   Reply (pInitMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmHotSwap::Initialize () */


//  We are called back from m_CmdServer.csrvInitialize(), in Initialize2().
//  Note that since there is no cookie parameter to csrvInitialize, we
//  treat this callback as a "stub" rather than part of our main flow
//  of initialization.
//
//  Inputs:
//    ulStatus - Result of initialize operation.
//

void  CDdmHotSwap::Initialize2 (STATUS sStatus)
{


   Tracef ("CDdmHotSwap::Initialize2(), sStatus = %X\n", sStatus);
   TRACE_PROC(CDdmHotSwap::Initialize2);

   //  as a stub, all we can do is flag if init went bad, no init message
   //  available to reply to here.
   assert (sStatus == CTS_SUCCESS);

   return;

}  /* end of CDdmHotSwap::Initialize2 */

//
//  CDdmHotSwap::Quiesce (pMsg)
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
//       So effectively, we are "quiesced" during our Initialize() and
//       Enable() processing.
//
//  Inputs:
//    pMsg - Points to quiesce message which triggered this call.
//             Must be replied to in order to signal quiesce completion.
//
//  Outputs:
//    CDdmHotSwap::Quiesce - Returns OK, or a highly descriptive error code.
//

STATUS CDdmHotSwap::Quiesce (Message *pMsg)
{ 


   TRACE_PROC(CDdmHotSwap::Quiesce);
   
   //  * * *  do local Quiesce stuff here.  * * *

   //  shut down whatever listens we have outstanding (e.g., KeyPosition
   //  in the EVC Status table, if we do handle shutdown)

   //  signal CHAOS that our DDM instance is finished quiescing.
   Reply (pMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmHotSwap::Quiesce */

//
//  CDdmHotSwap::Enable (pMsg)
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
//    CDdmHotSwap::Ctor - Returns a pointer to the new instance, or NULL.
//

STATUS CDdmHotSwap::Enable(Message *pMsg)
{ 


   Tracef ("CDdmHotSwap::Enable()\n");
   TRACE_PROC(CDdmHotSwap::Enable);

   //  post a listen on the EVC Status table, to watch the KeyPosition
   //  field for changes to the "off" key position.
   //  (iff we wind up owning "shutdown" logic)
//   ...

   //  let base class know that we're done with enable phase
   //  (allows new requests to be processed by our DDM instance --
   //   until this call, CHAOS was holding them off from us to let
   //   us finish initialization / reply processing)
   Reply (pMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of CDdmHotSwap::Enable */

