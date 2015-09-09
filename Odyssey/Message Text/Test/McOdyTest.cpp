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
// File: McOdyTest.cpp
// 
// Description:
//    This file contains a test DDM used to exercise the message compiler
//    emitted C++ language database code, and the rendering code used to
//    access it.
// 
// $Log: /Gemini/Odyssey/Message Text/Test/McOdyTest.cpp $
// 
// 1     7/12/99 6:10p Ewedel
// Initial revision.
//
/*************************************************************************/


#include  "McOdyTest.h"

#include  "CtMessages.h"      // our message translation support

#include  "BuildSys.h"

#include  "Odyssey_Trace.h"

#include  "stdio.h"

#include  "CtEvent.h"

#include  "OsHeap.h"          // heap debug support (from ..\msl)

#include  <assert.h>

   
//  Class Link Name used by Buildsys.cpp
CLASSNAME(CMcOdyTest, SINGLE);



// Ctor -- Create ourselves --------------------------------------------CMcOdyTest-
//
Ddm *CMcOdyTest::Ctor (DID did)
{
   
   return (new CMcOdyTest(did));

}  /* end of CMcOdyTest::Ctor */


// CMcOdyTest -- Constructor -------------------------------------------CMcOdyTest-
//
CMcOdyTest::CMcOdyTest (DID did) : Ddm(did)
{

   Tracef("CMcOdyTest::CMcOdyTest()\n");
//   SetConfigAddress(&config,sizeof(config)); // tell Ddm:: where my config area is

}  /* end of CMcOdyTest::CMcOdyTest */

//
//  CMcOdyTest::Enable (pMsg)
//
//  Description:
//    Our DDM "enable" callback - this member is called every time that
//    our DDM is enabled, both the first time right after Initialize()
//    has been called, and when we're enabled after being quiesced.
//
//    As it happens, this routine runs our test code right now.
//
//  Inputs:
//    pMsg - Points to the enable message which triggered our invocation.
//
//  Outputs:
//    CMcOdyTest::Enable - Always returns CTS_SUCCESS.
//

STATUS CMcOdyTest::Enable(Message *pMsg)
{


   Tracef("CMcOdyTest::Enable()\n");

   //  run our test routines
   DoTests ();

   //  report successful enable
   Reply (pMsg, CTS_SUCCESS);
   return (CTS_SUCCESS);

}  /* end of CMcOdyTest::Enable */

//
//  CMcOdyTest::DoTests ()
//
//  Description:
//    Runs the test operations which are our entire raison d'etre.
//
//  Inputs:
//    none
//
//  Outputs:
//    none
//

void CMcOdyTest::DoTests ()
{

CCtMsgView  Viewer;        // no ctor arg, so default is English
const char *pszMessage;
STATUS      ulRet;


   //  report what our viewer's current language setting is
   printf("CMcOdyTest::DoTests: viewer's language ID = 0x%04X\n",
          Viewer.GetViewLanguage());

   //  do a few message lookups, and we're done.
   ulRet = Viewer.GetMessageText(pszMessage, CTS_SUCCESS);
   printf("CMcOdyTest::DoTests: status = %X, event code = %08X, "
          "message = \n\"%s\"", ulRet, CTS_SUCCESS, pszMessage);

   ulRet = Viewer.GetMessageText(pszMessage, CTS_CMB_INVALID_PARAMETER);
   printf("CMcOdyTest::DoTests: status = %X, event code = %08X, "
          "message = \n\"%s\"", ulRet, CTS_CMB_INVALID_PARAMETER,
          pszMessage);

   ulRet = Viewer.GetMessageText(pszMessage, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY);
   printf("CMcOdyTest::DoTests: status = %X, event code = %08X, "
          "message = \n\"%s\"", ulRet, CTS_CMB_REQUESTED_IOP_SLOT_EMPTY,
          pszMessage);

   return;

}  /* end of CMcOdyTest::DoTests */



