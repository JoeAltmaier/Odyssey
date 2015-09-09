/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfPageMap.cpp
// 
// Description:
// Implements Flash File Page Map
//
// 11/11/98 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfCache.h"
#include "FfCommon.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"
#include <String.h>


/*************************************************************************/
// FF_Page_Map::Allocate
// Called by FF_Initialize to allocate all page map structures
/*************************************************************************/
Status	FF_Page_Map::Allocate(FF_Mem *p_mem, FF_Interface *p_flash)
{ 	
	// The page map is not yet open.
	m_map_is_open = 0;

	// Save pointer to flash interface object.
	m_p_flash = p_flash;

	m_num_pages_erased = 0;
	m_num_system_blocks_mapped = 0;

	// Allocate virtual page map.
	m_p_virtual_to_real_map = (FF_VIRTUAL_TO_REAL_MAP_ENTRY *)p_mem->Allocate(
		sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY) * Flash_Address::Num_Virtual_Pages());
	if (m_p_virtual_to_real_map == 0)
		return FF_ERROR(NO_MEMORY);

	// Allocate page map table.
	// The page map table is a list of all the pages in the page map.
	// This is used when we close.
	m_p_page_map_table = (Flash_Address *)p_mem->Allocate(
		sizeof(Flash_Address) * Num_Pages_Page_Map(), ALIGN64);
	if (m_p_page_map_table == 0)
		return FF_ERROR(NO_MEMORY);

	// Allocate real page map.
	m_p_real_to_virtual_map = (FF_REAL_TO_VIRTUAL_MAP_ENTRY *)p_mem->Allocate(
		sizeof(U32) * Flash_Address::Num_Virtual_Pages(), ALIGN64);
	if (m_p_real_to_virtual_map == 0)
		return FF_ERROR(NO_MEMORY);

	// Allocate TOC.
	m_p_toc = (FF_TOC *)p_mem->Allocate(Flash_Address::Bytes_Per_Page(), ALIGN64);
	if (m_p_toc == 0)
		return FF_ERROR(NO_MEMORY);

	// Allocate buffer for basic assurance test.
#ifdef RESERVE_BAT_BLOCK
	m_p_bat_buffer = p_mem->Allocate(Flash_Address::Bytes_Per_Page(), ALIGN64);
	if (m_p_bat_buffer == 0)
		return FF_ERROR(NO_MEMORY);
#endif // RESERVE_BAT_BLOCK

	// Initialize context for erasing pages.
	m_erase_context.Initialize();

	m_erase_context.Set_Status(OK);
	m_erase_context.m_p_page_map = this;
	m_erase_context.m_p_flash = p_flash;
	m_erase_in_progress = 0;
	
	// Validate the size of the bit map.
	// This depends on the number of sectors per block.
	CT_ASSERT((sizeof(m_p_deleted_map->bit_map) * 8 == FF_SECTORS_PER_BLOCK_MAX), 
		FF_Page_Map::Allocate);

 	// Allocate space for deleted maps.  One entry per block in the system.
 	m_p_deleted_map = (FF_DELETED_MAP_ENTRY *)p_mem->Allocate
		(sizeof(FF_DELETED_MAP_ENTRY) * Flash_Address::Num_Blocks());
 	if (m_p_deleted_map == 0)
 		return FF_ERROR(NO_MEMORY);

	// Initialize all page map structures to zero.
	Initialize();

	m_num_address_bits_for_offset = Flash_Address::Num_Address_Bits_For_Offset();

 	return OK;
	
} // FF_Page_Map::Allocate
	
#define FF_NOT_FOUND (U32) -1
/*************************************************************************/
// FF_Page_Map::Assign_Virtual
// Assign virtual addresses from a block of pages.
// This method is only used when the unit is formatted.
// Returns 0 or the number of pages left to be assigned.  This is used
// by the format operation to determine how many more erase block operations
// are necessary.
/*************************************************************************/
int FF_Page_Map::Assign_Virtual(Flash_Address flash_address)
{
	
	// If this block does not contain a system block,...
	if (!Is_System_Block(flash_address))
	{
		// Assign this real block to a virtual address, counting up if the
		// block was not marked by the format operation, or counting down
		// if the block is was marked bad.
		for (U32 index = 0; index < Flash_Address::Sectors_Per_Block(); index++)
		{
			if (Is_Page_Erased(flash_address))

				// Increment the number of pages we have erased so that we will know
				// how many to remap when we lay out the toc.
				m_num_pages_erased++;

			// If this is a bad page,...
			if (Is_Page_Bad(flash_address))
			{
				// Bad pages count down.
				Map_Virtual_To_Real(--m_p_toc->vp_first_bad_page, flash_address);
				Set_Page_State(flash_address, FF_PAGE_STATE_BAD_PAGE);
			}
			else
			{
				CT_ASSERT((m_next_va < m_p_toc->vp_first_bad_block), Assign_Virtual);
				Map_Virtual_To_Real(m_next_va++, flash_address);
				Set_Page_State(flash_address, FF_PAGE_STATE_MAPPED);
			}

			// Increment to next address in block.
			flash_address.Increment_Page();
		}
	}

	// Returns 0 if minimum number of erased pages has been assigned.
	if (m_next_va >= m_min_pages_to_erase)
		return 0;

	// Return number of pages left to be assigned.
	return m_min_pages_to_erase - m_next_va;


} // Assign_Virtual

/*************************************************************************/
// FF_Page_Map::Check_Replacement_Pages
/*************************************************************************/
Status FF_Page_Map::Check_Replacement_Pages()
{
	if (m_p_toc->num_replacement_pages_available <= m_p_toc->replacement_page_threshold)
	{
		m_p_flash->m_stats.Inc_Num_Replacement_Threshold();
		return FF_ERROR(REPLACEMENT_PAGE_THRESHOLD);
	}
	return OK;
} // Check_Replacement_Pages


/*************************************************************************/
// FF_Page_Map::Create
// Called by Create when bad block table is created.
// Called by FF_Format before unit is formatted.
/*************************************************************************/
Status	FF_Page_Map::Create(FF_CONFIG *p_config)
{ 	
	// Initialize all internal structures.
	Initialize();

	// Initialize Table of Contents.
	Status status = Initialize_Toc(p_config);
	if (status != OK)
		return status;

 	return OK;
	
} // FF_Page_Map::Create
	
/*************************************************************************/
// FF_Page_Map::Initialize
// Called by Open and Format to initialize all page map structures.
// Structures were allocated by Allocate method.
/*************************************************************************/
void	FF_Page_Map::Initialize()
{ 	
	// Initialize virtual to real page map
	ZERO(m_p_virtual_to_real_map, 
		sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY) * Flash_Address::Num_Virtual_Pages()); 
	
	// Initialize page map table
	ZERO(m_p_page_map_table, 
		sizeof(Flash_Address) * Num_Pages_Page_Map()); 
	
	// Initialize real to virtual page map
	ZERO(m_p_real_to_virtual_map, sizeof(U32) * Flash_Address::Num_Virtual_Pages()); 
	
	// Initialize TOC
	ZERO(m_p_toc, Flash_Address::Bytes_Per_Page()); 

	// Set deleted maps to zero.
	ZERO(m_p_deleted_map, sizeof(FF_DELETED_MAP_ENTRY) * Flash_Address::Num_Blocks());
 		
 	// Initialize each FF_DELETED_MAP_ENTRY.
	U32 index;
	for (index = 0; index < Flash_Address::Num_Blocks(); index ++)
	{
		FF_DELETED_MAP_ENTRY *p_deleted_map_entry = m_p_deleted_map + index;
  		LIST_INITIALIZE(&p_deleted_map_entry->list);
	}
 	
 	// Initialize list of deleted map entries.	
 	for (index = 0; index < NUM_DELETED_LISTS; index ++)
	{
  		LIST_INITIALIZE(&m_p_deleted_list[index]);
	}

	// Initialize lists
	LIST_INITIALIZE(&m_list_wait_erased_page);

	m_close_state = OPEN;
 	
} // FF_Page_Map::Initialize
	
/*************************************************************************/
// Wait_For_Erased_Page
// Queue the context of the list of contexts waiting for an erased page.
/*************************************************************************/
void FF_Page_Map::Wait_For_Erased_Page(Callback_Context *p_callback_context)
{
	CT_ASSERT(Is_Erase_In_Progress(), Wait_For_Erased_Page);
	LIST_INSERT_TAIL(&m_list_wait_erased_page, &p_callback_context->m_list);
	m_p_flash->m_stats.Inc_Num_Waits_Erased_Page();
}


/*************************************************************************/
// Set_Page_Map_Table_Indexes
// Set the first and last page indexes of the page map table.
// These are used by Close to write out the page map table, and by Open
// to read in the page map table.
/*************************************************************************/
void FF_Page_Map::Set_Page_Map_Table_Indexes()
{
    // Calculate the virtual address of the first entry of 
    // the page map table.
	// Point to the first entry in the page map of the page map table.
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_page_map_table_first_entry = 
		m_p_virtual_to_real_map + m_p_toc->vp_first_page_map_copy_1;
		
	// Calculate the page index of the first page that contains the page map table.
	m_page_map_table_first_index = ((char *)p_page_map_table_first_entry 
		- (char *)m_p_virtual_to_real_map)
		/ Flash_Address::Bytes_Per_Page();

	// Point to the last entry in the page map of the page map table.
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_page_map_table_last_entry = 
		m_p_virtual_to_real_map + m_p_toc->vp_first_page_map_copy_1
		+ m_p_toc->num_pages_page_map;
		
	// Calculate the page index of the last page that contains the page map table.
	m_page_map_table_last_index = ((char *)p_page_map_table_last_entry 
		- (char *)m_p_virtual_to_real_map)
		/ Flash_Address::Bytes_Per_Page();
		
} // Set_Page_Map_Table_Indexes


/*************************************************************************/
// Create_Virtual_To_Real_Map
// The real to virtual map has been read in.
// Create the virtual to real page map
// from the real to virtual map.
/*************************************************************************/
Status FF_Page_Map::Create_Virtual_To_Real_Map()
{
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map;
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map;
	U32 num_virtual_pages = Flash_Address::Num_Virtual_Pages();
	Flash_Address flash_address;

	CT_ASSERT((m_erase_in_progress == 0), FF_Page_Map::Create_Virtual_To_Real_Map);

	// Initialize virtual to real page map
	ZERO(m_p_virtual_to_real_map,
		sizeof(FF_VIRTUAL_TO_REAL_MAP_ENTRY) * Flash_Address::Num_Virtual_Pages()); 
	
	// Look at each entry in the real to virtual map.
	for (U32 real_address = 0; real_address < num_virtual_pages; real_address++)
	{
		// Create Flash_Address from real address.
		flash_address = real_address;

		// Point to the next real to virtual map entry.
		// This is in the map that we just read in.
		p_real_to_virtual_map = m_p_real_to_virtual_map + real_address;

		// Get the virtual page number that this entry maps to.
		U32 virtual_address = p_real_to_virtual_map->virtual_address;

		// validate the page map entry
		if (virtual_address > num_virtual_pages)
		{
			Tracef("\nInvalid page map, real address %d has virtual address %d > number of virtual pages %d",
				real_address, virtual_address, num_virtual_pages);
			return FF_ERROR(INVALID_PAGE_MAP_VIRTUAL_ADDRESS);
		}
		
		// Point to virtual to real entry for this virtual address.
		p_virtual_to_real_map = m_p_virtual_to_real_map + virtual_address;

		if (*p_virtual_to_real_map != 0)
		{
			// If we already set this virtual address, then more than one
			// real address maps to the same virtual address.

			// Just for debugging, see what we are already mapped to.
			U32 real_address_already_mapped = *p_virtual_to_real_map;
			FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_entry_already_mapped = 
				m_p_real_to_virtual_map + real_address_already_mapped;
			Tracef("\nVirtual address %d already mapped to real address %d",
				virtual_address, real_address_already_mapped);
			Tracef("\nReal address %d mapped to virtual address %d",
				real_address_already_mapped, p_real_to_virtual_entry_already_mapped->virtual_address);
			return FF_ERROR(INVALID_PAGE_MAP_DUPLICATE_VIRTUAL_ADDRESS);
		}
		
		// Set virtual map to point to real address.
		*p_virtual_to_real_map = real_address;
	
		switch (p_real_to_virtual_map->page_state)
		{
			case FF_PAGE_STATE_DELETED:
	
				Mark_Page_Deleted(flash_address);
				break;
	
			case FF_PAGE_STATE_PAGE_MAP:
			case FF_PAGE_STATE_TOC:
			case FF_PAGE_STATE_PAGE_MAP_TABLE:
			case FF_PAGE_STATE_BAT:
			case FF_PAGE_STATE_BAD_BAT:
			case FF_PAGE_STATE_MAPPED:
			case FF_PAGE_STATE_REPLACEMENT:
			case FF_PAGE_STATE_BAD_PAGE:
			case FF_PAGE_STATE_BAD_BLOCK:
			case FF_PAGE_STATE_BAD_BLOCK_TABLE:
				break;
	
			case FF_PAGE_STATE_ERASED:
				if (p_real_to_virtual_map->erased)
					break;
	
			default:
				Tracef("\nInvalid page map state.  Real address %d has invalid state %d.",
					real_address, p_real_to_virtual_map->page_state);
				return FF_ERROR(INVALID_PAGE_MAP_STATE);

		} // switch
		
	} // for
	
	return OK;
	
} // Create_Virtual_To_Real_Map

// TEMPORARY move /* inline */s here from FfPageMap.h because
// Metrowerks will not step into an inline .

/*************************************************************************/
// Get_Page_Map_Entry
/*************************************************************************/
/* inline */ FF_VIRTUAL_TO_REAL_MAP_ENTRY FF_Page_Map::Get_Page_Map_Entry(
    U32 virtual_address)
{
	// Validate virtual page number.
		CT_ASSERT((virtual_address < Flash_Address::Num_Virtual_Pages()),
		Set_Page_Block_Erased);

	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map = m_p_virtual_to_real_map 
		+ virtual_address;
	return 	*p_virtual_to_real_map;
}

/*************************************************************************/
// Get_Num_User_Pages
/*************************************************************************/
/* inline */ U32 FF_Page_Map::Get_Num_User_Pages()
{
	return 	m_p_toc->vp_last_user_page + 1;
}

/*************************************************************************/
// Get_Page_State
/*************************************************************************/
/* inline */ FF_PAGE_STATE FF_Page_Map::Get_Page_State(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Get_Page_State);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	return 	(FF_PAGE_STATE)p_real_to_virtual_map->page_state;
}

/*************************************************************************/
// Get_Real_Flash_Address
// Given a virtual address, return the real flash address.
/*************************************************************************/
/* inline */ Flash_Address FF_Page_Map::Get_Real_Flash_Address(U32 virtual_address)
{
	// Validate virtual page number.
	CT_ASSERT((virtual_address < Flash_Address::Num_Virtual_Pages()), Map_Virtual_To_Real);

	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map = m_p_virtual_to_real_map 
		+ virtual_address;
	U32 real_flash_address = *p_virtual_to_real_map;

#ifdef _DEBUG
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ real_flash_address;
	CT_ASSERT((p_real_to_virtual_map->virtual_address == virtual_address), Get_Real_Flash_Address);
#endif 

	// Create Flash_Address from flash address index.
	return Flash_Address(real_flash_address);

} // Get_Real_Flash_Address

/*************************************************************************/
// Get_TOC
// Get pointer to TOC is used by Get_Statistics
/*************************************************************************/
/* inline */ FF_TOC *FF_Page_Map::Get_Toc()
{
	return 	m_p_toc;
}

/*************************************************************************/
// Get_Virtual_Flash_Address
/*************************************************************************/
/* inline */ U32 FF_Page_Map::Get_Virtual_Flash_Address(
	Flash_Address flash_address)
{
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	U32 virtual_address = p_real_to_virtual_map->virtual_address;

	// Assert that virtual map points to real page if virtual map has been initialized
#ifdef _DEBUG
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map = m_p_virtual_to_real_map 
		+ virtual_address;
	//CT_ASSERT(((m_state != FF_STATE_OPEN) || 
		//*p_virtual_to_real_map == flash_address), Get_Virtual_Flash_Address);
#endif

	return virtual_address;

} // Get_Virtual_Flash_Address

/*************************************************************************/
// Is_Page_Bad
/*************************************************************************/
/* inline */ int FF_Page_Map::Is_Page_Bad(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()),
		Set_Page_Block_Erased);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = 
		m_p_real_to_virtual_map + flash_address.Index();
	return p_real_to_virtual_map->bad_page;

} // Is_Page_Bad

/*************************************************************************/
// Is_Page_Erased
/*************************************************************************/
/* inline */ int FF_Page_Map::Is_Page_Erased(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Is_Page_Erased);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	return 	p_real_to_virtual_map->erased;
}

/*************************************************************************/
// Is_System_Block
/*************************************************************************/
/* inline */ int FF_Page_Map::Is_System_Block(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Is_System_Block);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	switch 	(p_real_to_virtual_map->page_state)
	{
	case FF_PAGE_STATE_PAGE_MAP:
	case FF_PAGE_STATE_PAGE_MAP_TABLE:
	case FF_PAGE_STATE_BAT:
	case FF_PAGE_STATE_BAD_BAT:
	case FF_PAGE_STATE_TOC:
	case FF_PAGE_STATE_BAD_BLOCK_TABLE:
	case FF_PAGE_STATE_BAD_BLOCK:
		return true;
	}
	return false;
}

/*************************************************************************/
// Remap_Virtual_To_Real
// Set virtual map to point to real address.
// Set real map to point to virtual address.
/*************************************************************************/
/* inline */ void FF_Page_Map::Remap_Virtual_To_Real(U32 virtual_address,
												 Flash_Address flash_address)
{
	Map_Virtual_To_Real(virtual_address, flash_address.Index(), true /* is_remap */ );
}

/*************************************************************************/
// Map_Virtual_To_Real
// Like Remap_Virtual_To_Real, but this is used when the page map has
// not yet been initialized.
/*************************************************************************/
/* inline */ void FF_Page_Map::Map_Virtual_To_Real(U32 virtual_address,
												 Flash_Address flash_address)
{
	Map_Virtual_To_Real(virtual_address, flash_address.Index(), false /* is_remap */ );
}

/* inline */ void FF_Page_Map::Map_Virtual_To_Real(U32 virtual_address,
												 U32 index, U32 is_remap)
{
	// Validate virtual page number.
	CT_ASSERT((virtual_address < Flash_Address::Num_Virtual_Pages()), Map_Virtual_To_Real);

	// Validate real page number.
	CT_ASSERT((index < Flash_Address::Num_Virtual_Pages()), Map_Virtual_To_Real);

	// Set virtual map to point to real address.
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map = m_p_virtual_to_real_map 
		+ virtual_address;

	// If this is not remap, make sure this map entry has not already been initialized.
	CT_ASSERT(((is_remap == true) 
		|| (*p_virtual_to_real_map == 0)
		|| (*p_virtual_to_real_map == index)), Map_Virtual_To_Real);

	*p_virtual_to_real_map = index;

	// Set real map to point to virtual address.
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ index;

	// If this is not remap, make sure this map entry has not already been initialized.
	CT_ASSERT(((is_remap == true) 
		|| (p_real_to_virtual_map->virtual_address == 0)
		|| (p_real_to_virtual_map->virtual_address == virtual_address)), Map_Virtual_To_Real);

	p_real_to_virtual_map->virtual_address = virtual_address;

} // Map_Virtual_To_Real

/*************************************************************************/
// Unmap_Virtual
/*************************************************************************/
/* inline */ void FF_Page_Map::Unmap_Virtual(U32 virtual_address)
{
	// Validate virtual page number.
	CT_ASSERT((virtual_address < Flash_Address::Num_Virtual_Pages()), Map_Virtual_To_Real);

	// Point to map entry.
	FF_VIRTUAL_TO_REAL_MAP_ENTRY *p_virtual_to_real_map = m_p_virtual_to_real_map 
		+ virtual_address;

	*p_virtual_to_real_map = 0;

} // Unmap_Virtual

/*************************************************************************/
// Map_Virtual_Bat_To_Bad_Block
// This is called when the real address of a bat block has already 
// been mapped to a bad block.  We must remap the bad block.
// We remove the bad block from the range of bad block virtual addresses.
// This means we decrease the virtual address space of the bad blocks.
// This bad block will use the virtual address of its bat block.
// We also change the state of the page to FF_PAGE_STATE_BAD_BAT.
/*************************************************************************/
/* inline */ void FF_Page_Map::Map_Virtual_Bat_To_Bad_Block(U32 vp_bad_bat,
	Flash_Address flash_address_bad_bat)
{
	// Remove the first block from the bad block virtual address space.
	U32 vp_bad_block_remove = m_p_toc->vp_first_bad_block;
	m_p_toc->vp_first_bad_block += Flash_Address::Sectors_Per_Block();
	Flash_Address flash_address_bad_block_remove;

	// Remap the bad block to its corresponding bat block virtual address.
	U32 index;
	U32 vp_bad_block;
	for (index = 0; index < Flash_Address::Sectors_Per_Block(); index++)
	{
		// Save the virtual address currently mapped to this bad bat block.
		vp_bad_block = Get_Virtual_Flash_Address(flash_address_bad_bat);
		
		// Save the flash address of the next bad page we are removing.
		flash_address_bad_block_remove = Get_Real_Flash_Address(vp_bad_block_remove);
	
		// Remap the real address of the next bat block to point to
		// the bat block address space.
		Remap_Virtual_To_Real(vp_bad_bat, flash_address_bad_bat);
		
		// Change the state from bad block to bad bat
		CT_ASSERT((Get_Page_State(flash_address_bad_bat) == FF_PAGE_STATE_BAD_BLOCK), 
			Map_Virtual_Bat_To_Bad_Block);
		Set_Page_State(flash_address_bad_bat, FF_PAGE_STATE_BAD_BAT);
		
		// Remap the virtual bad block page that used to point to the
		// bat block to now point to the flash address that we are
		// removing from the address space.
		Remap_Virtual_To_Real(vp_bad_block, flash_address_bad_block_remove);
		
		// Unmap the virtual address that we are removing.
		Unmap_Virtual(vp_bad_block_remove);
		
		// Step the flash address of the next bat block
		flash_address_bad_bat.Increment_Page();
		
		// Increment the virtual address being removed.
		vp_bad_block_remove++;
		
		// Increment the virtual address of the bad block.
		vp_bad_bat++;
	}

} // Map_Virtual_Bat_To_Bad_Block

/*************************************************************************/
// Set_Bad_Block
// Mark every page in the block bad in the page table
// and set the page state.  These blocks never get used.
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Bad_Block(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Bad_Block);

	// flash_address must be on a block boundary.
	CT_ASSERT((Flash_Address::Num_Virtual_Pages()), Set_Bad_Block);

	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (U32 index = 0; index < sectors_per_block; index++)
	{
		Map_Virtual_To_Real(--m_p_toc->vp_first_bad_block, flash_address);
		Set_Bad_Page(flash_address);
		Set_Page_State(flash_address, FF_PAGE_STATE_BAD_BLOCK);

		// Increment flash address
		flash_address.Increment_Page();
	}
} // Set_Bad_Block

/*************************************************************************/
// Set_Bad_Page
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Bad_Page(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Page_Block_Erased);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	p_real_to_virtual_map->bad_page = 1;

#if 0
	if (flash_address.Index() == 0x180)
		FF_Break();
#endif

} // Set_Bad_Page

/*************************************************************************/
// Set_Page_Block_Erased
// Mark every page in the block erased in the page table.
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Page_Block_Erased(Flash_Address flash_address)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Page_Block_Erased);

	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (U32 index = 0; index < sectors_per_block; index++)
	{
 		// Mark page erased.
		Set_Page_Erased(flash_address, 1);

		// Increment flash address
		flash_address.Increment_Page();
	}
}

/*************************************************************************/
// Set_Page_Block_State
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Page_Block_State(Flash_Address flash_address, 
	FF_PAGE_STATE page_state)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Page_Block_Erased);

	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (U32 index = 0; index < sectors_per_block; index++)
	{
 		// Set page state.
		Set_Page_State(flash_address, page_state);

		// Increment flash address
		flash_address.Increment_Page();
	}
}

/*************************************************************************/
// Set_Page_Erased
// Set page erased flag to 0 or 1.
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Page_Erased(Flash_Address flash_address, int erased)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Page_Erased);

	// The erased flag could already be set if we are opening
	// the flash file and erasing the page map or Toc.
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = m_p_real_to_virtual_map 
		+ flash_address.Index();
	p_real_to_virtual_map->erased = erased;
	FF_Check_Break_Address(flash_address);
}

/*************************************************************************/
// Set_Page_State
/*************************************************************************/
/* inline */ void FF_Page_Map::Set_Page_State(Flash_Address flash_address, 
	FF_PAGE_STATE page_state)
{
	// Validate real page number.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Set_Page_State);

	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map = 
		m_p_real_to_virtual_map + flash_address.Index();

#ifdef _DEBUG

	if (m_map_is_open)
	{
		// If this is a user page
		if (p_real_to_virtual_map->virtual_address <= m_p_toc->vp_last_user_page) 
		{
			// The state must be mapped or deleted.
			CT_ASSERT(((page_state == FF_PAGE_STATE_MAPPED)
				|| (page_state == FF_PAGE_STATE_DELETED)), Set_Page_State);
		}
	}
#endif

#if 0
	// TEMPORARY
	if (p_real_to_virtual_map->virtual_address == 0x0D8)
		FF_Break();
#endif

	FF_Check_Break_Address(flash_address);
	p_real_to_virtual_map->page_state = page_state;

} // Set_Page_State

/*************************************************************************/
// FF_Page_Map::Map_System_Block
// Assign virtual addresses to a block of pages.
/*************************************************************************/
void FF_Page_Map::Map_System_Block(U32 virtual_page, FF_PAGE_STATE page_state,
	Flash_Address flash_address)
{
	// Map each page of the system block.
	for (U32 index = 0; index < Flash_Address::Sectors_Per_Block(); index++)
	{
		// If this is a bad page,...
		if (Is_Page_Bad(flash_address))
		{
			CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BLOCK),
				Map_System_Block);
		}
		else
		{
			CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_UNMAPPED),
				Map_System_Block);
			Map_Virtual_To_Real(virtual_page + index, flash_address);
			Set_Page_State(flash_address, page_state);
		}

		// Increment to next address in block.
		flash_address.Increment_Page();
	}

} // Map_System_Block(Flash_Address flash_address, 
						
