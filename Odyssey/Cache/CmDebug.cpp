/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmDebug.cpp
// 
// Description:
// This file implements procedures used for debugging.
// 
// Update Log 
// 
// 07/27/99 Jim Frandeen: Create file
/*************************************************************************/

#include "CmCache.h"
#include "CmContext.h"

/*************************************************************************/
// CM_Break
// Set a break here.
/*************************************************************************/
void CM_Break()
{
}
	
/*************************************************************************/
// CM_Error
// Come here when any error is detected.
/*************************************************************************/
Status CM_Error(Status status)
{
	CM_Break();
	return status;
}
	
/*************************************************************************/
// Break_Waiting_Context
// When Get_Statistics is called, we rifle through all the structures to
// find all waiting contexts.  Set a break here.
/*************************************************************************/
void CM_Cache::Break_Waiting_Context(Callback_Context *p_callback_context)
{
	CM_Cache_Context *p_cache_context = (CM_Cache_Context *)p_callback_context;
	CM_Break();
}

/*************************************************************************/
// Break_Waiting_Context
// When Get_Statistics is called, we rifle through all the structures to
// find all waiting contexts.  Set a break here.
/*************************************************************************/
void CM_Cache::Break_Waiting_Context(I64 m_page_number, Callback_Context *p_callback_context)
{
	CM_Cache_Context *p_cache_context = (CM_Cache_Context *)p_callback_context;
	CM_Break();
}

/*************************************************************************/
// Break_Open_Page
// When Get_Statistics is called, we rifle through all the structures to
// find all Open pages.  Set a break here.
/*************************************************************************/
void CM_Cache::Break_Open_Page(I64	m_page_number, U32 num_read_locks,
	U32 write_lock, U32 writeback_lock, U32 num_remap_locks)
{
	CM_Break();
}

/*************************************************************************/
// CM_Cache::Find_Waiting_Contexts()
/*************************************************************************/
void CM_Cache::Find_Waiting_Contexts()
{
	m_p_frame_table->Find_Waiting_Contexts();
}

/*************************************************************************/
// CM_Frame_Table::Find_Waiting_Contexts()
/*************************************************************************/
void CM_Frame_Table::Find_Waiting_Contexts()
{
	Callback_Context *p_callback_context;

	// Anyone waiting for writeback locks?
	if (!LIST_IS_EMPTY(&m_list_waiting_to_flush))
	{
		p_callback_context = (Callback_Context *)
			LIST_POINT_HEAD(&m_list_waiting_to_flush);

		// First context in list waiting for writeback lock
		CM_Cache::Break_Waiting_Context(p_callback_context);
		while (!LIST_ENTRY_IS_LAST(&m_list_waiting_to_flush, &p_callback_context->m_list))
		{
			p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);
			CM_Cache::Break_Waiting_Context(p_callback_context);
		}
	}

	// Anyone waiting for a frame?
	if (!LIST_IS_EMPTY(&m_list_wait_frame))
	{
		p_callback_context = (Callback_Context *)
			LIST_POINT_HEAD(&m_list_wait_frame);

		// First context waiting for a frame
		CM_Cache::Break_Waiting_Context(p_callback_context);
		while (!LIST_ENTRY_IS_LAST(&m_list_wait_frame, &p_callback_context->m_list))
		{
			// Next context waiting for a frame
			p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);
			CM_Cache::Break_Waiting_Context(p_callback_context);
		}
	}

	// Rifle through each frame
    CM_Frame *p_frame;
	for (U32 index = 0; index < m_num_page_frames; index++)
	{
		// Point to the next frame.
		p_frame = Get_Frame(index + 1);

		// Anyone waiting for this frame?
		p_frame->Find_Waiting_Contexts();
	}
}

/*************************************************************************/
// CM_Frame::Find_Waiting_Contexts()
/*************************************************************************/
void CM_Frame::Find_Waiting_Contexts()
{
	Callback_Context *p_callback_context;

	if (!LIST_IS_EMPTY(&m_list_wait_for_unlock))
	{
		p_callback_context = (Callback_Context *)
			LIST_POINT_HEAD(&m_list_wait_for_unlock);

		// First context waiting for this frame
		CM_Cache::Break_Waiting_Context(m_page_number, p_callback_context);
		while (!LIST_ENTRY_IS_LAST(&m_list_wait_for_unlock, &p_callback_context->m_list))
		{
			p_callback_context = (Callback_Context *)LIST_POINT_NEXT(&p_callback_context->m_list);

			// Next context waiting for this frame
			CM_Cache::Break_Waiting_Context(m_page_number, p_callback_context);
		}
	}
}

/*************************************************************************/
// CM_Cache::Find_Open_Pages()
/*************************************************************************/
void CM_Cache::Find_Open_Pages()
{
	m_p_frame_table->Find_Open_Pages();
}

/*************************************************************************/
// CM_Frame_Table::Find_Open_Pages()
/*************************************************************************/
void CM_Frame_Table::Find_Open_Pages()
{
	// Rifle through all the frames
    CM_Frame *p_frame;
	for (U32 index = 0; index < m_num_page_frames; index++)
	{
		// Point to the next frame.
		p_frame = Get_Frame(index + 1);

		// Is this frame open?
		p_frame->Find_Open_Pages();
	}
}

/*************************************************************************/
// CM_Frame::Find_Open_Pages()
/*************************************************************************/
void CM_Frame::Find_Open_Pages()
{
	if (Is_Page_Locked())
		CM_Cache::Break_Open_Page(m_page_number, m_num_read_locks,
			m_write_lock, m_writeback_lock, m_num_remap_locks);
}
