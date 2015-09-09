// Main.cpp -- Test Ddm Main
//

//#define _DEBUG

#include "Os.h"
#include "Odyssey_trace.h"


extern "C" void  StartTask(UNSIGNED argc, VOID *argv);


void  StartTask(UNSIGNED , VOID *)
{


	Os::Initialize();

}

