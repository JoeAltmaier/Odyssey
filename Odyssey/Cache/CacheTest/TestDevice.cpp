/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FbTestDevice.cpp
// 
// Description:
// This file simulates the flash device for the Flash Block. 
// 
// Update Log 
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "ErrorLog.h"
#include "Simple.h"
#include <String.h>
#include <afxmt.h>
#include <Windows.h>
#include "Callback.h"
#include "CacheTestData.h"
 
/*************************************************************************/
// Device globals
/*************************************************************************/
HANDLE file_handle_0;
HANDLE file_handle_1;
HANDLE hMutex_0;
HANDLE hMutex_1;
void *p_erase_buffer;
	

/*************************************************************************/
// Device_Write_Page
// Write page from flash buffer
/*************************************************************************/
STATUS FB_Device_Write_Page(U32 controller_number, 
	U32 real_page_number,
	U16 alternate_address,
	U16 bad_cell_map,
	void *p_flash_buffer,
    Callback_Context *p_callback_context,
    U32 page_size)
{
	STATUS status = CT_asyncIO.Do_Async_IO(
		controller_number? hMutex_1 : hMutex_0 , 
		controller_number? file_handle_1 : file_handle_0 , 
		real_page_number * page_size,
		p_flash_buffer, 
		page_size, 
		p_callback_context,
		WinAsyncIO::WRITE
	   );
	return status;

}  // FB_Device_Write_Page
	
/*************************************************************************/
// Device_Read_Page
// Read page into flash buffer
/*************************************************************************/
STATUS FB_Device_Read_Page(U32 controller_number, 
	U32 real_page_number,
	U16 alternate_address,
	U16 bad_cell_map,
	void *p_flash_buffer,
    Callback_Context *p_callback_context,
    U32 page_size)
{
	STATUS status = CT_asyncIO.Do_Async_IO(
		controller_number? hMutex_1 : hMutex_0 , 
		controller_number? file_handle_1 : file_handle_0 , 
		real_page_number * page_size,
		p_flash_buffer, 
		page_size, 
		p_callback_context,
		WinAsyncIO::READ
	   );
	return status;
}  // FB_Device_Read_Page
	
/*************************************************************************/
// Close the file for each controller.
/*************************************************************************/
STATUS Device_Close()
{
	BOOL success = CloseHandle(file_handle_0);

	if (success == 0)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Close", 
			"CloseHandle failed",
			erc,
			0);
		return erc;
	}

	success = CloseHandle(file_handle_1);

	if (success == 0)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Close", 
			"CloseHandle failed",
			erc,
			0);
		return erc;
	}

	CT_asyncIO.Stop();
	return NU_SUCCESS;
} // Device_Close

/*************************************************************************/
// Create a file for each controller.
/*************************************************************************/
STATUS Device_Open(const char *p_file_name)
{
	char file_name[100];
	file_name[0] = 0;
	strcat(file_name, p_file_name);
	strcat(file_name, "0");

	file_handle_0 = CreateFile(
		file_name, // LPCTSTR lpFileName,	// pointer to name of the file 
		GENERIC_READ | GENERIC_WRITE, // DWORD dwDesiredAccess,	// access (read-write) mode 

		// If dwShareMode is 0, the file cannot be shared. No other open operations 
		// can be performed on the file.
		0, // DWORD dwShareMode,	// share mode 

		// If lpSecurityAttributes is NULL, the handle cannot be inherited. 
		0, // LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to security descriptor 

		// Opens the file, if it exists. If the file does not exist, the function creates
		// the file as if dwCreationDistribution were CREATE_NEW.
		OPEN_ALWAYS, // DWORD dwCreationDistribution,	// how to create 
		FILE_FLAG_RANDOM_ACCESS, // DWORD dwFlagsAndAttributes,	// file attributes 
		NULL // HANDLE hTemplateFile 	// handle to file with attributes to copy  
	   );

	if (file_handle_0 == INVALID_HANDLE_VALUE)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Open", 
			"CreateFile failed",
			erc,
			0);
		return erc;
	}

	file_name[0] = 0;
	strcat(file_name, p_file_name);
	strcat(file_name, "1");

	// Create a mutex for file 0
	hMutex_0 = CreateMutex(
		NULL, // LPSECURITY_ATTRIBUTES lpEventAttributes,	// pointer to security attributes
		
		// If TRUE, the calling thread requests immediate ownership of the mutex object. 
		// Otherwise, the mutex is not owned. 

		0, // flag for initial ownership 
		// If lpName is NULL, the event object is created without a name. 
		NULL // LPCTSTR lpName 	// pointer to semaphore-object name  
	   );

	if (hMutex_0 == 0)
	{

		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Open", 
			"CreateMutex failed",
			erc,
			0);
		return erc;
	}

	file_handle_1 = CreateFile(
		file_name, // LPCTSTR lpFileName,	// pointer to name of the file 
		GENERIC_READ | GENERIC_WRITE, // DWORD dwDesiredAccess,	// access (read-write) mode 

		// If dwShareMode is 0, the file cannot be shared. No other open operations 
		// can be performed on the file.
		0, // DWORD dwShareMode,	// share mode 

		// If lpSecurityAttributes is NULL, the handle cannot be inherited. 
		0, // LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to security descriptor 

		// Opens the file, if it exists. If the file does not exist, the function creates
		// the file as if dwCreationDistribution were CREATE_NEW.
		OPEN_ALWAYS, // DWORD dwCreationDistribution,	// how to create 
		FILE_FLAG_RANDOM_ACCESS, // DWORD dwFlagsAndAttributes,	// file attributes 
		NULL // HANDLE hTemplateFile 	// handle to file with attributes to copy  
	   );

	if (file_handle_1 == INVALID_HANDLE_VALUE)
	{
		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Open", 
			"CreateFile failed",
			erc,
			0);
		return erc;
	}

	// Create a mutex for file 1
	hMutex_1 = CreateMutex(
		NULL, // LPSECURITY_ATTRIBUTES lpEventAttributes,	// pointer to security attributes
		
		// If TRUE, the calling thread requests immediate ownership of the mutex object. 
		// Otherwise, the mutex is not owned. 

		0, // flag for initial ownership 
		// If lpName is NULL, the event object is created without a name. 
		NULL // LPCTSTR lpName 	// pointer to semaphore-object name  
	   );

	if (hMutex_1 == 0)
	{

		DWORD erc = GetLastError();
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Device_Open", 
			"CreateMutex failed",
			erc,
			0);
		return erc;
	}

	// Allocate memory for AsyncIO object.
	void *p_memory = malloc(1000);

	// Initialize AsyncIO object
	CT_asyncIO.Initialize(4, // num_worker_threads
		p_memory, 1000); 
	return NU_SUCCESS;

} // Device_Open

