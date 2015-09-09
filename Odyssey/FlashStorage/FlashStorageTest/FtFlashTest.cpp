/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FtFlashTest.cpp
// 
// Description:
// This file tests the flash file system. 
// 
// $Log: /Gemini/Odyssey/FlashStorage/FlashStorageTest/FtFlashTest.cpp $
// 
// 3     12/10/99 9:37p Jfrandeen
// Use message compiler
// 
// 2     9/21/99 3:07p Jfrandeen
// 
// 1     8/03/99 11:34a Jfrandeen
// 
// 4     5/19/99 9:23a Jfrandeen
// 
// 3     5/16/99 12:31p Jfrandeen
// Update callback.h
// 
// 2     5/05/99 2:42p Jfrandeen
// 
// 1     4/01/99 7:36p Jfrandeen
// Files common to all flash file system test drivers
// 
// 11/16/98 Jim Frandeen: Create file
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "TraceMon.h"
#include "ErrorLog.h"
#include "Cache.h"
#include "FtFlashTest.h"
#include "FfCommon.h"


/*************************************************************************/
// Globals
/*************************************************************************/
extern FF_HANDLE			FT_flash_handle;

/*************************************************************************/
// Forward References
/*************************************************************************/
void Read_Next_Callback(
	U32 transfer_byte_count,
	I64 logical_byte_address,
	STATUS status,
	void *p_context);
void Read_Next(void *p_context, STATUS status);
void Write_Next_Callback(
	U32 transfer_byte_count,
	I64 logical_byte_address,
	STATUS status,
	void *p_context);
void Write_Next(void *p_context, STATUS status);

/*************************************************************************/
// Read_Next_Callback
/*************************************************************************/
void Read_Next_Callback(

	// number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	STATUS status,

	// pointer passed in to Flash File method
	void *p_context)
{
    FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;

	if (status != OK)
	{
		// Save number of pages read in parent context
		((FT_Test_Context *)p_test_context->Get_Parent())->Set_Return_Value
			(p_test_context->m_num_pages);

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Verify data read.
	// Each word contains its byte offset.
	U32 *p_data = (U32 *)p_test_context->m_p_buffer;
	U32 num_words = p_test_context->m_block_size / sizeof(U32);
	U32 word_read;
	U32 word_expected; 
	U32 next_logical_byte_address = p_test_context->m_block_size * p_test_context->m_page_number;
	for (U32 index = 0; index < num_words; index++)
	{
		word_read = *(p_data + index);
		word_expected = next_logical_byte_address + (index * sizeof(U32));
		if (word_read != word_expected)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Read_Next", 
				"Invalid data read",
				0,
				0);

			// Terminate and run parent context.
			Callback_Context::Terminate(p_test_context, FF_ERROR(DATA_MISCOMPARE));
			return;
		}
	}

    // Increment the page number.
    p_test_context->m_page_number++;

	p_test_context->Make_Ready();

} // Read_Next_Callback

/*************************************************************************/
// Read_Next
/*************************************************************************/
void Read_Next(void *p_context, STATUS status)
{
    FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;

	TRACE_ENTRY(Read_Next);

    if (p_test_context->m_page_number >= p_test_context->m_num_pages)
	{
		// Save number of pages written in parent context
		((FT_Test_Context *)p_test_context->Get_Parent())->Set_Return_Value
			(p_test_context->m_num_pages);

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, OK);
		return;
	}

	status = FF_Read(
		FT_flash_handle,
		p_test_context->m_p_buffer, 
		p_test_context->m_block_size, // transfer_byte_count, 
		p_test_context->m_block_size * p_test_context->m_page_number, // logical_byte_address,
		p_context,
		&Read_Next_Callback);
	if (status != OK)
	{

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

} // Read_Next

/*************************************************************************/
// Read_Sequential
/*************************************************************************/
STATUS Read_Sequential(
						U32 block_size,
						U32 num_blocks,
						U32 starting_block,
						void *p_buffer,
						Callback_Context *p_callback_context)
{
	// Create a child context to do the read.
	// When it terminates, the parent will run.
    FT_Test_Context *p_test_context = 
		(FT_Test_Context *)p_callback_context->Allocate_Child(sizeof(FT_Test_Context));
    if (p_test_context == 0)
        return FF_ERROR(NO_MEMORY);

    p_test_context->m_page_number = starting_block;
    p_test_context->m_num_pages = num_blocks;
    p_test_context->m_block_size = block_size;
    p_test_context->m_p_buffer = p_buffer;

	// Start the context at Read_Next
    p_test_context->Set_Callback(&Read_Next);
    p_callback_context->Make_Children_Ready();

    return OK;

} // Read_Sequential

/*************************************************************************/
// Validate
/*************************************************************************/
void Validate()
{
	FF_Validate(FT_flash_handle);
}

/*************************************************************************/
// Write_Next_Callback
/*************************************************************************/
void Write_Next_Callback(

	// number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	STATUS status,

	// pointer passed in to Flash File method
	void *p_context)
{
    FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;

	if (status != OK)
	{
		// Save number of pages written in parent context
		((FT_Test_Context *)p_test_context->Get_Parent())->m_page_number =
			p_test_context->m_page_number;

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, status);
		return;
	}
	p_test_context->Make_Ready();

} // Write_Next_Callback

/*************************************************************************/
// Write_Next
/*************************************************************************/
void Write_Next(void *p_context, STATUS status)
{
    FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;

	TRACE_ENTRY(Write_Next);

    if (p_test_context->m_page_number >= p_test_context->m_num_pages)
	{
		// Save number of pages written in parent context
		((FT_Test_Context *)p_test_context->Get_Parent())->Set_Return_Value
			(p_test_context->m_num_pages);

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, OK);
		return;
	}

	// Fill the page with data that we can verify.
	// Each word contains its byte offset.
	U32 *p_data = (U32 *)p_test_context->m_p_buffer;
	U32 num_words = p_test_context->m_block_size / sizeof(U32);
	U32 word_written;
	U32 logical_byte_address = p_test_context->m_block_size * p_test_context->m_page_number;
	for (U32 index = 0; index < num_words; index++)
	{
		word_written = logical_byte_address + (index * sizeof(U32));
		*(p_data + index) = word_written;
	}

	status = FF_Write(
		FT_flash_handle,
		p_test_context->m_p_buffer, 
		p_test_context->m_block_size, // transfer_byte_count, 
		p_test_context->m_block_size * p_test_context->m_page_number, // logical_byte_address,
		p_context,
		&Write_Next_Callback);
	if (status != OK)
	{

		// Terminate and run parent context.
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

    // Increment the page number.
    p_test_context->m_page_number++;

} // Write_Next

/*************************************************************************/
// Write_Sequential
/*************************************************************************/
STATUS Write_Sequential(
						U32 block_size,
						U32 num_blocks,
						U32 starting_block,
						void *p_buffer,
						Callback_Context *p_callback_context)
{

	// Create a child context to do the write.
	// When it terminates, the parent will run.
    FT_Test_Context *p_test_context = 
		(FT_Test_Context *)p_callback_context->Allocate_Child(sizeof(FT_Test_Context));
    if (p_test_context == 0)
        return FF_ERROR(NO_MEMORY);

    p_test_context->m_page_number = starting_block;
    p_test_context->m_num_pages = num_blocks;
    p_test_context->m_block_size = block_size;
    p_test_context->m_p_buffer = p_buffer;

	// Start the context at Write_Next
    p_test_context->Set_Callback(&Write_Next);
    p_callback_context->Make_Children_Ready();

    return OK;
} // Write_Sequential


