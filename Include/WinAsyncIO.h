/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: WinAsyncIO.h
// 
// Description:
// WinAsyncIO class provides asynchronous IO for Windows applications 
// 
// 
// Update Log 
// 9/19/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(WinAsyncIO_H)
#define WinAsyncIO_H

#include "Callback.h"
#include "Nucleus.h"
#include <Windows.h>

/*************************************************************************/
// WinAsyncIO
/*************************************************************************/
class WinAsyncIO
{
public:

	typedef enum {
		READ,
		WRITE
	} Async_IO_Type;

	// Initialize WinAsyncIO object.
	// num_worker_threads is the number of worker threads to allocate
	// to perform IO operations.
	STATUS Initialize(UNSIGNED num_worker_threads,
							  void *p_memory, UNSIGNED size_memory);

	STATUS Do_Async_IO(
		HANDLE hMutex, // handle of file's mutex
		HANDLE hFile, // handle of file 
		UNSIGNED offset, // offset in file 
		void *p_buffer, // pointer to data 
		UNSIGNED num_bytes, // number of bytes 
		Callback_Context *p_callback_context, // context to run when I/O is complete
		Async_IO_Type async_IO_type
		); 

	void Stop();

private: // member data

	static DWORD __stdcall WinAsyncIO::Async_IO_Thread(LPVOID lpParameter);

	HANDLE		m_h_semaphore;

	// Set when it's time to stop 
	int			m_stop;

	UNSIGNED	m_num_worker_threads;

	// List of I/O control blocks waiting to be executed.
	LIST		m_IOCB_list;

	// IOCBs are allocated from this list.
	LIST		m_list_avail;

	// Critical section to protect the list
	CRITICAL_SECTION	m_critical_section;

}; // WinAsyncIO

#endif //   WinAsyncIO_H
