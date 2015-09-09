/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmInitialize.cpp
// 
// Description:
// This file implements the Initialize method of the Cache Manager. 
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 12/10/98 Jim Frandeen: Set m_page_number to -1 if uninitialized.
//		Initialize m_num_remap_locks.
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#define	TRACE_INDEX		TRACE_DDM_CACHE
#include "CmCache.h"
#include "CmMem.h"
#include "CmCommon.h"
#include "TraceMon.h"
#include <String.h>

/*************************************************************************/
// CM_Cache::Initialize
// Initialize the Cache Manager object 
/*************************************************************************/
Status CM_Cache::Initialize(
	CM_CONFIG				*p_config,
	CM_PREFETCH_CALLBACK	*p_prefetch_callback,
	CM_WRITE_CALLBACK		*p_write_callback,
	CM_CACHE_HANDLE 		*p_cache_handle,
	void					*p_callback_context)
{
	// Turn on tracing.
	//TraceLevel[TRACE_DDM_CACHE] = 8;
	TRACE_ENTRY(CM_Cache::Initialize);
	
	// Get size of memory required for tables.
	// Assume user has allocated this much memory.
	U32 size_memory;
	Status status = CM_Get_Memory_Size_Required(p_config, &size_memory);
	if (status != OK)
		return status;

	// Allocate CM_Cache object
	void *p_table_memory = (char *)p_config->p_table_memory;
	CM_Cache *p_cache = (CM_Cache *)p_table_memory;
	p_table_memory = (char *)p_table_memory + sizeof(CM_Cache);

	// Initialize the memory object.  
	// We will use this to allocate all our internal memory.
	p_cache->m_mem.Initialize(size_memory - sizeof(CM_Cache), p_table_memory);

	// Validate config parameters
	status = Validate_Config(p_config);
	if (status != OK)
		return status;

	// Save config in cache manager object.
	memcpy(&p_cache->m_config, p_config, sizeof(CM_CONFIG));
	p_cache->m_cookie = CM_COOKIE;

	// Is this a write through cache or a write back cache?
	if (p_write_callback)
		p_cache->m_is_write_back = 1;
	else
		p_cache->m_is_write_back = 0;

	// Does this cache use associative mapping or linear mapping?
	if (p_cache->m_config.hash_table_size)
	{
		// Associative mapping
		p_cache->m_p_page_table = 0;
		p_cache->m_p_page_table_2 = 0;

		// Allocate hash table.
		Status status = p_cache->m_hash_table.Allocate(&p_cache->m_mem, 
			p_config->hash_table_size);
		if (status != OK)
			return status;

		if (p_config->num_pages_secondary)
		{
			// Allocate hash table for secondary cache.
			Status status = p_cache->m_hash_table_2.Allocate(&p_cache->m_mem, 
				p_config->hash_table_size);
			if (status != OK)
				return status;
		}
	}
	else
	{
		// Linear mapping
		// Allocate and initialize Page Table
		p_cache->m_p_page_table = CM_Page_Table::Allocate(&p_cache->m_mem, p_config);
		if (p_cache->m_p_page_table == 0)
			return CM_NO_MEMORY;

		if (p_config->num_pages_secondary)
		{
			// Allocate and initialize Page Table for secondary cache.
			p_cache->m_p_page_table_2 = CM_Page_Table::Allocate(&p_cache->m_mem, p_config);
			if (p_cache->m_p_page_table_2 == 0)
				return CM_NO_MEMORY;
		}
		else
			p_cache->m_p_page_table_2 = 0; 
	}

	// Save amount of memory used for use with statistics object.
	p_cache->m_memory_used = size_memory;

	// Allocate CM_Stats object.
	p_cache->m_p_stats = CM_Stats::Allocate(&p_cache->m_mem);
	if (p_cache->m_p_stats == 0)
		return CM_NO_MEMORY;

	if (p_config->num_pages_secondary)
	{
		// Allocate page buffer, used for exchanging cache pages.
		p_cache->m_p_page_buffer = p_cache->m_mem.Allocate(p_config->page_size);
		if (p_cache->m_p_page_buffer == 0)
			return 0;
	
	}
	else
		p_cache->m_p_page_buffer = 0;

	// Allocate and initialize Frames.
	p_cache->m_p_frame_table = CM_Frame_Table::Allocate(&p_cache->m_mem, p_config, 
		p_prefetch_callback, p_write_callback, p_cache->m_p_stats, p_callback_context,
		p_config->num_pages, p_config->p_page_memory);
	if (p_cache->m_p_frame_table == 0)
		return CM_NO_MEMORY;

	if (p_config->num_pages_secondary)
	{
		// Allocate and initialize Frames for secondary cache.
		p_cache->m_p_frame_table_2 = CM_Frame_Table::Allocate(&p_cache->m_mem, 
			p_config, p_prefetch_callback, p_write_callback, p_cache->m_p_stats, 
			p_callback_context, 
			p_config->num_pages_secondary, p_config->p_page_memory_secondary);
		if (p_cache->m_p_frame_table_2 == 0)
			return CM_NO_MEMORY;
	}
	else
		p_cache->m_p_frame_table_2 = 0;

	// Reset statistics.
	p_cache->Reset_Statistics();
	p_cache->Reset_Event_Data();

	// Return pointer to CM_Cache object as cache handle
	*p_cache_handle = p_cache;

	return OK;

} // CM_Cache::Initialize


/*************************************************************************/
// CM_Frame::Initialize
// Initialize the frame .
/*************************************************************************/
void CM_Frame::Initialize(U32 frame_index, CM_PAGE_STATE page_state)
{
	//Tracef("\n\rInit Frame, index = %i, address = %X", frame_index, this);
	
	// Initialize frame lists;
	LIST_INITIALIZE(&m_list);
	LIST_INITIALIZE(&m_list_wait_for_unlock);

	// Frame is currently empty.
	m_page_number = -1;

	m_frame_index = frame_index;

	m_num_read_locks = 0;
	m_num_remap_locks = 0;
	m_write_lock = 0;
	m_writeback_lock = 0;
	m_is_page_dirty = 0;
	m_is_page_accessed = 0;
	m_p_next_hash_entry = 0;

	m_page_state = page_state;

} //CM_Frame::Initialize

