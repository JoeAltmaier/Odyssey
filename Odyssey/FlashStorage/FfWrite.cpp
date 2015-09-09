/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfWrite.cpp
// 
// Description:
// This file implements the BSA Write request. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCache.h"
#include "FfCommon.h"
#include "Dma.h"
#include "FfController.h"
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
// Forward references
/*************************************************************************/
void Write_Cache_Read_Callback(
	U32 transfer_byte_count,
	I64 logical_byte_address,
	Status status,
	void *p_context);

/*************************************************************************/
// Write transfer_byte_count bytes from specified buffer
// into logical_byte_address of the file.
// Call callback routine with pointer to context when write has completed.
/*************************************************************************/
Status FF_Interface::Write(
	FF_SGL *p_sgl, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	
	// Check to make sure we still have enough replacement pages.
	Status status = m_page_map.Check_Replacement_Pages();
	if (status != OK)
		return status;

	m_stats.Inc_Num_Write_Bytes_Total(transfer_byte_count);

	// Create a child context to write each virtual page.  
	// Each child context will call Write_Cache.
	return FF_Request_Context::Create_Child_Read_Write_Contexts(
		this,
		p_sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback,
		&FF_Request_Context::Write_Cache);

} // FF_Write

/*************************************************************************/
// FF_Request_Context::Write_Cache
// Write one page to the cache.
// At this level we are writing a virtual page.
/*************************************************************************/
void FF_Request_Context::Write_Cache(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Write_Cache);
 		
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Increment the total number of page writes.
	p_flash->m_stats.Inc_Num_Page_Writes();

	// See if page is in cache.
	status = CM_Open_Page(p_flash->m_cache_handle, 
		p_request_context->m_virtual_address,
		CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_LOCKED | CM_CALLBACK_IF_NO_FRAME,
//		CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_LOCKED | CM_CALLBACK_IF_NO_FRAME | CM_SECONDARY,
		p_request_context,
		&p_request_context->m_p_page_frame,
		&p_request_context->m_page_handle);
		
	TRACEF(TRACE_L5, ("\nWrite_Cache Cache CM_Open_Page status = %X, virtual address = %X", 
		status, p_request_context->m_virtual_address));
	switch (status)
	{
		case OK:
		
			// The page is in the cache.
			p_flash->m_stats.Inc_Num_Page_Writes_Cache_Hit();

			// Copy the data from the buffer to the cache.
			Write_DMA_Buffer_To_Cache(p_context, OK);
			break;
			
		case CM_ERROR_CACHE_MISS:
		
			// The page is not in the cache. 
			p_flash->m_stats.Inc_Num_Page_Writes_Cache_Miss();

			// Our context has a pointer to an empty page frame. 
			// See if we need to read the page.
			if (p_request_context->m_transfer_byte_count < Flash_Address::Bytes_Per_Page())
			{
				// The write is for less than a full page,
				// so we will need to read the page into the cache.
				p_flash->m_stats.Inc_Num_Page_Writes_Partial_Cache_Miss();

				// Set to call Write_DMA_Buffer_To_Cache
				// when read has completed.
				p_request_context->Set_Callback(&Write_DMA_Buffer_To_Cache);

				// Our context has a pointer to an empty page frame.
 				// Get real address in flash for this page. 
 				Flash_Address flash_address = p_flash->m_page_map.
					Get_Real_Flash_Address(
					p_request_context->m_virtual_address);

				// Start the read operation to read the real page
				// into the cache.
				status = p_flash->m_controller.Read_Page(
					p_request_context,
					flash_address,
					p_request_context->m_p_page_frame);

				if (status != OK)
					p_request_context->Terminate_With_Error(status);
				break;
					
			}
			
			// The write operation is for a full page.
			// We have an empty page frame in the cache.
			// Write the data to the cache.
			Write_DMA_Buffer_To_Cache(p_context, OK);
			break;
						
		case CM_ERROR_PAGE_LOCKED:
		
			// FF_Request_Context::Write_Cache will be called when the page
			// is unlocked.
			break;
		
		case CM_ERROR_MAX_DIRTY_PAGES:
			// There are too many dirty pages.
			// FF_Request_Context::Write_Cache will be called when 
			// a page is available.
			break;

		case CM_ERROR_NO_PAGE_FRAMES:
		
			// No page frames are available.
			// FF_Request_Context::Write_Cache will be called when 
			// a page is available.
			break;
		
		default:
		
			// Unexpected error
			CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
				"FF_Request_Context::Write_Cache", 
				"Unexpected cache error",
				status,
				0);
				
			// Terminate context with error
			p_request_context->Terminate_With_Error(status);
			break;
	}
	
} // FF_Request_Context::Write_Cache

/*************************************************************************/
// Write_Cache_Read_Callback.
// Called by FF_Read when read of page into cache has completed.
/*************************************************************************/
void Write_Cache_Read_Callback(
	// number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	Status status,

	// pointer passed in to Flash File
	void *p_context)
{
	TRACEF(TRACE_L5, ("\nWrite_Cache_Read_Callback status = %X, transfer_byte_count = %d", 
		status, transfer_byte_count));
 	
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	if (status != OK)
	{
		// Terminate context with error
		Callback_Context::Terminate(p_request_context, status);
		return;
	}

	// Schedule context to start DMA operation.
	p_request_context->Set_Callback(&FF_Request_Context::Write_DMA_Buffer_To_Cache);
	p_request_context->Make_Ready();

	// Return to Read and scheduler.

} // Write_Cache_Read_Callback

/*************************************************************************/
// Write_DMA_Buffer_To_Cache.
// First, get an erased page, then
// start DMA operation to transfer data from buffer to cache.
/*************************************************************************/
void FF_Request_Context::Write_DMA_Buffer_To_Cache(void *p_context, Status status)
{
 	TRACE_ENTRY(Write_DMA_Buffer_To_Cache);
 	
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	TRACEF(TRACE_L5, ("\nFF_Request_Context::Write_DMA_Buffer_To_Cache status = %X", 
		status));

	// Get an erased page for this data.
	// This remaps the page to a new page that has been erased.
	status = p_flash->m_page_map.
		Get_Erased_Page(p_request_context->m_virtual_address);
	if (status != OK)
	{
		// No erased pages are available.
		// Abort our open page.
		CM_Abort_Page(p_flash->m_cache_handle, 
			p_request_context->m_page_handle);

		// Set up to be scheduled again when an erased page is available.
		p_request_context->Set_Callback(&FF_Request_Context::Write_Cache);
		p_flash->m_page_map.Wait_For_Erased_Page(p_request_context);

		// Context will be called when an erased page is available
		return;
	}
	
	TyDma *p_ty_dma;

 	// Is this a full page operation?
 	if (p_request_context->m_transfer_byte_count == Flash_Address::Bytes_Per_Page())
 	{
		// Create TyDma from SGL to transfer page from client's buffer to cache.
		status = p_request_context->m_sgl.Build_TyDma(
			p_request_context->m_p_page_frame, // transfer address is page frame in cache,
			Flash_Address::Bytes_Per_Page(), // transfer one page
			FF_SGL::Transfer_Address_Is_Dest,
	 		&Write_DMA_Complete,
	 		p_context,
			TyDma::flagsDefault,
			&p_ty_dma);

 	}
 	else
 	{
		// This write is for a partial page.
		p_flash->m_stats.Inc_Num_Page_Writes_Partial();

 		// Calculate offset in page frame for partial page.
 		char *p_page_frame = (char *)p_request_context->m_p_page_frame 
 			+ Flash_Address::Offset_From_Logical_Byte_Address((U32)p_request_context->m_logical_byte_address);
 			
		// Create TyDma from SGL to transfer partial page from client's buffer to cache.
		status = p_request_context->m_sgl.Build_TyDma(
			p_page_frame, // transfer address is calculated offset from page frame in cache.
			p_request_context->m_transfer_byte_count,
			FF_SGL::Transfer_Address_Is_Dest,
	 		&Write_DMA_Complete,
	 		p_context,
			TyDma::flagsDefault,
			&p_ty_dma);
 	}
	
	// Did we create a good TyDma?
	if (status != OK)
	{
		p_request_context->Terminate(p_context, status);
		return;
	}
	
	// Start DMA transfer operation.
	Dma::Transfer(p_ty_dma);

 	// Scheduler will call FF_Request_Context::Write_DMA_Complete when DMA complete.

} // Write_DMA_Buffer_To_Cache

/*************************************************************************/
// FF_Request_Context::Write_DMA_Complete
// Called by the DMA interrupt handler when the DMA operation to transfer the
// write data to the cache has completed.
// Note that we are not running in the thread of the flash service, so
// we don't want to do any work here.  It would be especially bad if we
// called CM_Close because this attempts to use a semaphore from
// an interrupt handler.
/*************************************************************************/
void FF_Request_Context::Write_DMA_Complete(void *p_context, Status status)
{
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;
	
	TRACEF(TRACE_L5, ("\nFF_Request_Context::Write_DMA_Complete status = %X", 
		status));

	p_request_context->Set_Status(status);
	p_request_context->Set_Callback(&Write_Request_Complete);
	p_request_context->Make_Ready();
	
} // FF_Request_Context::Write_DMA_Complete

/*************************************************************************/
// FF_Request_Context::Write_Request_Complete
// Called by the Scheduler when the DMA operation to transfer the
// write data to the cache has completed.
// Now we are back in the thread of the flash service.
/*************************************************************************/
void FF_Request_Context::Write_Request_Complete(void *p_context, Status status)
{

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;
	FF_Request_Context *p_parent_context = 
		(FF_Request_Context *)p_request_context->Get_Parent();

	TRACEF(TRACE_L5, ("\nFF_Request_Context::Write_Request_Complete status = %X, transfer_byte_count = %d", 
		status, p_request_context->m_transfer_byte_count));

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Check the status of the DMA operation.
 	if (status != OK)
 	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"FF_Request_Context::Write_DMA_Complete", 
			"DMA buffer to cache failed",
			status,
			0);
			
		// Terminate context with error
		p_request_context->Terminate_With_Error(status);
		return;
 	}
 	
	// Close the page in the cache.
	status = CM_Close_Page(p_flash->m_cache_handle, 
		p_request_context->m_page_handle);
 	if (status != OK)
 	{
		CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
			"FF_Request_Context::Write_DMA_Complete", 
			"CM_Close_Page failed",
			status,
			0);
			
		// Do not terminate;
 	}
 	
	p_request_context->m_p_page_frame = 0;
	
	// Increment byte count in parent.
	p_parent_context->m_transfer_byte_count += 
		p_request_context->m_transfer_byte_count;

	// Terminate this context and run the parent context.
	p_request_context->Terminate(p_context, OK);
	
} // FF_Request_Context::Write_Request_Complete

/*************************************************************************/
// FF_Controller::Write_Page using SGL
// Begin a write page operation for this context.
// This can be called from Write_Cache_Callback.
/*************************************************************************/
Status FF_Controller::Write_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	FF_SGL *p_sgl,
	U32 flags)
{
 	TRACEF(TRACE_L5,
		(EOL "Write_Page, unit = %d", 
		flash_address));
 	
	// Allocate FF_Controller_Context as child of caller's context.
	// So, when we terminate, caller's context will run.
	FF_Controller_Context *p_controller_context = 
		(FF_Controller_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Controller_Context));
	if (p_controller_context == 0)
	{
		// Return to calling context.  No child context was created.
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Save parameters in context.
	p_controller_context->m_p_controller = this;
	p_controller_context->m_flash_address = flash_address;
	p_controller_context->m_sgl = *p_sgl;
	p_controller_context->m_flags = flags;
	p_controller_context->Set_Callback(&FF_Controller::Authenticate_Write);

	FF_Check_Break_Address(p_controller_context->m_flash_address);

	// Schedule our context to run.
	p_callback_context->Make_Children_Ready();

	return OK;

} // FF_Controller::Write_Page

/*************************************************************************/
// FF_Controller::Write_Page using pointer to buffer.
// We could be writing from the cache, from a toc page, from
// a page map page.
/*************************************************************************/
Status FF_Controller::Write_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	void *p_buffer,
	U32 flags)
{
 	TRACEF(TRACE_L5,
		(EOL "Write_Page, unit = %d", 
		flash_address));
 	
	// Allocate FF_Controller_Context as child of caller's context.
	// So, when we terminate, caller's context will run.
	FF_Controller_Context *p_controller_context = 
		(FF_Controller_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Controller_Context));
	if (p_controller_context == 0)
	{
		// Return to calling context.  No child context was created.
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Save parameters in context.
	p_controller_context->m_p_controller = this;
	p_controller_context->m_flash_address = flash_address;
	p_controller_context->m_flags = flags;
	p_controller_context->Set_Callback(&FF_Controller::Authenticate_Write);

	// Initialize sgl from pointer.
	p_controller_context->m_sgl.Initialize(p_buffer, Flash_Address::Bytes_Per_Page());

	FF_Check_Break_Address(p_controller_context->m_flash_address);

	// Schedule our context to run.
	p_callback_context->Make_Children_Ready();

	return OK;

} // FF_Controller::Write_Page

/*************************************************************************/
// Authenticate_Write
// This is where we make the final checks before writing the page.
// Called from Write_Page, and called from Write_Complete if we need
// to retry the write.
/*************************************************************************/
void FF_Controller::Authenticate_Write(void *p_context, Status status)
{	
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL "Authenticate_Write, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Make sure we don't write over the bad block table.
	if ((p_controller_context->m_flags & FF_WRITING_BAD_BLOCK_TABLE) == 0)
	{
		if (p_page_map->Get_Page_State(p_controller_context->m_flash_address) == 
			FF_PAGE_STATE_BAD_BLOCK_TABLE)
		{
			p_controller_context->Terminate(p_controller_context, 
				FF_ERROR(WRITING_BAD_BLOCK_TABLE));
			return;
		}
	}

	p_controller_context->Set_Callback(&FF_Controller::Write_Wait_Controller);

	// Make sure we are writing to a good page.
	CT_ASSERT((p_page_map->Is_Page_Bad(p_controller_context->m_flash_address) == 0), 
		FF_Controller::Write_Page);

	// Make sure the page has been erased.
	CT_ASSERT((p_page_map->Is_Page_Erased(p_controller_context->m_flash_address)), 
		FF_Controller::Authenticate_Write);

	FF_Check_Break_Address(p_controller_context->m_flash_address);

	if (p_flash->m_flash_config.verify_page_erased_before_write &&
		((p_controller_context->m_flags & FF_NO_VERIFY) == 0))
	{

		// Verify that this page is erased.
		// Write_Wait_Controller will be called when page has been verified.
		Status status = p_controller->Read_Page(
			p_controller_context, 
			p_controller_context->m_flash_address, 
			p_flash->m_p_erased_page,
			FF_VERIFY_READ); 

		if (status != OK)
			p_controller_context->Terminate(p_context, status);
		return;	
	}

	Write_Wait_Controller(p_controller_context, OK);

} // Authenticate_Write

/*************************************************************************/
// Write_Wait_Controller
// This context will keep getting scheduled to run until the controller
// is available.
/*************************************************************************/
void FF_Controller::Write_Wait_Controller(void *p_context, Status status)
{	
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL "Write_Wait_Controller, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Check verify status
	if (status == FF_ERROR_CODE(VERIFY))
	{
		if (status == FF_ERROR_CODE(ECC))
		{
			// There won't be any ECC when the device is new.
			status = OK;
			p_controller_context->Set_Status(OK);
		}
		else
		{
			// We got a verify error because the page is not erased.
			// This is an internal error.
			// The page should be erased!
			status = FF_ERROR(PAGE_NOT_ERASED);
			CT_ASSERT(p_flash->m_flash_config.verify_page_erased_before_write, Write_Wait_Controller);
			
			p_page_map->Set_Bad_Page(p_controller_context->m_flash_address);
			p_controller->Retry_Write(p_controller_context, status);
			return;
		}
	}
	
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Write_Wait_Controller", 
			"Verify failed",
			status,
			0);
		Callback_Context::Terminate(p_controller_context, status);
		return;
	}

	// Check to see if the buss is busy.
	if (p_controller->Is_Buss_Busy(p_controller_context))

		// The buss is busy. Our context has been placed on a wait list.
		// When the buss is not busy, this context will be scheduled to run.
		return;

	// Check to see if the logical unit is busy.
	if (p_controller->Is_Unit_Busy(p_controller_context))
	{
		// The unit is busy. Our context has been placed on a wait list.
		// When the unit is not busy, this context will be scheduled to run.
		TRACE_STRING(TRACE_L5, EOL "Unit is busy");
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		return;
	}

	// Initialize return value.
	p_controller_context->Set_Return_Value(0);

	// Set callback to Write_Transfer_Complete when write has completed
	p_controller_context->Set_Callback(&FF_Controller::Write_Complete);

	// Get flash map from flash address.
	Flash_Page_Map flash_page_map;
	status = p_flash->m_block_address.Get_Flash_Page_Map(
		p_controller_context->m_flash_address, 
		flash_page_map, p_controller_context->m_flags);

	// Make sure we don't write over the bad block table.
	if (status != OK)
	{
		// We should not be writing over the bad block table.
		// Mark the buss not busy and check to see if another
		// context is waiting for the buss.
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		
		// Mark this unit not busy and check to see if another
		// context is waiting for this unit.
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);

		p_controller_context->Terminate(p_controller_context, status);
		return;
	}

	// The page is no longer erased.
	p_page_map->Set_Page_Erased(p_controller_context->m_flash_address, 0);

	// Start the unit write operation.
	status = p_flash->m_p_device->Write_Page(
		p_controller_context,
		p_controller_context->m_flash_address,
		flash_page_map,
		&p_controller_context->m_sgl);

	// Was the operation started?
	if (status != OK)
	{
 		TRACEF( TRACE_L5, 
			(EOL "FF_Device_Write_Page failed to start, status = %d", status));
 	
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);
		Callback_Context::Terminate(p_controller_context, status);
	}

} // FF_Controller::Write_Wait_Controller

/*************************************************************************/
// FF_Controller::Write_Complete
// Both the data transfer portion and the device portion
// of the write have completed.
/*************************************************************************/
void FF_Controller::Write_Complete(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL "Write_Transfer_Complete, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Save the pointer to this context for the interrupt handler.
	U32 unit = p_controller_context->m_flash_address.Unit_Index();
	CT_ASSERT((p_controller->m_p_controller_context[unit] 
		== p_controller_context), Write_Transfer_Complete);

	// Get the device-dependent controller status that was
	// passed to the context by the interrupt handler.
	UI64 controller_status = p_controller_context->Get_Return_Value();

	// Did we get a timeout?
	if (status == FF_ERROR_CODE(TIMEOUT))
		p_flash->m_p_device->Abort_Command(p_controller_context->m_flash_address);
	
 	// Get the status of the write operation.
	Flash_Device_Status device_status;
	Status command_status = p_flash->m_p_device->Get_Device_Status(
		p_controller_context->m_flash_address, controller_status, &device_status);

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
		status = FF_ERROR(WRITE_TIMEOUT);

		if (device_status.device_is_ready)
		{
			// The unit is ready, so we failed to get the interrupt.
			
#ifdef TRACE_ERROR
		Tracef(EOL "Write timeout error detected, array %X, column %X, block %X, page %X",
			p_controller_context->m_flash_address.Array(), 
			p_controller_context->m_flash_address.Column(),
			p_controller_context->m_flash_address.Block(), 
			p_controller_context->m_flash_address.Page_Index() );
		Tracef(EOL " unit status = %LX, %LX, %LX %LX", 
			device_status.bank_status[0], device_status.bank_status[1], 
			device_status.bank_status[2], device_status.bank_status[3]);
#endif
			// Set status to OK and continue.
			status = OK;
			p_controller_context->Set_Status(OK);
			
		} // unit is actually ready
	} // timeout

	if (status != OK)
	{
		p_controller_context->Terminate(p_context, status);
		return;
	}

	// Check the command status to see if this sector had an error.
	IF_FF_ERROR(command_status != OK, WRITE_ERROR)
	{
		// The operation status is not OK.
		// The write did not work.  We have a bad sector.

		// In case error injection simulated an error, 
		// change status from OK to VERIFY_WRITE
		FF_INJECT_STATUS(command_status, FF_ERROR_CODE(VERIFY_WRITE));

		// Mark this page bad.
		p_page_map->Set_Bad_Page(p_controller_context->m_flash_address);
		
#ifdef TRACE_ERROR
			Tracef(EOL "Write error detected, array %X, column %X, block %X, page %X",
				p_controller_context->m_flash_address.Array(), 
				p_controller_context->m_flash_address.Column(),
				p_controller_context->m_flash_address.Block(), 
				p_controller_context->m_flash_address.Page_Index() );

			Tracef(EOL " unit status = %LX, %LX, %LX %LX", 
				device_status.bank_status[0], device_status.bank_status[1], 
				device_status.bank_status[2], device_status.bank_status[3]);
#endif

		p_flash->m_stats.Inc_Num_Write_Errors(p_controller_context->m_flash_address.Unit_Index());
		p_controller->Retry_Write(p_controller_context, command_status);
	}
	else
	{
		// The write operation was successful.
		// Do we need to verify?
		if (p_flash->m_flash_config.verify_write &&
			((p_controller_context->m_flags & FF_NO_VERIFY) == 0))
		{
			p_controller_context->Set_Callback(&FF_Controller::Verify_Write);

			// Verify that this page was correctly written.
			// Verify_Write will be called when page has been verified.
			Status status = p_controller->Read_Page(
				p_controller_context, 
				p_controller_context->m_flash_address, 
				&p_controller_context->m_sgl,
				FF_VERIFY_READ); 

			if (status != OK)
				p_controller_context->Terminate(p_context, status);
			return;
		}

		// We don't need to verify.  Terminate the context.
		p_controller_context->Terminate(p_context, status);
		return;	
	}

} // FF_Controller::Write_Complete

/*************************************************************************/
// FF_Request_Context::Retry_Write
/*************************************************************************/
void FF_Controller::Retry_Write(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
 	TRACEF( TRACE_L5, 
		(EOL "Retry_Write, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Should we retry the write?
	// No if this a write into a page that must have a fixed address, such as a
	// basic assurance test page or a toc page.
	if ((p_controller_context->m_flags & FF_RETRY_WRITE_IF_ERROR) == 0)
	{
		// We don't get replacement pages for bat pages written in error.
		// Terminate the context
		p_controller_context->Terminate(p_controller_context, status);
		return;	
	}

	// Get a replacement page.
	Flash_Address real_SSD_replacement_address;
	status = p_page_map->
		Get_Replacement_Page(p_controller_context->m_flash_address, &real_SSD_replacement_address);
	if (status != OK)
	{
		if (status == FF_ERROR_CODE(REPLACEMENT_PAGES_ERASING))
		{
			// All pages in the replacement pool are currently being erased.
			p_controller_context->Set_Callback(&Retry_Write);
			p_page_map->Wait_For_Erased_Page(p_controller_context);
			return;
		}

		// Terminate the context with error.
		p_controller_context->Terminate(p_controller_context, status);
		return;
	}

	// Reset the error code.
	p_controller_context->Set_Status(OK);

	// Try writing the page to the new replacement address.
	p_controller_context->m_flash_address = real_SSD_replacement_address;
	Authenticate_Write(p_controller_context, OK);

} // Retry_Write

/*************************************************************************/
// FF_Request_Context::Verify_Write
// The verify has completed.
/*************************************************************************/
void FF_Controller::Verify_Write(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_controller->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
 	TRACEF( TRACE_L5, 
		(EOL "Verify_Write, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));

	// Check the verify status.
	if (status == OK)
	{
		// Verify was successful.  Terminate the context.
		p_controller_context->Terminate(p_context, OK);
		return;	
	}

	// We had a verify error!
	p_flash->m_stats.Inc_Num_Verify_Errors(
		p_controller_context->m_flash_address.Unit_Index());

	p_page_map->Set_Bad_Page(p_controller_context->m_flash_address);
	p_controller->Retry_Write(p_controller_context, status);

} // Verify_Write


