/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfWearLevel.cpp
// 
// Description:
// Implements Flash Storage wear leveling algorithm.
//
// 6/24/99 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"
#include <String.h>

/*************************************************************************/
// Increment_Wear_Level_Block
// Given the address of the last block erased for wear leveling,
// increment the address by one block.
/*************************************************************************/
Flash_Address FF_Block_Address::Increment_Wear_Level_Block(Flash_Address flash_address_last)
{
	Flash_Address flash_address = flash_address_last;

	while (1)
	{
		// Increment to the next block address.
		flash_address.Increment_Block();

		// Is this a system block? (e.g., toc, page map, bad block, bat block)
		if (m_p_flash->m_page_map.Is_System_Block(flash_address))
			continue;

		// Get the unit index for this address so that we can index into the
		// block map table for the unit.
		U32 unit_index = flash_address.Unit_Index();

		// Get the block number for this address.
		U32 block = flash_address.Block();

		// Find out how many replacement entries this unit has.
		U32 num_replacement_entries = m_block_map_table[unit_index].Get_Num_Replacement_Entries();

		// Calculate the block number of the first replacement entry.
		U32 replacement_block_number = Flash_Address::Blocks_Per_Device() - num_replacement_entries;

		// See if the block number of the next address is in the range
		// of replacement block numbers.
		if (block < replacement_block_number)

			// The next SSD address is the one to be erased.
			break;

		// Increment to the next block address.

	} // while
	
	return flash_address;

} // Increment_Wear_Level_Block

/*************************************************************************/
// Check_Wear_Level_Threshold
// Check to see if it's time to start the wear level algorithm.
/*************************************************************************/
void FF_Page_Map::Check_Wear_Level_Threshold()
{
	// The page map must be open
	CT_ASSERT(m_map_is_open, Check_Wear_Level_Threshold);

	// Don't do any wear leveling while we are closing.
	// Just to make things simple.
	if (m_p_flash->m_state == FF_STATE_CLOSING)
		return;

	// Has the wear level counter gone over the threshold?
 	if (++m_p_toc->wear_level_counter < m_p_toc->wear_level_threshold)
 		return;

	// Is there an erase operation already in progress?
	if (Is_Erase_In_Progress())
 	{
	 	TRACEF( TRACE_L5, (EOL "Check_Wear_Level_Threshold, In progress"));
 		return;
 	}

	TRACEF( TRACE_L5, (EOL "Check_Wear_Level_Threshold, Must erase"));
	
	// Increment the wear level block address.
	m_p_toc->flash_address_wear_level =
		m_p_flash->m_block_address.Increment_Wear_Level_Block(
		m_p_toc->flash_address_wear_level);

	// Reset the wear level counter.
	m_p_toc->wear_level_counter = 0;

	// Increment the number of times the wear level algorithm was invoked.
	m_p_toc->num_wear_level_threshold++;

	m_erase_context.m_first_flash_address = m_p_toc->flash_address_wear_level;
	m_erase_context.m_next_flash_address = m_erase_context.m_first_flash_address;

	// The erase context is for wear level, not erase threshold.
	m_erase_context.m_is_wear_level = 1;

	// Initialize page index for the erase context.
	m_erase_context.m_page_index = 0;

	// Initialize each page handle.
	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (U32 index = 0; index < sectors_per_block; index++)
		m_erase_context.m_page_handle[index] = CM_NULL_PAGE_HANDLE;

	// Schedule context to remap each mapped page in the
	// page block to be erased.
	Set_Erase_In_Progress();
	m_erase_context.Set_Callback(&FF_Page_Map::Read_Cache);
	m_erase_context.Make_Ready();

} // Check_Wear_Level_Threshold

