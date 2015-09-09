/* DdmTimer.cpp -- Implementation of the Timer DDM.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
**/

// Revision History: 
//  8/17/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "DdmTimer.h"
#include "BuildSys.h"
#include "CtEvent.h"

// BuildSys Linkage

CLASSNAME(DdmTimer,SINGLE);

SERVELOCAL(DdmTimer,RqOsTimerStart::RequestCode);
SERVELOCAL(DdmTimer,RqOsTimerReset::RequestCode);
SERVELOCAL(DdmTimer,RqOsTimerStop::RequestCode);

// static
DdmTimer::TimerContext *DdmTimer::TimerContext::pHead=NULL;

// DdmTimer -- Constructor ----------------------------------------------------------------DdmTimer-
//
DdmTimer::DdmTimer(DID did) : Ddm(did) { 

	DispatchRequest(RqOsTimerStart::RequestCode, REQUESTCALLBACK(DdmTimer, ProcessTimerStart));
	DispatchRequest(RqOsTimerReset::RequestCode, REQUESTCALLBACK(DdmTimer, ProcessTimerReset));
	DispatchRequest(RqOsTimerStop::RequestCode,  REQUESTCALLBACK(DdmTimer, ProcessTimerStop));
}

// .Terminate -- Process termination request ----------------------------------------------DdmTimer-
//
ERC DdmTimer::Terminate(RqOsDdmTerminate *_pRequest) {

	TimerContext *pNext;
	
	for (TimerContext *pContext = TimerContext::pHead; pContext; pContext = pNext) {
		pNext = pContext->pNext;
		// Search for dead timer messages
		if (pContext->pMsg->IsDead()) {
			Reply(pContext->pMsg,OK);
			delete pContext;
		}
	}
	Reply(_pRequest,OK);
	
	return OK;
}

// .ProcessTimerStart -- Process Timer Start Request --------------------------------------DdmTimer-
//
STATUS DdmTimer::ProcessTimerStart(Message* _pRequest) {
	
	RqOsTimerStart *pMsg = (RqOsTimerStart*) _pRequest;
	
	TimerContext *pContext = new TimerContext(HandleTimerExpire, this, pMsg);

	Critical section;	
	// Mark message using status code: one-shot: success  retriggering: timeout
	pMsg->DetailedStatusCode = (pMsg->timeN == 0)? OK : CTS_CHAOS_TIMEOUT;
	
	if (pContext->timer.Enable(pMsg->time0,pMsg->timeN) != OK) {
		delete pContext;
		return Reply(pMsg, CTS_CHAOS_INSUFFICIENT_RESOURCE_SOFT);
	}

	// Timer started.  Dont reply to 'TimerStart' message until timer expires.
	return OK;
}

// .ProcessTimerStop -- Process Timer Stop Request ----------------------------------------DdmTimer-
//
STATUS DdmTimer::ProcessTimerStop(Message* _pRequest) {

	RqOsTimerStop *pMsg = (RqOsTimerStop*) _pRequest;
	
	// Find Context from pMsg
	TimerContext *pContext = TimerContext::pHead;
	
	while (pContext && pContext->pMsg->refnum != pMsg->refNum)
		pContext = pContext->pNext;

	if (pContext) {
		pContext->timer.Disable();			// Stop timer pointed to by pMessage.
		pContext->pMsg->DetailedStatusCode = OK;	// Reply last time to pMessage
		pContext->pMsg->pCookie = pMsg->pCookie;    // set cookie for last reply

		Critical section;
		HandleTimerExpire(pContext);		// One last tick!
		Reply(pMsg, OK);
	}
	else
		Reply(pMsg, CTS_CHAOS_INVALID_PARAMETER);

	return OK;
}

// .ProcessTimerReset -- Process Timer Reset Request --------------------------------------DdmTimer-
//
STATUS DdmTimer::ProcessTimerReset(Message* _pRequest) {

	RqOsTimerReset *pMsg = (RqOsTimerReset*) _pRequest;
	
	// Find Context from pTimerMsg in 
	TimerContext *pContext = TimerContext::pHead;
	
	while (pContext && pContext->pMsg->refnum != pMsg->refNum) {
		pContext = pContext->pNext;
	}
	if (pContext) {
		pContext->timer.Disable();	// Stop timer
		pContext->timer.Enable(pMsg->time0,pMsg->timeN);
		pContext->pMsg->pCookie = pMsg->pCookie;
		Reply(pMsg, OK);
	}
	else
		Reply(pMsg, CTS_CHAOS_INVALID_PARAMETER);

	return OK;
}

// .DeleteTimerContext -- Timer Action Handler --------------------------------------------DdmTimer-
//
// This is an Action handler for a class Timer object.
//
ERC DdmTimer::DeleteTimerContext(void *pCt) {
	TimerContext *pContext = (TimerContext*)pCt;
	delete pContext;
	return OK;
}

// .HandleTimerExpire -- Timer Callback Handler -------------------------------------------DdmTimer-
//
// This is the callback handler for a class Timer object.
//
void DdmTimer::HandleTimerExpire(void *pCt) {
//Tracef("DdmTimer::HandleTimerExpire\n");
	
	TimerContext *pContext = (TimerContext*)pCt;
	if (!pContext->fDone) {

		STATUS status = pContext->pMsg->DetailedStatusCode;
		BOOL fLast = (status == OK);

		pContext->pDdm->Reply(pContext->pMsg, status, fLast);

		if (fLast) {
			pContext->fDone=true; // Do this only once per TimerContext
			pContext->pDdm->Action(pContext->pDdm, ACTIONCALLBACK(DdmTimer, DeleteTimerContext), pContext);
		}
	}
}

// .TimerContext -- -----------------------------------------------------------------------DdmTimer-
//
// This is the callback handler for a class Timer object.
//
DdmTimer::TimerContext::~TimerContext() {
	// Find Context from pMsg
	TimerContext *pContext = pHead;
	TimerContext **ppUnlink = &pHead;
	
	while (pContext && pContext != this) {
		ppUnlink = &pContext->pNext;
		pContext = pContext->pNext;
	}
	if (pContext)
		*ppUnlink = pContext->pNext;		// Unlink this Context from chain
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmTimer.cpp $
// 
// 16    2/08/00 8:57p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 17    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 15    12/09/99 2:07a Iowa
// 
//  8/28/99 Joe Altmaier: Critical sections arount Context linking
//  6/27/99 Tom Nelson:	  Use RequestCode enum from Timer Message classes
//						  Use TimerPIT only when on Odyssey platform
//  5/11/99 Eric Wedel:   Removed old cut&paste relic ppUnlink in
//                        DdmTimer::ProcessTimerReset().
//  5/07/99 Eric Wedel:   Changed for classname in functor macros (GH).
//  3/27/99 Tom Nelson:   Support derived timer request classes
//  3/18/99 Tom Nelson:   Changed to use Timer.h and SERVE macros
//  3/04/99 Tom Nelson:	  Change NU_Create_Timer to Kernel::Create_Timer
// 12/10/98 Joe Altmaier: Use new Ddm api
//  9/4/98  Joe Altmaier: Implement OOS_TIMER_START, OOS_TIMER_STOP
//  8/17/98 Joe Altmaier: Create file

