/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfDeleteMap.cpp
// 
// Description:
// Implements Flash File Delete Map
//
// 8/17/98 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"
#include <String.h>

/*************************************************************************/
// Globals
/*************************************************************************/
U32					  FF_bit_mask[32] = {
						0x00000001,
						0x00000002,
						0x00000004,
						0x00000008,
						0x00000010,
						0x00000020,
						0x00000040,
						0x00000080,
						0x00000100,
						0x00000200,
						0x00000400,
						0x00000800,
						0x00001000,
						0x00002000,
						0x00004000,
						0x00008000,
						0x00010000,
						0x00020000,
						0x00040000,
						0x00080000,
						0x00100000,
						0x00200000,
						0x00400000,
						0x00800000,
						0x01000000,
						0x02000000,
						0x04000000,
						0x08000000,
						0x10000000,
						0x20000000,
						0x40000000,
						0x80000000};

/*************************************************************************/
// FF_Page_Map::Abort_Block_Erase
// We were not able to open every page in the block to be erased.
// Abort all the pages that were open.  
// We will try again later.
/*************************************************************************/
void FF_Page_Map::Abort_Block_Erase(FF_Erase_Context *p_erase_context)
{	
	U32 index;
	for (index = 0; index < Flash_Address::Sectors_Per_Block(); index++)
	{
		// Is this a page that we made present in the cache?
		if (p_erase_context->m_page_handle[index] != CM_NULL_PAGE_HANDLE)
		{

			// We must have made this page present in the cache.
			// Now close the page.  The page can now be rewritten.
			Status status = CM_Abort_Page(m_p_flash->m_cache_handle, 
				p_erase_context->m_page_handle[index]);

			if (status != OK)
				CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
					"FF_Page_Map::Abort_Block_Erase", 
					"CM_Abort_Page failed",
					status,
					0);
		}
	}

	m_p_flash->m_stats.Inc_Num_Block_Erase_Aborted();

	// Is this erase context for erase threshold?
	if (p_erase_context->m_is_wear_level == 0)

		// We need erased pages.
		// Find another block to erase
		Find_Next_Block_To_Erase();
	else

		// Turn off the erase in progress flag.
		Reset_Erase_In_Progress();

} // Abort_Block_Erase

/*************************************************************************/
// FF_Page_Map::Is_Page_Marked_Deleted
// Return true if page is marked deleted in delete map.
/*************************************************************************/
int FF_Page_Map::Is_Page_Marked_Deleted(Flash_Address flash_address)
{
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_PAGE_MAP_TABLE), Is_Page_Marked_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_TOC), Is_Page_Marked_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_BAD_BLOCK_TABLE), Is_Page_Marked_Deleted);

	// Point to FF_DELETED_MAP_ENTRY for the specified block.
	U32 deleted_map_index = flash_address.Block() 
		+ (flash_address.Unit_Index() * Flash_Address::Blocks_Per_Device());
	FF_DELETED_MAP_ENTRY *p_deleted_map_entry = m_p_deleted_map + deleted_map_index;

	// Return the deleted bit for this page.
	return (p_deleted_map_entry->bit_map & FF_bit_mask[flash_address.Sector()]);

} // Is_Page_Marked_Deleted

/*************************************************************************/
// FF_Page_Map::Mark_Page_Deleted
// Mark page deleted in delete map.
/*************************************************************************/
void FF_Page_Map::Mark_Page_Deleted(Flash_Address flash_address)
{
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_PAGE_MAP_TABLE), Mark_Page_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_TOC), Mark_Page_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_BAD_BLOCK_TABLE), Mark_Page_Deleted);

	// Point to FF_DELETED_MAP_ENTRY for the specified block.
	U32 deleted_map_index = flash_address.Block() + 
		(flash_address.Unit_Index() * Flash_Address::Blocks_Per_Device());

	// Point to deleted map entry for the page specified by flash_address.
	FF_DELETED_MAP_ENTRY *p_deleted_map_entry = m_p_deleted_map + deleted_map_index;

	// Turn on deleted bit for this page.
	CT_ASSERT(((p_deleted_map_entry->bit_map & FF_bit_mask[flash_address.Sector()]) == 0), Mark_Page_Deleted);
	p_deleted_map_entry->bit_map |= FF_bit_mask[flash_address.Sector()];

	// Set deleted state for this page.
	Set_Page_State(flash_address, FF_PAGE_STATE_DELETED);

	// Increment number of deleted pages in this block.
	CT_ASSERT((p_deleted_map_entry->deleted_count 
		< Flash_Address::Sectors_Per_Block()), Mark_Page_Deleted);
	p_deleted_map_entry->deleted_count++;

	// Remove the FF_DELETED_MAP_ENTRY from the list it is currently linked to.
	LIST_REMOVE(&p_deleted_map_entry->list);

	// Link FF_DELETED_MAP_ENTRY to list of entries with the same count.
	LIST_INSERT_TAIL(&m_p_deleted_list[p_deleted_map_entry->deleted_count], &p_deleted_map_entry->list);

} // FF_Page_Map::Mark_Page_Deleted

/*************************************************************************/
// FF_Page_Map::Mark_Page_Not_Deleted
// Mark page not deleted in delete map.
/*************************************************************************/
void FF_Page_Map::Mark_Page_Not_Deleted(Flash_Address flash_address)
{
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_PAGE_MAP), Mark_Page_Not_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_PAGE_MAP_TABLE), Mark_Page_Not_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_TOC), Mark_Page_Not_Deleted);
	CT_ASSERT((Get_Page_State(flash_address) != FF_PAGE_STATE_BAD_BLOCK_TABLE), Mark_Page_Not_Deleted);

	// Point to FF_DELETED_MAP_ENTRY for the specified block.
	U32 deleted_map_index = flash_address.Block() + 
		(flash_address.Unit_Index() * Flash_Address::Blocks_Per_Device());
	FF_DELETED_MAP_ENTRY *p_deleted_map_entry = m_p_deleted_map + deleted_map_index;

	// Turn off deleted bit for this page.
	CT_ASSERT(((p_deleted_map_entry->bit_map & FF_bit_mask[flash_address.Sector() ] )), Mark_Page_Not_Deleted);
	p_deleted_map_entry->bit_map &= ~FF_bit_mask[flash_address.Sector()];

	// Decrement number of deleted pages in this block.
	CT_ASSERT((p_deleted_map_entry->deleted_count), Mark_Page_Not_Deleted);
	p_deleted_map_entry->deleted_count--;

	// Remove the FF_DELETED_MAP_ENTRY from the list it is currently linked to.
	LIST_REMOVE(&p_deleted_map_entry->list);

	// Link FF_DELETED_MAP_ENTRY to list of entries with the same count.
	LIST_INSERT_TAIL(&m_p_deleted_list[p_deleted_map_entry->deleted_count], &p_deleted_map_entry->list);

} // FF_Page_Map::Mark_Page_Not_Deleted

#define FF_ENTRY_NOT_FOUND (U32) -1
/*************************************************************************/
// FF_Page_Map::Find_Block_To_Erase
// Called when we have passed the erase threshold.
// We need to create some more erased pages, and an erase context 
// is not active.
/*************************************************************************/
void FF_Page_Map::Find_Block_To_Erase()
{
 	// The page map must be open
	CT_ASSERT(m_map_is_open, Find_Block_To_Erase);
	CT_ASSERT(Is_Erase_In_Progress(), Find_Block_To_Erase);

	// Find the list that has the most deleted sectors.
	// Search each list -- one list for each count (32 today)
	m_erase_context.m_p_deleted_map_entry = 0;
 	for (m_erase_context.m_deleted_list_index = 32; m_erase_context.m_deleted_list_index > 0; m_erase_context.m_deleted_list_index--)
	{
		if (!LIST_IS_EMPTY(&m_p_deleted_list[m_erase_context.m_deleted_list_index]))
		{
			// Point to the first entry in the list for this count.
			m_erase_context.m_p_deleted_map_entry = (FF_DELETED_MAP_ENTRY *)
				LIST_POINT_HEAD(&m_p_deleted_list[m_erase_context.m_deleted_list_index]);
			break;
		}
	}

	Start_Erase_Context();

} // Find_Block_To_Erase

/*************************************************************************/
// FF_Page_Map::Find_Next_Block_To_Erase
// Called when an erase has been aborted, and we need to find
// another block to erase.
/*************************************************************************/
void FF_Page_Map::Find_Next_Block_To_Erase()
{
 	// The page map must be open
	CT_ASSERT(m_map_is_open, Find_Next_Block_To_Erase);
	CT_ASSERT(Is_Erase_In_Progress(), Find_Next_Block_To_Erase);

#if 0
	// The entry we were working on could have been moved to another list.

	// See if the list we were working on has another entry.
	if (!LIST_ENTRY_IS_LAST(&m_p_deleted_list[m_erase_context.m_deleted_list_index],
		&m_erase_context.m_p_deleted_map_entry->list))
	{
		// Point to the next entry in the list for this count.
		m_erase_context.m_p_deleted_map_entry = (FF_DELETED_MAP_ENTRY *)
			LIST_POINT_NEXT(&m_erase_context.m_p_deleted_map_entry->list);
		Start_Erase_Context();
		return;
	}
#endif

	// Are we at the end of the list?
	if (m_erase_context.m_deleted_list_index == 0)
	{
		// Start from the beginning.
		Find_Block_To_Erase();
		return;
	}

	// Find the next list that has the most deleted sectors.
	// Search each list -- one list for each count (32 today)
	m_erase_context.m_p_deleted_map_entry = 0;
 	for (--m_erase_context.m_deleted_list_index; 
	m_erase_context.m_deleted_list_index > 0; m_erase_context.m_deleted_list_index--)
	{
		if (!LIST_IS_EMPTY(&m_p_deleted_list[m_erase_context.m_deleted_list_index]))
		{
			// Point to the first entry in the list for this count.
			m_erase_context.m_p_deleted_map_entry = (FF_DELETED_MAP_ENTRY *)
				LIST_POINT_HEAD(&m_p_deleted_list[m_erase_context.m_deleted_list_index]);
			Start_Erase_Context();
			return;
		}
	}

	// Did we find another block to erase?
	if (m_erase_context.m_p_deleted_map_entry == 0)
	{
		// Start from the beginning.
		Find_Block_To_Erase();
		return;
	}

} // Find_Next_Block_To_Erase

/*************************************************************************/
// FF_Page_Map::Start_Erase_Context
/*************************************************************************/
void FF_Page_Map::Start_Erase_Context()
{
	CT_ASSERT(Is_Erase_In_Progress(), Start_Erase_Context);

	// Assert that we found an entry to erase.
	CT_ASSERT(m_erase_context.m_p_deleted_map_entry, Find_Block_To_Erase);

	// Assert that this entry has the proper number of deleted entries.
	CT_ASSERT((m_erase_context.m_p_deleted_map_entry->deleted_count), Find_Block_To_Erase);
	CT_ASSERT((m_erase_context.m_p_deleted_map_entry->deleted_count == m_erase_context.m_deleted_list_index), Find_Block_To_Erase);

	// Calculate the index of this FF_DELETED_MAP_ENTRY.
	U32 deleted_map_index = m_erase_context.m_p_deleted_map_entry - m_p_deleted_map;
	U32 index;
	CT_ASSERT((deleted_map_index < Flash_Address::Num_Blocks()), Find_Block_To_Erase);

	// Calculate the unit_index for this entry.
	U32 unit_index = deleted_map_index / Flash_Address::Blocks_Per_Device();

	// Calculate the block for this entry.
	U32 block = deleted_map_index % Flash_Address::Blocks_Per_Device();

	// Calculate the flash address for this entry.
	m_erase_context.m_first_flash_address.Initialize();
	m_erase_context.m_first_flash_address.Unit_Index(unit_index);
	m_erase_context.m_first_flash_address.Block(block);

#ifdef _DEBUG
	// Validate the flash address that we just calculated.
	// Point to FF_DELETED_MAP_ENTRY for the specified block.
	index = m_erase_context.m_first_flash_address.Block() + 
		(m_erase_context.m_first_flash_address.Unit_Index() * Flash_Address::Blocks_Per_Device());

	// Point to deleted map entry for the page specified by flash_address.
	FF_DELETED_MAP_ENTRY *p_deleted_map_entry = m_p_deleted_map + index;

	CT_ASSERT((p_deleted_map_entry == m_erase_context.m_p_deleted_map_entry), Start_Erase_Context);

#endif

	m_erase_context.m_next_flash_address = m_erase_context.m_first_flash_address;
	CT_ASSERT((Get_Page_State(m_erase_context.m_first_flash_address) != FF_PAGE_STATE_TOC), Find_Block_To_Erase);
	CT_ASSERT((Get_Page_State(m_erase_context.m_first_flash_address) != FF_PAGE_STATE_BAD_BLOCK_TABLE), Find_Block_To_Erase);

	// Initialize page m_erase_context.m_deleted_list_index for the erase context.
	m_erase_context.m_page_index = 0;

	// Initialize each page handle.
	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (index = 0; index < sectors_per_block; index++)
		m_erase_context.m_page_handle[index] = CM_NULL_PAGE_HANDLE;

	// The erase context is for erase threshold, not wear level
	m_erase_context.m_is_wear_level = 0;

	// Schedule context to remap each mapped page in the
	// page block to be erased.
	m_erase_context.Set_Callback(&FF_Page_Map::Read_Cache);
	m_erase_context.Make_Ready();

} // Start_Erase_Context

/*************************************************************************/
// FF_Page_Map::Read_Cache
// Each mapped page in the page block to be erased must be read into
// the cache and remapped.
/*************************************************************************/
void FF_Page_Map::Read_Cache(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Page_Map::Read_Cache);

	FF_Erase_Context *p_erase_context = (FF_Erase_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_erase_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_erase_context->m_p_flash;

	// The page map must be open
	CT_ASSERT(p_page_map->m_map_is_open, Read_Cache);

	CT_ASSERT(p_page_map->Is_Erase_In_Progress(), Read_Cache);

	// Check to see if we have paged in the last page in the  
	// block to be erased.
	while (p_erase_context->m_page_index < Flash_Address::Sectors_Per_Block())
	{
		// Get the page state of the next page.
		FF_PAGE_STATE page_state = p_page_map->Get_Page_State(
			p_erase_context->m_next_flash_address);

		// Save the page state before the erase was started.  The page state
		// could change while we are erasing the page.
		p_erase_context->m_page_state[p_erase_context->m_page_index] = page_state;

		// Remember if the page was already erased.
		U8 page_erased = p_page_map->Is_Page_Erased(p_erase_context->m_next_flash_address);
		p_erase_context->m_page_erased[p_erase_context->m_page_index] = page_erased;

		// Check to see if the next page is a bad page.
		if (page_state == FF_PAGE_STATE_BAD_PAGE)
		{
			// This is a bad page,
			// so we won't have to make it present.
			CT_ASSERT((p_page_map->Is_Page_Bad(p_erase_context->m_next_flash_address)), 
				Read_Cache);

			// Increment the address of the next page.
			p_erase_context->m_page_index++;
			p_erase_context->m_next_flash_address.Increment_Page();

			// Check next page in while loop.
			continue;
		}

		// Check to see if the next page has been deleted.
		else if (page_state == FF_PAGE_STATE_DELETED)
		{
			// This page has been deleted,
			// so we won't have to make it present.

			// Validate that this page is marked deleted in the deleted map.
			CT_ASSERT((p_erase_context->m_p_page_map->
				Is_Page_Marked_Deleted(p_erase_context->m_next_flash_address)), 
				Read_Cache);

			// Increment the address of the next page.
			p_erase_context->m_page_index++;
			p_erase_context->m_next_flash_address.Increment_Page();

			// Check next page in while loop.
			continue;
		}

		// Check to see if the next page belongs to the page map.
		else if (page_state == FF_PAGE_STATE_PAGE_MAP)

		{
			// We won't have to make it present.
			// Increment the address of the next page.
			p_erase_context->m_page_index++;
			p_erase_context->m_next_flash_address.Increment_Page();

			// Check next page in while loop.
			continue;
		}

		// Check to see if the next page belongs to the 
		// erased page pool or the replacement page pool.
		else if ((page_state == FF_PAGE_STATE_ERASED)
			|| (page_state == FF_PAGE_STATE_REPLACEMENT))

		{
			// Change the page state to erasing to prevent this
			// page from being assigned while we are erasing.
			p_page_map->Set_Page_State(p_erase_context->m_next_flash_address,
				FF_PAGE_STATE_ERASING);

			// We won't have to make it present.
			// Increment the address of the next page.
			p_erase_context->m_page_index++;
			p_erase_context->m_next_flash_address.Increment_Page();

			// Check next page in while loop.
			continue;
		}

		// Make sure we are dealing with a valid page state.
		else if (page_state == FF_PAGE_STATE_MAPPED)
			;
		else
		{
			// Unexpected page state
			CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
				"FF_Page_Map::Read_Cache", 
				"Unexpected page state",
				0,
				0);
		}

		// Validate that this page is not marked deleted in the deleted map.
			CT_ASSERT((!p_erase_context->m_p_page_map->
				Is_Page_Marked_Deleted(p_erase_context->m_next_flash_address)), 
				Read_Cache);
#if 0
		// Check to see if the next page is already erased.
		if (p_erase_context->m_p_page_map->
			Is_Page_Erased(p_erase_context->m_next_flash_address))
		{
			// This page is already erased, so we won't have to make it present.
			// Increment the address of the next page.
			p_erase_context->m_page_index++;
			p_erase_context->m_next_flash_address.Increment_Page();

			// Check next page in while loop.
			continue;
		}
#endif

		// Get the virtual address of the next page to bring into the cache.
		U32 virtual_address = 
			p_page_map->
			Get_Virtual_Flash_Address(p_erase_context->m_next_flash_address);
		CT_ASSERT((virtual_address <= p_page_map->m_p_toc->vp_last_user_page), Read_Cache);

		// See if page is in the cache.
		status = CM_Open_Page(
			p_flash->m_cache_handle,
			virtual_address,
			CM_OPEN_MODE_REMAP | CM_PRIORITY_RESERVE,
			p_erase_context,
			&p_erase_context->m_p_page_frame,
			&p_erase_context->m_page_handle[p_erase_context->m_page_index]);
					
		switch (status)
		{
			case OK:
			
				// The page is in the cache.
				break;
				
			case CM_ERROR_CACHE_MISS:
				{
					// The page is not in the cache. 
					// Our context has a pointer to an empty page frame.

					// Is this page erased?  A page can be mapped an erased
					// when the page map is first initialized.
					if (!page_erased)
					{
						// Page is not erased, so read it in.
						// Set callback for when the read completes.
						p_erase_context->Set_Callback(&FF_Page_Map::Read_Cache_Complete);

						// Start the read operation to read the real page
						// into the cache.
						status = p_flash->m_controller.Read_Page(
							p_erase_context,
							p_erase_context->m_next_flash_address,
							p_erase_context->m_p_page_frame);

						if (status != OK)
						{
							CT_Log_Error(CT_ERROR_TYPE_FATAL,
								"FF_Page_Map::Read_Cache", 
								"Error reading page",
								status,
								0);
						}
						return;

					} // page is not erased

					// Page is erased; no need to bring it into the cache.
					break;
				}
				
			case CM_ERROR_PAGE_LOCKED:
			
				// Abort the page erase operation.
				p_erase_context->m_p_page_map->
					Abort_Block_Erase(p_erase_context);
				return;
				
			case CM_ERROR_NO_PAGE_FRAMES:
			
				// No page frames are available.
				// Abort the page erase operation.
				p_erase_context->m_p_page_map->
					Abort_Block_Erase(p_erase_context);
				return;
			
			default:
			
				// Unexpected error
				CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
					"FF_Page_Map::Read_Cache", 
					"Unexpected cache error",
					status,
					0);
					
				break;
		} // switch

		// Increment the address of the next page.
		p_erase_context->m_page_index++;
		p_erase_context->m_next_flash_address.Increment_Page();

	} // while

	// All of the mapped pages in the block to be erased have been
	// made present.  
	// Reset the flash address to the first page in the block.
	p_erase_context->m_next_flash_address = p_erase_context->m_first_flash_address;

	// Set the callback for when the erase has completed.
	p_erase_context->Set_Callback(&FF_Page_Map::Erase_Complete);

	// Now we can start the erase operation.
	status = p_flash->m_controller.Erase_Page_Block(
		p_erase_context, 
		p_erase_context->m_first_flash_address);

	
} // FF_Page_Map::Read_Cache

/*************************************************************************/
// FF_Page_Map::Erase_Complete
// The erase operation has completed to erase a page block.
/*************************************************************************/
void FF_Page_Map::Erase_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Page_Map::Erase_Complete);

	FF_Erase_Context *p_erase_context = (FF_Erase_Context *)p_context;

	// Get pointer to page map for this context.
	FF_Page_Map *p_page_map = p_erase_context->m_p_page_map;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_page_map->m_p_flash;

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Erase_Complete",
			"Error erasing page",
			status,
			0);
		p_erase_context->Terminate(p_erase_context, status);
		return;
	}
	
 	// Get page map entry for each page in the block
	Flash_Address next_flash_address = p_erase_context->m_first_flash_address;

	Flash_Address real_SSD_replacement_address;
	U32 index;
	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();

	// Starting with sector 0.
	CT_ASSERT((next_flash_address.Sector() == 0), Erase_Complete);

	// Check every sector in the block we just erased.
	for (index = 0; index < sectors_per_block; index++)
	{
        // Get the page state of this page.
		FF_PAGE_STATE page_state = p_page_map->Get_Page_State(next_flash_address);

		// Find out if the erase failed for this page.
		int bad_page = p_page_map->Is_Page_Bad(next_flash_address);

 		// What was the state of the page before we erased it?
 		switch (page_state)
 		{
 			case FF_PAGE_STATE_BAD_PAGE:

				// We erased a bad page.
				// We don't check to see if we got an error.
				CT_ASSERT((p_page_map->Is_Page_Bad(next_flash_address)), Erase_Complete);

				// The page state did not change. FF_PAGE_STATE_BAD_PAGE is only 
				// set by Get_Replacement_Page or by Assign_Virtual when 
				// the device is initialized.
				CT_ASSERT((p_erase_context->m_page_state[index] == page_state), Erase_Complete);
				break;

 			case FF_PAGE_STATE_PAGE_MAP:
				{

				// We erased a page that is mapped to the page map.
				// Get the virtual address that corresponds to this flash address.
				U32 virtual_page_number = p_page_map->Get_Virtual_Flash_Address(next_flash_address);

				// Calculate the page map index of this page.
				U32 page_map_index = virtual_page_number - p_page_map->m_p_toc->vp_first_page_map_copy_1;

				// Validate that this page is not marked deleted in the deleted map.
				CT_ASSERT((!p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// If this page belongs to the page map, it must already be erased.
				CT_ASSERT((p_page_map->Is_Page_Erased(next_flash_address)), 
					Erase_Complete);

				if (p_erase_context->m_page_state[index] != page_state)
				{
					// This page was a page map page with a lower address
					// in the same block that was exchanged for an erased
					// page in the same block.
					CT_ASSERT((p_erase_context->m_page_state[index] == FF_PAGE_STATE_ERASED), Erase_Complete);
					break;
				}

				// Did this page have an erase error?
				if (bad_page)
				{
					// This page map page must be replaced with a page from the
					// replacement page pool.
					status = 
						p_page_map->Get_Replacement_Page(next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}
				} // bad_page

				break;
				}
 			case FF_PAGE_STATE_ERASED:

				// We erased a page that was in the available portion of the 
				// erased page map.
				// Validate that this page is not marked deleted in the deleted map.
				CT_ASSERT((!p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// The page state did not change.
				CT_ASSERT((p_erase_context->m_page_state[index] == page_state), Erase_Complete);

				// Did this page have an erase error?
				if (bad_page)
				{
					// This erased page must be replaced with a page from the
					// replacement page pool.
					status = p_page_map->
						Get_Replacement_Page(next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}

				} // bad_page

				// We leave the erased page where it was.
				break;

 			case FF_PAGE_STATE_REPLACEMENT:

				// We erased a page that was in the available portion of the 
				// replacement page map.
				// We simply leave the page where it was.

				// Validate that this page is not marked deleted in the deleted map.
				CT_ASSERT((!p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// The page state did not change.
				CT_ASSERT((p_erase_context->m_page_state[index] == page_state), Erase_Complete);

				// Did this page have an erase error?
				if (bad_page)
				{
					// This erased page must be replaced with a different page from the
					// replacement page pool.
					 status = p_page_map->
						Get_Replacement_Page(next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}

				} // bad_page

				break;

 			case FF_PAGE_STATE_MAPPED:

				// Validate that this page is not marked deleted in the deleted map.
				CT_ASSERT((!p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// The page state did not change.  An erased page cannot be assigned
				// during an erase operation of that page.
				CT_ASSERT((p_erase_context->m_page_state[index] == page_state), Erase_Complete);

				// Did this page have an erase error?
				if (bad_page)
				{
					// This mapped page must be replaced with a page from the
					// replacement page pool.
					 status = p_page_map->Get_Replacement_Page(
						next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}

					// Replacement page is already mapped.
					CT_ASSERT((p_page_map->Get_Page_State(real_SSD_replacement_address) 
						== FF_PAGE_STATE_MAPPED), 
						Erase_Complete);
				} // bad_page

				// Exchange this erased page for a different erased page.
				p_page_map->Exchange_Erased_Page(next_flash_address);

				// Is this a page that we made present in the cache?
				// Not if the page was already erased.
				if (p_erase_context->m_page_handle[index] != CM_NULL_PAGE_HANDLE)
				{
					// We must have made this page present in the cache.
					if (p_erase_context->m_page_erased[index])

						// If this page was previously erased, we did not try to read it.
						// Now abort the page.  
						CM_Close_Page(p_erase_context->m_p_flash->m_cache_handle, 
							p_erase_context->m_page_handle[index]);
					else
						// Now close the page.  The page can now be rewritten.
						CM_Close_Page(p_erase_context->m_p_flash->m_cache_handle, 
							p_erase_context->m_page_handle[index]);
				}
				break;

 			case FF_PAGE_STATE_DELETED:

				// The page we erased is mapped to the used erase page table portion
				// of the virtual address space.

				// Validate that this page is marked deleted in the deleted map.
				CT_ASSERT((p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// The page state did not change.  A mapped page cannot be deleted
				// because it is locked during the erase operation.
				CT_ASSERT((p_erase_context->m_page_state[index] == page_state), Erase_Complete);

				// This page was not made present in the cache.
				CT_ASSERT((p_erase_context->m_page_handle[index] == CM_NULL_PAGE_HANDLE), Erase_Complete);

				// Mark this page not deleted in the deleted map.
				p_page_map->Mark_Page_Not_Deleted(next_flash_address);

				// Did this page have an erase error?
				if (bad_page)
				{
					// We erased a deleted page, but we were not able to successfully
					// erase the page.
					// This mapped page must be replaced with a page from the
					// replacement page pool.
					 status = p_page_map->Get_Replacement_Page(
						next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}

					// Add the replacement page to the erased page map.
					p_page_map->Add_Erased_Page(real_SSD_replacement_address);

				} // bad_page
				else
				{
					// Add the erased page to the erased page map.
					p_page_map->Add_Erased_Page(next_flash_address);
				}

				break;
 			
 			case FF_PAGE_STATE_ERASING:

				// The page we erased is mapped to the erased page table 
				// or the replacement page portion of the virtual address space.

				// Validate that this page is not marked deleted in the deleted map.
				CT_ASSERT((!p_page_map->Is_Page_Marked_Deleted(next_flash_address)), 
					Erase_Complete);

				// Validate that the page state did change.
				CT_ASSERT((p_erase_context->m_page_state[index] == FF_PAGE_STATE_ERASED)
					|| (p_erase_context->m_page_state[index] == FF_PAGE_STATE_REPLACEMENT), Erase_Complete);

				// This page was not made present in the cache.
				CT_ASSERT((p_erase_context->m_page_handle[index] == CM_NULL_PAGE_HANDLE), Erase_Complete);

				// Restore its previous page state.
				p_page_map->Set_Page_State(next_flash_address, 
					(FF_PAGE_STATE)p_erase_context->m_page_state[index]);

				// Did this page have an erase error?
				if (bad_page)
				{
					// We were not able to successfully erase the page.
					// This mapped page must be replaced with a page from the
					// replacement page pool.
					 status = p_page_map->Get_Replacement_Page(
						next_flash_address, &real_SSD_replacement_address);
					if (status != OK)
					{
						// TODO
						CT_Log_Error(CT_ERROR_TYPE_FATAL,
							"Erase_Complete", 
							"Invalid page state for erase",
							status,
							0);
					}

				} // bad_page

				break;
 			
 			default:
				status = FF_ERROR(INVALID_PAGE_STATE);
				CT_Log_Error(CT_ERROR_TYPE_FATAL,
					"Erase_Complete", 
					"Invalid page state for erase",
					status,
					0);
 		}

		// Increment to next address in block.
		next_flash_address.Increment_Page();
 	
	} // for

	// We are back at sector 0.
	CT_ASSERT((next_flash_address.Sector() == 0), Erase_Complete);

	p_page_map->VALIDATE_MAP;
 	
	// Turn off the erase in progress flag.
	p_page_map->Reset_Erase_In_Progress();

	// Increment the number of page blocks erased.
	// This statistic is maintained in the Toc for persistence.
	p_page_map->m_p_toc->num_page_blocks_erased++;

	// After each erase, check the wear level threshold.
	p_page_map->Check_Wear_Level_Threshold();

} // FF_Page_Map::Erase_Complete

/*************************************************************************/
// FF_Page_Map::Read_Cache_Complete
// The read operation has completed for a page that we wish to erase.
/*************************************************************************/
void FF_Page_Map::Read_Cache_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(Read_Cache_Complete::Read_Cache_Complete);

	FF_Erase_Context *p_erase_context = (FF_Erase_Context *)p_context;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_erase_context->m_p_page_map->m_p_flash;
	
	CT_ASSERT(p_erase_context->m_p_page_map->Is_Erase_In_Progress(), Read_Cache_Complete);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Page_Map::Read_Cache", 
			"Error reading page",
			status,
			0);
	}
	
	// Increment the address of the next page.
	p_erase_context->m_page_index++;
	p_erase_context->m_next_flash_address.Increment_Page();

	// Schedule context to remap the next page.
	p_erase_context->Set_Callback(&FF_Page_Map::Read_Cache);
	p_erase_context->Make_Ready();

} // FF_Page_Map::Read_Cache_Complete

