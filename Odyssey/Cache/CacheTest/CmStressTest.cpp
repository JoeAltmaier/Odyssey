/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmStressTest.cpp
// 
// Description:
// This file tests the cache manager 
// 
// Update Log 
// 
// 10/1/98 Jim Frandeen: Create file
/*************************************************************************/

#include "ErrorLog.h"
#include "Cache.h"
#include "TestDevice.h"
#include <afx.h>
#include "TraceMon.h"
#include "WinAsyncIO.h"
#include <stdlib.h>
#include "CacheTestData.h"
#include "CmTest.h"
#include "CmFrameHandle.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
void Cache_Initialization_Complete(void *p_context, STATUS status);
void Stop_Stress_Test(void *p_context, STATUS status);
void Stop_Stress_Test_1(void *p_context, STATUS status);
void Stress_Read_0(void *p_context, STATUS status);
void Stress_Read_1(void *p_context, STATUS status);
void Stress_Read_Complete_0(void *p_context, STATUS status);
void Stress_Read_Complete_1(void *p_context, STATUS status);
void Stress_Reads_Complete(void *p_context, STATUS status);
void Stress_Routine(void *p_context, STATUS status);
DWORD __stdcall Stress_Thread(LPVOID lpParameter);
void Stress_Write_0(void *p_context, STATUS status);
void Stress_Write_1(void *p_context, STATUS status);
void Write_Data_To_Page(CM_Test_Context *p_test_context);

/*************************************************************************/
// globals
/*************************************************************************/
U32 write_sequence_number = 0;
U32 stop_stress_test = 0;

/*************************************************************************/
// 	Stop_Flush_Cache
/*************************************************************************/
void Stop_Flush_Cache(void *p_context, STATUS status)
{
    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	// When the close has completed, call Terminate.
	// When both caches have been flushed, the parent will run again.
    p_test_context->Set_Callback(&Callback_Context::Terminate);

	// Flush the cache
	CM_Flush_Cache(p_test_context->m_cache_handle, p_test_context);

} // Stop_Flush_Cache

/*************************************************************************/
// 	Stop_Stress_Test
/*************************************************************************/
void Stop_Stress_Test(void *p_context, STATUS status)
{
    CM_Test_Context *p_parent_context = (CM_Test_Context *)p_context;

	// Create a child context for each cache.
	// When it terminates, the parent will run.
	CM_Test_Context *p_child_context = 
		(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
	if (p_child_context == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stop_Stress_Test", 
			"Could not allocate context to stop test",
			NU_NO_MEMORY,
			0);
		Callback_Context::Terminate(p_parent_context, NU_NO_MEMORY); 
		return;
	}

	// Set up parameters in child context.
	p_child_context->m_verify = 1;
	p_child_context->m_cache_handle = CT_cache_handle_0;

	// Set up context to start at Stop_Flush_Cache.
	p_child_context->Set_Callback(&Stop_Flush_Cache);

	// Allocate child for second cache.
	p_child_context = 
		(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
	if (p_child_context == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stop_Stress_Test", 
			"Could not allocate context to stop test",
			NU_NO_MEMORY,
			0);
		Callback_Context::Terminate(p_parent_context, NU_NO_MEMORY); 
		return;
	}

	// Set up parameters in child context.
	p_child_context->m_verify = 1;
	p_child_context->m_cache_handle = CT_cache_handle_1;

	// Set up context to start at Stop_Flush_Cache.
	p_child_context->Set_Callback(&Stop_Flush_Cache);

	// Call Stop_Stress_Test_1 when both caches have been flushed.
	p_parent_context->Set_Callback(&Stop_Stress_Test_1);

	// Start the children.
	p_parent_context->Make_Children_Ready();

} // Stop_Stress_Test

/*************************************************************************/
// 	Stop_Stress_Test_1
// Come here when both caches have been flushed.
// Now read and verify both caches to be sure the flush was successful.
/*************************************************************************/
void Stop_Stress_Test_1(void *p_context, STATUS status)
{
    CM_Test_Context *p_parent_context = (CM_Test_Context *)p_context;

	p_parent_context->m_page_number = 0;
	p_parent_context->m_verify = 1;
	Stress_Read_0(p_context, status);

} // Stop_Stress_Test_1

/*************************************************************************/
// Stress_Test_Cache
/*************************************************************************/
STATUS Stress_Test_Cache(CM_CONFIG *p_cache_config, 
	U32 memory_size_to_allocate, U32 num_threads,
	WPARAM message, HWND hWnd)
{
    // Allocate memory for the test
    void *p_memory = malloc(memory_size_to_allocate);
    if (p_memory == 0)
        return NU_NO_MEMORY;

	// Initialize the second cache.
    STATUS status =  CM_Initialize(
	    p_cache_config,
	    p_memory, 
	    memory_size_to_allocate,
	    (CM_PREFETCH_CALLBACK *)0,
	    &Write_Callback,
	    &CT_cache_handle_1,
		0 // p_callback_context
		);

	// Create a parent context.
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_parent_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_parent_context == 0)
        return NU_NO_MEMORY;

	// Save completion parameters in parent context.
    p_parent_context->m_num_threads = num_threads;
    p_parent_context->m_message = message;
    p_parent_context->m_hWnd = hWnd;
    p_parent_context->m_p_cache_config = p_cache_config;

	// Set up the parent context to call Cache_Initialization_Complete when it terminates.
	p_parent_context->Set_Callback(&Cache_Initialization_Complete);

	// Create a child context to write every page in the address space.
	// When it terminates, the parent will run.
    CM_Test_Context *p_child_context = 
		(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
    if (p_child_context == 0)
        return NU_NO_MEMORY;

    p_child_context->m_page_number = 0;
    p_child_context->m_num_pages = p_cache_config->page_table_size;
	p_child_context->m_cache_handle = CT_cache_handle_0;
    p_child_context->m_p_cache_config = p_cache_config;

	// Start the context at Write_Next
    p_child_context->Set_Callback(&Write_Next);

	// Create a second child context to write every page in the address space.
	// When it terminates, the parent will run.
    p_child_context = 
		(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
    if (p_child_context == 0)
        return NU_NO_MEMORY;

    p_child_context->m_page_number = 0;
    p_child_context->m_num_pages = p_cache_config->page_table_size;
	p_child_context->m_cache_handle = CT_cache_handle_1;

	// Start the context at Write_Next
    p_child_context->Set_Callback(&Write_Next);
    p_parent_context->Make_Children_Ready();
    return status;

} // Stress_Test_Cache

/*************************************************************************/
// 	Cache_Initialization_Complete
/*************************************************************************/
void Cache_Initialization_Complete(void *p_context, STATUS status)
{
    CM_Test_Context *p_parent_context = (CM_Test_Context *)p_context;

	// Create a child context for each thread.
	// When it terminates, the parent will run.
	for (U32 index = 0; index < p_parent_context->m_num_threads; index++)
	{
		CM_Test_Context *p_child_context = 
			(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
		if (p_child_context == 0)
		{
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Cache_Initialization_Complete", 
				"Could not allocate context for thread",
				NU_NO_MEMORY,
				0);
			Callback_Context::Terminate(p_parent_context, NU_NO_MEMORY); 
			return;
		}

		// Set up parameters in child context.
		p_child_context->m_verify = 0;
		p_child_context->m_p_cache_config = p_parent_context->m_p_cache_config;

		// Set up context to start at Stress_Routine.
		p_child_context->Set_Callback(&Stress_Routine);

	}

	// When children finish, call Stop_Stress_Test.
	p_parent_context->Set_Callback(&Stop_Stress_Test);

	// Schedule the child contexts.
	p_parent_context->Make_Children_Ready();

} // Cache_Initialization_Complete

/*************************************************************************/
// Stress_Read_0
// Read the page from the first cache.
/*************************************************************************/
void Stress_Read_0(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Read_0);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

    p_test_context->Set_Callback(&Stress_Read_0);

	// Open the page from cache 0,
	status =  CM_Open_Page(
		CT_cache_handle_0, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_READ | CM_CALLBACK_IF_NO_FRAME | CM_CALLBACK_IF_LOCKED,
		p_test_context,
		&p_test_context->m_p_page_frame_0,
		&p_test_context->m_page_handle);

	switch (status)
	{
	case CM_ERROR_CACHE_MISS:

		// We have a page frame
		// Read the page into this frame.
		p_test_context->Set_Callback(&Stress_Read_Complete_0);
		CT_ASSERT((CM_Frame_Handle(p_test_context->m_page_handle).Get_Open_Mode() 
			== CM_OPEN_MODE_READ_NOT_PRESENT), Stress_Read_0);
		status = FB_Device_Read_Page(
			0,  // controller
			p_test_context->m_page_number,
			0, // U16 alternate_address,
			0, // U16 bad_cell_map,
			p_test_context->m_p_page_frame_0,
			p_test_context,
			CT_config.page_size);
		if (status != NU_SUCCESS)
		{
			Callback_Context::Terminate(p_test_context, status);
			return;
		}

		break;

	case NU_SUCCESS:

		// We have a page frame.
		// Read the page from the second cache.
		CT_ASSERT((CM_Frame_Handle(p_test_context->m_page_handle).Get_Open_Mode() == CM_OPEN_MODE_READ), 
			Stress_Read_0);
		Stress_Read_1(p_context, status);
		break;

    case CM_ERROR_NO_PAGE_FRAMES:

        // Stress_Read_0 will be rescheduled when page frame is available.
        break;

    case CM_ERROR_PAGE_LOCKED:
        // Stress_Read_0 will be rescheduled when the page has been unlocked
        break;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stress_Read", 
			"Unexpected error on read",
			status,
			0);
		Callback_Context::Terminate(p_test_context, status);
		return;
		break;

	} // switch

} // Stress_Read_0

/*************************************************************************/
// Stress_Read_1
// Read the page from the second cache.
/*************************************************************************/
void Stress_Read_1(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Read_1);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

    p_test_context->Set_Callback(&Stress_Read_1);

	// The page in the first cache should be locked for read.
	void *p_page_frame_0;
	CM_PAGE_HANDLE page_handle_0;
	status =  CM_Open_Page(
		CT_cache_handle_0, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_NO_FRAME,
		p_test_context,
		&p_page_frame_0,
		&page_handle_0);
	if (status == CM_ERROR_MAX_DIRTY_PAGES)

        // Stress_Read_1 will be rescheduled when a page is available.
		return;

	CT_ASSERT((status == CM_ERROR_PAGE_LOCKED), Stress_Read_1);

	// Open the page from cache 1.
	status =  CM_Open_Page(
		CT_cache_handle_1, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_READ | CM_CALLBACK_IF_NO_FRAME | CM_CALLBACK_IF_LOCKED,
		p_test_context,
		&p_test_context->m_p_page_frame_1,
		&p_test_context->m_page_handle_1);

	switch (status)
	{
	case CM_ERROR_CACHE_MISS:

		// We have a page frame
		// Read the page into this frame.
		CT_ASSERT((CM_Frame_Handle(p_test_context->m_page_handle_1).Get_Open_Mode() 
			== CM_OPEN_MODE_READ_NOT_PRESENT), Stress_Read_0);
		p_test_context->Set_Callback(&Stress_Read_Complete_1);
		status = FB_Device_Read_Page(
			1,  // controller
			p_test_context->m_page_number,
			0, // U16 alternate_address,
			0, // U16 bad_cell_map,
			p_test_context->m_p_page_frame_1,
			p_test_context,
			CT_config.page_size);
		if (status != NU_SUCCESS)
		{
			Callback_Context::Terminate(p_test_context, status);
			return;
		}

		return;
		break;

	case NU_SUCCESS:

		// We have a page frame
		// Complete the reads for both caches.
		CT_ASSERT((CM_Frame_Handle(p_test_context->m_page_handle_1).Get_Open_Mode() 
			== CM_OPEN_MODE_READ), Stress_Read_0);
		Stress_Reads_Complete(p_test_context, status);
		break;

 		// If we can open the page in the first cache, you might think
		// we should be able to open the page in the second cache.  What happens
		// is that task one could be reading the same pair of pages.  Task one
		// gets a cache miss on the second page of the second cache.  Then task
		// two gets a read lock on cache one, then gets a page lock error on the
		// second cache because the page is being read in.
    case CM_ERROR_NO_PAGE_FRAMES:

        // Stress_Read_1 will be rescheduled when page frame is available.
        break;

    case CM_ERROR_PAGE_LOCKED:
        // Stress_Read_1 will be rescheduled when the page has been unlocked
        break;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stress_Read", 
			"Unexpected error on read",
			status,
			0);
		Callback_Context::Terminate(p_test_context, status);
		return;
		break;

	} // switch

} // Stress_Read_1

/*************************************************************************/
// Stress_Read_Complete_0
// Called when read is complete for the first cache.
/*************************************************************************/
void Stress_Read_Complete_0(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Read_Complete_0);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	U32 word_0_frame_0 = *((U32 *)p_test_context->m_p_page_frame_0);
	U32 word_2_frame_0 = *((U32 *)p_test_context->m_p_page_frame_0 + 2);

	// If the page was written, then the second word should have been copied
	// from the first word.
	CT_ASSERT((word_0_frame_0 == word_2_frame_0), Stress_Read_Complete_0);

	// Check status of read operation
	if (status != NU_SUCCESS)
	{
		CM_Abort_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Read the page for the second cache.
	Stress_Read_1(p_context, status);

} // Stress_Read_Complete_0

/*************************************************************************/
// Stress_Read_Complete_1
// Called when read is complete for the second cache.
/*************************************************************************/
void Stress_Read_Complete_1(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Read_Complete_1);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	U32 word_0_frame_1 = *((U32 *)p_test_context->m_p_page_frame_1);
	U32 word_2_frame_1 = *((U32 *)p_test_context->m_p_page_frame_1 + 2);

	// If the page was written, then the second word should have been copied
	// from the first word.
	CT_ASSERT((word_0_frame_1 == word_2_frame_1), Stress_Read_Complete_1);

	// Check status of read operation
	if (status != NU_SUCCESS)
	{
		CM_Abort_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle_1);
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Complete the reads for both caches.
	Stress_Reads_Complete(p_context, status);

} // Stress_Read_Complete_1

/*************************************************************************/
// Stress_Reads_Complete
// Both reads have completed.
/*************************************************************************/
void Stress_Reads_Complete(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Reads_Complete);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	U32 word_0_frame_0 = *((U32 *)p_test_context->m_p_page_frame_0);
	U32 word_1_frame_0 = *((U32 *)p_test_context->m_p_page_frame_0 + 1);
	U32 word_2_frame_0 = *((U32 *)p_test_context->m_p_page_frame_0 + 2);
	U32 word_0_frame_1 = *((U32 *)p_test_context->m_p_page_frame_1);
	U32 word_1_frame_1 = *((U32 *)p_test_context->m_p_page_frame_1 + 1);
	U32 word_2_frame_1 = *((U32 *)p_test_context->m_p_page_frame_1 + 2);
	CT_ASSERT((word_1_frame_0 == 0), Stress_Reads_Complete);
	CT_ASSERT((word_1_frame_1 == 1), Stress_Reads_Complete);


	if (word_0_frame_0 != word_0_frame_1)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Read_Next", 
			"Unexpected error on read",
			status,
			0);
		Callback_Context::Terminate(p_test_context, status);
		return;
	}


	// Close the page in the second cache.
	status = CM_Close_Page(CT_cache_handle_1, 
		p_test_context->m_page_handle_1);
	if (status != NU_SUCCESS)
	{
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Close the page in the first cache.
	status = CM_Close_Page(CT_cache_handle_0, 
		p_test_context->m_page_handle);
	if (status != NU_SUCCESS)
	{
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Check to see if we are in verify mode
	if (p_test_context->m_verify)
	{
		// Increment the page number to verify.
		p_test_context->m_page_number++;

		// Have we finished?
		if (p_test_context->m_page_number >= p_test_context->m_p_cache_config->page_table_size)
		{
			Post_Complete(p_test_context, NU_SUCCESS);
			return;
		}

		// Verify next page.
		p_test_context->Set_Callback(&Stress_Read_0);
		p_test_context->Make_Ready();
	}
	else
	{
		// Set up the next action
		p_test_context->Set_Callback(&Stress_Routine);
		p_test_context->Make_Ready();
	}

} // Stress_Reads_Complete 

/*************************************************************************/
// Stress_Routine
// Come here from Cache_Initialization_Complete, from Stress_Reads_Complete,
// from Stress_Write_1.
/*************************************************************************/
void Stress_Routine(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Routine);

	if (rand() & 1)

		// Relinquish the remainder of the time slice to any other thread of 
		// equal priority that is ready to run. 
		Sleep(0);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	if (stop_stress_test)
	{

		// When all children have run, parent will run.
		Callback_Context::Terminate(p_test_context, status);
		return;
	}

	// Pick a random page number between zero and the number of pages mapped.
	p_test_context->m_page_number = (U32)rand() % p_test_context->m_p_cache_config->page_table_size;

	// Decide what to do next -- read or write
	if (rand() & 1)
		Stress_Read_0(p_context, status);
	else
		Stress_Write_0(p_context, status);
		//Stress_Write(p_context, status);

} // Stress_Routine

/*************************************************************************/
// Stress_Write_0
// Write the page to the first cache.
/*************************************************************************/
void Stress_Write_0(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Write_0);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

    p_test_context->Set_Callback(&Stress_Write_0);

	// Open the page from cache 0,
	status =  CM_Open_Page(
		CT_cache_handle_0, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_NO_FRAME | CM_CALLBACK_IF_LOCKED,
		p_test_context,
		&p_test_context->m_p_page_frame_0,
		&p_test_context->m_page_handle);

	U32 open_mode = CM_Frame_Handle(p_test_context->m_page_handle).Get_Open_Mode();
	switch (status)
	{
	case NU_SUCCESS:

		// We have a page frame to write into
		// Write a random number
		CT_ASSERT((open_mode == CM_OPEN_MODE_WRITE), Stress_Write_0);

		// Abort a page every once in a while.
		//if ((rand() % 10) == 0)
		if (0)
		{
			status = CM_Abort_Page(CT_cache_handle_0, p_test_context->m_page_handle);
			if (status != NU_SUCCESS)
			{
				Callback_Context::Terminate(p_test_context, status);
				return;
			}

			// Set up the next action
			p_test_context->Set_Callback(&Stress_Routine);
			p_test_context->Make_Ready();
			return;
		}

		Write_Data_To_Page(p_test_context);

		// Now write to second cache.
		Stress_Write_1(p_context, status);
		break;

	case CM_ERROR_CACHE_MISS:

		// We have an empty page frame to write into
		// Write a random number
		CT_ASSERT((open_mode == CM_OPEN_MODE_WRITE_NOT_PRESENT), Stress_Write_0);

		// Abort a page every once in a while.
		//if ((rand() % 10) == 0)
		if (0)
		{
			status = CM_Abort_Page(CT_cache_handle_0, p_test_context->m_page_handle);
			if (status != NU_SUCCESS)
			{
				Callback_Context::Terminate(p_test_context, status);
				return;
			}

			// Set up the next action
			p_test_context->Set_Callback(&Stress_Routine);
			p_test_context->Make_Ready();
			return;
		}

		Write_Data_To_Page(p_test_context);

		// Now write to second cache.
		Stress_Write_1(p_context, status);
		break;

    case CM_ERROR_NO_PAGE_FRAMES:

        // Stress_Write_0 will be rescheduled when page frame is available.
        break;

    case CM_ERROR_PAGE_LOCKED:
        // Stress_Write_0 will be rescheduled when the page has been unlocked
        break;

	case CM_ERROR_MAX_DIRTY_PAGES:
        // Stress_Write_0 will be rescheduled when a page is available.
        break;

	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stress_Write", 
			"Unexpected error on Write",
			status,
			0);
		Callback_Context::Terminate(p_test_context, status);
		return;
		break;

	} // switch

} // Stress_Write_0

/*************************************************************************/
// Stress_Write_1
// Write the page to the first cache.
/*************************************************************************/
void Stress_Write_1(void *p_context, STATUS status)
{
	TRACE_ENTRY(Stress_Write_1);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

    p_test_context->Set_Callback(&Stress_Write_1);

	// The page in the first cache should be locked for write.
	void *p_page_frame_0;
	CM_PAGE_HANDLE page_handle_0;
	status =  CM_Open_Page(
		CT_cache_handle_0, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_READ,
		p_test_context,
		&p_page_frame_0,
		&page_handle_0);
	CT_ASSERT((status == CM_ERROR_PAGE_LOCKED), Stress_Write_1);

	// Open the page from cache 1,
	status =  CM_Open_Page(
		CT_cache_handle_1, 
		p_test_context->m_page_number,
		CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_NO_FRAME,
		p_test_context,
		&p_test_context->m_p_page_frame_1,
		&p_test_context->m_page_handle_1);

	switch (status)
	{
		U32 open_mode;
	case NU_SUCCESS:
	case CM_ERROR_CACHE_MISS:

		// We have a page frame to write into
		// Write the same data we wrote into the first cache.
		open_mode = CM_Frame_Handle(p_test_context->m_page_handle_1).Get_Open_Mode();
		CT_ASSERT(((open_mode == CM_OPEN_MODE_WRITE_NOT_PRESENT) | 
			(open_mode == CM_OPEN_MODE_WRITE)), Stress_Write_1);
		CT_ASSERT((p_test_context->m_value == *((U32 *)p_test_context->m_p_page_frame_0)),
			Stress_Write_1);
		*((U32 *)p_test_context->m_p_page_frame_1) = 
			*((U32 *)p_test_context->m_p_page_frame_0);

		// Set the second word of the page frame to be one for cache 1.
		*((U32 *)p_test_context->m_p_page_frame_1 + 1) = 1;

		// Set the third word of the page frame to be zero.
		// We will copy the first word into the second when writeback is called.
		*((U32 *)p_test_context->m_p_page_frame_1 + 2) = 0;

		// Close the page in the second cache first!
		status = CM_Close_Page(CT_cache_handle_1, 
			p_test_context->m_page_handle_1);
		if (status != NU_SUCCESS)
		{
			Callback_Context::Terminate(p_test_context, status);
			return;
		}

		// Close the page in the first cache.
		status = CM_Close_Page(CT_cache_handle_0, 
			p_test_context->m_page_handle);
		if (status != NU_SUCCESS)
		{
			Callback_Context::Terminate(p_test_context, status);
			return;
		}

		// Set up the next action
		p_test_context->Set_Callback(&Stress_Routine);
		p_test_context->Make_Ready();
		break;

		// If we can open the page in the first cache,
		// we should be able to open the page in the second cache.
    case CM_ERROR_NO_PAGE_FRAMES:
    case CM_ERROR_PAGE_LOCKED:
	default:
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Stress_Write_1", 
			"Unexpected error on Write",
			status,
			0);
		Callback_Context::Terminate(p_test_context, status);
		return;
		break;

	} // switch

} // Stress_Write_1 

/*************************************************************************/
// Write_Data_To_Page
/*************************************************************************/
void Write_Data_To_Page(CM_Test_Context *p_test_context)
{

#if 1
	// Write a random number
	p_test_context->m_value = rand();
#else
	// Write a sequence number
	// This would need a critical section.
	p_test_context->m_value = write_sequence_number++;
#endif
	*((U32 *)p_test_context->m_p_page_frame_0) = p_test_context->m_value;

	// Set the second word of the page frame to be zero for cache 0.
	*((U32 *)p_test_context->m_p_page_frame_0 + 1) = 0;

	// Set the third word of the page frame to be zero.
	// We will copy the first word into the second when writeback is called.
	*((U32 *)p_test_context->m_p_page_frame_0 + 2) = 0;

} // Write_Data_To_Page

