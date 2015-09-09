// OsHeap.cpp -- Master CHAOS Heap
//
// Copyright (C) ConvergeNet Technologies, 1998-1999
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Log: /Gemini/Odyssey/MSL/OsHeap.cpp $
// 
// 32    1/07/00 5:38p Agusev
// Fixed WIN32 build. 
// Is this the last time or what?!
// 
// 31    12/16/99 3:33p Iowa
// Heap leak detection.
// 
// 30    12/09/99 1:54a Iowa
// 
// 29    11/11/99 3:40p Iowa
// Fix eval memory initialization
// 
// 28    10/19/99 4:52p Jnespor
// Enabled gp relative addressing (sdata) and set compiler optimization to
// level 4.  Disabled run time type information (RTTI).
// 
// 25    9/07/99 1:51p Joehler
// Modified definition of bzero() in WIN32 environment to correctly zero
// out memory allocated with new (tZERO) function
// 
// 24    9/02/99 2:13p Iowa
// 
// 23    8/30/99 2:58p Iowa
// Large allocations come from heapBig.
// 
// 20    8/25/99 12:19p Jaltmaier
// tPCIBIT
// 
// 19    8/25/99 12:11p Jhatwich
// win32 compile fix
// 
// 18    8/25/99 11:55a Jaltmaier
// Win32 Initialize
// 
// 17    8/24/99 8:27p Jaltmaier
// Fragmented heaps.  PCI/NONPCI heaps.
// 
// 16    7/07/99 9:08a Jhatwich
// 
// 15    6/23/99 3:57p Agusev
// Increased the size of the memory pool for WIN32
// 
// 14    6/23/99 11:25a Jhatwich
// win32 fix
// 
// 13    6/16/99 10:17p Jaltmaier
// Add tZERO feature to new()
// 
// 12    5/18/99 10:54a Agusev
// Overloaded new[] (parm1, parm2, parm3) for WIN32
// 
// 11    5/13/99 11:32a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 10    5/10/99 9:16a Jhatwich
// win32
// 
// 9     5/06/99 12:00a Jaltmaier
// Removed spurious TRACEs.
// 
// 8     4/21/99 3:27p Jaltmaier
// WIN32
// 
// 7     4/06/99 7:20p Tnelson
// Check for NULL passed to OsHeap::Free
// 
// 6     4/01/99 12:38p Jaltmaier
// Use new Critical.h
// 
// 5     3/18/99 11:41a Ewedel
// Changed CheckOs() so that it takes an optional "comment" string which
// is displayed when a code fault is detected.  This can label, e.g., the
// calling DDM.
//
//   4/20/99 Joe Altmaier: WIN32 version
//   12/15/98 Joe Altmaier: bHeapInitialized
//   11-11-98 Tom Nelson: Added new with allocation type argument
//   10/19/98 Tom Nelson: Created
//

//345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_HEAP
#include "Odyssey_Trace.h"
// Define _HEAPCHECK to use any of the following heap debugging features
// Set TRACE_HEAP to >= TRACE_L8 to print heap trace
// Set TRACE_HEAP1 to anything to enable CheckHeap on every heap operation
// Set TRACE_HEAP1 to >= TRACE_L4 to erase deleted memory


#include "OsHeap.h"
#include "Address.h"
#define TYPES_H	// don't include Types.h
#define I32 signed long
#include "Mips_util.h"

extern "C" void OsHeap_Initialize(U32 nFragment, U32 aPa[], U32 aCb[], U32 nReserved, U32 aPaRes[], U32 aCbRes[], U32 cbHeapSmall);

#ifdef WIN32

#include "stdlib.h"
#include "malloc.h"

#else
#include "Hw.h"
#include <__rts_info_t__.h>
extern "C" char _erostuff[];
#endif

#ifdef _HEAPCHECK
#define HEAPCHECK(ra) 	if (TraceLevel[TRACE_HEAP1]) OsHeap::heapSmall.CheckHeap(ra);

#else
#define HEAPCHECK(ra)	
#endif

extern "C" void *AllocBlock(U32 nBytes);
#ifdef WIN32
void bzero(void* pMem, U32 nBytes) {
	// JLO - changed definition of bzero() on
	// WIN32 to correctly zero out allocated memory
	memset(pMem, 0, nBytes);
}
#else
extern "C" void	bzero(void *pMem, U32 nBytes);
#endif

#ifdef WIN32
I64 raRet=0;
#define RARET(raRet)	

#else
#ifdef _HEAPCHECK
#define RARET(raRet)	I64 raRet=0; asm { sd ra, raRet;	}
#else
I64 raRet=0;
#define RARET(raRet)	
#endif
#endif // WIN32

#ifdef _HEAPCHECK
extern "C" void Alloc(U32 cb, U32 ip);
extern "C" void Free(U32 cb, U32 ip);
#define ALLOC(cb,ip) Alloc(cb,ip)
#define FREE(cb,ip) ::Free(cb,ip)
#else
#define ALLOC(cb, ip)	
#define FREE(cb, ip)	
#endif

BOOL OsHeap::bHeapInitialized=false;

CHeapBlock OsHeap::heapBigPci;	// The PCI Block Heap
CHeapBlock OsHeap::heapBig;		// The Master Block Heap
HeapNoFrag OsHeap::heapSmall;	// The No Frag Heap (New/Malloc)

U32 addrCodeLow;
U32 addrCodeHigh;
U32 *pHistAlloc;


// Initialize -- Setup CHAOS Big and Small heaps --------------------------OsHeap-
//
void OsHeap::Initialize()
{
#ifdef WIN32
	Initialize(calloc(20000000,1), 20000000, 8000000);
#else
	U32 sHeap=1048576;
	Initialize(_end, sHeap, sHeap - 1024);
#endif
}

// OsHeap_Initialize -- Setup CHAOS Big and Small heaps -------------------OsHeap-
//
extern "C" void OsHeap_Initialize(U32 nFragment, U32 aPa[], U32 aCb[], U32 nReserved, U32 aPaRes[], U32 aCbRes[], U32 cbHeapSmall) {
//	mips_init_cache();
	RARET(raRet);

	// Edit the fragment list, to shrink a fragment that contains reserved memory.
	// Also shrink the fragment that contains the pci slave memory, assuming frag starts with pci memory.
	for (int iRes=0; iRes < nReserved; iRes++) {
		for (int i=0; i < nFragment; i++) {
			if (iRes == 0 && i == 0)
				continue; // Don't remove PCI reserved from PCI fragment
				
			// Reserved memory contained in fragment?
			if (aPaRes[iRes] + aCbRes[iRes] > aPa[i] && aPaRes[i] < aPa[i] + aCb[i]) {
				// Reserved at front, end or in middle?
				if (aPa[i] == aPaRes[iRes]) {
					// Front
					aCb[i] -= aCbRes[iRes];
					aPa[i] += aCbRes[iRes];
				}
				else if (aPa[i] + aCb[i] == aPaRes[iRes] + aCbRes[iRes]) {
					// End
					aCb[i] -= aCbRes[iRes];
				}
				else {
					// Middle, split into two
					// Make room for 2nd split
					for (int j=nFragment; j > i+1; j--) {
						aPa[j] = aPa[j-1];
						aCb[j] = aCb[j-1];
					}
					nFragment++;
					// New fragment starts where reserved memory ends
					aPa[i+1] = aPaRes[iRes] + aCbRes[iRes];
					aCb[i+1] = (aPa[i] + aCb[i]) - aPa[i+1];
					// 1st split starts same place, size == distance to reserved
					aCb[i] = (aPaRes[iRes] - aPa[i]);
				}
				break; // Found and processed this reserve memory, on to the next.
			}
		}
	}

	// Lay out three heaps:
	// Block heap containing PCI-mapped (pci slave) memory
	// 	inside that, small heap
	// Block heap containing the rest of memory
	
	// Find unreserved fragments of PCI slave window.  Use aPaRes[0] to find slave window bounds.
	int nSlave=0;
	for (; nSlave < nFragment; nSlave++)
		if (aPa[nSlave] >= aPaRes[0] + aCbRes[0])
			break;

	{
	U32 aP[]={(U32)P_PA(aPa[0]), (U32)P_PA(aPa[1]), (U32)P_PA(aPa[2])}; // at most three
	OsHeap::heapBigPci.Initialize(nSlave, aP, aCb, 2048);
	}
	
	void *pMemSmall=OsHeap::heapBigPci.Alloc(cbHeapSmall, raRet);	// Uncached
	
	OsHeap::heapSmall.Initialize(16, PUNCACHED(pMemSmall),cbHeapSmall,65536);	// Cached or uncached

#ifdef _HEAPCHECK
	pMemSmall=OsHeap::heapBigPci.Alloc(cbHeapSmall, raRet);
	
	OsHeap::heapSmall.InitDebug(PUNCACHED(pMemSmall), cbHeapSmall);
#endif


	if (nFragment > nSlave)
		{
		U32 aP[]={(U32)P_PA(aPa[nSlave]), (U32)P_PA(aPa[nSlave+1]), (U32)P_PA(aPa[nSlave+2])}; // at most three
		OsHeap::heapBig.Initialize(nFragment - nSlave, aP, &aCb[nSlave], 2048); // Cached or uncached
		}
		
	OsHeap::bHeapInitialized=true;
}

void OsHeap::Initialize(void *pRam,U32 sRamBig,U32 sRamSmall)
{
//	OsHeap::heapBigPci.Initialize(nSlave, aP, aCb, 2048);

	OsHeap::heapSmall.Initialize(16, pRam, sRamSmall, 65536);

	U32 aP[]={(U32)((char*)pRam + sRamSmall)};
	U32 aCb[]={sRamBig - sRamSmall};
	
	OsHeap::heapBig.Initialize(1, aP, aCb, 2048);

	OsHeap::bHeapInitialized=true;
}

// Free -- Return memory to it's appropriate heap -----------------------OsHeap-
//
STATUS OsHeap::Free(void *pMem, U32 raRet)
{
	HeapLink *pHeap;

	if (pMem == NULL)
		return OK;

	pMem=PUNCACHED(pMem);
			
	pHeap = *(((HeapLink **) pMem)-1);
	
	if (HeapLink::IsValidRamPtr(pHeap) && pHeap->signature == HEAPSIGN) {
		HEAPCHECK(raRet);

		pHeap->Free(pMem);
	}
	else
		Tracef("Invalid Heap Signature: pMem=%lx; pHeap=%lx; ->signature=\"%4.4s\" (%lx)\n", pMem, pHeap,&pHeap->signature,pHeap->signature);
		
	return OK;
}

// new[]
//
// PURPOSE:		Overloaded for WIN32. Sets actual allocated size to whatever was requested
#ifdef WIN32
void *operator new[](unsigned int cb_, DID did_, int *pCbActual_) {

	*pCbActual_=cb_;
	return new char[cb_];
}
#endif // WIN32

// New -- Overload Operator ----------------------------------------------------
//
// Overload memory New/Delete allocations
//
// Overloaded New is always tSMALL | tCACHED
//
void* operator new(unsigned int nBytes)
{
	RARET(raRet);

	if (!OsHeap::bHeapInitialized)
			OsHeap::Initialize();
			
	HEAPCHECK(raRet);
		
	void *pMem = OsHeap::heapSmall.Alloc((U32 &)nBytes, raRet);
	ALLOC(nBytes, raRet);

	TRACEF(TRACE_L8, ("%08lx =new[%u]\n", pMem, nBytes));

	pMem = PCACHED(pMem);
//	mips_sync_cache(pMem, (nBytes + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_CPU);

	return pMem;
}

// New -- Specify Allocation Type ----------------------------------------------

void* operator new[](unsigned int nBytes) {
	RARET(raRet);

	if (!OsHeap::bHeapInitialized)
			OsHeap::Initialize();

	TRACEF(TRACE_L8, ("new(%u)[%u]\n",tSMALL, nBytes));
	
	HEAPCHECK(raRet);

	void *pMem = OsHeap::heapSmall.Alloc( (U32 &)nBytes, raRet);
	ALLOC(nBytes, raRet);

	TRACEF(TRACE_L8, ("%08lx =new[%u]\n", pMem, nBytes));

	pMem = PCACHED(pMem);
//	mips_sync_cache(pMem, (nBytes + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_CPU);

	return pMem;
}

void* operator new[](unsigned int nBytes,int tRam)
{
	RARET(raRet);

	if (!OsHeap::bHeapInitialized)
			OsHeap::Initialize();

	TRACEF(TRACE_L8, ("new(%u)[%u]\n",tRam, nBytes));	

	void *pMem;
	
	if (nBytes > OsHeap::heapSmall.CbMax())
		tRam |= tBIG;

	switch (tRam & tRAMMASK) {
	case tBIG:
		if (tRam & tPCIBIT)
			pMem = OsHeap::heapBigPci.Alloc((U32 &)nBytes, raRet); // always uncached
		else
			pMem = OsHeap::heapBig.Alloc((U32 &)nBytes, raRet);

		break;

	default:	// Only specified modifier
	case tSMALL:
		// Small heap is always pci!
		
		HEAPCHECK(raRet);

		pMem = OsHeap::heapSmall.Alloc((U32 &)nBytes, raRet);
	}

	ALLOC(nBytes, raRet);

	if ((tRam & tCACHEMASK) == tUNCACHED) {
		pMem = PUNCACHED(pMem);
		mips_sync_cache(pMem, (nBytes + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_DEV);
	}
	else {
		pMem = PCACHED(pMem);
//		mips_sync_cache(pMem, (nBytes + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_CPU);
	}

	if (tRam & tZERO)
		bzero(pMem, nBytes); // Use same pMem user will see
			
	return pMem;
}

// Delete -- Overload Operator -------------------------------------------------
//
// Will free heapSmall or heapBig memory or to any heap derived
// from HeapLink.
//
void operator delete(void *pMem)
{
	RARET(raRet);

	TRACEF(TRACE_L8, ("delete(%lx)\n", pMem));	
	
	OsHeap::Free(pMem, raRet);
}


//------------------------------------------------------------------------------
//
// Overload "C" memory Malloc()/Free()
//
#ifndef _WIN32

extern "C" void *malloc(unsigned nBytes)
{
	RARET(raRet);

	if (!OsHeap::bHeapInitialized)
			OsHeap::Initialize();
			
	TRACEF(TRACE_L8, ("malloc(%u)\n", nBytes));
	HEAPCHECK(raRet);

	void *pMem = OsHeap::heapSmall.Alloc(nBytes, raRet);
	ALLOC(nBytes, raRet);

	pMem = PCACHED(pMem);
//	mips_sync_cache(pMem, (nBytes + 31) & 0xFFFFFFE0, SYNC_CACHE_FOR_CPU);

	return pMem;
}


// Frees heapSmall or heapBig memory

extern "C" void free(void *pMem)
{ 
	RARET(raRet);

	TRACEF(TRACE_L8, ("free(%lx)\n", pMem));	
	
	OsHeap::Free(pMem, raRet);
}

#endif//_WIN32

/*
//
// "C" memory AllocBlock()
//
extern "C" void *AllocBlock(U32 nBytes)
{
	if (!OsHeap::bHeapInitialized)
			OsHeap::Initialize();
			
	TRACEF(TRACE_L8, ("AllocBlock(%u)\n", nBytes));

	void *pMem = OsHeap::heapBig.Alloc(nBytes, raRet);
	ALLOC(nBytes, raRet);

	return pMem;
}
*/

//static
void OsHeap::CheckHeap() 
{
	RARET(raRet);

#ifdef _HEAPCHECK
	heapSmall.CheckHeap(raRet);
#endif
}


#undef TRACE_INDEX
#define	TRACE_INDEX		TRACE_HEAP1

U32 checksumOs=0;
U32 raChecksumLast;

//static
void OsHeap::CheckOs(const char *pszCallerId /* = NULL */ ) 
{
	U32 checksum=0xDEADF00D;
	
	RARET(raRet);

#ifndef WIN32
#ifdef _HEAPCHECK

	U32 *p=(U32*)&_ftext;
	while (p < (U32*)&_erostuff)
		checksum = checksum ^ *p++ ^ *p++;
		
	if (checksumOs && checksumOs != checksum) {
		TRACEF(TRACE_L8, ("[FATAL] Code overwritten!"));
      if (pszCallerId != NULL) {
         TRACEF(TRACE_L8, ("  (%s)", pszCallerId));
      }
      TRACEF(TRACE_L8, ("\n"));
		Break(raChecksumLast, raRet);
	}
	
	checksumOs=checksum;
	raChecksumLast=raRet;
#endif
#endif
}

void OsHeap::Break(U32 raLast, U32 raNow) {
	TRACEF(TRACE_L8, ("Previous call: %08lx\nThis call:     %08lx\n", raLast, raNow));

	// Your code has overwritten itself somewhere
	//
	*(int*)-1=0; // (Cause a fault and enter the debugger)
}

#ifdef _HEAPCHECK
void Alloc(U32 cb, U32 ip) {
	if (pHistAlloc)
		if (ip >= addrCodeLow && ip < addrCodeHigh)
			pHistAlloc[(ip - addrCodeLow) >> 2] += cb;
	}

void Free(U32 cb, U32 ip) {
	if (pHistAlloc)
		if (ip >= addrCodeLow && ip < addrCodeHigh)
			pHistAlloc[(ip - addrCodeLow) >> 2] -= cb;
	}
#endif