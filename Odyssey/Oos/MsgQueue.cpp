/* MsgQueue.cpp -- Ddm MsgQueue Class
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
 * THIS CLASS MUST BE THREAD-SAFE!!!
 *
**/

// Revision History:
//  1/27/99 Tom Nelson:	Created
//  2/10/99 Joe Altmaier: Put Reschedule
//  2/15/99 Joe Altmaier: Put cannot Reschedule (done from ISR), but Get can.
//  3/16/99 Tom Nelson: Added Put Actions (Callbacks)
//


#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "CtEvent.h"
#include "MsgQueue.h"

// my ruler	
//345678901234567890123456789012345678901234567890123456789012345678901234567890



#if 0
// MsgQueue -- Constructor --------------------------------------------MsgQueue-
//
MsgQueue::MsgQueue(U32 sQueue) {

	MsgLnk *pLnk;
	U16 ii;
	
	pFirst = pLast = NULL;
	pTaskBlocked = NULL;
	pFree = new MsgLnk[++sQueue];	// Plus 1 dis-allows zero
//Tracef("sQueue=%u; pFree=%08lx\n",sQueue,pFree);
	
	for (pLnk = pFree,ii=0; ii < sQueue-1; pLnk++,ii++) 
		pLnk->pNext = pLnk+1;
	pLnk->pNext = NULL;

//for (pLnk=pFree,ii=0; pLnk!=NULL; pLnk=pLnk->pNext,ii++) ;
//Tracef("Total queue size=%u\n",ii);
		
}


// Get -- Get next item from queue ------------------------------------MsgQueue-
//
MsgLnk* MsgQueue::Get(Task *pTask) {

	MsgLnk *pLnk = NULL;
	
	while (pLnk == NULL) {
		CriticalSection::Enter();
		if (IsEmpty()) {
			pTaskBlocked = pTask;
			CriticalSection::Leave();
			pTaskBlocked->Suspend();
		}
		else {
			pLnk = UnlinkFirst();
			CriticalSection::Leave();
		}
	}
	Task::Reschedule();

	return pLnk;
}

// .GetMessage -- Get Message for task ---------------------------------MsgQueue-
//
Message * MsgQueue::GetMessage(Task *pTask) {

	MsgLnk *pLnk;
	Message *pMsg;
	
	pLnk = Get(pTask);
	pMsg = pLnk->pMsg;
	FreeLnk(pLnk);
	
	return pMsg;
}


// Must not have Constructor/Destructor!!
class MsgLnk : public QueLnk {
	Message *pMsg;
	
	
	virtual Link(Message *_pMsg) {
		
	}
	virtual Invoke() {
}

// Put -- Put Message in queue ----------------------------------------MsgQueue-
//
ERC MsgQueue::Put(TagDdm *pDdmTag,Message *pMsg) {

	MsgLnk *pLnk;
	
	CriticalSection::Enter();
		
	if (IsFull()) {		// No Link Blocks!
		CriticalSection::Leave();
		return CTS_CHAOS_TPT_SEND_CONGESTION;
	}
		
	pLnk = AllocLnk();
	pLnk->tMsg = tMSG;
	pLnk->pDdmTag = pDdmTag;

	pLnk->pMsg = pMsg;
	pLnk->pMsg->flags |= MESSAGE_FLAGS_INQUEUE;
	
	LinkLast(pLnk);
			
	if (pTaskBlocked != NULL) {
		Task *pTaskResume = pTaskBlocked;
		pTaskBlocked = NULL;
		CriticalSection::Leave();
	
		pTaskResume->Resume();
	}
	else
		CriticalSection::Leave();
		
	return CTS_CHAOS_SUCCESS;
}

// Put -- Put Signal in queue -----------------------------------------MsgQueue-
//
ERC MsgQueue::Put(TagDdm *pDdmTag,U16 nSignal,void *pPayload) {

	MsgLnk *pLnk;
	
	CriticalSection::Enter();
		
	if (IsFull()) {		// No Link Blocks!
		CriticalSection::Leave();
		return CTS_CHAOS_TPT_SEND_CONGESTION;
	}
		
	pLnk = AllocLnk();
	pLnk->tMsg = tSIGNAL;
	pLnk->pDdmTag = pDdmTag;

	pLnk->nSignal = nSignal;
	pLnk->pPayload = pPayload;

	LinkLast(pLnk);
			
	if (pTaskBlocked != NULL) {
		Task *pTaskResume = pTaskBlocked;
		pTaskBlocked = NULL;
		CriticalSection::Leave();
	
		pTaskResume->Resume();
	}
	else
		CriticalSection::Leave();
		
	return CTS_CHAOS_SUCCESS;
}

// Put -- Put Action (callback) in queue -------------------------------MsgQueue-
//
ERC MsgQueue::Put(TagDdm *pDdmTag,DdmOsServices *pInstance,ActionCallback pMethod,void *pPayload) {

	MsgLnk *pLnk;
	
	CriticalSection::Enter();
		
	if (IsFull()) {		// No Link Blocks!
		CriticalSection::Leave();
		return CTS_CHAOS_TPT_SEND_CONGESTION;
	}
		
	pLnk = AllocLnk();
	pLnk->tMsg = tACTION;
	pLnk->pDdmTag = pDdmTag;

	pLnk->actionMethod.pInstance = pInstance;
	pLnk->actionMethod.pMethod = pMethod;
	pLnk->pPayload = pPayload;
	
	LinkLast(pLnk);
			
	if (pTaskBlocked != NULL) {
		Task *pTaskResume = pTaskBlocked;
		pTaskBlocked = NULL;
		CriticalSection::Leave();
	
		pTaskResume->Resume();
	}
	else
		CriticalSection::Leave();
		
	return CTS_CHAOS_SUCCESS;
}

// Put -- Put Action (callback to static ) in queue --------------------MsgQueue-
//
ERC MsgQueue::Put(TagDdm *pDdmTag,ActionCallbackStatic pCallback,void *pPayload) {

	MsgLnk *pLnk;
	
	CriticalSection::Enter();
		
	if (IsFull()) {		// No Link Blocks!
		CriticalSection::Leave();
		return CTS_CHAOS_TPT_SEND_CONGESTION;
	}
		
	pLnk = AllocLnk();
	pLnk->tMsg = tACTIONSTATIC;
	pLnk->pDdmTag = pDdmTag;

	pLnk->actionStatic.pCallback = pCallback;
	pLnk->pPayload = pPayload;
	
	LinkLast(pLnk);
			
	if (pTaskBlocked != NULL) {
		Task *pTaskResume = pTaskBlocked;
		pTaskBlocked = NULL;
		CriticalSection::Leave();
	
		pTaskResume->Resume();
	}
	else
		CriticalSection::Leave();
		
	return CTS_CHAOS_SUCCESS;
}

#endif

