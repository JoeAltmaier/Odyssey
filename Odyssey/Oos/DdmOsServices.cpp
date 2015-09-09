/* DdmOsServices.cpp -- Ddm Os Services Base Class
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99 
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
//   1/20/99 Tom Nelson: Split out from class Ddm
// ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#define _TRACEF
#define	TRACE_INDEX		TRACE_DDM
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#include "DdmOsServices.h"
#include "CtEvent.h"

// Private Includes
#include "DdmManager.h"
#include "ClassTable.h"
#include "Messenger.h"
#ifndef WIN32
	#include "Transport.h"
#endif

extern "C" char* NameRq(REQUESTCODE);

// .DdmOsSerives -- Constructors -----------------------------------------------------DdmOsServices-
//
DdmOsServices::DdmOsServices(DdmServices *_pParentDdmSvc) : pServiceTag(NULL) {
	pParentDdmSvs = _pParentDdmSvc;
	
	if (pParentDdmSvs != NULL)
		pServiceTag = DidMan::services.Add(this);
}

// DEPRECATED - Always pass instance of parent!
//
DdmOsServices::DdmOsServices()	{	
	pParentDdmSvs = NULL;
}


// .~DdmOsSerives -- Destructor ------------------------------------------------------DdmOsServices-
//
DdmOsServices::~DdmOsServices()	{	
	TRACE_PROC(DdmOsServices::~DdmOsServices);
	
	DidMan::callbacks.DeleteTarget(pServiceTag);
	DidMan::services.Delete(pServiceTag);
}


// .GetDid -- Get Device ID of Parent Ddm --------------------------------------------DdmOsServices-
//
// Gets Did of Parent Ddm via Parent Ddm's base DdmServices Class.  If called
// from the actual Ddm this virtual function will be overriden and will exist
// in the derived Ddm class.
//
DID DdmOsServices::GetDid() { // virtual
	return (pParentDdmSvs != NULL) ? pParentDdmSvs->GetDid() : DIDNULL;
}

// .GetVdn -- Get Virtual Device Number of Parent Ddm---------------------------------DdmOsServices-
//
// Gets Vdn of Parent Ddm via Parent Ddm's base DdmServices Class.  If called
// from the actual Ddm this virtual function will be overriden and will exist
// in the derived Ddm class.
//
VDN DdmOsServices::GetVdn() { // virtual
	return (pParentDdmSvs != NULL) ? pParentDdmSvs->GetVdn() : VDNNULL;
}

// .Dispatch -- Define function code handler method ----------------------------------DdmOsServices-
//
STATUS DdmOsServices::DispatchRequest(REQUESTCODE _reqCode, DdmOsServices *pInst, RequestCallback mcMethod) { // virtual
	return (pParentDdmSvs != NULL) ? pParentDdmSvs->DispatchRequest(_reqCode,pInst,mcMethod) : DDMNOPARENTerc;
}


// .Dispatch -- Define function code handler method ----------------------------------DdmOsServices-
//
STATUS DdmOsServices::DispatchRequest(REQUESTCODE _reqCode, RequestCallback mcMethod) {
	return DispatchRequest(_reqCode,this,mcMethod);
}

// .DispatchSignal -- Define signal handler method -----------------------------------DdmOsServices-
//
STATUS DdmOsServices::DispatchSignal(SIGNALCODE nSignal, DdmOsServices *pInst, SignalCallback mcMethod) { // virtual
	return (pParentDdmSvs != NULL) ? pParentDdmSvs->DispatchSignal(nSignal,pInst,mcMethod) : DDMNOPARENTerc;
}

// .DispatchSignal -- Define signal handler method -----------------------------------DdmOsServices-
//
STATUS DdmOsServices::DispatchSignal(SIGNALCODE nSignal, SignalCallback mcMethod) {
	return DispatchSignal(nSignal,this,mcMethod);
}

// .FilterReply -- Invoke Reply Filter -----------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::FilterReply(Message *pMsg) { // virtual
	return (pParentDdmSvs != NULL) ? pParentDdmSvs->FilterReply(pMsg) : DDMNOPARENTerc;
}

// .Serve -- Private function code messages ------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Serve(REQUESTCODE, BOOL fLocal) {

	Tracef("[WARNING] Obsolete method called in \"%s\". Use BuildSys %s Macro (DdmOsServices::Serve)\n",GetDdmClassName(),fLocal ? "SERVELOCAL" : "SERVEVIRTUAL");
	return OK;
}

// .DiscardReply -- Generic Reply callback handler -----------------------------------DdmOsServices-
//
ERC DdmOsServices::DiscardReply(Message *pMsg) {
	TRACEF(TRACE_L3,("EXEC  Ddm::DiscardReply; RQ=%x did=%x pMsg=%x\n",pMsg->reqCode,GetDid(),pMsg));
	
	delete pMsg;
	
	return OK;
}

// .DiscardOkReply -- Generic Reply callback handler ---------------------------------DdmOsServices-
//
ERC DdmOsServices::DiscardOkReply(Message *pMsg) {
	
	TRACEF(TRACE_L3,("EXEC  Ddm::DiscardOkReply; RQ=%x did=%x pMsg=%x\n",pMsg->reqCode,GetDid(),pMsg));
	if (pMsg->Status() != OK) {
		Tracef("*\n* did=%lx(%s) Rq=%s(%x) erc=%u (DdmOsServices::DiscardOkReply)\n*\n",GetDid(),GetDdmClassName(), NameRq(pMsg->reqCode),pMsg->reqCode,pMsg->Status());
	}
	delete pMsg;
	
	return OK;
}
// .GetDdmClassName -- Returns pointer to ClassName text -----------------------------DdmOsServices-
//
// This cannot be defined in DdmOsServices.h since it references DdmManager.h
// which is not a public include.
//
// Returns "" if did is not valid on this IOP.
//
char *DdmOsServices::GetDdmClassName(DID did) { // static

	DidMan *pDidMan;

	if ((pDidMan = DidMan::GetDidMan(did)) == NULL)
		return "";
	
	return pDidMan->pClass->pszName;
}

//***			   ***
//*** Send Methods ***
//***			   ***

// .Send -- Route request code messages w/callback -----------------------------------DdmOsServices-
//
STATUS DdmOsServices::Send(Message *pMsg, void *pContext, ReplyCallback rc) {

	pMsg->BindContext(this, rc, pContext, GetDid()); 

	STATUS status_;
	
	if (DdmManager::requestRouteMap.Get(pMsg->reqCode)->IsDid())
		status_= FailSafe::Send(DdmManager::requestRouteMap.Get(pMsg->reqCode)->did, pMsg);

	else if (DdmManager::requestRouteMap.Get(pMsg->reqCode)->IsVdn())
		status_= FailSafe::Send(DdmManager::requestRouteMap.Get(pMsg->reqCode)->vdn, pMsg);
	
	else status_=CTS_CHAOS_UNKNOWN_FUNCTION;
	
	if (status_ != OK)
		return Reply(pMsg, status_);
		
	return OK;
}

// .Send -- Route request code messages No callback ----------------------------------DdmOsServices-
//
STATUS DdmOsServices::Send(Message *pMsg, void *pContext, DID didInitiator, IcQueue *pQueue) {

	pMsg->BindContext(0l, pContext, didInitiator); 

	if (pQueue)
		pMsg->pInitiatorContext->Link(pQueue);

	STATUS status_;
	
	if (DdmManager::requestRouteMap.Get(pMsg->reqCode)->IsDid())
		status_= FailSafe::Send(DdmManager::requestRouteMap.Get(pMsg->reqCode)->did, pMsg);

	else if (DdmManager::requestRouteMap.Get(pMsg->reqCode)->IsVdn())
		status_= FailSafe::Send(DdmManager::requestRouteMap.Get(pMsg->reqCode)->vdn, pMsg);
		
	else status_=CTS_CHAOS_UNKNOWN_FUNCTION;

	if (status_ != OK) {
		pMsg->DetailedStatusCode = status_;
		return Messenger::Reply(pMsg);
	}
		
	return OK;
}

// .Send -- Send a message to particular Ddm selected by the did ---------------------DdmOsServices-
//
// Non-FailSafe message methods
//
STATUS DdmOsServices::Send(Message *pMsg, long signature, void *pContext, DID didTarget) {

	pMsg->BindContext(signature, pContext, GetDid()); 
	// Use FailSafe anyway, so messages get returned if target fails.
	STATUS status_= FailSafe::Send(didTarget, pMsg);
	
	if (status_ != OK)
		return Reply(pMsg, status_);

	return OK;
}


// .Reply -- Return the message to the sender ----------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Reply(Message *pMsg) { 
	TRACE(TRACE_MESSAGE, TRACE_L4, ("%s(%08lx)::Reply(%s(%08lx),%x)\n",DdmOsServices::GetDdmClassName(GetDid()),GetDid(),NameRq(pMsg->reqCode),pMsg->reqCode,pMsg->DetailedStatusCode));

	if (pMsg->flags & MESSAGE_FLAGS_FILTER)
		FilterReply(pMsg);
		
	return Messenger::Reply(pMsg, (pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
}

// .Reply -- Reply to message  -------------------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Reply(Message *pMsg, STATUS detailedStatus) {
	TRACE(TRACE_MESSAGE, TRACE_L4, ("%s(%08lx)::Reply(%s(%08lx),%x)\n",DdmOsServices::GetDdmClassName(GetDid()),GetDid(),NameRq(pMsg->reqCode),pMsg->reqCode,detailedStatus));

	if (pMsg->flags & MESSAGE_FLAGS_FILTER)
		FilterReply(pMsg);
		
	pMsg->DetailedStatusCode = detailedStatus;
		
	return Messenger::Reply(pMsg, (pMsg->flags & MESSAGE_FLAGS_LAST) != 0);
}

// .Reply -- Reply to message  -------------------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Reply(Message *pMsg, STATUS detailedStatus, BOOL fLast) {
	TRACE(TRACE_MESSAGE, TRACE_L4, ("%s(%08lx)::Reply(%s(%08lx),%x)\n",DdmOsServices::GetDdmClassName(GetDid()),GetDid(),NameRq(pMsg->reqCode),pMsg->reqCode,detailedStatus));

	if (pMsg->flags & MESSAGE_FLAGS_FILTER)
		FilterReply(pMsg);
		
	pMsg->DetailedStatusCode = detailedStatus;
		
	return Messenger::Reply(pMsg, fLast); 
}

// .Signal -- Signal Ddm (ourself) ---------------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Signal(SIGNALCODE nSignal,void *pPayload) {

	return Messenger::Signal(nSignal,pPayload,GetDid());	// Signal Myself
}

// .Action -- Queue Action Callback --------------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Action(DdmServices *pInst,ActionCallback ecb,void *pPayload) {

	U32 idLocal = DeviceId::IdLocal(GetDid());

	return DidMan::PutAction(idLocal,pInst,ecb,pPayload);
}

// .Action -- Queue Action Callback Static -------------------------------------------DdmOsServices-
//
STATUS DdmOsServices::Action(ActionCallbackStatic ecbs,void *pPayload) {

	U32 idLocal = DeviceId::IdLocal(GetDid());

	return DidMan::PutAction(idLocal,ecbs,pPayload);
}

// .Fail -- Fail this slot -----------------------------------------------------------DdmOsServices-
//
void DdmOsServices::Fail() {
#ifndef WIN32
	Transport::Fail();
#endif
}


// .Fail -- Fail specified slot ------------------------------------------------------DdmOsServices-
//
void DdmOsServices::Fail(TySlot tySlot_, Status status_) {
#ifndef WIN32
	Transport::Fail(tySlot_, status_);
#endif
}

//**************************************************************************************************
// $Log: /Gemini/Odyssey/Oos/DdmOsServices.cpp $
// 
// 35    2/09/00 3:08p Tnelson
// Removed references to unused include files.  No code changes.
// 
// 34    12/09/99 2:06a Iowa
// 
// 32    9/16/99 3:19p Tnelson
// 
// 3/27/99 Tom Nelson: Minor changes for DidMan class
//

