// DdmStress_win.cpp : Defines the entry point for the console application.

#include "stdio.h"
#include "Application_Initialize.h"
#include "Os.h"
#include "Kernel.h"
#include "TestDdm.h"
#include "windows.h"
#include "Odyssey_Trace.h"

void StartTask(VOID* pParam) {
	TestDdm::fStress = true;

	TraceLevel[TRACE_DDM_MGR]=0;
	TraceLevel[TRACE_DDM]=0;

	Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	Oos::Initialize();
}

int main(int argc, char* argv[]) {

	printf("calling application initialize\n");



	StartTask(NULL);

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "my_event");
	while(1) {
		WaitForSingleObject(hEvent, 1000);
	}

	return 0;
}
