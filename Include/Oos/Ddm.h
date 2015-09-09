/* Ddm.h -- Ddm Class Definitions (public)
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
//  6/12/98 Joe Altmaier: Create file
// ** Log at end-of-file **

#ifndef __Ddm_h
#define __Ddm_h

// Public Includes
#include "DdmOsServices.h"

//**
//** Public Base class requests
//**
class RqOsDdmPing : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_PING };
	
	RqOsDdmPing() : Message(RequestCode) {}
};


//**
//** Internal Use Only 
//**
class RqOsDdmQuiesce : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_QUIESCE };
	
	RqOsDdmQuiesce() : Message(RequestCode) {}
};

class RqOsDdmEnable : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_ENABLE };
	
	struct Payload {
		DDMMODE mode;
		
		Payload(DDMMODE _mode) : mode(_mode) {}
	} payload;
	
	RqOsDdmEnable(DDMMODE _mode) : payload(_mode), Message(RequestCode) {}
	
	DDMMODE Mode()	{ return payload.mode; }
};

class RqOsDdmMode : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_MODE };
	
	struct Payload {
		DDMMODE mode;
		
		Payload(DDMMODE _mode) : mode(_mode) {}
	} payload;
	
	RqOsDdmMode(DDMMODE _mode) : payload(_mode), Message(RequestCode) {}
};

class RqOsDdmInitialize : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_INITIALIZE };
	
	struct Payload {
		DDMMODE mode;
		
		Payload(DDMMODE _mode) : mode(_mode) {}
	} payload;
	
	RqOsDdmInitialize(DDMMODE _mode) : payload(_mode), Message(RequestCode) {}
};

class RqOsDdmTerminate : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_TERMINATE };
	
	RqOsDdmTerminate() : Message(RequestCode) {}
};

// Messages in queue after delete will be bounced back
// to the sender with No such device.  Replies in the
// queue will be deleted without notice.
class RqOsDdmHalt : public Message {
public:
	enum { RequestCode = REQ_OS_DDM_HALT };
	
	RqOsDdmHalt() : Message(RequestCode) {}
};

// Want to keep dispatch classes private to OS
class RequestDispatchTable;	
class SignalDispatchTable;

class Ddm : public DdmServices {
	friend class DidMan;
	friend class ReplyLnk;
	friend class SignalLnk;
	friend class EnableLnk;
	friend class InitializeLnk;
	
//protected:
private:
	DidMan *pDidMan;	// -> Our DidMan
		
	void *pConfig;		// -> derived Ddm config area
	U32   sConfig;		// Size of config area (in bytes)
	
	RequestDispatchTable *pRequestDispatchTable;
	SignalDispatchTable *pSignalDispatchTable;
	
	BOOL fExit;

public:
	Ddm(DID did);	// constructor
	~Ddm();
	
//***
//*** API DdmServices Method Hooks
//***
	virtual DID GetDid();
	virtual VDN GetVdn();
	ERC SendDirect(Message *pMsg,ReplyCallback pMethod);

protected:
	virtual ERC DispatchRequest(REQUESTCODE reqCode, DdmServices *pInst, RequestCallback mc);
	virtual ERC DispatchSignal(SIGNALCODE nSignal, DdmServices *pInst, SignalCallback mc);
	
	ERC DispatchRequest(REQUESTCODE reqCode, RequestCallback mc) {
		return DispatchRequest(reqCode,this,mc);
	};
	ERC DispatchSignal(SIGNALCODE nSignal, SignalCallback mc) {
		return DispatchSignal(nSignal,this,mc);
	};
	
//***
//*** API Ddm only 
//***
protected:
	void SetConfigAddress(void *_pConfig,U32 _sConfig)	{ pConfig = _pConfig; sConfig = _sConfig; }
	
//***
//*** API Default Handlers
//***
protected:
	virtual ERC SignalDefault(U16 nSignal,void *pPayload);		// Derived Ddm may overide
	virtual ERC RequestDefault(Message *pMsg);					// Derived Ddm may overide
	virtual ERC ReplyDefault(Message *pMsg);					// Derived Ddm may overide

	virtual ERC DoWork(Message *pMsg);	// ***OBSOLETE***		// Derived Ddm may overide

//***
//*** Default Initialize Request
//***
private:
	ERC ProcessInitializeRequest(Message *);
	ERC ProcessInitializeReply(Message *);
protected:
	virtual ERC Initialize(RqOsDdmInitialize *pMsg);// Derived Ddm may overide
	virtual ERC Initialize(Message *pMsg);			// DEPRECATED
	
//***
//*** Default Enable Request
//***
private:
	ERC ProcessEnableRequest(Message *pMsg);
	ERC ProcessEnableReply(Message *pMsg);
protected:
	virtual ERC Enable(RqOsDdmEnable *pMsg);	// Derived Ddm may overide
	virtual ERC Enable(Message *pMsg);			// DEPRECATED

//***
//*** Default ModeChange Request
//***
private:
	ERC ProcessModeRequest(Message *pMsg);
	ERC ProcessModeReply(Message *pMsg);
protected:
	virtual ERC ModeChange(RqOsDdmMode *pMsg);		// Derived Ddm may overide

//***
//*** Default Terminate Request
//***
private:
	ERC ProcessTerminateRequest(Message *pMsg);
protected:
	virtual ERC Terminate(RqOsDdmTerminate *pMsg);		// Derived Ddm may overide

//***
//*** Default Quiesce Request
//***
private:
	ERC ProcessQuiesceRequest(Message *pMsg);
protected:	
	virtual ERC Quiesce(RqOsDdmQuiesce *);		// Derived Ddm may overide
	virtual ERC Quiesce(Message *);				// DEPRECATED
	
//***
//*** Defaul Halt Request
//***
private:
	ERC ProcessHaltRequest(Message *pMsg);

//***
//*** Default Ping Request
//***
private:
	ERC ProcessPingRequest(Message *pMsg);

//***
//*** DidMan Entry points
//***
private:
	ERC RequestDispatcher(Message *pMsg);						// Requests (message)
	ERC ReplyDispatcher(Message *pMsg);							// Reply (message)
	ERC SignalDispatcher(SIGNALCODE nSignal,void *pPayload);	// Signals
	ERC StartInitialize();
	ERC StartEnable();
	ERC QueueModeChange();
	BOOL FilterRequest(Message *pMsg);

	virtual ERC FilterReply(Message *pMsg);

//***
//*** Support Methods
//***
private:
	void CopyConfigData(Message *pReply);
};

#endif	// __Ddm_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/Ddm.h $
// 
// 20    2/08/00 8:49p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 21    2/08/00 6:12p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
//  5/07/99 Eric Wedel:   Removed inline keys on virtuals which aren't inline (GH).
//  3/23/99 Tom Nelson:   Rewrite using DidMan. Supports Ddm Fault-in/Deferred Requests
//  1/21/98 Tom Nelson:   Split DdmServices Class from Ddm
// 12/10/98 Joe Altmaier: new Ddm api
//  9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
//  6/12/98 Joe Altmaier: Create file
