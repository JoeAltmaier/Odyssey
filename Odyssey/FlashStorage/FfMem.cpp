/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfMem.cpp
// 
// Description:
// This file implements the FF_Mem class methods of the Flash File
// Controller. Memory is allocated in one chunk in the beginning.
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "FlashDevice.h"
#include "FfCommon.h"

/*************************************************************************/
// FF_Mem::Allocate
// Allocate memory and return a pointer to it.  Return 0 if none available.
/*************************************************************************/
void * FF_Mem::Allocate(U32 size, U32 alignment) 
{
	void	*p_memory;
	void	*p_next_memory;
	
	// Align memory on specified boundary.
	p_memory = (void *)ALIGN(m_p_memory, alignment);
		 
	// Calculate pointer to next available memory location
	p_next_memory = (char*)p_memory + size;
	
	// Calculate aligned size.
	U32 aligned_size = ((char*)p_next_memory - (char*)m_p_memory);
		 
	// Check to see if we have that much memory.
	if (m_memory_available < aligned_size)
		return 0;
		
	// Update amount of memory left
	m_memory_available -= aligned_size;
	
	// Update pointer to next available memory
	m_p_memory = p_next_memory;
	
	// Return pointer to memory allocated.
	return p_memory;

} //FF_Mem::Allocate


