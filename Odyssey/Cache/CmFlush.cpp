/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmFlush.cpp
// 
// Description:
// This file implements the Flush method of the Cache Manager. 
// 
// Update Log 
// 
// 9/10/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "ErrorLog.h"
#include "CmCache.h"
#include "CmContext.h"
#include "CmCommon.h"
#include "TraceMon.h"

/*************************************************************************/
// CM_Frame_Table::Flush_Cache
// Flush all pages in the cache. 
/*************************************************************************/
Status CM_Frame_Table::Flush_Cache(Callback_Context *p_callback_context,
	CM_CACHE_HANDLE cache_handle, int destroy)
{
	TRACE_ENTRY(CM_Frame_Table::Flush_Cache);

	// Allocate a child context of the caller's context.
	// When our child context terminates, the parent
	// context will be scheduled to run.
	CT_ASSERT((sizeof(CM_Cache_Context) <= Callback_Context::Get_Max_Context_Size()), 
		CM_Frame_Table::Flush_Cache);
	CM_Cache_Context *p_child_context = 
		(CM_Cache_Context *)p_callback_context->Allocate_Child(sizeof(CM_Cache_Context));
	if (p_child_context == 0)
		return CM_NO_MEMORY;

	// Save parameters in context.
	p_child_context->m_p_frame_table = this;
	p_child_context->m_cache_handle = cache_handle;
	p_child_context->m_destroy = destroy;

	// Start the flush operation
	p_child_context->Set_Callback(&Flush_Next);
	p_callback_context->Make_Children_Ready();

	// Return to caller.
	return OK;

} // CM_Frame_Table::Flush_Cache

/*************************************************************************/
// CM_Frame_Table::Flush_Next
// Flush all pages in the cache. 
/*************************************************************************/
void CM_Frame_Table::Flush_Next(void *p_context, Status status)
{
	TRACE_ENTRY(CM_Frame_Table::Flush_Next);

	CM_Cache_Context *p_cache_context = (CM_Cache_Context *)p_context;
	CM_Frame_Table *p_frame_table = p_cache_context->m_p_frame_table;
	CM_Cache *p_cache = (CM_Cache *)p_cache_context->m_cache_handle;

	p_cache->Obtain_Mutex();

	// Check to see if there are any dirty pages
	if ((p_frame_table->m_num_pages_dirty_clock 
		+ p_frame_table->m_num_pages_being_written) == 0)
	{
		// We have flushed the last page.
		if (p_cache_context->m_destroy)
			p_cache->Release_Resources();

		p_cache->Release_Mutex();

		// When we terminate this context, the parent context will be
		// scheduled to run.
		p_cache_context->Terminate(p_cache_context, p_cache_context->Get_Status());
		return;
	}

	// There are more dirty pages that need to be flushed.
	while (p_frame_table->m_num_pages_dirty_clock)
	{
		// Call user's callback to write a dirty page.
		Status status = p_frame_table->Write_Dirty_Page(p_cache_context->m_cache_handle);

		if (status != OK)
			// The user was not able to create another context to write
			// a dirty page.  We will try again when a writeback is closed.
			// Exit the while loop.
			break;
	}

	// Check again to see if there are any dirty pages. Write_Dirty_Page above released
	// the mutex.  At this time, the dirty pages could have been closed.
	if ((p_frame_table->m_num_pages_dirty_clock 
		+ p_frame_table->m_num_pages_being_written) == 0)
	{
		// We have flushed the last page.
		if (p_cache_context->m_destroy)
			p_cache->Release_Resources();

		p_cache->Release_Mutex();

		// When we terminate this context, the parent context will be
		// scheduled to run.
		p_cache_context->Terminate(p_cache_context, p_cache_context->Get_Status());
		return;
	}

	// There are still dirty pages to be flushed.
	// Queue this context on the list of contexts waiting for a page to be unlocked.
	// We will run again the next time a page opened for writeback is closed.
	LIST_INSERT_TAIL(&p_frame_table->m_list_waiting_to_flush, &p_cache_context->m_list);
	p_frame_table->m_p_stats->Inc_Num_Flush_Waits_For_Lock();
	p_cache->Release_Mutex();



} // CM_Frame_Table::Flush_Next
