/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfRead.cpp
// 
// Description:
// This file implements the Read Page operation. 
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "Cache.h"
#include "CtEvent.h"
#include "FfCommon.h"
#include "Dma.h"
#include "FfController.h"
#include "FlashDevice.h"
#include "FfPageMap.h"
#include "FfRequest.h"
#include "FfSgl.h"
#include "FfStats.h"

#if 1
// Turn on TRACE_ERROR
#define TRACE_ERROR Tracef
#else
// Turn off TRACE_ERROR -- if 1 then do nothing, else call Tracef
#define TRACE_ERROR 1? 0 : Tracef  
#endif

/*************************************************************************/
// Read transfer_byte_count from logical_byte_address into specified buffer.
// Call callback routine with pointer to context when read has completed.
/*************************************************************************/
Status FF_Interface::Read(
	FF_SGL *p_sgl, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback)
{
	m_stats.Inc_Num_Read_Bytes_Total(transfer_byte_count);
	
	// Create a child context to read each page.  Each child context will
	// call Read_Cache.
	return FF_Request_Context::Create_Child_Read_Write_Contexts(
		this,
		p_sgl,
		transfer_byte_count,
		logical_byte_address,
		p_context,
		p_completion_callback,
		&FF_Request_Context::Read_Cache);

} // FF_Read

/*************************************************************************/
// Create_Child_Read_Write_Contexts 
// Create as many child contexts as necessary to complete the read or
// write operation.  Each child context will transfer one logical page
// or a portion of a logical page.
// Each child context will transfer data to or from a logical byte address.
// Each logical byte address is translated into a virtual flash address 
// (virtual SSD page number),
/*************************************************************************/
Status FF_Request_Context::Create_Child_Read_Write_Contexts(
	FF_Interface *p_flash,
	FF_SGL *p_sgl, 
	U32 transfer_byte_count, 
	I64 logical_byte_address,
	void *p_context,
	FF_CALLBACK *p_completion_callback,
	Callback read_write_callback)
{
	// Allocate FF_Request_Context.  This is the parent context that will
	// contain all of the request parameters.
	FF_Request_Context *p_request_context = 
		(FF_Request_Context *)Callback_Context::Allocate(sizeof(FF_Request_Context));
	if (p_request_context == 0)
	{
		p_flash->m_stats.Inc_Num_No_Contexts();

		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}

	// Initialize request context.
	p_request_context->m_p_flash = p_flash;
	p_request_context->m_transfer_byte_count = 0;
	p_request_context->m_logical_byte_address = 0;
	p_request_context->m_p_completion_callback = p_completion_callback;
	p_request_context->m_p_context = p_context;

	// Set callback for parent context.  
	p_request_context->Set_Callback(&FF_Request_Context::Read_Write_Transfer_Complete);

	// Create a child context for each cache page.
	char *next_data_buffer = (char *)p_sgl->Address(0);
	U32 remaining_transfer_byte_count = transfer_byte_count;
	I64 next_logical_byte_address = logical_byte_address;
	U32 next_page_offset = Flash_Address::Offset_From_Logical_Byte_Address(next_logical_byte_address);
	U32 next_virtual_address = Flash_Address::Page_Index_From_Logical_Byte_Address(next_logical_byte_address);
	U32 element_offset = 0;
	while (remaining_transfer_byte_count != 0)
	{
		FF_Request_Context *p_child = (FF_Request_Context *)
			p_request_context->Allocate_Child(sizeof(FF_Request_Context));
		if (p_child == 0)
		{
			p_flash->m_stats.Inc_Num_No_Contexts();
			Callback_Context::Terminate(p_request_context, FF_ERROR(NO_CONTEXT));
			return FF_ERROR(NO_CONTEXT);
		}

		// Copy SGL from caller's space into context.
		p_child->m_sgl = *p_sgl;

		// Initialize offset into this SGL.
		p_child->m_sgl.Offset(element_offset);

		// Set parameters for transfer address
		p_child->m_p_flash = p_flash;
		p_child->m_virtual_address = next_virtual_address;
		p_child->m_logical_byte_address = next_logical_byte_address;
		p_child->m_p_data_buffer = next_data_buffer;

		// Calculate number of bytes for this transfer.
		U32 next_transfer_byte_count = Flash_Address::Bytes_Per_Page() - next_page_offset;
		if (next_transfer_byte_count > remaining_transfer_byte_count)
			next_transfer_byte_count = remaining_transfer_byte_count;
		p_child->m_transfer_byte_count = next_transfer_byte_count;

		// Calculate parameters for next child.
		element_offset += next_transfer_byte_count;
		remaining_transfer_byte_count -= next_transfer_byte_count;
		next_logical_byte_address += next_transfer_byte_count;
		next_data_buffer += next_transfer_byte_count;
		next_page_offset = 0;
		next_virtual_address = Flash_Address::Page_Index_From_Logical_Byte_Address(next_logical_byte_address);

		// Set start address for child context.
		p_child->Set_Callback(read_write_callback);
	}

	// Schedule the child contexts to run.
	p_request_context->Make_Children_Ready();
	return OK;

} // FF_Request_Context::Create_Child_Read_Write_Contexts

/*************************************************************************/
// FF_Request_Context::Read_Cache
// Read one page from the cache.
// At this level we are reading a virtual page.
/*************************************************************************/
void FF_Request_Context::Read_Cache(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_Cache);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Increment the total number of page reads.
	p_flash->m_stats.Inc_Num_Page_Reads();

	// See if page is in the cache.
	status = CM_Open_Page(
		p_flash->m_cache_handle,
		p_request_context->m_virtual_address,
//		CM_OPEN_MODE_READ | CM_CALLBACK_IF_LOCKED | CM_CALLBACK_IF_NO_FRAME,
		CM_OPEN_MODE_READ | CM_CALLBACK_IF_LOCKED | CM_CALLBACK_IF_NO_FRAME | CM_SECONDARY,
		p_request_context,
		&p_request_context->m_p_page_frame,
		&p_request_context->m_page_handle);
				
	switch (status)
	{
		case OK:
		
			// The page is in the cache.
			p_flash->m_stats.Inc_Num_Page_Reads_Cache_Hit();

			// Transfer data from the cache to the requestor.
			Read_DMA_Cache_To_Buffer(p_context, status);
			break;
			
		case CM_ERROR_CACHE_MISS:
			{
		
				// The page is not in the cache. 
				p_flash->m_stats.Inc_Num_Page_Reads_Cache_Miss();

				// Our context has a pointer to an empty page frame.
 				// Get real address in flash for this page. 
 				Flash_Address flash_address = p_flash->m_page_map.
					Get_Real_Flash_Address(
					p_request_context->m_virtual_address);

				// Set callback for when the read completes.
				p_request_context->Set_Callback(&FF_Request_Context::Read_Cache_Complete);

				// Start the read operation to read the real page
				// into the cache.
				status = p_flash->m_controller.Read_Page(
					p_request_context,
					flash_address,
					p_request_context->m_p_page_frame,
					FF_REPORT_ECC_ERROR);

				if (status != OK)
					p_request_context->Terminate_With_Error(status);
				break;
			}
			
		case CM_ERROR_PAGE_LOCKED:
		
			// This method will be called again when the page
			// is unlocked.
			break;
			
		case CM_ERROR_NO_PAGE_FRAMES:
		
			// This method will be called again when a page frame is available.
			break;
		
		default:
		
			// Unexpected error
			CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
				"FF_Request_Context::Read_Cache", 
				"Unexpected cache error",
				status,
				0);
				
			p_request_context->Terminate_With_Error(status);
			break;
	}
	
} // FF_Request_Context::Read_Cache

/*************************************************************************/
// FF_Controller::Read_Page using SGL.
// Begin a read page operation for this context.
// We could be reading from the cache, from a toc page, from
// a page map page.
// If verify is true, we don't read the page -- rather, we compare the
// data in the buffer to the data in the flash.
/*************************************************************************/
Status FF_Controller::Read_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	FF_SGL *p_sgl,
	U32 flags)
{
 	TRACEF( TRACE_L5, 
		(EOL "Read_Page, unit = %d", 
		flash_address.Unit_Index()));
 	
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
	p_controller_context->m_p_controller = this;
	p_controller_context->m_flash_address = flash_address;
	p_controller_context->m_sgl = *p_sgl;
	p_controller_context->m_flags = flags;
	p_controller_context->m_retry_count = 0;

	// Schedule our context to run.
	p_controller_context->Set_Callback(&FF_Controller::Read_Wait_Controller);
	p_callback_context->Make_Children_Ready();

	return OK;

} // FF_Controller::Read_Page

/*************************************************************************/
// FF_Controller::Read_Page into a buffer.
// Begin a read page operation for this context.
// We could be reading from the cache, from a toc page, from
// a page map page.
/*************************************************************************/
Status FF_Controller::Read_Page(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	void *p_buffer,
	U32 flags)
{
 	TRACEF( TRACE_L5, 
		(EOL "Read_Page, unit = %d", 
		flash_address.Unit_Index()));
 	
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
	p_controller_context->m_p_controller = this;
	p_controller_context->m_flash_address = flash_address;
	p_controller_context->m_flags = flags;
	p_controller_context->m_retry_count = 0;

	// Initialize sgl from pointer.
	p_controller_context->m_sgl.Initialize(p_buffer, Flash_Address::Bytes_Per_Page());

	// Schedule our context to run.
	p_controller_context->Set_Callback(&FF_Controller::Read_Wait_Controller);
	p_callback_context->Make_Children_Ready();

	return OK;

} // FF_Controller::Read_Page

/*************************************************************************/
// Read_Wait_Controller.
// Start the unit read if the controller is not busy.
// This context will keep getting scheduled to run until the controller
// is available.
/*************************************************************************/
void FF_Controller::Read_Wait_Controller(void *p_context, Status status)
{	
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;

	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash interface object
	FF_Interface *p_flash = p_controller->m_p_flash;

 	TRACEF( TRACE_L5, 
		(EOL "Read_Wait_Controller, unit = %d", 
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
		TRACE_STRING(TRACE_L5, EOL "Unit is busy");
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		return;
	}

	FF_Check_Break_Address(p_controller_context->m_flash_address);

	// Initialize return value.
	p_controller_context->Set_Return_Value(0);

	// Set callback to Read_Complete when data transfer is complete.
	p_controller_context->Set_Callback(&FF_Controller::Read_Complete);

	// Get flash page map from flash address.
	Flash_Page_Map flash_page_map;
	p_flash->m_block_address.Get_Flash_Page_Map(p_controller_context->m_flash_address, 
		flash_page_map);

	if (p_controller_context->m_flags & (FF_VERIFY_READ | FF_VERIFY_ERASE))

		// Start the unit read into verify buffer.
		status = p_flash->m_p_device->Verify_Page(
			p_controller_context,
			p_controller_context->m_flash_address,
			flash_page_map,
			&p_controller_context->m_sgl,
			p_controller_context->m_flags);
	else
	{

 		// Any page we are reading should be marked not erased.
		// This assert is not working for Bob Butler.
		// In the future, return an erc instead.
		// CT_ASSERT((!p_flash->m_page_map.Is_Page_Erased(p_controller_context->m_flash_address)), Read_Wait_Controller);

		// Start the unit read operation.
		status = p_flash->m_p_device->Read_Page(
			p_controller_context,
			p_controller_context->m_flash_address,
			flash_page_map,
			&p_controller_context->m_sgl);
	}

	// Was the operation started?
	if (status != OK)
	{
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);
		Callback_Context::Terminate(p_controller_context, status);
	}

} // FF_Controller::Read_Wait_Controller

/*************************************************************************/
// FF_Controller::Read_Complete
// The read operation has completed.
// Terminate this context and return to the parent context.
/*************************************************************************/
void FF_Controller::Read_Complete(void *p_context, Status status)
{
	FF_Controller_Context *p_controller_context = (FF_Controller_Context *)p_context;
	
	// Get pointer to controller object for this context.
	FF_Controller *p_controller = p_controller_context->m_p_controller;
	
	// Get pointer to flash interface object
	FF_Interface *p_flash = p_controller->m_p_flash;

 	TRACEF( TRACE_L5, 
		(EOL "Read_Complete, unit = %d", 
		p_controller_context->m_flash_address.Unit_Index()));
 	
	// Get the device-dependent controller status that was
	// passed to the context by the interrupt handler.
	UI64 controller_status = p_controller_context->Get_Return_Value();

	// Did we get a timeout?
	if (status == FF_ERROR_CODE(TIMEOUT))
		p_flash->m_p_device->Abort_Command(p_controller_context->m_flash_address);
	
 	// Get the status from the last command.
 	// We know the buss is not busy, so we can get status.
	Flash_Device_Status device_status;
	Status command_status = p_flash->m_p_device->Get_Device_Status(
		p_controller_context->m_flash_address, controller_status, &device_status);

	// Did we get a timeout?
	if (status == FF_ERROR_CODE(TIMEOUT))
	{
		// Timeout occurred.
		status = FF_ERROR(READ_TIMEOUT);
		
#ifdef TRACE_ERROR
		Tracef(EOL "Read timeout error detected, array %X, column %X, block %X, page %X",
			p_controller_context->m_flash_address.Array(), 
			p_controller_context->m_flash_address.Column(),
			p_controller_context->m_flash_address.Block(), 
			p_controller_context->m_flash_address.Page_Index() );
		Tracef(EOL " unit status = %LX, %LX, %LX %LX", 
			device_status.bank_status[0], device_status.bank_status[1], 
			device_status.bank_status[2], device_status.bank_status[3]);
			
		// Get controller status.  We don't get it in the case of a timeout
		// because it comes from the LISR.
		// 0xCAFECAFE is a kludge way of getting the controller status from the device
		// in a device-dependent way so we can display it for debugging.
		// The controller status gets stored in device_status.bank_status[0].
		// And the DMA count register gets stored in device_status.bank_status[1].
		p_flash->m_p_device->Get_Device_Status(
			p_controller_context->m_flash_address, 
			0xCAFECAFE, &device_status);
			
			
		Tracef(EOL " FPGA_status_register = %LX", device_status.bank_status[0]);
		Tracef(EOL " DMA CH0 count register = %X", (U32)device_status.bank_status[1]);

#endif
		// Wait until after Get_Device_Status to mark buss not busy.
		// Mark the buss not busy and check to see if another
		// context is waiting for the buss.
		p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);
	
		// Mark this unit not busy and check to see if another
		// context is waiting for this unit.
		p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);

		// Have we retried?
		if (p_controller_context->m_retry_count == 0)
		{
			// We have not retried.
			p_controller_context->m_retry_count++;
			p_controller_context->Set_Status(OK);
			Read_Wait_Controller(p_controller_context, OK);
		} 
		else
		{
			// We have retried, so terminate with error.
			p_controller_context->Terminate(p_context, status);
		}
		
		return;

	} // timeout

	// Mark the buss not busy and check to see if another
	// context is waiting for the buss.
	p_controller->Mark_Buss_Not_Busy(p_controller_context->m_flash_address);

	// Mark this unit not busy and check to see if another
	// context is waiting for this unit.
	p_controller->Mark_Unit_Not_Busy(p_controller_context->m_flash_address);

	if (status != OK)
	{
		// Terminate context and schedule parent.
		p_controller_context->Terminate(p_context, status);
		return;
	}
	
	// Was an ECC error corrected?
	if (command_status == FF_ERROR_CODE(ECC_CORRECTED))
	{
		// We got an ECC error that was corrected.
		// Does the caller want to know about it?
		if ((p_controller_context->m_flags & FF_REPORT_ECC_ERROR) == 0)
			command_status = OK;
		p_controller_context->Terminate(p_context, command_status);
		return;
	}

#if 0
	// Simulate error for debugging
	IF_FF_ERROR(command_status != OK, READ_ECC_ERROR)
		p_controller_context->Terminate(p_context, FF_ERROR_CODE(ECC));
	else
		p_controller_context->Terminate(p_context, command_status);
#else
		p_controller_context->Terminate(p_context, command_status);
#endif
	
} // FF_Controller::Read_Complete

/*************************************************************************/
// FF_Request_Context::Read_Cache_Complete
// The read operation from the device
// into the cache has completed for the request.
/*************************************************************************/
void FF_Request_Context::Read_Cache_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(Read_Cache_Complete);
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;
 	
	if (status != OK)
	{
		if (status == FF_ERROR_CODE(ECC_CORRECTED))
		{
			// Reset the status for this context.
			p_request_context->Set_Status(OK);

			// Set this page dirty.  
			// It must be rewritten with corrected data.
			status = CM_Set_Page_Dirty(p_request_context->m_p_flash->m_cache_handle, 
				p_request_context->m_page_handle);
			if (status != OK)
			{
				p_request_context->Terminate_With_Error(status);
				return;
			}

			Remap_Corrected_Page(p_context, OK);
			return;
		}
		else
		{
			// Terminate context and schedule parent.
			p_request_context->Terminate_With_Error(status);
			return;
		}
	}

	// Transfer the data from the cache to the user's buffer.
	FF_Request_Context::Read_DMA_Cache_To_Buffer(p_context, status);
	
} // FF_Request_Context::Read_Cache_Complete

/*************************************************************************/
// Remap_Corrected_Page
// When an ECC error has been corrected, we need to remap the page
// so that the data will be rewritten.
/*************************************************************************/
void FF_Request_Context::Remap_Corrected_Page(void *p_context, Status status)
{
 	TRACE_ENTRY(Remap_Corrected_Page);
 	
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	// Get an erased page for this data.
	// This remaps the page to a new page that has been erased.
	status = p_flash->m_page_map.
		Get_Erased_Page(p_request_context->m_virtual_address);
	if (status != OK)
	{
		// No erased pages are available.
		// Set up to be scheduled again when an erased page is available.
		p_request_context->Set_Callback(&FF_Request_Context::Remap_Corrected_Page);
		p_flash->m_page_map.Wait_For_Erased_Page(p_request_context);

		// Context will be called when an erased page is available
		return;
	}

	// Transfer the data from the cache to the user's buffer.
	FF_Request_Context::Read_DMA_Cache_To_Buffer(p_context, status);
	
} // Remap_Corrected_Page

/*************************************************************************/
// Read_DMA_Cache_To_Buffer
// Start a DMA operation to transfer data from the cache to the 
// buffer of the requestor.
/*************************************************************************/
void FF_Request_Context::Read_DMA_Cache_To_Buffer(void *p_context, Status status)
{
 	TRACE_ENTRY(Read_DMA_Cache_To_Buffer);
 	
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Get pointer to flash interface object
	FF_Interface *p_flash = p_request_context->m_p_flash;

	TyDma *p_ty_dma;

 	// Is this a full page operation?
	if (p_request_context->m_transfer_byte_count == Flash_Address::Bytes_Per_Page())
 	{
		// Create TyDma from SGL to transfer page from cache to client's buffer.
		status = p_request_context->m_sgl.Build_TyDma(
			p_request_context->m_p_page_frame, // transfer address is page frame in cache,
			Flash_Address::Bytes_Per_Page(), // transfer one page
			FF_SGL::Transfer_Address_Is_Source,
	 		&FF_Request_Context::Read_DMA_Complete,
	 		p_context,
			TyDma::flagsDefault,
			&p_ty_dma);
 	}
 	else
 	{
		// This read is for a partial page.
		p_flash->m_stats.Inc_Num_Page_Reads_Partial();

 		// Calculate offset in page frame for partial page.
 		char *p_page_frame = (char *)p_request_context->m_p_page_frame 
			+ Flash_Address::Offset_From_Logical_Byte_Address((U32)p_request_context->m_logical_byte_address);
 			
		// Create TyDma from SGL to transfer partial page from cache to client's buffer.
		status = p_request_context->m_sgl.Build_TyDma(
			p_page_frame, // transfer address is calculated offset from page frame in cache.
			p_request_context->m_transfer_byte_count,
			FF_SGL::Transfer_Address_Is_Source,
	 		&FF_Request_Context::Read_DMA_Complete,
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

	// Scheduler will call FF_Request_Context::Read_DMA_Complete 
 	// when transfer is complete.
 	
} // Read_DMA_Cache_To_Buffer

/*************************************************************************/
// FF_Request_Context::Read_DMA_Complete
// Called by the DMA interrupt handler when the DMA operation to transfer the
// data from the cache has completed.
// Note that we are not running in the thread of the flash service, so
// we don't want to do any work here.  It would be especially bad if we
// called CM_Close because this attempts to use a semaphore from
// an interrupt handler.
/*************************************************************************/
void FF_Request_Context::Read_DMA_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_DMA_Complete);
 		
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;
	
	p_request_context->Set_Status(status);
	p_request_context->Set_Callback(&Read_Request_Complete);
	p_request_context->Make_Ready();
	
} // FF_Request_Context::Read_DMA_Complete

/*************************************************************************/
// FF_Request_Context::Read_Request_Complete
// The DMA of data from cache to buffer has completed.
/*************************************************************************/
void FF_Request_Context::Read_Request_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_DMA_Complete);
 	
	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;
	FF_Request_Context *p_parent_context = 
		(FF_Request_Context *)p_request_context->Get_Parent();

 	// Check the status of the DMA operation.
 	if (status != OK)
 	{
 #if 0 // CTS_DMA_TIMEOUT has been removed
 		if (status == CTS_DMA_TIMEOUT)
 		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Request_Context::Read_DMA_Complete", 
				"DMA cache to buffer failed: DMA timeout",
				status,
				0);
 		}
 		else
 #endif
 		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"FF_Request_Context::Read_DMA_Complete", 
				"DMA cache to buffer failed",
				status,
				0);
		}
		p_request_context->Terminate_With_Error(status);
		return;
 	}
 	
	// Close page.
	CM_Close_Page(p_request_context->m_p_flash->m_cache_handle, 
		p_request_context->m_page_handle);
	p_request_context->m_p_page_frame = 0;
		 	
	// Increment byte count in parent.
	p_parent_context->m_transfer_byte_count += 
		p_request_context->m_transfer_byte_count;

	p_request_context->Terminate(p_context, status);
	
} // FF_Request_Context::Read_Request_Complete

/*************************************************************************/
// FF_Request_Context::Read_Write_Transfer_Complete
// Come here when all of the child contexts have completed.
// Return to caller.
/*************************************************************************/
void FF_Request_Context::Read_Write_Transfer_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Read_Write_Transfer_Complete);

	FF_Request_Context *p_request_context = (FF_Request_Context *)p_context;

	// Call user's callback
	p_request_context->m_p_completion_callback(
		p_request_context->m_transfer_byte_count, 
		p_request_context->m_logical_byte_address,
		p_request_context->Get_Status(),
		p_request_context->m_p_context);

	// Terminate this context
	Callback_Context::Terminate(p_request_context, status);

} // Read_Write_Transfer_Complete

/*************************************************************************/
// FF_Request_Context::Terminate_With_Error
// Come here if an error is encountered in the child context.
/*************************************************************************/
void FF_Request_Context::Terminate_With_Error(Status status)
{
 	TRACE_ENTRY(FF_Request_Context::Terminate_With_Error);

	FF_Request_Context *p_parent_context = (FF_Request_Context *)Get_Parent();

	// Save error offset in parent.
	p_parent_context->m_logical_byte_address = m_logical_byte_address;

	// Terminate child context.
	Callback_Context::Terminate(this, status);

} // Error
