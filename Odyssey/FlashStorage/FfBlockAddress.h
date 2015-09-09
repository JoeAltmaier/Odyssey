/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfBlockAddress.h
// 
// Description:
// This file defines the bad block table for the Flash File 
// 
// 4/19/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfBlockAddress_H)
#define FfBlockAddress_H

#include "Callback.h"
#include "FlashStorage.h"
#include "FlashDevice.h"
#include "ErrorLog.h"
#include "FlashAddress.h"
#include "FfBadBlockHeader.h"
#include "FfBadSpotTable.h"
#include "FfBlockMapTable.h"
#include "FfController.h"
#include "FfPageMapEntry.h"

#pragma pack(1)

class FF_Interface;
class FF_Bad_Block_Context;

// The maximum number of pages to read searching for the bad block table
// before we give up and declare the bad block table is not on this device.
#define MAX_BAD_BLOCK_TABLE_RETRY 20

// 
#define DO_ERASE 1
#define DO_WRITE 2
#define DO_READ  3

/*************************************************************************/
// FF_Bad_Block_Bit_Map_Entry
// Each FF_Bad_Block_Bit_Map_Entry is a 64-bit word.
// Each bit represents one device block.
// CCCDDD is used as an index into the doubleword, where
// C = Column, D = device.
/*************************************************************************/
typedef UI64 FF_Bad_Block_Bit_Map_Entry;

typedef enum {
	FF_Mode_Open,
	FF_Mode_Create
} FF_Open_Mode;

class FF_Block_Address
{
public: // methods

	Status Allocate(FF_Interface *p_flash);
	Status Initialize(FF_Interface *p_flash);

	Status Open(Callback_Context *p_callback_context);

	Status Create(Callback_Context *p_callback_context,
		U32 test_bad_blocks);

	Flash_Address Address_Last(Flash_Address flash_address,
													U32 num_blocks);
	Flash_Address Address_Toc_Copy_1();
	Flash_Address Address_Toc_Copy_2();
	Flash_Address Address_Toc_Copy_1_Last();
	Flash_Address Address_Toc_Copy_2_Last();
	Flash_Address Address_Page_Map_Copy_1();
	Flash_Address Address_Page_Map_Copy_2();
	Flash_Address Address_Page_Map_Table_Copy_1();
	Flash_Address Address_Page_Map_Table_Copy_2();
	
	Flash_Address Address_Bad_Block_Table_Array_0_Copy_1(FF_Open_Mode open_mode);
	Flash_Address Address_Bad_Block_Table_Array_0_Copy_2(FF_Open_Mode open_mode);
	Flash_Address Address_Bad_Block_Table_Array_1_Copy_1(FF_Open_Mode open_mode);
	Flash_Address Address_Bad_Block_Table_Array_1_Copy_2(FF_Open_Mode open_mode);

	Flash_Address Increment_Wear_Level_Block(Flash_Address flash_address);

	Status Allocate_Block_Map_Tables();
	Status Create_Block_Map_Tables();
	Status Mark_Bad_Blocks();

	Status Get_Flash_Page_Map(Flash_Address flash_address, 
		Flash_Page_Map flash_page_map, U32 flags = FF_WRITING_BAD_BLOCK_TABLE);

	// Return memory size required by object.
	static U32 Memory_Size(FF_CONFIG *p_config);

	Status Initialize_System_Flash_Addresses();

	Status Run_Surface_Test(Callback_Context *p_callback_context, void *p_buffer,
		U32 test_level);
		
	U32 Num_Bad_Blocks();

#ifdef RESERVE_BAT_BLOCK
	U32 Bat_Block_Number();
#endif

private: // helper methods

	Status Allocate_Bad_Spot_Tables();
	Status Create_Bad_Spot_Tables();

	Status Create_Block_Map_Table(U32 index);

	void Mark_Replacement_Blocks_Bad();
	void Mark_Bad_Blocks_Bad();

	void Mark_Block_Bad(Flash_Address flash_address, U32 offset);

	Status Add_Bad_Block(Flash_Address flash_address, U32 device);

	void Erase_Bad_Block_Table(FF_Bad_Block_Context *p_bad_block_context);
	void Write_Bad_Block_Table(FF_Bad_Block_Context *p_bad_block_context);

	int Get_Replacement_Block(FF_Bad_Spot_Iterator bad_spot_iterator,
		U32 unit);

	static U32 Num_Bad_Block_Bitmap_Entries_Per_Array();
	static U32 Num_Bad_Block_Bitmap_Entries();
	static U32 Bad_Block_Bitmap_Size();

	Status Allocate_System_Blocks(U32 virtual_page,
		FF_PAGE_STATE page_state, U32 num_blocks, U32 *p_block_number,
		U32 unit);

	Status Test_Each_Unit(FF_Bad_Block_Context *p_bad_block_context, U32 IO_code, UI64 test_value);

	Status Create_Unit_IO_Context(
		Callback_Context *p_callback_context,
		U32 unit_index,
		U32 IO_code, UI64 test_value);

	void Do_Erase(FF_Bad_Block_Context *p_bad_block_context, Status status);
	void Do_Read(FF_Bad_Block_Context *p_bad_block_context, Status status);
	void Do_Write(FF_Bad_Block_Context *p_bad_block_context, Status status);

private: // callback methods

	static void Check_Next_Page_Blank				(void *p_context, Status status);
	static void Create_Find_Complete				(void *p_context, Status status);
	static void Read_Next_Bad_Block_Page			(void *p_context, Status status);
	static void Write_Next_Bad_Block_Page			(void *p_context, Status status);
	static void Erase_Next_Bad_Block				(void *p_context, Status status);
	static void Do_Unit_IO							(void *p_context, Status status);
	static void Run_Next_Test						(void *p_context, Status status);
	static void Surface_Test_Complete				(void *p_context, Status status);



private: // member data

	FF_Interface *m_p_flash;

	// Each FF_Bad_Block_Bit_Map_Entry is a 64-bit word.
	// Each bit represents one device block.
	// CCCDDD is used as an index into the doubleword, where
	// C = Column, D = device.
	// This table contains entries for both arrays if two arrays are present.
	FF_Bad_Block_Bit_Map_Entry	*m_p_bad_block_bitmap;

	// flash address of bad block table -- one for each array.
	Flash_Address		m_address_bad_block_array_0_copy_1;
	Flash_Address		m_address_bad_block_array_1_copy_1;
	Flash_Address		m_address_bad_block_array_0_copy_2;
	Flash_Address		m_address_bad_block_array_1_copy_2;

#ifdef MOVE_BAD_BLOCK_0

	// flash address of bad block table when allocated at block 0.
	Flash_Address		m_address_bad_block_array_0_copy_1_block0;
	Flash_Address		m_address_bad_block_array_1_copy_1_block0;
	Flash_Address		m_address_bad_block_array_0_copy_2_block0;
	Flash_Address		m_address_bad_block_array_1_copy_2_block0;
#endif // MOVE_BAD_BLOCK_0
	U32					m_if_create_bad_block_0;
	U32					m_if_move_bad_block_table;
	
	// flash address of each copy of the Table of Contents.
	Flash_Address		m_address_toc_copy_1;
	Flash_Address		m_address_toc_copy_2;

	// flash address of page map.
	Flash_Address		m_address_page_map_copy_1;
	Flash_Address		m_address_page_map_copy_2;

	// flash address of page map table.
	Flash_Address		m_address_page_map_table_copy_1;
	Flash_Address		m_address_page_map_table_copy_2;

#ifdef RESERVE_BAT_BLOCK
	// Block number assigned to the Basic Assurance Test.
	U32		m_bat_block_number;
#endif

	U32		m_initialized;

	// Alternate column specifies column where copy 2 of
	// system structures get stored.  
	U32		m_alternate_column;
	
	U32		m_bad_block_bitmap_num_pages_per_array;

	// Array of bad spot tables, one for each unit.
	FF_Bad_Spot_Table	m_bad_spot_table[FF_NUM_UNITS_MAX];

	// Block map tables are only used when we are interleaving
	// (when we have banks, as in the case of the SSD).
	// For the HBC flash, we do not use block map tables.
	U32		m_using_block_map_tables;
	
	// Count of bad blocks detected when bad block table is created.
	U32		m_num_bad_blocks;

	// Array of block map tables, one for each unit.
	FF_Block_Map_Table	m_block_map_table[FF_NUM_UNITS_MAX];

	// block allocation map for blocks allocated on unit 0.
	char	*m_p_block_allocation_map;

}; // FF_Block_Address

#ifdef RESERVE_BAT_BLOCK
/*************************************************************************/
// Bat_Block_Number returns the number of the basic assurance test block.
// One block in every unit is allocated for basic assurance testing.
/*************************************************************************/
inline U32 FF_Block_Address::Bat_Block_Number()
{
	CT_ASSERT(m_initialized, FF_Block_Address);

	return m_bat_block_number;
}
#endif

/*************************************************************************/
// Address_Bad_Block_Table_Array_0_Copy_1 
// returns the flash address of the first copy of the 
// Bad Block Table for Array 0.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Bad_Block_Table_Array_0_Copy_1(
	FF_Open_Mode open_mode)
{
#ifdef MOVE_BAD_BLOCK_0
	if (open_mode == FF_Mode_Open)

		// If we start reading at the old address, we will find it at the new.
		return m_address_bad_block_array_0_copy_1_block0;

	// We are creating.
#ifdef CREATE_BAD_BLOCK_0
	if (m_if_create_bad_block_0)
		return m_address_bad_block_array_0_copy_1_block0;
#endif
#endif
	return m_address_bad_block_array_0_copy_1;
}

/*************************************************************************/
// Address_Bad_Block_Table_Array_0_Copy_2 
// returns the flash address of the second copy of the 
// Bad Block Table for Array 0.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Bad_Block_Table_Array_0_Copy_2(
	FF_Open_Mode open_mode)
{
#ifdef MOVE_BAD_BLOCK_0
	if (open_mode == FF_Mode_Open)

		// If we start reading at the old address, we will find it at the new.
		return m_address_bad_block_array_0_copy_2_block0;

	// We are creating.
#ifdef CREATE_BAD_BLOCK_0
	if (m_if_create_bad_block_0)
		return m_address_bad_block_array_0_copy_2_block0;
#endif
#endif
	return m_address_bad_block_array_0_copy_2;
}

/*************************************************************************/
// Address_Bad_Block_Table_Array_1_Copy_1 
// returns the flash address of the first copy of the 
// Bad Block Table for Array 0.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Bad_Block_Table_Array_1_Copy_1(
	FF_Open_Mode open_mode)
{
#ifdef MOVE_BAD_BLOCK_0
	if (open_mode == FF_Mode_Open)

		// If we start reading at the old address, we will find it at the new.
		return m_address_bad_block_array_1_copy_1_block0;

	// We are creating.
#ifdef CREATE_BAD_BLOCK_0
	if (m_if_create_bad_block_0)
		return m_address_bad_block_array_1_copy_1_block0;
#endif
#endif
	return m_address_bad_block_array_1_copy_1;
}

/*************************************************************************/
// Address_Bad_Block_Table_Array_1_Copy_2 
// returns the flash address of the second copy of the 
// Bad Block Table for Array 0.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Bad_Block_Table_Array_1_Copy_2(
	FF_Open_Mode open_mode)
{
#ifdef MOVE_BAD_BLOCK_0
	if (open_mode == FF_Mode_Open)

		// If we start reading at the old address, we will find it at the new.
		return m_address_bad_block_array_1_copy_2_block0;

	// We are creating.
#ifdef CREATE_BAD_BLOCK_0
	if (m_if_create_bad_block_0)
		return m_address_bad_block_array_1_copy_2_block0;
#endif
#endif
	return m_address_bad_block_array_1_copy_2;
}

/*************************************************************************/
// Address_Toc_Copy_1 
// returns the flash address of the first copy of the Table of Contents.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Toc_Copy_1()
{
	return m_address_toc_copy_1;
}

/*************************************************************************/
// Address_Last 
// returns the last flash address assigned to a system block.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Last(Flash_Address flash_address,
													U32 num_blocks)
{
	// Set flash address for last block and last page in block.
	flash_address.Block(flash_address.Block() + num_blocks - 1);
	flash_address.Sector(Flash_Address::Sectors_Per_Block() - 1);

	return flash_address;
}

/*************************************************************************/
// Address_Toc_Copy_2 
// returns the flash address of the second copy of the Table of Contents.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Toc_Copy_2()
{
	return m_address_toc_copy_2;
}

/*************************************************************************/
// Address_Page_Map_Copy_1 
// returns the flash address of the first copy of the page map.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Page_Map_Copy_1()
{
	return m_address_page_map_copy_1;
}

/*************************************************************************/
// Address_Page_Map_Copy_2 
// returns the flash address of the second copy of the page map.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Page_Map_Copy_2()
{
	return m_address_page_map_copy_2;
}

/*************************************************************************/
// Address_Page_Map_Table_Copy_1 
// returns the flash address of the first copy of the page map table.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Page_Map_Table_Copy_1()
{
	return m_address_page_map_table_copy_1;
}

/*************************************************************************/
// Address_Page_Map_Table_Copy_2 
// returns the flash address of the first copy of the page map table.
/*************************************************************************/
inline Flash_Address FF_Block_Address::Address_Page_Map_Table_Copy_2()
{
	return m_address_page_map_table_copy_2;
}

/*************************************************************************/
// FF_Block_Address::Memory_Size
// Calculate the memory size required for the block address object.
/*************************************************************************/
U32 inline FF_Block_Address::Memory_Size(FF_CONFIG *p_config) 
{
	U32 memory_size;

	U32 num_entries = Flash_Address::Blocks_Per_Device();
	U32 num_units = Flash_Address::Num_Units();

	// Allocate memory for each bad spot table.
	memory_size = num_units * num_entries * sizeof(FF_Bad_Spot_Entry)
		+ 8; // plus alignment

	// Add size of bad block bitmap.
	memory_size += Bad_Block_Bitmap_Size();

	// Plus alignment
	memory_size += ALIGN64;

	// Allocate memory for each block map table.
	memory_size = num_units * num_entries * sizeof(FF_Block_Map_Entry)
		+ 8; // plus alignment

	// Return number of bytes of memory required.
	return memory_size;

} //FF_Block_Address::Memory_Size

/*************************************************************************/
// Num_Bad_Blocks 
/*************************************************************************/
inline U32 FF_Block_Address::Num_Bad_Blocks()
{
	return 
		m_num_bad_blocks;
}

/*************************************************************************/
// Num_Bad_Block_Bitmap_Entries_Per_Array 
/*************************************************************************/
inline U32 FF_Block_Address::Num_Bad_Block_Bitmap_Entries_Per_Array()
{
	return 
		Flash_Address::Blocks_Per_Device() * Flash_Address::Columns_Per_Array();
}

/*************************************************************************/
// Num_Bad_Block_Bitmap_Entries_Per_Array 
/*************************************************************************/
inline U32 FF_Block_Address::Num_Bad_Block_Bitmap_Entries()
{
	return Num_Bad_Block_Bitmap_Entries_Per_Array() * Flash_Address::Num_Arrays();
}

/*************************************************************************/
// Bad_Block_Bitmap_Size 
/*************************************************************************/
inline U32 FF_Block_Address::Bad_Block_Bitmap_Size()
{
	// Calculate the bad block bitmap size.
	// For a device with 2K blocks, the size of the map will be:
	// 2,048 blocks per device * 4 banks per column * 8 bytes/entry
	//	* 2 arrays = 128K bytes
	U32 bad_block_bitmap_num_bytes = Num_Bad_Block_Bitmap_Entries_Per_Array() * 
		sizeof(FF_Bad_Block_Bit_Map_Entry);

	// Round to integral number of pages.
	U32 bytes_per_page = Flash_Address::Bytes_Per_Page();
	bad_block_bitmap_num_bytes = ((bad_block_bitmap_num_bytes + bytes_per_page - 1) / bytes_per_page)
		* bytes_per_page;

	// And multiply by the number of arrays.
	bad_block_bitmap_num_bytes *= Flash_Address::Num_Arrays();

	return bad_block_bitmap_num_bytes;
}

/*************************************************************************/
// FF_Bad_Block_Context
// This is the callback context used by FF_Block_Address methods.
/*************************************************************************/
class FF_Bad_Block_Context : public Callback_Context
{
private: // member data

	// FF_Block_Address is a friend so it can access its context data
	friend class FF_Block_Address;
	
	UI64					 m_test_value;
	U32						 m_state;
	U32						 m_IO_code;
	U32						 m_test_bad_blocks;
	U32						 m_page_number;
	U32						 m_copy_number;
	U32						 m_copy_count;
	U32						 m_retry_count;
	U32						 m_array_number;
	U32						 m_num_pages;
	Flash_Address			 m_flash_address;
	void					*m_p_page_frame;
	FF_Block_Address		*m_p_block_address;
	FF_Controller			*m_p_controller;

}; // FF_Bad_Block_Context



#endif // FfBlockAddress_H

