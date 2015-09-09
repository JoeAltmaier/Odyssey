// Main.cpp -- Demo Ddm Main.  Used to start up CHAOS [OOS] execution.
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
//
// $Log: /Gemini/Odyssey/Testddm/Main.cpp $
// 
// 3     5/13/99 11:43a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 2     5/04/99 1:49p Tnelson
// 
// 1     5/03/99 3:30p Legger
// Created by Lee Egger 
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

	Os::DumpTables("BuildSys Tables");
	
	Os::Initialize();
}

