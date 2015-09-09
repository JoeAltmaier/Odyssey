/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBlockMapTable.cpp
// 
// Description:
// This file implements methods to create and access the block map table.
// for the flash file system 
// 
// 6/4/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfBlockAddress.h"
#include "FfBadSpotTable.h"
#include "FfBlockMapTable.h"
#include "FfInterface.h"

/*************************************************************************/
// Allocate FF_Block_Map_Table object.  FF_Block_Address object has
// an array of FF_Block_Map_Table, one for each device.
// Each FF_Block_Map_Table has an array of FF_Block_Map_Entry.  
// We initialize this array of FF_Block_Map_Entry here.
// Caller allocates memory for num_entries * sizeof(FF_Block_Map_Entry)
/*************************************************************************/
void FF_Block_Map_Table::Allocate(void *p_memory, U32 num_entries)
{
	CT_ASSERT((sizeof(FF_Block_Map_Entry) == sizeof(UI64)), 
		FF_Block_Map_Table::Initialize);

	m_num_entries = num_entries;
	m_num_replacement_entries = 0;
	m_p_block_map_entry = (FF_Block_Map_Entry *)p_memory;

} // Allocate

/*************************************************************************/
// Initialize FF_Block_Map_Table object.  
/*************************************************************************/
void FF_Block_Map_Table::Initialize(U32 num_entries)
{
	m_num_entries = num_entries;
	m_num_replacement_entries = 0;

	// Initialize each entry to map one-to-one.
	for (U32 index = 0; index < num_entries; index++)
		(m_p_block_map_entry + index)->Map_One_To_One();

} // Initialize

/*************************************************************************/
// Create_Block_Map_Tables
// Create a block map table for each unit_index.
// Called when bad block table has been created or opened.
// Mark each replacement block as a bad block.
/*************************************************************************/
Status FF_Block_Address::Create_Block_Map_Tables()
{
	//if (m_using_block_map_tables)
	{
		// Create bad spot tables.  These are intermediate tables used to
		// create the block map tables.  Note that these will be used in
		// case we reformat the device, so we must keep them.
		Status status = Create_Bad_Spot_Tables();
		if (status != OK)
			return status;

		// Create each block map table.
		U32 num_units = Flash_Address::Num_Units();
		U32 unit_index;
		for (unit_index = 0; unit_index < num_units; unit_index++)
		{
			Status status = Create_Block_Map_Table(unit_index);
			if (status != OK)
				return status;
		}

	}

	return Mark_Bad_Blocks();

} // Create_Block_Map_Tables

/*************************************************************************/
// Mark_Bad_Blocks
// Set the bad block bit in the page table for every page in every block
// that is bad.  We do this when we have finished creating the block map
// tables, and just before we format the device, so that we do not
// try to erase bad blocks.
/*************************************************************************/
Status FF_Block_Address::Mark_Bad_Blocks()
{
	if (m_using_block_map_tables)
	{
		// Mark every replacement block as a bad block that cannot be used.
		Mark_Replacement_Blocks_Bad();
	}
	else
	{
		// Mark every bad block as a bad block that cannot be used.
		Mark_Bad_Blocks_Bad();
	}

	return OK;

} // Mark_Bad_Blocks

/*************************************************************************/
// Create_Block_Map_Table
// Create a block map table for the specified unit_index.
/*************************************************************************/
Status FF_Block_Address::Create_Block_Map_Table(U32 unit_index)
{
	// Point to first bad spot entry for this unit_index
	FF_Bad_Spot_Iterator bad_spot_iterator = m_bad_spot_table[unit_index].Begin();

	// Point to last bad spot entry for this unit_index
	FF_Bad_Spot_Iterator last_bad_spot_iterator = m_bad_spot_table[unit_index].End();

	// block_number will be used as an index into the block map table.
	U32 block_number = 0;

	// Iterate through the bad spot table.
	U32 device_address;
	U32 bad_spot_index;
	while (bad_spot_iterator != last_bad_spot_iterator)
	{
		// Get the number of bad spots for the next bad spot entry.
		U32 num_bad_spots = (*bad_spot_iterator).Get_Num_Bad_Spots();

		switch (num_bad_spots)
		{
		case 0:

			// No bad spots.
			// This block gets mapped one-to-one.
			m_block_map_table[unit_index].Map_One_To_One(block_number);
			break;

		case 1:
		case 2:
		case 3:
		case 4:

			// One to four bad spots.
			// Get an alternate block for each bad spot.
			for (bad_spot_index = 0; bad_spot_index < num_bad_spots; bad_spot_index++)
			{
				// Get the device address that is bad for this block.
				// device_address is used to address the device as
				// bbDDD, where b = bank, D = device page
				device_address = (*bad_spot_iterator).Get_Bad_Spot(bad_spot_index);

				// Get an alternate block for this device page.
				U32 alternate_block_number = m_bad_spot_table[unit_index].Get_Alternate_Block(
					device_address, bad_spot_iterator);

				// Did we get a replacement block?
				if (alternate_block_number == 0)

					// There are no more replacement blocks.
					return OK;

				// Store the alternate block in the alternate address.
				m_block_map_table[unit_index].Map_Alternate(block_number, alternate_block_number, device_address,
					bad_spot_index );

				// Point to last bad spot entry for this unit_index. The pointer may have changed
				// since we may have removed an entry to use for replacement spots.
				last_bad_spot_iterator = m_bad_spot_table[unit_index].End();
			}
			break;

		default:

			// The block has more than four bad spots.
			if (Get_Replacement_Block(bad_spot_iterator, unit_index) == 0)

				// There are no more replacement blocks.
				return OK;

			// Point to last bad spot entry for this unit_index. The pointer may have changed
			// since we may have removed an entry to use for replacement spots.
			last_bad_spot_iterator = m_bad_spot_table[unit_index].End();
		}

		// Point to next bad spot entry
		bad_spot_iterator++;

		// Step to the next block entry in the block map table.
		block_number++;
	}

	return OK;

} // Create_Block_Map_Table

/*************************************************************************/
// Get_Replacement_Block
// Map entire block to a replacement block.
// This is called when the bad spot table entry has more than four 
// bad spots.
/*************************************************************************/
int FF_Block_Address::Get_Replacement_Block(FF_Bad_Spot_Iterator bad_spot_iterator,
	U32 unit_index) 
{
	FF_Bad_Spot_Iterator end_iterator = m_bad_spot_table[unit_index].End_Replacement();

	// Get a replacement block for this unit_index.
	FF_Bad_Spot_Iterator replacement_iterator = m_bad_spot_table[unit_index].
		Get_Replacement_Block(bad_spot_iterator);
	if (replacement_iterator == end_iterator)

		// There are no more replacement blocks.
		return 0;

	// Get the block number of the entry we are replacing.
	U32 block_number = (*bad_spot_iterator).Get_Block_Number();

	// Get the replacement block number.
	U32 replacement_block_number = (*replacement_iterator).Get_Block_Number();

	// Map this block number to a replacement block.  This puts the replacement block
	// in the second alternate block entry.  This leaves room for one or two
	// alternate blocks in case the replacement block has any bad spots.
	m_block_map_table[unit_index].Map_Replacement(block_number, replacement_block_number);

	// Get the number of bad spots for the replacement entry.
	U32 num_bad_spots = (*replacement_iterator).Get_Num_Bad_Spots();

	U32 bad_spot_index;
	U32 device_address;
	switch (num_bad_spots)
	{
	case 0:

		// No bad spots.
		break;

	case 1:
	case 2:

		// One or two spots.
		// Get an alternate block for each bad spot.
		for (bad_spot_index = 0; bad_spot_index < num_bad_spots; bad_spot_index++)
		{
			// Get the device address that is bad for this block.
			// device_address is used to address the device as
			// bbDDD, where b = bank, D = device page
			device_address = (*replacement_iterator).Get_Bad_Spot(bad_spot_index);

			// Get an alternate block for this device page.
			U32 alternate_block_number = m_bad_spot_table[unit_index].Get_Alternate_Block(
				device_address, bad_spot_iterator);

			// Did we get a replacement block?
			if (alternate_block_number == 0)

				// There are no more replacement blocks.
				return 0;

			// Store the alternate block in the alternate address.
			m_block_map_table[unit_index].Map_Alternate(block_number, alternate_block_number, device_address,
				bad_spot_index + 2);

		}
		break;

	default:

		CT_ASSERT((num_bad_spots < 3), Get_Replacement_Block);
	}

	return 1;

} // Get_Replacement_Block


/*************************************************************************/
// Mark_Replacement_Blocks_Bad
// Mark every replacement block as a bad block that cannot be used.
/*************************************************************************/
void FF_Block_Address::Mark_Replacement_Blocks_Bad()
{
	// One entry in block map table for each block.
	U32 num_entries = Flash_Address::Blocks_Per_Device();

	U32 num_units = Flash_Address::Num_Units();
	U32 num_replacement_entries;
	FF_Block_Map_Iterator replacement_iterator;
	FF_Block_Map_Iterator end_iterator;

	// Iterate through each block map table.
	for (U32 unit_index = 0; unit_index < num_units; unit_index++)
	{
		// Each block map table has a corresponding bad spot table.
		// Ask the corresponding bad spot table how many entries
		// are replacement entries.  Then the block map table will have
		// the same replacement entries.
		num_replacement_entries = m_bad_spot_table[unit_index].Get_Num_Replacement_Entries();
		m_block_map_table[unit_index].Set_Num_Replacement_Entries(num_replacement_entries);

		// Now that we know how many replacement entries the block map table has,
		// we can iterate through the replacement entries.
		replacement_iterator = m_block_map_table[unit_index].Begin_Replacement();
		end_iterator = m_block_map_table[unit_index].End_Replacement();

		// Calculate the block number of the first replacement entry.
		U32 block_number = Flash_Address::Blocks_Per_Device() - num_replacement_entries;
		while (replacement_iterator != end_iterator)
		{
			// Get flash address for this unit_index and replacement block
			Flash_Address flash_address;
			flash_address.Unit_Index(unit_index);
			flash_address.Block(block_number);

			// If this is a replacement block, it cannot be used.
			m_p_flash->m_page_map.Set_Bad_Block(flash_address);

			// Next entry
			replacement_iterator++;

			// Next block_number
			block_number++;
		}
	}

} // Mark_Replacement_Blocks_Bad

/*************************************************************************/
// Mark_Bad_Blocks_Bad
// Mark every bad block as a bad block that cannot be used.
// We do this as an alternative to using a block map table.
/*************************************************************************/
void FF_Block_Address::Mark_Bad_Blocks_Bad()
{
	// Point to the first entry in the bad block bitmap table.
	FF_Bad_Block_Bit_Map_Entry *p_bitmap_entry = m_p_bad_block_bitmap;

	// bad_block_address will iterate through the bad block bitmap table.
	Flash_Address bad_block_address;

	// entry_index will iterate through each entry in the table.
	U32 entry_index;

	// Test each entry in the bitmap table.
	// If any bit is set, get the corresponding bad block address.
	U32 num_bad_block_bitmap_entries = Num_Bad_Block_Bitmap_Entries();
	for (entry_index = 0; entry_index < num_bad_block_bitmap_entries; entry_index++)
	{
		// Does this entry have a bad block?
		if (*p_bitmap_entry)
		{
			// Set Bad_Block_Entry_Index
			// ABBBBBBBBBBBbb is index into bad block doubleword entry.
			// A = Array, B = Block number, b = bank number.
			bad_block_address.Bad_Block_Entry_Index(entry_index);

			m_p_flash->m_page_map.Set_Bad_Block(bad_block_address);

		} // bad block entry

		// Next bitmap entry
		p_bitmap_entry++;

	} // for

} // Mark_Bad_Blocks_Bad

