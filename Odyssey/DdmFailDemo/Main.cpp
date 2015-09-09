// Main.cpp -- Demo Ddm Main
//
// This Demo requires that Interrupt 0 on your eval CPU
// be jumpered for serial interrupts.
//
#define _TRACEF
#include "stdio.h"
#include "Os.h"
#include "Odyssey_Trace.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{ 
TraceLevel[TRACE_DDM_MGR] = 7; //TRACE_ALL_LVL;
//TraceLevel[TRACE_DDM ] 	  = TRACE_ALL_LVL;
//TraceLevel[TRACE_HEAP1] = 1;
TraceLevel[TRACE_FAILOVER] = TRACE_ALL_LVL;
TraceLevel[TRACE_VIRTUAL_MGR] = TRACE_ALL_LVL;
TraceLevel[TRACE_PTSPROXY] = TRACE_ALL_LVL; 
TraceLevel[TRACE_PTSLOADER] = TRACE_ALL_LVL; 

    Tracef("Start Task\n");	
	Tracef("Hit key to start...");
	getchar();

	Oos::DumpTables("In StartTask...");
	Oos::Initialize();

	Tracef("Here we go...\n");

	/* Task0 forever loop */
	while(1) {
		switch (getchar()) {
        case 0x08:  /* BACKSPACE */
        case 0x09:  /* TAB */
        case 0x0D:  /* ENTER */
        case 0x1B:  /* ESC */
            printf(" \007");
            break;

		case 'x':
			printf("\n**\n** CRASH **\n**\n");
			for(;;) ;
			break;
        default:
//          printf("%c", key);
            break;

	    }  /* switch (key = Get_Char()) */
	        		
		NU_Sleep(1);
	}
}

