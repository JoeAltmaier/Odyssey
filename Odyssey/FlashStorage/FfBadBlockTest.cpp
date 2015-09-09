/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBadBlockTest.cpp
// 
// Description:
// This module implents bad block test methods.
// 
// 1/25/00 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfCache.h"
#include "FfPageMap.h"
#include "FfRequest.h"
#include "FfStats.h"
#include <String.h>

#define FF_BLANK			0XFFFFFFFFFFFFFFFF
#define FF_CHECKERBOARD		0XAAAAAAAAAAAAAAAA
#define FF_NOT_CHECKERBOARD	0X5555555555555555

/*************************************************************************/
// FF_Block_Address::Run_Surface_Test.
// The parent callback context will be scheduled when all tests 
// have completed.
// p_buffer points to a buffer area, one page for each unit.
//
// If test_level is 0, simply test each page for blank.
// If test_level > 0:
//	Erase each unit.
//	Test each unit for blank.
//	Write checkerboard to each unit.
//	Test each unit for checkerboard.
//	Erase each unit.
//	Write ~checkerboard to each unit.
//	Test each unit for ~checkerboard.
//	Erase each unit.
/*************************************************************************/
Status FF_Block_Address::Run_Surface_Test(Callback_Context *p_callback_context,
	void *p_buffer, U32 test_level)
{
 	TRACE_ENTRY(FF_Block_Address::Run_Surface_Test);
	FF_Bad_Block_Context *p_bad_block_context = 
		(FF_Bad_Block_Context *)p_callback_context->Allocate_Child(
		sizeof(FF_Bad_Block_Context));
	if (p_bad_block_context == 0)
		return FF_ERROR(NO_CONTEXT);

	if (test_level)

		// Do all tests.
		p_bad_block_context->m_state = 2;
	else

		// Only check for blanks.
		p_bad_block_context->m_state = 0;

	// p_buffer points to a buffer area, one page for each unit.
	p_bad_block_context->m_p_page_frame = p_buffer;
	p_bad_block_context->m_p_block_address = this;
	p_bad_block_context->Set_Callback(Run_Next_Test);

	// Schedule the child contexts to run.
	p_callback_context->Make_Children_Ready();
	return OK;
	
} // Run_Surface_Test

/*************************************************************************/
// Run_Next_Test
// 
/*************************************************************************/
void FF_Block_Address::Run_Next_Test(void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Run_Next_Test);

	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	U32 num_bad_blocks = p_block_address->Num_Bad_Blocks();

	switch(p_bad_block_context->m_state++)
	{
	case 0:

		// Test each unit for blank.
		// Start here normally when we get a board with bad blocks
		// marked from the factory.
		TRACEF(TRACE_L3, ("\nTesting each unit for blank"));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_READ, FF_BLANK);
		break;

	case 1:
		// and then terminate.
		TRACEF(TRACE_L3, ("\nNumber of bad blocks detected: %d", num_bad_blocks));
		p_bad_block_context->Terminate(p_bad_block_context, OK);
		break;

	case 2:

		// Start with 2 to erase every unit, then test each unit.
		// Start here if the bad block table has been destroyed, and we need to
		// start over.
		// Erase each unit
		TRACEF(TRACE_L3, ("\nErasing each unit"));

		p_block_address->Test_Each_Unit(p_bad_block_context, DO_ERASE, 0);
		break;

	case 3:

		// Test each unit for blank.
		TRACEF(TRACE_L3, ("\nTesting each unit for blank"));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_READ, FF_BLANK);
		break;

	case 4:

		// Write checkerboard to each unit.
		TRACEF(TRACE_L3, ("\nNumber of bad blocks detected: %d", num_bad_blocks));
		TRACEF(TRACE_L3, ("\nWriting checkerboard to each unit."));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_WRITE, FF_CHECKERBOARD);
		break;

	case 5:

		// Test each unit for checkerboard.
		TRACEF(TRACE_L3, ("\nTesting each unit for checkerboard"));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_READ, FF_CHECKERBOARD);
		break;

	case 6:

		// Erase each unit.
		TRACEF(TRACE_L3, ("\nNumber of bad blocks detected: %d", num_bad_blocks));
		TRACEF(TRACE_L3, ("\nErasing each unit"));

		p_block_address->Test_Each_Unit(p_bad_block_context, DO_ERASE, 0);
		break;

	case 7:

		// Write NOT checkerboard to each unit.
		TRACEF(TRACE_L3, ("\nWriting NOT checkerboard to each unit."));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_WRITE, FF_NOT_CHECKERBOARD);
		break;

	case 8:

		// Test each unit for NOT checkerboard.
		TRACEF(TRACE_L3, ("\nTesting each unit for NOT checkerboard"));
	
		p_block_address->Test_Each_Unit(p_bad_block_context, DO_READ, FF_NOT_CHECKERBOARD);
		break;

	case 9:

		// Erase each unit
		TRACEF(TRACE_L3, ("\nNumber of bad blocks detected: %d", num_bad_blocks));
		TRACEF(TRACE_L3, ("\nErasing each unit"));

		p_block_address->Test_Each_Unit(p_bad_block_context, DO_ERASE, 0);
		break;

	default:
		TRACEF(TRACE_L3, ("\nNumber of bad blocks detected: %d", num_bad_blocks));
		p_bad_block_context->Terminate(p_bad_block_context, OK);
	}
	
} // Run_Next_Test

/*************************************************************************/
// FF_Block_Address::Test_Each_Unit.
// Create context to erase, write or read each unit.
// Schedule parent when all units have been tested.
/*************************************************************************/
Status FF_Block_Address::Test_Each_Unit(FF_Bad_Block_Context *p_bad_block_context,
	U32 IO_code, UI64 test_value)
{
 	TRACE_ENTRY(FF_Block_Address::Test_Each_Unit);

	// Create a child context to erase, write or read pages in each unit.
	for (U32 unit_index = 0; unit_index < Flash_Address::Num_Units(); 
		unit_index++)
	{
		// Allocate a child context to read, write or erase this unit.
		FF_Bad_Block_Context *p_child = 
			(FF_Bad_Block_Context *)p_bad_block_context->Allocate_Child(
			sizeof(FF_Bad_Block_Context));
		if (p_child == 0)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Test_Each_Unit", 
				"Allocate_Child failed",
				FF_ERROR(NO_MEMORY),
				0);
			
			m_p_flash->m_stats.Inc_Num_No_Contexts();
			return FF_ERROR(NO_CONTEXT);
		}

		// Set parameters for child context.
		p_child->m_test_value = test_value;
		p_child->m_IO_code = IO_code;
		p_child->m_page_number = 0;
		p_child->m_flash_address.Initialize();
		p_child->m_flash_address.Unit_Index(unit_index);
		p_child->m_p_block_address = this;
		p_child->m_p_controller = &m_p_flash->m_controller;

		// Calculate buffer address for this unit.
		p_child->m_p_page_frame = (char *)p_bad_block_context->m_p_page_frame +
			unit_index * Flash_Address::Bytes_Per_Page();

		// Set start address for child context.
		p_child->Set_Callback(&Do_Unit_IO);
	}

	// Schedule the child contexts to run.
	p_bad_block_context->Make_Children_Ready();
	return OK;
	
} // Test_Each_Unit

/*************************************************************************/
// FF_Block_Address::Do_Unit_IO
// This context will erase all blocks in the unit.
// When this context is called:
//	It was called for the first time (p_bad_block_context->m_page_number == 0)
//	or an erase was finished by Erase_Page_Block_If_Good
/*************************************************************************/
void FF_Block_Address::Do_Unit_IO(void *p_context, Status status)
{
	//TRACE_ENTRY(FF_Block_Address::Do_Unit_IO);

	FF_Bad_Block_Context *p_bad_block_context = (FF_Bad_Block_Context *)p_context;

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	switch (p_bad_block_context->m_IO_code)
	{
	case (DO_ERASE):

		p_block_address->Do_Erase(p_bad_block_context, status);
		break;

	case (DO_READ):

		p_block_address->Do_Read(p_bad_block_context, status);
		break;

	case (DO_WRITE):

		p_block_address->Do_Write(p_bad_block_context, status);
		break;

	default:

		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Do_Unit_IO", 
			"Invalid IO code",
			status,
			p_bad_block_context->m_IO_code);

		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

} //  FF_Block_Address::Do_Unit_IO

/*************************************************************************/
// FF_Block_Address::Do_Erase
// This context will erase all blocks in the unit.
// When this context is called:
//	It was called for the first time (p_bad_block_context->m_page_number == 0)
//	or an erase was finished by Erase_Page_Block_If_Good
/*************************************************************************/
void FF_Block_Address::Do_Erase(FF_Bad_Block_Context *p_bad_block_context, Status status)
{
	//TRACE_ENTRY(FF_Block_Address::Do_Erase);

	if (status != OK)
	{
		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

	// The first time we get called, we have not yet started an erase operation.
	if (p_bad_block_context->m_page_number++ != 0)
	{		

		// A page block was erased last time.
		// Increment the block number for the next erase.
		if (p_bad_block_context->m_flash_address.Increment_Unit_Block() == 0)
		{
			// The last block has been erased for this unit, so we can terminate and
			// free this erase unit context.  When the last erase unit
			// context has terminated, the parent context
			// will be scheduled to run again.
			p_bad_block_context->Terminate(p_bad_block_context, OK);
			return;
		}
	}

	// Start erase operation for this block.
	status = p_bad_block_context->m_p_controller->Erase_Page_Block(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address,

		// If FF_WRITING_BAD_BLOCK_TABLE is specified, the page will not be verified.
		FF_WRITING_BAD_BLOCK_TABLE);

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Do_Erase", 
			"Erase_Page_Block failed",
			status,
			0);
		p_bad_block_context->Terminate(p_bad_block_context, status);
	}

} //  FF_Block_Address::Do_Erase

/*************************************************************************/
// FF_Block_Address::Do_Read
// Check each page for a value specified in the FF_Bad_Block_Context.
/*************************************************************************/
void FF_Block_Address::Do_Read(FF_Bad_Block_Context *p_bad_block_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Do_Read);

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	
	// The first time we get called, we have not yet started a read operation.
	if (p_bad_block_context->m_page_number++ != 0)
	{	

		// Check the status of the read.
		if (status != OK)
		{
			if (status == FF_ERROR_CODE(ECC))
			{
				// There won't be any ECC when the device is new.
				status = OK;
				p_bad_block_context->Set_Status(OK);
			}
			else
			{
				p_bad_block_context->Terminate(p_bad_block_context, status);
				return;
			}
		}

		// Check the entire interleaved page for a value.
		UI64 test_value = p_bad_block_context->m_test_value;
		UI64 *p_device_page = (UI64 *)p_bad_block_context->m_p_page_frame;
		U32 dwords_per_page = Flash_Address::DWords_Per_Page();
		
		// TEMPORARY to catch erase problem.
		CT_ASSERT((*(p_device_page + 3) != 0), Do_Read);
		for (U32 dword_index = 0; dword_index < dwords_per_page; dword_index++)
		{
			if (*(p_device_page + dword_index) != test_value)
			{
				// The dword is not blank.  Check each byte of the dword.
				U8 *p_byte_page = (U8 *)(p_device_page + dword_index);
				for (U32 byte_index = 0; byte_index < 8; byte_index++)
				{
					if (*(p_byte_page + byte_index) != (U8)test_value)
						p_block_address->Mark_Block_Bad(p_bad_block_context->m_flash_address, 
							byte_index + dword_index * 8);
				}
			}
		}

		// Increment the address to point to the next page.
		if (p_bad_block_context->m_flash_address.Increment_Unit_Page() == 0)
		{
			// The last page in the unit has been processed.
			p_bad_block_context->Terminate(p_bad_block_context, OK);
			return;
		}

	} // (p_bad_block_context->m_page_number++ != 0)

#ifdef TRACE_BAD_BLOCK_CREATE
	// The first sector of every block, display how many 
	// blocks we have left to read.
	if ((p_bad_block_context->m_page_number % Flash_Address::Sectors_Per_Block()) == 0)
	{
		U32 num_blocks_total = p_bad_block_context->m_num_pages / 
			Flash_Address::Sectors_Per_Block();
		U32 current_block_number = p_bad_block_context->m_page_number / 
			Flash_Address::Sectors_Per_Block();
		U32 num_blocks_left = num_blocks_total - current_block_number;
		Tracef(" %d", num_blocks_left);
	}
#endif

	// Start the read of the next page.
	status = p_flash->m_controller.Read_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame); 

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Do_Read", 
			"Read_Page failed",
			status,
			0);

		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

} // FF_Block_Address::Do_Read

/*************************************************************************/
// FF_Block_Address::Do_Write
// Write a value specified in the FF_Bad_Block_Context to the next page.
/*************************************************************************/
void FF_Block_Address::Do_Write(FF_Bad_Block_Context *p_bad_block_context, Status status)
{
 	TRACE_ENTRY(FF_Block_Address::Do_Write);

	// Get pointer to bad block object for this context.
	FF_Block_Address *p_block_address = p_bad_block_context->m_p_block_address;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_block_address->m_p_flash;
	
	// The first time we get called, we have not yet started a write operation.
	if (p_bad_block_context->m_page_number++ == 0)
	{	
		// Fill the buffer with the value to write to the page.
		UI64 test_value = p_bad_block_context->m_test_value;
		UI64 *p_device_page = (UI64 *)p_bad_block_context->m_p_page_frame;
		U32 dwords_per_page = Flash_Address::DWords_Per_Page();
		for (U32 dword_index = 0; dword_index < dwords_per_page; dword_index++)
		{
			*(p_device_page + dword_index) = test_value;
		}

	} // (p_bad_block_context->m_page_number++ != 0)
	else
	{
		// Not the first time -- we wrote a page the last time.
		// Increment the address to point to the next page.
		if (p_bad_block_context->m_flash_address.Increment_Unit_Page() == 0)
		{
			// The last page in the unit has been processed.
			p_bad_block_context->Terminate(p_bad_block_context, OK);
			return;
		}
	}

	// Don't try to write to any bad pages.
	while (p_flash->m_page_map.Is_Page_Bad(p_bad_block_context->m_flash_address))
	{
		// Increment the address to point to the next page.
		if (p_bad_block_context->m_flash_address.Increment_Unit_Page() == 0)
		{
			// The last page in the unit has been processed.
			p_bad_block_context->Terminate(p_bad_block_context, OK);
			return;
		}
	}

	// Start the write of the next page.
	status = p_flash->m_controller.Write_Page(
		p_bad_block_context, 
		p_bad_block_context->m_flash_address, 
		p_bad_block_context->m_p_page_frame,

		// Don't verify after write.
		FF_WRITING_BAD_BLOCK_TABLE | FF_NO_VERIFY); 

	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Do_Write", 
			"Write_Page failed",
			status,
			0);

		p_bad_block_context->Terminate(p_bad_block_context, status);
		return;
	}

} // FF_Block_Address::Do_Write

