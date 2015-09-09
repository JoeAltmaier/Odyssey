/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfBat.cpp
// 
// Description:
// This module implements the Flash File Basic Assurance Test.
// 
// 1/6/99 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "FfCommon.h"
#include "FfPageMap.h"
#include "FfBat.h"
#include "FfStats.h"
#include "FfInterface.h"

// None of this gets compiled if we do not reserve a bat block
// in each unit block.
#ifdef RESERVE_BAT_BLOCK

#if 1
// Turn on TRACE_ERROR
#define TRACE_ERROR Tracef
#else
// Turn off TRACE_ERROR -- if 1 then do nothing, else call Tracef
#define TRACE_ERROR 1? 0 : Tracef  
#endif

extern U32					  FF_bit_mask[32];
/*************************************************************************/
// Run_Bat is the external entry point to run
// the basic assurance test.
/*************************************************************************/
Status FF_Interface::Run_Bat(
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	Callback_Context *p_callback)
{	
 	TRACE_ENTRY(FF_Run_Bat);
 	
	// Initialize 
	if (p_block_test_data_buffer)
	{
		for (U32 index = 0; index < FF_NUM_CELL_PARTS;index++)
		{
			(p_block_test_data_buffer + index)->write_sector_success_map = 0;
			(p_block_test_data_buffer + index)->read_sector_success_map = 0;
			(p_block_test_data_buffer + index)->verify_sector_success_map = 0;
		}
	}

	U32 array_range_high = Flash_Address::Num_Arrays();
	if (array_range_high)
		array_range_high--;

	return Test_Unit(
		p_block_test_data_buffer,
		size_buffer,
		0, // array_range_low,
		array_range_high,
		0, // column_range_low,
		Flash_Address::Columns_Per_Array() - 1, //  column_range_high,
		m_block_address.Bat_Block_Number(), // block_range_low,
		m_block_address.Bat_Block_Number(), // block_range_high,
		p_callback);

} // FF_Run_Bat

/*************************************************************************/
// FF_Interface::Test_Unit
// Run the surface test on the entire unit.
// NOTE: This will destroy the contents of the unit!
/*************************************************************************/
Status FF_Interface::Test_Unit(
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	U32 array_range_low,
	U32 array_range_high,
	U32 column_range_low,
	U32 column_range_high,
	U32 block_range_low,
	U32 block_range_high,
	Callback_Context *p_callback)
{	
 	TRACE_ENTRY(FF_Interface::Test_Unit);
 	
	// Initialize 
	if (p_block_test_data_buffer)
	{
		for (U32 index = 0; index < FF_NUM_CELL_PARTS;index++)
		{
			(p_block_test_data_buffer + index)->write_sector_success_map = 0;
			(p_block_test_data_buffer + index)->read_sector_success_map = 0;
			(p_block_test_data_buffer + index)->verify_sector_success_map = 0;
		}
	}

	// Create a child context to run the test.
	return m_page_map.Create_Child_Test_Unit_Context(
		p_block_test_data_buffer,
		size_buffer,
		array_range_low,
		array_range_high,
		column_range_low,
		column_range_high,
		block_range_low,
		block_range_high,
		p_callback);

} // FF_Interface::Test_Unit

/*************************************************************************/
// Create_Child_Test_Unit_Context.
/*************************************************************************/
Status FF_Page_Map::Create_Child_Test_Unit_Context(
	FF_BLOCK_TEST_DATA *p_block_test_data_buffer,
	U32 size_buffer,
	U32 array_range_low,
	U32 array_range_high,
	U32 column_range_low,
	U32 column_range_high,
	U32 block_range_low,
	U32 block_range_high,
	Callback_Context *p_callback)
{
 	TRACE_ENTRY(FF_Page_Map::Create_Child_Test_Unit_Context);
 	
	// Allocate FF_Bat_Context.  This is the child context that will
	// contain all of the request parameters.
	FF_Bat_Context *p_bat_context = 
		(FF_Bat_Context *)p_callback->Allocate_Child(sizeof(FF_Bat_Context));
	if (p_bat_context == 0)
	{

		// Return to calling context.  No context was created.
		m_p_flash->m_stats.Inc_Num_No_Contexts();
		return FF_ERROR(NO_CONTEXT);
	}

	// Initialize bat context.
	p_bat_context->m_flash_address.Initialize();
	p_bat_context->m_p_flash = m_p_flash;
	p_bat_context->m_p_block_test_data_buffer = p_block_test_data_buffer;
	p_bat_context->m_array_number = array_range_low;
	p_bat_context->m_column_number = column_range_low;
	p_bat_context->m_block_number = block_range_low;
	p_bat_context->m_array_range_low = array_range_low;
	p_bat_context->m_array_range_high = array_range_high;
	p_bat_context->m_column_range_low = column_range_low;
	p_bat_context->m_column_range_high = column_range_high;
	p_bat_context->m_block_range_low = block_range_low;
	p_bat_context->m_block_range_high = block_range_high;

	// Set callback for parent context.  
	p_bat_context->Set_Callback(&Test_Next_Unit_Block);

	// Schedule the child context to run.
	p_callback->Make_Children_Ready();
	return OK;
	
} // Create_Child_Test_Unit_Context

/*************************************************************************/
// Test_Next_Unit_Block .
/*************************************************************************/
void FF_Page_Map::Test_Next_Unit_Block (void *p_context, Status status)
{
 	TRACE_ENTRY(Test_Next_Bat_Block::Erase_Bat_Block);
 	
	FF_Bat_Context *p_bat_context = (FF_Bat_Context *)p_context;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_bat_context->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Have we tested the last block?
	if (p_bat_context->m_block_number > p_bat_context->m_block_range_high)
	{
		// We have tested the last block.
		// Have we tested the last column in this array?
		if (p_bat_context->m_column_number == p_bat_context->m_column_range_high)
		{
			// Have we tested the last array?
			if (p_bat_context->m_array_number == p_bat_context->m_array_range_high)
			{
				// Terminate the context.  
				p_bat_context->Terminate(p_context, status);
				return;
			}
			else
			{
				// Increment the array number for the next text.
				p_bat_context->m_array_number++;

				// Set to start the next test with the low block
				p_bat_context->m_block_number = p_bat_context->m_block_range_low;
			
				// Set to start the next test with the low column
				p_bat_context->m_column_number = p_bat_context->m_column_range_low;
			}
		} // last column in array
		else
		{
			// Set to start the next test with the low block
			p_bat_context->m_block_number = p_bat_context->m_block_range_low;
			
			// Increment the column number for the next test.
			p_bat_context->m_column_number++;
		}
	} // last block

	// Allocate FF_Bat_Context.  This is the child context that will
	// contain all of the request parameters to erase and
	// test the next block.
	FF_Bat_Context *p_child_context = 
		(FF_Bat_Context *)p_bat_context->Allocate_Child(sizeof(FF_Bat_Context));
	if (p_child_context == 0)
	{

		// Return to calling context.  No context was created.
		p_bat_context->m_p_flash->m_stats.Inc_Num_No_Contexts();
		p_bat_context->Terminate(p_context, FF_ERROR(NO_CONTEXT));
;
		return;
	}

	// Set up the parameters for the next unit.
	p_child_context->m_flash_address.Initialize();
	p_child_context->m_flash_address.Array(p_bat_context->m_array_number);
	p_child_context->m_flash_address.Column(p_bat_context->m_column_number);
	p_child_context->m_flash_address.Block(p_bat_context->m_block_number);
	p_child_context->m_p_flash = p_bat_context->m_p_flash;
	p_child_context->m_first_page = 1;

	// Set virtual address for the next unit.
	// Point to next block test data for the next unit block.
	if (p_bat_context->m_p_block_test_data_buffer)
		p_child_context->m_p_next_block_test_data_buffer = 
			&p_bat_context->m_p_block_test_data_buffer[p_child_context->m_flash_address.Unit_Index()];
	else
		p_child_context->m_p_next_block_test_data_buffer = 0;

	// Increment block number for the next time.
	p_bat_context->m_block_number++;

	// Begin the next unit test by erasing the page block.
	// Set callback for parent context.  
	p_child_context->Set_Callback(&Erase_Test_Block);

	// Schedule the child context to run.
	p_bat_context->Make_Children_Ready();
	return;

} // Test_Next_Unit_Block

/*************************************************************************/
// Erase_Test_Block .
/*************************************************************************/
void FF_Page_Map::Erase_Test_Block (void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Page_Map::Erase_Test_Block);
 	
	FF_Bat_Context *p_bat_context = (FF_Bat_Context *)p_context;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_bat_context->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Set up context parameters.
	p_bat_context->m_next_flash_address = p_bat_context->m_flash_address;
	p_bat_context->m_sector_number = 0;

	// When erase has completed, call Write_Test_Block.  
	p_bat_context->Set_Callback(&Write_Test_Block);

	TRACE_ERROR(EOL " Testing array = %X, column = %X, block = %X, page = %X",
		p_bat_context->m_flash_address.Array(),
		p_bat_context->m_flash_address.Column(),
		p_bat_context->m_flash_address.Block(),
		p_bat_context->m_flash_address.Sector()
		);
		
	// See if this bat block is also a bad block.
	if (p_page_map->Get_Page_State(p_bat_context->m_flash_address) == FF_PAGE_STATE_BAD_BAT)
	{
		TRACE_ERROR(EOL "Skipping bad block");
		p_bat_context->Terminate(p_context, OK);
		return;
	}

	// Start erase operation for this block.
	status = p_bat_context->m_p_flash->m_controller.Erase_Page_Block(
		p_bat_context, 
		p_bat_context->m_flash_address);
	if (status != OK)
	{
		p_bat_context->Terminate(p_context, status);
		return;
	}

} // Erase_Test_Block

/*************************************************************************/
// Write_Test_Block .
/*************************************************************************/
void FF_Page_Map::Write_Test_Block (void *p_context, Status status)
{
 	TRACE_ENTRY(Write_Test_Block::Write_Bat_Page);
 	
	FF_Bat_Context *p_bat_context = (FF_Bat_Context *)p_context;

	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_bat_context->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Have we written a page yet?
	if (p_bat_context->m_first_page)
		p_bat_context->m_first_page = 0;
	else
	{
		// We wrote a page the last time this context was called,
		// so check the write operation.
		if (!p_page_map->
			Is_Page_Bad(p_bat_context->m_next_flash_address))
		{
			// This page was written correctly, so 
			// set the corresponding bit for this sector in the test result record.
			if (p_bat_context->m_p_next_block_test_data_buffer)
				p_bat_context->m_p_next_block_test_data_buffer->write_sector_success_map
					|= FF_bit_mask[p_bat_context->m_sector_number];
		}
		else
		{
			// Page was not written correctly.
#ifdef TRACE_ERROR
			Tracef(EOL "Write_Test_Block Write error detected, array %X, column %X, block %X, page %X, status = %X", 
				p_bat_context->m_next_flash_address.Array(), 
				p_bat_context->m_next_flash_address.Column(),
				p_bat_context->m_next_flash_address.Block(), 
				p_bat_context->m_next_flash_address.Sector(),
				status);
#endif
			// Reset the status code for this context to continue.
			p_bat_context->Set_Status(OK);
		}

		// Calculate the next flash address
		p_bat_context->m_next_flash_address.Increment_Page();

	} // We wrote a page the last time

	// Have we written the last sector in this block?
	while (p_bat_context->m_sector_number < Flash_Address::Sectors_Per_Block())
	{
		// Increment the sector number for the next time.
		p_bat_context->m_sector_number++;

		if (p_page_map->Is_Page_Bad(p_bat_context->m_next_flash_address))
		{
			// This page was marked bad when it was erased, so we don't try to write to it.
			// Calculate the next flash address
			p_bat_context->m_next_flash_address.Increment_Page();

			break;
		}
		
		// Fill the buffer with a test pattern.
#if 0
		U32 *p_test_pattern = (U32 *)p_page_map->m_p_bat_buffer;
		for (U32 index = 0; index < Flash_Address::Bytes_Per_Page() / sizeof(U32); 
			index++)
		{
			U32 test_pattern = (index << 24) | p_bat_context->m_next_flash_address.Index();
			*(p_test_pattern + index) = test_pattern;
		}
#else
		// Fill the buffer with a test pattern.
		UI64 *p_test_pattern = (UI64 *)p_page_map->m_p_bat_buffer;
		for (U32 index = 0; index < Flash_Address::DWords_Per_Page(); index++)
		{
			UI64 test_pattern = 0x8000000000000000
				| ((UI64)p_bat_context->m_next_flash_address.Index() << 32)
				
				// Index should be in low order so we can see it
				// with the logic analyzer.
				| index;
			*(p_test_pattern + index) = test_pattern;
		}
#endif

		// Start write operation to write out the 
		// next page of test data.
		status = p_bat_context->m_p_flash->m_controller.Write_Page(
			p_bat_context,
			p_bat_context->m_next_flash_address,
			p_page_map->m_p_bat_buffer,
			FF_NO_RETRY_WRITE_IF_ERROR); // Don't retry write if error

		if (status != OK)
		{
			p_bat_context->Terminate(p_context, status);
			return;
		}

		return;
	}

	// We have written to all the sectors in the block.
	// Begin reading each sector.
	p_bat_context->m_sector_number = 0;
	p_bat_context->m_next_flash_address.Sector(0);
	p_bat_context->m_first_page = 1;
	p_bat_context->m_next_flash_address = p_bat_context->m_flash_address;

	// Set callback for parent context.  
	p_bat_context->Set_Callback(&Read_Test_Block);
	p_bat_context->Make_Ready();

} // Write_Test_Block

/*************************************************************************/
// Read_Test_Block .
/*************************************************************************/
void FF_Page_Map::Read_Test_Block (void *p_context, Status status)
{
 	TRACE_ENTRY(FF_Page_Map::Read_Test_Block);
 	
	FF_Bat_Context *p_bat_context = (FF_Bat_Context *)p_context;
	
	// Get pointer to flash object for this context.
	FF_Interface *p_flash = p_bat_context->m_p_flash;
	
	// Get pointer to page map object for this context.
	FF_Page_Map *p_page_map = &p_flash->m_page_map;
	
	// Have we read a page yet?
	if (p_bat_context->m_first_page)
		p_bat_context->m_first_page = 0;
	else
	{
		// We read a page the last time this context was called,
		// so check the read operation.
		if (status == OK)
		{
			// This page was read correctly, so 
			// set the corresponding bit for this sector in the test result record.
			if (p_bat_context->m_p_next_block_test_data_buffer)
				p_bat_context->m_p_next_block_test_data_buffer->read_sector_success_map
					|= FF_bit_mask[p_bat_context->m_sector_number];
		}
		else
		{
#ifdef TRACE_ERROR
			Tracef(EOL "Read_Test_Block Read error detected, array %X, column %X, block %X, page %X, status = %X", 
				p_bat_context->m_next_flash_address.Array(), 
				p_bat_context->m_next_flash_address.Column(),
				p_bat_context->m_next_flash_address.Block(), 
				p_bat_context->m_next_flash_address.Page_Index(),
				status);

#endif
		}
		
		// Check the contents of the sector we just read.
		Status status = OK;
#if 0
		U32 *p_test_pattern = (U32 *)p_page_map->m_p_bat_buffer;

		for (U32 index = 0; index < Flash_Address::Bytes_Per_Page() / sizeof(U32); 
		index++)
		{
			U32 test_pattern = (index << 24) | p_bat_context->m_next_flash_address.Index();
			if (*(p_test_pattern + index) != test_pattern)
			{
				status = FF_ERROR(DATA_MISCOMPARE);
				break;
			}
		} // for
#else
		UI64 *p_test_pattern = (UI64 *)p_page_map->m_p_bat_buffer;
		UI64 test_pattern_expected;
		UI64 test_pattern_found;
		for (U32 index = 0; index < Flash_Address::DWords_Per_Page(); index++)
		{
			test_pattern_expected = 0x8000000000000000
				| ((UI64)p_bat_context->m_next_flash_address.Index() << 32)
				
				// Index should be in low order so we can see it
				// with the logic analyzer.
				| index;
			test_pattern_found = *(p_test_pattern + index);
			if (test_pattern_expected != test_pattern_found)
			{
				status = FF_ERROR(DATA_MISCOMPARE);
				break;
			}
		}
#endif
		
		if (status != OK)
		{
#ifdef TRACE_ERROR
			Tracef(EOL "Verify error array %X, column %X, block %X, page %X", 
				p_bat_context->m_next_flash_address.Array(), 
				p_bat_context->m_next_flash_address.Column(),
				p_bat_context->m_next_flash_address.Block(), 
				p_bat_context->m_next_flash_address.Page_Index() 
				);
#else
			CT_Log_Error(CT_ERROR_TYPE_INFORMATION,
				"FF_Page_Map::Read_Bat_Page", 
				"Bat verify error",
				0,
				0);
			p_bat_context->Terminate(p_context, ERROR_DATA_MISCOMPARE);
			return;
#endif
}

		// This page was verified correctly, so 
		// set the corresponding bit for this sector in the test result record.
		if (p_bat_context->m_p_next_block_test_data_buffer)
			p_bat_context->m_p_next_block_test_data_buffer->verify_sector_success_map
				|= FF_bit_mask[p_bat_context->m_sector_number];

		// Calculate the next flash address
		p_bat_context->m_next_flash_address.Increment_Page();
	}

	// Have we read the last sector in the block?
	while (p_bat_context->m_sector_number < Flash_Address::Sectors_Per_Block())
	{

		if (p_page_map->Is_Page_Bad(p_bat_context->m_next_flash_address))
		{
			// This page was marked bad when it was erased, so we don't try to write to it.
			// Calculate the next flash address
			p_bat_context->m_next_flash_address.Increment_Page();

			break;
		}
		
		// Start read operation to read in the 
		// next page of test data.
		status = p_bat_context->m_p_flash->m_controller.Read_Page(
			p_bat_context,
			p_bat_context->m_next_flash_address,
			p_page_map->m_p_bat_buffer);

		if (status != OK)
		{
			p_bat_context->Terminate(p_context, status);
			return;
		}

		// Increment the sector number for the next time.
		p_bat_context->m_sector_number++;

		return;
	}

	// We have read all the bat blocks.
	// Terminate the context.  
	p_bat_context->Terminate(p_context, status);

} // Read_Test_Block

#endif // RESERVE_BAT_BLOCK

