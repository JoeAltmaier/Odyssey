/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmInterface.cpp
// 
// Description:
// This file implements the interface to the Cache Manager. 
// Each method converts a cache handle into a pointer to
// a cache object and then calls the corresponding cache method.
// 
// Update Log 
// 
// 9/6/98 Jim Frandeen: Create file
// 02/08/99 Jim Frandeen: Add write through, 64-bit keys and hash
/*************************************************************************/

#include "CmCache.h"

#if 1
#define VALIDATE \
p_cache->Validate()
#else
#define VALIDATE
#endif

/*************************************************************************/
// CM_Abort_Page
// The user calls Abort Page when the read or write operation has completed.
// The page has not been modified.
/*************************************************************************/
Status CM_Abort_Page(CM_CACHE_HANDLE cache_handle, CM_PAGE_HANDLE page_handle) 
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	Status status = p_cache->Abort_Page(CM_Frame_Handle(page_handle));
	VALIDATE;
	return status;

} // CM_Abort_Page

/*************************************************************************/
// CM_Close_Cache
// Flush cache and release all resources.  
// The close callback method will be called with a status
// when the flush operation has completed.
/*************************************************************************/
Status CM_Close_Cache(CM_CACHE_HANDLE cache_handle, Callback_Context *p_callback_context)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	Status status = p_cache->Close(p_callback_context);
	VALIDATE;
	return status;

} // CM_Close_Cache

/*************************************************************************/
// CM_Close_Page
// The user calls Close Page when the read or write operation has completed.  
// If any contexts are waiting for the page, the Cache Manager calls 
// their Notify callback at this time.
/*************************************************************************/
Status CM_Close_Page(CM_CACHE_HANDLE cache_handle, CM_PAGE_HANDLE page_handle)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	Status status = p_cache->Close_Page(CM_Frame_Handle(page_handle));
	VALIDATE;
	return status;

} // CM_Close_Page

/*************************************************************************/
// CM_Display_Stats_And_Events.
// Create a string that contains the flash stats and event data.
/*************************************************************************/
Status CM_Display_Stats_And_Events(
	CM_CACHE_HANDLE cache_handle)
{	
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	return p_cache->Display_Stats_And_Events();

} // CM_Display_Stats_And_Events

/*************************************************************************/
// CM_Format_Stats_And_Events.
// Create a string that contains the flash stats and event data.
/*************************************************************************/
Status CM_Format_Stats_And_Events(
	CM_CACHE_HANDLE cache_handle, char *p_string)
{	
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	return p_cache->Format_Stats_And_Events(p_string);

} // CM_Format_Stats_And_Events

/*************************************************************************/
// CM_Flush_Cache
// Flush cache.  The flush callback method will be called with a status
// when the flush operation has completed.
/*************************************************************************/
Status CM_Flush_Cache(CM_CACHE_HANDLE cache_handle, Callback_Context *p_callback_context)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	Status status = p_cache->Flush_Cache(p_callback_context);
	VALIDATE;
	return status;

} // Flush_Cache

/*************************************************************************/
// CM_Get_Event_Data
/*************************************************************************/
Status CM_Get_Event_Data(CM_CACHE_HANDLE cache_handle,
	CM_EVENT_DATA *p_event_data_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(p_event_data_buffer), CM_Get_Event_Data);
#endif
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	p_cache->Get_Event_Data(p_event_data_buffer, size_buffer);
	return OK;

} // CM_Get_Event_Data

/*************************************************************************/
// CM_Get_Statistics
/*************************************************************************/
Status CM_Get_Statistics(CM_CACHE_HANDLE cache_handle,
	CM_STATISTICS *p_statistics_buffer, 
	U32 size_buffer) 
{
#ifndef _WINDOWS
	CT_ASSERT(IS_ALIGNED_8(p_statistics_buffer), CM_Get_Statistics);
#endif
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	p_cache->Find_Waiting_Contexts();
	p_cache->Find_Open_Pages();

	p_cache->Get_Statistics(p_statistics_buffer, size_buffer);
	return OK;

} // CM_Get_Statistics

/*************************************************************************/
// CM_Get_Config
// Get configuration record.
/*************************************************************************/
Status CM_Get_Config(CM_CACHE_HANDLE cache_handle,
	CM_CONFIG *p_config_buffer, 
	U32 size_buffer) 
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);
	p_cache->Get_Config(p_config_buffer, size_buffer);
	return OK;

} // CM_Get_Config

/*************************************************************************/
// CM_Get_Memory_Size_Required_Required
// Get memory size required for cache tables.
/*************************************************************************/
Status CM_Get_Memory_Size_Required(CM_CONFIG *p_config, U32 *p_memory_size)
{
	U32 memory_size;
	if (p_config->hash_table_size)
	{
		memory_size = CM_Hash_Table::Memory_Size(p_config);
		if (p_config->num_pages_secondary)

			// Plus hash table for secondary cache.
			memory_size += CM_Hash_Table::Memory_Size(p_config);
	}
	else
	{
		memory_size = CM_Page_Table::Memory_Size(p_config);
		if (p_config->num_pages_secondary)

			// Plus page table for secondary cache.
			memory_size += CM_Page_Table::Memory_Size(p_config);
	}

	// Add size for frame table.
	memory_size += CM_Frame_Table::Memory_Size(p_config->num_pages);
	if (p_config->num_pages_secondary)

		// Plus frame table for secondary cache.
		memory_size += CM_Frame_Table::Memory_Size(p_config->num_pages_secondary);

	// Add size of stats data.
	memory_size += CM_Stats::Memory_Size(p_config);
	if (p_config->num_pages_secondary)
	{
		// Page buffer for secondary cache
		memory_size += p_config->page_size;
	}

	// Add size of CM_Cache object.
	memory_size += sizeof(CM_Cache);

	*p_memory_size = memory_size;

	return OK;

} // CM_Get_Memory_Size_Required

/*************************************************************************/
// CM_Initialize
// Initialize the Cache Manager object 
/*************************************************************************/
Status CM_Initialize(
	CM_CONFIG				*p_config,
	CM_PREFETCH_CALLBACK	*p_prefetch_callback,
	CM_WRITE_CALLBACK		*p_write_callback,
	CM_CACHE_HANDLE 		*p_cache_handle,
	void					*p_callback_context)
{
	Status status = CM_Cache::Initialize(
		p_config,
		p_prefetch_callback,
		p_write_callback,
		p_cache_handle,
		p_callback_context);

	return status;

} // CM_Initialize

/*************************************************************************/
// CM_Open_Page
// Open Page gets read or write access to a page in the cache unless the 
// page is already locked by another operation. If a cache hit is reported,
// a pointer to the page frame is returned.  If a cache miss is reported,
// a pointer to an empty page frame is returned.  
// The page is locked until the user calls Close Page.
/*************************************************************************/
Status CM_Open_Page(CM_CACHE_HANDLE cache_handle, 
	I64 page_number,
	U32 flags,
	Callback_Context *p_callback_context,
	void **pp_page_frame,
	CM_PAGE_HANDLE *p_page_handle)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	Status status = p_cache->Open_Page(
		page_number,
		flags,
		p_callback_context,
		pp_page_frame,
		p_page_handle);
	VALIDATE;
	return status;

} // CM_Open_Page

/*************************************************************************/
// CM_Reset_Event_Data
// Reset event data record.
/*************************************************************************/
Status CM_Reset_Event_Data(CM_CACHE_HANDLE cache_handle)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	p_cache->Reset_Event_Data();
	return OK;

} // CM_Reset_Event_Data

/*************************************************************************/
// CM_Reset_Statistics
// Reset statistics record.
/*************************************************************************/
Status CM_Reset_Statistics(CM_CACHE_HANDLE cache_handle)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	p_cache->Reset_Statistics();
	return OK;

} // CM_Reset_Statistics

/*************************************************************************/
// CM_Set_Page_Dirty
// This is only used when the page was opened for read, and an ECC
// error was corrected when the page was read in.
/*************************************************************************/
Status CM_Set_Page_Dirty(CM_CACHE_HANDLE cache_handle, CM_PAGE_HANDLE page_handle)
{
	CM_Cache *p_cache = CM_Cache::Validate_Cache_Handle(cache_handle);
	if (p_cache == 0)
		return CM_ERROR(INVALID_CACHE_HANDLE);

	return p_cache->Set_Page_Dirty(page_handle);

} // CM_Set_Page_Dirty

/*************************************************************************/
// CM_Cache::Validate_Config
// Initialize the Cache Manager object 
/*************************************************************************/
Status CM_Cache::Validate_Config(CM_CONFIG *p_config)
{
    if (p_config->version != CM_CONFIG_VERSION)
        return CM_ERROR(INVALID_CONFIG_VERSION);

    if (p_config->page_size < CM_MIN_PAGE_SIZE)
        return CM_ERROR(INVALID_CONFIG_DATA);

	// The user must select either linear mapping (page_table_size)
	// or associative mapping (hash_table_size)
    if ((p_config->page_table_size == 0) && (p_config->hash_table_size == 0))
        return CM_ERROR(INVALID_CONFIG_DATA);

    if (p_config->page_table_size)
	{
		// Linear mapping
		if (p_config->num_reserve_pages > p_config->page_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);

		if (p_config->num_prefetch_forward > p_config->page_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);

		if (p_config->num_prefetch_backward > p_config->page_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);
	}
	else
	{
		// Associative mapping
		if (p_config->num_reserve_pages > p_config->hash_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);

		if (p_config->num_prefetch_forward > p_config->hash_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);

		if (p_config->num_prefetch_backward > p_config->hash_table_size)
			return CM_ERROR(INVALID_CONFIG_DATA);
	}

	// dirty_page_writeback_threshold is a percentage.
    if (p_config->dirty_page_writeback_threshold > 99)
        return CM_ERROR(INVALID_CONFIG_DATA);

	// If no percentage is specified...
    if (p_config->dirty_page_writeback_threshold == 0)
        p_config->dirty_page_writeback_threshold = 50;

	// dirty_page_error_threshold is a percentage.
    if (p_config->dirty_page_error_threshold > 100)
        return CM_ERROR(INVALID_CONFIG_DATA);

	// If no percentage is specified...
    if (p_config->dirty_page_error_threshold == 0)
        p_config->dirty_page_error_threshold = 95;

	// Get size of memory required for tables.
	// Assume user has allocated this much memory.
	U32 size_memory;
	Status status = CM_Get_Memory_Size_Required(p_config, &size_memory);
	if (status != OK)
		return status;

	// Check to be sure that memory specified does not overlap.
	char *p_table_memory_first = (char *)p_config->p_table_memory;
	char *p_table_memory_last = p_table_memory_first + size_memory;
	char *p_page_memory_first = (char *)p_config->p_page_memory;
	char *p_page_memory_last = p_page_memory_first 
		+ (p_config->num_pages * p_config->page_size);

	if (p_page_memory_last > p_table_memory_last)
	{
		if (p_table_memory_last > p_page_memory_first)
			return CM_ERROR(CONFIG_MEMORY_OVERLAP);
	}
	else
	{
		if (p_page_memory_last > p_table_memory_first)
			return CM_ERROR(CONFIG_MEMORY_OVERLAP);
	}

    return OK;

} // Validate_Config
