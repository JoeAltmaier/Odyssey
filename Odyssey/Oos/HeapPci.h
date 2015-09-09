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
// This class manages transfer buffers in the pci window.
// Alloc returns the next free buffer, if any.
// Freeing a block returns it to the free list.
// 
// Update Log: 
// 11/10/98 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __HeapPci_h
#define __HeapPci_h

#include "HeapLink.h"
#include "DeviceId.h"
#include "WaitQueue_T.h"
#include "Galileo.h"

class Message;

class TransportLnk {
public:
	Message *pMsg;
	
	TransportLnk *pNext;
	TransportLnk *pPrev;
	
	TransportLnk(Message *_pMsg) : pMsg(_pMsg) {}
};

typedef WaitQueue_T<TransportLnk> _BaseTransportQueue;

class TransportQueue : public _BaseTransportQueue {
public:
	STATUS PutMsg(Message *_pMsg) { 
		Put(new TransportLnk(_pMsg));
		return OK;
	}

	Message *GetMsg() { 
		TransportLnk *pLnk;
		Message *pMsg = NULL; 
		if ((pLnk = Get()) != NULL) {
			pMsg = pLnk->pMsg;
			delete pLnk;
		}
		return pMsg;
	}
	TransportQueue() : _BaseTransportQueue(1024) {};
};

class HeapPci : public HeapLink {
	typedef struct Header {	// ** Must be 8 byte aligned **
		U8 guard[8];		// Guard bytes
		int iQueue;
		HeapLink *pHeap;		// ** Must be last in header **
	} Header;

	U32 modQueue;

public:
	HeapPci();
	void *Alloc(U32 cb); // Allocate from heap buckets
	virtual U32 Free(void *); // Free block to heap bucket
		
	// Initialization
	void I2oLayout(U32 i2oBase, int cbQueue);
	void I2oFreeInit(char *pFree, U32 cb, U32 cbBuffer);
	void I2oFree(char *pFree);
	void I2oBufInit(char *pFree, U32 cb, U32 cbBuffer);
	void I2oBuf(char *pFree);
	void PublishBufferSizes(U32 sMsg, U32 sBuf);

	// Buffer mechanism
	void FreeLocalBuffer(char *pLocal, int iQueue);
	U32 GetLocalMessage();
	U32 GetLocalReturnBuffer();
	TySlot GetSlot(U32 pciRemote);

	// IOP state mechanism	
	static
	void SetState(Address::StateIop state);
	static
	BOOL CheckState();
	static
	void FailPci();
};
#endif


