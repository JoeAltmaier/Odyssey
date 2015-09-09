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
// File: CommandIntf.cpp
//
// Description:
//    Hot Swap Master DDM's CmdQueue-based interface handlers.
//
//    This file contains the member routines for processing commands sent
//    to our DDM via the CmdSender / CmdServer inter-DDM comm machinery.
//    See CDdmCMB::CmdReady() below for more info.
//
// $Log: /Gemini/Odyssey/DdmHotSwap/CommandIntf.cpp $
// 
// 3     12/22/99 6:35p Ewedel
// Pass DDM instance ptr to smart cookie, which needs it.
// 
// 2     12/16/99 1:08a Ewedel
// Removed IOP swap stuff, which is now part of boot master.  Made drive
// hot swap real.
// 
// 1     10/11/99 7:51p Ewedel
// First cut, really just a shell now.
//
/*************************************************************************/

#include  "DdmHotSwap.h"

#include  "DdmHotSwapCommands.h"    // our CmdQueue interface defs

#include  "HotSwapCmdHandler.h"     // worker "cookie" class defs

#include  "CtEvent.h"               // standard status codes

#include  "Odyssey_Trace.h"

#include  <assert.h>



//
//  CDdmHotSwap::CmdReady (hRequest, pvRequest)
//
//  Description:
//    Called when CmdSender::csndrExecute() is used to send us a request.
//    We dispatch the request for appropriate processing.  After processing
//    completes (may span multiple DDM messages) we issue a suitable reply.
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pvRequest - Veiled form of request which we're to process.  As submitted
//                to csndrExecute() [the pCmdData formal], this must be a
//                pointer to an instance of our CHotSwapIntf parameters type.
//
//  Outputs:
//    CDdmHotSwap::CmdReady - Returns CTS_SUCCESS (will become void later on).
//

void  CDdmHotSwap::CmdReady (HANDLE hRequest, void *pvRequest)
{

CHotSwapIntf       * pRequest;
CHotSwapCmdHandler * pHandler;


   TRACE_PROC(CDdmHotSwap::CmdReady);

   pRequest = (CHotSwapIntf *) pvRequest;
   assert (pRequest != NULL);

   if (pRequest == NULL)
      {
      //  we just can't do a thing with it

      CHotSwapIntf   strResponse;

      m_CmdServer.csrvReportCmdStatus (hRequest, 
                                       CTS_HSW_INVALID_PARAMETER,
                                       NULL,    // (our result data is in req pkt)
                                       &strResponse); // (short lived var is ok)

      return;
      }

   //  got a command to process, get to it:

   //  make up a "smart cookie" to handle this request.  Construction
   //  also starts this cookie doing its thing.
   pHandler = new CHotSwapCmdHandler (this, *pRequest, hRequest, m_CmdServer);

   //  (new is reputed to never fail in this box, or it won't come back)

   //  all done, our new CHotSwapCmdHandler instance will take care
   //  of replying to our request.

   return;

}  /* end of CDdmHotSwap::CmdReady */


