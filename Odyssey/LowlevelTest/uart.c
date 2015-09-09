/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: init.c
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
#define	PLUS	1

#include  "nucleus.h"
#include  "types.h"
#include  "hw.h"
#include  "tty.h"
#include  "pcimap.h"
#include  "pcidev.h"
#include  "pcialloc.h"
#include  "system.h"


/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;
NU_TASK         Test_Task3;


//#define	CMB_HACK
/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);
void    test_3(UNSIGNED argc, VOID *argv);


extern	U32 r7k_get_ic();
extern	void r7k_set_ic(U32 val);
extern	U32 r7k_get_18();
extern	U32 r7k_get_19();


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
                        first_available_memory, 2000000, 56, NU_FIFO);
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
	/* Initialize LISRs for Unhandled Interrupts */
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


    /* Create Test_Task 0 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
    /* Create Test_Task 1 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task 2 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task2, "Test2", test_2, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
    /* Create Test_Task 3 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task3, "Test3", test_3, 0, NU_NULL, 
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
	printf("\033[02;15HConvergeNet Technologies - UART Demonstration"); 
	printf("\033[03;07H*********************************************************************");
	printf("\n\r\n\r");
}



extern	INT TCD_Interrupt_Level;
extern	INT TCD_R7KInterrupt_Level;
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	int	i=0, j, k;
	U32	padr;
	U32 *ptr;
	int bus_num = 0;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	
#define	CPORT	3
	print_help();
	printf("CS0 %08X\n\r", gt_read(0x45C));
// Default
//	gt_write(0x45C, 0x3C4797FF);
// TOff
//	gt_write(0x45C, 0x3C479447);

// AccToFirst - F
//	gt_write(0x45C, 0x3C47947B);
// AccToFirst - 10
//	gt_write(0x45C, 0x3C479453);
// AccToFirst - 9
	gt_write(0x45C, 0x3C47944B);
// AccToFirst - 8
//	gt_write(0x45C, 0x3C479443);
// AccToFirst - 7
//	gt_write(0x45C, 0x3C47943B);



// AccToFirst - 8, 7
//	gt_write(0x45C, 0x3C479447);
// AccToFirst - 9, 7, 0
//	gt_write(0x45C, 0x3C47904F);
	ttyinit(0, 115200);
	ttyinit(2, 115200);
	ttyinit(3, 115200);
	ttyinit(4, 115200);
	ttyioctl(CPORT, FIOCINT,TTYRX);
	for(;;){
		if ( ttyhit(0)) {
			ch = ttyin(0);
			ttyout(0, ch);
		}
		if ( ttyhit(2)) {
			ch = ttyin(2);
			ttyout(2, ch);
		}
		
		if ( ttyhit(3)) {
			ch = ttyin(3);
			ttyout(3, ch);
		}
		if ( ttyhit(4)) {
			ch = ttyin(4);
			ttyout(4, ch);
		}
	}
}
void   
test_1(UNSIGNED argc, VOID *argv)
{
    STATUS    		status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		NU_Sleep(100);
//		ttyout(CPORT, '.');
	}
	
}



void   
test_2(UNSIGNED argc, VOID *argv)
{
    STATUS    		status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		NU_Sleep(100);
	}
	
}

void   
test_3(UNSIGNED argc, VOID *argv)
{
    STATUS    		status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		NU_Sleep(100);
	}
	
}

