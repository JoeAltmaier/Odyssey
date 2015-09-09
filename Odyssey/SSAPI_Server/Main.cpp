// Main.cpp -- SSAPI Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "Os.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *) {

#ifdef _DEBUG
	TraceLevel[TRACE_OOS]=0;
	TraceLevel[TRACE_DDM_MGR]=0;
	TraceLevel[TRACE_DDM]=0;
	TraceLevel[TRACE_HEAP1]=0;
	TraceLevel[TRACE_TRANSPORT]=0;
#endif

	//Oos::DumpTables("\n----------------------\nIn StartTask...\n--------------------------\n");
	Oos::Initialize();
}

