/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmClose.cpp
// 
// Description:
// This file implements the Close Page and Abort Page
// methods of the Cache Manager. Abort is similar to Close, but it indicates
// that the Open operation failed.
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "CmCache.h"
#include "CmContext.h"
#include "TraceMon.h"
#include <stdarg.h>

#if 1
#define VALIDATE \
Validate()
#else
#define VALIDATE
#endif

/*************************************************************************/
// CM_Cache::Abort_Page
// Abort a page.  
/*************************************************************************/
Status CM_Cache::Abort_Page(CM_Frame_Handle frame_handle)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Abort_Page, page handle = ", 
		frame_handle.Get_Page_Handle());

	// Get the frame index for this page.  
	U32 frame_index = frame_handle.Get_Frame_Index();
	if (frame_index == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
			"CM_Cache::Abort_Page", 
			"Invalid page handle",
			CM_ERROR_INVALID_PAGE_HANDLE,
			0);
		return CM_ERROR(INVALID_PAGE_HANDLE);
	}

	// Ask the frame table to abort its open page.
	m_p_frame_table->Close_Page((CM_CACHE_HANDLE)this, frame_handle,
			TRUE /* aborted */, this);

	return OK;

} // CM_Cache::Abort_Page

/*************************************************************************/
// CM_Cache::Close_Page
// Close a page 
/*************************************************************************/
Status CM_Cache::Close_Page(CM_Frame_Handle frame_handle)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Cache::Close_Page, page handle = ", 
		frame_handle.Get_Page_Handle());

	// Get the frame index for this page.  
	U32 frame_index = frame_handle.Get_Frame_Index();
	if (frame_index == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
			"CM_Cache::Close_Page", 
			"Invalid page handle",
			CM_ERROR_INVALID_PAGE_HANDLE,
			0);
		return CM_ERROR(INVALID_PAGE_HANDLE);
	}


	// Ask the frame table to close its page.
	m_p_frame_table->Close_Page((CM_CACHE_HANDLE)this, frame_handle,
		FALSE /* NOT aborted */, this);

	return OK;

} // CM_Cache::Close_Page

/*************************************************************************/
// CM_Frame_Table::Close_Page
// Close a page. page_aborted is TRUE if page is aborted. 
// The frame table needs to know if the state of the page changed.
/*************************************************************************/
void CM_Frame_Table::Close_Page(CM_CACHE_HANDLE cache_handle,
	CM_Frame_Handle frame_handle, BOOL page_aborted,
	CM_Cache *p_cache)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame_Table::Close_Page, page handle = ", 
		frame_handle.Get_Page_Handle());

	// Keep track of any contexts that are waiting for a frame to be closed.
	// If any contexts are waiting, they will be scheduled
	// at the end, when we have released the mutex.
	LIST	list_waiting_contexts;
	LIST_INITIALIZE(&list_waiting_contexts);
	UI64 page_number;

	Obtain_Mutex();

	if (page_aborted)

		// Increment the number of page aborts
		m_p_stats->Inc_Num_Abort_Page();

	else
		// Increment the number of page closes
		m_p_stats->Inc_Num_Close_Page();

	// Get the frame index for this page.
	// The frame index has already been validated.
	U32 frame_index = frame_handle.Get_Frame_Index();

	// Given the frame index, point to the frame for this page.
	CM_Frame *p_frame = Get_Frame(frame_index);

	// Have the frame close itself.
	// Returns contexts (if any) that were waiting for this frame to be unlocked.
	CM_STATE_CHANGE state_change = p_frame->Close_Page(frame_handle, m_p_stats,
		&list_waiting_contexts, page_aborted);

    // Did the state of the page change?
    switch (state_change)
	{
    case CM_JUST_BECAME_DIRTY:

		// The page was clean, and it just became dirty.
		CT_ASSERT((p_frame->Get_Page_State() == CM_PAGE_STATE_CLEAN), 
			CM_Frame_Table::Close_Page);
		p_frame->Set_Page_State(CM_PAGE_STATE_DIRTY);

		// Remove this frame from the clock list.
		// Link this frame back on the dirty clock list
		Move_Frame_To_Dirty_Clock(p_frame);

		VALIDATE;

		// Turn off the accessed bit for this page.  If the page is opened again,
		// chances are it won't be selected to be written.
		p_frame->Set_Page_Not_Accessed();

		// Tell the stats object how many dirty pages we have.
		m_p_stats->Set_Num_Pages_Dirty(m_num_pages_dirty_clock);

        break;

    case CM_JUST_BECAME_DIRTY_AGAIN:

		// The page just became dirty again after being cleaned.
		CT_ASSERT((p_frame->Get_Page_State() == CM_PAGE_STATE_WRITE), 
			CM_Frame_Table::Close_Page);
		p_frame->Set_Page_State(CM_PAGE_STATE_DIRTY);

		// Decrement the number of pages being written.
		CT_ASSERT((m_num_pages_being_written != 0), 
			CM_Frame_Table::Close_Page CM_JUST_BECAME_DIRTY_AGAIN);
		m_num_pages_being_written--;

		// Remove this frame from the clock list.
		// Link this frame back on the dirty clock list
		Move_Frame_To_Dirty_Clock(p_frame);

		// Tell the stats object how many dirty pages we have.
		m_p_stats->Set_Num_Pages_Dirty(m_num_pages_dirty_clock);
		m_p_stats->Inc_Num_Dirty_Again();

        break;

    case CM_JUST_BECAME_CLEAN_REPLACEABLE:

		// The page just became clean again after being written.
		// The page is not locked, so it is also replaceable.
		Inc_Replaceable_Pages(&list_waiting_contexts);

        // continue below

    case CM_JUST_BECAME_CLEAN:

		// The page just became clean again after being written.
		// However, the page is still locked, so it is not replaceable.
		p_frame->Set_Page_State(CM_PAGE_STATE_CLEAN);

		// Decrement the number of pages being written.
		CT_ASSERT((m_num_pages_being_written != 0), 
			CM_Frame_Table::Close_Page CM_JUST_BECAME_CLEAN);
		m_num_pages_being_written--;

        break;

	case CM_JUST_BECAME_NOT_PRESENT:

		// Unmap this page from its former address.
		page_number = p_frame->Get_Page_Number();
		p_cache->Unmap_Page(page_number);
		p_frame->Set_Page_Number(-1);
		CT_ASSERT(p_frame->Is_Page_Clean(), CM_Frame_Table::Close_Page);
		CT_ASSERT(!p_frame->Is_Page_Locked(), CM_Frame_Table::Close_Page);

		// Continue below.  

    case CM_JUST_BECAME_REPLACEABLE:

		// The state does not change here.  The state was clean before,
		// but the page was locked, so it was not replaceable.
		CT_ASSERT((p_frame->Is_Replaceable()), CM_Frame_Table::Close_Page);
		Inc_Replaceable_Pages(&list_waiting_contexts);
        break;

	case CM_STATE_NO_CHANGE:
		break;

	default:
		CT_ASSERT((0), CM_Frame_Table::Close_Page); // Internal error
       break;

	} // switch

	// Decrement the number of open pages.   
	m_p_stats->Dec_Num_Pages_Open();

	// Was this page closed for writeback or remap?
	U32 open_mode = frame_handle.Get_Open_Mode();
	if ((open_mode == CM_OPEN_MODE_WRITE_BACK) || (open_mode == CM_OPEN_MODE_REMAP))
	{
		// See if there are any contexts waiting.
		// This happens when we are waiting for a flush operation.
		if (!LIST_IS_EMPTY(&m_list_waiting_to_flush))
		{
			CM_Cache_Context *p_context_waiting_to_flush = 
				(CM_Cache_Context *)LIST_REMOVE_TAIL(&m_list_waiting_to_flush);
			m_p_stats->Dec_Num_Flush_Waits_For_Lock();

			LIST_INSERT_TAIL(&list_waiting_contexts, &p_context_waiting_to_flush->m_list);
		}
	}

    // Check to see if the number of dirty pages has exceeded the writeback threshold.
    if (m_num_pages_dirty_clock > m_dirty_page_writeback_threshold)
    {
		m_p_stats->Inc_Dirty_Page_Threshold();
        Write_Dirty_Page(cache_handle);
    }

	Release_Mutex();

	// Make ready any contexts that were waiting for this page to be closed.
	while (!LIST_IS_EMPTY(&list_waiting_contexts))
	{
		Callback_Context *p_waiting_context = 
			(Callback_Context *)LIST_REMOVE_TAIL(&list_waiting_contexts);
		p_waiting_context->Make_Ready();
	}

} // CM_Frame_Table::Close_Page

/*************************************************************************/
// CM_Frame::Close_Page
// Close a page.  Return state change.
// Return pointer to context (if any) waiting for this page.
// page_aborted is TRUE if page is being aborted.
/*************************************************************************/
CM_STATE_CHANGE CM_Frame::Close_Page(CM_Frame_Handle frame_handle, CM_Stats *p_stats,
	LIST *p_list_waiting_contexts, BOOL page_aborted)
{
	// Set the accessed flag for the clock.
	Set_Page_Accessed();

	// Remember if this page was already dirty.
	char page_previously_dirty = Is_Page_Dirty();

	switch (frame_handle.Get_Open_Mode())
	{
	case CM_OPEN_MODE_READ:

		// A page opened for read is locked read and closed clean.
		// Return any contexts waiting for this page.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode read, page = ", (U32)m_page_number);
		Unlock_Page_Read(p_stats, p_list_waiting_contexts);

		// Switch on the state of the page before we closed it.
		switch (m_page_state)
		{
		case CM_PAGE_STATE_CLEAN:

			CT_ASSERT((Is_Page_Clean()), CM_Frame::Close_Page);
			if (!Is_Page_Locked())

				// The state was clean before, but it was locked.
				return CM_JUST_BECAME_REPLACEABLE;
			break;

		case CM_PAGE_STATE_DIRTY:

			CT_ASSERT((!Is_Page_Clean()), CM_Frame::Close_Page);
			break;
		
		case CM_PAGE_STATE_WRITE:

			CT_ASSERT((Is_Page_Clean()), CM_Frame::Close_Page);
			break;

		default:

			CT_ASSERT((0), CM_Frame_Table::Close_Page); // Internal error
		}

        return CM_STATE_NO_CHANGE;

	case CM_OPEN_MODE_READ_NOT_PRESENT:

		// A page opened for read when the page is not present
		// is locked write and closed clean.
		// Return any contexts waiting for this page.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode read not present, page = ", (U32)m_page_number);
		Unlock_Page_Read_Not_Present(p_stats, p_list_waiting_contexts);

		if (page_aborted)

			// The user was not able to make the page present.
			// The page gets unmapped, unlocked, and made replaceable.
			return CM_JUST_BECAME_NOT_PRESENT;

		// The page could be closed dirty if the user called Set_Page_Dirty.
		// This occurs when an ECC error is corrected.
		if (page_previously_dirty)
			return CM_JUST_BECAME_DIRTY;

		// The page just became replaceable.
        return CM_JUST_BECAME_REPLACEABLE;

	case CM_OPEN_MODE_WRITE_BACK:

		// The page was being written from the cache to backing store.
		// We turned off the dirty flag before the write was started.
		// The dirty flag could have been turned on again.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode writeback, page = ", (U32)m_page_number);
		CT_ASSERT((m_page_state == CM_PAGE_STATE_WRITE), CM_Frame::Close_Page);
		Unlock_Page_Writeback(p_stats, p_list_waiting_contexts);

		if (page_aborted)
		{
			// The page was never cleaned, so mark it dirty again.
			Set_Page_Dirty();
			return CM_JUST_BECAME_DIRTY_AGAIN;
		}

		// See if the page is still clean
        if (Is_Page_Clean())
		{
			if (!Is_Page_Locked())
				return CM_JUST_BECAME_CLEAN_REPLACEABLE;
			return CM_JUST_BECAME_CLEAN;
		}

		// Page is no longer clean.  It became dirty again while we were writing it.
        return CM_JUST_BECAME_DIRTY_AGAIN;

	case CM_OPEN_MODE_WRITE_THROUGH_NOT_PRESENT:

		// We distinguish from CM_OPEN_MODE_WRITE_THROUGH only in the case of abort.
		// If the page was not present when it was opened, it must 
		// be marked not present again.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode write through not present, page = ", (U32)m_page_number);
		if (page_aborted)
		{
			Unlock_Page_Write(p_stats, p_list_waiting_contexts);
			return CM_JUST_BECAME_NOT_PRESENT;
		}

		// Continue below.

	case CM_OPEN_MODE_WRITE_THROUGH:

		// The page was opened for write, and the cache is write through.
		// This means the user has written the page to the backing store.
        // A page opened for write is locked write and closed dirty.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode write, page = ", (U32)m_page_number);
		Unlock_Page_Write(p_stats, p_list_waiting_contexts);
		return CM_JUST_BECAME_REPLACEABLE;

	case CM_OPEN_MODE_WRITE_NOT_PRESENT:

		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode write not present, page = ", (U32)m_page_number);
		if (page_aborted)
		{
			// We distinguish from CM_OPEN_MODE_WRITE only in the case of abort.
			// If the page was not present when it was opened, it must 
			// be marked not present again.
			// The page also gets marked replaceable.
			Unlock_Page_Write(p_stats, p_list_waiting_contexts);
			return CM_JUST_BECAME_NOT_PRESENT;
		}

		// Continue below.

	case CM_OPEN_MODE_WRITE:

        // A page opened for write is locked write and closed dirty.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode write, page = ", (U32)m_page_number);
		Unlock_Page_Write(p_stats, p_list_waiting_contexts);

		// Page is dirty now.
		// Return any contexts waiting for this page.
		Set_Page_Dirty();

		switch (m_page_state)
		{
		case CM_PAGE_STATE_CLEAN:

			CT_ASSERT((!page_previously_dirty), CM_Frame::Close_Page);
			return CM_JUST_BECAME_DIRTY;

		case CM_PAGE_STATE_DIRTY:

			return CM_STATE_NO_CHANGE;
		
		case CM_PAGE_STATE_WRITE:

			return CM_STATE_NO_CHANGE;

		default:
			CT_ASSERT((0), CM_Frame_Table::Close_Page); // Internal error
		}

		break;

	case CM_OPEN_MODE_REMAP:

		// A page opened for remap is locked remap and closed dirty
		// Return any contexts waiting for this page.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode remap, page = ", (U32)m_page_number);
		Unlock_Page_Remap(p_stats, p_list_waiting_contexts);

		if (page_aborted)
		{
			// Page not marked dirty because the remap was aborted.

			if (page_previously_dirty || Is_Page_Locked() )
				return CM_STATE_NO_CHANGE;
			return CM_JUST_BECAME_REPLACEABLE;
		}

		// Page is dirty now.
		Set_Page_Dirty();

		switch (m_page_state)
		{
		case CM_PAGE_STATE_CLEAN:

			CT_ASSERT((!page_previously_dirty), CM_Frame::Close_Page);
			return CM_JUST_BECAME_DIRTY;

		case CM_PAGE_STATE_DIRTY:

			CT_ASSERT((page_previously_dirty), CM_Frame::Close_Page);
			return CM_STATE_NO_CHANGE;
		
		case CM_PAGE_STATE_WRITE:

			return CM_STATE_NO_CHANGE;

		default:

			CT_ASSERT((0), CM_Frame_Table::Close_Page); // Internal error
		}
        break;

	case CM_OPEN_MODE_REMAP_NOT_PRESENT:

		// A page opened for remap when the page is not present
		// is locked write and closed dirty.
		// Return any contexts waiting for this page.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Close_Page mode remap not present, page = ", (U32)m_page_number);
		CT_ASSERT((m_page_state == CM_PAGE_STATE_CLEAN), CM_Frame::Close_Page);

		if (page_aborted)
		{
			Unlock_Page_Write(p_stats, p_list_waiting_contexts);
			return CM_JUST_BECAME_NOT_PRESENT;
		}

		Set_Page_Dirty();

		Unlock_Page_Write(p_stats, p_list_waiting_contexts);

		// Page just became dirty.
		return CM_JUST_BECAME_DIRTY;

	default:
		CT_ASSERT((0), CM_Frame_Table::Close_Page); // Internal error
		break;

	} // switch

	return CM_STATE_NO_CHANGE;

} // CM_Frame::Close_Page

/*************************************************************************/
// CM_Cache::Release_Resources
// Release all cache resources. 
/*************************************************************************/
void CM_Cache::Release_Resources()
{
	m_p_frame_table->Release_Resources();

	// Invalidate cache handle.
	m_cookie = 0;

} // CM_Cache::Release_Resources

/*************************************************************************/
// CM_Frame_Table::Release_Resources
// Release all cache resources. 
/*************************************************************************/
void CM_Frame_Table::Release_Resources()
{

} // CM_Frame_Table::Release_Resources
