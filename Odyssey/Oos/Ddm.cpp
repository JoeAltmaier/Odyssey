/* Ddm.cpp -- Standard Ddm base class
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
//  6/15/98 Joe Altmaier: Create file
// ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_DDM
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#include "Ddm.h"
#include "CtEvent.h"

// Private Includes
#include "DidMan.h"	
#include "DispatchTables.h"
#include "RqOsVirtualManager.h"

extern char *NameRq(REQUESTCODE rqCode);

// .Ddm -- Constructor -------------------------------------------------------------------------Ddm-
//
Ddm::Ddm(DID _did) : DdmServices(NULL), fExit(FALSE), pConfig(NULL), sConfig(0) {
	TRACE_PROC(Ddm::Ddm);
	
	pDidMan = DidMan::GetDidMan(_did);
	pDidMan->pClass->nDdmInstance++;
	
	TRACEF(TRACE_L3, ("ENTER Ddm::Ddm; did=%lx; vdn=%u; class=\"%s\"; pDidMan=%lx\n",_did,pDidMan->vdn,pDidMan->pClass->pszName,pDidMan));

	pRequestDispatchTable = new RequestDispatchTable;
	pSignalDispatchTable = new SignalDispatchTable;

	DispatchRequest(RqOsDdmInitialize::RequestCode,	REQUESTCALLBACK(Ddm,ProcessInitializeRequest));
	DispatchRequest(RqOsDdmEnable::RequestCode,		REQUESTCALLBACK(Ddm,ProcessEnableRequest));
	DispatchRequest(RqOsDdmQuiesce::RequestCode,	REQUESTCALLBACK(Ddm,ProcessQuiesceRequest));
	DispatchRequest(RqOsDdmHalt::RequestCode,		REQUESTCALLBACK(Ddm,ProcessHaltRequest));
	DispatchRequest(RqOsDdmMode::RequestCode,		REQUESTCALLBACK(Ddm,ProcessModeRequest));
	DispatchRequest(RqOsDdmTerminate::RequestCode,	REQUESTCALLBACK(Ddm,ProcessTerminateRequest));

	DispatchRequest(RqOsDdmPing::RequestCode,		REQUESTCALLBACK(Ddm,ProcessPingRequest));
}

// .~Ddm -- Destructor -------------------------------------------------------------------------Ddm-
//
Ddm::~Ddm() {
	TRACE_PROC(Ddm::~Ddm);
	
	delete pRequestDispatchTable;
	delete pSignalDispatchTable;
	
	pDidMan->RequeueDeferred();

	pDidMan->pClass->nDdmInstance--;
}

// .GetDid -- Returns current DID --------------------------------------------------------------Ddm-
//
DID Ddm::GetDid() {	// virtual
	return pDidMan->did;
}

// .GetVdn -- Returns current VDN --------------------------------------------------------------Ddm-
//
VDN Ddm::GetVdn() {	// virtual
	return pDidMan->vdn;
}

// .DispatchRequest -- Define function code handler method -------------------------------------Ddm-
//
STATUS Ddm::DispatchRequest(U32 reqCode, DdmServices *pInst, RequestCallback mcMethod) { // virtual
	
	return pRequestDispatchTable->Set(reqCode,pInst,mcMethod);
}

// .DispatchSignal -- Define signal handler method ---------------------------------------------Ddm-
//
STATUS Ddm::DispatchSignal(U16 nSignal, DdmServices *pInst, SignalCallback mcMethod) { // virtual

	return pSignalDispatchTable->Set(nSignal,pInst,mcMethod);
}

//***
//*** Default Message Handlers
//***

// .SignalDefault -- Default Signal Handler ----------------------------------------------------Ddm-
//
// Process all messages that do not have a dispatch handler set.
//
STATUS Ddm::SignalDefault(U16 ,void *) { // Virtual

	return CTS_CHAOS_INAPPROPRIATE_FUNCTION;
}

// .RequestDefault -- Default Message Handler --------------------------------------------------Ddm-
//
// Process all messages that do not have a dispatch handler set.
//
STATUS Ddm::RequestDefault(Message *pMsg) { // Virtual
	
	return DoWork(pMsg);	// DoWork is OBSOLETE
}

// .ReplyDefault -- Default Reply Handler ------------------------------------------------------Ddm-
//
// Process all replies that do not have a callback defined.
//
STATUS Ddm::ReplyDefault(Message *pMsg) {	// Virtual
	
	return DoWork(pMsg);	// DoWork is OBSOLETE	
}

// .DoWork -- Message processing method --------------------------------------------------------Ddm-
//
// This is ** OBSOLETE **
//
STATUS Ddm::DoWork(Message *) {	// virtual

	return CTS_CHAOS_INAPPROPRIATE_FUNCTION;
}

//***		     ***
//*** INITIALIZE ***
//***		     ***

// .ProcessInitializeRequest -- Default Initialize Request Handler -----------------------------Ddm-
//
// Ddm Initialize Requests are dispatched to this method.
//
STATUS Ddm::ProcessInitializeRequest(Message *pMsg) {
	TRACE_PROC(Ddm::ProcessInitializeRequest);

	if (pConfig == NULL || GetVdn() == VDNNULL)
		Initialize(pMsg);
	else { // Fetch Vdn-based persistent data from PTS
		Message *pMsgGet = new RqOsVirtualManagerGetConfig(GetVdn());

		// Remember the Initialize message as context to the Get.
		Send(pMsgGet, pMsg, REPLYCALLBACK(Ddm,ProcessInitializeReply));
	}
	return OK;
}

// .ProcessInitializeReply -- Reply to Get Handler for Initialize ------------------------------Ddm-
//
STATUS Ddm::ProcessInitializeReply(Message *_pReply) {
	TRACE_PROC(Ddm::ProcessInitializeReply);

	// Copy returned config data to pConfig.
	CopyConfigData(_pReply);
	
	Initialize((Message*) _pReply->GetContext());

	delete _pReply;

	return OK;
}

// .Initialize -- Base Initialize Handler (DEPRECATED) -----------------------------------------Ddm-
//
// This underived message argument version has been DEPRECATED!
//
STATUS Ddm::Initialize(Message *pMsg) {	// virtual
	TRACE_PROC(Ddm::Initialize(Message*));
		
	Initialize((RqOsDdmInitialize*) pMsg);	// Go to correct Initialize with derived message

	return OK;
}

// .Initialize -- Base Initialize Handler ------------------------------------------------------Ddm-
//
// Base class replies to INITIALIZE it there is no virtual derived method.
//
STATUS Ddm::Initialize(RqOsDdmInitialize *pMsg) {	// virtual
	TRACE_PROC(Ddm::Initialize(RqOsDdmInitialize*));
		
	Reply(pMsg, OK);	// Reply to the INITIALIZE that triggered all this.

	return OK;
}

//***		 ***
//*** ENABLE ***
//***		 ***

// .ProcessEnable -- Default Enable Message Handler --------------------------------------------Ddm-
//
ERC Ddm::ProcessEnableRequest(Message *pMsg) {
	TRACE_PROC(Ddm::ProcessEnableRequest);
	
	if (pConfig == NULL || GetVdn() == VDNNULL)
		Enable(pMsg);
	else { // Fetch VD-based persistent data from PTS
		Message *pMsgGet = new RqOsVirtualManagerGetConfig(GetVdn());

		// Remember the Enable message as context to the Get.
		Send(pMsgGet, pMsg, REPLYCALLBACK(Ddm,ProcessEnableReply));
	}
	return OK;
}

// .ProcessEnableReply -- Reply to Get Handler for Enable --------------------------------------Ddm-
//
ERC Ddm::ProcessEnableReply(Message *_pReply) {
	TRACE_PROC(Ddm::ProcessEnableReply);
	
	// Copy returned config data to pConfig.
	CopyConfigData(_pReply);
	
	Enable((Message*) _pReply->GetContext());

	delete _pReply;

	return OK;
}

// .Enable -- Base Enable Handler (DEPRECATED) -------------------------------------------------Ddm-
//
// This underived message argument version has been DEPRECATED!
//
ERC Ddm::Enable(Message *pMsg) {	// virtual
	TRACE_PROC(Ddm::Enable(Message*));
	
	Enable((RqOsDdmEnable*) pMsg);		// Go to correct Enable with derived message
	
	return OK;
}

// .Enable -- Base Enable Handler --------------------------------------------------------------Ddm-
//
// Base class replies to ENABLE it there is no virtual derived method.
//
ERC Ddm::Enable(RqOsDdmEnable *pMsg) {	// virtual
	TRACE_PROC(Ddm::Enable(RqOsDdmEnable*));
	
	Reply(pMsg, OK);	// Reply to the ENABLE that triggered all this.
	
	return OK;
}

//***	   ***
//*** MODE ***
//***	   ***

// .ProcessModeRequest -- Default Mode Message Handler -----------------------------------------Ddm-
//
ERC Ddm::ProcessModeRequest(Message *pMsg) {
	TRACE_PROC(Ddm::ProcessModeRequest);
	
	if (pConfig == NULL || GetVdn() == VDNNULL)
		ModeChange((RqOsDdmMode*)pMsg);
	else { // Fetch VD-based persistent data from PTS
		Message *pMsgGet = new RqOsVirtualManagerGetConfig(GetVdn());

		// Remember the Enable message as context to the Get.
		Send(pMsgGet, pMsg, REPLYCALLBACK(Ddm,ProcessModeReply));
	}
	return OK;
}

// .ProcessModeReply -- Reply to Get Handler for Mode -------------------------------------------Ddm-
//
ERC Ddm::ProcessModeReply(Message *_pReply) {
	TRACE_PROC(Ddm::ProcessModeReply);
	
	// Copy returned config data to pConfig.
	CopyConfigData(_pReply);
	
	ModeChange((RqOsDdmMode*) _pReply->GetContext());

	delete _pReply;

	return OK;
}

// .ModeChange -- Base Mode Handler ------------------------------------------------------------Ddm-
//
// Base class replies to ENABLE it there is no virtual derived method.
//
ERC Ddm::ModeChange(RqOsDdmMode *pMsg) {	// virtual
	TRACE_PROC(Ddm::Mode);
	
	Reply(pMsg, OK);	// Reply to the Mode that triggered all this.
	
	return OK;
}

//***	   		***
//*** TERMINATE ***
//***	   		***

// .ProcessTerminateRequest -- Default Terminate Message Handler --------------------------------Ddm-
//
ERC Ddm::ProcessTerminateRequest(Message *pMsg) {
	TRACE_PROC(Ddm::ProcessTerminateRequest);
	
	Terminate((RqOsDdmTerminate*) pMsg);

	return OK;
}

// .Terminate -- Base Terminate Handler --------------------------------------------------------Ddm-
//
// Base class replies to TERMINATE if there is no virtual derived method.
//
ERC Ddm::Terminate(RqOsDdmTerminate *pMsg) {	// virtual
	TRACE_PROC(Ddm::Terminate);
	
	Reply(pMsg, OK);	// Reply to the Mode that triggered all this.
	
	return OK;
}

//***		  ***
//*** QUIESCE ***
//***		  ***

// .ProcessQuiesce -- Quiesce Message Handler --------------------------------------------------Ddm-
//
// Process Quiesce Requests.
//
ERC Ddm::ProcessQuiesceRequest(Message *pMsg) { 
	TRACE_PROC(Ddm::ProcessQuiesceRequest);
	
	Quiesce(pMsg);

	return OK;
}

// .Quiesce -- Base Quiesce Handler (DEPRECATED) ----------------------------------------------Ddm-
//
// This underived message argument version has been DEPRECATED!
//
ERC Ddm::Quiesce(Message *pMsg) { // virtual
	TRACE_PROC(Ddm::Quiesce);
	
	Quiesce((RqOsDdmQuiesce*) pMsg);	// Go to correct Quiesce with derived message
	
	return OK;
}

// .Quiesce -- Base Quiesce Handler ------------------------------------------------------------Ddm-
//
// Base class replies to QUIESCE if there is no virtual derived method.
//
ERC Ddm::Quiesce(RqOsDdmQuiesce *pMsg) { // virtual
	TRACE_PROC(Ddm::Quiesce(RqOsDdmQuiesce*));
	
	Reply(pMsg, OK);	// Reply to the Quiesce/Halt that triggered all this.
	
	return OK;
}


//***	   ***
//*** HALT ***
//***	   ***

// .ProcessHalt -- Halt Message Handler -------------------------------------------------------Ddm-
//
// Process Halt Requests.  Invokes Quiesce Method.
//
ERC Ddm::ProcessHaltRequest(Message *pMsg) { 
	TRACE_PROC(Ddm::ProcessHaltRequest);
	
	Quiesce(pMsg);	// Redudant Halt Requests are caught by DidMan
	
	return OK;
}

//***	   ***
//*** PING ***
//***	   ***

// .ProcessPing -- Ping Message Handler --------------------------------------------------------Ddm-
//
ERC Ddm::ProcessPingRequest(Message *pMsg) {
	TRACE_PROC(Ddm::ProcessPingRequest);
	
	Reply(pMsg, OK);

	return OK;
}

//***
//*** DidMan Entry points
//***

// .SendDirect -- Send message to ourself with priority ----------------------------------------Ddm-
//
ERC Ddm::SendDirect(Message *pMsg,ReplyCallback pMethod) {
	
	ERC erc;

	// Add reply callback context so handler can Reply() in the normal manner.
	pMsg->BindContext(this, pMethod, NULL, GetDid());
	
	if ((erc = pMsg->BindTarget(GetDid())) != OK)
		return erc;

	return pDidMan->InsertMessage(pMsg);	// Insert at head of queue
}

// .SignalDispatcher -- Dispatch DDM Signal ----------------------------------------------------Ddm-
//
ERC Ddm::SignalDispatcher(U16 nSignal,void *pPayload)
{
	SignalDispatchMethod *pSdm;
	ERC erc;
	
	if ((pSdm = pSignalDispatchTable->Get(nSignal))->IsValid())
		erc = pSdm->Invoke(nSignal,pPayload);
	else
		erc = SignalDefault(nSignal, pPayload);
	
	return erc;
}

// .RequestDispatcher -- Dispatch DDM Message --------------------------------------------------Ddm-
//
ERC Ddm::RequestDispatcher(Message *pMsg) {
	TRACE_PROC(Ddm::RequestDispatcher);
	
TRACE(TRACE_MESSAGE, TRACE_L3, ("%s(%08lx)::RequestDispatcher(%s(%08lx) %s \n", DdmOsServices::OSGetClassName(GetDid()), GetDid(), NameRq(pMsg->reqCode),pMsg->reqCode,pRequestDispatchTable->Get(pMsg->reqCode)->IsValid() ? "Dispatched" : "DoWork"));
	
TRACEF(TRACE_L3, ("ENTER Ddm::RequestDispatcher; %s(%08lx) %s did=%x \"%s\"\n",NameRq(pMsg->reqCode),pMsg->reqCode,pRequestDispatchTable->Get(pMsg->reqCode)->IsValid() ? "Dispatched" : "DoWork",GetDid(),DdmOsServices::OSGetClassName(GetDid())));
TRACEF(TRACE_L3, ("      Ddm::RequestDispatcher; pMsg=%x didTarget=%x didInitiator=%x\n",pMsg,pMsg->didTarget,pMsg->didInitiator));

		
	STATUS erc;
	RequestDispatchMethod *pRdm;

	if ((pRdm = pRequestDispatchTable->Get(pMsg->reqCode))->IsValid())
		erc = pRdm->Invoke(pMsg);
	else {
		if ((erc = RequestDefault(pMsg)) != OK)
			Reply(pMsg,erc,TRUE);
	}

	return OK;	// QueueDeferred
}

// .ReplyDispatcher -- Dispatch DDM Message ----------------------------------------------------Ddm-
//
ERC Ddm::ReplyDispatcher(Message *pMsg) {
	TRACE_PROC(Ddm::ReplyDispatcher);
	
#ifndef WIN32
TRACE(TRACE_MESSAGE, TRACE_L3, ("%s(%08lx)::ReplyDispatcher(%s(%08lx)) \n",OSGetClassName(),GetDid(),NameRq(pMsg->reqCode),pMsg->reqCode));
TRACEF(TRACE_L3, ("ENTER Ddm::ReplyDispatcher; RQ=%x did=%x \"%s\"\n",pMsg->reqCode,GetDid(),OSGetClassName()));
TRACEF(TRACE_L3, ("EXIT  Ddm::ReplyDispatcher; pMsg=%x didTarget=%x didInitiator=%x\n",pMsg,pMsg->didTarget,pMsg->didInitiator));
#endif

	InitiatorContext *pTc = pMsg->PopIc();
	BOOL fLast = pMsg->IsLast();
	ERC erc;
	
	if (pTc == NULL)
		TRACEF(TRACE_L8, ("Ddm::ReplyDispatcher .....  Invalid!  pTc=%x\n",pTc));
		
	if (pTc->replyCallbackMethod.IsValid()) {
#ifdef DEBUG
		BOOL fDeleted=false;
		pMsg->pDeleted=&fDeleted;
		REQUESTCODE rqCode=pMsg->reqCode;
#endif
		erc = pTc->replyCallbackMethod.Invoke((MessageReply*)pMsg);
#ifdef DEBUG
		if (!fDeleted)
			TRACEF(TRACE_L2, ("Ddm::ReplyDispatcher [WARNING] Reply not deleted, possible leak in %s handler for message %s.\nUse MSGREPLYKEEP(pMsg) to suppress this warning.\n", pDidMan->pClass->pszName, NameRq(rqCode)));
#endif
		}
	else
		if ((erc = ReplyDefault(pMsg)) != OK) {
			if (pMsg->pInitiatorContext != NULL)
				Reply(pMsg);
			else
				delete pMsg;
		}
TRACEF(TRACE_L3, ("      Ddm::ReplyDispatcher; erc=%u\n",erc));
	
	if (fLast)
		delete pTc;
			
TRACEF(TRACE_L3, ("EXIT  Ddm::ReplyDispatcher\n"));
	return OK;
}

// .FilterReply -- Filter all Ddm OS Replies we send -------------------------------------------Ddm-
//
ERC Ddm::FilterReply(Message *pMsg) {	// virtual
	TRACE_PROC(Ddm::FilterReply);
	
TRACEF(TRACE_L3, ("     Ddm::FilterReply; RQ=%x state=%u/%s did=%x \"%s\"\n",pMsg->reqCode,pDidMan->state,DidMan::GetStateName(pDidMan->state),GetDid(),OSGetClassName()));

	pDidMan->FilterReply(pMsg);

	return OK;
}

// .CopyConfigData -- Copy PTS config data from reply message ----------------------------------Ddm-
//
void Ddm::CopyConfigData(Message *_pReply) {
	TRACE_PROC(Ddm::CopyConfigData);
	
	// The Persistent Data Service replied with our configuration information.
	// Copy it and return the INITIALIZE request.

	// Copy returned config data to pConfig.
	RqOsVirtualManagerGetConfig *pReply = (RqOsVirtualManagerGetConfig *)_pReply;
	
	U32 cb;
	char *pData = (char *) pReply->GetConfigDataPtr(&cb);
	if (cb != sConfig)
		Tracef("\*\n* WARNING Size of config area does not match PTS record size.\n        Class \"%s\" did=%x; cbPts=%u; cbConfig=%u\n*\n",
				OSGetClassName(), GetDid(),cb, sConfig);
	
	for (unsigned int ib=0; ib < cb && ib < sConfig; ib++)
		((char*)pConfig)[ib] = pData[ib];
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/Ddm.cpp $
// 
// 42    2/15/00 7:30p Jaltmaier
// Leak:    Undeleted Reply detector.  Better leak statistics.
// 13121: CleanAllSgl usage cleanup.
// Unit test: remove GetBootData reference from MSL_Initialize.
// 
// 43    2/14/00 11:15a Jaltmaier
// Termination stuff.
// Message reply leak detector.
// 
// 42    2/08/00 7:48p Tnelson
// 
// 40    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 39    12/09/99 2:05a Iowa
//
//  5/10/99 Eric Wedel:   Tweaked use of REQUESTCALLBACK() macro (Green Hills).
//  3/23/99 Tom Nelson:   Rewrite using DidMan. Supports Ddm Fault-in/Deferred Requests
// 12/07/98 Tom Nelson:   Implement Call-back methods

