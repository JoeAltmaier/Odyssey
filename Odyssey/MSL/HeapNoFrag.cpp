/* HeapNoFrag.cpp -- Non-fragmenting heap manager.
 *
 * Copyright (c) ConvergeNet Technologies (c) 1998 
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
 * 		This class manages a memory pool as free lists of blocks of discrete sizes.
 * 		Alloc returns the smallest block size that will fit.
 * 		Freeing a block returns it to the free list.
 * 		The partition is used to create more blocks when none are available
 * 		on some free list.
 * 
 * Revision History:
 *		6/13/98 Joe Altmaier: Create file
 *	   10/05/98 Tom Nelson:   Added block headers
 *	   11/11/98 Joe Altmaier: use sizeof(Header)
 *	   9/01/99 Joe Altmaier: allocate on cache line boundary, invalidate cached header
 *
**/
#define _TRACEF
#define	TRACE_INDEX		TRACE_HEAP1
#include "Odyssey_Trace.h"

#include "OsTypes.h"
#include "Critical.h"
#define TYPES_H	// don't include Types.h
#define I32 signed long
#include "Mips_util.h"
#include "HeapNoFrag.h"

extern void Dump(char *pTitle, U32 *pDump, int nDump);
	
	U32 raLast;

#ifdef _HEAPCHECK
extern "C" void Free(U32 cb, U32 ra);
#define FREE(cb,ip) ::Free(cb,ip)
#else
#define FREE(cb, ip)	
#endif


	HeapNoFrag::HeapNoFrag() {
#ifdef HEAP_LEAK_CHECK
		cbAllocReportPrev = 0;
#endif
	}


// Initialize -- Initialize the Non-Fragmenting Heap ----------------HeapNoFrag-
//
// Use memory at pMemory size cb.  Construct heap buckets for blocks of
// different sizes, largest cbMax.
//
	void HeapNoFrag::Initialize(int nBuckets, void *pMemory, int cb, int cbMax) {
		this->nBuckets=nBuckets;
		pFree=(char *)( ((U32)pMemory + 0x1F) & 0xFFFFFFE0 );
		cbFree=cb - (pFree - (char*)pMemory);
		cbAllocTotal=0;
#ifdef _HEAPCHECK
		pStart=pFree;
#endif

		this->cbMax=cbMax;
		
		pBuckets=(Header **)AllocFree(sizeof(Header*) * nBuckets);
		pSize=(int*)AllocFree(sizeof(int) * nBuckets);

		int i;

		for (i=nBuckets-1; i >= 0; i--) {
			pBuckets[i]=NULL;
			pSize[i]=cbMax;		// Remember size of bucket[i]
			if (cbMax)
				cbMin=cbMax;	// smallest bucket size used
			cbMax /= 2;
			if (cbMax < sizeof(Header))
				cbMax=0;
			}

		// Allocate bucket lookup table
		int _iMapMax=(this->cbMax / this->cbMin);
		pBucket_Cb=(int*)AllocFree(sizeof(int) * _iMapMax);
		
		// Never use bucket 0 - can't tell 0 from NULL in union (pNext, iBucket).
		int iBucket=1;

		// Map is indexed by cb / cbMin
		// Loop through all map entries (all cb's)
		for (i=0; i < _iMapMax; i++) {
			// Find bucket index that maps this cb
			while (pSize[iBucket] <= (i+1) * cbMin - 1)
				iBucket++;
			// Set map to index that bucket
			pBucket_Cb[i] = iBucket;
			}
		}

// Alloc -- Alloc block from the Non-Fragmenting Heap --------------HeapNoFrag-
//
// Attempts to get block from a bucket if available otherwise gets it
// from the never used free pile
//
	void *HeapNoFrag::Alloc(U32 &cb, U32 ra) {
		Header *pHdr;

		if (cb > cbMax)
			return NULL; // No bucket big enough;

		// Use bucket map to find bucket just big enough.
		int i=pBucket_Cb[(cb? (cb-1) :0) / cbMin];
		Critical section;
				
		if (pBuckets[i] != NULL) {
			pHdr=pBuckets[i];
			pBuckets[i]=pHdr->pNext;
			}
					
		// Out of blocks in that bucket.  Make another one.
		else {
			pHdr=(Header*)AllocFree(pSize[i] + sizeof(Header));
			// Invalidate cached header before we modify it using uncached ptr
			mips_sync_cache(pHdr, 32, SYNC_CACHE_FOR_CPU);
			pHdr->pHeap = this;
#ifdef _HEAPCHECK
			((U32*)&pHdr->guard[0])[0]=NEWNODE;
#endif
			}					

		cbAllocTotal += pSize[i];

#ifdef _HEAPCHECK
		// Bind a NodeDesc with each allocated heap node
		U32 *pGuard=(U32*)&pHdr->guard[0];
		
		if (pGuard[0] == NEWNODE) {
			// Make a new NodeDesc.
			NodeDesc *pNd=NewNode();
			pGuard[0]=GUARD;
			pGuard[1]=(U32)pNd;
			pNd->SetRa(ra);
			pNd->SetI(i);
			pNd->SetState(NodeDesc::INUSE);
			pNd->SetP(pHdr);
		} else {
			NodeDesc *pNd=(NodeDesc*)pGuard[1];
			pNd->SetRa(ra);
			pNd->SetState(NodeDesc::INUSE);
		}
#endif

		section.Leave();

		pHdr->iBucket=i;
		cb = pSize[i];

		return (char*)(++pHdr);
		}


// Free -- Return block to it's free bucket -----------------------HeapNoFrag-
//
	U32 HeapNoFrag::Free(void *pBlock) {
		Header *pHdr=((Header*)pBlock)-1;

		unsigned iHeader=pHdr->iBucket;
		if (iHeader && iHeader <= (unsigned)nBuckets) {
			Critical section;

			// Invalidate cached header before we modify it using uncached ptr
			mips_sync_cache(pHdr, 32, SYNC_CACHE_FOR_CPU);
// shouldn't be necessary if nobody writes over header using cached ptr
			U32 cb=pSize[iHeader];

#ifdef _HEAPCHECK
			NodeDesc *pNd=((NodeDesc**)&pHdr->guard[0])[1];
			FREE(cb, pNd->raAlloc);
			pNd->SetState(NodeDesc::FREE);

			if (TraceLevel[TRACE_HEAP1] > TRACE_L4)
				for (int id=0; id < (cb >> 2); id++)
					((U32*)pBlock)[id]=0xdeaddead;
#endif
			pHdr->pNext=pBuckets[iHeader];
			pBuckets[iHeader]=pHdr;
			cbAllocTotal -= cb;
			return cb;
			}

		return 0;
		}

// AllocFree -- Alloc block from the never used free pile --------HeapNoFrag-
//
	void *HeapNoFrag::AllocFree(int cb) {
		cb = (cb + 0x1F) & ~0x1F; // always align next allocation on 32-byte boundary - cache line size
		
		if (cbFree < cb) {
			TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::AllocFree Out of memory\n"));
			return NULL;
			}

		char *pRet=pFree;

		pFree += cb;
		cbFree -= cb;
				
		return pRet;
		}



#ifdef _HEAPCHECK
	// Prepare to track heap nodes
	void HeapNoFrag::InitDebug(void *pMemory, int cb) {
		pNd=(NodeDesc*)pMemory;
		nNd=0;
		nNdMax=cb / sizeof(NodeDesc);
	}
	
	// Allocate a node descriptor for a new node
	HeapNoFrag::NodeDesc *HeapNoFrag::NewNode() {
		return &pNd[nNd++];
	}

	//
	// Check heap nodes against heap descriptors
	//
	
	void HeapNoFrag::CheckHeap(U32 raRet) {
	BOOL fFatal=false;
	
	// Verify node descriptor checksums

	int iStart=-1;
	for (int iNd=0; iNd < nNd; iNd++) {
		switch (pNd[iNd].ChecksOut()) {
		case true:
			if (iStart != -1) {
				TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap Node descriptors trashed from %lx to %lx\n", &pNd[iStart], &pNd[iNd]));
				iStart=-1;
				}
			break;
			
		case false:
			fFatal=true;
			if (iStart == -1)
				iStart=iNd;
			break;
		}
	}

	if (iStart != -1)
		TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap Node descriptors trashed from %lx to %lx\n", &pNd[iStart], &pNd[nNd]));
		
	if (fFatal)
		Break(raLast, raRet);

				
	// Check node descriptors match node contents
	// Set node states
	int cbAlloc=nBuckets * 8;
	int cbUsed=0;

	{Critical section;

	for (int iNd=0; iNd < nNd; iNd++) {
		NodeDesc &nd=pNd[iNd];
		cbAlloc += pSize[nd.iBucket] + sizeof(Header);
		
		Header *pHdr=nd.pNode;
		U32 *pGuard=(U32*)&pHdr->guard[0];
		
		if (PTR_BAD(pHdr)) {
			TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap Node descriptor trashed\n"));
			BadNode(iNd);
			continue;
		}			

		if (pHdr->iBucket != nd.iBucket) {
			if (PTR_BAD(pHdr->pNext)) {
				TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap block link broken\n"));
				BadNode(iNd);
				continue;
			}
		}
		else {// Heap header indicates block in use
			cbUsed += pSize[nd.iBucket];
			if (nd.state != NodeDesc::INUSE) {
				TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap allocated block not marked INUSE\n"));
				BadNode(iNd);
				continue;
			}
		}

		if (pGuard[0] != GUARD) {
			TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap block guard bytes overwritten\n"));
			BadNode(iNd);
		}
		else if ((NodeDesc*)pGuard[1] != &nd) {
			TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap block guard bytes overwritten\n"));
			BadNode(iNd);
		}
		else if (pHdr->pHeap != this) {
			TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap block heap link broken\n"));
			BadNode(iNd);
		}
	}


	// Test heap metrics against calculated heap use
	if (cbUsed != cbAllocTotal)
		TRACEF(TRACE_L8, ("[WARNING] HeapNoFrag::CheckHeap memory use totals don't agree:\n total         %08lx\n actually used %08lx delta 0x%08lx \n", cbAllocTotal, cbUsed, cbUsed-cbAllocTotal));
	
	if (cbAlloc != (pFree - pStart))
		TRACEF(TRACE_L8, ("[WARNING] HeapNoFrag::CheckHeap memory allocation totals don't agree:\n total              %08lx\n actually allocated %08lx delta 0x%08lx\n", cbAlloc, (pFree - pStart), (pFree-pStart)-cbAlloc));

	}// end critical section


	// Check bucket linkage
	for (int iB = 0; iB < nBuckets; iB++) {
		// scan bucket, check nodes are free
		Critical section;

		Header *pHdrLast=NULL;
		Header *pHdr=pBuckets[iB];
		while (pHdr != NULL) {
			if (PTR_BAD(pHdr)) {
				TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap free list chain broken linking from %08lx -> %08lx\n", pHdrLast, pHdr));
				if (pHdrLast)
					BadNode(pHdrLast);
				else
					cErrors++;
				break;
			}
			if (!IS_ND(((NodeDesc**)pHdr->guard)[1])) {
				TRACEF(TRACE_L8, ("[FATAL] HeapNoFrag::CheckHeap free block header corrupt\n"));
				BadNode(pHdr);
				break;
			}
			U32 *pGuard=(U32*)&pHdr->guard[0];
			NodeDesc &nd=*((NodeDesc*)pGuard[1]);
			if (nd.state != NodeDesc::FREE) {
				TRACEF(TRACE_L8, ("[WARNING] HeapNoFrag::CheckHeap allocated block still on free list\n"));
				BadNode(pHdr);
				}
			pHdrLast=pHdr;
			pHdr=pHdr->pNext;
		}
	}

	if (cErrors) {
		TRACEF(TRACE_L8, ("[INFORM] HeapNoFrag::CheckHeap %u errors encountered.\n", cErrors));
		Break(raLast, raRet);
	}

	raLast=raRet;
}

	// Node desc at index iNd points to bad node
	void HeapNoFrag::BadNode(int iNd) {
		TRACEF(TRACE_L8, ("\nHeapNoFrag Node %u\n", iNd));
		NodeDesc &nd=pNd[iNd];

		if (cErrors++ < 25) {
			TRACEF(TRACE_L8, ("%c%08lx %08lx  pData %s\n",  PTR_BAD(nd.pNode)? '!' :' ',
					&nd.pNode, nd.pNode, (PTR_BAD(nd.pNode)? "BAD" :"")));
			TRACEF(TRACE_L8, ("%c%08lx %08lx  Size=%u\n",  nd.iBucket < nBuckets? ' ' :'!',
					&nd.iBucket, nd.iBucket, pSize[nd.iBucket]));
			TRACEF(TRACE_L8, (" %08lx %08lx  Allocator code address\n", 
					&nd.raAlloc, nd.raAlloc));
			TRACEF(TRACE_L8, ("%c%08lx %08lx  State %s\n",  (nd.state == NodeDesc::FREE || nd.state == NodeDesc::INUSE) ? ' ' :'!',
					&nd.state, nd.state, (nd.state == NodeDesc::FREE? "FREE" : nd.state == NodeDesc::INUSE? "INUSE" :"BAD")));
			TRACEF(TRACE_L8, ("%c%08lx %08lx  Checksum %s\n",  nd.ChecksOut()? ' ' :'!',
					&nd.checksum, nd.checksum, (nd.ChecksOut()? "OK" :"FAILS")));
			
			Header *pHdr=nd.pNode;
			U32 *pGuard=(U32*)&pHdr->guard[0];
		
			if (!PTR_BAD(pHdr)) {
				TRACEF(TRACE_L8, ("Heap node header\n%c%08lx %08lx  Guard bytes %s\n", (pGuard[0] == GUARD)? ' ' :'!',
						pGuard, pGuard[0], "0xbeefd0d0"));
				TRACEF(TRACE_L8, ("%c%08lx %08lx  Backlink to descriptor %08lx\n", (pGuard[1] == (U32)&nd)? ' ' :'!', 
						&pGuard[1], pGuard[1], &nd));
				if (nd.state == NodeDesc::FREE) {
					TRACEF(TRACE_L8, ("%c%08lx %08lx  Free link %s\n", PTR_BAD(pHdr->pNext)? '!' :' ',
						&pHdr->pNext, pHdr->pNext, PTR_BAD(pHdr->pNext)? "BAD PTR" :"OK" ));
				}
				else {
					TRACEF(TRACE_L8, ("%c%08lx %08lx  Size %s\n", pHdr->iBucket == nd.iBucket? ' ' :'!',
						&pHdr->iBucket, pHdr->iBucket, pHdr->iBucket == nd.iBucket? "MATCH" :"MISMATCH" ));
				}
				TRACEF(TRACE_L8, ("%c%08lx %08lx  Heap link to %08lx %s\n", pHdr->pHeap == this? ' ' :'!',
						&pHdr->pHeap, pHdr->pHeap, this, pHdr->pHeap == this? "OK" :"BROKEN"));
			}

			// Dump starting before bad data
			Dump("Heap contents starting 32 bytes before the header\n", ((U32*)pHdr)-8, 24);
		}
				
	}

	// Node at pHdr is bad node
	void HeapNoFrag::BadNode(Header *pHdr) {
		int iNd;
		for (iNd=0; iNd < nNd; iNd++) {
			if (pNd[iNd].pNode == pHdr) {
				BadNode(iNd);
				break;
			}
		}
	}

void HeapNoFrag::Break(U32 raLast, U32 raNow) {
	TRACEF(TRACE_L8, ("Heap start: %08lx\nHeap end:   %08lx\n", pStart, pFree));
	TRACEF(TRACE_L8, ("Previous call: %08lx\nThis call:     %08lx\n", raLast, raNow));

	// Your code has corrupted the OS heap
	//
	*(int*)-1=0; // (Cause a fault and enter the debugger)
	//
	// Somewhere between the last call to the heap checker
	// (whose address raLast is this procedure argument),
	// and the call on the return stack right now
	// (whose address raNow is this procedures other argument)
	// your code has overwritten a heap structure.
	//
	// Examine your console output for a description of the
	// heap fault(s) detected.
	//
	// View raLast and raNow in hexadecimal, then find the code
	// in your link map.
	//
	// You may wish to turn on TRACE_HEAP trace level
	// to get a heap access trace.
	}


void Dump(char *pTitle, U32 *pDump, int nDump) {
	TRACEF(TRACE_L8, (pTitle));
	for (int i=0; i < nDump/4; i++) {
		TRACEF(TRACE_L8, ("%08lx  ", &pDump[i*4]));
		for (int j=0; j < 4; j++)
			TRACEF(TRACE_L8, ("%08lx ", pDump[i*4 + j]));
		TRACEF(TRACE_L8, ("\n"));
	}
}
#endif

