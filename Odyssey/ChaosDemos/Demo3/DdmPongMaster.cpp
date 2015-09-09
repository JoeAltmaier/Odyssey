/* DdmPongMaster.cpp -- Demo a basic Ddm
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

#include "DdmPongMaster.h"
#include "RqDdmPongMaster.h"

// BuildSys linkage
	
CLASSNAME(DdmPongMaster,SINGLE);

// Catch all DdmPongMaster requests on this processor
SERVEVIRTUAL(DdmPongMaster,RqDdmPongMasterTick::RequestCode);


// .DdmPongMaster -- Constructor -------------------------------------------------------DdmPongMaster-
//
DdmPongMaster::DdmPongMaster(DID did) : Ddm(did) {

	// Send all caught DdmPongMaster requests to ProcessRqDemo3 method.
	DispatchRequest(RqDdmPongMasterTick::RequestCode, REQUESTCALLBACK(DdmTimer, ProcessRqPongTick));
}


// .Enable -- Start-it-up -------------------------------------------------------------DdmPongMaster-
//
ERC DdmPongMaster::Enable(Message *_pEnableMsg)
{
	Tracef("Enabling DID=%x (DdmPongMaster::Enable)\n",GetDid());

	Reply(_pEnableMsg);
	
	return OK;
}

// .ProcessRqPongTick --------------------------------------------------------------------DdmPongMaster-
//
ERC DdmPongMaster::ProcessRqPongTick(Message *_pRequest) {

	RqDdmPongMasterTick *pRequest = (RqDdmPongMasterTick *) _pRequest;
	
	U32 count = pRequest->GetCount();
	
	Tracef("%u(%x)", count,pRequest->GetSlot() );
	
	pRequest->SetCount(count+1);	// Return updated count
	
	Reply(pRequest,OK);
	
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo3/DdmPongMaster.cpp $
// 
// 1     2/13/00 7:06p Tnelson
//  Demo shows communication between a SYSTEMENTRY Ddm on an IOP and a
// SYSTEMMASTER Ddm on an HBC.
// 
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
// 1     2/12/00 6:15p Tnelson
// Demo minimum build to run a Ddm on a single HBC
// 
