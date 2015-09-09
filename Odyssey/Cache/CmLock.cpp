/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmLock.cpp
// 
// Description:
// This file implements the page locking methods of the Cache Manager. 
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmCache.h"
#include "TraceMon.h"

/*************************************************************************/
// CM_Frame::Lock_Page_Not_Present
// Lock a page when the page being opened is not present.  
// Return handle of locked page.
/*************************************************************************/
CM_PAGE_HANDLE CM_Frame::Lock_Page_Not_Present(U32 flags,
											   CM_Stats *p_stats)
{
	CT_ASSERT((!Is_Page_Locked()), CM_Frame::Lock_Page_Not_Present);
	CT_ASSERT((m_is_page_dirty == 0), CM_Frame::Lock_Page_Not_Present);

	switch (CM_OPEN_MODE(flags))
	{
	case CM_OPEN_MODE_REMAP:

		// Change open mode to reflect state not present.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Lock_Page_Not_Present mode remap, page = ", (U32)m_page_number);
		CM_SET_OPEN_MODE(flags, CM_OPEN_MODE_REMAP_NOT_PRESENT);

		// Tell the stats record how many pages are locked for write
		p_stats->Inc_Num_Pages_Locked_Write();
		break;

	case CM_OPEN_MODE_READ:

		// Change open mode to reflect state not present.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Lock_Page_Not_Present mode read, page = ", (U32)m_page_number);
		CM_SET_OPEN_MODE(flags, CM_OPEN_MODE_READ_NOT_PRESENT);

		// Tell the stats record how many pages are locked for read not present
		p_stats->Inc_Num_Pages_Locked_Read_Not_Present();
		break;

	case CM_OPEN_MODE_WRITE:

		// Change open mode to reflect state not present.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Lock_Page_Not_Present mode write, page = ", (U32)m_page_number);
		CM_SET_OPEN_MODE(flags, CM_OPEN_MODE_WRITE_NOT_PRESENT);

		// Tell the stats record how many pages are locked for write
		p_stats->Inc_Num_Pages_Locked_Write();
		break;

	case CM_OPEN_MODE_WRITE_THROUGH:

		// Change open mode to reflect state not present.
		TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Lock_Page_Not_Present mode write, page = ", (U32)m_page_number);
		CM_SET_OPEN_MODE(flags, CM_OPEN_MODE_WRITE_THROUGH_NOT_PRESENT);

		// Tell the stats record how many pages are locked for write
		p_stats->Inc_Num_Pages_Locked_Write();
		break;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame::Lock_Page_Not_Present", 
			"Invalid mode",
			CM_ERROR(INVALID_PAGE_HANDLE),
			0);
		break;

	} // switch

	// A page that is not present is always opened with a write lock.
	// It must have exclusive access until the page has been filled.
	m_write_lock = 1;

	// Return page handle
	return CM_Frame_Handle::Create_Page_Handle(flags, Get_Frame_Index());

} // CM_Frame::Lock_Page_Not_Present

/*************************************************************************/
// CM_Frame::Lock_Page_Writeback
// Lock a page.  Return handle of locked page.
/*************************************************************************/
CM_PAGE_HANDLE CM_Frame::Lock_Page_Writeback(CM_Stats *p_stats)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Lock_Page_Writeback, page = ", (U32)m_page_number);

	CT_ASSERT((m_writeback_lock == 0), CM_Frame::Lock_Page_Writeback);

	m_writeback_lock = 1;

	// Tell the stats record how many pages are locked for writeback
	p_stats->Inc_Num_Pages_Locked_Writeback();

	// Return page handle
	return CM_Frame_Handle::Create_Page_Handle(CM_OPEN_MODE_WRITE_BACK, Get_Frame_Index());

} // CM_Frame::Lock_Page_Writeback


/*************************************************************************/
// CM_Frame::Unlock_Page_Read
// Unlock a page locked for read
// Return context, if any, waiting for this frame.
/*************************************************************************/
void CM_Frame::Unlock_Page_Read(CM_Stats *p_stats, LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Unlock_Page_Read, page = ", (U32)m_page_number);

	CT_ASSERT((m_num_read_locks != 0), CM_Frame::Unlock_Page_Read);

	// Decrement number of read locks
	if (--m_num_read_locks != 0)
		return;

	// Tell the stats record how many pages are locked for read
	p_stats->Dec_Num_Pages_Locked_Read();

	// There are no read locks on this page.
	// See if any context is waiting.
	Wakeup_Contexts_Waiting_For_Lock(p_stats, p_list_waiting_contexts);

} // CM_Frame::Unlock_Page_Read

/*************************************************************************/
// CM_Frame::Unlock_Page_Remap
// Unlock a page locked for remap
// Return context, if any, waiting for this frame.
/*************************************************************************/
void CM_Frame::Unlock_Page_Remap(CM_Stats *p_stats, LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Unlock_Page_Remap, page = ", (U32)m_page_number);

	CT_ASSERT((m_num_remap_locks != 0), CM_Frame::Unlock_Page_Remap);

	// Decrement number of remap locks
	if (--m_num_remap_locks != 0)
		return;

	// Tell the stats record how many pages are locked for remap
	p_stats->Dec_Num_Pages_Locked_Remap();

	// There are no remap locks on this page.
	// See if any context is waiting.
	Wakeup_Contexts_Waiting_For_Lock(p_stats, p_list_waiting_contexts);

} // CM_Frame::Unlock_Page_Remap


/*************************************************************************/
// CM_Frame::Unlock_Page_Read_Not_Present
// Unlock a page locked for read not present
// Return contexts waiting for this frame.
/*************************************************************************/
void CM_Frame::Unlock_Page_Read_Not_Present(CM_Stats *p_stats, 
											LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Unlock_Page_Read_Not_Present, page = ", (U32)m_page_number);

	// Tell the stats record how many pages are locked for read not present
	p_stats->Dec_Num_Pages_Locked_Read_Not_Present();

	CT_ASSERT((m_write_lock != 0), CM_Frame::Unlock_Page_Write);

	// Write lock is either on or off.  Turn off write lock.
	m_write_lock = 0;

	Wakeup_Contexts_Waiting_For_Lock(p_stats, p_list_waiting_contexts);

} // CM_Frame::Unlock_Page_Read_Not_Present

/*************************************************************************/
// CM_Frame::Unlock_Page_Write
// Unlock a page locked for write
// Return pointer to context (if any) that was waiting for this frame.
/*************************************************************************/
void CM_Frame::Unlock_Page_Write(CM_Stats *p_stats, LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Unlock_Page_Write, page = ", (U32)m_page_number);

	// Tell the stats record how many pages are locked for write
	p_stats->Dec_Num_Pages_Locked_Write();

	CT_ASSERT((m_write_lock != 0), CM_Frame::Unlock_Page_Write);

	// Write lock is either on or off.  Turn off write lock.
	m_write_lock = 0;

	Wakeup_Contexts_Waiting_For_Lock(p_stats, p_list_waiting_contexts);

} // CM_Frame::Unlock_Page_Write

/*************************************************************************/
// CM_Frame::Wakeup_Contexts_Waiting_For_Lock
// Unlock a page locked for write or read not present
// Return pointer to context (if any) that was waiting for this frame.
/*************************************************************************/
void CM_Frame::Wakeup_Contexts_Waiting_For_Lock(CM_Stats *p_stats, LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Wakeup_Contexts_Waiting_For_Lock, page = ", (U32)m_page_number);

	// See if any context is waiting.
	while (!LIST_IS_EMPTY(&m_list_wait_for_unlock))
	{
		// There is a context waiting for write locks.
		// Remove it from the list and put it on the ready list.
		Callback_Context *p_callback_context = 
			(Callback_Context *)LIST_REMOVE_TAIL(&m_list_wait_for_unlock);
		p_stats->Dec_Num_Waits_For_Lock();

		LIST_INSERT_TAIL(p_list_waiting_contexts, &p_callback_context->m_list);
	}
} // CM_Frame::Wakeup_Contexts_Waiting_For_Lock

/*************************************************************************/
// CM_Frame::Unlock_Page_Writeback
// Unlock a page locked for writeback.
// The page was being written from the cache to backing store.
/*************************************************************************/
void CM_Frame::Unlock_Page_Writeback(CM_Stats *p_stats, LIST *p_list_waiting_contexts)
{
	TRACE_NUMBER_EOL(TRACE_L5, "CM_Frame::Unlock_Page_Writeback, page = ", (U32)m_page_number);

	CT_ASSERT((m_writeback_lock != 0), CM_Frame::Unlock_Page_Writeback);

	// Tell the stats record how many pages are locked for writeback
	p_stats->Dec_Num_Pages_Locked_Writeback();

	// Writeback lock is either on or off.  Turn off Writeback lock.
	m_writeback_lock = 0;

	Wakeup_Contexts_Waiting_For_Lock(p_stats, p_list_waiting_contexts);

} // CM_Frame::Unlock_Page_Write

