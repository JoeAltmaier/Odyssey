// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//

// $Log: /Gemini/Odyssey/UpgradeMasterTest/Main.cpp $
// 
// 4     11/17/99 3:28p Joehler
// add command queues to upgrade master
// 
// 3     10/12/99 10:25a Joehler
// 
// 2     10/07/99 9:59a Joehler
// modified testing of default images and add image
// 
// 1     9/30/99 7:50a Joehler
// First cut of Upgrade Master test driver
// 
// 5     9/02/99 11:46a Joehler
// added comments

#define _TRACEF
#include "Odyssey_Trace.h"

#include "Os.h"
#include "windows.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void StartTask(VOID* pParam)
{
//TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;
//TraceLevel[TRACE_PTS1] = TRACE_ALL_LVL;
//TraceLevel[TRACE_PTS_LISTEN] = TRACE_ALL_LVL;
//TraceLevel[TRACE_PTS_LISTEN1] = TRACE_ALL_LVL;
//TraceLevel[TRACE_PTSDEFAULT] = TRACE_ALL_LVL;

	//TraceLevel[TRACE_HEAP] = TRACE_ALL_LVL;
	TraceLevel[TRACE_UPGRADE] = TRACE_ALL_LVL;
	TraceLevel[TRACE_FILESYS] = TRACE_ALL_LVL;

    Tracef("Start Task\n");	

	Oos::DumpTables("In StartTask...");
	Oos::Initialize();

	TraceToFile("c:\\trace.txt");

	Tracef("Here we go...\n");
}

void QuitTest() {

	static int i=0;
	i++;
	Tracef("QuitTest i = %d\n", i);
	if (i==1)
	{
		Sleep(1000);
		exit(0);
	}
}

int main(int argc, char* argv[]) {

	StartTask(NULL);

	while(1) {
		Sleep(5000);
	}

	return 0;
}