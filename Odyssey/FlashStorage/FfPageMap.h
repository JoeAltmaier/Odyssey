/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfPageMap.h
// 
// Description:
// This file defines the interface to the Flash File virtual page map. 
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfPageMap_H)
#define FfPageMap_H

#include "Callback.h"
#include "FfCommon.h"
#include "FlashAddress.h"
#include "FfDeleteMap.h"
#include "FlashDevice.h"
#include "FfPageMapEntry.h"
#include "FfToc.h"
#include "FfBat.h"

#pragma pack(1)

class FF_Interface;
class FF_Page_Map;

#ifdef _DEBUG
#define VALIDATE_MAP FF_Page_Map::Validate_Map()

// Allocate virtual addresses at the end of the virtual address space --
// one block in each unit.
#define FF_RESERVE_FIRST_VA Flash_Address::Num_Virtual_Pages() \
- Flash_Address::Sectors_Per_Block() * Flash_Address::Num_Units()

#else
#define VALIDATE_MAP
#endif // _DEBUG

#define FIRST_TIME (U32)-1

/*************************************************************************/
// FF_Page_Map_Context
// This is the callback context used by FF_Page_Map methods.
/*************************************************************************/
class FF_Page_Map_Context : public Callback_Context
{
private: // member data

	// FF_Page_Map is a friend so it can access its context data
	friend class FF_Page_Map;
	
	U32						 m_page_index;
	Flash_Address			 m_flash_address;
	U32						 m_virtual_address;
	void					*m_p_page_frame;
	CM_PAGE_HANDLE			 m_page_handle;
	FF_Page_Map				*m_p_page_map;

	// copy_count = 1 when we write the first of two copies.
	U32						 m_copy_count;

}; // FF_Page_Map_Context

#define FF_NULL_PAGE_HANDLE -1

/*************************************************************************/
// FF_Erase_Context
// This is the callback context used to erase and remap a page block.
// There is only one static erase context.
/*************************************************************************/
class FF_Erase_Context : public Callback_Context
{
private: // member data

	// FF_Page_Map is a friend so it can access its context data
	friend class FF_Page_Map;
	
	U32						 m_page_index;
	Flash_Address			 m_first_flash_address;
	Flash_Address			 m_next_flash_address;
	void					*m_p_page_frame;

	// Is this a wear level erase context?
	// If not, then it's an erase threshold context.
	U32						 m_is_wear_level;

	// Index of deleted map entry being erased
	U32						 m_deleted_list_index;
	FF_DELETED_MAP_ENTRY	*m_p_deleted_map_entry;

	// One page handle for each page in the page block to be erased.
	// Every page is made present before the block is erased.
	CM_PAGE_HANDLE			 m_page_handle[FF_SECTORS_PER_BLOCK_MAX];

	FF_Interface			*m_p_flash;
	FF_Page_Map				*m_p_page_map;

	// Remember the page state before the erase was started.
	U8						 m_page_state[FF_SECTORS_PER_BLOCK_MAX];

	// Remember if the page was erased before the erase was started.
	U8						 m_page_erased[FF_SECTORS_PER_BLOCK_MAX];

}; // FF_Erase_Context


// Define the ranges of virtual address space
typedef enum {
	FF_VIRTUAL_ADDRESS_SPACE_USER,
	FF_VIRTUAL_ADDRESS_SPACE_ERASED_USED,
	FF_VIRTUAL_ADDRESS_SPACE_ERASED_UNUSED
} FF_VIRTUAL_ADDRESS_SPACE;

/*************************************************************************/
// FF_Page_Map
/*************************************************************************/
class FF_Page_Map
{
public:
	
	// Allocate all internal structures.
	Status Allocate(FF_Mem *p_mem, FF_Interface *p_flash);

	// Open existing FF_Page_Map.
	Status Open(Callback_Context *p_callback_context);
	
	// Close FF_Page_Map.
	Status Close(Callback_Context *p_callback_context);
	
	// Create new FF_Page_Map.
	Status Create(FF_CONFIG *p_config);
	
	// Get/Set page state
	FF_PAGE_STATE Get_Page_State(Flash_Address flash_address);
	void Set_Page_State(Flash_Address flash_address, FF_PAGE_STATE page_state);
	void Set_Page_Block_State(Flash_Address flash_address, FF_PAGE_STATE page_state);

	// Get/Set erased
	int Is_Page_Erased(Flash_Address flash_address);
	void Set_Page_Erased(Flash_Address flash_address, int erased);
	void Set_Page_Block_Erased(Flash_Address flash_address);
	
	// Get page map entry
	FF_VIRTUAL_TO_REAL_MAP_ENTRY Get_Page_Map_Entry(U32 virtual_address);
	
	// Get flash address from virtual address
	Flash_Address Get_Real_Flash_Address(U32 virtual_address);

	// Set virtual page number
	void Map_Virtual_To_Real(U32 virtual_address, Flash_Address flash_address);
	void Remap_Virtual_To_Real(U32 virtual_address, Flash_Address flash_address);
	void Map_Virtual_To_Real(U32 virtual_address, U32 index, U32 is_remap);
	void Map_Virtual_Bat_To_Bad_Block(U32 virtual_address, Flash_Address flash_address);
	void Unmap_Virtual(U32 virtual_address);
	
	// Get virtual page number
	U32 Get_Virtual_Flash_Address(Flash_Address flash_address);
	
	// Get erased page.
	// This remaps the page to a new page that is erased
	Status Get_Erased_Page(U32 virtual_address);

	// Mark a page deleted/not deleted
	void Mark_Page_Deleted(Flash_Address flash_address);
	void Mark_Page_Not_Deleted(Flash_Address flash_address);

	// Assign virtual addresses from a block that has been erased.
	// Returns true if minimum number of erased pages has been assigned.
	int Assign_Virtual(Flash_Address flash_address);
	
	Status Layout_Page_Map(int erase_all_pages);

	void Check_Erased_Page_Threshold();

	Status Check_Replacement_Pages();

	void Validate_Map();

	void Wait_For_Erased_Page(Callback_Context *p_callback_context);

	void Set_Bad_Block(Flash_Address flash_address);
	void Set_Bad_Page(Flash_Address flash_address);
	int Is_Page_Bad(Flash_Address flash_address);

	// Get pointer to TOC is used by Get_Statistics
	FF_TOC *Get_Toc(); 
	
	void Validate_Virtual_Address(U32 virtual_address, FF_VIRTUAL_ADDRESS_SPACE);

	int Is_Page_Marked_Deleted(Flash_Address flash_address);

	Status Get_Replacement_Page(Flash_Address flash_address, 
		Flash_Address *p_replacement_flash_address);

	U32 Get_Num_User_Pages();

	Status Create_Child_Test_Unit_Context(
		FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
		U32 size_buffer,
		U32 array_range_low,
		U32 array_range_high,
		U32 column_range_low,
		U32 column_range_high,
		U32 block_range_low,
		U32 block_range_high,
		Callback_Context *p_callback);

	Status Open_Cache();

	int Is_Initialized();
	
	Status Create_Virtual_To_Real_Map();

	void Find_Waiting_Contexts();

	static U32 Memory_Size(FF_CONFIG *p_config);

	// Number of blocks used by the toc
	static U32 Num_Blocks_Toc();

	// Number of reserve blocks to allocate for the toc in case of errors.
	static U32 Num_Blocks_Toc_Reserve();

	// Number of blocks used by the page map
	static U32 Num_Blocks_Page_Map();

	// Number of reserve blocks to allocate for the page map in case of errors.
	static U32 Num_Blocks_Page_Map_Reserve();

	// Number of blocks used by the page map table
	static U32 Num_Blocks_Page_Map_Table();

	// Number of reserve blocks to allocate for the page map table in case of errors.
	static U32 Num_Blocks_Page_Map_Table_Reserve();

	// Number of pages to allocate for the page map.
	static U32 Num_Pages_Page_Map();

	typedef enum {
		vp_first_page_map_copy_1,
		vp_first_page_map_copy_2,
		vp_first_page_map_table_copy_1,
		vp_first_page_map_table_copy_2,
		vp_first_toc_copy_1,
		vp_first_toc_copy_2,
		vp_first_bad_block_table_array_0_copy_1,
		vp_first_bad_block_table_array_0_copy_2,
		vp_first_bad_block_table_array_1_copy_1,
		vp_first_bad_block_table_array_1_copy_2,
		vp_first_bat
	} Real_Block;

	U32 Get_Virtual_Page_For_Real_Block(Real_Block real_block);

	int Is_System_Block(Flash_Address flash_address);

	void Map_System_Block(U32 virtual_page, FF_PAGE_STATE page_state,
		Flash_Address flash_address);

	// Initialize Table of Contents
	Status Initialize_Toc(FF_CONFIG *p_config);

	Status Initialize_Bad_Page_Addresses();

	Status Erase_Toc_And_Page_Map(Callback_Context *p_callback_context,
										   U32 copy_count);
private: // callback methods

	static void Erase_Complete			(void *p_context, Status status);
	static void Read_TOC_Complete		(void *p_context, Status status);
	static void Retry_Write_TOC			(void *p_context, Status status);
	static void Write_TOC_Complete		(void *p_context, Status status);
	static void Read_Page_Map_Table		(void *p_context, Status status);
	static void Write_Page_Map_Table	(void *p_context, Status status);
	static void Read_Page_Map			(void *p_context, Status status);
	static void Write_Page_Map			(void *p_context, Status status);
	static void Erase_Page_Map			(void *p_context, Status status);
	static void Erase_Page_Map_Table	(void *p_context, Status status);
	static void Erase_Toc				(void *p_context, Status status);
	static void Page_Map_Opened			(void *p_context, Status status);
	static void Read_Cache				(void *p_context, Status status);
	static void Read_Cache_Complete		(void *p_context, Status status);
	static void Read_Test_Block			(void *p_context, Status status);
	static void Write_Test_Block		(void *p_context, Status status);
	static void Erase_Test_Block		(void *p_context, Status status);
	static void Test_Next_Unit_Block	(void *p_context, Status status);
	static void Wait_For_Erase			(void *p_context, Status status);
	static void Read_Toc_Copy_1			(void *p_context, Status status);
	static void Read_Toc_Copy_2			(void *p_context, Status status);

private: // helper methods

	// Initialize all internal structures.
	void Initialize();
	
	void Add_Erased_Page(Flash_Address flash_address);

	void Find_Block_To_Erase();
	void Find_Next_Block_To_Erase();
	void Start_Erase_Context();

	void Exchange_Erased_Page(Flash_Address flash_address);

	void Abort_Block_Erase(FF_Erase_Context *p_erase_context);

	Status Read_Next_Toc(FF_Page_Map_Context *p_page_map_context,
						   U32 copy = 2);

	void Set_Page_Map_Table_Indexes();

	void Check_Wear_Level_Threshold();

	Status Get_Real_Erased_Page(Flash_Address *p_flash_address_erased);
	Status Get_Real_Replacement_Page(Flash_Address *p_flash_address_erased);

	U32 Is_Erase_In_Progress();
	void Set_Erase_In_Progress();
	void Reset_Erase_In_Progress();


private: // member data

	typedef enum {
		OPEN,
		FLUSHING_CACHE,
		WAITING_FOR_ERASE,
		CLOSING_CACHE,
		CACHE_CLOSED,
		WRITING_MAP,
		WRITING_MAP_TABLE,
		ERASING_TOC,
		ERASING_MAP,
		WRITING_TOC,
		CLOSED
	} Close_State;

	Close_State						 m_close_state;

	typedef enum {
		OPEN_2,
		NEVER_CLOSED,
		OPEN_1
	} Open_State;

 	// pointer to flash table of contents
	FF_TOC							*m_p_toc;

	Open_State						 m_open_state;

	// Pointer to interface object set up by Allocate.
	FF_Interface					*m_p_flash;

	// pointer to virtual page map indexed by the virtual page number.
	// This maps virtual pages to real pages.
	FF_VIRTUAL_TO_REAL_MAP_ENTRY	*m_p_virtual_to_real_map;
	
	// The page map table contains a list of all the flash addresses
	// where the page map was written.  The page map table is a buffer.
	// The toc contains a list of all the pages that make up the 
	// page map table.
	Flash_Address					*m_p_page_map_table;
	
	// pointer to real page map indexed by real page number.
	// This maps real pages to virtual pages.
	FF_REAL_TO_VIRTUAL_MAP_ENTRY	*m_p_real_to_virtual_map;
	
	// Next virtual address to assign -- used during formatting
	U32								 m_next_va;

	// Minimum number of pages to erase if we are not erasing the 
	// whole SSD.
	U32								 m_min_pages_to_erase;
	
	// True if erase is in progress to create more erased pages
	// or for wear leveling.
	U32								 m_erase_in_progress;

	// Context is always available to erase pages.
	FF_Erase_Context				 m_erase_context;

	// Context is always available to close cache.
	FF_Page_Map_Context				 m_page_map_context;
	
	// Pointer to all of the deleted maps,
	// one map for each block in each cell.
	FF_DELETED_MAP_ENTRY			*m_p_deleted_map;

	// Deleted page entries are kept on linked lists.  There are 17
	// lists, one list for entries with zero deleted pages, 
	// one list for entries with one deleted page, one list for
	// entries with two deleted pages, and so forth.
	LIST	m_p_deleted_list[NUM_DELETED_LISTS]; 

	// List of contexts waiting for an erased page
	LIST	m_list_wait_erased_page;

	// Next virtual address to assign for basic assurance testing 
	// -- used during formatting
	U32								 m_next_bat_va;

	// Pointer to buffer to use for basic assurance test.
	// This is also used to validate the toc.
	void							*m_p_bat_buffer;

	// Flag indicates that page map is open or closed.
	// When the map is closed, erased pages and replacement pages
	// cannot be allocated because the page map is being written out.
	U32								 m_map_is_open;

	// The indexes of the first and last pages of the page map table
	// are used by close to write out the page map table, and by open
	// to read it in.  Set up by Set_Page_Map_Table_Indexes.
	U32								 m_page_map_table_first_index;
	U32								 m_page_map_table_last_index;

	U32								 m_num_pages_erased;

	U32								 m_num_system_blocks_mapped;

	// Used to shift the offset in order to give a page index.
	U32								 m_num_address_bits_for_offset;

}; // FF_Page_Map


/*************************************************************************/
// FF_Page_Map::Memory_Size
/*************************************************************************/
inline U32 FF_Page_Map::Memory_Size(FF_CONFIG *p_config)
{
	U32 memory_size;

	// Allocate virtual page map.
	memory_size = 
		sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY) * Flash_Address::Num_Virtual_Pages();

	// Plus alignment
	memory_size += 8;

	// Allocate page map table.
	memory_size += 
		sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY) * Num_Pages_Page_Map();

	// Plus alignment
	memory_size += ALIGN64;

	// Allocate real page map.
	memory_size += 
		sizeof(FF_REAL_TO_VIRTUAL_MAP_ENTRY) * Flash_Address::Num_Virtual_Pages();

	// Plus alignment
	memory_size += ALIGN64;

	// Allocate TOC.
	memory_size += Flash_Address::Bytes_Per_Page();

	// Plus alignment
	memory_size += ALIGN64;

	// Allocate buffer for basic assurance test.
#ifdef RESERVE_BAT_BLOCK
	memory_size += Flash_Address::Bytes_Per_Page();

	// Plus alignment
	memory_size += ALIGN64;

#endif // RESERVE_BAT_BLOCK

	// Allocate deleted map entries.
	memory_size += sizeof(FF_DELETED_MAP_ENTRY) * Flash_Address::Num_Blocks();

	// Plus alignment
	memory_size += 8;

	return memory_size;

} // Memory_Size

/*************************************************************************/
// FF_Page_Map::Is_Erase_In_Progress
/*************************************************************************/
inline U32 FF_Page_Map::Is_Erase_In_Progress()
{
	return m_erase_in_progress;
}

/*************************************************************************/
// FF_Page_Map::Set_Erase_In_Progress
/*************************************************************************/
inline void FF_Page_Map::Set_Erase_In_Progress()
{
	CT_ASSERT((m_erase_in_progress == 0), Set_Erase_In_Progress);
	m_erase_in_progress = 1;
}

/*************************************************************************/
// FF_Page_Map::Num_Pages_Page_Map
/*************************************************************************/
inline U32 FF_Page_Map::Num_Pages_Page_Map()
{
	// Calculate how many bytes to store the page map (4 meg today)
	U32 bytes_per_map = Flash_Address::Num_Virtual_Pages() * 
		FF_BYTES_PER_PAGE_MAP_ENTRY;
	CT_ASSERT((FF_BYTES_PER_PAGE_MAP_ENTRY == 4), Num_Pages_Page_Map);

	// Calculate how many pages to store the page map (256 today)
	return
		(bytes_per_map + Flash_Address::Bytes_Per_Page() - 1)
		/ Flash_Address::Bytes_Per_Page();
}

/*************************************************************************/
// Num_Blocks_Page_Map
// Number of blocks to allocate for the page map. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Page_Map()
{
	return (Num_Pages_Page_Map() 
		+ Flash_Address::Sectors_Per_Block() - 1) / 
		Flash_Address::Sectors_Per_Block();
}

/*************************************************************************/
// Num_Blocks_Page_Map_Reserve
// Number of blocks to allocate for the page map in case of error. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Page_Map_Reserve()
{
	return (Num_Blocks_Page_Map() / 2) + 2;
}

/*************************************************************************/
// Num_Blocks_Page_Map_Table
// Number of blocks to allocate for the page map table. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Page_Map_Table()
{
	return 1;
}

/*************************************************************************/
// Num_Blocks_Page_Map_Table_Reserve
// Number of reserve blocks to allocate for the page map table in case of error. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Page_Map_Table_Reserve()
{
	return 1;
}

/*************************************************************************/
// Num_Blocks_Toc
// Number of blocks to allocate for the toc. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Toc()
{
	return 1;
}

/*************************************************************************/
// Num_Blocks_Toc_Reserve
// Number of reserve blocks to allocate for the toc in case of error. 
/*************************************************************************/
inline U32 FF_Page_Map::Num_Blocks_Toc_Reserve()
{
	return 2;
}

/*************************************************************************/
// Get_Virtual_Page_For_Real_Block
// Return the virtual page number that corresponds to a real block. 
/*************************************************************************/
inline U32 FF_Page_Map::Get_Virtual_Page_For_Real_Block(Real_Block real_block)
{
	switch(real_block)
	{
	case vp_first_page_map_table_copy_1:
		return m_p_toc->vp_first_page_map_table_copy_1;
	case vp_first_page_map_table_copy_2:
		return m_p_toc->vp_first_page_map_table_copy_2;
	case vp_first_page_map_copy_1:
		return m_p_toc->vp_first_page_map_copy_1;
	case vp_first_page_map_copy_2:
		return m_p_toc->vp_first_page_map_copy_2;
	case vp_first_toc_copy_1:
		return m_p_toc->vp_first_toc_copy_1;
	case vp_first_toc_copy_2:
		return m_p_toc->vp_first_toc_copy_2;
	case vp_first_bad_block_table_array_0_copy_1:
		return m_p_toc->vp_first_bad_block_table_array_0_copy_1;
	case vp_first_bad_block_table_array_0_copy_2:
		return m_p_toc->vp_first_bad_block_table_array_0_copy_2;
	case vp_first_bad_block_table_array_1_copy_1:
		return m_p_toc->vp_first_bad_block_table_array_1_copy_1;
	case vp_first_bad_block_table_array_1_copy_2:
		return m_p_toc->vp_first_bad_block_table_array_1_copy_2;
	case vp_first_bat:
		return m_p_toc->vp_first_bat;
	default:
		return 0;
	}
	return 0;
}

#endif // FfPageMap_H
