// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _DEBUG

#include "Os.h"
#include "Trace.h"



extern "C" void  StartTask(UNSIGNED argc, VOID *argv);



void  StartTask(UNSIGNED , VOID *)
{


	Os::Initialize();

}

