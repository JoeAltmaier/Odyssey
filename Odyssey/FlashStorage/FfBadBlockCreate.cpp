/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBadBlockCreate.cpp
// 
// Description:
// This file implements the methods to create the bad block table
// for the flash file system 
// 
// 4/19/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfBlockAddress.h"
#include "FfController.h"
#include "FfInterface.h"
#include "FfStats.h"
#include <String.h>

// If TRACE_BAD_BLOCK_CREATE, display number of blocks left to scan
// each time we start a new block.
#define TRACE_BAD_BLOCK_CREATE

extern U32					  FF_bit_mask[32];

/*************************************************************************/
// Create bad block table.
// Only called if we know the bad block table does not already exist.
// If test_bad_blocks, we test every page by a sequence of erase, program, 
// read, erase program read again.  We only do this if the original
// bad block table has been destroyed for some reason.
/*************************************************************************/
Status FF_Block_Address::Create(Callback_Context *p_callback_context,
								U32 test_bad_blocks)
{
 	TRACE_ENTRY(FF_Block_Address::Create);

#ifdef CREATE_BAD_BLOCK_0
	// Create bad block table at 0, then move it for testing.
	m_if_create_bad_block_0 = 1;
#else
	m_if_create_bad_block_0 = 0;
#endif

	// Create a FF_Bad_Block_Context
	FF_Bad_Block_Context *p_bad_block_context = 
		(FF_Bad_Block_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Bad_Block_Context));
	if (p_bad_block_context == 0)
		return FF_ERROR(NO_CONTEXT);

	// Use the flash object's page buffer to check pages for blank.
	p_bad_block_context->m_p_page_frame = m_p_flash->m_p_page_buffer;

	// When surface test is complete, call Surface_Test_Complete.
	p_bad_block_context->Set_Callback(&Surface_Test_Complete);
	
	Status status = Run_Surface_Test(p_bad_block_context,
		
		// Use cache memory for a buffer for each unit.
		// Cache memory is not yet being used.
		m_p_flash->m_p_cache_memory,
		
		// Level zero is for blank check only.
		test_bad_blocks);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

	return status;

} // FF_Block_Address::Create

/*************************************************************************/
// FF_Block_Address::Surface_Test_Complete
/*************************************************************************/
void FF_Block_Address::Surface_Test_Complete(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Surface_Test_Complete);

	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	
#ifdef MOVE_BAD_BLOCK_0

	// Erase the old bad block table, then close and write it out.
	p_block_address->Erase_Bad_Block_Table(p_bad_block_context);
#else
	// Close the table and write it out.
	p_block_address->Write_Bad_Block_Table(p_bad_block_context);
#endif

} // FF_Block_Address::Surface_Test_Complete

#ifdef MOVE_BAD_BLOCK_0
/*************************************************************************/
// Erase the bad block table in block 0, if any.
// This is only used as long as boards exist with bad block table
// at block 0.
/*************************************************************************/
void FF_Block_Address::Erase_Bad_Block_Table(FF_Bad_Block_Context *p_bad_block_context)
{
 	TRACE_ENTRY(FF_Block_Address::Erase_Bad_Block_Table);

	// Set to erase the bad block header for array 0 first
	p_bad_block_context->m_page_number = 0;
	p_bad_block_context->m_array_number = 0;
	p_bad_block_context->m_flash_address = m_address_bad_block_array_0_copy_1_block0;

	// When erase is complete, call Erase_Next_Bad_Block.
	p_bad_block_context->Set_Callback(&Erase_Next_Bad_Block);
	
	// Start the erase of the first block.
	Status status = m_p_flash->m_controller.Erase_Page_Block(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		FF_WRITING_BAD_BLOCK_TABLE);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

} // FF_Block_Address::Erase_Bad_Block_Table
	
/*************************************************************************/
// FF_Block_Address::Erase_Next_Bad_Block
// Called when erase of one page of bad block has completed.
/*************************************************************************/
void FF_Block_Address::Erase_Next_Bad_Block(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Erase_Next_Bad_Block);

	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	
	// Check the status of the last erase operation.
	if (status != OK)
	{
		Tracef("\nUnable to erase bad block table, status = %X", status);
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

	// Set up next flash address to erase.
	switch(p_bad_block_context->m_page_number++)
	{
	case 0:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_0_copy_2_block0;
		break;

	case 1:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_1_copy_1_block0;
		break;

	case 2:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_1_copy_2_block0;
		break;

	case 3:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_0_copy_2_block0;
		break;

	case 4:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_0_copy_1;
		break;

	case 5:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_0_copy_2;
		break;

	case 6:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_1_copy_1;
		break;

	case 7:
		p_bad_block_context->m_flash_address = 
			p_block_address->m_address_bad_block_array_1_copy_2;
		break;

	case 8:
		p_block_address->Write_Bad_Block_Table(p_bad_block_context);
		return;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL	,
			"FF_Block_Address::Erase_Next_Bad_Block", 
			"internal error",
			0,
			p_bad_block_context->m_page_number);
	}

	// Start the erase of the next block.
	status = p_flash->m_controller.Erase_Page_Block(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		FF_WRITING_BAD_BLOCK_TABLE);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

} // Erase_Next_Bad_Block

#endif // MOVE_BAD_BLOCK_0

/*************************************************************************/
// Write out bad block table after all blocks have been checked.
/*************************************************************************/
void FF_Block_Address::Write_Bad_Block_Table(FF_Bad_Block_Context *p_bad_block_context)
{
 	TRACE_ENTRY(FF_Block_Address::Write_Bad_Block_Table);

	// Create block map tables from bad block table.
	// Mark replacement pages as bad pages in page table.
	Status status = Create_Block_Map_Tables();
	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

	// Now the bad block table is initialized.
	m_initialized = 1;
		
	// Set up flash addresses of system structures (TOC, page map, bat blocks).
	status = Initialize_System_Flash_Addresses();
	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

	// Initialize the first bad page address so that bad pages
	// encountered can count down.
	m_p_flash->m_page_map.Initialize_Bad_Page_Addresses();

	// Assign every real address to a virtual address.
	Flash_Address flash_address;
	U32 num_blocks = Flash_Address::Num_Blocks();
	for (U32 block_number = 0; block_number < num_blocks; block_number++)
	{
#ifdef MOVE_BAD_BLOCK_0
		// If we moved the bad block table, then pages are not erased.
		if (m_if_move_bad_block_table)
#endif
		{
			// Was this page block in the bad block table?
			if (!m_p_flash->m_page_map.Is_Page_Bad(flash_address))

				// Set this page block erased in the page map.
				m_p_flash->m_page_map.Set_Page_Block_Erased(flash_address);
		}

		// Assign the next block of addresses in the page map.
		m_p_flash->m_page_map.Assign_Virtual(flash_address);

		// Increment the address to point to the next page block.
		flash_address.Increment_Block();
	}

	// Use the flash object's page buffer to create the bad block header.
	FF_Bad_Block_Header *p_bad_block_header = 
		(FF_Bad_Block_Header *)m_p_flash->m_p_page_buffer;

	// Initialize bad block header.
	ZERO(p_bad_block_header, Flash_Address::Bytes_Per_Page());
	p_bad_block_header->Create();
	
	// Set to write out the bad block header for array 0 first
	p_bad_block_context->m_p_page_frame = p_bad_block_header;
	p_bad_block_context->m_page_number = 0;
	p_bad_block_context->m_array_number = 0;
	p_bad_block_context->m_flash_address = Address_Bad_Block_Table_Array_0_Copy_1(FF_Mode_Create);

	// Set the number of pages we need to write.
	p_bad_block_context->m_num_pages = m_bad_block_bitmap_num_pages_per_array;

	// When write is complete, call Write_Next_Bad_Block_Page.
	p_bad_block_context->Set_Callback(&Write_Next_Bad_Block_Page);
	
	// Start the write of the header page.
	status = m_p_flash->m_controller.Write_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame,
		FF_NO_RETRY_WRITE_IF_ERROR | FF_WRITING_BAD_BLOCK_TABLE);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

} // FF_Block_Address::Write_Bad_Block_Table
	
/*************************************************************************/
// FF_Block_Address::Write_Next_Bad_Block_Page
// Called when write of one page of bad block table has completed.
/*************************************************************************/
void FF_Block_Address::Write_Next_Bad_Block_Page(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Write_Next_Bad_Block_Page);

	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	
	// Check the status of the last write operation.
	if (status != OK)
	{
		Tracef("\nUnable to write bad block table, status = %X", status);
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

	// Did we just write the header page?
	if (p_bad_block_context->m_page_number == 0)
	{
		// The next page will be the first page of the bitmap.
		// Set the memory address of the bitmap to write out.
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
	}
	else
	{
		// See if we need to write more pages.
		if (p_bad_block_context->m_num_pages == p_bad_block_context->m_page_number++)
		{
			// We have finished writing a bad block table.
			// Do we also need to write the bad block table for array 1?
			if ((Flash_Address::Num_Arrays() > 1) && (p_bad_block_context->m_array_number == 0))
			{
				// Set to write out the bad block header for array 1 next.
				p_bad_block_context->m_p_page_frame = (FF_Bad_Block_Header *)p_flash->m_p_page_buffer;
				p_bad_block_context->m_page_number = 0;
				p_bad_block_context->m_array_number = 1;
				p_bad_block_context->m_flash_address = 
					p_block_address->Address_Bad_Block_Table_Array_1_Copy_1(FF_Mode_Create);
			}
			else
			{
				// We have finished writing both bad block tables.
				p_bad_block_context->Terminate(p_bad_block_context, status);
				return;
			}
		}
		else
		{
			// The last page was a bitmap page.
			// Point to the next page of the bitmap.
			p_bad_block_context->m_p_page_frame = 
				(char *)p_bad_block_context->m_p_page_frame +
				Flash_Address::Bytes_Per_Page();

			// Increment the SSD page address to point to the next page.
			p_bad_block_context->m_flash_address.Increment_Page();
		}
	}

	// Start the write of the next page of the bad block table.
	status = p_flash->m_controller.Write_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame,
		FF_NO_RETRY_WRITE_IF_ERROR | FF_WRITING_BAD_BLOCK_TABLE);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

} // FF_Block_Address::Write_Next_Bad_Block_Page

/*************************************************************************/
// FF_Block_Address::Mark_Block_Bad
/*************************************************************************/
void FF_Block_Address::Mark_Block_Bad(Flash_Address flash_address, U32 offset)
{
 	//TRACE_ENTRY(FF_Block_Address::Mark_Block_Bad);

	// Set the device and bank from the offset in the page.
	// O O O O O O O O O b b D D D
	flash_address.Device(offset);

	// Calculate entry into bad block bitmap.
	// Each FF_Bad_Block_Bit_Map_Entry is a 64-bit word.
	// Each bit represents one device block.
	// ABBBBBBBBBBBbb is index into bad block doubleword entry.
	// A = Array, B = Block number, b = bank number.
	U32 bad_block_entry_index = flash_address.Bad_Block_Entry_Index();

	// Calculate the bit index.
	// CCCDDD is used as an index into the doubleword, where
	// C = Column, D = device.
	U32 bad_block_bit_index = flash_address.Bad_Block_Bit_Index();
	
	// Create mask for this bad block.
	UI64 mask = (1 << bad_block_bit_index);

	// Is this already marked bad?
	if ((m_p_bad_block_bitmap[bad_block_entry_index] & mask) == 0)
	{
		// Set the bit in the bitmap for this bad block.
		m_p_bad_block_bitmap[bad_block_entry_index] |= mask;
		
		m_num_bad_blocks++;
	}
}

