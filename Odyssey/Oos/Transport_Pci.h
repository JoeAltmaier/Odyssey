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
// 08/14/99 Joe Altmaier: Create file from Transport.h
/*************************************************************************/

#ifndef __Transport_Pci_h
#define __Transport_Pci_h

#include "Transport.h"

#include "Address.h"
#include "Dma.h"
#include "HeapPci.h"
#include "PciSlot.h"
#include "RqOsTimer.h"

// Size of all Galileo I2O Free/Post queues
#define TRANSPORT_QUEUE_SIZE 16384

struct BufferList {
	PciSlot &pciSlot;
	U32 paPost;
	U32 nBuf;
	U32 aPaBuf[30];
	TyDma *pTd;
	
	BufferList(PciSlot &, U32 paPost);
	~BufferList();
	// Add a buffer
	void Add(U32 pa);
	// Free all buffers
	void Free();
	};


class Transport_Pci : public TransportBase {
	static
	char *pMsgs;
	static
	U32 sMsgs;
	static
	char *pBufs;
	static
	U32 sBufs;

	PciSlot pciSlot;
	
public:

	static
	HeapPci heapPci;
	
public:
	Transport_Pci(TySlot, PciSlot &);

	static
	void DeviceInitialize();

public:
	STATUS SendRemote(Message *pMsg, DID didTarget);
	STATUS ReplyRemote(Message *pMsg);
	void SetState(Address::StateIop state);
	STATUS GetStatus();
	U32 GetRemoteBuffer(U32 cb, U32 &pciRet);
	U32 GetRemoteMemory(U32 cb, char* &pRet);

	static
	TransportBase* Ctor(TySlot tySlot);

private:
	// Message internal
	void SendFixup(void *pMsg, Status status);
	void ReplyFixup(void *pMsg, Status status);

	static
	void Int_Inbound(long l);
	static
	void InboundFixup(void *pMsg, int status);
	static
	void RebuildDmaQueue(Message *pMsg, TyDma *pTd);
};
#endif


