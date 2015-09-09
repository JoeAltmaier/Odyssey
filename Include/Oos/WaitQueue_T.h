/* WaitQueue_T.h -- Blocking Queue Template Definition
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Create a TaskQueue class that holds items of specified type.
 *
 * Usage:
 *		typedef WaitQueue_T<MyItem> MyQueue;
 *
 *		class MyItem {
 *			// my stuff
 *		public:
 *			MyItem *pNext;
 *			MyItem *pPrev;
 *		}
**/
 
// Revision History:
//  3/24/99 Tom Nelson: Create file
//  5/07/99 Eric Wedel: Removed trailing ';' from funcs -- Green Hills fix.
//  8/06/99 Eric Wedel: Fixed bug in Unlink():  pLast not updated properly.
//

#ifndef __WaitQueue_T_h
#define __WaitQueue_T_h

#ifdef WIN32
#include "win_WaitQueue_T.h"
#else

#include "Critical.h"
#include "Semaphore.h"

template <class T>
class WaitQueue_T {
	T *pFirst;
	T *pLast;

	U32 sQueueMax;
	U32 cItems; 
	U32 nWait;
	Semaphore semaphore;
	
public:
	WaitQueue_T<T>(U32 _sQueueMax) : semaphore(0),nWait(0) { sQueueMax = _sQueueMax; pFirst = pLast = NULL; cItems=0; }
	
	BOOL IsEmpty() { return pFirst == NULL; }
	BOOL IsFull()  { return cItems >= sQueueMax; }
	U32  ItemCount() { return cItems;	}

	T *Peek(U32 nItem) { 
		Critical s;
		T *p;
		int ii;
		for (ii=0,p=pFirst; ii < (signed)nItem; ii++,p=p->pNext) {
			if (p == NULL)
				return NULL;
		}
		return p;
	}

	T *Get() { 		// Get without blocking
		Critical section;
		return UnlinkFirst(); 
	}
	T *GetWait();	// Get with blocking

	ERC Put(T *pT);

	// Link to beginning of Queue
	ERC Insert(T *pT);
	ERC Insert(WaitQueue_T<T> *pQ);

	void Unlink(T *pLnk);	// Remove from queue

private:	
	T *UnlinkFirst(void);
	void LinkFirst(T *pLnk);
	void LinkLast(T *pLnk);
};

// GetWait -- Get T from queue--------------------------------------WaitQueue_T-
// 
// Get first T in Queue. Block thread if Queue is empty
//
// Thread SAFE
//
template <class T>
T* WaitQueue_T<T>::GetWait() {

	T *pLnk;
	
	Critical section;

	while ((pLnk = UnlinkFirst()) == NULL) {
		if (cItems > 0) {
			Tracef("WARNING WaitQueue @%x is empty but cItems=%u\n",this,cItems);
			cItems = 0;
		}
		++nWait;

		section.Leave();
		semaphore.Wait();
		section.Enter();
	}
#ifdef WIN32
	section.Leave();
#endif
	Task::Reschedule();

	return pLnk;
}

// Put -- Put T in queue -------------------------------------------WaitQueue_T-
// 
// Put T as end of Queue.  Resume Thread if one is blocked on Queue.
// Returns TRUE if queue is overflowing.
//
// Thread SAFE
//
template <class T>
BOOL WaitQueue_T<T>::Put(T *pT) {

	Critical section;

	LinkLast(pT);
			
	if (nWait == 0)
		section.Leave();
	else {
		--nWait;
		section.Leave();
		semaphore.Signal();
	}
		
	return IsFull();
}

// Insert -- Insert T at beginning of queue ----------------------WaitQueue_T-
// 
// Put T as beginning of Queue.  Resume Thread if one is blocked on Queue.
// Returns TRUE if queue is overflowing.
//
// Thread SAFE
//
template <class T>
BOOL WaitQueue_T<T>::Insert(T *pT) {

	Critical section;

	LinkFirst(pT);
			
	if (nWait == 0)
		section.Leave();
	else {
		--nWait;
		section.Leave();
		semaphore.Signal();
	}
	return IsFull();
}

// Insert -- Put entire Queue at head on this queue --------------WaitQueue_T-
// 
// Put T as end of Queue.  Resume Thread if one is blocked on Queue.
// Returns TRUE if queue is overflowing.
//
// Thread SAFE
//
template <class T>
BOOL WaitQueue_T<T>::Insert(WaitQueue_T<T> *pQ) {

	Critical section;

	if (pQ->pLast == NULL)	// Other queue is empty!
		section.Leave();
	else {
		if ((pQ->pLast->pNext = pFirst) == NULL) {	// We were empty
			pFirst = pQ->pFirst;
			pLast  = pQ->pLast;
		}
		else {	// Insert at begining of our queue			
			pFirst->pPrev = pQ->pLast;
			pFirst = pQ->pFirst;
		}
		cItems += pQ->cItems;
		
		// Empty other queue
		pQ->pFirst = pQ->pLast = NULL;
		pQ->cItems = 0;
		
		if (nWait == 0)
			section.Leave();
		else {
			--nWait;
			section.Leave();
			semaphore.Signal();
		}
	}	
	return IsFull();
}

// .Unlink -- Unlink T from anywhere in queue ----------------------WaitQueue_T-
//
template <class T>
void WaitQueue_T<T>::Unlink(T *pLnk) {
	Critical section;
	if (pLnk->pPrev == NULL) // First
		UnlinkFirst();
	else {
		pLnk->pPrev->pNext = pLnk->pNext;
		if (pLnk->pNext)
			pLnk->pNext->pPrev=pLnk->pPrev;
		else // deletee is last in queue
			pLast = pLnk->pPrev;

		--cItems;
	}
}	


// .UnlinkFirst -- Unlink T from beginning of queue ----------------WaitQueue_T-
//
// Thread UNSAFE
//
template <class T>
T * WaitQueue_T<T>::UnlinkFirst(void) {		
	T *pT = pFirst;
		
	if (pT != NULL) {
		pFirst = pT->pNext;
		if (pFirst == NULL)
			pLast = NULL;
		else
			pFirst->pPrev = NULL;

		--cItems;
	}	
	
	return pT;	// NULL if empty queue
}

// .LinkFirst -- Link T to beginning of queue ----------------------WaitQueue_T-
//
// Thread UNSAFE
//
template <class T>
void WaitQueue_T<T>::LinkFirst(T *pT) {
	if (pT == NULL) {
		Tracef("WARNING Attempt to link NULL to WaitQueue @%x\n",this);
	}
	else {
		pT->pNext = pFirst;
		pT->pPrev = NULL;

		if (pFirst == NULL)
			pLast = pT;
		else
			pFirst->pPrev = pT;

		pFirst = pT;
	
		++cItems;
	}
}

// .LinkLast -- Link T to end of queue -----------------------------WaitQueue_T-
//
// Thread UNSAFE
//
template <class T>
void WaitQueue_T<T>::LinkLast(T *pT) {
	if (pT == NULL) {
		Tracef("WARNING Attempt to link NULL to WaitQueue @%x\n",this);
	}
	else {
		pT->pNext = NULL;
		pT->pPrev = pLast;

		if (pLast == NULL)
			pFirst = pT;
		else
			pLast->pNext = pT;

		pLast = pT;
		++cItems;
	}
}

#endif//win32
#endif 	// __WaitQueue_T_h
