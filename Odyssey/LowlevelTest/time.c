/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: time.c
//
// Description:
// This file contains the Application_Initialize() routine, which is the
// main application level module and IOPMaster routine which Initializes the
// hardware and spawns the rest of the tasks for the system. 
//
//
// Careted by
// 10/06/98 Sudhir Kasargod:
/*************************************************************************/


/* Include necessary Nucleus PLUS files.  */

#include  "nucleus.h"
#include  "types.h"
#include  "rtc.h"



/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;


/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);



/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
void    Application_Initialize(void *first_available_memory)
{
    VOID    *pointer;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
#ifdef	INCLUDE_ODYSSEY
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 100000000, 56, NU_FIFO);
#else
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 200000, 56, NU_FIFO);
#endif
                        
    /* Create IOPMaster_Task */
    NU_Allocate_Memory(&System_Memory, &pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&IOPMaster_Task, "IOPMaster", IOPMaster, 0, NU_NULL, 
					pointer, 10000, 1, 20, NU_PREEMPT, NU_START);

}



void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int		  ch = 0;
	int		  i;
	unsigned long pci_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	printf("\n\r");
	printf("Executing the System Code....\n\r");
	/* Initilaize the Hardware */
#ifdef	INCLUDE_ODYSSEY
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
#else
	init_galileo();
#endif	INCLUDE_ODYSSEY

	/* Spawn the rest of the tasks and initialize */
	Init_System(&System_Memory);
	
	/* Lower the Priority of this task */
	NU_Change_Priority( (NU_TASK *)&IOPMaster_Task, 250 );
	
	/* Now this task will be active with lowest priority and can be used to
	 * any cleanup required
	 */
	for(;;){
		NU_Sleep(500);
		Cleanup_System();
		/* rlw: can we implement erc91 functionality here? */
	}
}


void
Init_System( NU_MEMORY_POOL  *Memory_Pool)
{
	VOID *Stack_Pointer;
	STATUS sts;

    /* Create IOPMaster_Task */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
					
	NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);


}

void
Cleanup_System()
{
}

extern int	rlen;
char *clrscn = "\033[H\033[2J\033[0m";
void
print_help()
{
	printf("%s", clrscn);
	printf("\033[01;07H*********************************************************************");
	printf("\033[02;15HConvergeNet Technologies - Real Time Clock Demo");
	printf("\033[03;07H*********************************************************************");
	printf("\033[05;18Hs) Set Time");
	printf("\033[06;18Hg) Get Time");
	printf("\n\r\n\r");
}
time_t	gtime, stime;
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	int	i=0, j, k;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 's':
			case 'S':
				stime.year  = 99;
				stime.month = 8;
				stime.date  = 27;
				stime.day   = 6;
				stime.hour  = 16;
				stime.minutes  = 10;
				stime.seconds  = 5;
				stime.timezone = 0x01020304;
				stime.dstflag  = 0x05060708;
				SetTime(&stime);
				break;
			case 'g':
			case 'G':
				GetTime(&gtime);
				printf("Year %d, Month %d, Date %d, Day %d\n\r", 
						gtime.year, gtime.month, gtime.date, gtime.day);
				printf("Hour %d, Minutes %d, Seconds %d\n\r",
						gtime.hour, gtime.minutes, gtime.seconds);
				printf("Timezone %08X, DstFlag %08X\n\r", 
						gtime.timezone, gtime.dstflag);
				break;
			case ' ':
				print_help();
				break;
			default:
				printf("%c", ch);
				break;
			}
		}
		
		NU_Sleep(10);
	}
}

char *Day[8] = { "",
				 "Sun",
				 "Mon",
				 "Tue",
				 "Wed",
				 "Thu",
				 "Fri",
				 "Sat"
				};
				
char *Month[13] = { "",
					"Jan",
					"Feb",
					"Mar",
					"Apr",
					"May",
					"Jun",
					"Jul",
					"Aug",
					"Sep",
					"Oct",
					"Nov",
					"Dec"
					};
void   
test_1(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	int	i=0, j, k;
	time_t ltime;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
    while(1) {
    	GetTime(&ltime);
    	if ( ltime.month > 12 || ltime.day > 7) {
    		printf("Error: Out of range\n\r");
    		continue;
    	}
    	if ( ltime.hour > 12 ) 
    		printf("\033[15;07H%s %s %02d %02d:%02d:%02d PM 19%02d\r", 
    			Day[ltime.day], Month[ltime.month], ltime.date,
    			ltime.hour-12, ltime.minutes, ltime.seconds, ltime.year);
    	else
    		printf("\033[15;07H%s %s %02d %02d:%02d:%02d AM 19%02d\r", 
    			Day[ltime.day], Month[ltime.month], ltime.date,
    			ltime.hour, ltime.minutes, ltime.seconds, ltime.year);
    		
    	NU_Sleep(1);
    	
    }
}
