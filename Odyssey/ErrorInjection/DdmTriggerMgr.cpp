/*
 * DdmTriggerMgr.cpp - Error Injection Trigger Manager DDM
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *     2/6/1999 Bob Butler: Created
 *
*/

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#include "EISTriggers.h"

#include "DdmTriggerMgr.h"
#include "BuildSys.h"
#include "Event.h"
extern "C" {
#include "ct_fmt.h"
}

//void  *operator new[](int, unsigned long, int *) { return NULL; }

CLASSNAME(DdmTriggerMgr, SINGLE);	// Class Link Name used by Buildsys.cpp
SERVELOCAL(DdmTriggerMgr,PRIVATE_REQ_LOAD_TRIGGER_MGR);
SERVELOCAL(DdmTriggerMgr,PRIVATE_REQ_ARM_TRIGGER_MGR);
SERVELOCAL(DdmTriggerMgr,PRIVATE_REQ_DISARM_TRIGGER_MGR);
SERVELOCAL(DdmTriggerMgr,PRIVATE_REQ_REPORT_TRIGGER_MGR);

DdmTriggerMgr *DdmTriggerMgr::pTM = NULL;

extern int atoi(char *);
// void *operator new[](unsigned int i) { return ::operator new[]((long)i); }

// Static method to check for a trigger condition
BOOL DdmTriggerMgr::Trigger(U32 _ID, U32 _parameter)
{
	BOOL bAnyTrigger = false;
	if (DdmTriggerMgr::pTM->isArmed)
	{
		for (U16 i=0; i < DdmTriggerMgr::pTM->aET.NextIndex(); ++i)
			bAnyTrigger = DdmTriggerMgr::pTM->aET[i].Trigger(_ID, _parameter) || bAnyTrigger;
	}
	return bAnyTrigger;
}

// Static method to clear a trigger
U16 DdmTriggerMgr::Clear(U32 _ID)
{
	U16 cCleared = 0;
	if (DdmTriggerMgr::pTM->isArmed)
	{
		for (U16 i=0; i < DdmTriggerMgr::pTM->aET.NextIndex(); ++i)
			if (DdmTriggerMgr::pTM->aET[i].GetID() == _ID 
			     && DdmTriggerMgr::pTM->aET[i].GetState() == ErrorTrigger::TRIGGERED) 
		    	{
		  			DdmTriggerMgr::pTM->aET[i].SetCleared();  
			  		++cCleared;
			  	}
		DdmTriggerMgr::pTM->CompletionCheck();
	}
	return cCleared;
}

// Static method to clear a class of triggers
U16 DdmTriggerMgr::ClassClear(U32 _classID)
{
	U16 cCleared = 0;
	if (DdmTriggerMgr::pTM->isArmed)
	{
		for (U16 i=0; i < DdmTriggerMgr::pTM->aET.NextIndex(); ++i)
			if (MASK_EIS_ERROR_CLASS(DdmTriggerMgr::pTM->aET[i].GetID()) == _classID 
			     && DdmTriggerMgr::pTM->aET[i].GetState() == ErrorTrigger::TRIGGERED) 
		  		{
		  			DdmTriggerMgr::pTM->aET[i].SetCleared();  
			  		++cCleared;
			  	}
		DdmTriggerMgr::pTM->CompletionCheck();
	}
	return cCleared;
}

 
// Constructor 
DdmTriggerMgr::DdmTriggerMgr(DID did): Ddm(did)
{
}
	

// Ctor -- Create an instance
Ddm *DdmTriggerMgr::Ctor(DID did)
{

	return DdmTriggerMgr::pTM = new DdmTriggerMgr(did);
}

// Enable Message
STATUS DdmTriggerMgr::Enable(Message *pMsg)
{ 
	Reply(pMsg, OK);
	return OK;
}

// Initialize Message
STATUS DdmTriggerMgr::Initialize(Message *pMsg)
{ 
	DispatchRequest(PRIVATE_REQ_LOAD_TRIGGER_MGR, REQUESTCALLBACK(DdmTriggerMgr, LoadScript));
	DispatchRequest(PRIVATE_REQ_ARM_TRIGGER_MGR, REQUESTCALLBACK(DdmTriggerMgr, Arm));
	DispatchRequest(PRIVATE_REQ_DISARM_TRIGGER_MGR, REQUESTCALLBACK(DdmTriggerMgr, Disarm));
	DispatchRequest(PRIVATE_REQ_REPORT_TRIGGER_MGR, REQUESTCALLBACK(DdmTriggerMgr, Report));
	Reply(pMsg, OK);
	return OK;
	
}

// Load the Error Triggers
STATUS DdmTriggerMgr::LoadScript(Message *pMsg)
{ 
	U32 *_aPacked;
	U32 _size;

	if (pArmMsg)
	{
		Reply(pArmMsg, EIS_FORCED_DISARM);
		pArmMsg = NULL;
	}
	aET.Clear();
	isArmed = false;

	pMsg->GetSgl(0, 0, (void **)&_aPacked, &_size);
	
	for (U16 i = 0; i < _size/sizeof(U32); i += 5)
	{
		ErrorTrigger *_pET = new ErrorTrigger(_aPacked[i], _aPacked[i+1], _aPacked[i+2], _aPacked[i+3], _aPacked[i+4]);
		aET.Append(*_pET);
	}
	
	Reply(pMsg, OK);
	return OK;

}

// Status Report
STATUS DdmTriggerMgr::Report(Message *pMsg)
{ 
	TMReport *pPayload = new TMReport;
	
	pPayload->iSlot = DeviceId::ISlot(GetDid());
	pPayload->cTriggers = aET.NextIndex();
	if (pPayload->cTriggers)
	{
		pPayload->isArmed = isArmed;
		pPayload->iPhase = aET[0].GetPhase();
		for (U16 i=0; i < aET.NextIndex(); ++i)
		{
			switch (aET[i].GetState())
			{
				case ErrorTrigger::ARMED:
					pPayload->cArmed++;
					break;
				case ErrorTrigger::DISARMED:
					pPayload->cDisarmed++;
					break;
				case ErrorTrigger::TRIGGERED:
					pPayload->cTriggered++;
					break;
				case ErrorTrigger::CLEARED:
					pPayload->cCleared++;
					break;
			}
		}	
	}	
	pMsg->AddReplyPayload(pPayload, sizeof (TMReport));
	Reply(pMsg, OK);
	return OK;
		
}

// Arm the Error Triggers
STATUS DdmTriggerMgr::Arm(Message *pMsg)
{ 
	// complain if we are already armed
	if (isArmed)
		Reply(pMsg, EIS_ALREADY_ARMED);
	else
	{
		pArmMsg = pMsg; // send a reply once all the errors have triggered and been cleared
		for (U16 i=0; i < aET.NextIndex(); ++i)
			aET[i].SetArmed();
		isArmed = true;
	}
	return OK;
		
}

// Disarm the Error Triggers
STATUS DdmTriggerMgr::Disarm(Message *pMsg)
{ 
	isArmed = false;
	for (U16 i=0; i < aET.NextIndex(); ++i)
		aET[i].SetDisarmed();
	if (pArmMsg)
	{
		Reply(pArmMsg, EIS_FORCED_DISARM);
		pArmMsg = NULL;
	}
	Reply(pMsg, OK);
	return OK;
}

// Disarm the Error Triggers
void DdmTriggerMgr::CompletionCheck()
{ 
	for (U16 i=0; i < aET.NextIndex(); ++i)
		if (aET[i].GetState() != ErrorTrigger::CLEARED)
			return;
	// send back the orginal ARM message with an OK status so the
	// ScriptDirector DDM knows we're done.			
	if (pArmMsg)  
	{
		Reply(pArmMsg, OK);
		pArmMsg = NULL;
	}
}
