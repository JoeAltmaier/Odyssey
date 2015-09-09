// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _TRACEF
#include "Odyssey_Trace.h"

#include "Os.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
//TraceLevel[TRACE_DDM_MGR] = TRACE_ALL_LVL;
//TraceLevel[TRACE_DDM ] 	  = TRACE_ALL_LVL;
TraceLevel[TRACE_HEAP1] = 1;

    Tracef("Start Task\n");	

	Oos::DumpTables("In StartTask...");
	Oos::Initialize();

	Tracef("Here we go...\n");
}

