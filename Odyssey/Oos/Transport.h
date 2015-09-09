/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class implements the message transport.
// 
// Update Log: 
// 10/12/98 Joe Altmaier: Create file
// 11/11/98 Joe Altmaier: Add heapPci.
// 4/15/99 Joe Altmaier: reorder.
/*************************************************************************/

#ifndef __Transport_h
#define __Transport_h

#include "OsTypes.h"
#include "Message.h"
#include "DeviceId.h"
#include "Ddm.h"
#include "Dma.h"


// This is the base class for individual transport objects, one per connection.
class TransportBase: public DdmServices, public CallbackTarget {
protected:
	TySlot tySlot;

public:
	// Transport for messages TO each slot
	static
	TransportBase *aPTransport[NSLOT];

	// Queue of messages FROM each slot
	static
	IcQueue *pQueue_iSlot[NSLOT];

	TransportBase(TySlot tySlot);

	virtual
	STATUS SendRemote(Message *pMsg, DID didTarget)=0;
	virtual
	STATUS ReplyRemote(Message *pMsg)=0;

	virtual
	void SetState(Address::StateIop state)=0;

	virtual
	STATUS GetStatus()=0;

	virtual
	void SetParentDdm(DdmOsServices *pParentDdmSvc);

	virtual
	U32 GetRemoteMemory(U32 cb, char* &pRet)=0;

};


// This is the transport Ddm class that handles configuration messages 
class Transport : public Ddm {
		
	// Return path
	static
	DID didTransport;

public:
	// TransportBase registry
	typedef TransportBase* (*TransportConstructor)(TySlot);	
	enum TyTransport {TRANSPORT_PCI, TRANSPORT_NET, TRANSPORT_MAX};
	static
	TransportConstructor aTransportCtor[TRANSPORT_MAX];

	// Signals
	enum {SIG_IOP_FAIL, SIG_IOP_UP};
	
public:
	Transport(DID did);

	STATUS Initialize(Message *pMsg);

	static
	void DeviceInitialize();

	static
	Ddm *Ctor(DID did);

	static
	TransportBase* CtorNull(TySlot) { return NULL; }

	static
	void Fail();
	static
	void Fail(TySlot tySlot, Status status);
	
	static
	void SendFailed(Message *pMsg, Status status);
	static
	void ReplyFailed(Message *pMsg, Status status);
	static
	void NotifyFailure(TySlot tySlot, Status status);
	static
	void Register(TyTransport, TransportConstructor tc);

private:
	void SelectMasterHbc(TySlot tySlotHbcMaster);
	STATUS ReplyTimer(Message *pMsg);
	STATUS ReplyListenVdn(Message *pMsg);
	STATUS ReplyListenIop(Message *pMsg);

	STATUS ProcessVdnStopRequest(Message *pMsg);
	STATUS ProcessIopStopRequest(Message *pMsg);
	STATUS ProcessIopStartRequest(Message *pMsg);
	STATUS ProcessHbcMasterRequest(Message *pMsg);
	STATUS ProcessIopFail(SIGNALCODE sig, void *context);
	STATUS ProcessIopUp(SIGNALCODE sig, void *context);
};
#endif


