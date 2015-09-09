// Main.cpp -- Demo Ddm Main
//
#define _TRACEF
#include "stdio.h"
#include "Os.h"
#include "Odyssey_Trace.h"
#include "RqOsStatus.h"

#include "DdmPongMaster.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{ 
TraceLevel[TRACE_DDM_MGR] = 3; //TRACE_ALL_LVL;
//TraceLevel[TRACE_DDM ] 	  = TRACE_ALL_LVL;
//TraceLevel[TRACE_HEAP1] = 1;
TraceLevel[TRACE_FAILOVER] = TRACE_ALL_LVL;
TraceLevel[TRACE_VIRTUAL_MGR] = 2;//TRACE_ALL_LVL;
TraceLevel[TRACE_VIRTUAL_MSTR] = 2;//TRACE_ALL_LVL;
TraceLevel[TRACE_PTSLOADER] = 2;//TRACE_ALL_LVL; 

    Tracef("Start Task\n");	
	Tracef("Hit key to start...");
	getchar();

	Os::DumpTables("In StartTask...");
	
	Os::Initialize();

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

		case 'i':	// Dump IOPStatusTable
			RqOsStatusDumpIst::Invoke();
			break;
			
		case 'v':	// Dump VirtualDeviceTable
			RqOsStatusDumpVdt::Invoke();
			break;
		
		case 'd':	// Dump Did Activity (this slot)
			RqOsStatusDumpDidActivity::Invoke();
			break;
			
		case 'a':	// Dump Did Activity (all slots)
			RqOsStatusDumpAllDidActivity::Invoke();
			break;
			
		case 'k': 	// Kill DdmPongSlave
			printf("\nKilling DdmPongSlave\n");
			DdmPongMaster::KillSlave();
			break;
			
		case 'l': 	// Kill DdmDemo 
			printf("\nKilling DdmPongSlave\n");
			DdmPongMaster::KillLocalSlave();
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

