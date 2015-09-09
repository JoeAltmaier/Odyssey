/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CmCacheTest.cpp
// 
// Description:
// This file tests the cache manager 
// 
// Update Log 
// 
// 9/9/98 Jim Frandeen: Create file
/*************************************************************************/

#include "ErrorLog.h"
#include "Cache.h"
#include "TestDevice.h"
#include "CmTest.h"
#include "TraceMon.h"
#include "WinAsyncIO.h"
#include <stdlib.h>


//    #define CmData 
//    This causes the data in the module to be declared without the extern
//#define CmData
#include "CacheTestData.h"


/*************************************************************************/
// Get_Cache_Stats for cache 0 or cache 1
/*************************************************************************/
STATUS Get_Cache_Stats(CString *string, U32 cache_number)
{
	I64	align;
	CM_STATISTICS statistics;
	I64	align1;
	CM_EVENT_DATA event_data;

	STATUS status;
	if (cache_number)
	{
		status = CM_Get_Statistics(CT_cache_handle_1, &statistics, sizeof(statistics));
		if (status != NU_SUCCESS)
			return status;
		status = CM_Get_Event_Data(CT_cache_handle_1, &event_data, sizeof(event_data));
	}
	else
	{
		status = CM_Get_Statistics(CT_cache_handle_0, &statistics, sizeof(statistics));
		if (status != NU_SUCCESS)
			return status;
		status = CM_Get_Event_Data(CT_cache_handle_0, &event_data, sizeof(event_data));
	}

	if (status != NU_SUCCESS)
		return status;

	Format_Cache_Stats(string->GetBufferSetLength(8000), &CT_config, &event_data, &statistics);
	return NU_SUCCESS;

} // Get_Cache_Stats

/*************************************************************************/
// Close_Cache
/*************************************************************************/
STATUS Close_Cache(WPARAM message, HWND hWnd)
{
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_test_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_test_context == 0)
        return NU_NO_MEMORY;

    p_test_context->m_message = message;
    p_test_context->m_hWnd = hWnd;
	p_test_context->m_cache_handle = CT_cache_handle_0;

	// When the close has completed, call Post_Complete.
    p_test_context->Set_Callback(&Post_Complete);

	// Close the cache
	return CM_Close_Cache(CT_cache_handle_0, p_test_context);

} // Close_Cache

/*************************************************************************/
// Flush_Cache
/*************************************************************************/
STATUS Flush_Cache(WPARAM message, HWND hWnd)
{
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_test_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_test_context == 0)
        return NU_NO_MEMORY;

    p_test_context->m_message = message;
    p_test_context->m_hWnd = hWnd;
	p_test_context->m_cache_handle = CT_cache_handle_0;

	// When the flush has completed, call Post_Complete.
    p_test_context->Set_Callback(&Post_Complete);

	// Flush the cache
	return CM_Flush_Cache(CT_cache_handle_0, p_test_context);

} // Flush_Cache

#define MEMORY_FOR_CALLBACKS 10000
/*************************************************************************/
// Initialize_Test_Cache
// Called from Config setup window after user has entered 
// cache configuration parameters.
/*************************************************************************/
STATUS Initialize_Test_Cache(CM_CONFIG *p_cache_config, 
							 U32 memory_size_to_allocate,
							 int is_write_back)
{
    void *p_memory;
     
    // Allocate memory for the test
    p_memory = malloc(memory_size_to_allocate);
    if (p_memory == 0)
        return NU_NO_MEMORY;

	STATUS status = Callback_Context::Initialize(p_memory, MEMORY_FOR_CALLBACKS, 100);
	if (status != NU_SUCCESS)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Thread_Start_Routine", 
			"Callback_Context::Initialize failed",
			status,
			0);
	}

    status =  CM_Initialize(
	    p_cache_config,
	    p_memory, 
	    memory_size_to_allocate,
	    (CM_PREFETCH_CALLBACK *)0,
		// If write-back cache, specify write callback
		is_write_back ? &Write_Callback : 0,
	    &CT_cache_handle_0,
		0 // p_callback_context
		);

    return status;

} // Initialize_Test_Cache

/*************************************************************************/
// 	Post_Complete
/*************************************************************************/
void Post_Complete(void *p_context, STATUS status)
{
    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;
	BOOL success = PostMessage(
		p_test_context->m_hWnd,	// handle of destination window
		p_test_context->m_message,	// message to post 
		status,	// first message parameter
		p_test_context->m_page_number 	// second message parameter
	   );

	if (success == 0)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Post_Complete", 
			"PostMessage failed",
			GetLastError(),
			0);
	}
    Callback_Context::Terminate(p_test_context, status);
} // Post_Complete

/*************************************************************************/
// Called when read is complete.
/*************************************************************************/
void Read_Complete(void *p_context, STATUS status)
{
	TRACE_ENTRY(Read_Complete);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	// Check status of read operation
	if (status != NU_SUCCESS)
	{
		CM_Abort_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);
		Post_Complete(p_test_context, status);
		return;
	}
    status = CM_Close_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);
    p_test_context->Set_Callback(&Read_Next);
	Read_Next(p_context, status);

} // Read_Complete

/*************************************************************************/
// Read_Next
/*************************************************************************/
void Read_Next(void *p_context, STATUS status)
{
	TRACE_ENTRY(Read_Next);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;
	void            *p_page_frame;

    while (1)
    {
        if (p_test_context->m_page_number >= p_test_context->m_num_pages)
		{
			Post_Complete(p_test_context, NU_SUCCESS);
			return;
		}

	    status =  CM_Open_Page(p_test_context->m_cache_handle, 
		    p_test_context->m_page_number,
		    CM_OPEN_MODE_READ | CM_CALLBACK_IF_NO_FRAME | CM_CALLBACK_IF_LOCKED,
		    p_test_context,
		    &p_page_frame,
		    &p_test_context->m_page_handle);

	    switch (status)
	    {
		U32 data_read;
		case CM_ERROR_CACHE_MISS:

		    // We have a page frame
			// Read the page into this frame.
			p_test_context->Set_Callback(&Read_Complete);
			status = FB_Device_Read_Page(
				p_test_context->m_cache_handle == CT_cache_handle_0 ? 0 : 1, // controller number, 
				p_test_context->m_page_number,
				0, // U16 alternate_address,
				0, // U16 bad_cell_map,
				p_page_frame,
				p_test_context,
				CT_config.page_size);
			if (status != NU_SUCCESS)
		    {
				CT_Log_Error(CT_ERROR_TYPE_FATAL,
					"Read_Next", 
					"FB_Device_Read_Page failed",
					status,
					0);
				Post_Complete(p_test_context, status);
				return;
		    }
			// We will be called again when the read completes.
			// this time, the page will be present.
			return;
			break;

	    case NU_SUCCESS:

		    // We have a page frame
		    // Read and verify the number of the page
			data_read = *((U32 *)p_page_frame);
		    if (data_read != p_test_context->m_page_number)
			{
				CT_Log_Error(CT_ERROR_TYPE_FATAL,
					"Read_Next", 
					"Read compare failed",
					1,
					0);
				Post_Complete(p_test_context, 1);
				return;
			}

		    // Close the page
		    status = CM_Close_Page(p_test_context->m_cache_handle, 
				p_test_context->m_page_handle);
		    if (status != NU_SUCCESS)
		    {
				Post_Complete(p_test_context, status);
				return;
		    }
		    break;

        case CM_ERROR_NO_PAGE_FRAMES:

            // Read_Next will be rescheduled when page frame is available.
            return;
            break;

        case CM_ERROR_PAGE_LOCKED:
            // Read_Next will be rescheduled when the page has been unlocked
            return;
            break;

	    default:
			CT_Log_Error(CT_ERROR_TYPE_FATAL,
				"Read_Next", 
				"Unexpected error on read",
				status,
				0);
			Post_Complete(p_test_context, status);
			return;
		    break;


	    } // switch

        // Increment the page number.
        p_test_context->m_page_number++;

    } // while

} // Read_Next

/*************************************************************************/
// Read_Sequential
/*************************************************************************/
STATUS Read_Sequential(U32 page_number, U32 num_pages,
						WPARAM message, HWND hWnd)
{
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_test_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_test_context == 0)
        return NU_NO_MEMORY;

    p_test_context->m_page_number = page_number;
    p_test_context->m_num_pages = num_pages;
    p_test_context->m_message = message;
    p_test_context->m_hWnd = hWnd;
	p_test_context->m_cache_handle = CT_cache_handle_0;

	// Start the context at Read_Next
    p_test_context->Set_Callback(&Read_Next);
    p_test_context->Make_Ready();

    return NU_SUCCESS;
} // Read_Sequential

/*************************************************************************/
// We get called by the cache manager to write a page to storage.
/*************************************************************************/
STATUS Write_Callback(CM_CACHE_HANDLE cache_handle, 
	I64 page_number,
	void *p_page_frame,
	CM_PAGE_HANDLE page_handle)
{
	TRACE_ENTRY(Write_Callback);

	// Set the third word of the page frame to be the same as the first.
	U32 word_0 = *((U32 *)p_page_frame);
	*((U32 *)p_page_frame + 2) = word_0;

	// Create callback
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_test_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_test_context == 0)
        return NU_NO_MEMORY;

    // When context runs, Write_Complete will be called.
    p_test_context->Set_Callback(&Write_Complete);
    p_test_context->m_cache_handle = cache_handle;
    p_test_context->m_page_handle = page_handle;

    // Start write of page
    STATUS status = FB_Device_Write_Page(
        cache_handle == CT_cache_handle_0? 0 : 1, // controller number
	    (U32)page_number,
	    0, // U16 alternate_address,
	    0, // U16 bad_cell_map,
	    p_page_frame,
        p_test_context,
        CT_config.page_size);

	if (status != NU_SUCCESS)
		Callback_Context::Terminate(p_test_context, status);
    return status;
} // Write_Callback
	
/*************************************************************************/
// Called when writeback is complete.
/*************************************************************************/
void Write_Complete(void *p_context, STATUS status)
{
	TRACE_ENTRY(Write_Complete);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	// Check status of write operation
	if (status != NU_SUCCESS)
	{
		CM_Abort_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);
		Post_Complete(p_test_context, status);
		return;
	}
    status = CM_Close_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);
    Callback_Context::Terminate(p_context, status);

} // Write_Complete

/*************************************************************************/
// Called when write through is complete.
/*************************************************************************/
void Write_Through_Complete(void *p_context, STATUS status)
{
	TRACE_ENTRY(Write_Complete);

    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;

	// Check status of write operation
	if (status != NU_SUCCESS)
	{
		CM_Abort_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);

		// Save number of pages written in parent context
		((CM_Test_Context *)p_test_context->Get_Parent())->m_page_number =
			p_test_context->m_page_number;
		Callback_Context::Terminate(p_test_context, status);
		return;
	}
    status = CM_Close_Page(p_test_context->m_cache_handle, p_test_context->m_page_handle);

    // Increment the page number.
    p_test_context->m_page_number++;

	// Write the next page.
	Write_Next(p_context, status);

} // Write_Through_Complete

/*************************************************************************/
// Write_Next
/*************************************************************************/
void Write_Next(void *p_context, STATUS status)
{
    CM_Test_Context *p_test_context = (CM_Test_Context *)p_context;
	void            *p_page_frame;

	TRACE_ENTRY(Write_Next);

    while (1)
    {
        if (p_test_context->m_page_number >= p_test_context->m_num_pages)
		{
			// Save number of pages written in parent context
			((CM_Test_Context *)p_test_context->Get_Parent())->m_page_number =
				p_test_context->m_page_number;
			Callback_Context::Terminate(p_test_context, NU_SUCCESS);
			return;
		}

	    status =  CM_Open_Page(p_test_context->m_cache_handle, 
		    p_test_context->m_page_number,
		    CM_OPEN_MODE_WRITE | CM_CALLBACK_IF_NO_FRAME | CM_CALLBACK_IF_LOCKED,
		    p_test_context,
		    &p_page_frame,
		    &p_test_context->m_page_handle);

		// Save last status
		p_test_context->Set_Status(status);
	    switch (status)
	    {
	    case NU_SUCCESS:
	    case CM_ERROR_CACHE_MISS:

		    // We have a page frame to write into
		    // Write the number of the page
		    *((U32 *)p_page_frame) = p_test_context->m_page_number;

			// Set the second word of the page frame to be zero for cache 0
			// or 1 for cache 1.
			if (p_test_context->m_cache_handle == CT_cache_handle_0)
				*((U32 *)p_page_frame + 1) = 0;
			else
				*((U32 *)p_page_frame + 1) = 1;

			// Set the third word of the page frame to be zero.
			// We will copy the first word into the second when writeback is called.
			*((U32 *)p_page_frame + 2) = 0;

			if (CT_write_back == 0)
			{
				// This cache was opened in write through mode, so we must
				// write the page to the backing store before we close it.

				// When context runs, Write_Through_Complete will be called.
				p_test_context->Set_Callback(&Write_Through_Complete);

				// Start write of page
				STATUS status = FB_Device_Write_Page(
					p_test_context->m_cache_handle == CT_cache_handle_0? 0 : 1, // controller number
					(U32)p_test_context->m_page_number,
					0, // U16 alternate_address,
					0, // U16 bad_cell_map,
					p_page_frame,
					p_test_context,
					CT_config.page_size);

				if (status != NU_SUCCESS)
					Callback_Context::Terminate(p_test_context, status);

				// Return to scheduler.
				return;
			}

		    // Close the page
		    status = CM_Close_Page(p_test_context->m_cache_handle, 
				p_test_context->m_page_handle);
		    if (status != NU_SUCCESS)
		    {
				// Save number of pages written in parent context
				((CM_Test_Context *)p_test_context->Get_Parent())->m_page_number =
					p_test_context->m_page_number;
				Callback_Context::Terminate(p_test_context, status);
				return;
		    }
		    break;

        case CM_ERROR_NO_PAGE_FRAMES:

            // Write_Next will be rescheduled when page frame is available.
            return;
            break;

        case CM_ERROR_PAGE_LOCKED:

            // Write_Next will be rescheduled when the page has been unlocked
            return;
            break;

		case CM_ERROR_MAX_DIRTY_PAGES:

            // Write_Next will be rescheduled when the page becomes clean.
            return;
            break;

	    default:
			// Save number of pages written in parent context
			((CM_Test_Context *)p_test_context->Get_Parent())->m_page_number =
				p_test_context->m_page_number;
			Callback_Context::Terminate(p_test_context, status);
			return;
		    break;


	    } // switch

        // Increment the page number.
        p_test_context->m_page_number++;

    } // while

} // Write_Next

/*************************************************************************/
// Write_Sequential
/*************************************************************************/
STATUS Write_Sequential(U32 page_number, U32 num_pages,
						WPARAM message, HWND hWnd)
{
	// Create a parent context.
	CT_ASSERT((sizeof(CM_Test_Context) <= Callback_Context::Get_Max_Context_Size()), 
		Flush_Cache);
    CM_Test_Context *p_parent_context = 
		(CM_Test_Context *)Callback_Context::Allocate(sizeof(CM_Test_Context));
    if (p_parent_context == 0)
        return NU_NO_MEMORY;

    p_parent_context->m_message = message;
    p_parent_context->m_hWnd = hWnd;

	// Set up the parent context to call Post_Complete when it terminates.
	p_parent_context->Set_Callback(&Post_Complete);

	// Create a child context to do the write.
	// When it terminates, the parent will run.
    CM_Test_Context *p_test_context = 
		(CM_Test_Context *)p_parent_context->Allocate_Child(sizeof(CM_Test_Context));
    if (p_test_context == 0)
        return NU_NO_MEMORY;

    p_test_context->m_page_number = page_number;
    p_test_context->m_num_pages = num_pages;
	p_test_context->m_cache_handle = CT_cache_handle_0;

	// Start the context at Write_Next
    p_test_context->Set_Callback(&Write_Next);
    p_parent_context->Make_Children_Ready();

    return NU_SUCCESS;
} // Write_Sequential

