/* DidMan.h -- Defines a single Ddm instance (Private)
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

#ifndef __DidMan_h
#define __DidMan_h

// Private Includes
#include "Task.h"
#include "Ddm.h"
#include "WaitQueue_T.h"
#include "ClassTable.h"
#include "CtEvent.h"

#define DIDMAX		10240

typedef STATUS (Ddm::*DispatcherCallback)(Message *);
typedef STATUS (DdmServices::*ActionCallback)(void *);
typedef STATUS (*ActionStaticCallback)(void *);

class DdmTask : public Task {
public:
	DdmTask(ClassEntry *pClass,CT_Task_Entry taskEntry,class DidMan *pDidMan) : Task(pClass->pszName,pClass->cbStack,taskEntry,pDidMan) {}
};

typedef enum {
	tREQUEST, tREPLY, tSIGNAL, tACTION, tACTIONSTATIC, tENABLE, tINITIALIZE
} LNKTYPE;

class DidLnk {
public:
	DidMan **pDidTag;
	DidMan *pDidMan;
	LNKTYPE tLnk;
	
	// Links to pQueue as Active or Deferred
	DidLnk(DidMan **_pDidTag,LNKTYPE _tLnk) : pDidTag(_pDidTag),pDidMan(*_pDidTag), tLnk(_tLnk) {}
	
	virtual BOOL Invoke()=0;

	BOOL DidExists()	{ return *pDidTag != NULL; }
	
	DidLnk *pNext;
	DidLnk *pPrev;
};

class ServiceTag {
	friend ServiceList;
	
	DdmSerivce *pDdmService;
	ServiceTag *pPrev,*pNext;

public:
	ServiceTag(DdmService *_pDdmService) 
	: pDdmService(_pDdmService), pNext(NULL), pPrev(NULL) {}
};

class ServiceList {
	ServiceTag *pFirst;
	ServiceTag *pFree;
	
	ServiceList(Ddm *pParentDdm) 
	: pFirst(NULL), pFree(NULL) {}

	~ServiceList()	{ /* Delete entire list */ }

	void Unlink(ServiceTag *_pTag) {
		if (_pTag->pNext)
			_pTag->pNext->pPrev = _pTag->pPrev;

		if (_pTag->pPrev)
			_pTag-pPrev->pNext = _pTag->pNext;
		else
			pFirst = _pTag->pNext;
	}

	void Link(ServiceTag *_pTag) {
		_pTag->pNext = pFirst;
		pFirst = _pTag;
	}
	ServiceTag *Add(DdmOsServices *_pInstance) {
		if (pFree) {
			pTag = pFree;
			pFree = pFree->pNext;
		}
		else
			pTag = new ServiceTag(_pInstance);
			
		Link(pTag);

		return pTag;
	}
	void Delete(ServiceTag *_pTag) {
		Unlink(_pTag);

		_pTag->pNext = pFree;
		pFree = _pTag;
	}
};

class CallbackTag {
	friend CallbackList;

	CallbackTag *pPrevSrvs, *pNextSrvs;		// Links to target DdmService
	CallbackTag *pPrev, *pNext;	

	DdmOsServices *pDdmService;	// Callback instance
	ActionCallback pMethod;		// Callback method

public:
	CallbackTag(DdmOsServices *_pDdmService, ActionCallback _pMethod) 
	: pPrevSrvs(NULL), pNextSrvs(NULL), pPrevDidMan(NULL), pNextDidMan(NULL) {
		pDdmService = _pDdmService;
		pMethod = _pMethod;
	}
	BOOL TargetExists() {
		return pPrevSrvs || pNextSrvs;
	}
};

class CallbackList {

	CallbackList(Ddm *pParentDdm);
	~CallbackList()	{ /* Delete entire list */ }
	
	void Unlink(CallbackTag *_pTag) {
		if (_pTag->pNext)
			_pTag->pNext->pPrev = _pTag->pPrev;

		if (_pTag->pPrev)
			_pTag-pPrev->pNext = _pTag->pNext;
		else
			pFirst = _pTag->pNext;
	}

	void Link(CallbackTag *_pTag) {
		_pTag->pNext = pFirst;
		pFirst = _pTag;
	}
	CallbackTag *Add(DdmOsServices *_pInstance, ActionCallback _pMethod) {
		if (pFree) {
			pTag = pFree;
			pFree = pFree->pNext;
		}
		else
			pTag = new CallbackTag(_pInstance);
			
		Link(pTag);

		return pTag;
	}
};

typedef WaitQueue_T<DidLnk> DidQueue;


// Did Instance Tag
// Assumes current Cabinet & Slot have been determined
//
class DidMan {
public:
	DID did;
	VDN vdn;
	DDMSTATE state;
	
	DDMMODE mode;		// Current mode
	DDMMODE modeNew;	// Desired mode
	
	Ddm *pDdm;
	U32 idLocal;
	BOOL fStart;				// AutoStart flag
	
	DidMan **pDidTag;			// -> element in didTag_idLocal[]

	CallbackList callbacks;		// List of all callback tags for this Did
	ServiceListe services;		// List of all DdmService instances
	
	ClassEntry *pClass;			// -> Class Definition
	DdmTask  *pTask;			// -> Ddm Thread
	DidQueue *pQueue;			// One per Ddm thread
	DidQueue queueDeferred;		// One per Ddm instance

	U32 cSend;
	U32 cReply;
	U32 cSignal;
	U32 cAction;

	
	static U32 idLocalLast;

	static DidMan *didTag_idLocal[DIDMAX];	// Static should set to zero

	static DidMan *CreateDidMan(char *pszClassName,VDN vdn,BOOL fStart);
	
	static BOOL DeleteDidMan(DID _did) {
		U32 idLocal = DeviceId::IdLocal(_did);
		if (idLocal > idLocalLast)
			return FALSE;
			
	 	delete didTag_idLocal[idLocal];
	 	didTag_idLocal[idLocal] = NULL;
	 	
	 	return TRUE;
	}

	static DidMan *GetDidMan(DID _did) {
		U32 idLocal=DeviceId::IdLocal(_did);
		if (idLocal > idLocalLast)
			return NULL;
			
		return didTag_idLocal[idLocal];
	}
	static VDN GetVdn(DID did);

	// Return class flags for Did
	static ClassFlags GetClassFlags(DID did) {
		DidMan *pDidMan;
		if ((pDidMan = GetDidMan(did)) != NULL)
			return pDidMan->pClass->flags;
			
		return 0;
	}
	// Return class Ctor for Did
	static CtorFunc GetClassCtor(DID did) {
		DidMan *pDidMan;
		if ((pDidMan = GetDidMan(did)) != NULL)
			return pDidMan->pClass->ctor;
			
		return 0;
	}

	// Set mode
	static void SetMode(DID did, DDMMODE _mode) {
		DidMan *pDidMan;
		if ((pDidMan = GetDidMan(did)) != NULL) {
			if (_mode != pDidMan->mode)
				pDidMan->modeNew = _mode;
		}
	}
	
	static void StartDdm(DID did) {
		DidMan *pDidMan;
		if ((pDidMan = GetDidMan(did)) != NULL)
			pDidMan->StartDdm();
	}
	
	static ERC PutMessage(U32 idLocal, Message *pMsg);
	static ERC PutSignal(U32 idLocal, U16 nSignal,void *pContext);
	static ERC PutAction(U32 idLocal,DdmServices *pInstance,ActionCallback pMethod,void *pContext);
	static ERC PutAction(U32 idLocal,ActionStaticCallback pFunction,void *pContext);
	static char *GetStateName(DDMSTATE state);
	static char *GetLnkName(LNKTYPE type);
	static ERC StartDid(DID did);
	static void StartVirtuals();
	
	static ServiceTag *LinkDdmService(DdmService *pDdmService) {
		// Allocate ServiceTag
		// Link ServiceTag to DidMan of this DdmService
		return NULL;
	}
	static void UnlinkDdmService(ServiceTag *pServiceTag) {
		// Mark all callback tags for this DdmService as dead
		// Unlink this service tag
	}
	
	DID IssueDid(DidMan ***ppDidTag);
	
public:	
	DidMan(VDN _vdn,ClassEntry *_pClass,BOOL _fStart);
	~DidMan() 				{  services().flush(); callbacks.flush();
							   if (pDdm) delete pDdm; pClass->nDidInstance--; }
	
	void StartThread();
	void StartDdm();		// Create Ddm Instance and Enable
	
	void FaultInDdm()		{ if (pDdm == NULL)  pDdm = pClass->ctor(did);  }
	void DeleteDdm()		{ delete pDdm; pDdm = NULL; }
		
	ERC InsertMessage(Message *pMsg);
	ERC PutMessage(Message *pMsg);
	ERC PutSignal(U16 nSignal,void *pContext);
	ERC PutAction(DdmServices *pInstance,ActionCallback pMethod,void *pContext);
	ERC PutAction(ActionStaticCallback pFunction,void *pContext);
	ERC RequeueDeferred(void);

private:
	friend class RequestLnk;
	friend class Ddm;
	ERC FilterRequest(class RequestLnk *pLnk);
	ERC FilterReply(Message *pMsg);
	static void ClassThread(VOID *argv);
	static void QueuePeek(DidMan *pDidMan);
};


class InitializeLnk : public DidLnk {
public:
	InitializeLnk(DidMan **_pDidTag) : DidLnk(_pDidTag,tINITIALIZE) {}	
	
	virtual ERC Invoke() { 
		if (DidExists()) {
			pDidMan->FaultInDdm();
			pDidMan->pDdm->SendDirect(new RqOsDdmInitialize(pDidMan->mode),REPLYCALLBACK(Ddm,Ddm::DiscardReply));
		}
		return TRUE;
	}
};

class EnableLnk : public DidLnk {
public:
	EnableLnk(DidMan **_pDidTag) : DidLnk(_pDidTag,tENABLE) {}	
	
	virtual ERC Invoke() { 
		if (DidExists()) {
			pDidMan->FaultInDdm();
			pDidMan->pDdm->SendDirect(new RqOsDdmEnable(pDidMan->mode),REPLYCALLBACK(Ddm,Ddm::DiscardOkReply));
		}
		return TRUE;
	}
};

class RequestLnk : public DidLnk {
public:
	Message *pMsg;
	
	RequestLnk(DidMan **_pDidTag,Message *_pMsg) : DidLnk(_pDidTag,tREQUEST) {
		pMsg = _pMsg;
		pMsg->flags |= MESSAGE_FLAGS_INQUEUE;
	}	
	virtual ERC Invoke() { 
		pMsg->flags &= ~MESSAGE_FLAGS_INQUEUE;

		if (!DidExists()) {	// If DID was deleted...
			Messenger::Reply(pMsg, CTS_CHAOS_DEVICE_NOT_AVAILABLE); 
			return TRUE;
		}
		
#if 0 // ** Moved to DidMan::FilterRequest() **
		if (pMsg->flags & MESSAGE_FLAGS_NOFAULTIN)
			if (pDidMan->pDdm == NULL) {
				Messenger::Reply(pMsg, CTS_CHAOS_DEVICE_NOT_AVAILABLE); 
				return TRUE;
			}

		pDidMan->FaultInDdm();
#endif // ** //

		return pDidMan->FilterRequest(this);
	}
};

class ReplyLnk : public DidLnk {
public:
	Message *pMsg;
	
	ReplyLnk(DidMan **_pDidTag,Message *_pMsg) : DidLnk(_pDidTag,tREPLY) {
		pMsg = _pMsg;
		pMsg->flags |= MESSAGE_FLAGS_INQUEUE;
	}	
	virtual ERC Invoke() { 
		pMsg->flags &= ~MESSAGE_FLAGS_INQUEUE;

		if (DidExists()) {	// If DID not deleted...
			pDidMan->FaultInDdm();
			pDidMan->pDdm->ReplyDispatcher(pMsg);
		}
		return TRUE;
	}
};

class SignalLnk : public DidLnk {
public:
	SIGNALCODE sigCode;
	void *pContext;

	SignalLnk(DidMan **_pDidTag,SIGNALCODE _sigCode,void *_pContext) : DidLnk(_pDidTag,tSIGNAL) {
		sigCode = _sigCode;
		pContext = _pContext;
	}	
	ERC Invoke() { 
		if (DidExists()) {	// If DID not deleted...
			pDidMan->FaultInDdm();
			pDidMan->pDdm->SignalDispatcher(sigCode,pContext);
		}
		return TRUE;
	}
};

class DdmOsServices;
typedef STATUS (DdmOsServices::*ActionCallback)(void *);

class ActionLnk : public DidLnk {
public:
public:
	CallbackTag *pTag;
	void *pContext;

	ActionLnk(DidMan **_pDidTag,CallbackTag *_pTag,void *_pContext) : DidLnk(_pDidTag,tACTION) {
		pTag = _pTag;
		pContext = _pContext;
	}	
	ERC Invoke() {
		if (DidExists()) // If DID not deleted...
			pDidMan->ActionDispatcher(pTag,pContext);
//			(pInstance->*pMethod)(pContext);
	
		return TRUE;
	}
};

typedef STATUS (*ActionStaticCallback)(void *);

class ActionStaticLnk : public DidLnk {
public:
	ActionStaticCallback pFunction;	// Callback method
	void *pContext;

	ActionStaticLnk(DidMan **_pDidTag,ActionStaticCallback _pFunction,void *_pContext) : DidLnk(_pDidTag,tACTIONSTATIC) {
		pFunction = _pFunction;
		pContext = _pContext;
	}	
	ERC Invoke() { 
		(*pFunction)(pContext); 
		return TRUE;
	}
};


#endif 	// __DidMan.h


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DidMan.h $
// 
// 11    2/15/00 6:07p Tnelson
// Fixes for VirtualDevice delete
// 
// 10    2/08/00 8:53p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 11    2/08/00 6:08p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 9     1/24/00 11:04a Jlane
// VD Delete support changes.  Checked in by JFL for TN.
// 
// 8     12/17/99 10:31a Cwohlforth
// Files e-mailed from Iowa, checked in by Carl W
// 
// 7     12/09/99 2:08a Iowa
// 
// 6     9/16/99 3:22p Tnelson
//
//	8/27/99 Joe Altmaier: MESSAGE_FLAGS_NOFAULTIN


