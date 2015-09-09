/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FbTestSemaphore.cpp
// 
// Description:
// This file simulates the semaphore for the Flash Block. 
// 
// Update Log 
// 
// 9/3/98 Jim Frandeen: Create file
/*************************************************************************/

#include "Simple.h"
#include "Nucleus.h"
#include <Windows.h>

class Test_Semaphore
{
public:
	HANDLE semaphore_handle;
};

/*************************************************************************/
// SMC_Create_Semaphore
// with NU_NO_ERROR_CHECKING
/*************************************************************************/
STATUS  SMC_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name, 
                           U32 initial_count, OPTION suspend_type)
{
	Test_Semaphore *p_test_semaphore = (Test_Semaphore *)semaphore_ptr;
 
	HANDLE semaphore_handle = CreateSemaphore(
		NULL, // LPSECURITY_ATTRIBUTES lpEventAttributes,	// pointer to security attributes
		
		// The state of a semaphore is signaled when its count is greater than 
		// zero and nonsignaled when it is zero. The count is decreased by one 
		// whenever a wait function releases a thread that was waiting for the semaphore. 
		// The count is increased by a specified amount by calling the ReleaseSemaphore function.
		0, // lInitialCount

		1, // lMaximumCount
		
		// If lpName is NULL, the event object is created without a name. 
		NULL // LPCTSTR lpName 	// pointer to semaphore-object name  
	   );

	// Save semaphore handle in semaphore
	p_test_semaphore->semaphore_handle = semaphore_handle;

	if (semaphore_handle != 0)
		return NU_SUCCESS;

	DWORD erc = GetLastError();
	return erc;


}  // SMC_Create_Semaphore

/*************************************************************************/
// SMCE_Create_Semaphore
// SMCE_Create_Semaphore with error checking
/*************************************************************************/
STATUS  SMCE_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name, 
                           U32 initial_count, OPTION suspend_type)
{
	return SMC_Create_Semaphore(semaphore_ptr, name, initial_count, suspend_type);
}

/*************************************************************************/
// SMC_Obtain_Semaphore
// with NU_NO_ERROR_CHECKING
/*************************************************************************/
STATUS  SMC_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr,  U32 suspend)
{
	Test_Semaphore *p_test_semaphore = (Test_Semaphore *)semaphore_ptr;

	DWORD erc = WaitForSingleObject(
		p_test_semaphore->semaphore_handle, // HANDLE hHandle,	// handle of object to wait for
		
		// If dwMilliseconds is INFINITE, the function's time-out interval never elapses. 
		INFINITE // DWORD dwMilliseconds 	// time-out interval in milliseconds  
	   );

	if (erc != WAIT_FAILED)
		return NU_SUCCESS;

	erc = GetLastError();
	return erc;

} // SMC_Obtain_Semaphore

/*************************************************************************/
// SMCE_Obtain_Semaphore
// with error checking
/*************************************************************************/
STATUS  SMCE_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr,  U32 suspend)
{
	return SMC_Obtain_Semaphore(semaphore_ptr, suspend);
}

/*************************************************************************/
// SMC_Release_Semaphore
// with NU_NO_ERROR_CHECKING
/*************************************************************************/
STATUS  SMC_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{
	Test_Semaphore *p_test_semaphore = (Test_Semaphore *)semaphore_ptr;

	BOOL success = ReleaseSemaphore(
		p_test_semaphore->semaphore_handle, // handle of the semaphore object
		1, // amount to add to current count 
		NULL // address of previous count 
		);	
	if (success)
		return NU_SUCCESS;

	DWORD erc = GetLastError();
	return erc;

} // SMC_Release_Semaphore

/*************************************************************************/
// SMCE_Release_Semaphore
// with error checking
/*************************************************************************/
STATUS  SMCE_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{
	return SMC_Release_Semaphore(semaphore_ptr);
}
