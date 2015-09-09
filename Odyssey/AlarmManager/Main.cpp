// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//

// $Log: /Gemini/Odyssey/AlarmManager/Main.cpp $
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
TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTS1] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTS_LISTEN] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTS_LISTEN1] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTSDEFAULT] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTSDEFAULT1] = TRACE_ALL_LVL;

TraceLevel[TRACE_ALARM] = TRACE_ALL_LVL;

    Tracef("Start Task\n");	

	Oos::DumpTables("In StartTask...");
	Oos::Initialize();

//	TraceToFile("c:\\trace.txt");

	Tracef("Here we go...\n");
}

void QuitTest() {

	static int i=0;
	i++;
	Tracef("QuitTest i = %d\n", i);
	if (i==2)
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