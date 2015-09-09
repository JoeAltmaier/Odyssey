/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfValidate.cpp
// 
// Description:
// Implements validation procedures for debugging the Flash File 
//
// 12/14/98 Jim Frandeen: Create file
/*************************************************************************/
#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FlashDevice.h"
#include "FfInterface.h"
#include "FfPageMap.h"

/*************************************************************************/
// Validate_Map
/*************************************************************************/
void FF_Page_Map::Validate_Map()
{
// Don't validate unless we are simulating
// Otherwise, the map is too big!

	// Check to see if verify_structures is turned on.
	if (m_p_flash->m_flash_config.verify_structures == 0)
		return;

	// Check to see if the map has been initialized.
	if (m_p_toc->vp_last_user_page == 0)
		return;
		
	// Validate the page map entries.
	U32 index;
	U32 virtual_address;
	FF_REAL_TO_VIRTUAL_MAP_ENTRY *p_real_to_virtual_map;

	for (index = 0;
	index < Flash_Address::Num_Virtual_Pages(); index++)
	{
		Flash_Address flash_address = Get_Real_Flash_Address(index);

#if 0
		if (flash_address == bug_address)
		{
			if (Is_Page_Erased(flash_address))
				Break();
		}
#endif

		p_real_to_virtual_map = m_p_real_to_virtual_map + index;
		switch (p_real_to_virtual_map->page_state)
		{
			case FF_PAGE_STATE_BAD_PAGE:

				// If the state is FF_PAGE_STATE_BAD_PAGE, the page should be
				// in the bad page map.
				// There are two bad page maps -- one for initial bad pages,
				// and one for bad pages encountered later.
				virtual_address = Get_Virtual_Flash_Address(index);
				if (virtual_address >= m_p_toc->vp_first_bad_page)
				{
					CT_ASSERT((virtual_address <= m_p_toc->vp_last_bad_page), Validate_Map);
				}
				else
				{
					CT_ASSERT((virtual_address >= m_p_toc->vp_first_replacement_page), Validate_Map);
					CT_ASSERT((virtual_address < m_p_toc->vp_next_replacement_page), Validate_Map);
				}
				break;

			case FF_PAGE_STATE_ERASED:
				CT_ASSERT(((m_p_real_to_virtual_map + index)->erased), Validate_Map);
				break;

			case FF_PAGE_STATE_UNMAPPED:
			case FF_PAGE_STATE_MAPPED:
			case FF_PAGE_STATE_ERASING:
			case FF_PAGE_STATE_REPLACEMENT:
			case FF_PAGE_STATE_DELETED:
				Get_Virtual_Flash_Address(index);
				break;

			case FF_PAGE_STATE_PAGE_MAP:
			case FF_PAGE_STATE_TOC:
			case FF_PAGE_STATE_PAGE_MAP_TABLE:
			case FF_PAGE_STATE_BAT:
			case FF_PAGE_STATE_BAD_BAT:
			case FF_PAGE_STATE_BAD_BLOCK_TABLE:
			default:
				CT_ASSERT(1, Validate_Map);
		}
	}

	// Validate each page in the user portion of the page map.
	U32 vp = 0;
	Flash_Address flash_address;
	for (index = 0; index <= m_p_toc->vp_last_user_page; index++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_MAPPED), Validate_Map);
		CT_ASSERT((!Is_Page_Marked_Deleted(flash_address)), Validate_Map);
		CT_ASSERT((!Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		vp++;
	}

	// Validate each page in the bad page page map.
	for (vp = m_p_toc->vp_first_bad_page; 
		vp <= m_p_toc->vp_last_bad_page; 
		vp++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Bad(flash_address)), Validate_Map);
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_PAGE), Validate_Map);
	}

	// Validate each page in the bad page block map.
	for (vp = m_p_toc->vp_first_bad_block; 
		vp <= m_p_toc->vp_last_bad_block; 
		vp++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Bad(flash_address)), Validate_Map);
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BLOCK), Validate_Map);
	}

#ifdef RESERVE_BAT_BLOCK
	// Validate each page in the basic assurance test portion of the page map.
	for (vp = m_p_toc->vp_first_bat; 
		vp <= m_p_toc->vp_last_bat; 
		vp++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT(((Get_Page_State(flash_address) == FF_PAGE_STATE_BAT)
		|| (Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BAT)), Validate_Map);
	}
#endif

	// Validate each page in the available portion of the erased page map.
	vp = m_p_toc->vp_erased_page_out;
	for (index = 0; index < m_p_toc->num_erased_pages_available; index++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Erased(flash_address)), Validate_Map);

		// Page state could be ERASING
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_ERASED)
			|| (Get_Page_State(flash_address) == FF_PAGE_STATE_ERASING), Validate_Map);
		CT_ASSERT((!Is_Page_Marked_Deleted(flash_address)), Validate_Map);
		CT_ASSERT((!Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		if (vp < m_p_toc->vp_last_erased_page)

 			// Increment the pointer to the next unused erased page.
 			vp++;
		else

			// Start from the top of the list again.
			vp = m_p_toc->vp_first_erased_page;
	}

	// Validate each page in the allocated portion of the erased page map.
	vp = m_p_toc->vp_erased_page_in;
	for (index = 0; index < m_p_toc->num_erased_pages_allocated; index++)
	{
		flash_address = Get_Real_Flash_Address(vp);

		// Note that this page could be erased.  This happens when we erase a page
		// block that just happens to include a page from the used portion of the
		// erased page pool.
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_DELETED), Validate_Map);
		CT_ASSERT((Is_Page_Marked_Deleted(flash_address)), Validate_Map);
		CT_ASSERT((!Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		if (vp < m_p_toc->vp_last_erased_page)

 			// Increment the pointer to the next unused erased page.
 			vp++;
		else

			// Start from the top of the list again.
			vp = m_p_toc->vp_first_erased_page;
	}

	// Validate each page in the available portion of the replacement page map.
	vp = m_p_toc->vp_next_replacement_page;
	for (index = 0; index < m_p_toc->num_replacement_pages_available; index++)
	{
		flash_address = Get_Real_Flash_Address(vp);
		CT_ASSERT((Is_Page_Erased(flash_address)), Validate_Map);

		// Page state could be ERASING
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_REPLACEMENT)
			|| (Get_Page_State(flash_address) == FF_PAGE_STATE_ERASING), Validate_Map);
		CT_ASSERT((!Is_Page_Marked_Deleted(flash_address)), Validate_Map);
		CT_ASSERT((!Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		vp++;
	}

	// Validate each page in the allocated portion of the replacement page map.
	vp = m_p_toc->vp_first_replacement_page;
	for (index = 0; index < m_p_toc->num_replacement_pages_allocated; index++)
	{
		flash_address = Get_Real_Flash_Address(vp);

		// Note that this page could be erased.  This happens when we erase a page
		// block that just happens to include a page from the used portion of the
		// replacement page pool.
		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_PAGE), Validate_Map);
		CT_ASSERT((!Is_Page_Marked_Deleted(flash_address)), Validate_Map);
		CT_ASSERT((Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		vp++;
	}

	// Validate each page in both copies of the page map.
	vp = m_p_toc->vp_first_page_map_copy_1;
	CT_ASSERT(m_p_toc->num_pages_page_map, Validate_Map);
	for (index = 0; index < (2 * m_p_toc->num_pages_page_map); index++)
	{
		flash_address = Get_Real_Flash_Address(vp);

		CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_PAGE_MAP), Validate_Map);

		// Page map page could be bad.
		// CT_ASSERT((!Is_Page_Bad(flash_address)), Validate_Map);

		// next virtual address
 		vp++;
	}
} // Validate_Map

/*************************************************************************/
// Validate_Virtual_Address
/*************************************************************************/
void FF_Page_Map::Validate_Virtual_Address(U32 virtual_address, FF_VIRTUAL_ADDRESS_SPACE)
{
} // Validate_Virtual_Address


