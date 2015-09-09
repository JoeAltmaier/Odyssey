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
// This class manages a memory pool as free lists of blocks of discrete sizes.
// Alloc returns the smallest block size that will fit.
// Freeing a block returns it to the free list.
// The partition is used to create more blocks when none are available
// on some free list.
// 
// Update Log: 
// 6/12/98 Joe Altmaier: Create file
//  8/6/98 Joe Altmaier: add pSem
//10/05/98 Tom Nelson:   Added block headers
/*************************************************************************/

#ifndef __Heap_h
#define __Heap_h

#define HEAP_LEAK_CHECK

#include "HeapLink.h"
#include "Critical.h"

#ifdef HEAP_LEAK_CHECK
#include "stdio.h"
#endif

class HeapNoFrag : public HeapLink {
	typedef struct Header {	// ** Must be 32 byte aligned - cache line size **
		U8 guard[24];		// Guard bytes so takes a whole cache line
		union {
			int iBucket;
			Header *pNext;
			U32 lSpacer;	// Force 4 byte union
		};
		HeapLink *pHeap;		// ** Must be last in header **
	} Header;

	// Bucket definitions
	Header **pBuckets;
	int *pSize;
	int nBuckets;
	char *pFree;
	int cbFree;

	// Bucket lookup map
	int cbMin;
	int cbMax;
	int *pBucket_Cb;
	
#ifdef _HEAPCHECK
// for NodeDesc
#define CHECKSUM 0xBABE
#define NEWNODE 0x50FAB00Bl
#define GUARD 0xBEEFD0D0l

#define PTR_BAD(p)	(p && ( ((U32)p & 0x1FFFFFFF) < ((U32)pStart & 0x1FFFFFFF) || ((U32)p & 0x1FFFFFFF) >= ((U32)pFree & 0x1FFFFFFF) ))
#define IS_ND(p)	(p >= pNd && p < &pNd[nNdMax])

	typedef struct {
		Header *pNode;
		int iBucket;
		U32 raAlloc;
		enum TyState {INUSE, FREE} state;
		U32 checksum;
		void SetRa(U32 ra) 				{ raAlloc=ra; CheckSum(); }
		void SetI(U32 i) 				{ iBucket=i; CheckSum(); }
		void SetState(TyState state_) 	{ state=state_; CheckSum(); }
		void SetP(Header *p) 			{ pNode=p; CheckSum(); }
		void CheckSum() 				{ checksum=(U32)pNode ^ iBucket ^ raAlloc ^ state ^ CHECKSUM; }
		BOOL ChecksOut()				{ Critical section; BOOL b= ((checksum ^ (U32)pNode ^ iBucket ^ raAlloc ^ state) == CHECKSUM); return b; }
	} NodeDesc;

	char *pStart;
	NodeDesc *pNd; // for CheckHeap
	U32 nNd;
	U32 nNdMax;
	int cErrors;
#endif
	
public:
#ifdef HEAP_LEAK_CHECK
	long cbAllocReportPrev;
	void ReportDeltaMemoryUsage(char* sMsg) {
		long lRep = cbAllocTotal - cbAllocReportPrev;
		cbAllocReportPrev = cbAllocTotal;
		printf("%d\t%d %s\n",lRep, cbAllocTotal, sMsg);
	}
#endif

	long cbAllocTotal;
		
public:
	HeapNoFrag();
	void Initialize(int nBuckets, void *pMemory, int cb, int cbMax);
	void *Alloc(U32 &cb, U32 ra); // Allocate from heap buckets
	virtual U32 Free(void *); // Free block to heap bucket

#ifdef _HEAPCHECK
	void InitDebug(void *pMemory, int cb);
	void CheckHeap(U32 ra);
	NodeDesc *NewNode();
	void BadNode(int iNd);
	void BadNode(Header *);
	void Break(U32 raLast, U32 raNow);
#endif

	int CbMax() { return cbMax; }
	int CbFree() { return cbFree; }
private:
	void *AllocFree(int cb); // Allocate from partition e.g. ran out of blocks in a bucket
};

#endif