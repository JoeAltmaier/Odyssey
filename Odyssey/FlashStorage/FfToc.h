/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfToc.h
// 
// Description:
// This file defines the interface to the Flash File 
// Table of Contents. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FfToc_H)
#define FfToc_H

#include "FfPageMapEntry.h"
#include "FlashAddress.h"

#pragma pack(1)

#define FF_NUM_RESERVE_ERASED_PAGES 16

/*************************************************************************/
// Flash Table of Contents
/*************************************************************************/
#define FF_TOC_COOKIE 0X74877084	// JWFT
#define FF_TOC_VERSION 4

typedef struct  {
	// Test this value to make sure we have a valid TOC
	// on a formatted SSD.
	U32		cookie;
	
	// Version
	U32		version;
	
	// I64 variables must be here for alignment.
	// Total number of page blocks erased.
	I64		num_page_blocks_erased;
	
	// Total number of times wear level algorithm was invoked.
	I64		num_wear_level_threshold;
	
	// TOC serial number (number of times written)
	U32		serial_number;
	
	// virtual page number of the last user page.
	U32		vp_last_user_page;
	
	// Virtual address range of erased page map.
	U32		vp_first_erased_page;
	// Virtual page number of next erased page to be removed from the pool.
	U32		vp_erased_page_out;
	
	// Virtual page number of next erased page to be added to the pool.
	U32		vp_erased_page_in;
	
	U32		vp_last_erased_page;
	
	// Virtual address range of replacement page map.
	U32		vp_first_replacement_page;

	// Virtual page number of next replacement page.
	U32		vp_next_replacement_page;
	U32		vp_last_replacement_page;
	
	// Virtual address of first bad page found during format.
	// Bad pages count down from here.
	U32		vp_first_bad_page;
	U32		vp_last_bad_page;
	
	// Virtual address of first bad block from bad block table.
	U32		vp_first_bad_block;
	U32		vp_last_bad_block;
	
	// Virtual address range of the first copy of the page map.
	U32		vp_first_page_map_copy_1;
	U32		vp_last_page_map_copy_1;

	// Virtual address range of the second copy of the page map.
	U32		vp_first_page_map_copy_2;
	U32		vp_last_page_map_copy_2;

	// Virtual address range of the first copy of the page map table.
	U32		vp_first_page_map_table_copy_1;
	U32		vp_last_page_map_table_copy_1;

	// Virtual address range of the second copy of the page map table.
	U32		vp_first_page_map_table_copy_2;
	U32		vp_last_page_map_table_copy_2;

	// Virtual address range of the first copy of the toc.
	U32		vp_first_toc_copy_1;
	U32		vp_last_toc_copy_1;

	// Virtual address range of the second copy of the toc.
	U32		vp_first_toc_copy_2;
	U32		vp_last_toc_copy_2;

	// Virtual address range of the bad block table for array 0.
	U32		vp_first_bad_block_table_array_0_copy_1;
	U32		vp_last_bad_block_table_array_0_copy_1;
	U32		vp_first_bad_block_table_array_0_copy_2;
	U32		vp_last_bad_block_table_array_0_copy_2;

	// Virtual address range of the bad block table for array 1.
	U32		vp_first_bad_block_table_array_1_copy_1;
	U32		vp_last_bad_block_table_array_1_copy_1;
	U32		vp_first_bad_block_table_array_1_copy_2;
	U32		vp_last_bad_block_table_array_1_copy_2;

	// Virtual address range of the basic assurance test blocks.
	U32		vp_first_bat;
	U32		vp_last_bat;

	// Last virtual page.
	U32		vp_last;

	// Wear level counter is incremented each time a page is erased.
	// When counter exceeds threshold, wear level algorithm is started.
	U32		wear_level_counter;
	U32		wear_level_threshold;
   
	// Number of pages currently available in erased page pool.
	U32		num_erased_pages_available;
   
	// Number of pages currently allocated from the erased page pool.
	U32		num_erased_pages_allocated;
   
	// Erased page threshold.  When num_erased_pages_available falls below
	// this number, we need to erase some pages.
	U32		erased_page_threshold;
	
	// Number of pages currently available in the replacement page pool.
	U32		num_replacement_pages_available;
   
	// Number of pages currently allocated from the replacement page pool.
	U32		num_replacement_pages_allocated;
   
	// Replacement page threshold.  When the number of replacement pages falls below
	// this number, write commands are no longer permitted.
	U32		replacement_page_threshold;
	
	// Current page map -- 0 or 1
	U32		current_page_map;

	// Virtual address of the current copy of the page map.
	U32		vp_current_page_map;
	
	// Real address of the current copy of the toc.
	Flash_Address		flash_address_current_toc;
	
	// Real address of the next block to be erased for wear leveling.
	Flash_Address		flash_address_wear_level;
	
	// Number of pages to store one copy of the page map.
	U32		num_pages_page_map;

	// Number of blocks to store one copy of the page map,
	// plus room for errors.
	U32		num_blocks_page_map;

	// Number of pages to store one copy of the page map table.
	// i.e., the array of page map entries that map the map
	U32		num_pages_page_map_table;

	// page_map_table_flash_address contains a list of (2 * num_pages_page_map_table) 
	// real addresses.  These contain the real addresses of the pages that contain
	// the page map table.
	// These entries contain two copies of the page map.
	Flash_Address		page_map_table_flash_address[2];
	
}  FF_TOC;


#endif // FfToc_H

