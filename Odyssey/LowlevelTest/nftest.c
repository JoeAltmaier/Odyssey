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

#include  "nucleus.h"
#include  "types.h"
#include  "tty.h"
#include  "sysflash.h"
#include  "hw.h"
#include  "pcimap.h"
#include  "pcidev.h"
#include  "gt.h"
#include  "pcialloc.h"
#include  "que.h"
#include  "nflash.h"
#include  "common.h"
#include  "system.h"
#include   "mips_util.h"


extern	unsigned long long *hbc_ctl;


#define		DEBUG	1
#define		BANK2	1

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

#if 0					
	mips_init_cache(CACHE_WRITEBACK);
	mips_flush_icache();
#endif
}



void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int		  ch = 0;
	int		  i;
	unsigned long pci_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	ttyE_init(19200);
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
	printf("\033[02;15HConvergeNet Technologies - NAND Flash Demonstration");
	printf("\033[03;07H*********************************************************************");
	printf("\033[05;18Hi) Initialize the Flash");
	printf("\033[06;18Hg) Get Id");
	printf("\033[07;18He) Erase the Block 0");
	printf("\033[08;18Hr) Read the Block 0");
	printf("\033[09;18Hp) Program the Block 0");
	printf("\n\r\n\r");
}
#define	PAGE_SIZE 2048
unsigned char lbuf[PAGE_SIZE];
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
	
	
	printf("Ready###\n");
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'i':
				status = nflash_init();
				if ( status != OK)
					printf("Error in init - 528 \n\r");
				else
					printf("Init OK - 528 \n\r");
				break;
			case 'I':
				status = nflash_init_noecc();
				if ( status != OK)
					printf("Error in init 512 \n\r");
				else
					printf("Init OK - 512 \n\r");
				break;
			case 'g':
			case 'G':
				nflash_get_id(0);
				break;
			case 'e':
			case 'E':
				if ( nflash_erase_block(0) == OK)
					printf("Erase OK\n\r");
				else
					printf("Erase Fail\n\r");
				break;
			case 'r':
				for(j=0; j < 16; j++) {
					status = nflash_read_page(lbuf, j);
					printf("\n\rPage %d :", j);
					for(i=0; i < PAGE_SIZE; i++) {
						if (lbuf[i] != 0xFF) {
							printf("Not Blank", j);
							break;
						}
					}
					printf("STUTUS %d\n\r", status);
					mem_print((U32)lbuf, PAGE_SIZE);
				}
				break;
			case 'R':
				for(j=0; j < 16; j++) {
					nflash_read_noecc(lbuf, j * PAGE_SIZE, PAGE_SIZE);
					printf("\n\rPage %d :", j);
					for(i=0; i < PAGE_SIZE; i++) {
						if (lbuf[i] != 0xFF) {
							printf("Not Blank", j);
							break;
						}
					}
					printf("\n\r");
					mem_print((U32)lbuf, PAGE_SIZE);
				}
				break;
			case 'p':
				printf("Programming...");
				for(j=0; j < 16; j++) {
					for(i=0; i < PAGE_SIZE; i++) {
						lbuf[i] = j + 2;
						if ( (i % 128)  == 0)
							lbuf[i] |= 0x80;
					}
					if ( (status = nflash_write_page(lbuf, j)) != OK)
						printf("Err, Page %d status %d\n\r", j, status);
				}
				printf("Done\n\r");
				break;
			case 'P':
				for(j=0; j < 16; j++) {
					for(i=0; i < PAGE_SIZE; i++)
						lbuf[i] = j + 2;
					if ( nflash_program_noecc(lbuf, j * PAGE_SIZE, PAGE_SIZE) != OK)
						printf("Program Error, Page %d\n\r", j);
				}
				break;
			case 'x':
			case 'X':
				{
					U32 *mptr = (U32 *)0xA0000001;
					*mptr = 1;
				}
				break;
			case 'u':
			case 'U':
				{
					int i;
					for(i=0; i < 10; i++) {
						while(!(RUART(0)->rlsr & ST_LSR_TX_EMPTY))
							;
						RUART(0)->holding = 'A';
					}
				}
				break;
			case 'a':
			case 'A':
				*((U8 *)(0xBC088000)) = 0;
					
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
void (*__reboot)() = (void (*)())(0xBFC00000);

void
get_message()
{
	char lbuf[20];
	int i, ch;
	
	lbuf[0] = 'R';
	i = 1;
	while(i < 20) {
		if ( ttyE_poll() == 0) {
			ch = ttyE_in();
			lbuf[i] = ch;
			i++;
		}
		if ( ch == '\n') {
			break;	
		}
	}
	lbuf[9] = 0;
	if ( strcmp(lbuf, "Reboot###") == 0) {
		printf("Rebooting the System, Please Wait\n");
		NU_Sleep(200);
		DISABLE_INT;
		__reboot();
	}
	printf("Done\n");
}

void   
test_1(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int ch;
	unsigned char lbuf[2048];

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	for(;;){
		if ( ttyE_poll() == 0) {
			ch = ttyE_in();
			switch(ch){
			case 'R':
				get_message();
				break;
			}
		}
		
		NU_Sleep(10);
#if 0
	status = nflash_read_page(lbuf, 0);
#endif
	}
}

