/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: CmTest.h
// 
// Description:
// This file defines the interface to the Cache Manager test methods. 
// 
// Update Log 
// 
// 7/6/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(CmTest_H)
#define CmTest_H

#include <afx.h>
#include "Simple.h"
#include "Cache.h"
#include "Callback.h"

/*************************************************************************/
// CM_Test_Context
// This is the callback context used by the test methods.
/*************************************************************************/
class CM_Test_Context : public Callback_Context
{
public: // member data

    U32					 m_page_number;
    U32					 m_num_pages;
	U32					 m_num_threads;
    CM_CACHE_HANDLE      m_cache_handle;
    CM_PAGE_HANDLE       m_page_handle;
    CM_PAGE_HANDLE       m_page_handle_1;
	CM_CONFIG			*m_p_cache_config;
	WPARAM				 m_message;
	HWND				 m_hWnd;
	void				*m_p_page_frame_0;
	void				*m_p_page_frame_1;
	U32					 m_value;
	U32					 m_verify;


}; // CM_Test_Context

/*************************************************************************/
// Test methods
/*************************************************************************/
STATUS Close_Cache(WPARAM message, HWND window);
STATUS Device_Close();
STATUS Device_Open(const char *p_file_name);
void Format_Cache_Stats(char *string, CM_CONFIG *p_config, 
						CM_EVENT_DATA *p_event_data,
						CM_STATISTICS *p_statistics);
STATUS Flush_Cache(WPARAM message, HWND hWnd);
STATUS Get_Cache_Stats(CString *string, U32 cache_number);
STATUS Initialize_Test_Cache(CM_CONFIG *p_cache_config, 
							 U32 memory_size_to_allocate,
							 int is_write_back);
void Post_Complete(void *p_context, STATUS status);
void Read_Complete(void *p_context, STATUS status);
void Read_Next(void *p_context, STATUS status);
STATUS Stress_Test_Cache(CM_CONFIG *p_cache_config, 
	U32 memory_size_to_allocate, U32 num_threads,
	WPARAM message, HWND window);
DWORD __stdcall Thread_Start_Routine(LPVOID lpParameter);
STATUS Write_Callback(CM_CACHE_HANDLE cache_handle, 
	I64 page_number,
	void *p_page_frame,
	CM_PAGE_HANDLE page_handle);
void Write_Complete(void *p_context, STATUS status);
void Write_Next(void *p_context, STATUS status);
STATUS Write_Sequential(U32 page_number, U32 num_pages,
						WPARAM message, HWND window);


STATUS Read_Sequential(U32 page_number, U32 num_pages,
						WPARAM message, HWND window);

extern U32 stop_stress_test;

#endif // CmTest_H