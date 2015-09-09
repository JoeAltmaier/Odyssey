/* DdmOsServices.h -- Ddm Os Services Class Definitions
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
//  1/21/99 Tom Nelson: Split OsDdmServices Class from Ddm
// ** Log at end-of-file **

#ifndef __DdmOsServices_h
#define __DdmOsServices_h

#include "Os.h"
#include "CtEvent.h"
#include "Message.h"
#include "Messenger.h"
#include "FailSafe.h"
#include "RequestCodes.h"
#include "Kernel.h"
#include "Event.h"

class DdmOsServices  {
public:
	typedef STATUS (DdmOsServices::*RequestCallback)(Message*);
	typedef STATUS (DdmOsServices::*SignalCallback)(SIGNALCODE nSignal,void *pPayload);
//  yes, we now have three (3) different ways of getting a ptr to member func:
#ifdef WIN32
#	define REQUESTCALLBACK(clas,method)	(RequestCallback) method
#	define SIGNALCALLBACK(clas,method)	(SignalCallback) method
#elif defined(__ghs__) // Green Hills
#	define REQUESTCALLBACK(clas,method)	(RequestCallback) &clas::method
#	define SIGNALCALLBACK(clas,method)	(SignalCallback) &clas::method
#else	// MetroWerks
#	define REQUESTCALLBACK(clas,method)	(RequestCallback)&method
#	define SIGNALCALLBACK(clas,method)	(SignalCallback)&method
#endif

protected:	// Not thread-safe
	
	typedef RequestCallback	MessageCallback;	// ***DEPRECATED***
	
	DdmOsServices *pParentDdmSvs;
	class ServiceTag *pServiceTag;
	
	DdmOsServices();	// ***DEPRECATED ***
	
	DdmOsServices(DdmOsServices *_pParentDdmSvc);
	~DdmOsServices();

	// DEPRECATED - Use contructor with parent pointer argument
	void SetParentDdm(DdmOsServices *_pParentDdmSvc)	{ pParentDdmSvs = _pParentDdmSvc; }

public:
	// DdmServices-Ddm hooks only overriden in class Ddm
	virtual DID GetDid();
	virtual VDN GetVdn();

protected:
	virtual STATUS DispatchRequest(REQUESTCODE reqCode, DdmOsServices *pInst, RequestCallback mc);

	STATUS DispatchRequest(REQUESTCODE reqCode, RequestCallback mc);

	virtual STATUS DispatchSignal(SIGNALCODE nSignal, DdmOsServices *pInst, SignalCallback mc);

	STATUS DispatchSignal(SIGNALCODE nSignal, SignalCallback mc);

//#	define DISPATCH(function, callback) DispatchRequest((U16) function, (RequestCallback) &callback)

	virtual STATUS FilterReply(Message *pMsg);
	
	STATUS Serve(REQUESTCODE, BOOL );	//*** OBSOLETE - Use Buildsys SERVE macros ***

	ERC DiscardReply(Message *pMsg);	// Generic Reply Callback handler deletes message
	ERC DiscardOkReply(Message *pMsg);

//***
//*** API Send Methods
//***
public:	// Thread-Safe		

	//*** Send Via VDN Methods
	
	// With Callback

	STATUS Send(VDN vdnTarget, Message *pMsg, void *pContext, ReplyCallback rc) {
		pMsg->BindContext(this, rc, pContext, GetDid()); 
		STATUS status_=FailSafe::Send(vdnTarget, pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(VDN vdnTarget, Message *pMsg, ReplyCallback rc) {
		return Send(vdnTarget,pMsg, (void*)pMsg->contextTransaction.LowPart, rc); 
	}

	// No Callback

	STATUS Send(VDN vdnTarget, Message *pMsg, void *pContext) {
		pMsg->BindContext(0l, pContext, GetDid()); 
		STATUS status_=FailSafe::Send(vdnTarget, pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(VDN vdnTarget, Message *pMsg) {
		return Send(vdnTarget,pMsg,(void*)pMsg->contextTransaction.LowPart);
	}

	//*** Send Via DID Methods ***
	
	// With Callback
		
	STATUS Send(DID didTarget, Message *pMsg, void *pContext, ReplyCallback rc) {
		pMsg->BindContext(this, rc, pContext, GetDid());
		STATUS status_= FailSafe::Send(didTarget, pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(DID didTarget, Message *pMsg, ReplyCallback rc) {
		return Send(didTarget,pMsg,(void*)pMsg->contextTransaction.LowPart, rc); 
	}
	
	// No Callback
		
	STATUS Send(DID didTarget, Message *pMsg, void *pContext) {
		pMsg->BindContext(0l, pContext, GetDid()); 
		STATUS status_= FailSafe::Send(didTarget, pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(DID didTarget, Message *pMsg) {
		return Send(didTarget,pMsg,(void*)pMsg->contextTransaction.LowPart); 
	}
	
	//*** Send Via TySlot Methods ***
	
	// With Callback
		
	STATUS Send(TySlot tySlot, Message *pMsg, void *pContext, ReplyCallback rc) {
		if (tySlot == Address::iSlotMe)
			return Send(pMsg,pContext,rc);	// Attempt local routing
			
		pMsg->BindContext(this, rc, pContext, GetDid());
		STATUS status_=  FailSafe::Send(DeviceId::Did(0, tySlot, IDNULL), pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(TySlot tySlot, Message *pMsg, ReplyCallback rc) {
		return Send(tySlot,pMsg,(void*)pMsg->contextTransaction.LowPart, rc); 
	}
	
	// No Callback
		
	STATUS Send(TySlot tySlot, Message *pMsg, void *pContext) {
		if (tySlot == Address::iSlotMe)
			return Send(pMsg,pContext);		// Attempt local routing

		pMsg->BindContext(0l, pContext, GetDid()); 
		STATUS status_=  FailSafe::Send(DeviceId::Did(0, tySlot, IDNULL), pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}
	STATUS Send(TySlot tySlot, Message *pMsg) {
		return Send(tySlot,pMsg,(void*)pMsg->contextTransaction.LowPart); 
	}
	
	//*** Send Via Routed Methods ***

	// With Callback
	
	STATUS Send(Message *pMsg, void *pContext, ReplyCallback rc);
	
	STATUS Send(Message *pMsg, ReplyCallback rc) {
		return Send(pMsg, (void*)pMsg->contextTransaction.LowPart, rc);
	}
	
	// No Callback
	static
	STATUS Send(Message *pMsg, void *pContext, DID didInitiator, IcQueue *pQueue=NULL);
	
	STATUS Send(Message *pMsg, void *pContext) {
		return Send(pMsg, pContext, GetDid());
	}
	STATUS Send(Message *pMsg) {
		return Send(pMsg, (void*)pMsg->contextTransaction.LowPart, GetDid());
	}

	// Send the message pMsg to the virtual device vd, NOT anticipating a reply.
	// In the event of a failure reforward the message to the alternate driver
	// as defined by vdnTarget.
	STATUS Forward(Message *pMsg, VDN vdnTarget) {
		return FailSafe::Forward(vdnTarget, pMsg);
	}	

	//*** Send with signature *** OBSOLETE ***

	STATUS Send(Message *pMsg, long signature, void *pContext, DID did);

	STATUS Send(Message *pMsg, long signature, void *pContext, VDN vdnTarget) {
		pMsg->BindContext(signature, pContext, GetDid()); 
		STATUS status_= FailSafe::Send(vdnTarget, pMsg);
		if (status_ != OK)
			return Reply(pMsg, status_);

		return OK;
	}	

	// Bind the message pMsg to the source DID didInitiator.
	// Send it to the device didTarget, anticipating a reply.
	// No automatic failover processing will occur.
	// Used by the receiving transport.
	static
	STATUS Send(DID didTarget, Message *pMsg, DID didInitiator, IcQueue *pQueue_) {
		if (didTarget == DIDNULL)
			return Send(pMsg, NULL, didInitiator, pQueue_); // Route by function code

		else {
			pMsg->BindContext(0l, NULL, didInitiator);

			if (pQueue_)
				pMsg->pInitiatorContext->Link(pQueue_);
			
			STATUS status_= FailSafe::Send(didTarget, pMsg);
			if (status_ != OK) {
				pMsg->DetailedStatusCode = status_;
				return Messenger::Reply(pMsg);
				}

			return OK;
			}
	}

//***
//*** API Reply Methods
//***
public:	// Thread-Safe
	
	// Return the message pMsg to whoever sent it to us that was anticipating a reply.
	STATUS Reply(Message *pMsg);
	
	STATUS Reply(Message *pMsg, STATUS detailedStatus);

	STATUS Reply(Message *pMsg, STATUS detailedStatus, BOOL fLast);

//***
//*** API Actions/Signals
//***
public:	// Thread-Safe		
	STATUS Signal(SIGNALCODE nSignal,void *pPayload = NULL);

	typedef STATUS (DdmOsServices::*ActionCallback)(void *);
	typedef STATUS (*ActionCallbackStatic)(void *);

#ifdef WIN32
#	define ACTIONCALLBACK(clas,method)		(ActionCallback) method
#	define ACTIONCALLBACKSTATIC(clas,method)	(ActionCallbackStatic) method
#elif defined(__ghs__) // Green Hills
#	define ACTIONCALLBACK(clas,method)		(ActionCallback) &clas::method
#	define ACTIONCALLBACKSTATIC(clas,method)	(ActionCallbackStatic) &clas::method
#else	// MetroWerks
#	define ACTIONCALLBACK(clas,method)		(ActionCallback) &method
#	define ACTIONCALLBACKSTATIC(clas,method)	(ActionCallbackStatic) &method
#endif

	STATUS Action(ActionCallbackStatic ec,void *pPayload=NULL);
	STATUS Action(DdmOsServices *pInst,ActionCallback ec,void *pPayload=NULL);
	
	STATUS Action(ActionCallback ec,void *pPayload=NULL) {
		return Action(this,ec,pPayload);
	}
	
//***
//*** API Miscellaneous
//***
public:	// Thread-Safe
	I64 Time() { return Kernel::Time_Stamp(); }
	
	REFNUM MakeRefNum()	{ return Os::MakeRefNum();	}
	
	static char *GetDdmClassName(DID did);
	char *GetDdmClassName() 				{ return GetDdmClassName(GetDid()); }

	// **DEPRICATED** Use GetDdmClassName()
	static char *OSGetClassName(DID did)	{ return GetDdmClassName(did); }
	char *OSGetClassName() 					{ return GetDdmClassName(GetDid()); }

	Message *Migrate(Message *pMsg) { return pMsg->Migrate(); }
	
	void Fail();
	void Fail(TySlot tySlot, Status status);

	// These are DDM versions of logging functions.  Non-DDM versions have the same interface, 
	// but don't add DID and VDN to the Event object.
	inline void LogEvent(STATUS eventCode) 
	{ ::LogEvent(new Event(eventCode, GetDid(), GetVdn()), (void*)NULL, (void*)NULL, (void*)NULL); }
	template<class T1> inline void LogEvent(STATUS eventCode, T1 arg1)
	{ ::LogEvent(new Event(eventCode, GetDid(), GetVdn()), arg1, (void*)NULL, (void*)NULL); }
	template <class T1, class T2> inline void LogEvent(STATUS eventCode, T1 arg1, T2 arg2)
	{ ::LogEvent(new Event(eventCode, GetDid(), GetVdn()), arg1, arg2, (void*)NULL); }
	template <class T1, class T2, class T3> inline void LogEvent(STATUS eventCode, T1 arg1, T2 arg2, T3 arg3)
	{ ::LogEvent(new Event(eventCode, GetDid(), GetVdn()), arg1, arg2, arg3); }
	
};

typedef DdmOsServices DdmServices;

#endif // __DdmOsServices_h

//**************************************************************************************************
// $Log: /Gemini/Include/Oos/DdmOsServices.h $
// 
// 40    12/09/99 1:38a Iowa
// 
// 39    10/14/99 11:42a Jlane
// Remove duplicate declaration of DiscardOKReply().
// 
// 38    10/14/99 4:15a Iowa
// Iowa merge
//
//  1/21/99 Tom Nelson: Split OsDdmServices Class from Ddm
//  3/27/99 Tom Nelson: Minor changes for DidMan class
//  4/21/99 Joe Altmaier: Cheap.
//  4/21/99 Joe Altmaier: Cheap deferred.
//  5/07/99 Eric Wedel: Changed REQUESTCALLBACK() and SIGNALCALLBACK() macros
//                      to take new class name argument (for Green Hills).
//  5/10/99 Bob Butler: Added convenience functions for Event Logging.
//  5/11/99 Eric Wedel: Did same for ACTIONCALLBACK() and ACTIONCALLBACKSTATIC()
//                      macros as well (also for Green Hills).
