/* HeapBlock.cpp -- Block Heap Manager
 *
 * Copyright ConvergeNet Technologies (c) 1998 
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
 *		Allocates memory in multiples of a fixed block size.
 *
 * Revision History:
 *     10/02/98 Tom Nelson: Created
 *
 *
 * Conventions:
 *		"C"  - Begins Class names
 *		"_"  - Begins private member names
 *		
 *
**/


#include "OsTypes.h"
#include "OsStatus.h"
#include "Critical.h"
#include "Hw.h"
#include "Address.h"
#include "HeapBlock.h"


#ifdef _HEAPCHECK
extern "C" void Free(U32 cb, U32 ra);
#define FREE(cb,ip) ::Free(cb,ip)
#else
#define FREE(cb, ip)	
#endif


// Initialize -- Initialize Block Heap Manager ---------------------CHeapBlock-
//
// Free Block in heap:
//		{<sizePrev/sizeNext/signPrev/signNext><<FreeLinks> data >}
//
// Allocated Block in heap:
//		{<sizePrev/sizeNext/signPrev/signNext>< data >}
//
// The Heap is as follows:
//		<sizePrev=0/sizeOf1st=n/SIGNEND/signOf1st> <data block 1> ...
//		... <data block N> <0/sizeOfN/signOfN/SIGNEND>
//
// All <data> is allocated to fall on the ALIGNSIZE byte boundry.
//
// pFree -> {1st free block}
// pAlloc-> {1st allocated block}
//
// NOTE:  Block sizes must ALWAYS be valid except in critical regions
//
ERC CHeapBlock::Initialize(U32 nFragment, U32 aP[], U32 aCb[], U16 sUnit)
{
	sUnit_ = sUnit + (ALIGNSIZE - 1) & ~ALIGNMASK;
	sSplit_= sUnit + sizeof(BLKHEAD);

	// This code assumes the heap is at least large enough for three headers

	// Make block marking beginning of heap
	pHeap_ = (BLKHEAD *)( (aP[0] + ALIGNMASK) & ~ALIGNMASK );
	pHeap_->sPrev = 0;			// End Block Marker
	pHeap_->sThis = 0;
	pHeap_->tThis = TYPEEND;
	pHeap_->pThis = this;

	// Remove 1st header from fragment for loop
	aCb[0] = aCb[0] - ((int)(pHeap_+1) - (int)aP[0]);
	aP[0] = (U32)(pHeap_+1);

	pFree_ = (BLKHEAD*)aP[0];	// 1st free block

	// Add fragments to the free list
	U32 sPrev = 0;			// First Free Block
	BLKHEAD *pPrev = NULL;
	for (int iFragment=0; iFragment < nFragment; iFragment++) {
		// Enter fragment as a free block
		BLKHEAD *pFrag = (BLKHEAD *)( (aP[iFragment] + ALIGNMASK) & ~ALIGNMASK );
		// Reduce size of fragment by amount pFrag was rounded up
		U32 sThis = aCb[iFragment] - ((int)pFrag - (int)aP[iFragment]);

		pFrag->sPrev = sPrev;
		pFrag->sThis = sThis - sizeof(BLKHEAD)*2;
		pFrag->tThis = TYPEFREE;
		pFrag->pThis = this;

		// Link this free block on the free list
		GetLink(pFrag)->pNextFree = NULL;
		GetLink(pFrag)->pPrevFree = pPrev;
		if (pPrev)
			GetLink(pPrev)->pNextFree = pFrag;

		// Enter gap between fragments as 'inuse' block, except last one which is end block
		GetNext(pFrag)->sPrev = pFrag->sThis;
		if (iFragment+1 < nFragment) {
			BLKHEAD *pNext = (BLKHEAD *)( (aP[iFragment+1] + ALIGNMASK) & ~ALIGNMASK );
			GetNext(pFrag)->sThis = (int)pNext - (int)(GetNext(pFrag)+1);
			GetNext(pFrag)->tThis = TYPEALLOC;
		}
		else {
			GetNext(pFrag)->sThis = 0; // End Block Marker
			GetNext(pFrag)->tThis = TYPEEND;
		}
		GetNext(pFrag)->pThis = this;

		pPrev = pFrag;
		sPrev = pFrag->sThis;
	}

	return OK;
}

// _Link -- Link block to free list --------------------------------CHeapBlock-
//
void CHeapBlock::_Link(BLKHEAD *pBlk)
{
	Critical section;

	GetLink(pBlk)->pPrevFree = NULL;
	GetLink(pBlk)->pNextFree = pFree_;

	if (pFree_ != NULL)
		GetLink(pFree_)->pPrevFree = pBlk;
	
	pFree_ = pBlk;
}


// _Unlink -- Unlink free block ------------------------------------CHeapBlock-
//
void CHeapBlock::_Unlink(BLKHEAD *pBlk)
{
	BLKLINK *pLink;

	pLink = GetLink(pBlk);

	Critical section();

	if (pLink->pPrevFree != NULL)
		GetLink(pLink->pPrevFree)->pNextFree = pLink->pNextFree;
	else
		pFree_ = pLink->pNextFree;

	if (pLink->pNextFree != NULL)
		GetLink(pLink->pNextFree)->pPrevFree = pLink->pPrevFree;
}

// _SplitBlk -- Split free block -----------------------------------CHeapBlock-
//
// Splits specified block and links second half to the beginning
// of the free list.  Specified block links are not effected.
//
void CHeapBlock::_SplitBlk(BLKHEAD *pBlk,U32 nBytes)
{
	BLKHEAD *pNew;
	U32 sNew;

	// Resize current
	Critical section;

	sNew = pBlk->sThis - nBytes - sizeof(BLKHEAD);
	pBlk->sThis = nBytes;

	// Initialize new block size
	pNew = GetNext(pBlk);
	pNew->sPrev = pBlk->sThis;

	pNew->sThis = sNew;
	pNew->tThis = TYPEFREE;
	pNew->pThis = this;
	GetNext(pNew)->sPrev = pNew->sThis;

	section.Leave();

	_Link(pNew);
}

// _LockFirst
// 
// Mark first free block as locked and return -> Block.  If block is 
// corrupted then return NULL.
//
BLKHEAD * CHeapBlock::_LockFirst()
{
//	Critical section;

	if (pFree_->tThis != TYPEFREE) {
		return NULL;	//*** CORRUPTED HEAP ***
	}
//	pFree_->tThis = TYPELOCK;

	return pFree_;
}


// _LockNext
// 
// Unlock block then mark next block and return -> next block.  If next
// block is corrupted then returns NULL.  If next block is END then unlock
// this block and return NULL.
BLKHEAD * CHeapBlock::_LockNext(BLKHEAD *pBlk)
{
//	Critical section;

	// Skip locked free blocks

	for(pBlk = GetLink(pBlk)->pNextFree ; pBlk != NULL; pBlk = GetLink(pBlk)->pNextFree) {
		if (pBlk->tThis == TYPEFREE) {
//			pBlk->tThis = TYPELOCK;
			break;
		}
//		if (pBlk->tThis != TYPELOCK) {
		else {
			pBlk = NULL;	//*** CORRUPTED HEAP ***
			break;
		}
	}
	
	return pBlk;
}


// Alloc -- Allocate Memory ----------------------------------------CHeapBlock-
//
// Locate acceptable free block in free pool.  Split block if larger than
// required and link second half of block on free list.
//
// NOTE: Must be re-entrant
//
void *CHeapBlock::Alloc(U32 &nBytes, U32 raAlloc)
{
	if (nBytes < sUnit_)
		nBytes = sUnit_;

	nBytes = (nBytes + (ALIGNSIZE-1)) & ~ALIGNMASK;

	Critical section;

	for (BLKHEAD *pBlk = _LockFirst(); pBlk != NULL; pBlk = _LockNext(pBlk)) {
		if (nBytes <= pBlk->sThis) {		// Found 1st fit
			_Unlink(pBlk);

			pBlk->tThis = TYPEALLOC;

			if (nBytes + sSplit_ <= pBlk->sThis)	// Split free block
				_SplitBlk(pBlk,nBytes);

#ifdef _HEAPCHECK
			pBlk->raAlloc = raAlloc;
#endif

			return pBlk+1;	// return &<data>
		}
	}
Tracef("CHeapBlock::Alloc(%u) returning NULL!\n", nBytes);
	return NULL;
}



// Free -- Free Memory ---------------------------------------------CHeapBlock-
//
// When a block is freed it is merged with previous and next block
// if they are free.  If merged with next block then next block is
// removed from list.  If not merged with either block then this 
// block is added to free list.
//
// NULL pointers and invalid block pointers are ignored.
//
// NOTE: Must be re-entrant
//
U32 CHeapBlock::Free(void *pData)
{
	BLKHEAD *pBlk,*pPrevBlk,*pNextBlk;
	U32 cbRet=0;
	
	if (pData != NULL) {
		pBlk = GetBlk(pData);

		// Check if allocated block
		if (pBlk->tThis == TYPEALLOC) {
			Critical section;

			cbRet=pBlk->sThis;

			FREE(cbRet, pBlk->raAlloc);

			// Merge with next block if it is free
			pNextBlk = GetNext(pBlk);
			if (pNextBlk->tThis == TYPEFREE) {
				_Unlink(pNextBlk);
				pBlk->sThis += pNextBlk->sThis + sizeof(BLKHEAD);
				GetNext(pBlk)->sPrev = pBlk->sThis;
			}

			// Merge with previous block if it is free
			pPrevBlk = GetPrev(pBlk);
			if (pPrevBlk->tThis != TYPEFREE) {
				pBlk->tThis = TYPEFREE;
				_Link(pBlk);
			}
			else {
				pPrevBlk->sThis += pBlk->sThis + sizeof(BLKHEAD);
				GetNext(pPrevBlk)->sPrev = pPrevBlk->sThis;
			}
		}
	}

	return cbRet;
}


// Check -- Check Heap for Continuity ------------------------------CHeapBlock-
//
// Returns:
//		HEAPTRASHEDerc
//		OK
//
// NOTE: Must be re-entrant
//
ERC CHeapBlock::Check()
{
	BLKHEAD *pBlk;

	// Validate block headers

	if (pHeap_->pThis != this || pHeap_->tThis != TYPEEND)
		return HEAPTRASHEDerc;

	for (pBlk = GetNext(pHeap_); pBlk->tThis != TYPEEND; pBlk = GetNext(pBlk))
		if (pBlk->pThis != this || !_IsValidType(pBlk->tThis))
			return HEAPTRASHEDerc;
	
	// Validate free list
	for (pBlk = pFree_; pBlk != NULL; pBlk = GetLink(pBlk)->pNextFree)
		if (pBlk->pThis != this || !_IsValidType(pBlk->tThis))
			return HEAPTRASHEDerc;

	return OK;	
}


// Dump -- Dump Heap --------------------------------------------CHeapBlock-
//
// NOTE: Must be re-entrant
//
void CHeapBlock::Dump(U32 cDumpMax)
{
	BLKHEAD *pBlk;

	Check();

	int cDump=0;
	for (pBlk = GetNext(pHeap_); pBlk->tThis != TYPEEND; pBlk = GetNext(pBlk)) {
		char *pType=(pBlk->tThis == TYPEALLOC? "ALLOC" :
					pBlk->tThis == TYPEFREE? "FREE" :
//					pBlk->tThis == TYPELOCK? "LOCK" :
					pBlk->tThis == TYPEEND? "END" :
					(char*)&pBlk->tThis);
		Tracef("%08lx %s[%d] %08lx\n", pBlk, pType, pBlk->sThis, pBlk->pThis);
		if (cDump++ > cDumpMax) {
			Tracef("dump limit exceeded.\n");
			break;
			}
		}
}

