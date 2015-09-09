/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBadSpotTable.cpp
// 
// Description:
// This file implements methods to create and access the bad spot table.
// for the flash file system 
// 
// 4/19/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfBadSpotTable.h"
#include "FfBlockAddress.h"
#include "FfInterface.h"

/*************************************************************************/
// Initialize object
// Caller allocates memory for num_entries * sizeof(FF_Bad_Spot_Entry)
/*************************************************************************/
void FF_Bad_Spot_Table::Initialize(void *p_memory, U32 num_entries)
{
	m_num_entries = num_entries;
	m_num_replacement_entries = 0;
	m_p_bad_spot_entry = (FF_Bad_Spot_Entry *)p_memory;

	// Create each FF_Bad_Spot_Entry.
	for (U32 block_number = 0; block_number < num_entries; block_number++)
		*(m_p_bad_spot_entry + block_number) = FF_Bad_Spot_Entry(block_number);

} // Initialize

/*************************************************************************/
// Allocate_Bad_Spot_Tables
/*************************************************************************/
Status FF_Block_Address::Allocate_Bad_Spot_Tables()
{
	// One entry in bad spot table for each block.
	U32 num_entries = Flash_Address::Blocks_Per_Device();

	// Allocate memory for each bad spot table.
	U32 num_units = Flash_Address::Num_Units();
	for (U32 index = 0; index < num_units; index++)
	{
		// Allocate bad spot table for next unit.
		void * p_memory = 
			m_p_flash->m_mem.Allocate(num_entries * sizeof(FF_Bad_Spot_Entry));
		if (p_memory == 0)
			return FF_ERROR(NO_MEMORY);

		// Initialize the bad spot table.
		m_bad_spot_table[index].Initialize(p_memory, num_entries);
	}
	return OK;

} // Allocate_Bad_Spot_Tables

/*************************************************************************/
// Create_Bad_Spot_Tables
// The bad spot table is a temporary table used to construct the replacement 
// map.  A Bad Spot Table is created for each unit that can be addressed by 
// the software.  
// Two arrays * 8 columns per array gives 16 flash units, and 16 Bad Spot Tables.  
// The Bad Spot Table contains one entry for each block, or 2,048 entries
// if the device has 2K blocks.  
/*************************************************************************/
Status FF_Block_Address::Create_Bad_Spot_Tables()
{
	// Point to the first entry in the bad block bitmap table.
	FF_Bad_Block_Bit_Map_Entry *p_bitmap_entry = m_p_bad_block_bitmap;

	// bad_block_address will iterate through the bad block bitmap table.
	Flash_Address bad_block_address;

	// entry_index will iterate through each entry in the table.
	U32 entry_index;

	// bit_index will iterate through the bits of each non-zero entry.
	U32 bit_index;

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

			// Find each bit in the entry that is set, indicating a bad spot.
			// Each bit represents one device block.
			// CCCDDD is used as an index into the doubleword, where
			// C = Column, D = device.
			UI64 bitmap_entry = *p_bitmap_entry;
			for (bit_index = 0; bit_index < 64; bit_index++)
			{
				bad_block_address.Bad_Block_Bit_Index(bit_index);

				if (bitmap_entry & 1)
				{
					// Set bad spot for this block and device.
					// Device_Index is used to address the device as
					// bbDDD, where b = bank, D = device page
					m_bad_spot_table[bad_block_address.Unit_Index()].
						Add_Bad_Spot(bad_block_address.Block(), bad_block_address.Device_Index());
				}

				// Shift to the next bit
				bitmap_entry = bitmap_entry >> 1;

				// Are there any more bits set in this entry?
				if (bitmap_entry == 0)
					break;
			} // for
		} // bad block entry

		// Next bitmap entry
		p_bitmap_entry++;

	} // for

	return OK;

} // Create_Bad_Spot_Tables


/*************************************************************************/
// Get_Alternate_Block for the specified device_address
// Return the block number of an alternate block.
// Return 0 if none are available.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
U32 FF_Bad_Spot_Table::Get_Alternate_Block(U32 device_address,
	FF_Bad_Spot_Iterator bad_spot_iterator)
{
	FF_Bad_Spot_Iterator replacement_iterator = Begin_Replacement();
	FF_Bad_Spot_Iterator end_iterator = End_Replacement();

	// Search the current list of replacement spots.
	U32 alternate_block = 0;
	while (replacement_iterator != end_iterator)
	{
		alternate_block = (*replacement_iterator).Get_Good_Spot(device_address);
		if (alternate_block)
			return alternate_block;

		// Next replacement entry
		replacement_iterator++;
	}

	// We searched the current list and did not find a replacement spot.
	// Add one or more replacement entries.  
	// We add a replacement entry by removing the last entry.

	// bad_spot_iterator points to the entry we are currently accessing.
	// Increment this by one.  If this is equal to the iterator for the
	// first replacement entry, then we cannot allocate another replacement.
	bad_spot_iterator++;

	while (alternate_block == 0)
	{
		// Can we remove another replacement entry?
		if (bad_spot_iterator == replacement_iterator)
			return 0;

		// Get another replacement entry for our list.
		replacement_iterator = Add_Replacement();

		// See if the new replacement entry has the good spot we need.
		alternate_block = (*replacement_iterator).Get_Good_Spot(device_address);
	}

	return alternate_block;

} // Get_Alternate_Block

/*************************************************************************/
// Get_Replacement_Block
// Return the iterator of a replacement block.
// A replacement block cannot have more than two bad spots.
/*************************************************************************/
FF_Bad_Spot_Iterator FF_Bad_Spot_Table::Get_Replacement_Block(
	FF_Bad_Spot_Iterator bad_spot_iterator)
{
	FF_Bad_Spot_Iterator replacement_iterator = Begin_Replacement();
	FF_Bad_Spot_Iterator end_iterator = End_Replacement();

	// Search the current list of replacement spots.
	while (replacement_iterator != end_iterator)
	{
		// See if this entry can be used for a replacement block.
		if ((*replacement_iterator).Get_Replacement_Block_Number())
			return replacement_iterator;

		// Next replacement entry
		replacement_iterator++;
	}

	// We searched the current list and did not find a replacement block.
	// Add one or more replacement entries.  
	// We add a replacement entry by removing the last entry.

	// bad_spot_iterator points to the entry we are currently accessing.
	// Increment this by one.  If this is equal to the iterator for the
	// first replacement entry, then we cannot allocate another replacement.
	bad_spot_iterator++;

	while (bad_spot_iterator != replacement_iterator)
	{
		// Get another replacement entry for our list.
		replacement_iterator = Add_Replacement();

		// See if this entry can be used for a replacement block.
		if ((*replacement_iterator).Get_Replacement_Block_Number())
			return replacement_iterator;
	}

	return end_iterator;

} // Get_Replacement_Block

/*************************************************************************/
// Find_Nth_Good_Block.
// Find the nth block with no bad spots.  This is used to determine
// the block number of the bad spot table.
/*************************************************************************/
Status FF_Bad_Spot_Table::Find_Nth_Good_Block(U32 n, U32 *p_block_number, U32 if_skip_block0)
{
	// Point to first entry in table.
	FF_Bad_Spot_Iterator iterator = Begin();

	// Point past last entry in table.
	FF_Bad_Spot_Iterator end_iterator = End();

	if(if_skip_block0)

		// If we are not using block 0, skip the first entry.
		// We will never return block 0.
		iterator++;

	// Initialize count.
	U32 count = 0;

	// Search for an entry with no bad spots.
	while (iterator != end_iterator)
	{
		// See if this entry can be used for a replacement block.
		if ((*iterator).Get_Num_Bad_Spots() == 0)
		{
			// Increment the count of the number we found.
			count++;

			// Does this match the nth entry called for?
			if (count == n)
			{
				// Return block number of block with no bad spots.
				*p_block_number = (*iterator).Get_Block_Number();
				return OK;
			}
		}

		// Next entry
		iterator++;
	}

	return FF_ERROR(NO_GOOD_BLOCKS);

} // Find_Nth_Good_Block


// TEMPORARY so we can step into with Metrowerks.
/*************************************************************************/
// Get number of replacement entries.
/*************************************************************************/
/* inline */ U32 FF_Bad_Spot_Table::Get_Num_Replacement_Entries() 
{
	return m_num_replacement_entries;
}

/*************************************************************************/
// Begin returns iterator to first entry in table
/*************************************************************************/
/* inline */ FF_Bad_Spot_Iterator FF_Bad_Spot_Table::Begin() 
{
	return &m_p_bad_spot_entry[0];
}

/*************************************************************************/
// Begin_Replacement returns iterator to first replacement entry in table.
// Replacement entries are at the end of the table.
/*************************************************************************/
/* inline */ FF_Bad_Spot_Iterator FF_Bad_Spot_Table::Begin_Replacement() 
{
	return &m_p_bad_spot_entry[m_num_entries - m_num_replacement_entries];
}

/*************************************************************************/
// End_Replacement returns iterator to last replacement entry in table.
/*************************************************************************/
/* inline */ FF_Bad_Spot_Iterator FF_Bad_Spot_Table::End_Replacement() 
{
	return &m_p_bad_spot_entry[m_num_entries];
}

/*************************************************************************/
// End returns iterator to last entry in table.
// This iterator changes as entries are removed from the table
// to create replacement entries.
/*************************************************************************/
/* inline */ FF_Bad_Spot_Iterator FF_Bad_Spot_Table::End() 
{
	return &m_p_bad_spot_entry[m_num_entries - m_num_replacement_entries];
}

/*************************************************************************/
// Add_Replacement adds a replacement and 
// returns iterator to new first replacement entry in table.
/*************************************************************************/
/* inline */ FF_Bad_Spot_Iterator FF_Bad_Spot_Table::Add_Replacement() 
{
	// Add a replacement entry.
	m_num_replacement_entries++;
	return &m_p_bad_spot_entry[m_num_entries - m_num_replacement_entries];
}

/*************************************************************************/
// Add_Bad_Spot for the specified device address.
// device_address is used to address the device as
// bbDDD, where b = bank, D = device page
/*************************************************************************/
/* inline */ void FF_Bad_Spot_Table::Add_Bad_Spot(U32 block_number, U32 device_address)
{
	CT_ASSERT((block_number < (m_num_entries - m_num_replacement_entries)), Add_Bad_Spot);

	// Make sure the block number we think we are adding the bad spot for
	// is the same as the flash address for this entry in the bad spot table.
	CT_ASSERT(((m_p_bad_spot_entry + block_number)->Get_Block_Number() == block_number), Add_Bad_Spot);

	(m_p_bad_spot_entry + block_number)->Add_Bad_Spot(device_address);
}


