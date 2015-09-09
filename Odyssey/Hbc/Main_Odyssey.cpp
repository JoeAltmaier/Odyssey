// Main.cpp -- HBC Image Main
//
// This code requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _DEBUG

#include "Os.h"
#include "Odyssey_Trace.h"
#include "pcimap.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
    Tracef("Start TasK\n");	

	// Call Drivers code to initialize the Bridge Chips and PCI bus.
	Init_Hardware();	

   //  enable heap debugging, when using MSL-ISA3-noFPU-BE-ADBG.lib (phew!)
#ifdef _DEBUG
   TraceLevel[TRACE_HEAP1] = 1;
//   TraceLevel[TRACE_DDM_MGR] = 1;
 //  TraceLevel[TRACE_DDM] = 1;
#endif   
	
	Os::Initialize();
	
	Tracef("Here we go...\n");

}

