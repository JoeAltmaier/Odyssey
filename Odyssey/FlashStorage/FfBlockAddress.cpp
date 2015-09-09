/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBlockAddress.cpp
// 
// Description:
// This file implements the bad blocks methods 
// for the flash file system 
// 
// 4/19/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfBlockAddress.h"
#include "FfCommon.h"
#include "FfController.h"
#include "FfInterface.h"
#include "FfStats.h"
#include <String.h>

// Temporary switch for creating a quick bad block table.
// Only for devices where we have already destroyed the bad block markers.
//#define FAKE_BAD_BLOCK_TABLE

//#define MAKE_BIG_QUICK_BAD_BLOCK_TABLE

/*************************************************************************/
// Address_Toc_Copy_1_Last 
// returns the last flash address of the first copy of the Table of Contents.
/*************************************************************************/
Flash_Address FF_Block_Address::Address_Toc_Copy_1_Last()
{
	// Calculate last block of toc.
	U32 num_blocks = m_p_flash->m_page_map.Num_Blocks_Toc() +
		 m_p_flash->m_page_map.Num_Blocks_Toc_Reserve();

	return Address_Last(m_address_toc_copy_1, num_blocks);
}

/*************************************************************************/
// Address_Toc_Copy_2_Last 
// returns the last flash address of the first copy of the Table of Contents.
/*************************************************************************/
Flash_Address FF_Block_Address::Address_Toc_Copy_2_Last()
{
	// Calculate last block of toc.
	U32 num_blocks = m_p_flash->m_page_map.Num_Blocks_Toc() +
		 m_p_flash->m_page_map.Num_Blocks_Toc_Reserve();

	return Address_Last(m_address_toc_copy_2, num_blocks);
}

/*************************************************************************/
// FF_Block_Address::Get_Flash_Page_Map
// Given a flash address, return a flash page map.
// This maps each device page to a flash address.  Note that Flash_Page_Map
// is an array of words, so we are passing in a reference to an array.
/*************************************************************************/
Status FF_Block_Address::Get_Flash_Page_Map(Flash_Address flash_address, 
	Flash_Page_Map flash_page_map, U32 flags)
{
	Flash_Device *p_device = m_p_flash->m_p_device;

	// Get the unit index from the flash address to select the block map table.
	U32 unit_index = flash_address.Unit_Index();

	// Convert the FF_Block_Map_Entry into a flash page map.
	m_block_map_table[unit_index].Get_Flash_Page_Map(flash_address,
		flash_page_map);

	// If writing the bad block table, or reading, don't check.
	if (flags & FF_WRITING_BAD_BLOCK_TABLE)
		return OK;

	// Check to see if we are overwriting the bad block table.
	U32 column = flash_address.Column();
	Flash_Address bad_block_table_copy_1;
	Flash_Address bad_block_table_copy_2;
	if (flash_address.Array())
	{
		// Check array 1 bad block addresses.
		bad_block_table_copy_1 = Address_Bad_Block_Table_Array_1_Copy_1(FF_Mode_Create);
		bad_block_table_copy_2 = Address_Bad_Block_Table_Array_1_Copy_2(FF_Mode_Create);
	}
	else
	{
		// Check array 0 bad block addresses.
		bad_block_table_copy_1 = Address_Bad_Block_Table_Array_0_Copy_1(FF_Mode_Create);
		bad_block_table_copy_2 = Address_Bad_Block_Table_Array_0_Copy_2(FF_Mode_Create);
	}

	// See if the column matches the column of either copy of the bad block table.
	U32 bad_block_number;
	if (column == bad_block_table_copy_1.Column())
	{
		bad_block_number = bad_block_table_copy_1.Block();
	}
	else 
	{
		if (column == bad_block_table_copy_2.Column())
			bad_block_number = bad_block_table_copy_2.Block();
		else
			return OK;
	}

	// The column does match.
	// Check each of the block addresses created by Get_Flash_Page_Map.
	U32 num_address_bits_for_sector = Flash_Address::Num_Address_Bits_For_Sector();
	U32 page_address = bad_block_number << num_address_bits_for_sector;
	U32 num_device_pages = Flash_Address::Device_Pages_Per_Page();
	for (U32 device_address = 0; device_address < num_device_pages; device_address++)
	{
		if (flash_page_map[device_address] == page_address)
			return FF_ERROR(WRITING_BAD_BLOCK_TABLE);
	}
	return OK;

} // FF_Block_Address::Get_Flash_Page_Map

/*************************************************************************/
// FF_Block_Map_Entry::Get_Flash_Page_Map
// Called from FF_Block_Map_Table::Get_Flash_Page_Map to get the flash
// page map that corresponds to this FF_Block_Map_Entry.
// The flash page map is encoded in the FF_Block_Map_Entry in one doubleword.
/*************************************************************************/
void FF_Block_Map_Entry::Get_Flash_Page_Map(Flash_Address flash_address, 
	Flash_Page_Map flash_page_map)
{
	CT_ASSERT((sizeof(FF_Alternate_Block_Address) == sizeof(U16)), Get_Flash_Page_Map);
	CT_ASSERT(((sizeof(FF_Alternate_Block_Address) * FF_MAX_BAD_SPOTS) == sizeof(U64)), Get_Flash_Page_Map);
	
	U32 block_number = flash_address.Block();
	U32 sector = flash_address.Sector();
	
	// Does this entry have a replacement address?
	U32 block_address;
	U32 bad_spot_index;
	if ((m_alternate_entry[0] == 0) && (m_entry != 0))
	{
		// If the first alternate entry is zero, then we have a replacement address.
		// The whole block is being replaced because the original block had 
		// more than four bad spots.
		// The second alternate entry has the replacement address.
		block_address = m_alternate_block_address[1].Block();

		// The replacement address can have up to two bad spots.
		bad_spot_index = 2;
	} else
	{
		// There is no replacement address.
		block_address = block_number;

		// The replacement address can have up to four bad spots.
		bad_spot_index = 0;
	}

	// Calculate primary page address from block address and sector.
	U32 num_address_bits_for_sector = Flash_Address::Num_Address_Bits_For_Sector();
	CT_ASSERT((block_address < Flash_Address::Blocks_Per_Device()), Get_Flash_Page_Map);
	U32 page_address = (block_address << num_address_bits_for_sector) | sector;

	// Initialize flash address.  Store the primary address in each of
	// the page addresses.
	U32 num_device_pages = Flash_Address::Device_Pages_Per_Page();
	U32 device_address;
	for (device_address = 0; device_address < num_device_pages; device_address++)
		flash_page_map[device_address] = page_address;

	// Check for bad spots.
	while (m_alternate_entry[bad_spot_index])
	{
		// The block has at least one bad spot.

		// Set next alternate address block number
		block_address = m_alternate_block_address[bad_spot_index].Block();

		// Get the device address for the alternate block.
		// device_address is stored in 5 bits in alternate address 
		// of FF_Block_Map_Entry as
		// bbDDD, where b = bank, D = device page.
		device_address = m_alternate_block_address[bad_spot_index].Device();

		// Calculate the page address of the alternate address.
		page_address = block_address << num_address_bits_for_sector | sector;

		// Store the alternate address.
		flash_page_map[device_address] = page_address;

		// Is this the last bad spot?
		if (++bad_spot_index == FF_MAX_BAD_SPOTS)
			break;
	}

#ifdef _DEBUG
	for (device_address = 0; device_address < num_device_pages; 
	device_address++)
	{
		CT_ASSERT((flash_page_map[device_address] < Flash_Address::Pages_Per_Device()), Get_Flash_Page_Map);
	}
#endif

} // Get_Flash_Page_Map

/*************************************************************************/
// Allocate FF_Block_Address object.
// Called once when FF_Interface object is initialized.
/*************************************************************************/
Status FF_Block_Address::Allocate(FF_Interface *p_flash)
{
	// Save pointer to flash interface object.
	m_p_flash = p_flash;

	// Allocate the bad block bitmap.
	// For a device with 2K blocks, the size of the map will be:
	// 2,048 blocks per device * 4 banks per column * 8 bytes/entry
	//	* 2 arrays = 128K bytes
	U32 bad_block_bitmap_num_bytes = Bad_Block_Bitmap_Size();

	m_p_bad_block_bitmap = (FF_Bad_Block_Bit_Map_Entry *)
		m_p_flash->m_mem.Allocate(bad_block_bitmap_num_bytes, ALIGN64);
	if (m_p_bad_block_bitmap == 0)
		return FF_ERROR(NO_MEMORY);
	
	// Calculate number of pages in bitmap so we know how many pages
	// to read or write for each array.
	U32 page_size = Flash_Address::Bytes_Per_Page();
	m_bad_block_bitmap_num_pages_per_array = 
		(Num_Bad_Block_Bitmap_Entries_Per_Array() * sizeof(FF_Bad_Block_Bit_Map_Entry) 
		+ page_size - 1) / page_size;

	// Are we using block map tables?
	// Block map tables are only used when we are interleaving
	// (when we have banks, as in the case of the SSD).
	// For the HBC flash, we do not use block map tables.
	if (Flash_Address::Banks_Per_Array() == 1)
		m_using_block_map_tables = 0;
	else
		m_using_block_map_tables = 1;

	// Create block map tables.  These tables must be created before we do any
	// reads.  The block map tables must exist in order to read pages
	// One entry in block map table for each block.
	U32 num_entries = Flash_Address::Blocks_Per_Device();

	// Allocate memory for each block map table.
	U32 num_units = Flash_Address::Num_Units();
	U32 unit_index;
	for (unit_index = 0; unit_index < num_units; unit_index++)
	{
		void * p_memory = 
			m_p_flash->m_mem.Allocate(num_entries * sizeof(FF_Block_Map_Entry));
		if (p_memory == 0)
			return FF_ERROR(NO_CONTEXT);

		// Allocate the block map table.
		m_block_map_table[unit_index].Allocate(p_memory, num_entries);
	}

	return Allocate_Bad_Spot_Tables();

} // FF_Block_Address::Allocate
	
/*************************************************************************/
// Initialize FF_Block_Address object.
// Called once when FF_Interface object is initialized.
// Called again if device is formatted, including system structures.
/*************************************************************************/
Status FF_Block_Address::Initialize(FF_Interface *p_flash)
{
#ifdef MOVE_BAD_BLOCK_0
	// Assume we won't have to move the bad block table.
	m_if_move_bad_block_table = 0;
#endif
	
	// Set up column where copy 2 of system structures will be stored.
	// If we have more than one column, store system structures in column 1
	// to assure that, in case a device goes bad, it will not affect both copies.
	if (Flash_Address::Columns_Per_Array() > 1)
		m_alternate_column = 1;
	else
		m_alternate_column = 0;

#ifdef CREATE_BAD_BLOCK_0
	// Assume we will create it at block 0.
	m_if_create_bad_block_0 = 1;

	// Assume bad block table starts at block 0 for both array 0 and 1.
	m_address_bad_block_array_0_copy_1.Initialize();
	m_address_bad_block_array_1_copy_1.Initialize();
	m_address_bad_block_array_1_copy_1.Array(1);

	// Assume copy 2 of bad block table starts at block 1 for both array 0 and 1.
	m_address_bad_block_array_0_copy_2.Initialize();
	m_address_bad_block_array_0_copy_2.Block(1);
	m_address_bad_block_array_0_copy_2.Column(m_alternate_column);

	m_address_bad_block_array_1_copy_2.Initialize();
	m_address_bad_block_array_1_copy_2.Array(1);
	m_address_bad_block_array_1_copy_2.Block(1);
	m_address_bad_block_array_1_copy_2.Column(m_alternate_column);
#else
	// Assume we won't create it at block 0.
	m_if_create_bad_block_0 = 0;

	// Assume bad block table starts at block 1 for both array 0 and 1.
	m_address_bad_block_array_0_copy_1.Initialize();
	m_address_bad_block_array_0_copy_1.Block(1);
	m_address_bad_block_array_1_copy_1.Initialize();
	m_address_bad_block_array_1_copy_1.Block(1);
	m_address_bad_block_array_1_copy_1.Array(1);

	// Assume copy 2 of bad block table starts at block 2 for both array 0 and 1.
	m_address_bad_block_array_0_copy_2.Initialize();
	m_address_bad_block_array_0_copy_2.Block(2);
	m_address_bad_block_array_0_copy_2.Column(m_alternate_column);

	m_address_bad_block_array_1_copy_2.Initialize();
	m_address_bad_block_array_1_copy_2.Array(1);
	m_address_bad_block_array_1_copy_2.Block(2);
	m_address_bad_block_array_1_copy_2.Column(m_alternate_column);
#endif

#ifdef MOVE_BAD_BLOCK_0
	// As long as some boards still exist that use block 0.

	// Assume bad block table starts at block 0 for both array 0 and 1.
	m_address_bad_block_array_0_copy_1_block0.Initialize();
	m_address_bad_block_array_1_copy_1_block0.Initialize();
	m_address_bad_block_array_1_copy_1_block0.Array(1);

	// Assume copy 2 of bad block table starts at block 1 for both array 0 and 1.
	m_address_bad_block_array_0_copy_2_block0.Initialize();
	m_address_bad_block_array_0_copy_2_block0.Block(1);
	m_address_bad_block_array_0_copy_2_block0.Column(m_alternate_column);

	m_address_bad_block_array_1_copy_2_block0.Initialize();
	m_address_bad_block_array_1_copy_2_block0.Array(1);
	m_address_bad_block_array_1_copy_2_block0.Block(1);
	m_address_bad_block_array_1_copy_2_block0.Column(m_alternate_column);
#endif

	// Initialize toc addresses.
	m_address_toc_copy_1.Initialize();
	m_address_toc_copy_2.Initialize();

	// Initialize bad block bitmap.
	ZERO(m_p_bad_block_bitmap, Bad_Block_Bitmap_Size());

	// Initialize block map tables.  
	U32 num_entries = Flash_Address::Blocks_Per_Device();

	// Initialize each block map table.
	U32 num_units = Flash_Address::Num_Units();
	U32 unit_index;
	for (unit_index = 0; unit_index < num_units; unit_index++)
	{
		// Initialize the block map table.
		m_block_map_table[unit_index].Initialize(num_entries);
	}

	m_num_bad_blocks = 0;
	m_initialized = 1;
	
	return OK;

} // FF_Block_Address::Initialize
	
/*************************************************************************/
// Initialize_System_Flash_Addresses.
// Called once when bad block table is created and when it is opened.
/*************************************************************************/
Status FF_Block_Address::Initialize_System_Flash_Addresses()
{
	FF_Page_Map *p_page_map = &m_p_flash->m_page_map;

	// Use the page buffer as a block allocation map for blocks
	// allocated on unit 0 and unit 1.  
	// We will use this to allocate the system blocks.
	m_p_block_allocation_map = (char *)m_p_flash->m_p_page_buffer;
	ZERO(m_p_block_allocation_map, Flash_Address::Bytes_Per_Page());

#ifdef MOVE_BAD_BLOCK_0
	// As long as some boards still exist that use block 0.
	// Don't allocate anything that used to be allocated for block 0.

	// m_address_bad_block_array_0_copy_1_block0
	*(m_p_block_allocation_map) = 1;

	// m_address_bad_block_array_0_copy_2_block0
	*(m_p_block_allocation_map + 1 + 
		(m_alternate_column * Flash_Address::Blocks_Per_Device())) = 1;

	// m_address_bad_block_array_1_copy_1_block0
	*(m_p_block_allocation_map + 
		(m_alternate_column * Flash_Address::Blocks_Per_Device())) = 1;

	// m_address_bad_block_array_1_copy_2_block0
	*(m_p_block_allocation_map + 1 +
		(m_alternate_column * Flash_Address::Blocks_Per_Device())) = 1;

#endif
	
#ifdef RESERVE_BAT_BLOCK
#ifndef CREATE_BAD_BLOCK_0

	// Basic assurance test block will be at block 0.
	m_bat_block_number = 0;
	U32 vp_first_bat = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_bat);

	// Map each of the bat blocks.
	Flash_Address flash_address;
	flash_address.Initialize();
	flash_address.Block(m_bat_block_number);
	for (U32 unit = 0; unit < Flash_Address::Num_Units(); unit++)
	{
		// Calculate next virtual address.
		U32 virtual_address = vp_first_bat + (unit * Flash_Address::Sectors_Per_Block());

			// Is this block already assigned as a bad block?
		if (p_page_map->Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BLOCK)
			p_page_map->Map_Virtual_Bat_To_Bad_Block(virtual_address, flash_address);
		else
			p_page_map->Map_System_Block(virtual_address, 
				FF_PAGE_STATE_BAT, flash_address);

		flash_address.Increment_Unit();
	}
#endif
#endif

	// Select the first block that is good across all devices for unit 0
	// for copy 1 of the bad block table for array 0.
	U32 block_number;
	U32 index;

	// We normally don't use block 0.
	U32 if_skip_block0 = 1;

#ifdef CREATE_BAD_BLOCK_0
	// If creating the bad block table at 0 for testing,
	if (m_if_create_bad_block_0)
		if_skip_block0 = 0;
#endif

	Status status = m_bad_spot_table[0].Find_Nth_Good_Block(1 /* n */, &block_number,
		if_skip_block0);
	if (status != OK)
		return status;

	// Make sure the block number is not too far from the beginning;
	// otherwise, we won't find it when we open.
	if (block_number > MAX_BAD_BLOCK_TABLE_RETRY)
		return FF_ERROR(TOO_MANY_BAD_SPOTS);

	// Initialize flash address for copy 1 of the bad block table for array 0.
	m_address_bad_block_array_0_copy_1.Block(block_number);
	U32 virtual_page = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_bad_block_table_array_0_copy_1);
	p_page_map->Map_System_Block(virtual_page, FF_PAGE_STATE_BAD_BLOCK_TABLE, 
		m_address_bad_block_array_0_copy_1);

	// Mark this block allocated in the allocation map.  
	// Also mark any blocks before this block to be allocated.  This prevents a bat
	// block from being allocated in a place that conflicts with something else.
	for (index = 0; index <= block_number; index++)
		*(m_p_block_allocation_map + index) = 1;

	if (m_alternate_column)

		// Copy 2 will be on unit 1.
		// Select the first block that is good across all devices for unit 1
		// for copy 2 of the bad block table for array 0.
		status = m_bad_spot_table[1].Find_Nth_Good_Block(1 /* n */, &block_number,
			if_skip_block0);
	else
		// There is only one unit, so
		// Copy 2 will be on unit 0 along with copy 1.
		// Select the second block that is good across all devices for unit 0
		// for copy 2 of the bad block table for array 0.
		status = m_bad_spot_table[0].Find_Nth_Good_Block(2 /* n */, &block_number,
			if_skip_block0);

	if (status != OK)
		return status;

	// Initialize flash address for copy 2 of the bad block table for array 0.
	m_address_bad_block_array_0_copy_2.Block(block_number);
	m_address_bad_block_array_0_copy_2.Column(m_alternate_column);
	virtual_page = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_bad_block_table_array_0_copy_2);
	p_page_map->Map_System_Block(virtual_page, FF_PAGE_STATE_BAD_BLOCK_TABLE, 
		m_address_bad_block_array_0_copy_2);

	// Mark this block allocated.
	for (index = 0; index <= block_number; index++)
		*(m_p_block_allocation_map + index +
			(m_alternate_column * Flash_Address::Blocks_Per_Device())) = 1;

	// Do we need a bad block table for array 1?
	if (Flash_Address::Num_Arrays() > 1)
	{
		// Select the first block that is good across all devices for 
		// the first unit of array 1.
		status = m_bad_spot_table[Flash_Address::Columns_Per_Array()].
			Find_Nth_Good_Block(1 /* n */, &block_number, if_skip_block0);
		if (status != OK)
			return status;

		// Make sure the block number is not too far from the beginning;
		// otherwise, we won't find it when we open.
		if (block_number > MAX_BAD_BLOCK_TABLE_RETRY)
			return FF_ERROR(TOO_MANY_BAD_SPOTS);

		// Mark this block allocated.  Usually, it will be block 0 for both arrays.
		// If the block number is different for array 1, we want to be sure we
		// don't assign this block as a bat block.
		for (index = 0; index <= block_number; index++)
			*(m_p_block_allocation_map + index) = 1;

		// Initialize flash address for copy 1 of the bad block table for array 1.
		m_address_bad_block_array_1_copy_1.Array(1);
		m_address_bad_block_array_1_copy_1.Block(block_number);
		virtual_page = p_page_map->Get_Virtual_Page_For_Real_Block(
				FF_Page_Map::vp_first_bad_block_table_array_1_copy_1);
		p_page_map->Map_System_Block(virtual_page, FF_PAGE_STATE_BAD_BLOCK_TABLE, 
			m_address_bad_block_array_1_copy_1);

		if (m_alternate_column)

			// Copy 2 will be on unit 1.
			// Select the first block that is good across all devices for unit 1
			// for copy 2 of the bad block table for array 0.
			status = m_bad_spot_table[Flash_Address::Columns_Per_Array() + 1].
				Find_Nth_Good_Block(1 /* n */, &block_number, if_skip_block0);
		else
			// Copy 2 will be on unit 0 along with copy 1.
			// Select the second block that is good across all devices for unit 0
			// for copy 2 of the bad block table for array 0.
			status = m_bad_spot_table[Flash_Address::Columns_Per_Array()].
				Find_Nth_Good_Block(2 /* n */, &block_number, if_skip_block0);

		if (status != OK)
			return status;

		// Initialize flash address for copy 2 of the bad block table for array 1.
		m_address_bad_block_array_1_copy_2.Array(1);
		m_address_bad_block_array_1_copy_2.Block(block_number);
		m_address_bad_block_array_1_copy_2.Column(m_alternate_column);
		virtual_page = p_page_map->Get_Virtual_Page_For_Real_Block(
				FF_Page_Map::vp_first_bad_block_table_array_1_copy_2);
		p_page_map->Map_System_Block(virtual_page, FF_PAGE_STATE_BAD_BLOCK_TABLE, 
			m_address_bad_block_array_1_copy_2);

		// Mark this block allocated.  
		for (index = 0; index <= block_number; index++)
			*(m_p_block_allocation_map + index + 
			(m_alternate_column * Flash_Address::Blocks_Per_Device())) = 1;
	}

	// Allocate blocks for copy 1 of the toc.
	U32 vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_toc_copy_1);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_TOC,
		p_page_map->Num_Blocks_Toc() + p_page_map->Num_Blocks_Toc_Reserve(), 
		&block_number, 
		0); // on unit 0
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_TOC);
	}
	m_address_toc_copy_1.Initialize();
	m_address_toc_copy_1.Block(block_number);

	// Allocate blocks for copy 2 of the toc.
	vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_toc_copy_2);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_TOC,
		p_page_map->Num_Blocks_Toc() + p_page_map->Num_Blocks_Toc_Reserve(), 
		&block_number, 
		m_alternate_column); // on unit 0 or 1
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_TOC);
	}
	m_address_toc_copy_2.Initialize();
	m_address_toc_copy_2.Block(block_number);
	m_address_toc_copy_2.Column(m_alternate_column);

	// Allocate blocks for copy 1 of the page map table.
	vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_page_map_table_copy_1);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_PAGE_MAP_TABLE,
		p_page_map->Num_Blocks_Page_Map_Table() + 
		p_page_map->Num_Blocks_Page_Map_Table_Reserve(), 
		&block_number, 
		0); // on unit 0
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_PAGE_MAP_TABLE);
	}
	m_address_page_map_table_copy_1.Initialize();
	m_address_page_map_table_copy_1.Block(block_number);

	// Allocate blocks for copy 2 of the page map table.
	vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_page_map_table_copy_2);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_PAGE_MAP_TABLE,
		p_page_map->Num_Blocks_Page_Map_Table() + 
		p_page_map->Num_Blocks_Page_Map_Table_Reserve(), 
		&block_number, 
		m_alternate_column); // on unit 0 or 1
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_PAGE_MAP_TABLE);
	}
	m_address_page_map_table_copy_2.Initialize();
	m_address_page_map_table_copy_2.Block(block_number);
	m_address_page_map_table_copy_2.Column(m_alternate_column);

	// Allocate blocks for copy 1 of the page map.
	vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_page_map_copy_1);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_PAGE_MAP,
		p_page_map->Num_Blocks_Page_Map() + p_page_map->Num_Blocks_Page_Map_Reserve(), 
		&block_number, 
		0); // on unit 0
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_PAGE_MAP);
	}

	m_address_page_map_copy_1.Initialize();
	m_address_page_map_copy_1.Block(block_number);

	// Allocate blocks for copy 2 of the page map.
	vp = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_page_map_copy_2);
	status = Allocate_System_Blocks(vp, FF_PAGE_STATE_PAGE_MAP,
		p_page_map->Num_Blocks_Page_Map() + p_page_map->Num_Blocks_Page_Map_Reserve(), 
		&block_number,
		m_alternate_column); // on unit 0 or 1
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_PAGE_MAP);
	}

	m_address_page_map_copy_2.Initialize();
	m_address_page_map_copy_2.Block(block_number);
	m_address_page_map_copy_2.Column(m_alternate_column);

#ifdef RESERVE_BAT_BLOCK
#ifdef CREATE_BAD_BLOCK_0
	// Basic assurance test block will follow page map.
	U32 vp_first_bat = p_page_map->Get_Virtual_Page_For_Real_Block(
			FF_Page_Map::vp_first_bat);
	status = Allocate_System_Blocks(vp_first_bat, FF_PAGE_STATE_BAT, 1, &block_number, 0);
	if (status != OK)
	{
		return FF_ERROR(NO_GOOD_BLOCKS_BAT);
	}
	m_bat_block_number = block_number; 

	// Map each of the other bat blocks.
	Flash_Address flash_address;
	flash_address.Initialize();
	flash_address.Block(m_bat_block_number);
	for (U32 unit = 1; unit < Flash_Address::Num_Units(); unit++)
	{
		// Begin with column 1.  We allocated column 0 the first time.
		flash_address.Increment_Unit();

		// Calculate next virtual address.
		U32 virtual_address = vp_first_bat + (unit * Flash_Address::Sectors_Per_Block());

			// Is this block already assigned as a bad block?
		if (p_page_map->Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BLOCK)
			p_page_map->Map_Virtual_Bat_To_Bad_Block(virtual_address, flash_address);
		else
			p_page_map->Map_System_Block(virtual_address, 
				FF_PAGE_STATE_BAT, flash_address);
	}
#endif
#endif

	return OK;

} // Initialize_System_Flash_Addresses

/*************************************************************************/
// Allocate_System_Blocks.
// Allocate num_blocks consecutive blocks and return pointer to first block.
/*************************************************************************/
Status FF_Block_Address::Allocate_System_Blocks(U32 virtual_page,
	FF_PAGE_STATE page_state, U32 num_blocks, U32 *p_block_number, U32 unit)
{
	FF_Page_Map *p_page_map = &m_p_flash->m_page_map;

	// Point to block allocation map for unit 0 or 1.
	char *p_block_allocation_map = 
		m_p_block_allocation_map + (unit * Flash_Address::Blocks_Per_Device());

	// Find the next unallocated block.
	U32 first_block_number;
	U32 next_block_number = 0;
	U32 num_bad_blocks = 0;

	// Create a flash address that corresponds to the block number so that
	// we can check for bad block.
	Flash_Address flash_address;
	flash_address.Initialize();
	flash_address.Column(unit);
	while (next_block_number < Flash_Address::Blocks_Per_Device())
	{
		// Find the first block number that is not allocated and not bad.
		first_block_number = next_block_number;
		while (true)
		{
			// Is the block allocated?
			if(*(p_block_allocation_map + first_block_number) == 0)
			{
				// The block is not allocated.  See if it's a good block.
				flash_address.Block(first_block_number);
				if (!p_page_map->Is_Page_Bad(flash_address))

					// block is good and not allocated. Exit while loop.
					break;

				// Increment number of bad blocks in this string.
				num_bad_blocks++;
			}

			// Make sure we don't go past the limit.
			if (++first_block_number >= Flash_Address::Blocks_Per_Device())
				return FF_ERROR(NO_GOOD_BLOCKS);
		}

		// first_block_number has block number of first block that is
		// good and not allocated.
		// See if num_block in a row are not allocated.
		next_block_number = first_block_number + 1;
		U32 num_good_blocks = 1;
		while (num_good_blocks < num_blocks)
		{
			// Is the next block allocated?
			if (*(p_block_allocation_map + next_block_number))

				// We did not find enough consecutive blocks.
				// Look for the next free block.
				break;

			// The block is not allocated.  See if it's a good block.
			flash_address.Block(next_block_number);
			if (!p_page_map->Is_Page_Bad(flash_address))

				// block is good and not allocated. 
				// Increment number of good blocks in string.
				num_good_blocks++;

			// Make sure we don't go past the limit.
			if (++next_block_number >= Flash_Address::Blocks_Per_Device())
				return FF_ERROR(NO_GOOD_BLOCKS);
		}

		// did we find enough consecutive blocks?
		if (num_good_blocks == num_blocks)
		{
			// Yes, we found enough consecutive blocks.
			// Mark them allocated and map them.
			next_block_number = first_block_number;
			U32 num_allocated_blocks = 0;
			while (num_allocated_blocks < num_good_blocks)
			{
				flash_address.Block(next_block_number);
				p_page_map->Map_System_Block(virtual_page, page_state, flash_address);
				if (!p_page_map->Is_Page_Bad(flash_address))
				{
					// block is good and not allocated. 
					// Increment number of allocated blocks in string.
					num_allocated_blocks++;
					*(p_block_allocation_map + next_block_number) = 1;

					// Only increment the virtual page if it's a good block.
					// Bad pages get mapped to the range of bad page addresses.
					virtual_page += Flash_Address::Sectors_Per_Block();
				}
				next_block_number++;
			}

			// Return block number of first block allocated.
			*p_block_number = first_block_number;
			return OK;

		} // found enough 

		// We did not find enough good blocks.  
		// next_block_number points to the last block we looked at.
		// Continue the loop, looking at next_block_number
	}

	// We did not find enough consecutive blocks.
	return FF_ERROR_CODE(NO_GOOD_BLOCKS);

} // Allocate_System_Blocks

