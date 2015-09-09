/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmMem.h
// 
// Description:
// This file defines the interfaces for the Flash Block memory object.
// 
// Update Log 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/


#if !defined(CmMem_H)
#define CmMem_H

#include "Simple.h"

class CM_Mem
{
public:

	// Initialize object.
	void Initialize(U32 memory_size, void *p_memory);
	
	// Allocate memory and return pointer.
	// Return 0 if none available.
	void *Allocate(U32 size);
	
	// How much memory is left?
	U32 Memory_Available();

private:
	void 	*m_p_memory_initial;
	void 	*m_p_memory;
	U32 	 m_memory_size;
	U32 	 m_memory_available;
	
};

inline void CM_Mem::Initialize(U32 memory_size, void *p_memory)
{
	m_p_memory_initial = p_memory;

	// Align memory on dword boundary.
	m_p_memory = (void*)((U32)p_memory + 7 & 0xFFFFFFF8);
		 
	// Calculate pointer to next available memory location
	char *p_next_memory = (char*)p_memory + memory_size;
	
	// Align memory on dword boundary.
	p_next_memory = (char *)((U32)p_next_memory & 0xFFFFFFF8);

	m_memory_size = memory_size;

	// Update amount of memory left
	m_memory_available = ((char*)p_next_memory - (char*)m_p_memory);
}

inline U32 CM_Mem::Memory_Available()
{
	return m_memory_available;
}

#endif // CmMem_H

