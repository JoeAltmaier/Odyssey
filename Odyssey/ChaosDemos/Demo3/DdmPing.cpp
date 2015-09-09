/* DdmPing.cpp -- Demo a basic Ddm
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

#include "RqOsTimer.h"		// Public interface to DdmTimer
#include "BuildSys.h"

#include "DdmPing.h"
#include "RqDdmPongMaster.h"	// Public interface to DdmDemo2Pong

// BuildSys linkage
	
CLASSNAME(DdmPing,MULTIPLE);


// .Enable -- Start-it-up -------------------------------------------------------------DdmPing-
//
ERC DdmPing::Enable(Message *_pEnableMsg)
{
	Tracef("Enabling DID=%x (DdmPing::Enable)\n",GetDid());

	// Start a timer
	Message *pMsg = new RqOsTimerStart(1000000,1000000);	// Time in uSec.
	Send(pMsg, REPLYCALLBACK(DdmPing,ProcessReplyTimer));
	
	Reply(_pEnableMsg);
	
	return OK;
}

// .ProcessReplyTimer -----------------------------------------------------------------DdmPing-
//
ERC DdmPing::ProcessReplyTimer(Message *_pReply) {

	Tracef("%5d",count);
	
	// Send our ping count to DdmDemo2Pong for display & update
	Message *pMsg = new RqDdmPongMasterTick(Address::GetSlot(),count);
	Send(pMsg, REPLYCALLBACK(DdmPing, ProcessReplyPong));
	
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}

// .ProcessReplyPong -----------------------------------------------------------------DdmPing-
//
ERC DdmPing::ProcessReplyPong(Message *_pReply) {

	RqDdmPongMasterTick * pReply = (RqDdmPongMasterTick*) _pReply;
	
	if (pReply->Status() != OK) {
		Tracef("Erc = %u from RqDdmPongMasterTick (DdmPing::ProcessReplyPong)\n",pReply->Status());
	}
	else {
		Tracef(".");
	
		count = pReply->GetCount();	// Update our count returned from DdmDemo2Pong.
	}
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo3/DdmPing.cpp $
// 
// 1     2/13/00 7:05p Tnelson
//  Demo shows communication between a SYSTEMENTRY Ddm on an IOP and a
// SYSTEMMASTER Ddm on an HBC.
// 
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
