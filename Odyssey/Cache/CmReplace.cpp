/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmReplace.cpp
// 
// Description:
// This file implements the page replacement algorithm of the Cache Manager. 
// 
// Update Log 
// 
// 9/7/98 Jim Frandeen: Create file
// 12/2/98 Jim Frandeen: Fix for loop in Clock -- bug found by JFL.
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmCache.h"
#include "TraceMon.h"

/*************************************************************************/
// CM_Frame_Table::Replace_Page
// Find a page to replace. 
/*************************************************************************/
CM_Frame *CM_Frame_Table::Replace_Page(U32 flags,
									   Callback_Context *p_callback_context)
{
	TRACE_ENTRY(CM_Frame_Table::Replace_Page);

	CM_Frame* p_frame;

	// Check to see if this would exceed the maximum number of dirty pages
	if (CM_OPEN_MODE(flags) == CM_OPEN_MODE_WRITE)
	{
		if (Check_For_Max_Dirty_Pages() != OK)
            goto No_Frames_Available;
	}

	if ((flags & CM_PRIORITY_RESERVE) == 0)
	{
		// Check to be sure we have enough reserve pages
        if (m_num_pages_replaceable <= m_num_reserve_pages)
            goto No_Frames_Available;
	}

	// Get a page from the clock algorithm.
	p_frame = Clock();

	if (p_frame)
	{
		// We have allocated a page frame.
		CT_ASSERT(((m_p_clock_frame->Get_Page_State() == CM_PAGE_STATE_CLEAN)
			| (m_p_clock_frame->Get_Page_State() == CM_PAGE_STATE_WRITE)), CM_Frame_Table::Replace_Page);

		return p_frame;
	}

No_Frames_Available:

	TRACE_STRING(TRACE_L5, "No frames available\n\r");

	// There are no page frames available.
	// Does user want to wait for a page frame?
	if (flags & CM_CALLBACK_IF_NO_FRAME)
	{
		// Context will be scheduled again when we have a
		// replaceable page frame.
		LIST_INSERT_TAIL(&m_list_wait_frame, &p_callback_context->m_list); 
		m_p_stats->Inc_Num_Waits_For_Page_Frame();
		TRACE_STRING(TRACE_L5, "Context waiting for page frame\n\r");
	}
	return 0;

} // CM_Frame::Replace_Page

/*************************************************************************/
// CM_Frame_Table::Clock
// This is the clock replacement algorithm.  A user needs a new page frame, 
// so find a page that can be replaced.  Page frames are arranged in a 
// circular list (like the circumference of a clock).  The clock pointer 
// (or hand) points at the last page replaced and moves clockwise when the 
// algorithm is invoked to find the next replaceable page.  When a frame is 
// tested for replacement, the accessed bit in the corresponding frame 
// entry is tested and reset.  If the page has been referenced since the 
// last  test, the page is considered to be part of the current working set, 
// and the  pointer is advanced to the next frame.  If the page has not 
// been accessed, it is eligible for replacement.
/*************************************************************************/
CM_Frame *CM_Frame_Table::Clock()
{
	TRACE_ENTRY(CM_Frame_Table::Clock);

	// See if the clock is empty.
	// This would be the case if all frames were dirty or being written.
	if (m_num_pages_clock == 0)
		return 0;

	// If we have to use a frame that was accessed, use this one.
	CM_Frame *p_frame_accessed = 0;

	// Point to the first frame to test.
	CM_Frame *p_frame = m_p_clock_frame;

	// Test each frame in the clock plus the dummy frame
	// at the head of the list.
	for (U32 num_pages_tested = 0;

		// Test for the last time.
		num_pages_tested <= m_num_pages_clock;

		// Point to the next frame in the list each time around
		p_frame = (CM_Frame *)p_frame->m_list.forward_link,

		// Increment num_pages_tested each time around.
		num_pages_tested++)
	{
		// If we are pointing to the head of the list
		if (p_frame->Is_Dummy_Frame())
		{
			// Every time the clock hand goes all the way around, tell the
			// statistics object how many pages were either locked or referenced.
			m_p_stats->Set_Num_Pages_Working_Set(m_num_pages_working_set);
			m_num_pages_working_set = 0;
			continue;
		}

		CT_ASSERT(((m_p_clock_frame->Get_Page_State() == CM_PAGE_STATE_CLEAN)
			| (m_p_clock_frame->Get_Page_State() == CM_PAGE_STATE_WRITE)), CM_Frame_Table::Clock);

		// If this frame is locked, we can't replace it.
		if (p_frame->Is_Page_Locked())
		{
			m_num_pages_working_set++;
			continue;
		}

		// Check to see if this frame has been accessed since
		// the last time around the clock.
		if (!p_frame->Is_Page_Accessed())
			goto Found_Frame;

		// If this frame was accessed the last time, 
		// we will use it as a last resort.
		m_num_pages_working_set++;
		p_frame_accessed = p_frame;

	} // for

	// We went all the way around the clock and did not find a frame that
	// was not locked and not accessed.
	// Did we find a frame that was accessed and not locked?
	if ((p_frame = p_frame_accessed) == 0)

		// No frame was found.
		return 0;

Found_Frame:

	// Update the clock.
	m_p_clock_frame = (CM_Frame *)p_frame->m_list.forward_link;

	CT_ASSERT((p_frame->Is_Replaceable()), CM_Frame_Table::Clock);

	return p_frame;


} // CM_Frame_Table::Clock



