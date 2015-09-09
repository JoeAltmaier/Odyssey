/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfMem.h
// 
// Description:
// This file defines the interfaces for the Flash File memory object.
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/


#if !defined(FfMem_H)
#define FfMem_H

#include "Simple.h"

class FF_Mem
{
public:
	// Initialize object
	void Initialize(U32 memory_size, void *p_memory);
	
	// Allocate memory
	void *Allocate(U32 size, U32 alignment = 8);
	
	// How much memory is left?
	U32 Memory_Available();

	// How much memory was allocated?
	U32 Memory_Size();

private:
	void 		*m_p_memory_initial;
	void 		*m_p_memory;
	U32 		 m_memory_size;
	U32 		 m_memory_available;
	
};

inline void FF_Mem::Initialize(U32 memory_size, void *p_memory)
{
	m_p_memory_initial = p_memory;

	// Align memory on dword boundary.
	m_p_memory = (void*)ALIGN(p_memory, 8);
		 
	// Calculate pointer to next available memory location
	char *p_next_memory = (char*)p_memory + memory_size;
	
	// Align memory on dword boundary.
	p_next_memory = (char *)ALIGN(p_next_memory, 8);

	m_memory_size = memory_size;

	// Update amount of memory left
	m_memory_available = ((char*)p_next_memory - (char*)m_p_memory);
	
}

inline U32 FF_Mem::Memory_Available()
{
	return m_memory_available;
}

inline U32 FF_Mem::Memory_Size()
{
	return m_memory_size;
}

#endif // FfMem_H

