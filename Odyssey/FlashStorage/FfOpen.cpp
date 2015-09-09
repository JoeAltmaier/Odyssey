/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfOpen.cpp
// 
// Description:
// This file implements the FF_Open operation
// 
// 8/31/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "Cache.h"
#include "FfCache.h"
#include "FfCommon.h"
#include "FfInterface.h"
#include "FfRequest.h"
#include "FfPageMap.h"
#include "FfStats.h"

#if 0
// Turn on TRACE_OPEN
#define TRACE_OPEN Tracef
#else
// Turn off TRACE_OPEN -- if 1 then do nothing, else call Tracef
#define TRACE_OPEN 1? 0 : Tracef  
#endif

/*************************************************************************/
// FF_Page_Map::Open
// Read the contents of the Table of Contents from flash memory.
// Then read the other internal structures.
/*************************************************************************/
Status FF_Page_Map::Open(Callback_Context *p_callback_context)
{   
	// Initialize all internal structures.
	Initialize();

	// Allocate a child context of the caller's context.
	FF_Page_Map_Context *p_page_map_context = 
		(FF_Page_Map_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Page_Map_Context));
	if (p_page_map_context == 0)
	{
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Save pointer to page map object in context.
	p_page_map_context->m_p_page_map = this;

	// Initialize context to read the first copy
	// of the internal structures.
	p_page_map_context->m_copy_count = 1;
	p_page_map_context->m_page_index = FIRST_TIME;

	// Set the callback for when the TOC has been read.
	p_page_map_context->Set_Callback(&FF_Page_Map::Read_TOC_Complete);

	// Start read operation for the first toc.
	Read_Toc_Copy_1(p_page_map_context, OK);

	return OK;

} // FF_Page_Map::Open

/*************************************************************************/
// FF_Page_Map::Read_Toc_Copy_1
/*************************************************************************/
void FF_Page_Map::Read_Toc_Copy_1(void *p_context, Status status)
{  
	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash interface object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	// Set the callback for when the TOC has been read.
	p_page_map_context->Set_Callback(&FF_Page_Map::Read_Toc_Copy_1);

	// Point to buffer for copy 1 of toc. 
	FF_TOC *p_toc_1 = (FF_TOC *)p_flash->m_p_cache_memory;

	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		p_page_map_context->m_p_page_frame = p_toc_1;
		p_page_map_context->m_flash_address = p_flash->m_block_address.Address_Toc_Copy_1();
		p_page_map_context->m_page_index = 0;
	}
	else 
	{
		// Is the first copy of the toc valid?
		if (p_toc_1->cookie == FF_TOC_COOKIE)
		{
			// Yes, it is valid.
			// Read the second copy of the toc.
			p_page_map_context->m_page_index = FIRST_TIME;
			Read_Toc_Copy_2(p_page_map_context, OK);
			return;
		}

		// The first copy is not valid.
		// Have we tried the maximum number of places?
		if (p_page_map_context->m_flash_address == 
			p_flash->m_block_address.Address_Toc_Copy_1_Last() )
		{
			// We have looked in all the right places for copy 1.
			// Read the second copy of the toc.
			p_page_map_context->m_page_index = FIRST_TIME;
			Read_Toc_Copy_2(p_page_map_context, OK);
			return;
		}

		// Increment the flash address to the next page.
		p_page_map_context->m_flash_address.Increment_Page();
	}

	// Start read operation.
	status = p_flash->m_controller.Read_Page(
		p_page_map_context,
		p_page_map_context->m_flash_address,
		p_page_map_context->m_p_page_frame);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Read_Toc", 
			"Read_Page failed",
			status,
			0);
		p_page_map_context->Terminate(p_page_map_context, status);
	}

} // Read_Toc_Copy_1

/*************************************************************************/
// FF_Page_Map::Read_Toc_Copy_2
/*************************************************************************/
void FF_Page_Map::Read_Toc_Copy_2(void *p_context, Status status)
{  
	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash interface object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	// Set the callback for when the TOC has been read.
	p_page_map_context->Set_Callback(&FF_Page_Map::Read_Toc_Copy_2);

	// Point to buffer for copy 2 of toc.
	FF_TOC *p_toc_2 = (FF_TOC *)((char *)p_flash->m_p_cache_memory 
		+ Flash_Address::Bytes_Per_Page());

	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		p_page_map_context->m_p_page_frame = p_toc_2;
		p_page_map_context->m_flash_address = p_flash->m_block_address.Address_Toc_Copy_2();
		p_page_map_context->m_page_index = 0;
	}
	else 
	{
		// Is the first copy of the toc valid?
		if (p_toc_2->cookie == FF_TOC_COOKIE)
		{
			// Yes, it is valid.
			Read_TOC_Complete(p_page_map_context, OK);
			return;
		}

		// The first copy is not valid.
		// Have we tried the maximum number of places?
		if (p_page_map_context->m_flash_address == 
			p_flash->m_block_address.Address_Toc_Copy_2_Last() )
		{
			// We have looked in all the right places for copy 2.
			Read_TOC_Complete(p_page_map_context, OK);
			return;
		}

		// Increment the flash address to the next page.
		p_page_map_context->m_flash_address.Increment_Page();
	}

	// Start read operation.
	status = p_flash->m_controller.Read_Page(
		p_page_map_context,
		p_page_map_context->m_flash_address,
		p_page_map_context->m_p_page_frame);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Read_Toc", 
			"Read_Page failed",
			status,
			0);
		p_page_map_context->Terminate(p_page_map_context, status);
	}

} // Read_Toc_Copy_2

/*************************************************************************/
// FF_Page_Map::Read_TOC_Complete
// Come here when both copies of the toc have been read in.
/*************************************************************************/
void FF_Page_Map::Read_TOC_Complete(void *p_context, Status status)
{   
	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash interface object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	// We have read two copies of the toc.
	// Point to each of the toc buffers.
	FF_TOC *p_toc_1 = (FF_TOC *)p_flash->m_p_cache_memory;
	FF_TOC *p_toc_2 = (FF_TOC *)((char *)p_flash->m_p_cache_memory 
		+ Flash_Address::Bytes_Per_Page());

	// Is the first copy of the toc valid?
	if (p_toc_1->cookie == FF_TOC_COOKIE)
	{
		// The first copy is valid.  Check the version.
		if (p_toc_1->version != FF_TOC_VERSION)
		{
			p_page_map_context->Terminate(p_context, FF_ERROR(INVALID_TOC_VERSION));
			return;
		}

		// Is the second copy also valid?
		if (p_toc_2->cookie == FF_TOC_COOKIE)
		{
			// The second copy is valid.  Check the version.
			if (p_toc_2->version != FF_TOC_VERSION)
			{
				p_page_map_context->Terminate(p_context, FF_ERROR(INVALID_TOC_VERSION));
				return;
			}

			CT_ASSERT((p_toc_1->serial_number == p_toc_2->serial_number), Read_Toc);

			// We just successfully read the second toc.
			// Both copies are valid.
			p_page_map->m_open_state = OPEN_2;

			// Move the toc into the toc buffer.
			// Note that we move the second copy of the toc because this toc has
			// the addresses for both copies of the page map table.
			// The first copy of the toc has only addresses for the first copy
			// of the page map table.
			memcpy(p_page_map->m_p_toc, p_toc_2, sizeof(FF_TOC));

			// and start reading the page map.
			p_page_map_context->m_page_index = FIRST_TIME;
			Read_Page_Map_Table(p_page_map_context, OK);
			return;

		} // both copies are valid

		// The first toc is valid, but the second is not.
		// This happens when the flash was not completely closed.
		// When the first copy has been written, the second copy is erased
		// and then written, so the second copy did not get written.
		// We have only one valid copy of the internal structures.
		p_page_map->m_open_state = OPEN_1;

		// Continue with the open.
		// Move the first toc into the toc buffer.
		memcpy(p_page_map->m_p_toc, p_toc_1, sizeof(FF_TOC));

		// and start reading the page map table.
		p_page_map_context->m_page_index = FIRST_TIME;
		Read_Page_Map_Table(p_page_map_context, OK);
		return;

	} // first toc is valid.

	// The first copy of the toc is not valid.  
	// Is the second copy valid?
	if (p_toc_2->cookie == FF_TOC_COOKIE)
	{
		// The second copy is valid.  Check the version.
		if (p_toc_2->version != FF_TOC_VERSION)
		{
			p_page_map_context->Terminate(p_context, FF_ERROR(INVALID_TOC_VERSION));
			return;
		}

		// The second copy of toc is valid, but the first is not.
		// This happens when the flash was not closed.
		// When the flash is opened, the first copy is erased.
		// We have only one copy of the internal structures the way they were
		// the last time the flash was opened.
		p_page_map_context->Terminate(p_context, FF_ERROR(NEVER_CLOSED));
		return;

	} // second toc is valid

	// Neither copy of the toc is valid.
	p_page_map_context->Terminate(p_context, FF_ERROR(INVALID_TOC));
	return;

} // Read_TOC_Complete

/*************************************************************************/
// Read_Page_Map_Table
// Read in the page map table from its pages in the toc block.
// The toc tells us the number of pages in the page map table and the address
// of each page.  The page map table is a list
// of all of the flash addresses that contain the page map.
/*************************************************************************/
void FF_Page_Map::Read_Page_Map_Table(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Erase_Page_Map);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	p_page_map_context->Set_Callback(&FF_Page_Map::Read_Page_Map_Table);

	// Did we read a page last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not read a page last time.
		// This was the first time called.
		// Should we read copy 1 or copy 2?
		if (p_page_map->m_open_state == NEVER_CLOSED)
		{
			// Only the second copy is available, and it's the old version
			// that was not updated.
			p_page_map_context->m_copy_count = 2;
			p_flash->m_block_address.Address_Page_Map_Table_Copy_2();
		}
		else
		{
			// The first copy is valid.
			p_page_map_context->m_copy_count = 1;
			p_flash->m_block_address.Address_Page_Map_Table_Copy_1();
		}

		// Set to read the first page.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We read a page the last time we were called.
		// Check status of previous read operation.
		if (status == OK)
		{
			// Increment the page index to the next page.
			p_page_map_context->m_page_index++;
		}
		else
		{
			// The read was not successful.
			// Are both copies of the internal structures available?
			if (p_page_map->m_open_state == OPEN_2)
			{
				// Both copies are available.
				// Have we already tried copy 2?
				if (p_page_map_context->m_copy_count == 2)
				{
					p_page_map_context->Terminate(p_page_map_context, status);
					return;
				}

				// Read copy 2 of the same page.
				p_page_map_context->m_copy_count = 2;
			}
			else
			{
				// Only one copy is available.
				p_page_map_context->Terminate(p_page_map_context, status);
				return;
			}
		}

	} // NOT FIRST_TIME

	// Check to see if we need to read another page.
	if (p_page_map_context->m_page_index < p_page_map->m_p_toc->num_pages_page_map_table)
	{
		// Get the flash address of the next page of the page map table from the toc.
		// We save these addresses in pairs: (copy 1, copy 2)
		U32 index = (2 * p_page_map_context->m_page_index) + 
			p_page_map_context->m_copy_count - 1;
		p_page_map_context->m_flash_address =
			p_page_map->m_p_toc->page_map_table_flash_address[index];
			
		// Validate the flash address of the page map table.
		status = Flash_Address::Validate(p_page_map_context->m_flash_address);
		if (status != OK)
		{
			Tracef("\nBad page map table address %x at index %d", 
				p_page_map_context->m_flash_address, index);
			p_page_map_context->Terminate(p_page_map_context, FF_ERROR(BAD_PAGE_MAP_TABLE_ADDRESS));
			return;
		}

		// Get the next memory address to read into.
		p_page_map_context->m_p_page_frame = 
			((char *)p_page_map->m_p_page_map_table)
			+ (p_page_map_context->m_page_index * Flash_Address::Bytes_Per_Page());

		// Start read operation for the next page of the page map table.
		Status status = p_page_map->m_p_flash->m_controller.Read_Page(
			p_page_map_context,

			// Real address of the next page of the page map table
			p_page_map_context->m_flash_address,

			// Address in memory of the next page that contains
			// the page map table.
			p_page_map_context->m_p_page_frame);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Read_Page_Map_Table", 
				"Read_Page failed",
				status,
				0);
			p_page_map_context->Terminate(p_page_map_context, status);
			return;
		}

		// Return to this callback when page read completes.
		return;
	} 

	// The page map table has been read in. 
	// Initialize context to read the first page of the page map.
	p_page_map_context->m_page_index = FIRST_TIME;
	Read_Page_Map(p_page_map_context, OK);

} // Read_Page_Map_Table

/*************************************************************************/
// Read_Page_Map
// Using the flash addresses from the page map table, read in the page map.
// When the page map has been read in, erase copy 1 of the toc.
/*************************************************************************/
void FF_Page_Map::Read_Page_Map(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Read_Page_Map);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	p_page_map_context->Set_Callback(&FF_Page_Map::Read_Page_Map);

	// Did we read a page last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// Set to read the first page.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We read a page the last time we were called.
		// Check status of previous read operation.
		if (status == OK)
		{
			// Increment the page index to the next page.
			p_page_map_context->m_page_index++;
		}
		else
		{
			// The read was not successful.
			// TODO
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Read_Page_Map", 
				"Failed to read of page map page",
				status,
				0);
			p_page_map_context->Terminate(p_context, status);
			return;
		}
	} // NOT FIRST_TIME

	// Check to see if we need to read another page.
	if (p_page_map_context->m_page_index < Num_Pages_Page_Map())
	{
		// Get the flash address of the next page.
		p_page_map_context->m_flash_address = 
			*(p_page_map->m_p_page_map_table + p_page_map_context->m_page_index);

		// Validate the flash address of the page map.
		status = Flash_Address::Validate(p_page_map_context->m_flash_address);
		if (status != OK)
		{
			Tracef("\nBad page map address %x in page map table at index %d", 
				p_page_map_context->m_flash_address, p_page_map_context->m_page_index);
			p_page_map_context->Terminate(p_page_map_context, FF_ERROR(BAD_PAGE_MAP_ADDRESS));
			return;
		}

		// Get the next memory address to read into.
		p_page_map_context->m_p_page_frame = 
			((char *)p_page_map->m_p_real_to_virtual_map) 
			+ (p_page_map_context->m_page_index * Flash_Address::Bytes_Per_Page());

		// Read the next page of the real to virtual page map.
		status = p_flash->m_controller.Read_Page(
			p_page_map_context,

			// Flash address where next page gets written.
			p_page_map_context->m_flash_address,

			// Address of next page of page map in memory
			p_page_map_context->m_p_page_frame);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Read_Page_Map", 
				"Failed to start read of page map page",
				status,
				0);
		}

		// Return to this callback when page read completes.
		return;
	} 

 	// The real to virtual map has been read in.
	// Create the virtual to real page map
	// from the real to virtual map.
	status = p_page_map->Create_Virtual_To_Real_Map();
	if (status != OK)
	{
		p_page_map_context->Terminate(p_context, status);
		return;
	}

	// Prepare to erase the first copy of the toc and page map.
	// When complete, schedule Page_Map_Opened.
	p_page_map_context->Set_Callback(&FF_Page_Map::Page_Map_Opened);
	p_page_map_context->m_copy_count = 1;
	p_page_map_context->m_page_index = FIRST_TIME;
	status = p_page_map->Erase_Toc_And_Page_Map(p_page_map_context, 1);
	if (status != OK)
	{
		p_page_map_context->Terminate(p_context, status);
		return;
	}

} // Read_Page_Map

/*************************************************************************/
// FF_Page_Map::Page_Map_Opened
// Come here when the first copy of the page map has been erased.
/*************************************************************************/
void FF_Page_Map::Page_Map_Opened(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Page_Map_Opened);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	if (status != OK)
	{
		p_page_map_context->Terminate(p_context, status);
		return;
	}

	// Open the cache.
	// Map the number of pages allocated to the user.
	// We must open the cache before we erase any pages.
	status = p_page_map->Open_Cache();

	if (status != OK)
	{
		p_page_map_context->Terminate(p_context, status);
		return;
	}

	// The page map is now open.
	p_page_map->m_map_is_open = 1;
	
	// Increment the serial number each time the toc is opened.
	p_page_map->m_p_toc->serial_number++;

	switch (p_page_map->m_open_state)
	{
	case OPEN_2:

		// Both copies are valid.
		break;

	case OPEN_1:

		// One copy of the internal structures is valid.
		break;

	case NEVER_CLOSED:

		// The flash was not closed the last time it was opened.
		status = FF_ERROR(NEVER_CLOSED);
		break;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Page_Map_Opened", 
			"Invalid open state",
			status,
			0);
	} 

	p_page_map_context->Terminate(p_context, status);

} // FF_Request_Context::Page_Map_Opened
	
/*************************************************************************/
// FF_Request_Context::Page_Map_Opened
// Come here when the page map has been opened.
/*************************************************************************/
void FF_Request_Context::Page_Map_Opened(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Request_Context::Page_Map_Opened);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Set state to open even if it failed in case we need to format.
	p_request_context->m_p_flash->m_state = FF_STATE_OPEN;

	// Is the bad block table present?
	if (p_request_context->m_bad_block_status == OK)
		p_request_context->Terminate(p_context, status);
	else

		// Bad block table is not present.  Terminate with error,
		// but program will be allowed to continue for debugging
		// and testing purposes.
		p_request_context->Terminate(p_context, p_request_context->m_bad_block_status);
	return;

} // FF_Request_Context::Page_Map_Opened
	


