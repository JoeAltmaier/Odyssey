//win_WaitQueue_T.h

#ifndef __win_WaitQueue_T_h
#define __win_WaitQueue_T_h

#include "Critical.h"
#include "Semaphore.h"
#include "stdio.h"

template <class T>
class WaitQueue_T {
	T *pFirst;
	T *pLast;

	CT_Semaphore m_Semaphore;

	U32 sQueueMax;
	U32 cItems;

	HANDLE m_Event;
	
public:
	
	WaitQueue_T<T>(U32 _sQueueMax) {
		m_Event = CreateEvent(NULL, TRUE, FALSE, "");
		Kernel::Create_Semaphore(&m_Semaphore, "safe", 1);
		sQueueMax = _sQueueMax; 
		pFirst = pLast = NULL; 
		cItems=0;
	}
	
	BOOL IsEmpty() {
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
		BOOL bRet = (pFirst == NULL);
		Kernel::Release_Semaphore(&m_Semaphore);
		return bRet;
	}

	BOOL IsFull()  {
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
		BOOL bRet = (cItems >= sQueueMax);
		Kernel::Release_Semaphore(&m_Semaphore);
		return bRet;
	}

	U32  ItemCount() {
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
		U32 cRet = cItems;
		Kernel::Release_Semaphore(&m_Semaphore);
		return cRet;
	}

	T *Peek(U32 nItem) { 
		T *p;
		int ii;

		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

		for (ii=0,p=pFirst; ii < (signed)nItem; ii++,p=p->pNext) {
			if (p == NULL)
				return NULL;
		}

		Kernel::Release_Semaphore(&m_Semaphore);

		return p;
	}

	T *Get() { 		// Get without blocking
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

	while ((pLnk = UnlinkFirst()) == NULL) {
		WaitForSingleObject(m_Event, INFINITE);
		ResetEvent(m_Event);
	}

	return pLnk;
}

template <class T>
BOOL WaitQueue_T<T>::Put(T *pT) {

	LinkLast(pT);
			
	SetEvent(m_Event);
		
	return IsFull();
}

// Insert -- Insert T at beginning of queue ----------------------WaitQueue_T-
// 
// Put T as beginning of Queue.  Resume Thread if one is blocked on Queue.
// Returns TRUE if queue is overflowing.

template <class T>
BOOL WaitQueue_T<T>::Insert(T *pT) {

	LinkFirst(pT);
	
	SetEvent(m_Event);

	return IsFull();
}

// Insert -- Put entire Queue at head on this queue --------------WaitQueue_T-
// 
// Put T as end of Queue.  Resume Thread if one is blocked on Queue.
// Returns TRUE if queue is overflowing.
//
// Thread SAFE... maybe too safe

template <class T>
BOOL WaitQueue_T<T>::Insert(WaitQueue_T<T> *pQ) {

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
	Kernel::Obtain_Semaphore(&(pQ->m_Semaphore), CT_SUSPEND);

	if (pQ->pLast != NULL) {
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

		Kernel::Release_Semaphore(&(pQ->m_Semaphore));
		Kernel::Release_Semaphore(&m_Semaphore);

		SetEvent(m_Event);
	}
	else {
		Kernel::Release_Semaphore(&(pQ->m_Semaphore));
		Kernel::Release_Semaphore(&m_Semaphore);
	}

	return IsFull();
}


// .Unlink -- Unlink T from anywhere in queue ----------------------WaitQueue_T-
//
// Thread UNSAFE
//
template <class T>
void WaitQueue_T<T>::Unlink(T *pLnk) {
	if (pLnk->pPrev == NULL) // First
		UnlinkFirst();
	else {
		pLnk->pPrev->pNext=pLnk->pNext;
		if (pLnk->pNext)
			pLnk->pNext->pPrev=pLnk->pPrev;
		else
			pLast = pLnk->pPrev;
		--cItems;
	}
}	


// .UnlinkFirst -- Unlink T from begining of queue -----------------WaitQueue_T-
//
// Thread SAFE

template <class T>
T * WaitQueue_T<T>::UnlinkFirst(void) {	

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	T *pT = pFirst;
		
	if (pT != NULL) {
		pFirst = pT->pNext;
		if (pFirst == NULL)
			pLast = NULL;
		else
			pFirst->pPrev = NULL;

		--cItems;
	}	

	Kernel::Release_Semaphore(&m_Semaphore);
	
	return pT;	// NULL if empty queue
}

// .LinkFirst -- Link T to begining of queue -----------------------WaitQueue_T-
//
// Thread SAFE
//
template <class T>
void WaitQueue_T<T>::LinkFirst(T *pT) {
	
	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	pT->pNext = pFirst;
	pT->pPrev = NULL;

	if (pFirst == NULL)
		pLast = pT;
	else
		pFirst->pPrev = pT;

	pFirst = pT;
	
	++cItems;

	Kernel::Release_Semaphore(&m_Semaphore);
}

// .LinkLast -- Llink T to end of queue ----------------------------WaitQueue_T-
//
// Thread UNSAFE
//
template <class T>
void WaitQueue_T<T>::LinkLast(T *pT) {

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	pT->pNext = NULL;
	pT->pPrev = pLast;

	if (pLast == NULL)
		pFirst = pT;
	else
		pLast->pNext = pT;

	pLast = pT;
	++cItems;

	Kernel::Release_Semaphore(&m_Semaphore);
}

#endif 	// __WaitQueue_T_h
