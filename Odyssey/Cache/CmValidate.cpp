/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmValidate.cpp
// 
// Description:
// This file implements validation procedures used for debugging.
// 
// Update Log 
// 
// 09/06/99 Jim Frandeen: Create file
/*************************************************************************/

#include "CmCache.h"

/*************************************************************************/
// void CM_Cache::Validate
/*************************************************************************/
void CM_Cache::Validate()
{
	// Only validate if simulating -- too time-consuming otherwise.
#ifdef SIM
	m_p_frame_table->Validate();

	if (m_p_frame_table_2)
		m_p_frame_table_2->Validate();

	// Validate page numbers
	CM_Frame	*p_frame;
	U32 index;
	U32 num_page_frames = m_p_frame_table->Num_Page_Frames();
	for (index = 0; index < num_page_frames; index++)
	{
		p_frame = Get_Frame(index + 1); // frame indexes start with 1

		U32 frame_index;
		U32 frame_index1;
		frame_index = p_frame->Get_Frame_Index();

		// This page number must be present.
		CT_ASSERT(frame_index == (index + 1), Validate);

		I64	page_number = p_frame->Get_Page_Number();
		if (page_number != -1)
		{
			frame_index1 = Get_Frame_Index(page_number);
			CT_ASSERT((frame_index1 == frame_index), Validate);
		}
		else
		{
			CT_ASSERT((p_frame->Is_Page_Clean()), Validate);
			CT_ASSERT((!p_frame->Is_Page_Locked()), Validate);
		}
	}
#endif // SIM
}

/*************************************************************************/
// CM_Frame_Table::Validate
/*************************************************************************/
void CM_Frame_Table::Validate()
{
	Validate_Clock();
	Validate_Dirty_Clock();
	Validate_Frames();
}

/*************************************************************************/
// CM_Frame_Table::Validate_Frames
/*************************************************************************/
void CM_Frame_Table::Validate_Frames()
{
	CM_Frame	*p_frame;
	CM_Frame	*p_frame1;
	I64			 page_number;

	// Look at every frame
	U32 index;
	U32 index1;
	for (index = 0; index < m_num_page_frames; index++)
	{
		p_frame = m_p_frame_array + index;

		page_number = p_frame->Get_Page_Number();
		if (page_number != -1)
		{

			// Make sure no other frame has the same address
			for (index1 = 0; index1 < m_num_page_frames; index1++)
			{
				if (index != index1)
				{
					p_frame1 = m_p_frame_array + index1;
					CT_ASSERT((p_frame->Get_Page_Number() != 
						p_frame1->Get_Page_Number()), Validate_Frames);
				}
			}
		}
	}

} // Validate_Frames

/*************************************************************************/
// CM_Frame_Table::Validate_Clock
/*************************************************************************/
void CM_Frame_Table::Validate_Clock()
{
	if (m_num_pages_clock == 0)
		return;

	// Count the number of frames in the clock.
	U32 num_pages_clock = 0;

	// Count the number of replaceable frames in the clock.
	U32 num_pages_replaceable = 0;

	// Point to the first frame in the clock.
	CM_Frame *p_first_frame = m_p_clock_frame;

	// Point to the first frame to test.
	CM_Frame *p_frame = m_p_clock_frame;

	while(true)

	{
		// If we are pointing to the head of the list
		if (!p_frame->Is_Dummy_Frame())
		{
			num_pages_clock++;

			// If this frame is locked, we can't replace it.
			if (!p_frame->Is_Page_Locked())
				num_pages_replaceable++;

		} // NOT dummy frame

		// Point to the next frame in the list each time around
		p_frame = (CM_Frame *)p_frame->m_list.forward_link;

		// Are we back at the beginning?
		if (p_frame == p_first_frame)
		{
			CT_ASSERT((num_pages_clock == m_num_pages_clock), Validate_Clock);
			CT_ASSERT((num_pages_replaceable == m_num_pages_replaceable), Validate_Clock);
			return;
		}
	}


} // Validate_Clock

/*************************************************************************/
// CM_Frame_Table::Validate_Dirty_Clock
/*************************************************************************/
void CM_Frame_Table::Validate_Dirty_Clock()
{
	if (m_num_pages_dirty_clock == 0)
		return;

	// Count the number of frames in the clock.
	U32 num_pages_clock = 0;

	// Point to the first frame in the clock.
	CM_Frame *p_first_frame = m_p_dirty_clock_frame;

	// Point to the first frame to test.
	CM_Frame *p_frame = m_p_dirty_clock_frame;

	while(true)

	{
		if (!p_frame->Is_Dummy_Frame())
			num_pages_clock++;

		// Point to the next frame in the list each time around
		p_frame = (CM_Frame *)p_frame->m_list.forward_link;

		// Are we back at the beginning?
		if (p_frame == p_first_frame)
		{
			CT_ASSERT((num_pages_clock == m_num_pages_dirty_clock), Validate_Dirty_Clock);
			return;
		}
	}


} // Validate_Dirty_Clock

