/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidLock.cpp
//
// Description:	Raid Range locking class
//				An instance of this class may be used with Reqblks
//				or Ioreqs, but not both
//
//
// Update Log: 
//	4/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/


#include "OsTypes.h"
#include "CtTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "RaidStructs.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "RaidQ.h"
#include "Member.h"
#include "Raid.h"
#include "RaidLock.h"

/*************************************************************************/
// RaidLock
// Constructor method for the class RaidLock
/*************************************************************************/

RaidLock::RaidLock()
{
	pHead = pTail = NULL;
}

/*************************************************************************/
// GetReqLock
// Lock a range of LBAs for a Reqblk
// Return TRUE if able to lock
// Return FALSE if already locked
/*************************************************************************/

BOOL	RaidLock::GetReqLock(Reqblk *pReq)
{
	Lock	*pLock;

	TRACE_ENTRY(RaidLock::GetReqLock);

	// get new lock and fill in
	pLock = new Lock;
	pLock->Lba = pReq->Lba;
	pLock->Count = pReq->Count;
	pLock->pCaller = pReq;
	pLock->pWaiting = NULL;
	pLock->pLockForw = NULL;
	pLock->pLockBack = NULL;

	if (AcquireLock(pLock) != NULL)
	{	// range is already locked
		delete pLock;
		return (FALSE);
	}
	return (TRUE);
}

/*************************************************************************/
// GetReqLockWait
// Lock a range of LBAs for a Reqblk
// If range is already locked, chain lock request on waiting chain
// Callback routine in pReq will be called when lock is acquired
/*************************************************************************/

void	RaidLock::GetReqLockWait(Reqblk *pReq)
{
	Lock	*pLock, *pNext;

	TRACE_ENTRY(RaidLock::GetReqLockWait);

	// get new lock and fill in
	pLock = new Lock;
	pLock->Lba = pReq->Lba;
	pLock->Count = pReq->Count;
	pLock->pCaller = pReq;
	pLock->pWaiting = NULL;
	pLock->pLockForw = NULL;
	pLock->pLockBack = NULL;

	if ((pNext = AcquireLock(pLock)) != NULL)
	{	// range is locked, put on wait chain
		PutOnWaitingChain(pLock, pNext);
	}
	else	// got lock, do callback
		( (pReq->pInst)->*(pReq->pCallback) )(pReq);
}

/*************************************************************************/
// GetIoreqLock
// Lock a range of LBAs for an Ioreq
// Return TRUE if able to lock
// Return FALSE if already locked
/*************************************************************************/

BOOL	RaidLock::GetIoreqLock(Ioreq *pIoreq)
{
	Lock	*pLock;

	TRACE_ENTRY(RaidLock::GetIoreqLock);

	// get new lock and fill in
	pLock = new Lock;
	pLock->Lba = pIoreq->Lba;
	pLock->Count = pIoreq->Count;
	pLock->pCaller = pIoreq;
	pLock->pWaiting = NULL;
	pLock->pLockForw = NULL;
	pLock->pLockBack = NULL;

	if (AcquireLock(pLock) != NULL)
	{	// range is already locked
		delete pLock;
		return (FALSE);
	}
	return (TRUE);
}

/*************************************************************************/
// GetIoreqLockWait
// Lock a range of LBAs for an Ioreq
// If range is already locked, chain lock request on waiting chain
// Callback routine in pIoreq will be called when lock is acquired
/*************************************************************************/

void	RaidLock::GetIoreqLockWait(Ioreq *pIoreq)
{
	Lock	*pLock, *pNext;
	U8		index;

	TRACE_ENTRY(RaidLock::GetIoreqLockWait);

	// get new lock and fill in
	pLock = new Lock;
	pLock->Lba = pIoreq->Lba;
	pLock->Count = pIoreq->Count;
	pLock->pCaller = pIoreq;
	pLock->pWaiting = NULL;
	pLock->pLockForw = NULL;
	pLock->pLockBack = NULL;

	if ((pNext = AcquireLock(pLock)) != NULL)
	{	// range is locked, put on wait chain
		PutOnWaitingChain(pLock, pNext);
	}
	else	// got lock, do callback
	{
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
}

/*************************************************************************/
// ReleaseReqLock
// Release lock, and check for any requests that are waiting
// for a lock in this range
/*************************************************************************/

void	RaidLock::ReleaseReqLock(Reqblk *pReq)
{
	Lock	*pLock, *pWait, *pNext, *pLocked;

	TRACE_ENTRY(RaidLock::ReleaseReqLock);

	pLock = ReleaseLock( (void *)pReq);
	// anybody waiting for this lock ?
	pWait = pLock->pWaiting;
	while (pWait)
	{
		pNext = pWait->pWaiting;
		pWait->pWaiting = NULL;
		pReq = (Reqblk *)pWait->pCaller;
		// try to acquire the lock
		if ((pLocked = AcquireLock(pWait)) != NULL)
		{	// range is locked, put on wait chain
			PutOnWaitingChain(pWait, pLocked);
		}
		else	// got lock, do callback
			( (pReq->pInst)->*(pReq->pCallback) )(pReq);
		pWait = pNext;
	}
	delete pLock;
}

/*************************************************************************/
// ReleaseIoreqLock
// Release lock, and check for any requests that are waiting
// for a lock in this range
/*************************************************************************/

void	RaidLock::ReleaseIoreqLock(Ioreq *pIoreq)
{
	Lock	*pLock, *pWait, *pNext, *pLocked;
	U8		index;

	TRACE_ENTRY(RaidLock::ReleaseIoreqLock);

	pLock = ReleaseLock( (void *)pIoreq);
	// anybody waiting for this lock ?
	pWait = pLock->pWaiting;
	while (pWait)
	{
		pNext = pWait->pWaiting;
		pWait->pWaiting = NULL;
		pIoreq = (Ioreq *)pWait->pCaller;
		// try to acquire the lock
		if ((pLocked = AcquireLock(pWait)) != NULL)
		{	// range is locked, put on wait chain
			PutOnWaitingChain(pWait, pLocked);
		}
		else	// got lock, do callback
		{
			index = --pIoreq->iCall;
			( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
		}
		pWait = pNext;
	}
	delete pLock;
}

/*************************************************************************/
// AcquireLock
// 
/*************************************************************************/

Lock	*RaidLock::AcquireLock(Lock *pLock)
{
	Lock	*pPrev, *pNext;
	U32		Lba, Count;

	TRACE_ENTRY(RaidLock::AcquireLock);

	Lba = pLock->Lba;
	Count = pLock->Count;
	pPrev = NULL;
	pNext = pHead;
	while (pNext)
	{
		if (Lba < pNext->Lba)
		{
			if ((Lba + Count) <= pNext->Lba)
			{
				InsertLock(pLock, pPrev, pNext);
				return (NULL);		// got lock
			}
			else
				return (pNext);		// range is locked by pNext
		}
		else
		{
			if ((pNext->Lba + pNext->Count) > Lba)
				return (pNext);		// range is locked by pNext
		}
		pPrev = pNext;
		pNext = pNext->pLockForw;
	}
	InsertLock(pLock, pPrev, pNext);
	return (NULL);		// got lock
}

/*************************************************************************/
// InsertLock
// Put pLock on chain between pPrev and pNext
/*************************************************************************/

void	RaidLock::InsertLock(Lock *pLock, Lock *pPrev, Lock *pNext)
{
	TRACE_ENTRY(RaidLock::InsertLock);

	pLock->pLockForw = pNext;
	if (pNext)
		pNext->pLockBack = pLock;
	else		// pLock is new tail
	{
		if (pTail)
			pTail->pLockForw = pLock;
		pTail = pLock;
	}
	pLock->pLockBack = pPrev;
	if (pPrev)
		pPrev->pLockForw = pLock;
	else		// pLock is new head
	{
		if (pHead)
			pHead->pLockBack = pLock;
		pHead = pLock;
	}
}

/*************************************************************************/
// PutOnWaitingChain
// 
/*************************************************************************/

void	RaidLock::PutOnWaitingChain(Lock *pLock, Lock *pWaitHead)
{
	Lock	*pWait, *pPrev;

	TRACE_ENTRY(RaidLock::PutOnWaitingChain);

	pWait = pWaitHead->pWaiting;
	pPrev = NULL;
	while (pWait)
	{
		pPrev = pWait;
		pWait = pWait->pWaiting;
	}
	pLock->pWaiting = NULL;
	if (pPrev)
		pPrev->pWaiting = pLock;
	else
		pWaitHead->pWaiting = pLock;
}

/*************************************************************************/
// ReleaseLock
// Find pLock and remove from chain
/*************************************************************************/

Lock	*RaidLock::ReleaseLock(void *pCaller)
{
	Lock	*pNext, *pPrev, *pLock;

	TRACE_ENTRY(RaidLock::ReleaseLock);

	pLock = NULL;
	pNext = pHead;
	// find pLock
	while (pNext)
	{
		if (pNext->pCaller == pCaller)
		{
			pLock = pNext;
			break;
		}
		pNext = pNext->pLockForw;
	}
	// remove pLock from chain
	pPrev = pLock->pLockBack;
	pNext = pLock->pLockForw;
	if (pPrev)
		pPrev->pLockForw = pNext;
	else
		pHead = pNext;
	if (pNext)
		pNext->pLockBack = pPrev;
	else
		pTail = pPrev;
	return (pLock);
}



