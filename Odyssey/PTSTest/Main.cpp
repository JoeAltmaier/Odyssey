// Main.cpp -- Demo Ddm Main.  Used to start up CHAOS [OOS] execution.
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
//
// $Log: /Gemini/Odyssey/PTSTest/Main.cpp $
// 
// 4     5/13/99 11:37a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 3     3/01/99 7:43p Ewedel
// Rolled in oos.h -> os.h change.
//

#define _DEBUG

#include "Os.h"
#include "Odyssey_Trace.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{
    Tracef("Start TasK\n");	

	Oos::Initialize();
	
	
	Tracef("Here we go...\n");

}

