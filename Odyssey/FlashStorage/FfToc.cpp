/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfToc.cpp
// 
// Description:
// Implements Flash File Table Of Contents
//
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"

extern U32					  FF_bit_mask[32];

// To format quickly, for debugging...
#ifndef _WINDOWS
//#define MINIMUM_ERASED_PAGES  500
//#define MINIMUM_REPLACEMENT_PAGES  500
#endif

/*************************************************************************/
// FF_Page_Map::Initialize_Toc
// Create the flash table of contents.
// Called by Create when bad block table is created.
// Called by Open in order to initialize virtual addresses (Toc is then
// read and overwritten).
// Called by FF_Format before unit is formatted.
/*************************************************************************/
Status	FF_Page_Map::Initialize_Toc(FF_CONFIG *p_config)
{
 	// Make sure our numbers are not zero
	U32 percentage_erased_pages = p_config->percentage_erased_pages;
 	if (percentage_erased_pages == 0)
 		percentage_erased_pages = DEFAULT_PERCENTAGE_ERASED_PAGES;
 	
	U32 percentage_replacement_pages = p_config->percentage_replacement_pages;
 	if (percentage_replacement_pages == 0)
 		percentage_replacement_pages = DEFAULT_PERCENTAGE_REPLACEMENT_PAGES;
 	
	U32 replacement_page_threshold = p_config->replacement_page_threshold;
 	if (replacement_page_threshold == 0)
 		replacement_page_threshold = DEFAULT_REPLACEMENT_PAGE_THRESHOLD;
 	
	// Calculate wear level threshold.
	m_p_toc->wear_level_threshold = p_config->wear_level_threshold;
 	if (m_p_toc->wear_level_threshold == 0)
 		m_p_toc->wear_level_threshold = DEFAULT_WEAR_LEVEL_THRESHOLD;
 		
 	// Calculate number of erased pages as a percentage of the whole
 #ifdef MINIMUM_ERASED_PAGES
 	m_p_toc->num_erased_pages_available = MINIMUM_ERASED_PAGES;
 #else
 	m_p_toc->num_erased_pages_available = 
		(Flash_Address::Num_Virtual_Pages() / 100) * percentage_erased_pages

		// Add the number of reserved erased pages.  
		// We need these in order to create more erased pages.
		+ FF_NUM_RESERVE_ERASED_PAGES;
#endif

	m_p_toc->num_erased_pages_allocated = 0;

	// Calculate erased page threshold.
	m_p_toc->erased_page_threshold = m_p_toc->num_erased_pages_available / 2;
 		
 	// Calculate number of replacement pages as a percentage of the whole.
#ifdef MINIMUM_REPLACEMENT_PAGES
 	m_p_toc->num_replacement_pages_available = MINIMUM_REPLACEMENT_PAGES;
#else
 	m_p_toc->num_replacement_pages_available = 
		(Flash_Address::Num_Virtual_Pages() / 100) * percentage_replacement_pages;
#endif
	m_p_toc->num_replacement_pages_allocated = 0;
 	
	// Calculate replacement page threshold.
	m_p_toc->replacement_page_threshold = (m_p_toc->num_replacement_pages_available 
		* replacement_page_threshold) / 100;

	// Replacement page threshold must be at least a block.
	if (m_p_toc->replacement_page_threshold < Flash_Address::Sectors_Per_Block())
		m_p_toc->replacement_page_threshold = Flash_Address::Sectors_Per_Block();

	// Make sure the replacement page threshold is less than the number of replacement pages.
	while (m_p_toc->replacement_page_threshold > m_p_toc->num_replacement_pages_available)
		m_p_toc->num_replacement_pages_available += Flash_Address::Sectors_Per_Block();
 		
	// Replacement page threshold must be less than the number of replacement pages
	// by at least the number of pages in a block
	// in case all replacement pages are being erased when replacement pages
	// are needed.
	while ((m_p_toc->num_replacement_pages_available - m_p_toc->replacement_page_threshold)
		< Flash_Address::Sectors_Per_Block() )
		m_p_toc->num_replacement_pages_available += 1;

  	m_p_toc->cookie = FF_TOC_COOKIE;
  	m_p_toc->version = FF_TOC_VERSION;
	m_p_toc->vp_last = Flash_Address::Num_Virtual_Pages() - 1;
  	m_p_toc->serial_number = 1;

	// Calculate how many pages to store the page map (256 today)
	m_p_toc->num_pages_page_map = Num_Pages_Page_Map();

	// Calculate the number of blocks necessary to store the page map (8 today).
	m_p_toc->num_blocks_page_map = Num_Blocks_Page_Map();

	// Calculate how many bytes to store the page map table,
	// i.e., the page map entries for the page map itself (1,024 today today)
	U32 bytes_per_page_map_table =  m_p_toc->num_pages_page_map 
		* FF_BYTES_PER_PAGE_MAP_ENTRY;

	// Calculate the number of pages needed to store the page map table.
	m_p_toc->num_pages_page_map_table =  
		(bytes_per_page_map_table + Flash_Address::Bytes_Per_Page() - 1) 
		/ Flash_Address::Bytes_Per_Page();

	// Calculate the size of the Table of Contents.
	// We need to store the list of page map entries that make up the
	// page map table.  We store two copies of the page map.
	U32 bytes_per_toc = sizeof(FF_TOC) + (m_p_toc->num_pages_page_map_table - 1) 
		* FF_BYTES_PER_PAGE_MAP_ENTRY * 2;
	
	// Make sure the Table of Contents will fit in one page.
	if (bytes_per_toc > Flash_Address::Bytes_Per_Page())
	{
		Status status = FF_ERROR(TOC_TOO_BIG);
		CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
			"FF_Page_Map::Initialize_Toc", 
			"Page size is too small",
			status,
			0);
		return status;
	}
	
 	U32 vp = Flash_Address::Num_Virtual_Pages();

	// NOTE: All addresses that depend on real addresses must be
	// mapped here so that they can be assigned by Assign_Virtual.

	// Are we reserving one block in each unit block for basic assurance testing?
	// Assign virtual address for first copy of the TOC.
	vp -= 1;
	m_p_toc->vp_last_bat = vp;
	m_p_toc->vp_first_bat = vp;

#ifdef RESERVE_BAT_BLOCK
	vp = FF_RESERVE_FIRST_VA;
	m_p_toc->vp_first_bat = vp;
	m_next_bat_va = vp;
#endif
 	
	// Assign virtual address for bad block table for array 1.
	if (Flash_Address::Num_Arrays() == 2)
	{
		vp -= 1;
		m_p_toc->vp_last_bad_block_table_array_1_copy_2 = vp;
		vp = vp - Flash_Address::Sectors_Per_Block() + 1;
		m_p_toc->vp_first_bad_block_table_array_1_copy_2 = vp;

		vp -= 1;
		m_p_toc->vp_last_bad_block_table_array_1_copy_1 = vp;
		vp = vp - Flash_Address::Sectors_Per_Block() + 1;
		m_p_toc->vp_first_bad_block_table_array_1_copy_1 = vp;
	}

	// Assign virtual address for bad block table for array 0.
	vp -= 1;
	m_p_toc->vp_last_bad_block_table_array_0_copy_2 = vp;
	vp = vp - Flash_Address::Sectors_Per_Block() + 1;
	m_p_toc->vp_first_bad_block_table_array_0_copy_2 = vp;

	vp -= 1;
	m_p_toc->vp_last_bad_block_table_array_0_copy_1 = vp;
	vp = vp - Flash_Address::Sectors_Per_Block() + 1;
	m_p_toc->vp_first_bad_block_table_array_0_copy_1 = vp;

	// Assign virtual address for second copy of the TOC.
	U32 num_blocks_toc_total = Num_Blocks_Toc() + Num_Blocks_Toc_Reserve();
	vp -= 1;
	m_p_toc->vp_last_toc_copy_2 = vp;
	vp = vp - (num_blocks_toc_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_toc_copy_2 = vp;

	// Assign virtual address for first copy of the TOC.
	vp -= 1;
	m_p_toc->vp_last_toc_copy_1 = vp;
	vp = vp - (num_blocks_toc_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_toc_copy_1 = vp;

	// Assign virtual address for second copy of the page map table.
	U32 num_blocks_page_map_table_total = Num_Blocks_Page_Map_Table() + 
		Num_Blocks_Page_Map_Table_Reserve();
	vp -= 1;
	m_p_toc->vp_last_page_map_table_copy_2 = vp;
	vp = vp - (num_blocks_page_map_table_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_page_map_table_copy_2 = vp;

	// Assign virtual address for first copy of the page map table.
	vp -= 1;
	m_p_toc->vp_last_page_map_table_copy_1 = vp;
	vp = vp - (num_blocks_page_map_table_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_page_map_table_copy_1 = vp;
	
	// Assign virtual address for second copy of the page map.
	U32 num_blocks_page_map_total = Num_Blocks_Page_Map() + Num_Blocks_Page_Map_Reserve();
	vp -= 1;
	m_p_toc->vp_last_page_map_copy_2 = vp;
	vp = vp - (num_blocks_page_map_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_page_map_copy_2 = vp;

	// Assign virtual address for first copy of the page map.
	vp -= 1;
	m_p_toc->vp_last_page_map_copy_1 = vp;
	vp = vp - (num_blocks_page_map_total * Flash_Address::Sectors_Per_Block()) + 1;
	m_p_toc->vp_first_page_map_copy_1 = vp;
	
	// Assign virtual address for last bad block.
	vp -= 1;
	m_p_toc->vp_last_bad_block = vp;

	// Assign virtual address for first bad block.
	// The first time we increment vp_first_bad_block, we will have one.
	// vp_first_bad_block gets decremented for every bad block that is encountered
	// when we format the unit.
	m_p_toc->vp_first_bad_block = vp + 1;

 	return OK;
	
} // FF_Page_Map::Initialize_Toc
	
/*************************************************************************/
// FF_Page_Map::Initialize_Bad_Page_Addresses
// This is called after the bad block table has been opened to assign
// virtual addresses for bad blocks.
/*************************************************************************/
Status	FF_Page_Map::Initialize_Bad_Page_Addresses()
{
	U32 vp = m_p_toc->vp_first_bad_block;

	// Assign virtual address for last initial bad page.
	vp -= 1;
	m_p_toc->vp_last_bad_page = vp;

	// The first time we increment vp_first_bad_page, we will have one.
	// vp_first_bad_page gets decremented for every bad page that is encountered
	// when we format the unit.
	m_p_toc->vp_first_bad_page = vp + 1;

	// First user page is 0.
	m_next_va = 0;

	if (m_p_flash->m_flash_config.erase_all_pages)
		m_min_pages_to_erase = Flash_Address::Num_Virtual_Pages();
	else

		// Calculate the minimum number of good pages that we need
		// if we are not erasing the whole SSD.
		// Note that we do not include toc pages; these get mapped separately.
		m_min_pages_to_erase = m_p_toc->num_erased_pages_available 
			+ m_p_toc->num_replacement_pages_available;

	// Make it an integral number of blocks.
	m_min_pages_to_erase = ((m_min_pages_to_erase + Flash_Address::Sectors_Per_Block() - 1)
		/ Flash_Address::Sectors_Per_Block()) * Flash_Address::Sectors_Per_Block();
	return OK;

} // Initialize_Bad_Page_Addresses

/*************************************************************************/
// FF_Page_Map::Layout_Page_Map
// This method is called after the unit has been formatted.
// We finish the layout after the format because now we know how many
// bad pages we have.
/*************************************************************************/
Status FF_Page_Map::Layout_Page_Map(int erase_all_pages)
{
	// Virtual address of first initial bad page.
	U32 vp = m_p_toc->vp_first_bad_page;

 	// Last page of replacement page map.
 	vp  -= 1;
  	m_p_toc->vp_last_replacement_page = vp;
	
 	// First page of replacement page map
 	vp = vp - m_p_toc->num_replacement_pages_available + 1;
  	m_p_toc->vp_first_replacement_page = vp;
  	m_p_toc->vp_next_replacement_page = vp;
	
	// Last page of erased page map
 	vp -= 1;
  	m_p_toc->vp_last_erased_page = vp;
	
 	// First page of erased page map
 	vp = vp - m_p_toc->num_erased_pages_available + 1;
  	m_p_toc->vp_first_erased_page = vp;
  	m_p_toc->vp_erased_page_out = vp;
  	m_p_toc->vp_erased_page_in = vp;
 	
	// Last user page
 	vp -= 1;
  	m_p_toc->vp_last_user_page = vp;

	// Did we erase the minimum number of pages?
	if (erase_all_pages == 0)
	{
		// We erased the minimum number of pages and assigned them to virtual
		// addresses 0..m_min_erased_pages.  We must exchange the real addresses
		// that correspond with these virtual addresses with real addresses that
		// correspond to:
			// Copy 0 of the page map.
			// Copy 1 of the page map.
			// Replacement page map
			// Erased page map
		U32 vp_from = 0;
		U32 vp_to;
		Flash_Address flash_address_to;
		Flash_Address flash_address_from;
		U32 page_state;

		// Assign erased pages and replacement pages.
		for (vp_to = m_p_toc->vp_first_erased_page; 
			vp_to <= m_p_toc->vp_last_replacement_page; vp_to++)
		{
			flash_address_from = Get_Real_Flash_Address(vp_from);
			page_state = Get_Page_State(flash_address_from);
			CT_ASSERT(!Is_Page_Bad(flash_address_from), Layout_Page_Map);
			CT_ASSERT((Is_Page_Erased(flash_address_from)), Layout_Page_Map);
			CT_ASSERT((page_state == FF_PAGE_STATE_MAPPED), Layout_Page_Map);

			flash_address_to = Get_Real_Flash_Address(vp_to);
			Remap_Virtual_To_Real(vp_to, flash_address_from);
			Remap_Virtual_To_Real(vp_from, flash_address_to);

			// Next virtual page.
			vp_from++;
		}

	} // (erase_all_pages == 0)

	// Initialize the available portion of the erased page map.
	Flash_Address flash_address;
	for (vp = m_p_toc->vp_first_erased_page; vp <= m_p_toc->vp_last_erased_page; vp++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Erased(flash_address)), Validate_Map);
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_MAPPED), Layout_Page_Map);
		Set_Page_State(flash_address, FF_PAGE_STATE_ERASED);
	}

	// Initialize the available portion of the replacement page map.
	for (vp = m_p_toc->vp_first_replacement_page; vp <= m_p_toc->vp_last_replacement_page; vp++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Erased(flash_address)), Layout_Page_Map);
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_MAPPED), Layout_Page_Map);
		Set_Page_State(flash_address, FF_PAGE_STATE_REPLACEMENT);
	}

	// Initialize the wear level address.
	m_p_toc->flash_address_wear_level.Initialize();

	// The page map is now open.
	m_map_is_open = 1;
	
	VALIDATE_MAP;

 	return OK;
	
} // FF_Page_Map::Layout_Page_Map
	
