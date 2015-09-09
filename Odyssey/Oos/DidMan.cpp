/* DidMan.cpp -- Defines a single Ddm instance
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
//  3/21/99 Tom Nelson: Create file
// * Log at end-of-file *

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#define _TRACEF
#define	TRACE_INDEX		TRACE_DIDMAN
#include "Odyssey_Trace.h"

#include "DidMan.h"
#include "DdmManager.h"
#include "CtEvent.h"

DidMan * DidMan::didTag_idLocal[DIDMAX];	
U32 DidMan::idLocalLast;

// .DidMan -- Constructor ---------------------------------------------------------DidMan-
//
DidMan::DidMan(VDN _vdn,ClassEntry *_pClass,BOOL _fStart) 
: pClass(_pClass),queueDeferred(_pClass->sQueue), callbacks(this), services(this) {
	TRACE_PROC(DidMan::DidMan);
	
	TRACEF(TRACE_L3, ("ENTER DidMan::DidMan; Vdn=%u class=\"%s\"\n",_vdn,_pClass->pszName));
	vdn = _vdn;
	pDdm = NULL;	// No instance yet
	state = UNINITIALIZED;
	pTask = NULL;
	pQueue = NULL;
	fStart = _fStart;
	mode = modeNew = SYSTEM;
	
	cSend = 0;
	cReply = 0;
	cSignal = 0;
	cAction = 0;
			
	did = IssueDid(&pDidTag);

	StartThread();

	if (pClass->nDidInstance > 0 && (pClass->flags & MULTIPLE) == 0) {
		Tracef("[ERROR] Attempt to start more than one instance of \"%s\" (DidMan::DidMan)\n",pClass->pszName);
		for(;;) ;
	}
	pClass->nDidInstance++;		// Count this Did
}

// .CreateDidMan -- Alloc Did/Task/Queue for DDM ----------------------------------DidMan-
//
// Does not start associated Ddm
//
DidMan * DidMan::CreateDidMan(char *pszClassName,VDN vdn,BOOL fStart) {	// static
	TRACE_PROC(DidMan::CreateDidMan);
	
	TRACEF(TRACE_L3, ("      vdn=%u \"%s\" (DidMan::CreateDidMan)\n",vdn,pszClassName));
	ClassEntry *pClass;
	
	if ((pClass = ClassTable::Find(pszClassName)) == NULL) {
		Tracef("[ERROR] Class \"%s\" is not defined in class table! (DidMan::CreateDidMan)\n",pszClassName);
		return NULL;
	}
	DidMan *pDidMan = new DidMan(vdn,pClass,fStart);
	
	return pDidMan;
}

// .StartThread -- Start Did Thread -----------------------------------------------DidMan-
// 
void DidMan::StartThread() {	// Static
	TRACE_PROC(DidMan::StartThread);
	
	TRACEF(TRACE_L3, ("      did=%lx idLocal=%x \"%s\" (DidMan::StartThread)\n",did,idLocal,pClass->pszName));

	pTask = pClass->pTask; 
	pQueue = pClass->pQueue;
	if (pQueue == NULL) {
	TRACEF(TRACE_L3, ("      New Queue/Task (DidMan::StartThread; )\n"));
		pQueue = new DidQueue(pClass->sQueue);
		pTask = new DdmTask(pClass,ClassThread,this);
		pTask->Resume();

		// All Ddm instances of a specific Class use the same Queue/Task
		// Don't reset pClass to create one task/queue per instance.	
		pClass->pQueue = pQueue;
		pClass->pTask = pTask;
	}
}

// StartDdm -- Force Ddm to be instanciated ---------------------------------------DidMan-
//
void DidMan::StartDdm() {
	TRACE_PROC(DidMan::StartDdm);
	
	if (pDdm == NULL) {
		cSend++;
		pQueue->Put(new InitializeLnk(pDidTag));
		cSend++;
		pQueue->Put(new EnableLnk(pDidTag));
	}
}

// StartDid -- Force Did to be instanciated ----------------------------------------DidMan-
//
ERC DidMan::StartDid(DID did) {	// static
	TRACE_PROC(DidMan::StartDid);
	
	DidMan *pDidMan;
	
	if ((pDidMan = GetDidMan(did)) == NULL) {
		Tracef("[ERROR %x] Invalid idLocal=%x (DidMan::StartDid)\n",CTS_CHAOS_INVALID_TARGET_ADDRESS,did);
		return CTS_CHAOS_INVALID_TARGET_ADDRESS;
	}
	pDidMan->StartDdm();

	return OK;
}

// StartVirtuals -- Force Ddms with a VDN to be instanciated -----------------------DidMan-
//
void DidMan::StartVirtuals() {	// static
	TRACE_PROC(DidMan::StartVirtuals);
	
	DidMan *pDidMan;
	
	for (U32 ii=1; ii <= idLocalLast; ii++) {
		if ((pDidMan = didTag_idLocal[ii]) != NULL)
			if (pDidMan->fStart && pDidMan->pDdm == NULL && pDidMan->vdn != VDNNULL)
				pDidMan->StartDdm();
	}
}

// IssueDid -- Issue new local Id --------------------------------------------------DidMan-
//
// Issue new local Id.  Starts with one (1).
//
DID DidMan::IssueDid(DidMan ***ppDidTag) {
	TRACE_PROC(DidMan::IssueDid);
	
	while (idLocalLast < IDLOCALMAX) {
		if (didTag_idLocal[idLocal = ++idLocalLast] == NULL) {
			didTag_idLocal[idLocal] = this;
			*ppDidTag = didTag_idLocal + idLocal;

			return DeviceId::Did(Address::GetCabinet(), Address::GetSlot(), idLocal);
		}
	}
	//Tracef("[ERROR] Unable to allocate DidMan! idLocal=%u\n",idLocalLast);

	// TODO: throw exception-fail iop
	*ppDidTag = NULL;
	
	return DIDNULL;
}

// .GetStateName -- Return state name ---------------------------------------------DidMan-
//
char * DidMan::GetStateName(DDMSTATE state) {
	
	switch(state) {
	case UNINITIALIZED:
		return "UNINITIALIZED";
	case INITIALIZING:
		return "INITIALIZING";
	case QUIESCENT:
		return "QUIESCENT";
	case ENABLING:
		return "ENABLING";
	case ENABLED:
		return "ENABLED";
	case QUIESCING:
		return "QUIESCING";
	};
	return "???";
}

// .GetLnkName -- Return DidLnk name ----------------------------------------------DidMan-
//
char * DidMan::GetLnkName(LNKTYPE type) {
	
	switch(type) {
	case tREQUEST:
		return "REQUEST";
	case tREPLY:
		return "REPLY";
	case tSIGNAL:
		return "SIGNAL";
	case tACTION:
		return "ACTION";
	case tACTIONSTATIC:
		return "ACTIONSTATIC";
	case tENABLE:
		return "ENABLE";
	case tINITIALIZE:
		return "INITIALIZE";
	};
	return "???";
}

// .FilterRequest -- Filter Request as they are dispatched ------------------------DidMan-
//
// When a Ddm Reply(s) to Enable it will call DidMan::RequeueDeferred() to
// move all deferred items to the enabled queue.
//
// This method is on executed on the Ddm Thread
//
// Returning TRUE causes pLnk to be deleted.
//
BOOL DidMan::FilterRequest(RequestLnk *pLnk) {
	TRACE_PROC(DidMan::FilterRequest);
	
	TRACEF(TRACE_L3, ("      %s(%08lx) state=%u/%s did=%x \"%s\" (DidMan::FilterRequest)\n",NameRq(pLnk->pMsg->reqCode),pLnk->pMsg->reqCode,state,GetStateName(state),pLnk->pDidMan->did,pLnk->pDidMan->pClass->pszName));

	Message *pMsg = pLnk->pMsg;
		
	if (state == ENABLED) {	// Fastest delivery of requests when enabled.
		if (mode != modeNew) {
			// Queue a mode change request
			mode = modeNew;
			pDdm->Send(did,new RqOsDdmMode(mode),REPLYCALLBACK(Ddm,Ddm::DiscardOkReply));
		}
		if (pMsg->flags & MESSAGE_FLAGS_FILTER) {
			switch (pMsg->reqCode) {
			case REQ_OS_DDM_INITIALIZE:
			case REQ_OS_DDM_ENABLE:
				Messenger::Reply(pMsg,OK);
				return TRUE;
			case REQ_OS_DDM_MODE:
				state = MODECHANGING;
				break;
			case REQ_OS_DDM_TERMINATE:
				state = TERMINATING;
				break;
			case REQ_OS_DDM_QUIESCE:
				state = QUIESCING;
				break;
			case REQ_OS_DDM_HALT:
				state = HALTING;
				break;
			}
		}
		pDdm->RequestDispatcher(pMsg);
	}
	else {
		mode = modeNew;
		switch (state) {
		case UNINITIALIZED:	// Defers new Requests until initialized.
			switch (pMsg->reqCode) {
			case REQ_OS_DDM_INITIALIZE:
				FaultInDdm();
				state = INITIALIZING;
				pDdm->RequestDispatcher(pMsg);
				break;
			case REQ_OS_DDM_QUIESCE:
			case REQ_OS_DDM_TERMINATE:
			case REQ_OS_DDM_MODE:
				Messenger::Reply(pMsg,OK);
				break;
			case REQ_OS_DDM_HALT:
				state = HALTED;
				Messenger::Reply(pMsg,OK);
				break;
			case REQ_OS_DDM_ENABLE:
			default:
				TRACEF(TRACE_L3, ("      DidMan::FilterRequest DEFER items=%u  RQ=%x\n",queueDeferred.ItemCount(),pMsg->reqCode));
				TRACEF(TRACE_L3, ("      DidMan::FilterRequest STARTINITIALIZE\n"));

				if (pMsg->flags & MESSAGE_FLAGS_NOFAULTIN) {
					if (pDdm == NULL) {
						Messenger::Reply(pMsg, CTS_CHAOS_DEVICE_NOT_AVAILABLE); 
						return TRUE;
					}
				}
				FaultInDdm();
				queueDeferred.Put(pLnk);
				// Queue an Initialize Request at head of queue
				pDdm->SendDirect(new RqOsDdmInitialize(mode),REPLYCALLBACK(Ddm,Ddm::DiscardReply));
				
				return FALSE;
			}
			break;
			
		case INITIALIZING: 	// Defers all Requests
		case ENABLING:
		case MODECHANGING:
		case TERMINATING:
		case QUIESCING: 
		case HALTING:
			queueDeferred.Put(pLnk);
			TRACEF(TRACE_L3, ("      DidMan::FilterRequest; DEFER items=%u RQ=%x didInitiator=%x\n",queueDeferred.ItemCount(),pMsg->reqCode,pMsg->didInitiator));
			return FALSE;
		
		case QUIESCENT: 	// Defers new Requests until enabled.
			switch (pMsg->reqCode) {
			case REQ_OS_DDM_ENABLE:
				state = ENABLING;
				pDdm->RequestDispatcher(pMsg);
				break;
			case REQ_OS_DDM_INITIALIZE:
			case REQ_OS_DDM_MODE:
			case REQ_OS_DDM_QUIESCE:
			case REQ_OS_DDM_TERMINATE:
				Messenger::Reply(pMsg,OK);
				break;
			case REQ_OS_DDM_HALT:
				state = HALTED;
				Messenger::Reply(pMsg,OK); 
				break;
			default:
				TRACEF(TRACE_L3, ("      DidMan::FilterRequest; DEFER items=%u RQ=%x didInitiator=%x\n",queueDeferred.ItemCount(),pMsg->reqCode,pMsg->didInitiator));
				TRACEF(TRACE_L3, ("      DidMan::FilterRequest; STARTENABLE\n"));

				queueDeferred.Put(pLnk);
				// Queue an Enable Request at head of queue
				pDdm->SendDirect(new RqOsDdmEnable(mode),REPLYCALLBACK(Ddm,Ddm::DiscardOkReply));

				return FALSE;
			}
			break;

		case HALTED:	//  Must not restart once halted!
			Messenger::Reply(pMsg, CTS_CHAOS_DEVICE_NOT_AVAILABLE); 			
			break;

		default:
			Tracef("[IMPOSSIBLE] Class=\"%s\" State=%u ReqCode=%x (DidMan::FilterRequest)\n",pClass->pszName,state,pLnk->pMsg->reqCode);
		}
	}
	return TRUE;
}

// .FilterReply -- Filter all Ddm OS Replies sent (not delivered) ---------------------------------DidMan-
//
ERC DidMan::FilterReply(Message *pMsg) {
	TRACE_PROC(DidMan::FilterReply);
	
	switch (pMsg->reqCode) {
	case REQ_OS_DDM_ENABLE:
	case REQ_OS_DDM_MODE:
	case REQ_OS_DDM_TERMINATE:
		state = ENABLED;
		RequeueDeferred();
		break;
	case REQ_OS_DDM_INITIALIZE:
	case REQ_OS_DDM_QUIESCE:
		state = QUIESCENT;
		RequeueDeferred();
		break;
	case REQ_OS_DDM_HALT:
		state = HALTED;
		RequeueDeferred();
		break;
	}
	return OK;
}

// .ActionDispatcher -- Lookup callback and dispatch into DdmService ----------------------------DidMan-
//
ERC DidMan::ActionDispatcher(CallbackTag *pTag, void *pContext) {
	TRACE_PROC(DidMan::ActionDispatcher);
	
	if (pTag->TargetExists())
		pTag->Invoke(pContext);
	else {
		Tracef("WARNING: Callback target does not exists! DID=%x (DidMan::ActionDispatcher)\n",did);
		
	return OK;
}

// .RequeueDeferred -- Link content of deferred queue to head of class queue---------------------DidMan-
//
ERC DidMan::RequeueDeferred() {
	TRACE_PROC(DidMan::RequeueDeferred);
	
	TRACEF(TRACE_L3, ("     items=%u %s (DidMan::RequeueDeferred)\n",queueDeferred.ItemCount(),pClass->pszName));
	pQueue->Insert(&queueDeferred);
	
	return OK;
}

// .InsertMessage -- Put Message at head of Class Queue ---------------------------DidMan-
//
// This method must be Thread-Safe!
//
ERC DidMan::InsertMessage(Message *pMsg) {
	TRACE_PROC(DidMan::InsertMessage);
	
	TRACEF(TRACE_L3, ("      DidMan::InsertMessage; %s RQ=%x pMsg=%x pQueue=%x to did=%u\n",pMsg->IsReply() ? "REPLY" : "REQUEST", pMsg->reqCode,pMsg,pQueue,did));
	BOOL fFull; 
	if (pMsg->IsReply())
		fFull = pQueue->Insert(new ReplyLnk(pDidTag,pMsg));
	else
		fFull = pQueue->Insert(new RequestLnk(pDidTag,pMsg));

	if (fFull)
		Tracef("WARNING Queue exceeds max size. cItems=%u; did=%x\n",pQueue->ItemCount(),did);

	return OK;
}
// .PutMessage -- Put Message in Class Queue --------------------------------------DidMan-
//
// This method must be Thread-Safe!
//
ERC DidMan::PutMessage(Message *pMsg) {
	TRACE_PROC(DidMan::PutMessage(pMsg));
	
	TRACEF(TRACE_L3, ("      DidMan::PutMessage; %s Rq=%x pMsg=%x pQueue=%x did=%x\n",pMsg->IsReply() ? "REPLY" : "REQUEST", pMsg->reqCode,pMsg,pQueue,did));
	BOOL fFull;
	
	if (pMsg->IsReply()) {
		cReply++;
		fFull = pQueue->Put(new ReplyLnk(pDidTag,pMsg));
	}
	else {
		cSend++;
		fFull = pQueue->Put(new RequestLnk(pDidTag,pMsg));
	}

	if (fFull)
		Tracef("WARNING Queue exceeds max size. cItems=%u; did=%x\n",pQueue->ItemCount(),did);
	
	return OK;
}

// Static Method
ERC DidMan::PutMessage(U32 idLocal, Message *pMsg) {	// static
	TRACE_PROC(DidMan::PutMessage(idLocal,pMsg));
	
	DidMan *pDidMan;
	
	if ((pDidMan = GetDidMan(idLocal)) == NULL) {
		Tracef("[ERROR %x] Invalid idLocal=%x (DidMan::PutMessage)\n",CTS_CHAOS_INVALID_TARGET_ADDRESS,idLocal);
		return CTS_CHAOS_INVALID_TARGET_ADDRESS;
	}
	return pDidMan->PutMessage(pMsg);
}
	
// .PutSignal -- Put Signal in Class Queue ----------------------------------------DidMan-
//
// This method must be Thread-Safe!
//
ERC DidMan::PutSignal(U16 nSignal,void *pContext) {
	TRACE_PROC(DidMan::PutSignal(nSignal,pContext));
	
	TRACEF(TRACE_L3, ("      DidMan::PutSignal; nSignal=%u did=%x\n",nSignal,did));
	SignalLnk *pLnk = new SignalLnk(pDidTag,nSignal,pContext);
	
	cSignal++;
	return pQueue->Put(pLnk);
}

// Static Method
ERC DidMan::PutSignal(U32 idLocal, U16 nSignal,void *pContext) {	// static
	TRACE_PROC(DidMan::PutSignal(idLocal,nSignal,pContext));
	
	DidMan *pDidMan;
	
	if ((pDidMan = GetDidMan(idLocal)) == NULL) {
		Tracef("[ERROR %x] Invalid idLocal=%x (DidMan::PutSignal)\n",CTS_CHAOS_INVALID_TARGET_ADDRESS,idLocal);
		return CTS_CHAOS_INVALID_TARGET_ADDRESS;
	}
	return pDidMan->PutSignal(nSignal,pContext);
}

// .PutAction -- Put Action Callback in Class Queue -------------------------------DidMan-
//
// This method must be Thread-Safe!
//
ERC DidMan::PutAction(DdmServices *pInstance,ActionCallback pMethod,void *pContext) {
	TRACE_PROC(DidMan::PutAction(pInstance,pMethod,pContext));
	
	TRACEF(TRACE_L3, ("      DidMan::PutAction; did=%x\n",did));

	CallbackTag *pTag = callbacks.Add(pInstance,pMethod);
	ActionLnk *pLnk = new ActionLnk(pDidTag,pTag,pContext);
	
	cAction++;
	BOOL fFull = pQueue->Put(pLnk);	// Not deferred
	if (fFull)
		Tracef("WARNING Queue exceeds max size. cItems=%u; did=%x\n",pQueue->ItemCount(),did);
	
	return OK;
}

// Static Method
ERC DidMan::PutAction(U32 idLocal,DdmServices *pInstance,ActionCallback pMethod,void *pContext) {// static
	TRACE_PROC(DidMan::PutAction(idLocal,pInstance,pMethod,pContext));

	DidMan *pDidMan;
	
	if ((pDidMan = GetDidMan(idLocal)) == NULL) {
		Tracef("[ERROR %x] Invalid idLocal=%x (DidMan::PutAction)\n",CTS_CHAOS_INVALID_TARGET_ADDRESS,idLocal);
		return CTS_CHAOS_INVALID_TARGET_ADDRESS;
	}
	return pDidMan->PutAction(pInstance,pMethod,pContext);
}

// .PutAction -- Put Action Callback to static in Class Queue ---------------------DidMan-
//
// This method must be Thread-Safe!
//
ERC DidMan::PutAction(ActionStaticCallback pFunction,void *pContext) {
	TRACE_PROC(DidMan::PutAction(pStaticFunction,pContext));

	TRACEF(TRACE_L3, ("      DidMan::PutAction Static; did=%x\n",did));

	ActionStaticLnk *pLnk = new ActionStaticLnk(pDidTag,pFunction,pContext);
	
	cAction++;
	BOOL fFull = pQueue->Put(pLnk);
	
	if (fFull)
		Tracef("WARNING Queue exceeds max size. cItems=%u; did=%x\n",pQueue->ItemCount(),did);

	return OK;
}

// Static Method
ERC DidMan::PutAction(U32 idLocal,ActionStaticCallback pFunction,void *pContext) {// static
	TRACE_PROC(DidMan::PutAction(idLocal,pStaticFunction,pContext));
	
	DidMan *pDidMan;
	
	if ((pDidMan = GetDidMan(idLocal)) == NULL) {
		Tracef("[ERROR %x] Invalid idLocal=%u (DidMan::PutSignal)\n",CTS_CHAOS_INVALID_TARGET_ADDRESS,idLocal);
		return CTS_CHAOS_INVALID_TARGET_ADDRESS;
	}
	return pDidMan->PutAction(pFunction,pContext);
}

// .ClassThread -- Common Ddm Class Thread ----------------------------------------DidMan-
//
// A single common thread is created to execute this message dispatcher for
// each unique Class.  In other words, all Ddms of the same Class share the
// same thread for dispatching messages.
//
// This thread's job is to simply invoke work in the class queue.
//
void DidMan::ClassThread(VOID *argv) {	// static
	TRACE_PROC(DidMan::ClassThread);
	
	DidMan *pDidMan = (DidMan*)argv;
	DidLnk *pLnk;
TRACEF(TRACE_L1, ("START  DidMan::ClassThread; pDidMan=%x pQueue=%x\"%s\"\n",pDidMan,pDidMan->pQueue,pDidMan->pClass->pszName));
	
	while (true) {
		pLnk = pDidMan->pQueue->GetWait();
TRACEF(TRACE_L1, ("\nRESUME DidMan::ClassThread; items=%u tLnk=%u/%s did=%x \"%s\"\n",pDidMan->pQueue->ItemCount(),pLnk->tLnk,GetLnkName(pLnk->tLnk),pLnk->pDidMan->did,pDidMan->pClass->pszName));
#ifdef _TRACEF
	 QueuePeek(pDidMan);
#endif
		if (pLnk->Invoke())
			delete pLnk;
	}
}

void DidMan::QueuePeek(DidMan *pDidMan) {
	DidLnk *p;
TRACEF(TRACE_L3, ("PEEK  pQueue=%x (tLnk/did/Rq):",pDidMan->pQueue));
for (int ii=0; ii < 10; ii++)
	if ((p = pDidMan->pQueue->Peek(ii)) != NULL) {
		TRACEF(TRACE_L8, (" %x/%x",p->tLnk,p->pDidMan->did));
		if (p->tLnk == tREPLY) {
			TRACEF(TRACE_L8, ("/%x",((ReplyLnk *) p)->pMsg->reqCode));
		}
		else if (p->tLnk == tREQUEST) {
			TRACEF(TRACE_L8, ("/%x",((RequestLnk *) p)->pMsg->reqCode));
		}
	}		
	else
		break;
TRACEF(TRACE_L3, ("\n"));
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DidMan.cpp $
// 
// 21    2/08/00 8:53p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 22    2/08/00 6:09p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 20    1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 19    12/09/99 2:08a Iowa
// 
// 11/9/99 Joe Altmaier: Use CTS_CHAOS status codes
//
// 15    9/16/99 3:22p Tnelson
//
//	8/27/99 Joe Altmaier: MESSAGE_FLAGS_NOFAULTIN
//

