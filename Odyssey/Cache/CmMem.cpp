/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmMem.cpp
// 
// Description:
// This file implements the CM_Mem class methods of the 
// Cache Manager 
// 
// Update Log 
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "CmMem.h"
#include "CmCommon.h"

/*************************************************************************/
// CM_Mem::Allocate
// Allocate memory and return a pointer to it.  Return 0 if none available.
/*************************************************************************/
void * CM_Mem::Allocate(U32 size) 
{
	void	*p_memory;
	void	*p_next_memory;
	
	if (m_memory_available < size)
		return 0;
		
	// Align memory on dword boundary.
	p_memory = (void*)((U32)m_p_memory & 0xFFFFFFF8);
		 
	// Calculate pointer to next available memory location
	p_next_memory = (char*)p_memory + size + 7;
	
	// Align memory on dword boundary.
	p_next_memory = (void*)((U32)p_next_memory & 0xFFFFFFF8);
		 
	// Update amount of memory left
	m_memory_available -= ((char*)p_next_memory - (char*)m_p_memory);
	
	// Update pointer to next available memory
	m_p_memory = p_next_memory;
	
	// Return pointer to memory allocated.
	return p_memory;

} //CM_Mem::Allocate

/*************************************************************************/
// If we run out of memory, call a method so we can set a breakpoint
// and see where we run out of memory.
/*************************************************************************/
Status CM_No_Memory()
{
	return CM_ERROR(NO_MEMORY);
}

