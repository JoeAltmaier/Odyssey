/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiMemory.cpp
// 
// Description:
// This file implements interfaces to system memory via C++ new and delete.
// We can allocate NoFrag heap memory, Block heap memory or PCI window memory.
// Also includes code to translate addresses to a physical address usable
// for DMA.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiMemory.cpp $
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
// 
/*************************************************************************/

#include "HscsiCommon.h"
#include "HscsiMemory.h"
#include "OsTypes.h"
#include "HeapBlock.h"
#include "OsHeap.h"
#include "Address.h"

/*************************************************************************/
// Forward references
/*************************************************************************/


/*************************************************************************/
// Global references
/*************************************************************************/

extern	CHeapBlock OsHeap::heapBig;		// The Master Block Heap

/*************************************************************************/
// HSCSI_Memory_Create
// Create HSCSI_Memory object
/*************************************************************************/
STATUS	HSCSI_Memory_Create(PHSCSI_INSTANCE_DATA Id)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Memory_Create);

	return NU_SUCCESS;
	
} // HSCSI_Memory_Create

/*************************************************************************/
// HSCSI_Memory_Destroy
/*************************************************************************/
void	HSCSI_Memory_Destroy()
{
 	HSCSI_TRACE_ENTRY(HSCSI_Memory_Destroy);
		
} // HSCSI_Memory_Destroy

/*************************************************************************/
// HSCSI_Alloc
/*************************************************************************/
void * HSCSI_Alloc(U32 type, U32 size)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Alloc);

#ifdef HSCSI_DEBUG
	if (OsHeap::heapBig.Check() )
	{
		HSCSI_PRINT_STRING(TRACE_L2, "\n\rCorrupt Heap!"); 
	}
#endif

	return (new (type) char[size]);
} // HSCSI_Alloc

/*************************************************************************/
// HSCSI_Free
/*************************************************************************/
void HSCSI_Free(void *p_memory)
{
 	HSCSI_TRACE_ENTRY(HSCSI_Free);

	delete p_memory;
} // HSCSI_Free

/*************************************************************************/
// HSCSI_Get_DMA_Address
// Convert addr to a physical address in PCI space
/*************************************************************************/
void * HSCSI_Get_DMA_Address(void *addr)
{
	void	* address;
	
 	HSCSI_TRACE_ENTRY(HSCSI_Get_DMA_Address);
 	
	HSCSI_PRINT_HEX(TRACE_L3, "\n\rHSCSI_Get_DMA_Address: address in ", (U32) addr);
	
 	// is this a local physical PCI address?
 	if (IS_PALOCAL(PA_P(addr)))
 	{
 		// yes - this is a local address,
 		// convert to a local PCI address
 		address = (void *)PCI_PALOCAL(PA_P(addr));
 	}
 	else 
 	{
 		// no - must be an off-board PCI address
 		// convert to physical PCI address
 		address = (void *)PA_P(addr);
 	}
	
	HSCSI_PRINT_HEX(TRACE_L3, "\n\rHSCSI_Get_DMA_Address: address out ", (U32) address);
	
	return (address);
	
} // HSCSI_Get_DMA_Address


