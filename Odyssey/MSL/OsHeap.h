// OsHeap.h -- Master OOS Heap
//
// Copyright (C) ConvergeNet Technologies, 1998-1999.
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Log: /Gemini/Odyssey/MSL/OsHeap.h $
// 
// 10    9/02/99 2:13p Iowa
// 
// 9     8/30/99 2:58p Iowa
// Large allocations come from heapBig.
// 
// 8     8/25/99 12:19p Jaltmaier
// tPCIBIT
// 
// 7     8/24/99 8:27p Jaltmaier
// Fragmented heaps.  PCI/NONPCI heaps.
// 
// 6     5/10/99 9:11a Jhatwich
// 
// 5     4/01/99 12:38p Jaltmaier
// Use new Critical.h
// 
// 4     3/18/99 11:40a Ewedel
// Changed CheckOs() so that it takes an optional "comment" string which
// is displayed when a code fault is detected.  This can label, e.g., the
// calling DDM.
//
//   10/19/98 Tom Nelson: Created
//   12/15/98 Joe Altmaier: bHeapInitialized
//   07/29/99 Joe Altmaier: "C" initialization entry point.  Added heapBigPci.
//



#ifndef __OsHeap_H
#define __OsHeap_H

#include "OsTypes.h"
#include "HeapBlock.h"
#include "HeapNoFrag.h"


class OsHeap {
public:
	static CHeapBlock heapBigPci;
	static CHeapBlock heapBig;
	static HeapNoFrag heapSmall;

	static BOOL bHeapInitialized;
	
	static void Initialize();
	static void Initialize(void *pRam,U32 sRamBig,U32 sRamSmall);
	static STATUS Free(void *pMem, U32 raRet);
	
	static void CheckHeap();
	static void CheckOs(const char *pszCallerId = NULL);
	static void Break(U32 raLast, U32 raNow);
};

#endif	// __OsHeap_H

