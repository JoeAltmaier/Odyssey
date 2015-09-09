// Main.cpp -- Demo Main
//
#define _TRACEF
#include "Odyssey_Trace.h"

#include "stdio.h"
#include "Os.h"
#include "Kernel.h"

extern "C" void  StartTask(UNSIGNED argc, VOID *argv);

void  StartTask(UNSIGNED , VOID *)
{ 
    Tracef("Chaos Demo #3 (IOP)\n");	

#if 0
	Tracef("Hit key to start...");
	getchar();
#endif

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

        default:
//          printf("%c", key);
            break;

	    }
	        		
		Kernel::Delay(100);
	}
}

