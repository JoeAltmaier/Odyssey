/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmRaidMaster_Main.cpp
// 
// Description:
// Main for the Rmstr DDM
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/DdmRaidMaster_Main.cpp $
// 
// 4     8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 3     8/16/99 7:04p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 2     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

// DDMRaidMaster_Main.cpp : Defines the entry point for the console application.

#include "stdio.h"
#include "Application_Initialize.h"
#include "Os.h"
#include "Kernel.h"
#include "Odyssey_Trace.h"
#include "signal.h"
#include "stdio.h"

// Resolve
#ifdef WIN32
#include "windows.h"
#include <crtdbg.h>
#endif

void signalHandler(int sig);
void StartTask(VOID* pParam) {
	//TraceLevel[TRACE_ALARM] = TRACE_ALARM;
	TraceLevel[TRACE_RMSTR] = TRACE_RMSTR;
	TraceLevel[TRACE_RMSTR_1] = TRACE_RMSTR_1;
	Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	Oos::Initialize();
}


void OtherThread(void* pParam) {
	CT_Semaphore m_Semaphore = *((CT_Semaphore*)(pParam));

	while(1) {
		Tracef("OtherThread blocking semaphore\n");
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);
		Tracef("OtherThread obtained semaphore\n");
	}

}

int main(int argc, char* argv[]) {
#if 0
	CT_Semaphore m_Semaphore;
	Kernel::Create_Semaphore(&m_Semaphore, "test_semaphore", 0);
	CT_Task task;
	Kernel::Create_Thread(task,"my_thread",OtherThread, 0, &m_Semaphore,NULL,0);

	for(int i=0;i<3;i++) {
		Kernel::Sleep(10000);
		Kernel::Release_Semaphore(&m_Semaphore);
	}
#else
	signal(SIGTERM, &signalHandler);
	signal(SIGINT, &signalHandler);
#ifdef WIN32
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn On (OR) - 
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Set the new state for the flag
	_CrtSetDbgFlag( tmpFlag );
	tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif
	StartTask(NULL);
#endif

#ifdef WIN32
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "my_event");
	while(1) {
		WaitForSingleObject(hEvent, 5000);
	}
#endif
	return 0;
}

void signalHandler(int sig)
{
	sig = sig;
}

/*
int PASCAL WinMain(HANDLE h, HANDLE h1, LPSTR str, int x)
{
	main(1,NULL);
}
*/

