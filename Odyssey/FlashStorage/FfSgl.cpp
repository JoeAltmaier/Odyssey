/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfSgl.cpp
// 
// Description:
// 
// 1/3/00 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfSgl.h"
#include "FfCommon.h"
#include "Message.h"


/*************************************************************************/
// FF_SGL 
// construct from simple pointer and count.
/*************************************************************************/
FF_SGL::FF_SGL(void * pb, U32 cb)
{
	m_p_element = &m_element;
	m_element.count = cb;
	m_element.address = (U32)pb;
	m_element.flags = SGL_FLAGS_LAST_ELEMENT | SGL_FLAGS_LOCAL_ADDRESS;
	m_offset = 0;
}

/*************************************************************************/
// FF_SGL 
// construct from pointer to SGL in a message.
/*************************************************************************/
FF_SGL::FF_SGL(SGE_SIMPLE_ELEMENT *p_element)
{
	m_p_element = p_element;
	m_offset = 0;
}

/*************************************************************************/
// Build_TyDma 
/*************************************************************************/
Status FF_SGL::Build_TyDma(void *transfer_address, U32 transfer_byte_count, 
							Transfer_Direction transfer_direction,
							DmaCallback p_callback, void *p_context, U32 flags, 
							TyDma **pp_TyDma)
{
	// Point to first element in the list.
	SGE_SIMPLE_ELEMENT *p_element = m_p_element;

	// element_offset_byte_0 has offset of first byte of current element in the list.
	U32 element_offset_byte_0 = 0;

	// Get size of first element.
	U32 element_size = p_element->count;
	
	U32 element_index = 0;

	TRACEF(TRACE_L7, ("\nBuild_TyDma, transfer_byte_count = %d, element_size = %d, offset = %d", 
		transfer_byte_count, element_size, m_offset));

	// Find the element that contains the data we are looking for.
	// m_offset is the offset of the data to access within the SGL.
	while ((element_offset_byte_0 + element_size) <= m_offset)
	{
		// This element does not contain the data we are looking for.
		// Make sure we have another element.
		if (p_element->IsLast())
			return FF_ERROR(FLASH_SGL_OVERRUN);

		// Step to the next element.
		p_element++;
		element_offset_byte_0 += element_size;
		element_index++;

		// Get size of next element.
		element_size = p_element->count;
	}

	TRACEF(TRACE_L7, ("\nBuild_TyDma first element_size = %d, element_index = %d, element_offset_byte_0 = %d", 
		element_size, element_index, element_offset_byte_0));

	// p_element points to the element that contains the beginning
	// of the data we need to access.
	// Calculate the element address of the first byte of the current element.
	// P_SGL gets the address if it is local or remote.
	char* next_element_address = (char *)P_SGL(p_element[0]);
	
	// Calculate the offset within the current element of the data we wish to access.
	U32 element_offset_data = m_offset - element_offset_byte_0;
	
	// Increment next_element_address to point to the first byte of data to access.
	next_element_address += element_offset_data;

	// Calculate remaining bytes in this element.
	U32 remaining_element_byte_count = element_size - element_offset_data;

	// Set up first transfer_address (source or dest)
	char* next_transfer_address = (char *)transfer_address;
	void* source;
	void* dest;
	if (transfer_direction == Transfer_Address_Is_Dest)
	{
		dest = next_transfer_address;
		source = next_element_address;
	}
	else
	{
		// Transfer_Address_Is_Source
		dest = next_element_address;
		source = next_transfer_address;
	}

	// Should we transfer all of this element?
	U32 current_transfer_byte_count;
	if (remaining_element_byte_count > transfer_byte_count)
		current_transfer_byte_count = transfer_byte_count;
	else
		current_transfer_byte_count = remaining_element_byte_count;

	// Create first DMA request.		
	TyDma* p_dma_first = new TyDma(NULL, source, dest, current_transfer_byte_count, 
		NULL, NULL, flags);
	if (p_dma_first == NULL)
		return FF_ERROR(NO_MEMORY_TYDMA);
		
	TRACEF(TRACE_L7, ("\nBuild_TyDma first count = %d, source = %X, dest = %X", 
		current_transfer_byte_count, source, dest));

	TyDma* p_dma_last = p_dma_first;

	U32 remaining_transfer_byte_count = transfer_byte_count - current_transfer_byte_count;
	while (remaining_transfer_byte_count != 0)
	{
		// Make sure we have another element.
		if (p_element->IsLast())
			return FF_ERROR(FLASH_SGL_OVERRUN);

		// Step to the next element.
		p_element++;

		if (transfer_direction == Transfer_Address_Is_Dest)
		{
			// Should we increment the dest address?
			if ((flags & GT_DMA_DDIR_HOLD) == 0)
				next_transfer_address += current_transfer_byte_count;
			dest = next_transfer_address;

			source = (char *)P_SGL(p_element[0]);
		}
		else
		{
			// Transfer_Address_Is_Source
			// Should we increment the source address?
			if ((flags & GT_DMA_SDIR_HOLD) == 0)
				next_transfer_address += current_transfer_byte_count;
			source = next_transfer_address;

			dest = (char *)P_SGL(p_element[0]);
		}

		// Get size of next element.
		element_size = p_element->count;

		// Should we transfer all of this element?
		if (element_size > remaining_transfer_byte_count)
			current_transfer_byte_count = remaining_transfer_byte_count;
		else
			current_transfer_byte_count = element_size;
			
		// Create next DMA request.
		TyDma* p_dma_next = new TyDma(NULL, source, dest, current_transfer_byte_count,
			NULL, NULL, flags);
			
		TRACEF(TRACE_L7, ("\nBuild_TyDma next count = %d, source = %X, dest = %X", 
			current_transfer_byte_count, source, dest));
			
		if (p_dma_next == NULL)
			return FF_ERROR(NO_MEMORY_TYDMA);

		// Chain the last dma request to the new one.
		p_dma_last->pNext = p_dma_next;
		p_dma_last = p_dma_next;

		// And decrement the amount left to transfer.
		remaining_transfer_byte_count -= current_transfer_byte_count;

	} // while (remaining_transfer_byte_count != 0)
	
	// Store callback at end of chain.
	p_dma_last->pProcCallback = p_callback;
	p_dma_last->pArg = p_context;
	
	// Return pointer to first DMA request.
	*pp_TyDma = p_dma_first;

	return OK;

} // Build_TyDma


/*************************************************************************/
// FF_SGL operator = 
// assignment operator
/*************************************************************************/
FF_SGL& FF_SGL::operator=(const FF_SGL& right_side)
{
    // Does the element pointer in the source point to the
	// element in the source?
	if (right_side.m_p_element == &right_side.m_element)
	{
		m_p_element = &m_element;
		m_element.count = right_side.m_element.count;
		m_element.address = right_side.m_element.address;
		m_element.flags = SGL_FLAGS_LAST_ELEMENT | SGL_FLAGS_LOCAL_ADDRESS;
		m_offset = 0;
	}
	else
	{
		m_p_element = right_side.m_p_element;
		m_offset = right_side.m_offset;
	}

	return *this;
}

