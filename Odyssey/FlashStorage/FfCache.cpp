/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfCache.cpp
// 
// Description:
// This file implements the Flash File cache methods that interface
// to the cache manager. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCache.h"
#include "FfCommon.h"
#include "FfController.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"

/*************************************************************************/
// FF_Open_Cache.
// We don't open the cache until we know the size of the user's address
// space.
/*************************************************************************/
Status FF_Page_Map::Open_Cache()
{
 	TRACE_ENTRY(Open_Cache);
 	
	// Set number of pages to map in cache config.
	U32 num_pages = Get_Num_User_Pages();
	CT_ASSERT((num_pages), FF_Open_Cache);

	// Overwrite page table size with the number of user pages mapped.
	m_p_flash->m_cache_config.page_table_size = num_pages;
	m_p_flash->m_cache_config.hash_table_size = 0;

	// The cache memory is usually not set up, but it can be specified
	// for test purposes.
	if (m_p_flash->m_cache_config.p_table_memory == 0)
	{
		// Set the pointer to memory in the cache config.
		m_p_flash->m_cache_config.p_table_memory = m_p_flash->m_p_cache_memory;
		
		// Calculate the maximum number of pages we could have if we used
		// the rest of memory for page frames.
		U32 max_num_page_frames = m_p_flash->m_size_cache_memory / Flash_Address::Bytes_Per_Page();
		
		// Set the number of pages in the cache config.
		m_p_flash->m_cache_config.num_pages = max_num_page_frames;
		
		// Ask the cache manager how much memory it needs for internal tables.
		U32 memory_size_cache_manager;
		CM_Get_Memory_Size_Required(&m_p_flash->m_cache_config, &memory_size_cache_manager);
		
		// Calculate the address of the first page frame.
		// Set the cache config to point to the memory for pages.
		// Align the memory on a 64-byte boundary for DMA.
		m_p_flash->m_cache_config.p_page_memory = (char *)ALIGN(
			m_p_flash->m_cache_config.p_table_memory + memory_size_cache_manager, 64);
		
		// Calculate the actual number of page frames that we have room for.
		U32 num_page_frames = (m_p_flash->m_size_cache_memory - memory_size_cache_manager
			- 64) // minus 64 for alignment
			/ Flash_Address::Bytes_Per_Page();
		
		// Set the actual number of page frames in the cache config.
		m_p_flash->m_cache_config.num_pages = num_page_frames;
	}

	// Overwrite page size with the size of the page used by the device.
	m_p_flash->m_cache_config.page_size = Flash_Address::Bytes_Per_Page();

	// Initialize the cache manager.
	Status status = CM_Initialize(&m_p_flash->m_cache_config,
		0, // &FF_Cache_Context::Prefetch_Callback -- no prefetch for now
		&FF_Cache_Context::Write_Cache_Callback,
		&m_p_flash->m_cache_handle,
		m_p_flash // callback_context
		);

	return status;

} // Open_Cache
	
/*************************************************************************/
// Write_Cache_Callback
// This procedure is called when the Cache Manager needs a page to be
// written from a page frame to the user's storage unit. 
// The Cache Manager passes us a virtual flash address.
// We must map this to a real flash address.
/*************************************************************************/
Status FF_Cache_Context::Write_Cache_Callback(
	void *p_callback_context, 
	I64 virtual_address,
	void *p_page_frame,
	CM_PAGE_HANDLE page_handle)
{
	TRACE_ENTRY(FF_Cache_Context::Write_Cache_Callback);

	// Pointer to FF_Interface was passed to the cache manager CM_Initialize
	// method.  This pointer is passed back to us by the Write callback.
	FF_Interface *p_flash = (FF_Interface *)p_callback_context;
 		
	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;

	// Allocate FF_Cache_Context.  This is the context that will be
	// scheduled to run when the write has completed.
	FF_Cache_Context *p_cache_context = 
		(FF_Cache_Context *)Callback_Context::Allocate(sizeof(FF_Cache_Context));
	if (p_cache_context == 0)
	{
		p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Save callback cache parameters in context.
	p_cache_context->m_p_flash = p_flash;
	p_cache_context->m_cache_handle = p_flash->m_cache_handle;
	p_cache_context->m_page_handle = page_handle;
	p_cache_context->m_p_page_frame = p_page_frame;

 	// Get real address in flash for this page. 
	Flash_Address flash_address =  p_page_map->Get_Real_Flash_Address(
		(U32)virtual_address);
	p_cache_context->m_flash_address = flash_address;
 	 	
	// Set method to call when write operation has completed.
	p_cache_context->Set_Callback(&Write_Cache_Callback_Complete);
	
	// Make sure we are not writing to a bad page.
	int is_page_bad = p_page_map->Is_Page_Bad(p_cache_context->m_flash_address);
	Flash_Address real_SSD_replacement_address;
	if (is_page_bad)
	{
		// This page is marked bad.  This happens if we tried to write
		// to this page before, got a write error, and were not able to
		// get a replacement page.
		// Try again to get a replacement page.
		Status status = p_page_map->Get_Replacement_Page(
			p_cache_context->m_flash_address, &real_SSD_replacement_address);
		if (status != OK)
		{
			// We were still unable to get a replacement page.
			return status;
		}

 		// Save the replacement page address. 
 		p_cache_context->m_flash_address = real_SSD_replacement_address;
	}

	// Make sure we are writing to an erased page.
	int is_page_erased = p_page_map->Is_Page_Erased(p_cache_context->m_flash_address);
	Status get_erased_page_status = OK;
	Flash_Address flash_address_erased;
	if (!is_page_erased)
	{
		// Get an erased page for this data.
		// This remaps the page to a new page that has been erased.
		get_erased_page_status = 
			p_page_map->Get_Erased_Page((U32)virtual_address);
		if (get_erased_page_status != OK)

			// No erased pages are available.
			return get_erased_page_status;

 		// Get real address in flash for the new erased page. 
 		flash_address_erased = p_page_map->Get_Real_Flash_Address(
			(U32)virtual_address);
		p_cache_context->m_flash_address = flash_address_erased;
 	}

	// Create controller context to write the real page to flash.
	Status status = p_flash->m_controller.Write_Page(
		p_cache_context,
		p_cache_context->m_flash_address,
		p_page_frame,
		FF_RETRY_WRITE_IF_ERROR);

	if (status != OK)
	{
		// If write context was not successfully started, terminate it.
		Callback_Context::Terminate(p_cache_context, status);

		// NB: If we return a bad status to the cache manager, the cache manager
		// will close the page.  Hence, we must NOT close or abort the page.
	}

	// Return to the cache manager.
	return status;

} // FF_Cache_Context::Write_Cache_Callback
		

/*************************************************************************/
// FF_Context::Write_Cache_Callback_Complete
// Called when write page has completed.
// If the write failed, the Write routine got a replacement page
// and retried the write.
/*************************************************************************/
void FF_Cache_Context::Write_Cache_Callback_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Cache_Context::Write_Cache_Callback_Complete);
 	
	FF_Cache_Context *p_cache_context = (FF_Cache_Context *)p_context;
	
	// Check status of write operation
	if (status != OK)

		CM_Abort_Page(p_cache_context->m_cache_handle, p_cache_context->m_page_handle);
	else
	
		// The write was successful
		CM_Close_Page(p_cache_context->m_cache_handle, p_cache_context->m_page_handle);

	// Deallocate the context
	Callback_Context::Terminate(p_cache_context, status);
	return;

} // FF_Cache_Context::Write_Cache_Callback_Complete

/*************************************************************************/
// FF_Interface::Get_Cache_Statistics
// Get CM_STATISTICS record
/*************************************************************************/
Status FF_Interface::Get_Cache_Statistics(
	CM_STATISTICS *p_statistics_buffer,
	U32 size_buffer)
{
	return CM_Get_Statistics(m_cache_handle, p_statistics_buffer, size_buffer);

} // FF_Get_Cache_Statistics

/*************************************************************************/
// FF_Interface::Get_Cache_Event_Data
// Get CM_EVENT_DATA record
/*************************************************************************/
Status FF_Interface::Get_Cache_Event_Data(
	CM_EVENT_DATA *p_event_buffer,
	U32 size_buffer)
{
	return CM_Get_Event_Data(m_cache_handle, p_event_buffer, size_buffer);

} // FF_Get_Cache_Event_Data

/*************************************************************************/
// FF_Interface::Get_Cache_Config
// Get CM_Config record
/*************************************************************************/
Status FF_Interface::Get_Cache_Config(
	CM_CONFIG *p_config_buffer,
	U32 size_buffer)
{
	return CM_Get_Config(m_cache_handle, p_config_buffer, size_buffer);

} // FF_Get_Cache_Config


