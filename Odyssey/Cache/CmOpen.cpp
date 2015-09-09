/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmOpen.cpp
// 
// Description:
// This file implements the Open method of the Cache Manager. 
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 12/9/98 Jim Frandeen: Check to see if a page has been mapped before we
//		unmap it -- otherwise we unmap page 0.
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmCache.h"
#include "TraceMon.h"

/*************************************************************************/
// CM_Cache::Open_Page
// Open a page 
// Return OK if page can be locked.
// Return CM_ERROR_PAGE_LOCKED if page locked.
// Return CM_ERROR_CACHE_MISS and page frame if page is not in cache.
// Return CM_ERROR_NO_PAGE_FRAMES if cache miss and no frames are available.
// Return CM_ERROR_MAX_DIRTY_PAGES if too many dirty pages in the cache.
/*************************************************************************/
Status CM_Cache::Open_Page( 
	I64 page_number,
	U32 flags,
	Callback_Context *p_callback_context,
	void **pp_page_frame,
	CM_PAGE_HANDLE *p_page_handle)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Open_Page, page = ", (U32)page_number);

    if (m_config.page_table_size)
	{
		// Linear mapping
		// Validate the page number.
		if (page_number >= m_config.page_table_size)
			return CM_ERROR(INVALID_PAGE_NUMBER);
	}

	switch (CM_OPEN_MODE(flags))
	{
	case CM_OPEN_MODE_WRITE:

		m_p_stats->Inc_Num_Open_Write();
		
		// Is this a write through cache?
		if (m_is_write_back == 0)
		{
			// For a write through cache, change open mode to write through.
			CM_SET_OPEN_MODE(flags, CM_OPEN_MODE_WRITE_THROUGH);
		}
		break;

	case CM_OPEN_MODE_READ:

		m_p_stats->Inc_Num_Open_Read();
		break;

	case CM_OPEN_MODE_REMAP	:

		m_p_stats->Inc_Num_Open_Remap();
		break;

	default:

		return CM_ERROR(INVALID_MODE);
	}

	Obtain_Mutex();

	// Get the frame index for this page if it is present.  
	U32 frame_index = Get_Frame_Index(page_number);
	if (frame_index == 0)
	{
		// The page is not present.
		Status status = CM_ERROR_CACHE_MISS;

		// Replace some other page in the cache.
		CM_Frame *p_frame = Replace_Page(flags, p_callback_context);
		if (p_frame == 0)
		{
			// The page could not be replaced.
			m_p_stats->Inc_Error_No_Page_Frame();

			TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Open_Page CM_ERROR_NO_PAGE_FRAMES, page = ", 0);
			Release_Mutex();

			// If CM_CALLBACK_IF_NO_FRAME was specified,
			// Context will be scheduled again when we have a
			// replaceable page frame.
			return CM_ERROR(NO_PAGE_FRAMES);
		}

		// We have a pointer to a frame for an empty page frame.
		// Increment the number of open pages.   
		m_p_stats->Inc_Num_Pages_Open();

		// Get index of this page.
		frame_index = p_frame->Get_Frame_Index();

		// Should we look for this page in the secondary cache?
		if ((flags & CM_SECONDARY) && m_config.num_pages_secondary)
			status = Open_Secondary(page_number, flags, p_page_handle, p_frame);
		else
		{
			// Unmap this page from its former address if it has been previously mapped.
			if (p_frame->Get_Page_Number() != -1)
				Unmap_Page(p_frame->Get_Page_Number());

			// Set new page number in page frame.
			p_frame->Set_Page_Number(page_number);

			// Map this page into its new address in the page table.
			Map_Page(page_number, frame_index);

			// Lock this page.
			*p_page_handle = p_frame->Lock_Page_Not_Present(flags, m_p_stats);

			TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Open_Page CM_ERROR_CACHE_MISS, page = ", (U32)page_number);
		}

		// See if Open_Secondary got the page.
		if (status == CM_ERROR_CACHE_MISS)
		{
			// Increment the number of cache misses
			m_p_stats->Inc_Num_Cache_Misses();

			// Update the number of replaceable pages in the clock.
			Dec_Replaceable_Pages();

			// Return pointer to page frame.
			*pp_page_frame = Get_Page_Frame(frame_index);

			Release_Mutex();

			return status;
		}

		// Continue if Open_Secondary moved the page from the secondary cache.
		frame_index = Get_Frame_Index(page_number);
		CT_ASSERT(frame_index, CM_Cache::Open_Page);
	}

	// This page is present in the cache.
	// Increment the number of cache hits.
	m_p_stats->Inc_Num_Cache_Hits();

	Status status = m_p_frame_table->Open_Page(frame_index, flags, p_callback_context,
           pp_page_frame, p_page_handle);
	if (status == OK)

		// Increment the number of open pages.   
		m_p_stats->Inc_Num_Pages_Open();

	Release_Mutex();
	return status;

} // CM_Cache::Open_Page

/*************************************************************************/
// CM_Frame_Table::Open_Page
// Open a page that we know is in the cache.
// If the state of the page changes, update our counters.
/*************************************************************************/
Status CM_Frame_Table::Open_Page( 
	U32 frame_index,
	U32 flags,
	Callback_Context *p_callback_context,
	void **pp_page_frame,
	CM_PAGE_HANDLE *p_page_handle)
{
	TRACE_ENTRY(CM_Frame_Table::Open_Page);

	// This page is present in the cache.
	// Given the index, point to the frame for this page.
	CM_Frame *p_frame = Get_Frame(frame_index);
	CT_ASSERT((p_frame->Get_Page_Number() != -1), CM_Frame_Table::Open_Page);

	// Check to see if this would exceed the maximum number of dirty pages.
	if (CM_OPEN_MODE(flags) == CM_OPEN_MODE_WRITE)
	{
		// Is the frame already dirty?
		if (!p_frame->Is_Page_Dirty())
		{
			// The page frame is clean now.
			if (Check_For_Max_Dirty_Pages() != OK)
			{
				// There are too many dirty pages.
				// Does user want to wait for a page frame?
				if (flags & CM_CALLBACK_IF_NO_FRAME)
				{
					// Context will be scheduled again when we have a
					// replaceable page frame.
					LIST_INSERT_TAIL(&m_list_wait_frame, &p_callback_context->m_list); 
					m_p_stats->Inc_Num_Waits_For_Page_Frame();
				}
				TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame_Table::Open_Page CM_ERROR_MAX_DIRTY_PAGES, page = ", 0);
				return CM_ERROR(MAX_DIRTY_PAGES);
			}
		}
	}

	// Ask the frame to open itself.
    // Get back any change in page state.
    CM_STATE_CHANGE state_change;
	Status status = p_frame->Open_Page(flags, p_callback_context, 
        p_page_handle, &state_change, m_p_stats);

    switch (state_change)
    {
    case CM_JUST_BECAME_UNREPLACEABLE:

        // Update the number of replaceable pages in the clock.
		Dec_Replaceable_Pages();
        break;

    default:
        break;
    }

	if (status == OK)
	{
		// Return pointer to page frame
		*pp_page_frame = Get_Page_Frame(frame_index);
		return OK;
	}

	// Return CM_ERROR_PAGE_LOCKED if page locked; context is queued
	// if user specified CM_CALLBACK_IF_LOCKED.
	if (status == CM_ERROR_PAGE_LOCKED)
		m_p_stats->Inc_Num_Error_Page_Locked();

	// Return null pointer to page frame
	*pp_page_frame = 0;
	return status;

} // CM_Frame_Table::Open_Page


/*************************************************************************/
// CM_Frame::Open_Page
// Return OK if page can be locked.
// Return CM_ERROR_PAGE_LOCKED if page locked; context is queued
// if user specified CM_CALLBACK_IF_LOCKED.
// Return CM_ERROR_CACHE_MISS and page frame if page is not in cache.
/*************************************************************************/
Status CM_Frame::Open_Page(
	U32 flags,
	Callback_Context *p_callback_context,	
	CM_PAGE_HANDLE *p_page_handle,
    CM_STATE_CHANGE *p_state_change,
	CM_Stats *p_stats)
{
    Status status = OK;

    // Determine page state before open
    int previous_lock = Is_Page_Locked();

	switch (CM_OPEN_MODE(flags))
	{
	case CM_OPEN_MODE_REMAP:

		// A page opened for remap is locked remap and closed dirty
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode remap, page = ", (U32)m_page_number);

		// See if page is locked for write.
		if ((m_write_lock == 0) && (m_writeback_lock == 0))
		{
			// The page has no write locks, so we can open it for remap.
			if (m_num_remap_locks == 0)

				// This is the first time the page is locked for remap.
				// Tell the stats record how many pages are locked for remap.
				p_stats->Inc_Num_Pages_Locked_Remap();

			// Lock page for remap
			m_num_remap_locks++;

			// Return page handle
			*p_page_handle = CM_Frame_Handle::Create_Page_Handle(flags, Get_Frame_Index());
			goto Return_State_Change;
		}

		// The page is locked for write or writeback.
		// Did the user specify CM_CALLBACK_IF_LOCKED?
		if (flags & CM_CALLBACK_IF_LOCKED)
		{
			// Queue this context.
			LIST_INSERT_TAIL(&m_list_wait_for_unlock, &p_callback_context->m_list);
			if (m_write_lock)
				p_stats->Inc_Num_Waits_For_Write_Lock();
			else
				p_stats->Inc_Num_Waits_For_Writeback_Lock();
		}

		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode remap CM_ERROR_PAGE_LOCKED, page = ", (U32)m_page_number);
		status = CM_ERROR_PAGE_LOCKED;
        goto Return_State_Change;

	case CM_OPEN_MODE_READ:
		// A page opened for read is locked read and closed clean.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode read, page = ", (U32)m_page_number);

		// See if page is locked for write.
		// It's OK if the page is locked for remap.
		if (m_write_lock == 0)
		{
			if (m_num_read_locks == 0)

				// This is the first time the page is locked.
				// Tell the stats record how many pages are locked for read.
				p_stats->Inc_Num_Pages_Locked_Read();

			// Lock page for read
			m_num_read_locks++;

			// Return page handle
			*p_page_handle = CM_Frame_Handle::Create_Page_Handle(flags, Get_Frame_Index());
			goto Return_State_Change;
		}

		// The page is locked for write.
		// Did the user specify CM_CALLBACK_IF_LOCKED?
		if (flags & CM_CALLBACK_IF_LOCKED)
		{
			// Queue this context.
			LIST_INSERT_TAIL(&m_list_wait_for_unlock, &p_callback_context->m_list);
			p_stats->Inc_Num_Waits_For_Write_Lock();
		}

		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode read CM_ERROR_PAGE_LOCKED, page = ", (U32)m_page_number);
		status = CM_ERROR_PAGE_LOCKED;
        goto Return_State_Change;

	case CM_OPEN_MODE_WRITE:
	case CM_OPEN_MODE_WRITE_THROUGH:
		// A page opened for write is locked write and closed dirty
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode write, page = ", (U32)m_page_number);

		if ((m_write_lock == 0) && (m_num_remap_locks == 0) && (m_num_read_locks == 0)

			// Note that the page must not be locked for write if the page is already
			// locked for writeback.  If the page is being written, a verify may also be
			// required.  The verify will fail if the page is overwritten before it
			// is verified.
			&& (m_writeback_lock == 0) )
		{
			// The page is not locked.
			// Lock the page for write
			m_write_lock = 1;

			// Tell the stats record how many pages are locked for write.
			p_stats->Inc_Num_Pages_Locked_Write();

			// Return page handle
			*p_page_handle = CM_Frame_Handle::Create_Page_Handle(flags, Get_Frame_Index());
			goto Return_State_Change;
		}

		// The page is locked.
		// Did the user specify CM_CALLBACK_IF_LOCKED?
		if (flags & CM_CALLBACK_IF_LOCKED)
		{
			// Queue this context.
			LIST_INSERT_TAIL(&m_list_wait_for_unlock, &p_callback_context->m_list);
			if (m_write_lock)
				p_stats->Inc_Num_Waits_For_Write_Lock();
			else if (m_num_read_locks)
				p_stats->Inc_Num_Waits_For_Read_Lock();
			else
				p_stats->Inc_Num_Waits_For_Writeback_Lock();
		}

		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Open_Page mode write CM_ERROR_PAGE_LOCKED, page = ", (U32)m_page_number);
		status = CM_ERROR_PAGE_LOCKED;
        goto Return_State_Change;

	default:
        status = CM_ERROR(INVALID_MODE);
		break;
	} // switch

Return_State_Change:

    if (previous_lock || Is_Page_Dirty())

        // The page was previously locked or dirty.
        *p_state_change = CM_STATE_NO_CHANGE;

    else 

        // The page was not previously locked or dirty, and we just
        // opened it, so the page just became unreplaceable.
        *p_state_change = CM_JUST_BECAME_UNREPLACEABLE;
    
	return status;

} // CM_Frame::Open_Page


