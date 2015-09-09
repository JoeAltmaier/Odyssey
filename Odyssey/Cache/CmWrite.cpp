/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmWrite.cpp
// 
// Description:
// This file implements the page write algorithm of the Cache Manager. 
// 
// Update Log 
// 
// 9/8/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmCache.h"
#include "TraceMon.h"

/*************************************************************************/
// CM_Frame_Table::Write_Dirty_Page
// Find a page to write. We get called by Flush, and by Close
// when the number of dirty pages has exceeded the writeback threshold.
/*************************************************************************/
Status CM_Frame_Table::Write_Dirty_Page(CM_CACHE_HANDLE cache_handle)
{
	TRACE_ENTRY(CM_Frame_Table::Write_Dirty_Page);

	// Get a dirty page from the dirty clock list.
	CM_Frame* p_frame = Write_Clock();

	// If no dirty page could be selected to be written, return.
	// This only happens if only pages locked for remap are in the dirty clock.
	if (p_frame == 0)
		return CM_ERROR(ALL_PAGES_LOCKED);

    // We found a page to write.
	// It must not be locked for remap because the page is being erased.
	CT_ASSERT((p_frame->Is_Page_Remap_Locked() == 0), CM_Frame_Table::Write_Dirty_Page);

	// Lock page for writeback and create a page handle.
	CM_PAGE_HANDLE page_handle = p_frame->Lock_Page_Writeback(m_p_stats);
	CT_ASSERT((p_frame->Is_Page_Locked()), CM_Frame_Table::Write_Dirty_Page);

	// Remove this frame from the dirty clock list.
	// Link this frame on the clock list
	Move_Frame_To_Clock(p_frame);

	// Increment the number of pages being written.
	m_num_pages_being_written++;

	// Tell the stats object how many dirty pages we have.
	m_p_stats->Set_Num_Pages_Dirty(m_num_pages_dirty_clock);

	p_frame->Set_Page_State(CM_PAGE_STATE_WRITE);

	// Set the page clean before we begin the write.
	// It could become dirty again during the write.
	// The user will call Close when the write operation has finished.
	p_frame->Set_Page_Clean();

	// Increment the number of open pages.   
	m_p_stats->Inc_Num_Pages_Open();

	// Get parameters for callback before we release the mutex.
	I64 page_number = p_frame->Get_Page_Number();
	void *p_page_frame = Get_Page_Frame(p_frame->Get_Frame_Index());
	CT_ASSERT((p_frame->Is_Page_Locked()), 
		CM_Frame_Table::Write_Dirty_Page);

	// Release the mutex before calling the user's callback.
	Release_Mutex();

    // Call the user's write callback.
	TRACE_NUMBER_EOL(TRACE_L5, "Calling write callback for page ", (U32)p_frame->Get_Page_Number());

	CT_ASSERT((p_frame->Is_Page_Locked()), CM_Frame_Table::Write_Dirty_Page);

	Status status = m_p_write_callback(
        m_p_callback_context,
        p_frame->Get_Page_Number(),
		Get_Page_Frame(p_frame->Get_Frame_Index()), 
        page_handle);

	// Obtain the mutex again.
	Obtain_Mutex();

	// Increment number of write callbacks
	m_p_stats->Inc_Num_Write_Callback();

	// Was the write operation started?
	if (status != OK)
	{
		TRACE_NUMBER_EOL(TRACE_L5, "Write callback status = ", status);

		// Just to be sure we are dealing with the same page across the mutex...
		// Note that the page may have been unlocked.
		CT_ASSERT((p_frame->Get_Page_Number() == page_number), 
			CM_Frame_Table::Write_Dirty_Page);
		CT_ASSERT((Get_Page_Frame(p_frame->Get_Frame_Index()) == p_page_frame), 
			CM_Frame_Table::Write_Dirty_Page);

		// Increment number of write callbacks that failed.
		m_p_stats->Inc_Num_Write_Callback_Failed();

		// Decrement the number of pages being written.
		CT_ASSERT((m_num_pages_being_written != 0), CM_Frame_Table::Write_Dirty_Page);
		m_num_pages_being_written--;

		// When we released the mutex, another context may have made the page dirty again.
		// If this is so, the page is already marked dirty and back on the dirty list.
		if (p_frame->Get_Page_State() != CM_PAGE_STATE_DIRTY)
		{

			// Remove this frame from the clock list.
			// Link this frame back on the dirty clock list
			Move_Frame_To_Dirty_Clock(p_frame);

			// Tell the stats object how many dirty pages we have.
			m_p_stats->Set_Num_Pages_Dirty(m_num_pages_dirty_clock);

			// Set the page dirty again.
			p_frame->Set_Page_State(CM_PAGE_STATE_DIRTY);
			p_frame->Set_Page_Dirty();
		}

		// Decrement the number of open pages.   
		m_p_stats->Dec_Num_Pages_Open();

		// Keep track of any contexts that are waiting for a frame to be closed.
		LIST	list_waiting_contexts;
		LIST_INITIALIZE(&list_waiting_contexts);

		p_frame->Unlock_Page_Writeback(m_p_stats, &list_waiting_contexts);

		// Make ready any contexts that were waiting for this page to be closed.
		while (!LIST_IS_EMPTY(&list_waiting_contexts))
		{
			Callback_Context *p_waiting_context = 
				(Callback_Context *)LIST_REMOVE_TAIL(&list_waiting_contexts);
			p_waiting_context->Make_Ready();
		}
	}

	return status;

} // CM_Frame_Table::Write_Dirty_Page

/*************************************************************************/
// CM_Frame_Table::Write_Clock
// This is similar to the clock replacement algorithm.  
// We are searching for a dirty page to write.
/*************************************************************************/
CM_Frame *CM_Frame_Table::Write_Clock()
{
	// See if the clock is empty.
	if (m_num_pages_dirty_clock == 0)
		return 0;

	// If we have to use a frame that was accessed, use one of these.
	CM_Frame *p_frame_accessed = 0;
	CM_Frame *p_frame_locked = 0;
	CM_Frame *p_frame_write_locked = 0;

	// Point to the first frame to test.
	CM_Frame *p_frame = m_p_dirty_clock_frame;

	// Test each frame in the clock plus the dummy frame
	// at the head of the list.
	for (U32 num_pages_tested = 0;

		// Test for the last time.
		num_pages_tested <= m_num_pages_dirty_clock;

		// Point to the next frame in the list each time around
		p_frame = (CM_Frame *)p_frame->m_list.forward_link,

		// Increment num_pages_tested each time around.
		num_pages_tested++)
	{
		// If we are pointing to the head of the list
		if (p_frame->Is_Dummy_Frame())
			continue;

		CT_ASSERT((p_frame->Get_Page_State() == CM_PAGE_STATE_DIRTY), CM_Frame_Table::Write_Clock);

		// If this frame is locked, we prefer not to choose it.
		if (p_frame->Is_Page_Locked())
        {
			if (p_frame->Is_Page_Remap_Locked())

				// If the page is locked for remap, we must not try to write it.
				// This page will be mapped to a different real page.
				continue;
			if (p_frame->Is_Page_Write_Locked())

				// No sense writing a page that will be dirty again
				// before we finish writing it.
				p_frame_write_locked = p_frame;
			else
				// Save pointer to frame that was locked in case we need it.
				p_frame_locked = p_frame;
			continue;
        }

		// Check to see if this frame has been accessed since
		// the last time around the clock.
		if (!p_frame->Is_Page_Accessed())
			goto Found_Frame;

		// If this frame was accessed the last time, 
		// we will use it if necessary.
		p_frame_accessed = p_frame;
	}

	// We went all the way around the clock and did not find a frame that
	// was not locked and not accessed.
	// Did we find a frame that was accessed?
	if ((p_frame = p_frame_accessed) == 0)

		// We did not find a frame that was accessed but not locked.
		// Did we find a frame that was locked?
		if ((p_frame = p_frame_locked) == 0)

			// Did we find a frame that was write locked?
			// This would be our last resort.
			if ((p_frame = p_frame_write_locked) == 0)

				// No frame was found.
				return 0;

Found_Frame:

	// Update the clock.
	m_p_dirty_clock_frame = (CM_Frame *)p_frame->m_list.forward_link;
	CT_ASSERT((m_p_dirty_clock_frame->Get_Page_State() == CM_PAGE_STATE_DIRTY), 
		CM_Frame_Table::Write_Clock);

	return p_frame;

} // CM_Frame_Table::Write_Clock



