/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidLock.h
//
// Description:	Raid Range locking class
//
//
// Update Log: 
//	4/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidLock_h
#define __RaidLock_h

class Lock
{

public:

	U32			Lba;
	U32			Count;
	Lock		*pLockForw;
	Lock		*pLockBack;
	Lock		*pWaiting;
	void		*pCaller;
};

class RaidLock
{
	Lock		*pHead;
	Lock		*pTail;

public:

	RaidLock();
	BOOL		GetReqLock(Reqblk *pReq);
	void		GetReqLockWait(Reqblk *pReq);
	BOOL		GetIoreqLock(Ioreq *pIoreq);
	void		GetIoreqLockWait(Ioreq *pIoreq);
	void		ReleaseReqLock(Reqblk *pReq);
	void		ReleaseIoreqLock(Ioreq *pIoreq);

private:

	Lock		*AcquireLock(Lock *pLock);
	void		InsertLock(Lock *pLock, Lock *pPrev, Lock *pNext);
	void		PutOnWaitingChain(Lock *pLock, Lock *pWaitHead);
	Lock		*ReleaseLock(void *pCaller);
};


#endif






