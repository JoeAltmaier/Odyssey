// Main.cpp -- Demo Ddm Main
//
#ifndef _DEBUG
#define _DEBUG
#endif

#include "stdio.h"
#include "Application_Initialize.h"
#include "Os.h"
#include "Kernel.h"

#ifdef WIN32
	#include "windows.h"
#endif


#include "Odyssey_Trace.h" 

// extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void StartTask(VOID* pParam) {

	TraceLevel[TRACE_DDM_MGR]=0;
	TraceLevel[TRACE_DDM]=0;
	TraceLevel[TRACE_SYSTEMLOG]=9;

//	TraceToFile("c:\\trace.out");

	Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	Oos::Initialize();
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
