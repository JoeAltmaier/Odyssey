// Main.cpp -- Demo Ddm Main
//
#ifndef _DEBUG
#define _DEBUG
#endif

#include "stdio.h"
#include "Application_Initialize.h"
#include "Os.h"
#include "Kernel.h"
#include "Cache.h"
#include "FlashStorage.h"
#include "FlashAddress.h"
#ifdef WIN32
	#include "windows.h"
#endif

#include "CTtypes.h"
#include "Odyssey_Trace.h" 
#include "callback.h"
// extern "C" void  StartTask(UNSIGNED argc, VOID *argv);
#define MEMORY_FOR_CALLBACKS 100000
extern	"C" int ttyA_in();

void StartTask(VOID* pParam) {


	TraceLevel[TRACE_DDM_MGR]=0;
	TraceLevel[TRACE_DDM]=0;
	TraceLevel[TRACE_SSD] = 2;
	TraceLevel[TRACE_SSD1] = 2;
	TraceLevel[TRACE_CHAOSFILE] = 9;
	TraceLevel[TRACE_PARTITIONMGR] = 9;
//	TraceToFile("c:\\trace.out");

	Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	while (ttyA_in() != ' ')
		NU_Sleep(25);
	Oos::Initialize();
}

extern CM_CONFIG cacheConfig;
extern FF_CONFIG flashConfig;


int main(int argc, char* argv[]) {

//	flash_error_test.Allocate();		
	
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
