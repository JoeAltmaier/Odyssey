/* DdmDemo2Ping.cpp -- Demo a basic Ddm
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

#include "DdmDemo2Ping.h"
#include "RqDdmDemo2Pong.h"	// Public interface to DdmDemo2Pong

// BuildSys linkage
	
CLASSNAME(DdmDemo2Ping,MULTIPLE);


// .Enable -- Start-it-up -------------------------------------------------------------DdmDemo2Ping-
//
ERC DdmDemo2Ping::Enable(Message *_pEnableMsg)
{
	Tracef("Enabling DID=%x (DdmDemo2Ping::Enable)\n",GetDid());

	// Start a timer
	Message *pMsg = new RqOsTimerStart(1000000,1000000);	// Time in uSec.
	Send(pMsg, REPLYCALLBACK(DdmDemo2Ping,ProcessReplyTimer));
	
	Reply(_pEnableMsg);
	
	return OK;
}

// .ProcessReplyTimer -----------------------------------------------------------------DdmDemo2Ping-
//
ERC DdmDemo2Ping::ProcessReplyTimer(Message *_pReply) {

	Tracef("tick.");
	
	// Send our ping count to DdmDemo2Pong for display & update
	Message *pMsg = new RqDdmDemo2Pong(count);
	Send(pMsg, REPLYCALLBACK(DdmDemo2Ping, ProcessReplyPong));
	
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}

// .ProcessReplyPong -----------------------------------------------------------------DdmDemo2Ping-
//
ERC DdmDemo2Ping::ProcessReplyPong(Message *_pReply) {

	RqDdmDemo2Pong * pReply = (RqDdmDemo2Pong*) _pReply;
	
	Tracef(".");
	
	count = pReply->GetCount();	// Update our count returned from DdmDemo2Pong.
	
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo2/DdmDemo2Ping.cpp $
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
