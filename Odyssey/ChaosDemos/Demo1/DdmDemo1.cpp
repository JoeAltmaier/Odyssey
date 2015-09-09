/* DdmDemo1.cpp -- Demo a basic Ddm
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

#include "RqOsTimer.h"	// Interface to DdmTimer
#include "BuildSys.h"

#include "DdmDemo1.h"

// BuildSys linkage
	
CLASSNAME(DdmDemo1,MULTIPLE);


// .Enable -- Start-it-up -----------------------------------------------------------------DdmDemo1-
//
ERC DdmDemo1::Enable(Message *_pEnableMsg)
{
	Tracef("Enabling Demo did=%x (DdmDemo1::Enable)\n",GetDid());

	// Start a timer
	pStartMsg = new RqOsTimerStart(1000000,1000000);	// Time in uSec.
	Send(pStartMsg, REPLYCALLBACK(DdmDemo1,ProcessReplyTimer));	
	
	Reply(_pEnableMsg);
	
	return OK;
}

// .Quiesce -- End-it-all -----------------------------------------------------------------DdmDemo1-
//
ERC DdmDemo1::Quiesce(Message *_pQuiesceMsg)
{
	Tracef("Quiescing Demo did=%x (DdmDemo1::Quiesce)\n",GetDid());

	// Stop the timer
	Message *pMsg = new RqOsTimerStop(pStartMsg);
	Send(pMsg, _pQuiesceMsg, REPLYCALLBACK(DdmDemo1,ProcessReplyStopTimer));
	
	return OK;
}

// .ProcessReplyStopTimer -----------------------------------------------------------------DdmDemo1-
//
ERC DdmDemo1::ProcessReplyStopTimer(Message *_pReply) {

	Tracef("Timer Stopped (DdmDemo1::ProcessReplyStopTimer)\n");
	
	Reply((Message*) _pReply->GetContext(), OK);	// Quiesce
	
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}


// .ProcessReplyTimer ---------------------------------------------------------------------DdmDemo1-
//
ERC DdmDemo1::ProcessReplyTimer(Message *_pReply) {

	Tracef(".tick...");
	
	delete _pReply;		// Must always delete reply if we don't want it!
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo1/DdmDemo1.cpp $
// 
// 2     2/15/00 10:48p Tnelson
// Added Quiesce Support
// 
// 1     2/12/00 6:15p Tnelson
// Demo minimum build to run a Ddm on a single HBC
// 
