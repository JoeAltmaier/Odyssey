/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: IoreqQueue.cpp
//
// Description:
//			Keeps a Queue of outstanding Ioreqs for debug and
//			to be able to tell when Quiesced
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "Ddm.h"
#include "RaidStructs.h"
#include "RaidIorQ.h"

/*************************************************************************/
// IoreqQueue
// Constructor method for the class IoreqQueue
/*************************************************************************/

IoreqQueue::IoreqQueue()
{
	pHead = pTail = NULL;
	Count = 0;
}

/*************************************************************************/
// InsertInQueue
// Insert Ioreq at Tail of Queue
/*************************************************************************/

void	IoreqQueue::InsertInQueue(Ioreq *pIoreq)
{
	pIoreq->pForw = NULL;
	pIoreq->pBack = pTail;
	if (pTail)
		pTail->pForw = pIoreq;
	else
		pHead = pIoreq;
	pTail = pIoreq;
	Count++;
}

/*************************************************************************/
// RemoveFromQueue
// Remove Ioreq from Queue
/*************************************************************************/

void	IoreqQueue::RemoveFromQueue(Ioreq *pIoreq)
{
	Ioreq *pPrev, *pNext;

	pPrev = pIoreq->pBack;
	pNext = pIoreq->pForw;

	if (pPrev)
		pPrev->pForw = pNext;
	else
		pHead = pNext;

	if (pNext)
		pNext->pBack = pPrev;
	else
		pTail = pPrev;

	pIoreq->pForw = pIoreq->pBack = NULL;
	Count--;
}

