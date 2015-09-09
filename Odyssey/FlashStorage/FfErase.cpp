/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfErase.cpp
// 
// Description:
// This file implements erase operations for the Flash File. 
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FlashDevice.h"
#include "FfPageMap.h"
#include "FfRequest.h"
#include "FfStats.h"

#if 1
// Turn on TRACE_ERROR
#define TRACE_ERROR Tracef
#else
// Turn off TRACE_ERROR -- if 1 then do nothing, else call Tracef
#define TRACE_ERROR 1? 0 : Tracef  
#endif

/*************************************************************************/
// FF_Page_Map::Add_Erased_Page
// Add erased page to the erased page list.
// The real address of the page that was just erased will be assigned to
// the virtual address specified by vp_erased_page_in.
// vp_erased_page_in is the virtual address of the next input page in the
// erased page pool. vp_erased_page_in currently points to a real page that
// is deleted.
// The real address currently assigned to vp_erased_page_in 
// will be assigned to the virtual address of a deleted page.
// vp_erased_page_in will be incremented to point to the next input page
// in the erased page pool.
/*************************************************************************/
void FF_Page_Map::Add_Erased_Page(Flash_Address flash_address_erased)
{
 	TRACE_ENTRY(FF_Page_Map::Add_Erased_Page);

	// The page map must be open
	CT_ASSERT(m_map_is_open, Add_Erased_Page);

	// The page we are adding to the pool is mapped and erased.
	CT_ASSERT((Is_Page_Erased(flash_address_erased)), Add_Erased_Page);
	Set_Page_State(flash_address_erased, FF_PAGE_STATE_MAPPED);

 	// Verify that we have a space in the input erased page pool.
 	CT_ASSERT((m_p_toc->num_erased_pages_allocated), Add_Erased_Page);
 	CT_ASSERT((m_p_toc->vp_erased_page_in != m_p_toc->vp_erased_page_out), Add_Erased_Page);
 		
	// Get the virtual address of the erased page we are adding to the pool.
	U32 virtual_address_erased = Get_Virtual_Flash_Address(flash_address_erased);

	// Get the real address currently assigned to the next input erased page.
	// The next input erased page is currently deleted.
	Flash_Address flash_address_deleted = Get_Real_Flash_Address(m_p_toc->vp_erased_page_in);

	// Is the page we are replacing the same as the one that was just erased?
	if (virtual_address_erased != m_p_toc->vp_erased_page_in)
	{
		CT_ASSERT((Get_Page_State(flash_address_deleted) == FF_PAGE_STATE_DELETED), Add_Erased_Page);

		// The page could be erased if it was part of a block erase.
		//CT_ASSERT((!Is_Page_Erased(flash_address_deleted)), Add_Erased_Page);

		// Map the real page that was just erased to the input erased page.
		Remap_Virtual_To_Real(m_p_toc->vp_erased_page_in, flash_address_erased);
 			
		// Map the real address previously assigned to the next input erased page.
		Remap_Virtual_To_Real(virtual_address_erased, flash_address_deleted);
	}
 		
	// The new state of the page is erased.
	Set_Page_State(flash_address_erased, FF_PAGE_STATE_ERASED);

	// Increment number of remaining available erased pages.
	m_p_toc->num_erased_pages_available++;
	
	// Decrement the number of allocated erased pages.
	m_p_toc->num_erased_pages_allocated--;
			
 	// See if we have unused erased page map entries before the end.
 	if (m_p_toc->vp_erased_page_in < m_p_toc->vp_last_erased_page)
 	{
 		// Increment the pointer to the next used erased page.
 		m_p_toc->vp_erased_page_in++;
 		return;
 	}
 	
 	// We must be at the end of the erased page map.
 	CT_ASSERT ((m_p_toc->vp_erased_page_in == 
		(m_p_toc->vp_first_erased_page + m_p_toc->num_erased_pages_allocated 
		+ m_p_toc->num_erased_pages_available - 1)),
 		Add_Erased_Page);
 		
	// Start from the top of the list again.
	m_p_toc->vp_erased_page_in = m_p_toc->vp_first_erased_page;

	// N.B. We cannot validate the map here.  Add_Erased_Page is called for each
	// page in Erase_Complete.  For example, a page map page could be marked bad
	// but the page will be replaced later in the page block.  So we must not 
	// validate the map until the entire block has been handled.
	//VALIDATE_MAP;
	
} // Add_Erased_Page

/*************************************************************************/
// Check_Erased_Page_Threshold
// Check to see if we have enough erased pages in the erased page map.
// If not, start an erase operation to erase a block of pages.
/*************************************************************************/
void FF_Page_Map::Check_Erased_Page_Threshold()
{
	// The page map must be open
	CT_ASSERT(m_map_is_open, Check_Erased_Page_Threshold);

	// If we are closing, don't start an erase unless we are
	// actually out of pages.
	U32 threshold = m_p_toc->erased_page_threshold;
	if (m_close_state != OPEN)
		threshold = 0;

	// Has the erased page count fallen below the threshold?
 	if (m_p_toc->num_erased_pages_available > threshold)
 		return;

	// Is there an erase operation already in progress?
	if (Is_Erase_In_Progress())
 	{
	 	TRACEF( TRACE_L5, (EOL "Check_Erased_Page_Threshold, In progress"));
 		return;
 	}

	TRACEF( TRACE_L5, (EOL "Check_Erased_Page_Threshold, Must erase"));
	
	m_p_flash->m_stats.Inc_Num_Erased_Page_Threshold();

	Set_Erase_In_Progress();
	Find_Block_To_Erase();

} // Check_Erased_Page_Threshold

/*************************************************************************/
// Get_Erased_Page
// Map an erased page to the specified virtual address.
// Get the next erased page from the erased page map.
/*************************************************************************/
Status FF_Page_Map::Get_Erased_Page(U32 virtual_address)
{
	// The page map must be open
	CT_ASSERT(m_map_is_open, Get_Erased_Page);

	// Get the real page number.
	Flash_Address flash_address = Get_Real_Flash_Address(virtual_address);

	// See if the page is already erased.
	if (Is_Page_Erased(flash_address))
	{
		// The page is already erased.
	 	TRACEF( TRACE_L5, (EOL "Get_Erased_Page, page erased"));
 	
		CT_ASSERT((!Is_Page_Bad(flash_address)), Get_Erased_Page);
		return OK;
	}

	// The page is not erased, so we will have to exchange this 
	// real page for an erased page.

 	// See if we have any erased pages.
 	if (m_p_toc->num_erased_pages_available <= FF_NUM_RESERVE_ERASED_PAGES)
	{
	 	TRACEF( TRACE_L5, (EOL "Get_Erased_Page, FF_ERROR(NO_ERASED_PAGES)"));
		m_p_flash->m_stats.Inc_Num_No_Erased_Pages();
		Check_Erased_Page_Threshold();
 		return FF_ERROR(NO_ERASED_PAGES);
	}
 		
	// Get the current state of the page that needs to be replaced
	// with an erased page.
	FF_PAGE_STATE page_state = Get_Page_State(flash_address);

	// The virtual address that needs an erased page is mapped and not erased.
	CT_ASSERT(((page_state == FF_PAGE_STATE_MAPPED) ||
		(page_state == FF_PAGE_STATE_PAGE_MAP)), Get_Erased_Page);
	CT_ASSERT((!Is_Page_Erased(flash_address)), Get_Erased_Page);

	// Get the real address assigned to the next erased page.
	Flash_Address flash_address_erased;
	Status status = Get_Real_Erased_Page(&flash_address_erased);
	if (status != OK)
	{
		// No erased pages are available.
	 	TRACEF( TRACE_L5, (EOL "Get_Erased_Page, FF_ERROR(NO_ERASED_PAGES)"));
		m_p_flash->m_stats.Inc_Num_No_Erased_Pages();
		Check_Erased_Page_Threshold();
		return status;
	}
	
	// Mark this page deleted in the page map.
	Mark_Page_Deleted(flash_address);

	// Set the state of the replacement page to be the same as the state of the page 
	// that we just replaced.
	Set_Page_State(flash_address_erased, page_state);

	// Map our virtual page to the erased page.
	Remap_Virtual_To_Real(virtual_address, flash_address_erased);
 		
	// Map the deleted page.
	Remap_Virtual_To_Real(m_p_toc->vp_erased_page_out, flash_address);
 		
	// Decrement number of remaining availabale erased pages.
	m_p_toc->num_erased_pages_available--;
	
	// Increment the number of used erased pages.
	m_p_toc->num_erased_pages_allocated++;
	
 	// Verify that we still have erased pages.
 	CT_ASSERT((m_p_toc->num_erased_pages_available), Get_Erased_Page);
 		
 	// See if we have erased page map entries before the end.
 	if (m_p_toc->vp_erased_page_out < m_p_toc->vp_last_erased_page)

		// Increment the pointer to the next erased page.
 		m_p_toc->vp_erased_page_out++;
 	else
	{
 		// We must be at the end of the erased page map.
 		CT_ASSERT ((m_p_toc->vp_erased_page_out == 
			(m_p_toc->vp_first_erased_page + m_p_toc->num_erased_pages_allocated
			+ m_p_toc->num_erased_pages_available - 1)),
 			Get_Erased_Page);
 			
		// Start from the top of the list again.
		m_p_toc->vp_erased_page_out = m_p_toc->vp_first_erased_page;
	}
	
	Check_Erased_Page_Threshold();

 	VALIDATE_MAP;

	return status;

 } // Get_Erased_Page
 
/*************************************************************************/
// Get_Real_Erased_Page
// Get the real address of the next page in the erased page pool.
// Return FF_ERROR(NO_ERASED_PAGES) if none are available.
/*************************************************************************/
Status FF_Page_Map::Get_Real_Erased_Page(Flash_Address *p_flash_address_erased)
{
	// The page map must be open
	CT_ASSERT(m_map_is_open, Get_Erased_Page);

	// Get the real address assigned to the next erased page.
	Flash_Address flash_address_erased = Get_Real_Flash_Address(m_p_toc->vp_erased_page_out);
	
	// Check to see if this page is currently being erased.
	if (Get_Page_State(flash_address_erased) != FF_PAGE_STATE_ERASED)
	{
		CT_ASSERT((Get_Page_State(flash_address_erased) == FF_PAGE_STATE_ERASING), Get_Real_Erased_Page);

		// This page is currently being erased, so we cannot assign it.
		Flash_Address flash_address_erasing = flash_address_erased;

		// How many times we can check another erased page.
		U32 num_erased_pages_available = m_p_toc->num_erased_pages_available;

		// Get the vp of the next erased page to be assigned.
		U32 vp_next_erased_page = m_p_toc->vp_erased_page_out;

		// Find another available page in the erased page pool.
		while (1)
		{
			// Are there other erased pages in the pool?
			if (--num_erased_pages_available == 0)
				return FF_ERROR(NO_ERASED_PAGES);

 			if (vp_next_erased_page < m_p_toc->vp_last_erased_page)

				// Increment the pointer to the next erased page.
 				vp_next_erased_page++;
			else
				// Start from the top of the list again.
				vp_next_erased_page = m_p_toc->vp_first_erased_page;

			// Get the real address of the next erased page.
			flash_address_erased = Get_Real_Flash_Address(vp_next_erased_page);
			CT_ASSERT((Is_Page_Erased(flash_address_erased)), Get_Real_Erased_Page);
			CT_ASSERT((!Is_Page_Bad(flash_address_erased)), Get_Real_Erased_Page);

			// Is this one erased?
			if (Get_Page_State(flash_address_erased) == FF_PAGE_STATE_ERASED)
			{
				// This one is erased.  
				// The vp of the next erased page to be assigned gets mapped
				// to the real address of an erased page.
				Remap_Virtual_To_Real(m_p_toc->vp_erased_page_out, flash_address_erased);

				// The vp of the next erased page gets mapped to the real address
				// of the page being erased.
				Remap_Virtual_To_Real(vp_next_erased_page, flash_address_erasing);
				break;
			}

			// If the next page is not erased, it must be erasing.
			CT_ASSERT((Get_Page_State(flash_address_erased) == FF_PAGE_STATE_ERASING), Get_Real_Erased_Page);
		} // while
		
	} // this page is currently being erased.

	// Validate the state of the erased page.
	CT_ASSERT((Get_Page_State(flash_address_erased) == FF_PAGE_STATE_ERASED), Get_Real_Erased_Page);
	CT_ASSERT((Is_Page_Erased(flash_address_erased)), Get_Real_Erased_Page);
	CT_ASSERT((!Is_Page_Bad(flash_address_erased)), Get_Real_Erased_Page);

	// Return real address to caller.
	*p_flash_address_erased = flash_address_erased;
	return OK;

} // Get_Real_Erased_Page

/*************************************************************************/
// Exchange_Erased_Page
// The page with the specified virtual address has just been erased.
// This page is currently mapped.
// Exchange this erased page for an erased page with a different address.
// The purpose of this is for wear leveling.
/*************************************************************************/
void FF_Page_Map::Exchange_Erased_Page(Flash_Address flash_address)
{
 	TRACE_ENTRY(FF_Page_Map::Exchange_Erased_Page);
 	
	// Get the real virtual address currently assigned to the real
	// address that needs to be exchanged for a new erased page.
	U32 virtual_address = Get_Virtual_Flash_Address(flash_address);

	// Get the current state of the page that needs to be exchanged.
	// with an erased page.
	CT_ASSERT((Get_Page_State(flash_address) == FF_PAGE_STATE_MAPPED), Exchange_Erased_Page);

	// Get the real address assigned to the next erased page.
	Flash_Address flash_address_erased;
	Status status = Get_Real_Erased_Page(&flash_address_erased);
	if (status != OK)

		// If no erased pages are available, this page will not get exchanged.
		// This is not a problem.
		return;

	// Add this page to the erased page pool.
	Add_Erased_Page(flash_address);

	// Now our virtual_address is mapped to a deleted page.
	// Get the address of the deleted page.
	Flash_Address flash_address_deleted = Get_Real_Flash_Address(virtual_address);
	CT_ASSERT((Get_Page_State(flash_address_deleted) == FF_PAGE_STATE_DELETED), Exchange_Erased_Page);

	// Set the state of the erased page to be the same as the state of the page 
	// that we just replaced.
	Set_Page_State(flash_address_erased, FF_PAGE_STATE_MAPPED);

	// Map our virtual page to the erased page.
	Remap_Virtual_To_Real(virtual_address, flash_address_erased);
 		
	// Map the deleted page.
	Remap_Virtual_To_Real(m_p_toc->vp_erased_page_out, flash_address_deleted);
 		
	// Decrement number of remaining availabale erased pages.
	m_p_toc->num_erased_pages_available--;
	
	// Increment the number of used erased pages.
	m_p_toc->num_erased_pages_allocated++;
	
 	// Verify that we still have erased pages.
 	CT_ASSERT((m_p_toc->num_erased_pages_available), Exchange_Erased_Page);
 		
 	// See if we have erased page map entries before the end.
 	if (m_p_toc->vp_erased_page_out < m_p_toc->vp_last_erased_page)
 	{
 		// Increment the pointer to the next erased page.
 		m_p_toc->vp_erased_page_out++;
 		return;
 	}
 	
 	// We must be at the end of the erased page map.
 	CT_ASSERT ((m_p_toc->vp_erased_page_out == 
		(m_p_toc->vp_first_erased_page + m_p_toc->num_erased_pages_allocated
		+ m_p_toc->num_erased_pages_available - 1)),
 		Get_Erased_Page);
 		
	// Start from the top of the list again.
	m_p_toc->vp_erased_page_out = m_p_toc->vp_first_erased_page;
	
 	VALIDATE_MAP;

} // Exchange_Erased_Page
 
/*************************************************************************/
// FF_Context::Erase_Page_Block
// Start a controller operation to erase one block.
/*************************************************************************/
Status FF_Controller::Erase_Page_Block(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	U32 flags)
{
 	TRACEF( TRACE_L5, 
		(EOL " Erase_Page_Block, unit = %d", 
		flash_address.Unit_Index()));
 	
	// Validate address.
	CT_ASSERT((flash_address.Index() < Flash_Address::Num_Virtual_Pages()), Erase_Page_Block);	

	// Make sure we have a block boundary.
	CT_ASSERT((flash_address.Page_Index() % Flash_Address::Sectors_Per_Block() == 0),
		Erase_Page_Block);
 	
	// Make sure we don't write over the bad block table.
	if (m_p_flash->m_page_map.Get_Page_State(flash_address) == FF_PAGE_STATE_BAD_BLOCK_TABLE)
	{
		if ((flags & FF_WRITING_BAD_BLOCK_TABLE) == 0)
			return FF_ERROR(WRITING_BAD_BLOCK_TABLE);
	}

 	// Get page map entry for each page in the block
	Flash_Address next_flash_address = flash_address;
	Status status;
	for (U32 index = 0; index < Flash_Address::Sectors_Per_Block(); index++)
	{
        // Get the page state of this page.
		FF_PAGE_STATE page_state = m_p_flash->m_page_map.
			Get_Page_State(next_flash_address);
 		
 		// Validate state of map entry.  Note that FF_PAGE_STATE_REPLACEMENT
		// and FF_PAGE_STATE_ERASED get changed to FF_PAGE_STATE_ERASING during
		// an erase operation to prevent them from being assigned..
 		switch (page_state)
 		{
 			case FF_PAGE_STATE_TOC:
 			case FF_PAGE_STATE_PAGE_MAP_TABLE:
 			case FF_PAGE_STATE_BAT:
 			case FF_PAGE_STATE_PAGE_MAP:
 			case FF_PAGE_STATE_UNMAPPED:
 			case FF_PAGE_STATE_MAPPED:
 			case FF_PAGE_STATE_ERASING:
 			case FF_PAGE_STATE_DELETED:
 			case FF_PAGE_STATE_BAD_PAGE:
 			case FF_PAGE_STATE_BAD_BLOCK_TABLE:
 				break;
 			
 			default:
 				status = FF_ERROR(INVALID_PAGE_STATE);
				CT_Log_Error(CT_ERROR_TYPE_FATAL,
					"Erase_Page_Block", 
					"Invalid page state for erase",
					status,
					0);
				return status;
 		}
 		
		// Increment to next address in block.
		next_flash_address.Increment_Page();
 	
	} // for
 	
	// Initialize list of caller's context so that we can allocate
	// a child context.
	LIST_INITIALIZE(&p_callback_context->m_list);

	// Allocate FF_Controller_Context as child of caller's context.
	// So, when we terminate, caller's context will run.
	FF_Controller_Context *p_controller_context = 
		(FF_Controller_Context *)p_callback_context->Allocate_Child(sizeof(FF_Controller_Context));
	if (p_controller_context == 0)
	{
		// Return to calling context.  No child context was created.
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Save parameters in context.
	p_controller_context->m_p_controller = &m_p_flash->m_controller;
	p_controller_context->m_flash_address = flash_address;
	p_controller_context->m_flags = flags;

	// Start the erase.
	p_controller_context->Set_Callback(&Erase_Wait_Controller);
	p_callback_context->Make_Children_Ready();

	return OK;

} // FF_Controller::Erase_Page_Block

/*************************************************************************/
// Erase_Wait_Controller
// This context will keep getting scheduled to run until the controller
// is available.
/*************************************************************************/
void FF_Controller::Erase_Wait_Controller(void *p_context, Status status)
{	
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
 	TRACEF( TRACE_L5, 
		(EOL " Erase_Wait_Controller, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Check to see if the local buss is busy.
	if (p_controller->Is_Buss_Busy(p_controller_context))
	{
		// The controller is busy. Our context has been placed on a wait list.
		// When the controller is not busy, this context will be scheduled to run.
		TRACE_STRING(TRACE_L5, EOL "Buss is busy");
		return;
	}

	// Check to see if the logical unit is busy.
	if (p_controller->Is_Unit_Busy(p_controller_context))
	{
		// The unit is busy. Our context has been placed on a wait list.
		// When the unit is not busy, this context will be scheduled to run.
 		TRACEF( TRACE_L5, (EOL " Unit is busy"));
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		return;
	}

	// When interrupt goes off, call Erase_Complete
 	p_controller_context->Set_Callback(&FF_Controller::Erase_Complete);

	// Get flash address from flash address.
	Flash_Page_Map flash_page_map;
	status = p_flash->m_block_address.Get_Flash_Page_Map(
		p_controller_context->m_flash_address, flash_page_map, p_controller_context->m_flags);
	
	// Make sure we don't erase the bad block table.
	if (status != OK)
	{
		// We should not be erasing the bad block table.
		// Mark the buss not busy and check to see if another
		// context is waiting for the buss.
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		
		// Mark this unit not busy and check to see if another
		// context is waiting for this unit.
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);

		p_controller_context->Terminate(p_controller_context, status);
		return;
	}

	FF_Check_Break_Block(p_controller_context->m_flash_address);

	// Start the unit erase operation.
	status = p_flash->m_p_device->Erase_Page_Block(
		p_controller_context,
		p_controller_context->m_flash_address,
		flash_page_map);

	// The buss is free as soon as we start the erase command.
	// Mark the buss not busy and check to see if another
	// context is waiting for the buss.
	p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);

	// Was the operation started?
	if (status != OK)
	{
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);
		Callback_Context::Terminate(p_controller_context, status);
	}

}  // FF_Controller::Erase_Wait_Controller

/*************************************************************************/
// FF_Controller::Erase_Complete
// The device has finished erasing the page block.
/*************************************************************************/
void FF_Controller::Erase_Complete(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL " Erase_Complete, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Check to see if the local buss is busy.
	// We can't read status if the buss is busy.
	if (p_controller->Is_Buss_Busy(p_controller_context))
	{
		// The controller is busy. Our context has been placed on a wait list.
		// When the controller is not busy, this context will be scheduled to run.
		TRACE_STRING(TRACE_L5, EOL "Buss is busy");
		return;
	}

	// Did we get a timeout?
	if (status == FF_ERROR_CODE(TIMEOUT))
		p_flash->m_p_device->Abort_Command(p_controller_context->m_flash_address);
	
	// Get the status of the erase operation.
	// The status has one byte for each device block erased.
	Flash_Device_Status device_status;
	Status command_status = p_flash->m_p_device->Get_Device_Status(
		p_controller_context->m_flash_address, 

		// controller status does not have any meaning for an erase operation
		0,
		&device_status);

	// Mark the buss not busy and check to see if another
	// context is waiting for the buss.
	p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);

	// Mark this unit not busy and check to see if another
	// context is waiting for this unit.
	p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);

	// Did we get a timeout?
	if (status == FF_ERROR_CODE(TIMEOUT))
	{
		// Timeout occurred.
		status = FF_ERROR_CODE(ERASE_TIMEOUT);

		if (device_status.device_is_ready)
		{
			// The unit is ready, so we failed to get the interrupt.
			
#ifdef TRACE_ERROR
		Tracef(EOL "Erase_Complete Timeout error detected, array %X, column %X, block %X, page %X",
			p_controller_context->m_flash_address.Array(), 
			p_controller_context->m_flash_address.Column(),
			p_controller_context->m_flash_address.Block(), 
			p_controller_context->m_flash_address.Page_Index() );
		Tracef(EOL " unit status = %LX, %LX, %LX %LX", 
			device_status.bank_status[0], 
			device_status.bank_status[1], 
			device_status.bank_status[2], 
			device_status.bank_status[3]);

#endif
			// Set status to OK and continue.
			status = OK;
			p_controller_context->Set_Status(OK);
			
		} // unit is actually ready
	} // timeout

	// Check the status.
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Erase_Complete", 
			"Erase failed",
			status,
			0);
		p_controller_context->Terminate(p_context, status);
		return;
	}

	// Set error code for testing.
	IF_FF_ERROR(command_status != OK, ERASE_ERROR)
		command_status = FF_ERROR_CODE(ERASE_ERROR);

	// Did we get an erase error?
	if (command_status != OK)
	{
		// Increment the number of erase errors detected for this device.
		p_flash->m_stats.Inc_Num_Erase_Errors(p_controller_context->m_flash_address.Device_Index());

//#ifdef TRACE_ERROR
#if 0
		Tracef(EOL "Erase error detected, array %X, column %X, block %X, page %X",
			p_controller_context->m_flash_address.Array(), 
			p_controller_context->m_flash_address.Column(),
			p_controller_context->m_flash_address.Block(), 
			p_controller_context->m_flash_address.Page_Index() );
		Tracef(EOL " unit status = %LX, %LX, %LX %LX", 
			device_status.bank_status[0], 
			device_status.bank_status[1], 
			device_status.bank_status[2], 
			device_status.bank_status[3]);
#endif

		// Note that we don't do anything about erase errors.
		// There errors are not recoverable.  The block is simply a bad block.
		// We will catch the problem later when we verify the data.
	} 

 	// Get page map entry for each page in the block
	// and mark it erased.
	Flash_Address next_flash_address = p_controller_context->m_flash_address;
	U32 sectors_per_block = Flash_Address::Sectors_Per_Block();
	for (U32 device_index = 0; device_index < sectors_per_block; 
		device_index++)
	{
#if 0
		// Check to see if this page is mapped.  
		// This happens when a page from the erased page pool is erased,
		// Then, while the erase is happening, the page is assigned.
		// If this happens, we do not set the erased flag.
		U32 page_state = p_page_map->Get_Page_State(next_flash_address);
		if ((page_state == FF_PAGE_STATE_BAT) || (
			(page_state != FF_PAGE_STATE_MAPPED) && (page_state != FF_PAGE_STATE_BAD_PAGE)))
#endif

		p_page_map->Set_Page_Erased(next_flash_address, 1);

		// Increment to next address in block.
		next_flash_address.Increment_Page();
 	
	} // for

	// If we are erasing the entire device, don't verify.
	// We will verify when the new bad block table is created.
	if ((p_controller_context->m_flags & FF_WRITING_BAD_BLOCK_TABLE) == 0)
	{
		// if verify_erase specified in the config,
		// or if an erase error was detected.
		if (p_flash->m_flash_config.verify_erase || (command_status != OK))
		{
			// Set up context to verify each page in the block.
			p_controller_context->m_page_count = 0;
			p_controller_context->Set_Callback(&FF_Controller::Verify_Next_Erased_Page);
			
			// Verify that the first page in the block is erased.
			// Verify_Next_Erased_Page will be called when page has been verified.
			Status status = p_controller->Read_Page(
				p_controller_context, 
				p_controller_context->m_flash_address, 
				p_flash->m_p_erased_page,
				FF_VERIFY_ERASE); 
	
			if (status != OK)
				p_controller_context->Terminate(p_controller_context, status);
	
			return;
		}
	}
		
	// Terminate the context.  The status of the erase operation
	// will be passed back to the parent.
	p_controller_context->Terminate(p_controller_context, OK);

} // FF_Controller::Erase_Complete

/*************************************************************************/
// FF_Controller::Verify_Next_Erased_Page
// We verify each page if an erase error was detected, or if
// verify_erase was specified in the config.
/*************************************************************************/
void FF_Controller::Verify_Next_Erased_Page(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL " Verify_Next_Erased_Page, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Check status of last verify operation.
	if (status != OK)	
	{
		if (status == FF_ERROR_CODE(ECC))
		{
			// There won't be any ECC when the device is new.
			status = OK;
			p_controller_context->Set_Status(OK);
		}
		else
		{
#ifdef TRACE_ERROR
			Tracef(EOL "Erase verify error detected, array %X, column %X, block %X, page %X",
				p_controller_context->m_flash_address.Array(), 
				p_controller_context->m_flash_address.Column(),
				p_controller_context->m_flash_address.Block(), 
				p_controller_context->m_flash_address.Page_Index() );
#endif

			// Set the bad page flag for this page.
			p_page_map->Set_Bad_Page(p_controller_context->m_flash_address);

			p_flash->m_stats.Inc_Num_Erase_Errors(
				p_controller_context->m_flash_address.Unit_Index());
		}
	}

	// Have we verified the last page in the block?
	if (++p_controller_context->m_page_count == Flash_Address::Sectors_Per_Block())
	{
		// Terminate the context.  The status of the erase operation
		// will be passed back to the parent.
		p_controller_context->Terminate(p_context, status);
		return;
	}

	// Increment to next address in block.
	p_controller_context->m_flash_address.Increment_Page();
 	
	// Verify that the next page in the block is erased.
	// Verify_Next_Erased_Page will be called when page has been verified.
	status = p_controller->Read_Page(
		p_controller_context, 
		p_controller_context->m_flash_address, 
		p_flash->m_p_erased_page,
		FF_VERIFY_ERASE); 

	// TODO check status
	return;

} // Verify_Next_Erased_Page

/*************************************************************************/
// FF_Page_Map::Reset_Erase_In_Progress
/*************************************************************************/
void FF_Page_Map::Reset_Erase_In_Progress()
{
	CT_ASSERT((m_erase_in_progress == 1), Reset_Erase_In_Progress);
	m_erase_in_progress = 0;

	// See if any contexts are waiting for an erased page.
	while (!LIST_IS_EMPTY(&m_list_wait_erased_page))
	{
		// There is a context waiting for an erased page.
		// Remove it from the list and put it on the ready list.
		Callback_Context *p_waiting_context = 
			(Callback_Context *)LIST_REMOVE_TAIL(
			&m_list_wait_erased_page);
		m_p_flash->m_stats.Dec_Num_Waits_Erased_Page();

		p_waiting_context->Make_Ready();
	}

} // Reset_Erase_In_Progress

