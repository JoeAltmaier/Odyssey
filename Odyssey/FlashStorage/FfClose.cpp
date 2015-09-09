/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfClose.cpp
// 
// Description:
// This file implements the FF_Close operation
// 
// 8/31/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "Cache.h"
#include "FfCache.h"
#include "FfCommon.h"
#include "FfPageMap.h"
#include "FfRequest.h"
#include "FfStats.h"

#if 0
// Turn on TRACE_CLOSE
#define TRACE_CLOSE Tracef
#else
// Turn off TRACE_CLOSE -- if 1 then do nothing, else call Tracef
#define TRACE_CLOSE 1? 0 : Tracef  
#endif

/*************************************************************************/
// FF_Interface::Close
// Close everything
/*************************************************************************/
Status FF_Interface::Close(Callback_Context *p_callback_context)
{
	TRACE_ENTRY(FF_Close);
	
	Status status = m_page_map.Close(p_callback_context);

	// Invalidate the cookie so that handle cannot be used.
	m_cookie = 0;

	return status;

} // FF_Interface::Close


/*************************************************************************/
// FF_Page_Map::Close
/*************************************************************************/
Status FF_Page_Map::Close(Callback_Context *p_callback_context)
{
	TRACE_ENTRY(FF_Page_Map::Close);

    m_p_flash->m_state = FF_STATE_CLOSING;

	VALIDATE_MAP;

	// Create child context.
	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)
		p_callback_context->Allocate_Child(sizeof(FF_Page_Map_Context));
	if (p_page_map_context == 0)
	{
		m_p_flash->m_stats.Inc_Num_No_Contexts();
        return FF_ERROR(NO_CONTEXT);
	}

	// Save pointer to page map object in context.
	p_page_map_context->m_p_page_map = this;

	// The next thing to do is to flush the cache.
	// This causes all the pages to be written.
	m_close_state = FLUSHING_CACHE;

	// Set the callback for when the cache has been flushed.
	p_page_map_context->Set_Callback(&Wait_For_Erase);

	// Start the cache flush operation.  
	// When the cache has been flushed, call Wait_For_Erase.
	Status status = CM_Flush_Cache(m_p_flash->m_cache_handle, p_page_map_context);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Cache_Closed", 
			"CM_Flush_Cache failed",
			status,
			0);

		// We must go on!
		Wait_For_Erase(p_page_map_context, status);
	}

	return OK;

} // FF_Page_Map::Close
		
/*************************************************************************/
// Wait_For_Erase
// The cache has been flushed, and all dirty pages written.
// Before the close can proceed, we must wait for any erases in 
// progress to complete; otherwise, the page map would be updated after
// we had written it.
/*************************************************************************/
void FF_Page_Map::Wait_For_Erase(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Wait_For_Erase);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	p_page_map->m_close_state = WAITING_FOR_ERASE;

	// Is there an erase operation in progress?
	if (p_page_map->Is_Erase_In_Progress())
	{
		p_page_map->Wait_For_Erased_Page(p_page_map_context);
		return;
	}

	// All erases have completed.  
	// There are no dirty pages in the page map, so we can close the cache.
	p_page_map->m_close_state = CLOSING_CACHE;

	// Set the callback for when the cache has been closed.
	p_page_map_context->Set_Callback(&Write_Page_Map);

	// Initialize context to write the first copy
	// of the internal structures.
	p_page_map_context->m_copy_count = 1;
	p_page_map_context->m_page_index = FIRST_TIME;

	// Start the cache close operation.  
    // When the cache has been closed, call Write_Page_Map.
    status = CM_Close_Cache(p_page_map->m_p_flash->m_cache_handle, 
		p_page_map_context);
	if (status != OK)
    {
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Cache_Closed", 
			"CM_Flush_Cache failed",
			status,
			0);

        // We must go on!
		Write_Page_Map(p_page_map_context, status);
    }

} // FF_Page_Map::Wait_For_Erase

/*************************************************************************/
// Write_Page_Map
// Write one copy of the page map, depending on m_copy_count (1 or 2).
// As the page map is being written, save each flash address in the
// page map table.
/*************************************************************************/
void FF_Page_Map::Write_Page_Map(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Write_Next_Map);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	p_page_map->m_close_state = WRITING_MAP;

	// Did we write a page last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not write a page last time.
		// This was the first time called.
		if (p_page_map_context->m_copy_count == 1)
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Copy_1();
		else
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Copy_2();

		// Set to write the first page.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We wrote a page the last time we were called.
		// Check status of previous write operation.
		if (status == OK)
		{
			// Save the flash address of the last page written in the
			// page map table.
			*(p_page_map->m_p_page_map_table + p_page_map_context->m_page_index) =
				p_page_map_context->m_flash_address;

			// Increment the page index to the next page.
			p_page_map_context->m_page_index++;
		}
		else
		{
			// If the write was not successful, we simply
			// write again to the next page.
			p_page_map_context->Set_Status(OK);
		}

		// Increment the flash address to the next page.
		p_page_map_context->m_flash_address.Increment_Page();

	} // NOT FIRST_TIME

	// Make sure the next page we are writing to is not a bad page.
	while (p_page_map->Is_Page_Bad(p_page_map_context->m_flash_address))
	{
		// Increment the flash address to the next page.
		p_page_map_context->m_flash_address.Increment_Page();
	}

	// Make sure we have not run out of page map pages.
	// This could happen if we get too many write errors.
	Flash_Address first_block_address;
	if (p_page_map_context->m_copy_count == 1)
		first_block_address = 
			p_flash->m_block_address.Address_Page_Map_Copy_1();
	else
		first_block_address = 
			p_flash->m_block_address.Address_Page_Map_Copy_2();
	U32 last_block_number = first_block_address.Block() +
		FF_Page_Map::Num_Blocks_Page_Map() + FF_Page_Map::Num_Blocks_Page_Map_Reserve();

	if (p_page_map_context->m_flash_address.Block() > last_block_number)
	{
		p_page_map_context->Terminate(p_context, FF_ERROR(NO_GOOD_BLOCKS_PAGE_MAP));
		return;
	}

	// Check to see if we need to write another page.
	if (p_page_map_context->m_page_index < Num_Pages_Page_Map())
	{
		// Get the next memory address to write from.
		p_page_map_context->m_p_page_frame = 
			((char *)p_page_map->m_p_real_to_virtual_map) 
			+ (p_page_map_context->m_page_index * Flash_Address::Bytes_Per_Page());

		// Write the next page of the real to virtual page map.
		status = p_flash->m_controller.Write_Page(
			p_page_map_context,

			// Flash address where next page gets written.
			p_page_map_context->m_flash_address,

			// Address of next page of page map in memory
			p_page_map_context->m_p_page_frame,

			// In case of error, we must choose another page.
			 FF_NO_RETRY_WRITE_IF_ERROR);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Write_Next_Map", 
				"Failed to start write of page map page",
				status,
				0);
		}

		// Return to this callback when page write completes.
		return;
	} 

	// All the page map pages have been written. 
	// Write the corresponding page map table (copy 1 or copy 2).
	p_page_map_context->m_page_index = FIRST_TIME;
	p_page_map_context->Set_Callback(&FF_Page_Map::Write_Page_Map_Table);
	Write_Page_Map_Table(p_page_map_context, OK);

} // Write_Page_Map

/*************************************************************************/
// Write_Page_Map_Table
// Write one copy of the page map table, depending on m_copy_count (1 or 2).
/*************************************************************************/
void FF_Page_Map::Write_Page_Map_Table(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Write_Page_Map_Table);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	p_page_map->m_close_state = WRITING_MAP_TABLE;

	// Did we write a page last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not write a page last time.
		// This was the first time called.
		if (p_page_map_context->m_copy_count == 1)
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Table_Copy_1();
		else
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Table_Copy_2();

		// Set index to write the first page of the page map table.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We wrote a page the last time we were called.
		// Check status of previous write operation.
		if (status != OK)
		{
			// Increment the flash address to the next page.
			p_page_map_context->m_flash_address.Increment_Page();

			// TODO
			// Make sure we don't run out of pages.
		}
		else
		{
			// Save address of page map table page in the toc.
			// We save these addresses in pairs: (copy 1, copy 2)
			U32 index = (2 * p_page_map_context->m_page_index) + 
				p_page_map_context->m_copy_count - 1;
			p_page_map->m_p_toc->page_map_table_flash_address[index] = 
				p_page_map_context->m_flash_address;

			// Increment the flash address to the next page.
			p_page_map_context->m_flash_address.Increment_Page();

			// Increment the page index to the next page.
			p_page_map_context->m_page_index++;
		}

	} // NOT FIRST_TIME

	// Check to see if we need to write another page.
	if (p_page_map_context->m_page_index < p_page_map->m_p_toc->num_pages_page_map_table)
	{
		// Get the next memory address to write from.
		p_page_map_context->m_p_page_frame = 
			((char *)p_page_map->m_p_page_map_table)
			+ (p_page_map_context->m_page_index * Flash_Address::Bytes_Per_Page());

		// Write the next page of the page map table.
		status = p_flash->m_controller.Write_Page(
			p_page_map_context,

			// Flash address where next page gets written.
			p_page_map_context->m_flash_address,

			// Address of next page of page map in memory
			p_page_map_context->m_p_page_frame,

			// In case of error, we must choose another page here.
			 FF_NO_RETRY_WRITE_IF_ERROR);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Write_Page_Map_Table", 
				"Failed to start write of page map page",
				status,
				0);
		}

		// Return to this callback when page write completes.
		return;
	} 

 	// The page map table has been written.
	// Now we need to write the corresponding copy of the Toc.
	if (p_page_map_context->m_copy_count == 1)
		p_page_map_context->m_flash_address = 
			p_flash->m_block_address.Address_Toc_Copy_1();
	else
		p_page_map_context->m_flash_address = 
			p_flash->m_block_address.Address_Toc_Copy_2();

	// Set the callback for when the TOC has been written.
	p_page_map_context->Set_Callback(&FF_Page_Map::Write_TOC_Complete);

	p_page_map->m_close_state = WRITING_TOC;

	// Start write operation for the toc.
	status = p_page_map->m_p_flash->m_controller.Write_Page(
		p_page_map_context,
		p_page_map_context->m_flash_address,
		p_page_map->m_p_toc,
		FF_NO_RETRY_WRITE_IF_ERROR);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Write_Page_Map_Table", 
			"Unable to start write of TOC",
			status,
			0);
	}

} // Write_Page_Map_Table

/*************************************************************************/
// FF_Page_Map::Write_TOC_Complete
// Come here when the TOC has been written.
/*************************************************************************/
void FF_Page_Map::Write_TOC_Complete(void *p_context, Status status)
{   
	TRACE_ENTRY(FF_Page_Map::Write_TOC_Complete);

	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash interface object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	if (p_page_map->Is_Page_Bad(p_page_map_context->m_flash_address))
	{
		// The write failed.
		p_page_map->m_p_flash->m_stats.Inc_Num_Write_Toc_Failed();

		// Set the callback for when the TOC has been erased.
		p_page_map_context->Set_Callback(&FF_Page_Map::Retry_Write_TOC);

		// Prepare to erase this page block and retry.
		Flash_Address flash_address = p_page_map_context->m_flash_address;
		flash_address.Sector(0);
		status = p_flash->m_controller.Erase_Page_Block(
			p_page_map_context,
			flash_address);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Erase_Page_Map", 
				"Failed to start erase of toc for retry",
				status,
				0);
			p_page_map_context->Terminate(p_page_map_context, status);
			return;
		}

		// Retry write when erase has completed.
		return;
	}


	// Have we written both copies?
	if (p_page_map_context->m_copy_count == 1)
	{
		// We have written one copy of the internal structures.
		// When the second copy of the toc and page map have been erased, 
		// we will write out the page map table.
		p_page_map_context->m_copy_count = 2;
		p_page_map_context->m_page_index = FIRST_TIME;
		p_page_map_context->Set_Callback(&FF_Page_Map::Write_Page_Map_Table);

		// Erase the second copy of the TOC and page map.
		status = p_page_map->Erase_Toc_And_Page_Map(p_page_map_context, 2);
		if (status != OK)
		{
			p_page_map_context->Terminate(p_context, status);
			return;
		}
		return;
	}

	// Both copies of the internal structures have been written.
	// Close the device.
	p_page_map->m_p_flash->m_p_device->Close();

	p_page_map->m_close_state = CLOSED;

	p_page_map_context->Terminate(p_context, status);
	return;

} // FF_Page_Map::Write_TOC_Complete

/*************************************************************************/
// FF_Page_Map::Retry_Write_TOC
// Come here when the TOC has been erased for retry.
/*************************************************************************/
void FF_Page_Map::Retry_Write_TOC(void *p_context, Status status)
{   
	TRACE_ENTRY(FF_Page_Map::Retry_Write_TOC);

	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash interface object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	// Increment the flash address to the next page.
	p_page_map_context->m_flash_address.Increment_Page();

	// TODO -- Make sure we don't run past the last page allocated.

	// Set the callback for when the TOC has been written.
	p_page_map_context->Set_Callback(&FF_Page_Map::Write_TOC_Complete);

	// Start write operation for the toc.
	status = p_page_map->m_p_flash->m_controller.Write_Page(
		p_page_map_context,
		p_page_map_context->m_flash_address,
		p_page_map->m_p_toc,
		FF_NO_RETRY_WRITE_IF_ERROR);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Retry_Write_TOC", 
			"Unable to start write of TOC for retry",
			status,
			0);
	}

} // Retry_Write_TOC

/*************************************************************************/
// FF_Page_Map::Erase_Toc_And_Page_Map
// Create a child context to write out the toc and page map table.
// When complete, terminate and schedule the calling context.
/*************************************************************************/
Status FF_Page_Map::Erase_Toc_And_Page_Map(Callback_Context *p_callback_context,
										   U32 copy_count)
{
	TRACE_ENTRY(FF_Page_Map::Erase_Toc_And_Page_Map);

	// Create child context.
	FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)
		p_callback_context->Allocate_Child(sizeof(FF_Page_Map_Context));
	if (p_page_map_context == 0)
	{
		m_p_flash->m_stats.Inc_Num_No_Contexts();
        return FF_ERROR(NO_CONTEXT);
	}

	p_page_map_context->m_copy_count = copy_count;
	p_page_map_context->m_page_index = FIRST_TIME;

	// Save pointer to page map object in context.
	p_page_map_context->m_p_page_map = this;

	// Schedule the erase toc operation.
	p_page_map_context->Set_Callback(&Erase_Toc);
	p_callback_context->Make_Children_Ready();
	return OK;

} // Erase_Toc_And_Page_Map

/*************************************************************************/
// Erase_Toc
// Erase one copy of the toc, depending on m_copy_count (1 or 2).
// This is called when we open to erase copy 1, 
// and when we close to erase copy 2.
/*************************************************************************/
void FF_Page_Map::Erase_Toc(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Erase_Toc);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	// Set the callback to continue here until all blocks have been erased.
	p_page_map_context->Set_Callback(&FF_Page_Map::Erase_Toc);

	// Did we erase a page block last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not erase a page block last time.
		// This was the first time called.
		if (p_page_map_context->m_copy_count == 1)

			// Set to erase copy 1.  We are opening the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Toc_Copy_1();
		else

			// Set to erase copy 2.  We are closing the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Toc_Copy_2();

		// Set to erase the first block.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We erased a page block the last time we were called.
		// Increment the page index to the next page block.
		p_page_map_context->m_page_index++;

		// Increment the flash address to the next block.
		p_page_map_context->m_flash_address.Increment_Block();

	} // NOT FIRST_TIME

	// Skip any bad blocks.
	while (p_page_map->Get_Page_State(p_page_map_context->m_flash_address) == 
		FF_PAGE_STATE_BAD_BLOCK)
	{
		p_page_map_context->m_flash_address.Increment_Block();
	}

	// Check to see if we need to erase another page block.
	U32 num_blocks_toc_total = Num_Blocks_Toc() + Num_Blocks_Toc_Reserve();
	if (p_page_map_context->m_page_index < num_blocks_toc_total)
	{
		// Erase the next page block.
		status = p_flash->m_controller.Erase_Page_Block(
			p_page_map_context,
			p_page_map_context->m_flash_address);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Erase_Page_Map", 
				"Failed to start erase of page map block",
				status,
				0);
			p_page_map_context->Terminate(p_page_map_context, status);
		}

		// Return to this callback when page block erase completes.
		return;
	} 

	// All the toc page blocks have been erased. 
	// Erase the corresponding copy of the page map table.
	p_page_map_context->m_page_index = FIRST_TIME;
	Erase_Page_Map_Table(p_page_map_context, OK);

} // Erase_Toc

/*************************************************************************/
// Erase_Page_Map_Table
// Erase one copy of the page map, depending on m_copy_count (1 or 2).
// This is called when we open to erase copy 1, 
// and when we close to erase copy 2.
/*************************************************************************/
void FF_Page_Map::Erase_Page_Map_Table(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map_Table::Erase_Page_Map_Table);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	// Set the callback to continue here until all blocks have been erased.
	p_page_map_context->Set_Callback(&FF_Page_Map::Erase_Page_Map_Table);

	// Did we erase a page block last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not erase a page block last time.
		// This was the first time called.
		if (p_page_map_context->m_copy_count == 1)

			// Set to erase copy 1.  We are opening the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Table_Copy_1();
		else

			// Set to erase copy 2.  We are closing the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Table_Copy_2();

		// Set to erase the first block.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We erased a page block the last time we were called.
		// Increment the page index to the next page block.
		p_page_map_context->m_page_index++;

		// Increment the flash address to the next block.
		p_page_map_context->m_flash_address.Increment_Block();

	} // NOT FIRST_TIME

	// Skip any bad blocks.
	while (p_page_map->Get_Page_State(p_page_map_context->m_flash_address) == 
		FF_PAGE_STATE_BAD_BLOCK)
	{
		p_page_map_context->m_flash_address.Increment_Block();
	}

	// Check to see if we need to erase another page block.
	U32 num_blocks_page_map_table_total = Num_Blocks_Page_Map_Table() + Num_Blocks_Page_Map_Table_Reserve();
	if (p_page_map_context->m_page_index < num_blocks_page_map_table_total)
	{
		// Erase the next page block.
		status = p_flash->m_controller.Erase_Page_Block(
			p_page_map_context,
			p_page_map_context->m_flash_address);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Erase_Page_Map_Table", 
				"Failed to start erase of page map block",
				status,
				0);
		}

		// Return to this callback when page block erase completes.
		return;
	} 

	// All the page map table blocks have been erased. 
	// Erase the corresponding copy of the page map table.
	p_page_map_context->m_page_index = FIRST_TIME;
	Erase_Page_Map(p_page_map_context, OK);

} // Erase_Page_Map_Table

/*************************************************************************/
// Erase_Page_Map
// Erase one copy of the page map, depending on m_copy_count (1 or 2).
// This is called when we open to erase copy 1, 
// and when we close to erase copy 2.
/*************************************************************************/
void FF_Page_Map::Erase_Page_Map(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Page_Map::Erase_Page_Map);

    FF_Page_Map_Context *p_page_map_context = (FF_Page_Map_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_page_map_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;
	
	// Set the callback to continue here until all blocks have been erased.
	p_page_map_context->Set_Callback(&FF_Page_Map::Erase_Page_Map);

	// Did we erase a page block last time?
	if (p_page_map_context->m_page_index == FIRST_TIME)
	{
		// We did not erase a page block last time.
		// This was the first time called.
		if (p_page_map_context->m_copy_count == 1)

			// Set to erase copy 1.  We are opening the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Copy_1();
		else

			// Set to erase copy 2.  We are closing the flash file system.
			p_page_map_context->m_flash_address = 
				p_flash->m_block_address.Address_Page_Map_Copy_2();

		// Set to erase the first block.
		p_page_map_context->m_page_index = 0;

	} // FIRST_TIME
	else
	{
		// We erased a page block the last time we were called.
		// Increment the page index to the next page block.
		p_page_map_context->m_page_index++;

		// Increment the flash address to the next block.
		p_page_map_context->m_flash_address.Increment_Block();

	} // NOT FIRST_TIME

	// Skip any bad blocks.
	while (p_page_map->Get_Page_State(p_page_map_context->m_flash_address) == 
		FF_PAGE_STATE_BAD_BLOCK)
	{
		p_page_map_context->m_flash_address.Increment_Block();
	}

	// Check to see if we need to erase another page block.
	U32 num_blocks_page_map_total = Num_Blocks_Page_Map() + Num_Blocks_Page_Map_Reserve();
	if (p_page_map_context->m_page_index < num_blocks_page_map_total)
	{
		// Erase the next page block.
		status = p_flash->m_controller.Erase_Page_Block(
			p_page_map_context,
			p_page_map_context->m_flash_address);

		if (status != OK)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Page_Map::Erase_Page_Map", 
				"Failed to start erase of page map block",
				status,
				0);
		}

		// Return to this callback when page block erase completes.
		return;
	} 

	// All the page map blocks have been erased. 
	// Terminate this context and schedule the parent.
	p_page_map_context->Terminate(p_page_map_context, OK);

} // Erase_Page_Map





	
	
	
	
	
	
	
	
	
