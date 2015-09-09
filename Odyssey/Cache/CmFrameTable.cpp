/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmFrameTable.cpp
// 
// Description:
// This file implements the CM_Frame_Table class methods of the 
// Cache Manager 
// 
// Update Log 
// 
// 9/9/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "Callback.h"
#include "CmFrameTable.h"
#include "CmMem.h"
#include "TraceMon.h"
#include <String.h>

#ifdef _DEBUG
int CM_Frame_Table::Got_Mutex;
#endif

/*************************************************************************/
// CM_Frame_Table::Allocate
// Allocate and initialize the frame table.
// Return pointer to allocated table or zero if table
// could not be allocated.
/*************************************************************************/
CM_Frame_Table * CM_Frame_Table::Allocate(
		CM_Mem					*p_mem, 
		CM_CONFIG				*p_config, 
		CM_PREFETCH_CALLBACK	*p_prefetch_callback,
		CM_WRITE_CALLBACK		*p_write_callback,
		CM_Stats				*p_stats,
		void					*p_callback_context,
		U32						 num_page_frames,
		void					*p_page_memory)
{
	// Allocate CM_Frame_Table object
	CM_Frame_Table *p_frame_table = 
		(CM_Frame_Table *)p_mem->Allocate(sizeof(CM_Frame_Table));

	if (p_frame_table == 0)
		return 0;
	
	// Initialize table lists.
	LIST_INITIALIZE(&p_frame_table->m_list_waiting_to_flush);
	LIST_INITIALIZE(&p_frame_table->m_list_wait_frame);

	// Initialize each of our dummy frames.  Each of our frame lists
	// is a dummy frame so that, when the last frame in the list points
	// to the head of the list, we can treat it as a frame without
	// having to check to see if we are pointing to the head of the list.
	p_frame_table->m_clock_list.Initialize(0, CM_PAGE_STATE_CLEAN);
	p_frame_table->m_dirty_clock_list.Initialize(0, CM_PAGE_STATE_DIRTY);

	// Save callbacks.
	p_frame_table->m_p_prefetch_callback = p_prefetch_callback;
	p_frame_table->m_p_write_callback = p_write_callback;
	p_frame_table->m_p_callback_context = p_callback_context;

	// Make sure the page size is a multiple of 8 for alignment.
	p_frame_table->m_page_size = ((p_config->page_size + 7) / 8) * 8;

	p_frame_table->m_num_pages_replaceable = 0;
	p_frame_table->m_num_pages_clock = 0;
	p_frame_table->m_num_pages_dirty_clock = 0;
	p_frame_table->m_num_pages_being_written = 0;
	p_frame_table->m_num_pages_working_set = 0;
	p_frame_table->m_p_clock_frame = 0;
	p_frame_table->m_p_dirty_clock_frame = 0;

	// Find out how much memory is available.
	// Leave this for debugging.  Should be close to zero.
	U32 memory_available = p_mem->Memory_Available();

	// Save number of page frames.
	p_frame_table->m_num_page_frames = num_page_frames;

    if (p_config->page_table_size)
	{
		// Linear mapping
		// Don't allocate more pages than specified in the config.
		if (p_frame_table->m_num_page_frames > p_config->page_table_size)
			p_frame_table->m_num_page_frames = p_config->page_table_size;
	}

	// Allocate array of page frames
	p_frame_table->m_p_page_frame_array = (char *)p_page_memory;

	// Allocate array of frames
	p_frame_table->m_p_frame_array = 
		(CM_Frame *)p_mem->Allocate(sizeof(CM_Frame) * p_frame_table->m_num_page_frames);
	if (p_frame_table->m_p_frame_array == 0)
		return 0;

	// When m_p_clock_frame is zero, we know that the frame table is not yet initialized.
	p_frame_table->m_p_clock_frame = 0;

	// Initialize each frame
    CM_Frame *p_frame;
	for (U32 index = 0; index < p_frame_table->m_num_page_frames; index++)
	{
		// Point to the next frame.
		p_frame = p_frame_table->Get_Frame(index + 1);

		// Have the frame initialize itself.
		p_frame->Initialize(index + 1, CM_PAGE_STATE_CLEAN);
		CT_ASSERT((p_frame_table->Get_Frame(index + 1) == p_frame), CM_Frame_Table::Allocate);
		CT_ASSERT((p_frame->Get_Frame_Index() == (index + 1)), CM_Frame_Table::Allocate);

		// Make sure the list object is first in the frame.
		CT_ASSERT(((CM_Frame *)&p_frame->m_list == p_frame), CM_Frame_Table::Allocate);
		CT_ASSERT((p_frame->Is_Replaceable()), CM_Frame_Table::Allocate);

		// Initially, each frame is on the clock list
		LIST_INSERT_TAIL(&p_frame_table->m_clock_list.m_list, &p_frame->m_list);
		p_frame_table->m_num_pages_clock++;
		p_frame_table->m_num_pages_replaceable++;
	}

	// Initialize the clocks
	p_frame_table->m_p_clock_frame = p_frame;
	p_frame_table->m_p_dirty_clock_frame = &p_frame_table->m_dirty_clock_list;

	// Calculate dirty page writeback threshold from the percentage in the config file.
	p_frame_table->m_dirty_page_writeback_threshold = 
		(p_frame_table->m_num_page_frames * p_config->dirty_page_writeback_threshold)
		/ 100;

	// Calculate dirty page error threshold from the percentage in the config file.
	p_frame_table->m_dirty_page_error_threshold = (p_frame_table->m_num_page_frames 
		* p_config->dirty_page_error_threshold)
		/ 100;

	// Make sure the number of reserve pages is less than the number of pages in the cache.
	// This would only happen in a test environment where the cache size is small.
	p_frame_table->m_num_reserve_pages = p_config->num_reserve_pages;
	if (p_frame_table->m_num_reserve_pages > p_frame_table->m_num_pages_replaceable)

		// Make number of reserve pages less than number of pages in the cache.
		p_frame_table->m_num_reserve_pages = 
			p_frame_table->m_num_pages_replaceable - (p_frame_table->m_num_pages_replaceable / 2);

	// Calculate the maximum number of dirty pages.
	U32 max_number_dirty_pages = p_frame_table->m_num_pages_replaceable
		- p_frame_table->m_num_reserve_pages;

	// Make sure the dirty page error threshold is less than the maximum number of dirty pages;
	// otherwise we will have a context waiting for a page to be cleaned, and it will 
	// never happen.  This would only occur in a test situation, where cache size is small.
	if (p_frame_table->m_dirty_page_error_threshold > max_number_dirty_pages)

		// Make dirty page error threshold less than max_number_dirty_pages.
		p_frame_table->m_dirty_page_error_threshold = 
			max_number_dirty_pages - (max_number_dirty_pages / 2);

	// Make sure the writeback threshold is less than the dirty page error threshold.
	if (p_frame_table->m_dirty_page_writeback_threshold > p_frame_table->m_dirty_page_error_threshold)

		// Make dirty page writeback threshold less than dirty page error threshold.
		p_frame_table->m_dirty_page_writeback_threshold = 
			p_frame_table->m_dirty_page_error_threshold 
				- (p_frame_table->m_dirty_page_error_threshold / 2);

	// Initialize pointer to statistics object
	p_frame_table->m_p_stats = p_stats;

#ifdef _WINDOWS

	// Initialize Windows mutex
	p_frame_table->m_mutex = CreateMutex(
		NULL, // LPSECURITY_ATTRIBUTES lpEventAttributes,	// pointer to security attributes
		
		// If TRUE, the calling thread requests immediate ownership of the mutex object. 
		// Otherwise, the mutex is not owned. 
		0, // flag for initial ownership 
		// If lpName is NULL, the event object is created without a name. 
		NULL // LPCTSTR lpName 	// pointer to semaphore-object name  
	   );

	if (p_frame_table->m_mutex == 0)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Allocate", 
			"CreateMutex failed",
			erc,
			0);
		return p_frame_table;
	}
#else
#ifdef THREADX
	// Initialize Threadx semaphore
	Status	status = tx_semaphore_create(&p_frame_table->m_mutex, "CmMutex",
		1); // initial count
#else
	// Initialize Nucleus semaphore
	Status	status = NU_Create_Semaphore(&p_frame_table->m_mutex, "CmMutex",
		1, // initial count
		NU_FIFO);
#endif

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"CM_Frame_Table::Allocate", 
			"NU_Create_Semaphore failed",
			status,
			0);
		return p_frame_table;
	}
#endif

#ifdef _DEBUG
	Got_Mutex = 0;
#endif

	// Return pointer to table object
	return p_frame_table;

} // CM_Frame_Table::Allocate

/*************************************************************************/
// CM_Frame_Table::Inc_Replaceable_Pages
// Return any contexts waiting for a frame.
/*************************************************************************/
void CM_Frame_Table::Inc_Replaceable_Pages(LIST *p_list_waiting_contexts)
{
 	TRACE_ENTRY(CM_Frame_Table::Inc_Replaceable_Pages);

	// Increment number of replaceable pages.
	m_num_pages_replaceable++;
	CT_ASSERT((m_num_pages_replaceable <= m_num_page_frames), CM_Frame_Table::Inc_Replaceable_Pages);
	m_p_stats->Set_Num_Pages_Replaceable(m_num_pages_replaceable);

	// See if any context is waiting for a frame.
	if (LIST_IS_EMPTY(&m_list_wait_frame))
		return;

	// There is a context waiting for a page frame.
	// Remove it from the list and put it on the ready list.
	Callback_Context *p_callback_context = 
		(Callback_Context *)LIST_REMOVE_TAIL(&m_list_wait_frame);
	m_p_stats->Dec_Num_Waits_For_Page_Frame();

	LIST_INSERT_TAIL(p_list_waiting_contexts, &p_callback_context->m_list);

}  // CM_Frame_Table::Inc_Replaceable_Pages

