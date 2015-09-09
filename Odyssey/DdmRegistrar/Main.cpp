// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _DEBUG

#include "Oos.h"
#include "Odyssey_Trace.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
    Tracef("Start Task\n");	

	Oos::Initialize();
	
	Tracef("Here we go...\n");
}

