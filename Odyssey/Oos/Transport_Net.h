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
// 08/14/90 Joe Altmaier: Create file from Transport.h
/*************************************************************************/

#ifndef __Transport_Net_h
#define __Transport_Net_h

#include "Transport.h"
#include "Socket.h"
#include "Task.h"
#include "WaitQueue_T.h"

typedef struct LinkMessage { LinkMessage *pNext; LinkMessage *pPrev; Message *pMsg; LinkMessage(Message *pMsg_):pMsg(pMsg_){}; } LinkMessage;
typedef WaitQueue_T<LinkMessage> TransmitQueue;

class Transport_Net : public TransportBase {
	Socket socket;
	TransmitQueue transmitQueue;

public:
	Transport_Net(TySlot tySlot, U32 ipMe, U32 ipHim, U32 port);

	static
	void DeviceInitialize();

	void SetParentDdm(DdmOsServices *pParentDdmSvc);

	STATUS SendRemote(Message *pMsg, DID didTarget);
	STATUS ReplyRemote(Message *pMsg);
	void SetState(Address::StateIop state);
	STATUS GetStatus();
	U32 GetRemoteMemory(U32 cb, char* &pRet);

	static
	TransportBase* Ctor(TySlot tySlot);

private:
	static
	void SSocketReceiver(void *);
	static
	void SSocketTransmitter(void *);

	void SocketReceiver();
	void SocketTransmitter();

	BOOL MessageReceiver();
	BOOL MessageTransmitter();
};
#endif


