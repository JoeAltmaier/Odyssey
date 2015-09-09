 /* HeapBlock.h -- Block Heap Manager
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Conventions:
 *		"C"  - Begins Class names
 *		"_"  - Ends Member variable names
 *
 * Revision History:
 *     10/02/98 Tom Nelson: Created
 *
**/

#ifndef HeapBlock_H
#define HeapBlock_H

//#define _DEBUG
#include "Odyssey_Trace.h"
#include "HeapLink.h"

#ifdef _WIN32
#pragma pack(push, HeapBlock,1)		// Testing alloc alignments
#endif

#define TYPEALLOC		'Used'		// Allocated
#define TYPEFREE		'Free'		// Unlocked free block
#define TYPELOCK		'Lock'		// Locked free block
#define TYPEEND			'Last'		// End marker

#define ALIGNSIZE		64
#define ALIGNMASK		0x3F


typedef struct _blkHead {		// Block Header	** MUST BE 64 BYTE ALIGNED **
	U8  guard[ALIGNSIZE - 20];	// Guard bytes, so header adds up to 64 bytes

	U32 raAlloc;				// address of code allocating this block
	U32 sPrev;					// Size previous block
	U32 sThis;					// Size next block

	U32 tThis;					// Type of this block
	HeapLink *pThis;		    // Signature -> this Heap	** MUST BE LAST IN HEADER **
} BLKHEAD;

typedef struct _blkLink {		// Free Block link
	BLKHEAD blkHead;
	BLKHEAD *pPrevFree;
	BLKHEAD *pNextFree;
} BLKLINK;

#define GetLink(pBlk)	((BLKLINK *) pBlk )


class CHeapBlock : public HeapLink {
public:
	CHeapBlock()	{}
	ERC Initialize(U32 nFragment, U32 aP[], U32 aCb[], U16 sUnit);
	void *Alloc(U32 &nBytes, U32 ra);
	U32 Free(void *pBlock);
	ERC Check(void);
	void Dump(U32 cDumpMax);

protected:
	BLKHEAD *GetBlk(void *pData)		{ return ((BLKHEAD *) pData) - 1; }
	BLKHEAD *GetNext(BLKHEAD *pBlk)		{ return (BLKHEAD *) ((U8 *) (pBlk+1) + pBlk->sThis); }
	BLKHEAD *GetPrev(BLKHEAD *pBlk)		{ return (BLKHEAD *) ((U8 *) (pBlk-1) - pBlk->sPrev); }

	void _Link(BLKHEAD *pBlk);
	void _Unlink(BLKHEAD *pBlk);
	BLKHEAD * _LockFirst(void);
	BLKHEAD * _LockNext(BLKHEAD *pBlk);
	void _SplitBlk(BLKHEAD *pBlk,U32 nBytes);
	BOOL _IsValidType(U32 type)			{ return type == TYPEALLOC || type == TYPEFREE || type == TYPELOCK || type == TYPEEND; }

	BLKHEAD *pHeap_;		// -> Heap Ram
	U32   sHeap_;		// Total bytes in heap
	U16   sUnit_;		// size of minimum allocation
	U16   sSplit_;		// Size of unit + block header

	BLKHEAD *pFree_;	// -> First free block
	U32  sFree_;		// Total bytes free
};

#ifdef _WIN32
#pragma pack(pop, HeapBlock)
#endif


#endif // HeapBlock_H
