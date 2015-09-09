/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfInitialize.cpp
// 
// Description:
// This file implements the FF_Initialize operation
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "Cache.h"
#include "FfCache.h"
#include "FfConfig.h"
#include "FfCommon.h"
#include "FlashDevice.h"
#include "FfRequest.h"

//    Global Data
//    should be compiled once with
//    #define FfData 
//    This causes the data in the module to be declared without the extern
#define FfData

#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"
#include <String.h>

class Flash_Device;

/*************************************************************************/
// FF_Interface::Open
// Open Flash File object.
// If p_flash_handle == FF_CREATE, then create bad block table.
/*************************************************************************/
Status FF_Interface::Open(
	FF_CONFIG *p_flash_config,
	CM_CONFIG *p_cache_config,
	Callback_Context *p_callback_context,
	FF_HANDLE *p_flash_handle)
{
	Status status = OK;
	
 	TRACE_ENTRY(FF_Initialize);
 	
 	CT_ASSERT((sizeof(FF_REAL_TO_VIRTUAL_MAP_ENTRY) == 4), Open);
 	
	// Check memory size required.
	U32 memory_size_required;
	status = FF_Get_Memory_Size_Required(p_flash_config, &memory_size_required);
	if (status != OK)
		return status;

	// Validate memory parameters.
	if (p_flash_config->memory_size < memory_size_required)
		return FF_ERROR(MEM_SIZE_TOO_SMALL);
		
	// Allocate FF_Interface object
	void *p_memory = p_flash_config->p_memory;
	U32 memory_size = p_flash_config->memory_size;
	FF_Interface *p_flash = (FF_Interface *)p_memory;
	p_memory = (char *)p_memory + sizeof(FF_Interface);
	memory_size -= sizeof(FF_Interface);

    p_flash->m_state = FF_STATE_OPENING;
	p_flash->m_cache_handle = 0;

	// Save pointer to device object.
	p_flash->m_p_device = p_flash_config->p_device;

	// Initialize the memory object.  
	// We will use this to allocate all our memory.
	p_flash->m_mem.Initialize(memory_size, p_memory);

	// Get the capacity information.
	// Find out how many flash arrays, banks, columns are installed.
	Flash_Capacity flash_capacity;
	status = p_flash->m_p_device->Get_Capacity(&flash_capacity);
	if (status != OK)
		return status;

	// Initialize the flash capacity information.
	// This depends on how much flash is installed.
	status = Flash_Address::Initialize(&flash_capacity);
	if (status != OK)
		return status;
	
	// Open the device.
	status = p_flash->m_p_device->Open(&p_flash->m_mem);
	if (status != OK)
		return status;

	// Allocate a zero page to verify erased pages.
	p_flash->m_p_erased_page = p_flash->m_mem.Allocate(
		Flash_Address::Bytes_Per_Page(), ALIGN64);
	if (p_flash->m_p_erased_page == 0)
		return FF_ERROR(NO_MEMORY);

	// Initialize zero page.
	// An erased page is all ones.
	memset(p_flash->m_p_erased_page, 0XFF, Flash_Address::Bytes_Per_Page()); 

	// Allocate a page buffer to check for blank pages.
	p_flash->m_p_page_buffer = p_flash->m_mem.Allocate(
		Flash_Address::Bytes_Per_Page(), ALIGN64);
	if (p_flash->m_p_page_buffer == 0)
		return FF_ERROR(NO_MEMORY);

	// Save cache config info.
	// We don't open the cache until we know the size of the user's VM space.
	memcpy(&p_flash->m_cache_config, p_cache_config, sizeof(p_flash->m_cache_config));
 	
	// Initialize and validate configuration data
	// FF_Config_Initialize must come first because others depend on it.
	status = p_flash->Initialize_Config(p_flash_config);
	if (status != OK)
		return status;

	// Save global copy of flash configuration data
	memcpy(&p_flash->m_flash_config, p_flash_config, sizeof(p_flash->m_flash_config));

	// Create objects.  

	// Initialize controller object.
	status = p_flash->m_controller.Initialize(p_flash);
	if (status != OK)
		return status;
	
	// Initialize page map object.
	status = p_flash->m_page_map.Allocate(&p_flash->m_mem, p_flash);
	if (status != OK)
		return status;
	
	// Initialize Table of Contents.
	// We do this here to assign virtual addresses.
	status = p_flash->m_page_map.Initialize_Toc(p_flash_config);
	if (status != OK)
		return status;

	// Reset statistics.
	p_flash->m_stats.Reset_Statistics();
	p_flash->m_stats.Reset_Event_Data();

	// Allocate bad block object.
	status = p_flash->m_block_address.Allocate(p_flash);
	if (status != OK)
		return status;
	
	// Initialize bad block object.
	status = p_flash->m_block_address.Initialize(p_flash);
	if (status != OK)
		return status;
	
	// How much memory is available?
	p_flash->m_size_cache_memory = p_flash->m_mem.Memory_Available();

	// Did the user allocate memory for the cache manager?
	if (p_flash->m_cache_config.p_table_memory == 0)
	{
		// Make sure there is memory left over for the cache manager.
		if (p_flash->m_size_cache_memory < 10000)
		{
			return FF_ERROR(MEM_SIZE_TOO_SMALL);
		}
	}
	p_flash->m_size_cache_memory -= ALIGN64;
		
	// Allocate the rest of memory for cache memory.
	p_flash->m_p_cache_memory = p_flash->m_mem.Allocate(p_flash->m_size_cache_memory, ALIGN64);
	if (p_flash->m_p_cache_memory == 0)
	{
		return FF_ERROR(NO_MEMORY);
	}
	// Create a child context to open the bad block table.
	FF_Request_Context *p_child = (FF_Request_Context *)
		p_callback_context->Allocate_Child(sizeof(FF_Request_Context));
	if (p_child == 0)
	{
		Callback_Context::Terminate(p_child, FF_ERROR(NO_CONTEXT));
		return FF_ERROR(NO_CONTEXT);
	}

	// Save pointer to object in child context.
	p_child->m_p_flash = p_flash;

	if (p_flash_handle == FF_CREATE)
	{
		// Create page map.
		status = p_flash->m_page_map.Create(&p_flash->m_flash_config);
		if (status != OK)
			return status;

		// When bad block table has been created, call Open_Create_Complete
		p_child->Set_Callback(&FF_Request_Context::Open_Create_Complete);

		// Open bad block object to see if it already exists.
		status = p_flash->m_block_address.Open(p_child);
	} 
	else
	{
		// When bad block table has been initialized, call Open_Existing_Complete
		p_child->Set_Callback(&FF_Request_Context::Open_Existing_Complete);

		// Return pointer to FF_Interface object as flash handle
		*p_flash_handle = p_flash;

		// Open and initialize bad block object.
		status = p_flash->m_block_address.Open(p_child);
	}

 	return status;
	
} // FF_Interface::Open

/*************************************************************************/
// FF_Request_Context::Open_Existing_Complete
// Finish initializing when the bad block table has been initialized.
/*************************************************************************/
void FF_Request_Context::Open_Existing_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_Cache);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Initialize the first bad page address so that bad pages
	// encountered can count down.
	p_flash->m_page_map.Initialize_Bad_Page_Addresses();

	// Save status of opening the bad block table so that we can
	// continue even if the bad block table does not exist for
	// testing purposes.
	p_request_context->m_bad_block_status = status;
#if 0
	// Even if we didn't open, continue so that we can erase the device if necessary.
	if (status != OK)
	{
		if (status != FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST))
		{
			Abort_Open(p_request_context, status);
			return;
		}
	}
#endif
	
	// Set the callback for when the page map has been opened.
	p_request_context->Set_Callback(&Page_Map_Opened);

	// Validate FF_Interface object as flash handle.
	// We do this even if the open of the page map fails, so that
	// we can format the device if necessary.
	p_flash->m_cookie = FF_INTERFACE_COOKIE;

	// Start the page map open operation.  
    // When the page map has been opened, call Page_Map_Opened.
    status = p_flash->m_page_map.Open(p_request_context);
	if (status != OK)
	{
		Abort_Open(p_request_context, status);
		return;
	}

} // FF_Request_Context::Open_Existing_Complete

/*************************************************************************/
// FF_Request_Context::Open_Create_Complete
// Finish creating when the bad block table has been opened.
/*************************************************************************/
void FF_Request_Context::Open_Create_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_Cache);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Check to see if we should run a surface test.
	// This will destroy all data, including the bad block table.
	U32 test_bad_blocks = 0;
	if (p_flash->m_flash_config.erase_all_pages == ERASE_ALL_PAGES)
	{

		// If config.erase_all_pages is set to this secret value, 
		// we test every page by a sequence of erase, program, read,
		// erase program read again.
		// We only do this if the bad block table has been destroyed
		// for some reason.
		test_bad_blocks = 1;
		status = FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST);
	}

	if (status == OK)
	{
		// The bad block table already exists.
		Abort_Open(p_request_context, FF_ERROR_CODE(BAD_BLOCK_TABLE_ALREADY_EXISTS));
		return;
	}
	
	if (status != FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST))
	{
		Abort_Open(p_request_context, status);
		return;
	}

	// Reset context status.
	p_request_context->Set_Status(OK);
	
	// The bad block table does not exist, so now we can create it.
	// When bad block table has been created, call Create_Complete
	p_request_context->Set_Callback(&FF_Request_Context::Create_Complete);

	// Create bad block table.
	status = p_flash->m_block_address.Create(p_request_context, test_bad_blocks);
	if (status != OK)
		Abort_Open(p_request_context, status);

} // FF_Request_Context::Open_Create_Complete

/*************************************************************************/
// FF_Request_Context::Create_Complete
// Finish creating when the bad block table has been opened.
// Now we know the table does not already exist.
/*************************************************************************/
void FF_Request_Context::Create_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Create_Complete);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	if (status != OK)
	{
		Abort_Open(p_request_context, status);
		return;
	}

	// Lay out the virtual addresses for the page map.
	p_flash->m_page_map.Layout_Page_Map(TRUE);

	// Open the cache.
	status = p_flash->m_page_map.Open_Cache();
	if (status != OK)
	{
		Callback_Context::Terminate(p_request_context, status);
		return;
	}

	// Terminate the context when the close completes.
	p_request_context->Set_Callback(&Callback_Context::Terminate);

	// Close the page map and write out all the structures.
	p_flash->m_page_map.Close(p_request_context);

} // FF_Request_Context::Create_Complete


/*************************************************************************/
// FF_Request_Context::Abort_Open
/*************************************************************************/
void FF_Request_Context::Abort_Open(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_Cache);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Close the device.
	p_flash->m_p_device->Close();

	Callback_Context::Terminate(p_request_context, status);

} // Abort_Open


