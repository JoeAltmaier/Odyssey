/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpMemory.cpp
// 
// Description:
// This file implements interfaces to system memory via C++ new and delete.
// We can allocate NoFrag heap memory, Block heap memory or PCI window memory.
// Also includes code to translate addresses to a physical address usable
// for DMA.
// 
// Update Log
//	$Log: /Gemini/Odyssey/FCP/FcpMemory.cpp $
// 
// 5     10/06/99 4:33p Mpanas
// Remove pcimap.h to make jerry happy
// 
// 3     5/10/99 5:07p Mpanas
// Changes to support Eval and Odyssey FCP builds
// Move FCP_Get_DMA_Address() to FcpMemory.cpp
// so we can use the macros in Address.h for PCI
// address translation
// 
// 11/25/98 Michael G. Panas: Create file
/*************************************************************************/

#include "FcpCommon.h"
#include "FcpMemory.h"
#include "OsTypes.h"
#include "HeapBlock.h"
#include "OsHeap.h"
#include "Address.h"
#include "pcidev.h"

/*************************************************************************/
// Forward references
/*************************************************************************/


/*************************************************************************/
// Global references
/*************************************************************************/

extern	CHeapBlock OsHeap::heapBig;		// The Master Block Heap

/*************************************************************************/
// FCP_Memory_Create
// Create FCP_Memory object
/*************************************************************************/
STATUS	FCP_Memory_Create(PINSTANCE_DATA Id)
{
 	FCP_TRACE_ENTRY(FCP_Memory_Create);

	return NU_SUCCESS;
	
} // FCP_Memory_Create

/*************************************************************************/
// FCP_Memory_Destroy
/*************************************************************************/
void	FCP_Memory_Destroy()
{
 	FCP_TRACE_ENTRY(FCP_Memory_Destroy);
		
} // FCP_Memory_Destroy

/*************************************************************************/
// FCP_Alloc
/*************************************************************************/
void * FCP_Alloc(U32 type, U32 size)
{
 	FCP_TRACE_ENTRY(FCP_Alloc);

#ifdef FCP_DEBUG
	if (OsHeap::heapBig.Check() )
	{
		FCP_PRINT_STRING(TRACE_L2, "\n\rCorrupt Heap!"); 
	}
#endif

	return (new (type) char[size]);
} // FCP_Alloc

/*************************************************************************/
// FCP_Free
/*************************************************************************/
void FCP_Free(void *p_memory)
{
 	FCP_TRACE_ENTRY(FCP_Free);

	delete p_memory;
} // FCP_Free

/*************************************************************************/
// FCP_Get_DMA_Address
// Convert addr to a physical address in PCI space
/*************************************************************************/
void * FCP_Get_DMA_Address(void *addr)
{
	void	* address;
	
 	FCP_TRACE_ENTRY(FCP_Get_DMA_Address);
 	
	FCP_PRINT_HEX(TRACE_L3, "\n\rFCP_Get_DMA_Address: address in ", (U32) addr);
	
 	// is this a local physical PCI address?
 	if (IS_PALOCAL(PA_P(addr)))
 	{
 		// yes - this is a local address,
 		// convert to a local PCI address
#ifdef	_ODYSSEY
 		address = (void *)(PA_P(addr) - memmaps.paSlave + memmaps.pciSlave);
#else
 		address = (void *)PCI_PALOCAL(PA_P(addr));
#endif
 	}
 	else 
 	{
 		// no - must be an off-board PCI address
 		// convert to physical PCI address
 		address = (void *)PA_P(addr);
 	}
	
	FCP_PRINT_HEX(TRACE_L3, "\n\rFCP_Get_DMA_Address: address out ", (U32) address);
	
	return (address);
	
} // FCP_Get_DMA_Address


