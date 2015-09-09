/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmSecondary.cpp
// 
// Description:
// This file implements the Open method of the Cache Manager
// for the secondary cache. 
// 
// Update Log 
// 
// 9/22/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmCache.h"
#include "TraceMon.h"
#include <string.h>

/*************************************************************************/
// CM_Cache::Open_Secondary
// Open a page in the secondary cache.
// We get called if the page is NOT in the primary cache, and the user
// has specified CM_SECONDARY in the call to Open_Page.
/*************************************************************************/
Status CM_Cache::Open_Secondary( 
	I64 page_number,
	U32 flags,
	CM_PAGE_HANDLE *p_page_handle,
	CM_Frame *p_frame_primary)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Open_Secondary, page = ", (U32)page_number);

	// The page is not in the primary cache.
	Status status = OK;

	// Get frame index of primary page.
	U32 frame_index_primary = p_frame_primary->Get_Frame_Index();

	void *p_page_frame_primary;
	void *p_page_frame_secondary;
	CM_Frame *p_frame_secondary;

	// Get page number of page currently in primary cache.
	// Page number will be -1 if frame is empty.
	I64 page_number_frame_primary = p_frame_primary->Get_Page_Number();

	// Is the page in the secondary cache?
	// Get the frame index for this page if it is present.  
	U32 frame_index_secondary = Get_Frame_Index_Secondary(page_number);
	if (frame_index_secondary == 0)
	{
		// The page is not in the secondary cache.
		// Does the page frame in the primary cache have a mapped page?
		if (page_number_frame_primary != -1)
		{
			// The page currently in the primary cache is a mapped page.
			// Unmap the page from its frame in the primary cache.
			Unmap_Page(page_number_frame_primary);

			// We want to move this page to the secondary cache.
			// Replace some other page in the secondary cache.
			p_frame_secondary = Replace_Page_Secondary(
				flags & ~CM_CALLBACK_IF_NO_FRAME, 0);

			// We should always get a page in the secondary cache.
			// These pages are never locked or dirty.
			CT_ASSERT(p_frame_secondary, Open_Secondary);

			// Get index of page in secondary cache.
			frame_index_secondary = p_frame_secondary->Get_Frame_Index();

			// Unmap the page from its former address in the secondary cache.
			if (p_frame_secondary->Get_Page_Number() != -1)
				Unmap_Page_Secondary(p_frame_secondary->Get_Page_Number());

			// Point to the page frame in the primary cache.
			p_page_frame_primary = Get_Page_Frame(frame_index_primary);

			// Point to the page frame in the secondary cache.
			p_page_frame_secondary = Get_Page_Frame_Secondary(frame_index_secondary);

			// Move the page from the primary cache to the secondary cache.
			memcpy(p_page_frame_secondary, p_page_frame_primary, m_config.page_size);

			// Set new page number in secondary page frame.
			p_frame_secondary->Set_Page_Number(page_number_frame_primary);

			// Map the page in the secondary cache into its new address in the page table.
			Map_Page_Secondary(page_number_frame_primary, frame_index_secondary);

		} // Is the page frame in the primary cache NOT a free page

		// Set new page number in page frame in primary cache.
		p_frame_primary->Set_Page_Number(page_number);

		// Map this page into its new address in the page table.
		Map_Page(page_number, frame_index_primary);

		// Lock this page.
		*p_page_handle = p_frame_primary->Lock_Page_Not_Present(flags, m_p_stats);

		TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Open_Page CM_ERROR_CACHE_MISS, page = ", 
			(U32)page_number);

		return CM_ERROR_CACHE_MISS;

	} // frame_index_secondary == 0

	// The page IS in the secondary cache.
	p_frame_secondary = Get_Frame_Secondary(frame_index_secondary);

	// Unmap the page from its former address in the secondary cache.
	Unmap_Page_Secondary(page_number);
	p_frame_secondary->Set_Page_Number(-1);

	// Point to the page frame in the primary cache.
	p_page_frame_primary = Get_Page_Frame(frame_index_primary);

	// Point to the page frame in the secondary cache.
	p_page_frame_secondary = Get_Page_Frame_Secondary(frame_index_secondary);

	// Does the page frame in the primary cache have a mapped page?
	if (page_number_frame_primary != -1)
	{
		// The page frame in the primary cache does have a valid frame.
		// We want to exchange this with the page in the secondary cache.

		// Unmap this page from its former address in the primary cache.
		Unmap_Page(page_number_frame_primary);

		// Move the page from the secondary cache to a buffer.
		memcpy(m_p_page_buffer, p_page_frame_secondary, m_config.page_size);

		// Move the page from the primary cache to the secondary cache.
		memcpy(p_page_frame_secondary, p_page_frame_primary, m_config.page_size);

		// Move the page from the secondary cache to the primary cache.
		memcpy(p_page_frame_primary, m_p_page_buffer, m_config.page_size);

		// Set new page number in secondary page frame.
		p_frame_secondary->Set_Page_Number(page_number_frame_primary);

		// Map the page in the secondary cache into its new address in the page table.
		Map_Page_Secondary(page_number_frame_primary, frame_index_secondary);

	} // frame in primary cache has mapped page
	else
	{
		// The frame in the primary cache does not have a mapped page.
		// We want to move the page from the secondary cache to the empty
		// frame in the primary cache.
		memcpy(p_page_frame_primary, p_page_frame_secondary, m_config.page_size);
	}

	// Set new page number in page frame in primary cache.
	p_frame_primary->Set_Page_Number(page_number);

	// Map this page into its new address in the page table.
	Map_Page(page_number, frame_index_primary);

	return OK;

} // Open_Secondary