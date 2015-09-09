/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raidq.cpp
//
// Description:	Queueing class for Reqblks
//				Insert Reqblks in Q, remove from Q (FIFO, elevator)
//				and combine Reqblks
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#define MAX_SG_COMBINE	20

#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "Ddm.h"
#include "RaidStructs.h"
#include "Raidq.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "Member.h"
#include "RaidErr.h"
#include "RaidUtilTable.h"
#include "Raid.h"

/*************************************************************************/
// ReqQueue
// Constructor method for the class ReqQueue
// Pass QMethod when constructed (FIFO, Elevator)
/*************************************************************************/

ReqQueue::ReqQueue(Raid *pRaid, U8 QMethod)
{
	this->pRaid = pRaid;
	pHead = pTail = pCurrent = NULL;
	MaxCount = Count = 0;
	Direction = RAIDQ_FORWARD;
	QueueMethod = QMethod;
}


/*************************************************************************/
// EnQueue
// Insert Reqblk in queue
/*************************************************************************/

void	ReqQueue::EnQueue(Reqblk *pReq)
{
	if (pHead == NULL)
	{
		pReq->pForw = NULL;
		pReq->pBack = NULL;
		pHead = pTail = pCurrent = pReq;
	}
	else if (QueueMethod == RAIDQ_FIFO)
		EnQueueAtTail(pReq);
	else
	{	// insert sorted by LBA
		if (pReq->Lba < pCurrent->Lba)
			QueueBack(pReq);	// insert sorted between current and head
		else
			QueueForw(pReq);	// insert sorted between current and tail
	}
	Count++;					// for debug
	if (Count > MaxCount)
		MaxCount = Count;		// for debug
}

/*************************************************************************/
// EnQueueAtTail
// Insert Reqblk at Tail of Queue
/*************************************************************************/

void	ReqQueue::EnQueueAtTail(Reqblk *pReq)
{
	// Queue can't be empty
	pReq->pForw = NULL;
	pReq->pBack = pTail;
	pTail->pForw = pReq;
	pTail = pReq;
}

/*************************************************************************/
// EnQueueAtHead
// Insert Reqblk at Head of Queue
/*************************************************************************/

void	ReqQueue::EnQueueAtHead(Reqblk *pReq)
{
	// Queue can't be empty
	pReq->pForw = pHead;
	pReq->pBack = NULL;
	pHead->pBack = pReq;
	pHead = pReq;
}

/*************************************************************************/
// EnQueueAsCurrent
// Insert Reqblk in queue and make Current
/*************************************************************************/

void	ReqQueue::EnQueueAsCurrent(Reqblk *pReq)
{
	if (QueueMethod == RAIDQ_FIFO)
	{
		if (pHead == NULL)
		{
			pReq->pForw = NULL;
			pReq->pBack = NULL;
			pHead = pTail = pCurrent = pReq;
		}
		else
			EnQueueAtHead(pReq);
		Count++;
		if (Count > MaxCount)
			MaxCount = Count;
	}
	else
		EnQueue(pReq);
	pCurrent = pReq;
}

/*************************************************************************/
// IsInQueue
// Return whether pReq is in Queue
/*************************************************************************/

BOOL	ReqQueue::IsInQueue(Reqblk *pReq)
{
	Reqblk *pNext;

	pNext = pHead;
	while (pNext)
	{
		if (pNext == pReq)
			return (TRUE);
		pNext = pNext->pForw;
	}
	return (FALSE);
}

/*************************************************************************/
// QueueBack
// Search Queue from pCurrent backwards to head to insert Reqblk
/*************************************************************************/

void	ReqQueue::QueueBack(Reqblk *pReq)
{
	Reqblk *pCur, *pPrev;

	pCur = pCurrent;
	while (TRUE)
	{
		if (pCur->pBack == NULL)
		{
			// insert as head
			pReq->pBack = NULL;
			pReq->pForw = pCur;
			pCur->pBack = pReq;
			pHead = pReq;
			break;		// done
		}
		pPrev = pCur;
		pCur = pCur->pBack;
		if (pCur->Lba <= pReq->Lba)
		{
			// insert between pPrev and pCur
			pPrev->pBack = pReq;
			pReq->pBack = pCur;
			pReq->pForw = pPrev;
			pCur->pForw = pReq;
			break;		// done
		}
	}
}

/*************************************************************************/
// QueueForw
// Search Queue from pCurrent forwards to tail to insert Reqblk
/*************************************************************************/

void	ReqQueue::QueueForw(Reqblk *pReq)
{
	Reqblk *pCur, *pPrev;

	pCur = pCurrent;
	while (TRUE)
	{
		if (pCur->pForw == NULL)
		{
			// insert as tail
			pReq->pBack = pCur;
			pReq->pForw = NULL;
			pCur->pForw = pReq;
			pTail = pReq;
			break;		// done
		}
		pPrev = pCur;
		pCur = pCur->pForw;
		if (pCur->Lba > pReq->Lba)
		{
			// insert between pPrev and pCur
			pPrev->pForw = pReq;
			pReq->pBack = pPrev;
			pReq->pForw = pCur;
			pCur->pBack = pReq;
			break;		// done
		}
	}
}

/*************************************************************************/
// RemoveFromQueue
// Remove specified pReq from Queue - pReq must be on Queue
/*************************************************************************/

void	ReqQueue::RemoveFromQueue(Reqblk *pReq)
{
	Reqblk *pPrev, *pNext;

	pPrev = pReq->pBack;
	pNext = pReq->pForw;

	if (pPrev)
		pPrev->pForw = pNext;
	else
		pHead = pNext;

	if (pNext)
		pNext->pBack = pPrev;
	else
		pTail = pPrev;

	if (pReq == pCurrent)
	{	// removing pCurrent - need to set a new pCurrent
		if (Direction == RAIDQ_FORWARD)
		{
			if (pNext)
				pCurrent = pNext;
			else
				pCurrent = pPrev;
		}
		else
		{
			if (pPrev)
				pCurrent = pPrev;
			else
				pCurrent = pNext;
		}
	}
	if (pCurrent == NULL)
	{
		pHead = pTail = NULL;		// Queue is empty
	}
	pReq->pForw = pReq->pBack = NULL;
	Count--;
}

/*************************************************************************/
// DeQueue
// Remove Current Reqblk from queue and change direction if at end
/*************************************************************************/

Reqblk	*ReqQueue::DeQueue()
{
	Reqblk	*pReq;

	if (QueueMethod == RAIDQ_FIFO)
	{
		pReq = pHead;
		if (pReq)
		{
			pHead = pCurrent = pReq->pForw;
			if (pHead)
				pHead->pBack = NULL;
			else
				pTail = NULL;
			Count--;
			pReq->pForw = pReq->pBack = NULL;
		}
	}
	else
	{
		pReq = pCurrent;
		if (pReq)
			RemoveFromQueue(pReq);
		if (pCurrent)
		{	// Queue not empty
			if (Direction == RAIDQ_FORWARD)
				if (pTail == pCurrent)			// at end
					Direction = RAIDQ_BACK;		// change direction
			else
				if (pHead == pCurrent)			// at end
					Direction = RAIDQ_FORWARD;	// change direction
		}
	}
	return (pReq);
}

/*************************************************************************/
// SGCombine
// Enqueue pReq and combine with other requests, if possible
/*************************************************************************/

void	ReqQueue::SGCombine(Reqblk *pReq)
{

	// Put pReq in Queue
	EnQueue(pReq);

	// look behind pReq for combinations
	pReq = SGCombineBack(pReq);

	// look forward for combinations
	if (pReq)
		SGCombineForw(pReq);
}

/*************************************************************************/
// SGCombineBack
// Try to combine pReq with another request,
// searching backwards towards head
//
// Returns ptr to Master Req, if combined
// Returns NULL if out of resources to combine
// Returns pReq if not combined
/*************************************************************************/

Reqblk	*ReqQueue::SGCombineBack(Reqblk *pReq)
{
	Reqblk	*pNext;

	// look behind
	pNext = pReq->pBack;
	while (pNext)
	{
		if ((pNext->Type == pReq->Type) && (pNext->Member == pReq->Member))
		{
			if (pNext->Lba + pNext->Count == pReq->Lba)
			{
				// Requests are contiguous
				return (CombineContiguous(pNext, pReq));
			}
			else if (pNext->Lba + pNext->Count > pReq->Lba)
			{
				if (pReq->Type == RAID_WRITE)
				{
				 	// Additional check in case list is not sorted
				 	if (pNext->Lba <= pReq->Lba)
				 	{	// Overlapping writes
						return (CombineOverwrites(pNext, pReq));
				 	}
				}
			}
			else if (pNext->Lba == pReq->Lba)
			{
				if (pReq->Type == RAID_WRITE)
			 	{	// Overlapping writes
					return (CombineOverwrites(pNext, pReq));
				}
			}
			break;	// stop looking
		}
		pNext = pNext->pBack;
	}
	return (pReq);	// pReq not combined
}

/*************************************************************************/
// SGCombineForw
// Try to combine pReq with another request,
// searching forwards towards tail
//
// Returns ptr to Master Req, if combined
// Returns NULL if out of resources to combine
// Returns pReq if not combined
/*************************************************************************/

Reqblk	*ReqQueue::SGCombineForw(Reqblk *pReq)
{
	Reqblk	*pNext;

	// look in front
	pNext = pReq->pForw;
	while (pNext)
	{
		if ((pNext->Type == pReq->Type) && (pNext->Member == pReq->Member))
		{
			if (pReq->Lba + pReq->Count == pNext->Lba)
			{
				// Requests are contiguous
				return (CombineContiguous(pReq, pNext));
			}
			else if (pReq->Lba + pReq->Count > pNext->Lba)
			{
				if (pReq->Type == RAID_WRITE)
				{
				 	// Additional check in case list is not sorted
				 	if (pReq->Lba <= pNext->Lba)
				 	{	// Overlapping writes
						return (CombineOverwrites(pReq, pNext));
				 	}
				}
			}
			else if (pReq->Lba == pNext->Lba)
			{
				if (pReq->Type == RAID_WRITE)
			 	{	// Overlapping writes
					return (CombineOverwrites(pReq, pNext));
				}
			}
			break;	// stop looking
		}
		pNext = pNext->pForw;
	}
	return (pReq);	// pReq not combined
}

/*************************************************************************/
// StripeCombine
// Enqueue pReq and combine for stripe writes, if possible
/*************************************************************************/

void	ReqQueue::StripeCombine(Reqblk *pReq)
{

	// Put pReq in Queue
	EnQueue(pReq);

	// add code to SG combine and stripe combine
}

/*************************************************************************/
// CombineContiguous
// pReq1 must have the lower Lba
/*************************************************************************/

Reqblk	*ReqQueue::CombineContiguous(Reqblk *pReq1, Reqblk *pReq2)
{
	Reqblk	*pMaster;

	if (pReq1->NumSGLs + pReq2->NumSGLs > MAX_SG_COMBINE)
		return (NULL);
	if (pReq1->Flags & MASTER_BIT)	
	{
		if (pReq2->Flags & MASTER_BIT)
			// combine two Master reqs into one
			pMaster = CombineMasterSGReqs(pReq1, pReq2);
		else
			// add pReq2 to existing Master Reqblk
			pMaster = AddToMasterSGReq(pReq1, pReq2);
	}
	else if (pReq2->Flags & MASTER_BIT)
		// add pReq1 to existing Master Reqblk					
		pMaster = AddToMasterSGReq(pReq2, pReq1);
	else
		// create a Master Reblk for pReq1 and pReq2
		pMaster = CreateMasterSGReq(pReq1, pReq2);

	return (pMaster);
}

/*************************************************************************/
// CombineOverwrites
// pReq1 must have the lower or equal Lba
/*************************************************************************/

Reqblk	*ReqQueue::CombineOverwrites(Reqblk *pReq1, Reqblk *pReq2)
{
	Reqblk	*pMaster;

return (NULL);
	if (pReq1->Flags & MASTER_BIT)	
	{
		if (pReq2->Flags & MASTER_BIT)
			// combine two Master reqs into one
			pMaster = CombineMasterOverwrites(pReq1, pReq2);
		else
			// add pReq2 to existing Master Reqblk
			pMaster = AddToMasterOverwrites(pReq1, pReq2);
	}
	else if (pReq2->Flags & MASTER_BIT)
		// add pReq1 to existing Master Reqblk					
		pMaster = AddToMasterOverwrites(pReq2, pReq1);
	else
		// create a Master Reblk for pReq1 and pReq2
		pMaster = CreateMasterOverwrites(pReq1, pReq2);
	if (pMaster)
	{
		// Add to Stats
	}
	return (pMaster);
}


/*************************************************************************/
// AddToMasterSGReq
// Add pReq to pMaster
/*************************************************************************/

Reqblk	*ReqQueue::AddToMasterSGReq(Reqblk *pMaster, Reqblk *pReq)
{
	SGLIST	*pSGList1, *pSGList2, *pNewSGList;
	U8		i, NewNumSGLs;

	NewNumSGLs = pMaster->NumSGLs + pReq->NumSGLs;
	// Get new SGList for Master
	pNewSGList = new SGLIST[NewNumSGLs];
	if (pNewSGList == NULL)
		return NULL;

	pMaster->Count += pReq->Count;

	// take pReq off Reqblk chain
	RemoveFromQueue(pReq);

	if (pMaster->Lba < pReq->Lba)
	{
		// Put pReq at tail of combine chain
		pMaster->pCombForw->pForw = pReq;
		pReq->pForw = NULL;
		pReq->pBack = pMaster->pCombForw;	// old tail
		pMaster->pCombForw = pReq;			// new tail

		// Copy Master SGList to new SGList
		pSGList1 = pNewSGList;
		pSGList2 = pMaster->pSGList;
		for (i = 0; i < pMaster->NumSGLs; i++)
		{
			pSGList1->Address = pSGList2->Address;
			pSGList1->Length = pSGList2->Length;
			pSGList1++;
			pSGList2++;
		}
		// Append pReq SGList to new Master SGList
		pSGList2 = pReq->pSGList;
		for (i = pMaster->NumSGLs; i < NewNumSGLs; i++)
		{
			pSGList1->Address = pSGList2->Address;
			pSGList1->Length = pSGList2->Length;
			pSGList1++;
			pSGList2++;
		}
	}
	else
	{
		// Put pReq at head of combine chain
		pMaster->pCombBack->pBack = pReq;
		pReq->pBack = NULL;
		pReq->pForw = pMaster->pCombBack;	// old head
		pMaster->pCombBack = pReq;			// new head

		pMaster->Lba = pReq->Lba;	// starting Lba for request
		// Copy pReq SGList to new SGList
		pSGList1 = pNewSGList;
		pSGList2 = pReq->pSGList;
		for (i = 0; i < pReq->NumSGLs; i++)
		{
			pSGList1->Address = pSGList2->Address;
			pSGList1->Length = pSGList2->Length;
			pSGList1++;
			pSGList2++;
		}
		// Append old Master SGList to new Master SGList
		pSGList2 = pMaster->pSGList;
		for (i = pReq->NumSGLs; i < NewNumSGLs; i++)
		{
			pSGList1->Address = pSGList2->Address;
			pSGList1->Length = pSGList2->Length;
			pSGList1++;
			pSGList2++;
		}
	}

	// delete Master's old SGList
	delete []pMaster->pSGList;

	// Set new SGList in Master
	pMaster->NumSGLs = NewNumSGLs;
	pMaster->pSGList = pNewSGList;

	pRaid->UpdateSGCombined(pMaster->Type, 1);

	return (pMaster);
}


/*************************************************************************/
// CreateMasterSGReq
// Create a new Master Req and add pReq1 and pReq2
// pReq1 must have the lower Lba
/*************************************************************************/

Reqblk	*ReqQueue::CreateMasterSGReq(Reqblk *pReq1, Reqblk *pReq2)
{
	Reqblk	*pMaster;
	SGLIST	*pSGList, *pTempSGList, *pNewSGList;
	U8		i, NewNumSGLs;

	pMaster = new Reqblk;		// Get new Reqblk for Master
	if (pMaster == NULL)
		return NULL;

	*pMaster = *pReq1;			// Copy Req1 to Master
	pMaster->pIoreq = NULL;		// Master Reqblks don't have an Ioreq

	NewNumSGLs = pReq1->NumSGLs + pReq2->NumSGLs;
	pNewSGList = new SGLIST[NewNumSGLs];	// Get new SGList for Master
	if (pNewSGList == NULL)
	{
		delete pMaster;
		return NULL;
	}

	pSGList = pReq1->pSGList;
	pTempSGList = pNewSGList;
	// Copy Req1 SGList to Master SGList
	for (i = 0; i < pReq1->NumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		pTempSGList++;
		pSGList++;
	}
	pSGList = pReq2->pSGList;
	// Append Req2 SGList to Master SGList
	for (i = pReq1->NumSGLs; i < NewNumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		pTempSGList++;
		pSGList++;
	}

	pMaster->Count += pReq2->Count;	// pReq1->Count + pReq2->Count

	// Set new SGList in Master
	pMaster->NumSGLs = NewNumSGLs;
	pMaster->pSGList = pNewSGList;

	// Mark as Master Reqblk
	pMaster->Flags |= MASTER_BIT;

	// take pReq1 off Reqblk chain
	RemoveFromQueue(pReq1);

	// take pReq2 off Reqblk chain
	RemoveFromQueue(pReq2);

	// put Master on Reqblk chain
	EnQueue(pMaster);

	pMaster->pCombBack = pReq1;		// Head of Combine chain for Master
	pMaster->pCombForw = pReq2;		// Tail of Combine chain for Master

	// link reqs together
	pReq1->pBack = NULL;
	pReq1->pForw = pReq2;

	pReq2->pBack = pReq1;
	pReq2->pForw = NULL;

	pRaid->UpdateSGCombined(pMaster->Type, 2);

	return (pMaster);
}

/*************************************************************************/
// CombineMasterSGReqs
// Add pReq2's Requests to Master pReq1, and delete Master pReq2 
// pReq1 must have the lower Lba
/*************************************************************************/

Reqblk	*ReqQueue::CombineMasterSGReqs(Reqblk *pReq1, Reqblk *pReq2)
{
	SGLIST	*pSGList, *pTempSGList, *pNewSGList;
	U8		i, NewNumSGLs;

	NewNumSGLs = pReq1->NumSGLs + pReq2->NumSGLs;
	pNewSGList = new SGLIST[NewNumSGLs];	// Get new SGList for Master
	if (pNewSGList == NULL)
		return NULL;

	pSGList = pReq1->pSGList;
	pTempSGList = pNewSGList;
	// Copy Req1 old SGList to Req1 new SGList
	for (i = 0; i < pReq1->NumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		pTempSGList++;
		pSGList++;
	}
	pSGList = pReq2->pSGList;
	// Append Req2 SGList to Req1 new SGList
	for (i = pReq1->NumSGLs; i < NewNumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		pTempSGList++;
		pSGList++;
	}

	pReq1->Count += pReq2->Count;

	// delete Req1's old SGList
	delete []pReq1->pSGList;

	// Set new SGList in Req1
	pReq1->NumSGLs = NewNumSGLs;
	pReq1->pSGList = pNewSGList;

	// take pReq2 off Reqblk chain
	RemoveFromQueue(pReq2);

	// add Master pReq2's chain of Reqblks
	// to Master pReq1's chain
	pReq1->pCombForw->pForw = pReq2->pCombBack;		// tail of Req1 points forw to head of Req2
	pReq2->pCombBack->pBack = pReq1->pCombForw;		// head of Req2 points back to tail of Req1

	pReq1->pCombForw = pReq2->pCombForw;			// new tail

	// delete Req2's SGList
	delete []pReq2->pSGList;

	// now can delete Master pReq2
	delete pReq2;

	pRaid->UpdateSGCombined(pReq1->Type, 1);

	return (pReq1);
}

Reqblk	*ReqQueue::CombineMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2)
{
}

Reqblk	*ReqQueue::AddToMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2)
{
}

Reqblk	*ReqQueue::CreateMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2)
{
	Reqblk	*pMaster;
	SGLIST	*pSGList, *pTempSGList, *pTemp2SGList, *pNewSGList;
	U32		RemCount, End1, End2, Offset, TotalBytes, ByteOffset;
	U8		i, NewNumSGLs, SGIndex;

	pMaster = new Reqblk;		// Get new Reqblk for Master
	if (pMaster == NULL)
		return NULL;

	*pMaster = *pReq1;			// Copy Req1 to Master
	pMaster->pIoreq = NULL;		// Master Reqblks don't have an Ioreq

	// pReq1 has the lower or equal Lba than pReq2
	// Take all S/G elements of pReq1, then as many
	// as needed (if any) from pReq2

	End1 = pReq1->Lba + pReq1->Count;
	End2 = pReq2->Lba + pReq2->Count;

	if (End2 > End1)
	{
		RemCount = End2 - End1;
		if (End1 > pReq2->Lba)
			Offset = End1 - pReq2->Lba;		// Requests overlap
		else
			Offset = 0;						// Requests are contiguous
	}
	else
		RemCount = 0;	// pReq2's range is fully contained in pReq1's range

	TotalBytes = Offset * 512;	// Byte offset into pReq2's SGList to start
	pTemp2SGList = pReq2->pSGList;
	ByteOffset = 0;
	SGIndex = 0;
	while (TotalBytes)
	{
		if (pTemp2SGList->Length > TotalBytes)
		{
			ByteOffset = pTemp2SGList->Length - TotalBytes;
			TotalBytes = 0;
		}
		else
		{
			TotalBytes -= pTemp2SGList->Length;
			SGIndex++;
			pTemp2SGList++;
		}
	}

	pMaster->Count += RemCount;	// pReq1->Count + as much as needed from pReq2

	NewNumSGLs = pReq2->NumSGLs - SGIndex;
	NewNumSGLs += pReq1->NumSGLs;
	pNewSGList = new SGLIST[NewNumSGLs];	// Get new SGList for Master
	if (pNewSGList == NULL)
	{
		delete pMaster;
		return NULL;
	}

	pSGList = pReq1->pSGList;
	pTempSGList = pNewSGList;
	// Copy Req1 SGList to Master SGList
	for (i = 0; i < pReq1->NumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		pTempSGList++;
		pSGList++;
	}
	pSGList = pTemp2SGList;
	// Append as much as needed of Req2 SGList to Master SGList
	for (i = pReq1->NumSGLs; i < NewNumSGLs; i++)
	{
		pTempSGList->Address = pSGList->Address;
		pTempSGList->Length = pSGList->Length;
		if (i == pReq1->NumSGLs)
		{
			pTempSGList->Address += ByteOffset;
			pTempSGList->Length -= ByteOffset;
		}
		pTempSGList++;
		pSGList++;
	}

	// Set new SGList in Master
	pMaster->NumSGLs = NewNumSGLs;
	pMaster->pSGList = pNewSGList;

	// Mark as Master Reqblk
	pMaster->Flags |= MASTER_BIT;

	// take pReq1 off Reqblk chain
	RemoveFromQueue(pReq1);

	// take pReq2 off Reqblk chain
	RemoveFromQueue(pReq2);

	// put Master on Reqblk chain
	EnQueue(pMaster);

	pMaster->pCombBack = pReq1;		// Head of Combine chain for Master
	pMaster->pCombForw = pReq2;		// Tail of Combine chain for Master

	// link reqs together
	pReq1->pBack = NULL;
	pReq1->pForw = pReq2;

	pReq2->pBack = pReq1;
	pReq2->pForw = NULL;

	return (pMaster);
}
