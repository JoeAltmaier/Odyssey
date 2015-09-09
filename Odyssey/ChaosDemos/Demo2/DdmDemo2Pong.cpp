/* DdmDemo2Pong.cpp -- Demo a basic Ddm
 *
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Minimum build for a single Ddm on a single processor
 *
**/

// Revision History:
//  2/12/00 Tom Nelson: Created

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Odyssey_Trace.h"

#include "BuildSys.h"
#include "RqOsTimer.h"	// Interface to DdmTimer

#include "DdmDemo2Pong.h"
#include "RqDdmDemo2Pong.h"

// BuildSys linkage
	
CLASSNAME(DdmDemo2Pong,SINGLE);

// Catch all DdmDemo2Pong requests on this processor
SERVELOCAL(DdmDemo2Pong,RqDdmDemo2Pong::RequestCode);


// .DdmDemo2Pong -- Constructor -------------------------------------------------------DdmDemo2Pong-
//
DdmDemo2Pong::DdmDemo2Pong(DID did) : Ddm(did) {

	// Send all caught DdmDemo2Pong requests to ProcessRqDemo2 method.
	DispatchRequest(RqDdmDemo2Pong::RequestCode, REQUESTCALLBACK(DdmTimer, ProcessRqDemo2Pong));
}


// .Enable -- Start-it-up -------------------------------------------------------------DdmDemo2Pong-
//
ERC DdmDemo2Pong::Enable(Message *_pEnableMsg)
{
	Tracef("Enabling DID=%x (DdmDemo2Pong::Enable)\n",GetDid());

	Reply(_pEnableMsg);
	
	return OK;
}

// .ProcessRqDemo2 --------------------------------------------------------------------DdmDemo2Pong-
//
ERC DdmDemo2Pong::ProcessRqDemo2Pong(Message *_pRequest) {

	RqDdmDemo2Pong *pRequest = (RqDdmDemo2Pong *) _pRequest;
	
	U32 count = pRequest->GetCount();
	
	Tracef("%u", count);
	
	pRequest->SetCount(count+1);	// Return updated count
	
	Reply(pRequest,OK);
	
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo2/DdmDemo2Pong.cpp $
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
// 1     2/12/00 6:15p Tnelson
// Demo minimum build to run a Ddm on a single HBC
// 
