/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBadBlockOpen.cpp
// 
// Description:
// This file implements the methods to open an existing bad block table.
// for the flash storage system 
// 
// 4/19/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfBlockAddress.h"
#include "FfCommon.h"
#include "FfController.h"
#include "FfInterface.h"
#include "FfStats.h"
#include <String.h>

/*************************************************************************/
// FF_Block_Address::Open
// Open and initialize object, called by FF_Interface::Initialize.
// A flash handle cannot be created 
// unless the bad block table has been created.
/*************************************************************************/
Status	FF_Block_Address::Open(Callback_Context *p_callback_context)
{ 	
#if 0
	// Initialize the flash addresses first.  Do this here so that they will
	// be initialized even if the bad block table does not exist.  The flash
	// addresses will be initialized again if the bad block table is found.
	Initialize_System_Flash_Addresses();
#endif

	// Create a FF_Bad_Block_Context
	FF_Bad_Block_Context *p_bad_block_context = 
		(FF_Bad_Block_Context *)p_callback_context->Allocate_Child(sizeof(FF_Bad_Block_Context));
	if (p_bad_block_context == 0)
		return FF_ERROR(NO_CONTEXT);

	// Set to read header first.
	// Use the flash object's page buffer to read and check header.
	p_bad_block_context->m_p_page_frame = m_p_flash->m_p_page_buffer;
	
	// Zero page buffer.
	ZERO(m_p_flash->m_p_page_buffer, Flash_Address::Bytes_Per_Page());

	// Set to read in the bad block header for array 0 copy 1 first
	p_bad_block_context->m_p_block_address = this;
	p_bad_block_context->m_page_number = 0;
	p_bad_block_context->m_array_number = 0;
	p_bad_block_context->m_retry_count = 0;
	p_bad_block_context->m_copy_number = 1;
	p_bad_block_context->m_flash_address = Address_Bad_Block_Table_Array_0_Copy_1(FF_Mode_Open);

	// Set the number of pages we need to read.
	p_bad_block_context->m_num_pages = m_bad_block_bitmap_num_pages_per_array;

	// When read is complete, call Read_Next_Bad_Block_Page.
	p_bad_block_context->Set_Callback(&Read_Next_Bad_Block_Page);
	
	// Start the read of the header page for array 0.
	Status status = m_p_flash->m_controller.Read_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame); 

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return status;
	}

	return OK;

} // FF_Block_Address::Open
	
/*************************************************************************/
// FF_Block_Address::Read_Next_Bad_Block_Page
/*************************************************************************/
void FF_Block_Address::Read_Next_Bad_Block_Page(void *p_context, Status status)
{
	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;

	// Did we just read the header page?
	if (p_bad_block_context->m_page_number == 0)
	{
		// Check the cookie to see if we have a valid bad block page.
		FF_Bad_Block_Header *p_bad_block_header = 
			(FF_Bad_Block_Header *)p_block_address->m_p_flash->m_p_page_buffer;
		if (status == OK)
			status = p_bad_block_header->Open();
		if (status != OK)
		{
			// The bad block table is not in the page where we would expect to find it.
			// Have we tried the maximum number of times?
			if (++p_bad_block_context->m_retry_count < MAX_BAD_BLOCK_TABLE_RETRY)

				// Increment the block number to try the next block.
				p_bad_block_context->m_flash_address.Increment_Block();
			else
			{
				// Have we tried both copies?
				if (++p_bad_block_context->m_copy_count > 2)
				{
					p_bad_block_context->Terminate(p_bad_block_context, 
						FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST));
					return;
				}

				// Set up to try the second copy.
				// Is this the bad block bitmap for array 1?
				if (p_bad_block_context->m_array_number)
					p_bad_block_context->m_flash_address = 
						p_block_address->Address_Bad_Block_Table_Array_1_Copy_2(FF_Mode_Open);
				else
					p_bad_block_context->m_flash_address = 
						p_block_address->Address_Bad_Block_Table_Array_0_Copy_2(FF_Mode_Open);
				p_bad_block_context->m_retry_count = 0;
			}

			// Initialize the status since we are a parent context.
			p_bad_block_context->Set_Status(OK);

			// Start the read of the first page of the next block.
			status = p_flash->m_controller.Read_Page(
				p_bad_block_context, 
				p_bad_block_context->m_flash_address, 
				p_bad_block_context->m_p_page_frame);

			if (status != OK)
			{
				p_bad_block_context->Terminate(p_bad_block_context, status);
				return;
			}
			return;
		}

		// We successfully read the header page.

#ifdef MOVE_BAD_BLOCK_0

		// Was the bad block table at block 0?
		if (p_bad_block_context->m_flash_address.Block() == 0)
		{
			Tracef("\nBad Block Table found at block 0");
			p_block_address->m_if_move_bad_block_table = 1;
		}
#endif

		// The next page will be the first page of the bitmap.
		// Set the memory address of the bitmap to read in.
		// Is this the bad block bitmap for array 1?
		if (p_bad_block_context->m_array_number)
			p_bad_block_context->m_p_page_frame = p_block_address->m_p_bad_block_bitmap
				+ Num_Bad_Block_Bitmap_Entries_Per_Array();
		else
			p_bad_block_context->m_p_page_frame = p_block_address->m_p_bad_block_bitmap;

		// Increment the SSD page address to point to the next page following the header.
		p_bad_block_context->m_flash_address.Increment_Page();

		// Increment page number.
		p_bad_block_context->m_page_number++;
		
	} // just read header page
	else
	{
		// We just read a page of bad block data.
		// Check the status of the last read operation.
		if (status != OK)
		{
			Tracef("\nError reading bad block table, status = %X", status);
			p_bad_block_context->Terminate(p_bad_block_context, status);
			return;
		}
	
		// See if we need to read more pages.
		if (p_bad_block_context->m_num_pages == p_bad_block_context->m_page_number++)
		{
			// We have finished reading a bad block table.
			// Do we also need to read the bad block table for array 1?
			if ((Flash_Address::Num_Arrays() > 1) 
				&& (p_bad_block_context->m_array_number == 0))
			{
				// Set to read in the bad block header for array 1 next.
				p_bad_block_context->m_p_page_frame = 
					(FF_Bad_Block_Header *)p_flash->m_p_page_buffer;
				p_bad_block_context->m_page_number = 0;
				p_bad_block_context->m_array_number = 1;
				p_bad_block_context->m_retry_count = 0;
				p_bad_block_context->m_flash_address = 
					p_block_address->Address_Bad_Block_Table_Array_1_Copy_1(FF_Mode_Open);
			}
			else
			{
				// We have finished reading both bad block tables.
				// The table does exist.  Finish initializing.

#ifdef MOVE_BAD_BLOCK_0
				// Do we need to move the bad block table?
				if (p_block_address->m_if_move_bad_block_table)
				{
					Tracef("\nMoving Bad Block Table found at block 0");
					p_block_address->Erase_Bad_Block_Table(p_bad_block_context);
					return;
				}
#endif

				// Create block map tables from bad block table.
				// Mark replacement pages as bad pages in page table.
				status = p_block_address->Create_Block_Map_Tables();

				p_block_address->Initialize_System_Flash_Addresses();

				// Now we are initialized.
				p_block_address->m_initialized = 1;
				p_bad_block_context->Terminate(p_bad_block_context, status);
				return;
			}
		}
		else
		{
			// The last page was a bitmap page.
			// Point to the next page of the bitmap.
			p_bad_block_context->m_p_page_frame = (char *)p_bad_block_context->m_p_page_frame +
				Flash_Address::Bytes_Per_Page();

			// Increment the SSD page address to point to the next page.
			p_bad_block_context->m_flash_address.Increment_Page();
		}
	}
		
	// Start the read of the next page of the bad block table.
	status = p_flash->m_controller.Read_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

} // FF_Block_Address::Read_Next_Bad_Block_Page


// TEMPORARY Metrowerks won't step into an inline
/*************************************************************************/
// FF_Bad_Block_Header::Open 
/*************************************************************************/
/* inline */  Status FF_Bad_Block_Header::Open()
{
	if (m_cookie_A != FF_BAD_BLOCK_TABLE_COOKIE)
		return FF_ERROR_INVALID_BAD_BLOCK_TABLE_A;
	if (m_cookie_B != FF_BAD_BLOCK_TABLE_COOKIE)
		return FF_ERROR_INVALID_BAD_BLOCK_TABLE_B;
	if (m_version_A != FF_BAD_BLOCK_TABLE_VERSION)
		return FF_ERROR_INVALID_BAD_BLOCK_VERSION_A;
	if (m_version_B != FF_BAD_BLOCK_TABLE_VERSION)
		return FF_ERROR_INVALID_BAD_BLOCK_VERSION_B;
	return OK;
}




