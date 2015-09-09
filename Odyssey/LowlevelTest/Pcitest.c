/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: pcitest.c
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
#include  "galileo.h"
#include  "pcimap.h"
#include  "pcidev.h"
#include  "gt.h"
#include  "pcialloc.h"
#include  "que.h"
#include  "common.h"
#include  "ioptypes.h"
#include  "bootblock.h"

extern  bootblock_t bootblock;
extern void *GetSystemPool(int size);
extern void *GetPciPool(int size);

#define	IS_HBC0	((bootblock.b_type == IOPTY_HBC) && (bootblock.b_slot== 0))?1:0
#define	IS_HBC1	((bootblock.b_type == IOPTY_HBC) && (bootblock.b_slot== 1))?1:0




/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory_G;
NU_MEMORY_POOL  Local_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;
NU_TASK         Test_Task3;
NU_TASK         Test_Task4;

U32	nopBitFlag = 0;
/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);
void    test_3(UNSIGNED argc, VOID *argv);
void    test_4(UNSIGNED argc, VOID *argv);

U32 	CreateBitFlag();
void	SetConsoleIndex(int pciSegment);


/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
#ifdef	INCLUDE_ODYSSEY
void    Application_Initialize(void *first_available_memory)
#else
void    DmaTest_Initialize(void *first_available_memory)
#endif
{
    VOID    *pointer;

	/* If the code is running in Cache the vlaue of 
	 * virtualAddressSpace will become 0x80000000 else 0xA0000000
	 */
	virtualAddressSpace = (U32)first_available_memory & 0xE0000000;
#ifdef	INCLUDE_ODYSSEY
    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    NU_Create_Memory_Pool(&System_Memory_G, "SYSMEM", 
                        first_available_memory, M(16), 56, NU_FIFO);
    NU_Create_Memory_Pool(&Local_Memory, "LOCAL", 
				(void *)(virtualAddressSpace + M(64)), M(16), 56, NU_FIFO);

    /* Create IOPMaster_Task */
    NU_Allocate_Memory(&System_Memory_G, &pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&IOPMaster_Task, "IOPMaster", IOPMaster, 0, NU_NULL, 
					pointer, 10000, 1, 20, NU_PREEMPT, NU_START);
#else

    /* System Meomry is is in the PCI Space */
    
    pointer = GetPciPool(M(16));
    if ( pointer == 0 ) {
    	printf("Error getting System Memory\n\r");
    	return;
    }
    NU_Create_Memory_Pool(&System_Memory_G, "SYSMEM", 
    					pointer, M(16), 56, NU_FIFO);
    					
    /* Local bufs are in the System Space */
    pointer = GetSystemPool(M(16));
    if ( pointer == 0 ) {
    	printf("Error getting Pci Memory\n\r");
    	return;
    }
    NU_Create_Memory_Pool(&Local_Memory, "LOCAL", 
                        pointer, M(16), 56, NU_FIFO);
	IOPMaster(0, pointer);
#endif

}



void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int		  i;
	unsigned long devven_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	printf("\n\r");
#ifdef	INCLUDE_ODYSSEY
	printf("Executing the System Code....\n\r");

	/* Initilaize the Hardware */
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
#endif

	/* Probe the Devices, on HBC0 only */
	if (IS_HBC0) {
		/* Init the bridges again */
		InitBridgeFTree();
		for(i=2; i < MAX_SLOTS; i++)
			if ( memmaps.aIdPci[i]) {
				devven_id = pciconf_readl(memmaps.aIdPci[i], 0);
				if ( devven_id == BRIDGE_21154_ID)
					memmaps.aBoardType[i] = BOARD_NAC;
				if ( devven_id == GAL120_ID)
					memmaps.aBoardType[i] = BOARD_SSD;
			}
		/* Update the Bootblock */
		memcpy(&(bootblock.b_memmap), &memmaps, sizeof(memmaps));
	}
	
	/* Initialize the BoardDev */
	for(i=0; i < MAX_SLOTS; i++) {
		if (memmaps.aBoardType[i]) {
			BoardDev[i].found = 1;
		}
	}

	/* Init the Galileo for DMA */
	gt_init();
	gt_initdma(0);
	gt_initdma(1);
	
	
	/* Initialize i2o que/buf */
	i2odev = InitI2oBuffers(&System_Memory_G);
	
	/* Initialize galileo for i2o */
	if ( i2odev )
		InitI2oRegs(i2odev);
	
	/* Initialize the Buffers */
	InitDataBuffers(&Local_Memory, &System_Memory_G);

	/* Spawn the rest of the tasks and initialize */
	Init_System(&System_Memory_G);
	
	/* Start Handling the Galileo Interrupts Now */
	EnableGalileoIntHandler();
	
	if ( IS_HBC0) {
		/* Send Message saying that HBC is ready */
		SendHbcReadyMsg();
		/* Send the Ready Messages to other boards */
		SendReadyMsg();
		/* Init the CMB */
		cmb_init();
	}

	
#ifdef	INCLUDE_ODYSSEY
	/* Lower the Priority of this task */
	NU_Change_Priority( (NU_TASK *)&IOPMaster_Task, 250 );
	
	/* Now this task will be active with lowest priority and can be used to
	 * any cleanup required
	 */
	for(;;){
		NU_Sleep(500);
		Cleanup_System();
	}
#endif
}


void
Init_System( NU_MEMORY_POOL  *Memory_Pool)
{
	VOID *Stack_Pointer;
	STATUS sts;

#ifdef	INCLUDE_ODYSSEY
    /* Create Test_Task0 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task1 to handle Messages  */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task2 for verify Process */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task2, "Test2", test_2, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task3 for Display process */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task3, "Test3", test_3, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);
    /* Create Test_Task4 for send the Nop I2o Messages */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task4, "Test4", test_4, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);
#else
    /* Create Test_Task1 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 21, 20, NU_PREEMPT, NU_START);
#endif
}

void
Cleanup_System()
{
}

void
print_menu()
{
	printf("\033[07;18Hf) Start DMA Test");
	printf("\033[08;18Hx) Start DMA Test All");
	printf("\033[09;18Hg) Stop  DMA Test All");
	printf("\033[10;18Hk) Console On/Off");
	if ( IS_HBC0 ) {
		printf("\033[12;18Hy) Start DMA Test Remote/Local");
		printf("\033[13;18Hz) Stop  DMA Test Remote/Local");
		printf("\033[14;18Hm) Temperature Monitor");
		printf("\033[15;18Hs) Sytem Monitor");
		printf("\033[16;18HPress <A/B/C/D><1/2/3/4> for remote consoles");
	}
	printf("\n\r\n\r");
		
}
void
print_help()
{
	printf("\033[H\033[2J\033[0m");
	printf("\033[01;07H*********************************************************************");
	printf("\033[02;15HConvergeNet Technologies - %s Demonstration, Slot %s", 
					bname[bootblock.b_type], slotname[bootblock.b_slot]);
	printf("\033[03;07H*********************************************************************");
	print_menu();
}
#ifndef	INCLUDE_ODYSSEY
void
DmaMenu(int ch)
{
	int i;
	U32	bitFlag;
#else
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	U32	bitFlag;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
#endif
			switch(ch){


			case 'a':
			case 'A':
				SetConsoleIndex(24);
				break;
			case 'b':
			case 'B':
				SetConsoleIndex(16);
				break;
			case 'c':
			case 'C':
				SetConsoleIndex(28);
				break;
			case 'd':
			case 'D':
				SetConsoleIndex(20);
				break;
			case 'm':
			case 'M':
				consoleIndex = CONSOLE_MAIN_MENU;
				if ( IS_HBC0 ) 
					cmb_scan();
				break;
			case 'i':
			case 'I':
				printf("Init the Bridges\n\r");
				InitBridgeFTree();
				break;
			case 'G':
				printf("Our Galileo Configuration Space:\n\r");
				pciconf_print(0x80000000);
				break;
			case 'p':
			case 'P':
				printf("Devices Found on the PCI Bus 0\n\r");
				probe_pci_devs(0);
				break;
			case 'k':
				if ( consoleIndex == CONSOLE_LOCAL) {
					consoleIndex = CONSOLE_MAIN_MENU;
#ifdef	INCLUDE_ODYSSEY
					print_help();
#else
					DrawScreen();
#endif
				} else {
					consoleIndex = CONSOLE_LOCAL;
						
				}
				break;
			case 'n':
				if ( consoleIndex == CONSOLE_LOCAL_2) {
					consoleIndex = CONSOLE_MAIN_MENU;
#ifdef	INCLUDE_ODYSSEY
					print_help();
#else
					DrawScreen();
#endif
				} else {
					consoleIndex = CONSOLE_LOCAL_2;
				}
				print_pointers();
				break;
			case 'f':
				bitFlag = CreateBitFlag();
				printf("Starting individual tests\n\r");
				DisplayTestStartStat(bootblock.b_slot,StartDmaTest(bitFlag));
				break;
			case 'l':
				bitFlag = CreateBitFlag();
				printf("Starting NOP Message Test \n\r");
				nopBitFlag = bitFlag;
				break;
			case 'o':
				print_pointers();
				break;
			case 'x':
				printf("Starting test to all boards\n\r");
				DisplayTestStartStat(bootblock.b_slot,StartDmaTest(0xFFFFFFFF));
				break;
			case 'g':
				printf("Stopping test to all boards\n\r");
				nopBitFlag = 0;
				StopDmaTest(0xFFFFFFFF);
				break;
			case 's':
				consoleIndex = CONSOLE_SYSTEM_MONITOR;
				break;
			case 'y':
				printf("Starting test on remote/local boards\n\r");
				StartRemoteTest();
				break;
			case 'z':
				printf("Stopping test on remote/local boards\n\r");
				StopRemoteTest();
				break;
			case ' ':
				if ( consoleIndex == CONSOLE_MAIN_MENU)
#ifdef	INCLUDE_ODYSSEY
					print_help();
#else
					DrawScreen();
#endif
				break;
			default:
				printf("%c", ch);
				break;
			}
#ifdef	INCLUDE_ODYSSEY
		}
		
		NU_Sleep(10);
	}
#endif
}

void   
test_1(UNSIGNED argc, VOID *argv)
{
    STATUS    status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	process();
}

void   
test_2(UNSIGNED argc, VOID *argv)
{
    STATUS    status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	VerifyProcess();
	
}
void   
test_3(UNSIGNED argc, VOID *argv)
{
    STATUS    status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	DisplayProcess();
}

void   
test_4(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	U32		  stat;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		if ( nopBitFlag ) {
			/* Send Messages */
			stat = DoNopTest(nopBitFlag);
			/* Print the Status of the Test */
			//DisplayTestStartStat(stat);
		} else {
			NU_Sleep(100);
		}
	}
	
}

U32 
CreateBitFlag()
{
	char	ch;
	int i, j;
	char	matchKey[32];
	int 	board_count = 0;
	U32 	bitFlag = 0;

	/* Init the matchKey array */
	for(i=0; i < 32; i++) 
		matchKey[i] = 0;
	for(i=0; i < 32; i++) {
		/* if the slot is us skip it */
		if ( i == bootblock.b_slot )
			continue;
		/* If the baord exists init the keyMatch */
		if ( BoardDev[i].found) {
			/* increment the board_count */
			board_count++;
			
			/* Set the matchKey */
			if ( board_count >= 10 )
				matchKey[i] = 'A' + (board_count - 10);
			else
				matchKey[i] = '0' + board_count;
		}
	}
	
	/* print the Menu */
	j =1;
	for(i=0; i < 32; i++) {
		if ( matchKey[i]) {
			/* print the key and slot */
			printf("%c - %s", matchKey[i], slotname[i]);

			/* Format to next line */
			if ( (j % 4) == 0)
				printf("\n\r");
			else
				printf(",	");	
			j++;
		}
	}
	printf("\n\r");
	while(1) {
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			/* If LF or CR return */
			if ( (ch == '\r') || (ch == '\n')) {
				printf("\n\r");
				return(bitFlag);
			}
			printf("%c", ch);
			for(i = 0; i < 32; i++) {
				/* If the key matches set the bit in the bitFlag */
				if (matchKey[i] == ch)
					bitFlag = bitFlag | ( 1 << i);
			}
		}
		NU_Sleep(10);
	}
}
void
SetConsoleIndex(int pciSegment)
{
	int ch;
	int slotOffset;
	/* Wait for the next key */
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			/* set the console only if it is 1, 2, 3, 4 */
			switch(ch){
			case '1':
			case '2':
			case '3':
			case '4':
				/* Get the integer value , 0 -3 */
				slotOffset = ch - '1';
				/* If the segment is A or C, do this weird mapping */
				if ( (pciSegment == 24) || (pciSegment == 28))
					slotOffset = (slotOffset + 1) & 0x3;
				/* Set the sonsoleIndex */
				consoleIndex = pciSegment + slotOffset;
				break;
			default:
				break;
			}
			return;
		}
		NU_Sleep(10);
	}
}


