// Main.cpp -- HBC Image Main
//
// This code requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _DEBUG

#include "Os.h"
#include "Odyssey_Trace.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
    Tracef("Start TasK\n");	

   //  enable heap debugging, when using MSL-ISA3-noFPU-BE-ADBG.lib (phew!)
#ifdef _DEBUG
   TraceLevel[TRACE_HEAP1] = 1;
#endif   

	Os::Initialize();
	
	Tracef("Here we go...\n");

}

