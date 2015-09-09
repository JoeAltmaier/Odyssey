/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfFormat.cpp
// 
// Description:
// This module implements the Flash File format operation.
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfCache.h"
#include "FfPageMap.h"
#include "FfRequest.h"
#include "FfStats.h"
#include <String.h>

#if 0
// Turn on TRACE_FORMAT
#define TRACE_FORMAT Tracef
#else
// Turn off TRACE_FORMAT -- if 1 then do nothing, else call Tracef
#define TRACE_FORMAT 1? 0 : Tracef  
#endif

#define TRACE_BLOCKS_LEFT_TO_FORMAT

/*************************************************************************/
// FF_Interface::Format
/*************************************************************************/
Status FF_Interface::Format(
	FF_CONFIG *p_config,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{	
 	TRACE_ENTRY(FF_Interface::Format);
 	
	// Create page map.
	Status status = m_page_map.Create(p_config);
	if (status != OK)
		return status;

	if (p_config->erase_all_pages != ERASE_ALL_PAGES)
	{
		// Mark bad blocks from the bad block table so that we don't try to
		// erase any bad blocks.
		status = m_block_address.Mark_Bad_Blocks();
		if (status != OK)
			return status;

		// Since we initialized the page map, we must remap the
		// system flash addresses.
		m_block_address.Initialize_System_Flash_Addresses();
	}
	else
	{
		// Initialize bad block object.
		// Every block must map one to one.
		status = m_block_address.Initialize(this);
		if (status != OK)
			return status;
	}

	// Initialize the first bad page address so that bad pages
	// encountered can count down.
	m_page_map.Initialize_Bad_Page_Addresses();

	// Save the format config record, since the caller's record
	// could disappear.
	memcpy(&m_flash_config, p_config, sizeof(m_flash_config));

	// Create a child context to erase toc and page map.
	return FF_Request_Context::Format(
		this,
		&m_flash_config,
		p_context,
		p_completion_callback);

} // FF_Interface::Format

/*************************************************************************/
// FF_Request_Context::Format
// Create context to format each unit.
/*************************************************************************/
Status FF_Request_Context::Format(
	FF_Interface *p_flash,
	FF_CONFIG *p_config,
	void *	p_context,
	FF_CALLBACK *p_completion_callback)
{
	// Allocate FF_Request_Context.  This is the parent context that will
	// contain all of the request parameters.
	FF_Request_Context *p_request_context = 
		(FF_Request_Context *)Callback_Context::Allocate(sizeof(FF_Request_Context));
	if (p_request_context == 0)
	{
		// Return to calling context.  No context was created.
		p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Initialize request context.
	p_request_context->m_p_flash = p_flash;
	p_request_context->m_transfer_byte_count = 0;
	p_request_context->m_logical_byte_address = 0;
	p_request_context->m_p_completion_callback = p_completion_callback;
	p_request_context->m_p_context = p_context;
	p_request_context->m_p_config = p_config;

	// Set callback for parent context.  
	p_request_context->Set_Callback(&FF_Request_Context::Create_Child_Format_Contexts);

	// Are we erasing the entire drive?
	if (p_config->erase_all_pages == ERASE_ALL_PAGES)
	{
		Create_Child_Format_Contexts(p_request_context, OK);
		return OK;
	}

	// Prepare to erase the first copy of the toc and page map.
	// When complete, schedule Page_Map_Opened.
	Status status = p_flash->m_page_map.Erase_Toc_And_Page_Map(p_request_context, 1);
	if (status != OK)
	{
		p_request_context->Terminate(p_context, status);
		return status;
	}
	return OK;
	
} // Format

/*************************************************************************/
// FF_Request_Context::Create_Child_Format_Contexts.
// Come here when toc and page map have been formatted.
// Create context to format each unit.
/*************************************************************************/
void FF_Request_Context::Create_Child_Format_Contexts(void *p_context, Status status)
{
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_request_context->m_p_flash;
	
	// Set callback for parent context.  
	p_request_context->Set_Callback(&FF_Request_Context::Format_Erase_Complete);

	// Create a child context to erase pages in each unit.
	for (U32 unit_index = 0; unit_index < Flash_Address::Num_Units(); 
		unit_index++)
	{
		status = p_flash->m_controller.Create_Format_Unit_Context(
			p_request_context,
			unit_index, p_request_context->m_p_config);
		if (status != OK)
		{
			Callback_Context::Terminate(p_request_context, FF_ERROR(NO_MEMORY));
			return;
		}
	}

	// Schedule the child contexts to run.
	p_request_context->Make_Children_Ready();
	return;
	
} // Create_Child_Format_Contexts

/*************************************************************************/
// FF_Controller::Create_Format_Unit_Context
// Create a context to erase page blocks in the unit
// We may not erase more than one page block if we are erasing the
// minimum.
/*************************************************************************/
Status FF_Controller::Create_Format_Unit_Context(
	Callback_Context *p_callback_context,
	U32 unit_index, FF_CONFIG *p_config)
{
	TRACE_ENTRY(FF_Controller::Create_Format_Unit_Context);

	// Allocate a child context to format this unit.
	FF_Controller_Context *p_child = 
		(FF_Controller_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Controller_Context));
	if (p_child == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Create_Format_Unit_Context", 
			"Allocate_Child failed",
			FF_ERROR(NO_MEMORY),
			0);
		
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Set parameters for child context.
	p_child->m_p_controller = &m_p_flash->m_controller;
	p_child->m_flash_address.Initialize();
	p_child->m_flash_address.Unit_Index(unit_index);
	p_child->m_page_count = 0;
	p_child->m_erase_all_pages = p_config->erase_all_pages;

	// Set start address for child context.
	p_child->Set_Callback(&FF_Controller::Format_Unit);

	return OK;

} // FF_Controller::Create_Format_Unit_Context

/*************************************************************************/
// FF_Controller::Format_Unit
// This context will erase all blocks in the unit.
// When this context is called:
//	It was called for the first time (p_controller_context->m_page_count == 0)
//	or an erase was finished by Erase_Page_Block_If_Good
/*************************************************************************/
void FF_Controller::Format_Unit(void *p_context, Status status)
{
	TRACE_ENTRY(FF_Controller::Format_Unit);

	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	if (status != OK)
	{
		if (status == FF_ERROR_CODE(VERIFY))

			// If the status is verify error, then the page was set to bad page,
			// so we don't want to return an error to the parent.
			status = OK;
		else
		{
			p_controller_context->Terminate(p_context, status);
			return;
		}
	}

	// The first time we get called, we have not yet started an erase operation.
	if (p_controller_context->m_page_count == 0)
	{		
		// Start erase operation for the first block.
		p_controller->Erase_Page_Block_If_Good(p_controller_context);
		return;
	}

	// The page block has been erased.
	
	// Are we erasing the entire drive?
	int num_pages_left_to_erase;
	if (p_controller_context->m_erase_all_pages != ERASE_ALL_PAGES)
	{
		// Assign the real addresses from the last erased block
		// to virtual addresses.
		// Assign_Virtual Returns true if minimum number of erased pages has been assigned.
		num_pages_left_to_erase = 
			p_page_map->Assign_Virtual(p_controller_context->m_flash_address);

#ifdef TRACE_BLOCKS_LEFT_TO_FORMAT
		// The first sector of every block, display how many 
		// blocks we have left to erase.
		if (num_pages_left_to_erase)
		{
			if ((num_pages_left_to_erase % Flash_Address::Sectors_Per_Block()) == 0)
			{
				U32 num_blocks_left = num_pages_left_to_erase 
					/ Flash_Address::Sectors_Per_Block();
				Tracef(" %d", num_blocks_left);
			}
		}
#endif
	}

	// Increment the block number for the next erase.
	if (p_controller_context->m_flash_address.Increment_Unit_Block() == 0)
	{
		// The last block has been erased, so we can terminate and
		// free this erase unit context.  When the last erase unit
		// context has terminated, the parent context
		// will be scheduled to run again.
		p_controller_context->Terminate(p_context, OK);
		return;
	}
	
	CT_ASSERT((p_controller_context->m_flash_address.Index() 
		< Flash_Address::Num_Virtual_Pages()), Format_Unit);	

	// See if we need to erase more pages.
	if (p_controller_context->m_erase_all_pages || (num_pages_left_to_erase != 0))
	{
		// Increment the number of blocks erased.
		p_controller->Erase_Page_Block_If_Good(p_controller_context);
		return;
	}

	// We are erasing the minimum number of pages, and we have finished erasing.
	// Assign the remaining pages for this unit.
	// block will go to zero when we increment flash address to its max.
	Flash_Address last_flash_address_mapped;
	while (TRUE)
	{
		// Assign virtual addresses to the next block of pages.
		p_page_map->Assign_Virtual(p_controller_context->m_flash_address);
		last_flash_address_mapped = p_controller_context->m_flash_address;

		// Step to the next page block.
		if (p_controller_context->m_flash_address.Increment_Unit_Block() == 0)
		
			// The last block has been processed.
			break;
	}

#ifdef TRACE_FORMAT
	TRACE_FORMAT(EOL " Finished erasing %d blocks for unit_index %X, last erased address = %X, last mapped address = %X", 
		p_controller_context->m_page_count, p_controller_context->m_flash_address.Unit_Index(), 
		p_controller_context->m_flash_address.Index(), last_flash_address_mapped);
#endif

	p_controller_context->Terminate(p_context, status);

} //  FF_Controller::Format_Unit

/*************************************************************************/
// FF_Controller::Erase_Page_Block_If_Good
// If the page block is not marked bad, erase it.
// Also check to be sure we don't erase the bad block table.
/*************************************************************************/
Status FF_Controller::Erase_Page_Block_If_Good(FF_Controller_Context *p_controller_context)
{
	TRACE_ENTRY(FF_Controller::Erase_Page_Block_If_Good);

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Are we erasing the entire drive?
	U32 flags = 0;
	if (p_controller_context->m_erase_all_pages != ERASE_ALL_PAGES)
	{
		// Check to see if the block is a bad block or a system block.
		while (true)
		{
			if (m_p_flash->m_page_map.Is_System_Block(p_controller_context->m_flash_address))
			{
				// Increment the block address.
				if (p_controller_context->m_flash_address.Increment_Unit_Block() == 0)
				{
					// The last block has been erased, so we can terminate and
					// free this erase unit context.  When the last erase unit
					// block context has terminated, the parent context
					// will be scheduled to run again.
					p_controller_context->Terminate(p_controller_context, OK);
					return OK;
				}
				continue; // continue while loop to next block.
			}
			
			// It's not a system block, so continue below.
			break;
		}
	}
	else
		flags = FF_WRITING_BAD_BLOCK_TABLE;

	// Increment the number of blocks erased.
	p_controller_context->m_page_count++;
	
	// Start erase operation for this block.
	Status status = Erase_Page_Block(
		p_controller_context, 
		p_controller_context->m_flash_address,
		flags);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Format_Unit", 
			"Erase_Page_Block failed",
			status,
			0);
		p_controller_context->Terminate(p_controller_context, status);
	}

	return status;

} // Erase_Page_Block_If_Good

/*************************************************************************/
// FF_Request_Context::Format_Erase_Complete
// The child contexts have finished erasing all the units.
// Lay out the virtual addresses for the page map.
// Return to caller.
/*************************************************************************/
void FF_Request_Context::Format_Erase_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Format_Erase_Complete);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Are we erasing the whole device?
	if (p_request_context->m_p_config->erase_all_pages != ERASE_ALL_PAGES)
	{
		if (status == OK)
		{
			// Lay out the virtual addresses for the page map.
			p_request_context->m_p_flash->m_page_map.
				Layout_Page_Map(p_request_context->m_p_config->erase_all_pages);
				
			// Open the cache.
			// Map the number of pages allocated to the user.
			status = p_request_context->m_p_flash->m_page_map.
				Open_Cache();
		}
	}

	// Call user's callback
	p_request_context->m_p_completion_callback(
		p_request_context->m_transfer_byte_count, 
		p_request_context->m_logical_byte_address,
		status,
		p_request_context->m_p_context);

	// Terminate this context
	Callback_Context::Terminate(p_request_context, status);

} // Format_Erase_Complete

