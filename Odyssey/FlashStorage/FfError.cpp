/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfError.cpp
// 
// Description:
// This file implements error handling for the Flash File. 
// 
// 12/15/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FlashDevice.h"
#include "FfInterface.h"
#include "FfPageMap.h"
#include "FfStats.h"

/*************************************************************************/
// FF_Page_Map::Get_Replacement_Page
// Get a replacement page from the replacement page list.
// The real address of the page in error will exchanged with the real
// address specified by vp_next_replacement_page.
// vp_next_replacement_page is the virtual address of the next page in the
// replacement page pool. vp_next_replacement_page currently points to a 
// real page that is mapped and replacement.
// vp_next_replacement_page will be incremented to point to the next page
// in the replacement page pool.
// Returns real address of replacement page, or 0 if none is available.
/*************************************************************************/
Status FF_Page_Map::Get_Replacement_Page(Flash_Address flash_address_bad,
										 Flash_Address *p_replacement_flash_address)
{
	Flash_Address flash_address;

	// The page we are adding to the pool has the error bit set.
	CT_ASSERT((Is_Page_Bad(flash_address_bad)), Get_Replacement_Page);
	CT_ASSERT((Get_Page_State(flash_address_bad) != FF_PAGE_STATE_BAD_PAGE), Get_Replacement_Page);
	CT_ASSERT((Get_Page_State(flash_address_bad) != FF_PAGE_STATE_BAD_BLOCK), Get_Replacement_Page);

 	// Verify that we have a page in the replacement page pool.
 	if(m_p_toc->num_replacement_pages_available == 0)
	{
		// No replacement page is available.
		m_p_flash->m_stats.Inc_Num_Error_No_Replacement_Page();
		return FF_ERROR(NO_REPLACEMENT_PAGES);
	}

	// Get the virtual address of the bad page we are adding to the pool.
	U32 virtual_address_bad = Get_Virtual_Flash_Address(flash_address_bad);

	// Get the real address currently assigned to the next replacement page.
	Flash_Address flash_address_replacement;
	Status status = Get_Real_Replacement_Page(&flash_address_replacement);
	if (status != OK)
		return status;

	// Is the page we are replacing the same as the one that just became bad?
	if (flash_address_replacement != flash_address_bad)
	{

		CT_ASSERT((Get_Page_State(flash_address_replacement) == FF_PAGE_STATE_REPLACEMENT), Get_Replacement_Page);
		CT_ASSERT((Is_Page_Erased(flash_address_replacement)), Get_Replacement_Page);

		// Map the real page that just went bad to the replaced virtual page.
		Remap_Virtual_To_Real(m_p_toc->vp_next_replacement_page, flash_address_bad);
 			
		// Map the replacement real address to the virtual address that needs a replacement.
		Remap_Virtual_To_Real(virtual_address_bad, flash_address_replacement);
#if 1
		// Is this a deleted page?
		if (Get_Page_State(flash_address_bad) == FF_PAGE_STATE_DELETED)
		{
			// The bad page is already marked not deleted.
			CT_ASSERT((!Is_Page_Marked_Deleted(flash_address_bad)), 
				Get_Replacement_Page);

			// Mark the replacement page as deleted.
			Mark_Page_Deleted(flash_address_replacement);
		}
#endif
		// Set the state of the replacement page to be the same as the state of the page 
		// that we just replaced.
		Set_Page_State(flash_address_replacement, Get_Page_State(flash_address_bad));
	}
 		
	// The new state of the page is bad.
	Set_Page_State(flash_address_bad, FF_PAGE_STATE_BAD_PAGE);

	// Increment the pointer to the next replacement page.
	m_p_toc->vp_next_replacement_page++;

	// Decrement the number of replacement pages available.
	m_p_toc->num_replacement_pages_available--;

	// Increment the number of replacement pages allocated.
	m_p_toc->num_replacement_pages_allocated++;

	//VALIDATE_MAP;

	*p_replacement_flash_address = flash_address_replacement;
	return OK;
	
} // Get_Replacement_Page

/*************************************************************************/
// Get_Real_Replacement_Page
// Get the real address of the next page in the replacement page pool.
// Return FF_ERROR(NO_REPLACEMENT_PAGES) if none are available.
/*************************************************************************/
Status FF_Page_Map::Get_Real_Replacement_Page(Flash_Address *p_flash_address_replacement)
{
	// The page map must be open
	CT_ASSERT(m_map_is_open, Get_Real_Replacement_Page);

	// Get the real address assigned to the next replacement page.
	Flash_Address flash_address_replacement = Get_Real_Flash_Address(m_p_toc->vp_next_replacement_page);
	
	// Check to see if this page is currently being erased.
	if (Get_Page_State(flash_address_replacement) != FF_PAGE_STATE_REPLACEMENT)
	{
		CT_ASSERT((Get_Page_State(flash_address_replacement) == FF_PAGE_STATE_ERASING), Get_Real_Replacement_Page);

		// This page is currently being erased, so we cannot assign it.
		Flash_Address flash_address_erasing = flash_address_replacement;

		// How many times we can check another replacement page.
		U32 num_replacement_pages_available = m_p_toc->num_replacement_pages_available;

		// We know there is at least one, or we would not have been called.
		CT_ASSERT(num_replacement_pages_available, Get_Real_Replacement_Page);

		// Get the vp of the next replacement page to be assigned.
		U32 vp_next_replacement_page = m_p_toc->vp_next_replacement_page;

		// Find another available page in the replacement page pool.
		while (1)
		{
			// Are there other replacement pages in the pool?
			if (--num_replacement_pages_available == 0)
				return FF_ERROR(REPLACEMENT_PAGES_ERASING);

			// Increment the pointer to the next replacement page.
 			vp_next_replacement_page++;

			// Get the real address of the next replacement page.
			flash_address_replacement = Get_Real_Flash_Address(vp_next_replacement_page);
			CT_ASSERT((Is_Page_Erased(flash_address_replacement)), Get_Real_Replacement_Page);
			CT_ASSERT((!Is_Page_Bad(flash_address_replacement)), Get_Real_Replacement_Page);

			// Is this one replacement?
			if (Get_Page_State(flash_address_replacement) == FF_PAGE_STATE_REPLACEMENT)
			{
				// This one is replacement.  
				// The vp of the next replacement page to be assigned gets mapped
				// to the real address of a replacement page.
				Remap_Virtual_To_Real(m_p_toc->vp_next_replacement_page, flash_address_replacement);

				// The vp of the next replacement page gets mapped to the real address
				// of the page being replacement.
				Remap_Virtual_To_Real(vp_next_replacement_page, flash_address_erasing);
				break;
			}

			// If the next page is not replacement, it must be erasing.
			CT_ASSERT((Get_Page_State(flash_address_replacement) == FF_PAGE_STATE_ERASING), Get_Real_Replacement_Page);
		} // while
		
	} // this page is currently being erased.

	// Validate the state of the replacement page.
	CT_ASSERT((Get_Page_State(flash_address_replacement) == FF_PAGE_STATE_REPLACEMENT), Get_Real_Replacement_Page);
	CT_ASSERT((Is_Page_Erased(flash_address_replacement)), Get_Real_Replacement_Page);
	CT_ASSERT((!Is_Page_Bad(flash_address_replacement)), Get_Real_Replacement_Page);

	// Return real address to caller.
	*p_flash_address_replacement = flash_address_replacement;
	return OK;

} // Get_Real_Replacement_Page

