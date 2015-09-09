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
// File: HotSwapTest.cpp
// 
// Description:
//    This file contains a test DDM used to exercise the Hot Swap Master DDM.
// 
// $Log: /Gemini/Odyssey/DdmHotSwap/TestDdm/HotSwapTest.cpp $
// 
// 1     10/11/99 7:53p Ewedel
// First cut.
//
/*************************************************************************/


#include  "HotSwapTest.h"

#include  "DdmHotSwapCommands.h"

#include  "BuildSys.h"

#include  "CtEvent.h"

#include  "Odyssey_Trace.h"

#include  "ansi/stdio.h"

#include  "OsHeap.h"          // heap debug support (from ..\msl)

#include  <assert.h>

   
//  Class Link Name used by Buildsys.cpp
CLASSNAME(CHotSwapTest, SINGLE);






// Ctor -- Create ourselves --------------------------------------CHotSwapTest-
//
Ddm *CHotSwapTest::Ctor (DID did)
{
   
   return (new CHotSwapTest(did));

}  /* end of CHotSwapTest::Ctor */


// CHotSwapTest -- Constructor -----------------------------------CHotSwapTest-
//
CHotSwapTest::CHotSwapTest (DID did) : Ddm(did),
                           m_HotSwapQueueIntf (HSW_CONTROL_QUEUE,
                                               HSW_CONTROL_COMMAND_SIZE,
                                               HSW_CONTROL_STATUS_SIZE,
                                               this)
{

   Tracef("CHotSwapTest::CHotSwapTest()\n");

   return;

}  /* end of CHotSwapTest::CHotSwapTest */


// Enable -- Start-it-up -----------------------------------------CHotSwapTest-
//
STATUS CHotSwapTest::Enable(Message *pMsg)
{


   Tracef("CHotSwapTest::Enable()\n");

   //  this is a good place to actually do something, if we have
   //  anything to do (e.g., send a command to the Hot Swap Master)
   TestHotSwap ();

   //  report successful enable
   Reply (pMsg, CTS_SUCCESS);
   return (CTS_SUCCESS);

}  /* end of CHotSwapTest::Enable */

//
//  CHotSwapTest::TestHotSwap ()
//
//  Description:
//    Called by our Enable() routine.  We do whatever seems like a test
//    of the Hot Swap Master.  Right now, that's nada.
//
//  Inputs:
//    none
//
//  Outputs:
//    CHotSwapTest::TestHotSwap - Returns result of most recent OS operation
//             (Send, etc.).
//

void  CHotSwapTest::TestHotSwap (void)
{

STATUS   sRet;


   //  initialize our control interface, in a synchronous way
   sRet = m_HotSwapQueueIntf.csndrInitialize(INITIALIZECALLBACK (CHotSwapTest,
                                                                 TestHotSwap2));

   assert (sRet == CTS_SUCCESS);
   
   return;

}  /* end of CHotSwapTest::TestHotSwap */


//  We are called back from m_CmdServer.csrvInitialize(), in TestHotSwap().
//
//  Inputs:
//    ulStatus - Result of control interface initialize operation.
//

void  CHotSwapTest::TestHotSwap2 (STATUS sStatus)
{


   //  as a stub, all we can do is flag if init went bad, no Enable message
   //  available to reply to here.
   assert (sStatus == CTS_SUCCESS);

   //  all done, wait for CHotSwapTest::WakeUpCmbDdm1a to be called.
   return;

}  /* end of CHotSwapTest::TestHotSwap2 */


