// SSAPI_Server.cpp : Defines the entry point for the console application.

#include "stdio.h"
#include "Application_Initialize.h"
#include "Os.h"
#include "Kernel.h"
#include "windows.h"
#include "Odyssey_Trace.h"

void StartTask(VOID* pParam) {

	TraceLevel[TRACE_DDM_MGR]=TRACE_OFF_LVL;
	TraceLevel[TRACE_DDM]=TRACE_OFF_LVL;
	TraceLevel[TRACE_SSAPI_OBJECTS] = TRACE_L2;
	TraceLevel[TRACE_SSAPI_MANAGERS] = TRACE_L2;

	//TraceToFile("c:\\trace.out");

	Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	Oos::Initialize();
}

int main(int argc, char* argv[]) {

	StartTask(NULL);

	while(1) {
		Sleep(5000);
	}

	return 0;
}
