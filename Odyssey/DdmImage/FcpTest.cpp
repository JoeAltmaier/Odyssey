/*************************************************************************/
// File: FcpTest.c
// 
// Description:
// This file tests the FCP driver with HDM front ends.
//
// Test Mode 1
// The idea is to run the QLogic FCP driver in both target mode and
// initiator mode at the same time.  The FCP driver sits on a loop
// between a QLogic adapter card and a disk drive.
//
// It accepts commands from the host running on an NT system
// destined for the disk drive in target mode, sends them to the drive
// in initiator mode, and sends the results back to the host.
//
// Test Mode 2
// This mode will run two QLogic cards and one Galileo card. The disks
// are on one loop and the host is on the other loop.
//
// Test Mode 3
// This mode will run two QLogic cards and two Galileo cards to simulate
// a NIC and a RAC card set.
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//
//*************************************************************************
// Update Log 
// 
// 4/14/98 Jim Frandeen: Create file
// 5/5/98 Jim Frandeen: Use C++ comment style
// 8/25/98 Michael G. Panas: Redo to allow multiple QL2100 cards (Mode 2)
// 9/2/98 Michael G. Panas: Change to a DDM test of the FCP, add Joes
//							console stuff
// 9/13/98 Michael G. Panas: Rewrite to use PCI functions and Galileo.h
// 9/30/98 Michael G. Panas: support _DEBUG as a global
// 11/24/98 Michael G. Panas: update to new Oos changes and BuildSys model
// 12/29/98 Michael G. Panas: changes to pass config data for startup
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/12/99 Michael G. Panas: convert to new Oos model
// 03/03/99 Michael G. Panas: more ifdefs, split RAC/NIC functionality
/*************************************************************************/


/*************************************************************************/

#include "stdio.h"
#include "OsTypes.h"

#define	KEYS			// turn on the key decoder
//#define	START_OS		// start CHAOS before entring the key decoder

// Debugging is turned on
#define FCP_DEBUG 
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_APP_INIT
#include "Odyssey_Trace.h"

#include "FcpError.h"
#include "FcpTrace.h"
#include "FcpEvent.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "FcpProto.h"
#include "Odyssey.h"		// Systemwide parameters

#include "Fcp.h"
#include "FcpDebug.h"
#include "FcpData.h"
#include "FcpIsr.h"
#include "Pci.h"

#include "Os.h"
#include "Ddm.h"
#include "Interrupt.h"

#include "pcidev.h"
//#include "pcimap.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
int		Initialize_PCI();

void	FCP_Display_Pci(void);

void	DrawScreen();
void	RegHeader(U16 i);
void    printf_at(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
extern	"C" int kbhit();
extern	"C" void ttyA_out(U32 data);

extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);

void	Ext_Scan_Loops(Message *pMsg);
void	Ext_LIP(U32 loop);
void	Ext_Get_VP_DB(U32 loop);
void	Ext_Get_AL_PA_Map(U32 loop);
}

void	Read_Verify_Test();
void	Write_Verify_Test();
U32		Get_Hex_Number();
UI64	Get_Hex(char *p_message);

// external entry methods
void Ext_Read_Test(U32 drive_no);
void Ext_Sts_Read_Test(U32 vdn);
void Ext_Scan_For_Drives(Message *pMsg);
void Ext_Sts_Write_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);
void Ext_Sts_Read_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);


/*************************************************************************/
// Globals
/*************************************************************************/
int					PCI_error_code;
UNSIGNED			ISP_memory_base_address;
UNSIGNED			ISP_memory_base_address2;
UNSIGNED			ISP_Type;
UNSIGNED			ISP_Type2;

#define	NUMBER_OF_INSTANCES		2
INSTANCE_DATA		Instance_Data[NUMBER_OF_INSTANCES];	// Instance data

#define		FCP_TARGET_MEMORY_SIZE		(1048576*2)		// TODO get the real thing
#define		FCP_INITIATOR_MEMORY_SIZE	1048576

#define		ISP2100_1_ADDRESS			0x10000000	// Base address for board #1
#define		ISP2100_2_ADDRESS			0x10002000	// Base address for board #2
#define		PCI_CS						0x06		// PCI offset for ISP Control/Status Register
#define		PCI_HCCR					0xc0		// PCI offset for Host Command & Control Register

void		task_0(void);

// useful ascii escape sequences
char		*clreol = "\033[K";                /* clear to the end of the line */
char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
char		*bspace = "\033[D \033[D";         /* backspace */

#define		GALILEO_PCI_SLOT			0		// we are always 0
#define		GALILEO_1_PCI_SLOT			7		// the *other* guys can be 7 or 8
#define		GALILEO_2_PCI_SLOT			8
#define		ISP2100_1_SLOT				9		// NIC
#define		ISP2100_2_SLOT				6		// RAC

unsigned	TestVdn = 8;
unsigned	Drive = 0;

// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
unsigned	index[20] =	// pick the array indexes that will be used
			{
				TRACE_FCP,					// 0
				TRACE_FCP_TARGET,			// 1
				TRACE_FCP_INITIATOR,		// 2
				TRACE_DRIVE_MONITOR,		// 3
#if defined(SCSI_TARGET_SERVER)
				TRACE_RAM_DISK,				// 4
				TRACE_SCSI_TARGET_SERVER,	// 5
				TRACE_BSA,					// 6
#else
				TRACE_APP_INIT,				// 4
				TRACE_OOS,					// 5
				TRACE_ECHO_SCSI,			// 6
#endif
				TRACE_DDM,					// 7
				TRACE_DDM_MGR,				// 8
				TRACE_MESSENGER1,			// 9
				TRACE_TRANSPORT,			// 10 !
				TRACE_PTS,					// 11 @
				TRACE_MESSENGER,			// 12 #
				TRACE_FCP_LOOP,				// 13 $
				0,							// 14 %
				0,							// 15 ^
				0,							// 16 &
				TRACE_MESSAGE,				// 17 *
				TRACE_VIRTUAL_MGR,			// 18 (
				TRACE_VIRTUAL_MSTR			// 19 )
			};


/*************************************************************************/
// Initialize_PCI
// returns non zero if failure
/*************************************************************************/
int Initialize_PCI()
{
	U32		data;

	// setup the Galileo Chip (in slot 0)
	data = GetPciReg(0, GALILEO_PCI_SLOT, 0, 0);
	if (data != BYTE_SWAP32(0x462011AB))					/* 462011AB = GT-64120 */
	{
//		printf("\n\rERROR - Galileo not responding correctly.");
//		printf("\n\rReg 0 was: %08x", data);
		return 1;
	}

	/* enable ourselves as pci bus master */
	/* enable pci bus master mode */
	SetPciReg(0, GALILEO_PCI_SLOT, 0, 4, BYTE_SWAP32(0x00000007));

	// DDDD write mode
	//*(unsigned long*)(GT_CPU_CONFIG_REG) = BYTE_SWAP32(0x10000);

#if defined(RAC_TEST_BUILD)
	// find out what slot we are in by checking the other slots
	// for a Galileo board.
	
	data = GetPciReg(0, GALILEO_2_PCI_SLOT, 0, 0);
	if (data != 0x462011ab)									/* 462011ab = The other GT-64120 */
	{
//		printf("\n\rERROR - GT-64120 #2 not detected.");
//		printf("\n\rReg 0 was: %08x", data);
		return 4;
	}
   
#endif

#if defined(NIC_TEST_BUILD)
	// find out what slot we are in by checking the other slots
	// for a Galileo board.
	
	data = GetPciReg(0, GALILEO_1_PCI_SLOT, 0, 0);
	if (data != 0x462011ab)									/* 462011ab = The other GT-64120 */
	{
//		printf("\n\rERROR - GT-64120 #2 not detected.");
//		printf("\n\rReg 0 was: %08x", data);
		return 4;
	}
   
#endif

#if !defined(NONIC)
	// setup the the first QL2100 (Slot 9) Instance 0
	// This board is used as the NIC
	data = GetPciReg(0, ISP2100_1_SLOT, 0, 0);
	ISP_Type = (data >> 16);

	// check for supported types
	switch(data)
	{
		case 0x21001077:
		case 0x22001077:
			break;			// these OK
		
		default:
//			printf("\n\rERROR - QLogic #1 not detected.");
//			printf("\n\rReg 0 was: %08x", data);
			return 2;
	}
   
	/* enable pci bus master and slave mode        */
	SetPciReg(0, ISP2100_1_SLOT, 0, 4, 0x00000015);
	
	/* Set base address */
	SetPciReg(0, ISP2100_1_SLOT, 0, 0x10, ISP2100_1_ADDRESS);
	ISP_memory_base_address = GetPciReg(0, ISP2100_1_SLOT, 0, 0x10);
	ISP_memory_base_address &= 0xFFFFFFF0;
#endif

#if !defined(NORAC)
	// setup the the second QL2100 (Slot 6) Instance 1
	// This board is used as the RAC
	data = GetPciReg(0, ISP2100_2_SLOT, 0, 0);
	ISP_Type2 = (data >> 16);
   
	// check for supported types
	switch(data)
	{
		case 0x21001077:
		case 0x22001077:
			break;			// these OK
		
		default:
//			printf("\n\rERROR - QLogic #2 not detected.");
//			printf("\n\rReg 0 was: %08x", data);
			return 3;
	}
   
	/* enable pci bus master and slave mode        */
	SetPciReg(0, ISP2100_2_SLOT, 0, 4, 0x00000015);

	/* Set base address */
	SetPciReg(0, ISP2100_2_SLOT, 0, 0x10, ISP2100_2_ADDRESS);
	ISP_memory_base_address2 = GetPciReg(0, ISP2100_2_SLOT, 0, 0x10);
	ISP_memory_base_address2 &= 0xFFFFFFF0;
	
#endif

   return 0;

} // Initialize_PCI


/*************************************************************************/
// FCP_Display_Pci
// Display some special registers
/*************************************************************************/
void   FCP_Display_Pci(void)
{
	//SetPciReg(0, 0, 1, 0x114, 0x01000000);
	
	FCP_PRINT_HEX(TRACE_L8, "\n\rBAR 0 = ", GetPciReg(0, 0, 0, 0x10));
	FCP_PRINT_HEX(TRACE_L8, " SWAP BAR 0 = ", GetPciReg(0, 0, 1, 0x110));
	FCP_PRINT_HEX(TRACE_L8, "\n\rBAR 1 = ", GetPciReg(0, 0, 0, 0x14));
	FCP_PRINT_HEX(TRACE_L8, " SWAP BAR 1 = ", GetPciReg(0, 0, 1, 0x114));
	FCP_PRINT_HEX(TRACE_L8, "\n\rBAR 2 = ", GetPciReg(0, 0, 0, 0x18));
	FCP_PRINT_HEX(TRACE_L8, " SWAP BAR 2 = ", GetPciReg(0, 0, 1, 0x118));
	FCP_PRINT_HEX(TRACE_L8, "\n\rPCI_0 Enables = ", BYTE_SWAP32(*((unsigned long *)(0xB4000C3c))));
	
}

/*************************************************************************/
// StartTask
// Come here when Nucleus has finished initializing.  Startup the Target
// and the Initiator Tasks
/*************************************************************************/
void   StartTask(UNSIGNED argc, VOID *argv)
{
	INSTANCE_DATA	*Id;
	
	PCI_error_code = Initialize_PCI();
	
	//dma_base_addr = (void *)memmaps.pciSlave;
	//dma_base_addr = (void *)0;
	
#if defined(FCP_DEBUG) && defined(_DEBUG)

	// All levels set to 0 by default
	TraceLevel[TRACE_APP_INIT] = TRACE_ALL_LVL;		// set trace levels for debug
	TraceLevel[TRACE_DDM_MGR] = TRACE_L2;
	TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;	
	TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;	
	TraceLevel[TRACE_BSA] = TRACE_L2;	
	TraceLevel[TRACE_FCP] = TRACE_L2;	
	TraceLevel[TRACE_FCP_TARGET] = TRACE_L2;	
	TraceLevel[TRACE_FCP_INITIATOR] = TRACE_L2;	
	TraceLevel[TRACE_HEAP1] = TRACE_L1;	
	TraceLevel[TRACE_PTS] = TRACE_L2;	
	TraceLevel[TRACE_FCP_LOOP] = TRACE_L2;	
	//TraceLevel[TRACE_PTSDEFAULT] = TRACE_ALL_LVL;	
	
	FCP_PRINT_STRING(TRACE_ENTRY_LVL, "StartTask entered\n\r");
	FCP_PRINT_NUMBER(TRACE_L5, "PCI_error_code = ", PCI_error_code);
	FCP_PRINT_HEX(TRACE_L5, "\n\rISP_memory_base_address = ", ISP_memory_base_address);
	FCP_PRINT_HEX(TRACE_L5, "\n\rISP_memory_base_address2 = ", ISP_memory_base_address2);
	FCP_PRINT_HEX(TRACE_L5, "\n\rISP_Type = ", ISP_Type);
	FCP_PRINT_HEX(TRACE_L5, "\n\rISP_Type2 = ", ISP_Type2);

	// DEBUG - Lets look at some of the registers that are preset by RESET
	FCP_Display_Pci();
#endif

	// clear the instance data
	memset((void *)&Instance_Data[0], 0, sizeof(INSTANCE_DATA) * NUMBER_OF_INSTANCES);
	
	// setup for Eval Board addresses
	// poke the actual values into the RAC/NIC config and instance data stuctures
#if defined(NIC_TEST_BUILD)
	{
	extern FCP_CONFIG TargetParms;
	
	TargetParms.config_instance = TRGT_ONLY_INSTANCE;
	
	Instance_Data[0].ISP_Regs = (void *)ISP_memory_base_address;
	Instance_Data[0].FCP_interrupt = ISP_INTERRUPT_I1;
	Instance_Data[0].ISP_Type = ISP_Type;
	}
#elif !defined(NONIC)
	{
	extern FCP_CONFIG TargetParms;
	
	TargetParms.config_instance = TARGET_INSTANCE;
	
	Instance_Data[0].ISP_Regs = (void *)ISP_memory_base_address;
	Instance_Data[0].FCP_interrupt = ISP_INTERRUPT_I0;
	Instance_Data[0].ISP_Type = ISP_Type;
	}
#endif

	// if there is no NIC, must be a RAC in single mode
	// use the RAC slot address
#if defined(NONIC)
	{
	extern FCP_CONFIG InitParms;
	
	InitParms.config_instance = INIT_ONLY_INSTANCE;
	
	Instance_Data[0].ISP_Regs = (void *)ISP_memory_base_address2;
	Instance_Data[0].FCP_interrupt = ISP_INTERRUPT_I1;
	Instance_Data[0].ISP_Type = ISP_Type2;
	}
#endif

#if !defined(NORAC) && !defined(NONIC)
	{
	extern FCP_CONFIG InitParms;
	
	InitParms.config_instance = INITIATOR_INSTANCE;
	
	Instance_Data[1].ISP_Regs = (void *)ISP_memory_base_address2;
	Instance_Data[1].FCP_interrupt = ISP_INTERRUPT_I0;
	Instance_Data[1].ISP_Type = ISP_Type2;
	}
#endif

#ifdef KEYS
#ifdef START_OS
	// start the OS before key decoder
	Oos::Initialize();
#endif

	// loop forever
	task_0();
#else
	// start the OS
	Oos::Initialize();
#endif
}


#ifdef KEYS

/*************************************************************************/
// task_0
// Dummy task to allow keyboard input while system is running
/*************************************************************************/
void   task_0(void) {
	unsigned  key = 0;
	unsigned  i;
	U16 j; // Module Select Bits
	unsigned  cMsDelay=1;
	unsigned  idx;
	
	TRACE_ENTRY(task_0);
	
	// start on a fresh line
	printf("\n\r");
	
	DrawScreen();
	
	/* Task0 forever loop */
	while(1) {

		//cMsDelay=1;
		
        switch (key = getch()) {
        
        // The first set of commands are general purpose commands
        // for all projects
        case 'X':   /* X - cause address exception and return to boot code */
			printf("Exit with exception\r\n\r\n");
            unsigned long *d = ( unsigned long * ) 0x1;
            *d = 0xFFFF0000;
            break;

        case ' ':  /* SPACEBAR - redraw the screen */
			DrawScreen();
            break;

        case 0x08:  /* BACKSPACE */
        case 0x09:  /* TAB */
        case 0x1B:  /* ESC */
			printf(" \n\r");
        	//printf(" /008");
            break;
            
        case 0x0D:  /* ENTER */
        case 0x0A:  /*  or the real ENTER */
			printf("\n\r");
            break;

#if defined(FCP_DEBUG) && defined(_DEBUG)
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			// Set the Global TraceLevel index to the number specified
			idx = key - 0x30;
debug_set:
			printf("\n\rTraceLevel[%d] = %d",
						index[idx], TraceLevel[index[idx]]);
			break;

        case '!':
			idx = 10;
			goto debug_set;

        case '@':
			idx = 11;
			goto debug_set;

        case '#':
			idx = 12;
			goto debug_set;

        case '$':
			idx = 13;
			goto debug_set;

        case '%':
			idx = 14;
			goto debug_set;

        case '^':
			idx = 15;
			goto debug_set;

        case '&':
			idx = 16;
			goto debug_set;

        case '*':
			idx = 17;
			goto debug_set;

        case '(':
			idx = 18;
			goto debug_set;

        case ')':
			idx = 19;
			goto debug_set;

		case '+':
			// Increment the Global TraceLevel
			TraceLevel[index[idx]]++;
			printf("\n\rTraceLevel[%d] = %d",
						index[idx], TraceLevel[index[idx]]);
			break;

		case '-':
			// Increment the Global TraceLevel
			TraceLevel[index[idx]]--;
			printf("\n\rTraceLevel[%d] = %d",
						index[idx], TraceLevel[index[idx]]);
			break;
			
		case 'a':
			// Set the Global TraceLevel for all 15 to max
			for (idx = 0; idx < 20; idx++)
			{
				TraceLevel[index[idx]] = TRACE_ALL_LVL;
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
			}
			break;

		case 'm':
			// Set the Global TraceLevel for all 15 to min
			for (idx = 0; idx < 20; idx++)
			{
				TraceLevel[index[idx]] = TRACE_L2;
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
			}
			break;
#endif

		// Oos specific command extensions
        case 'i':
        	TRACE_STRING(TRACE_L2, "\n\rOs:Initialize called\n\r");
			Os::Initialize();
        	break;
        	
        case 'n':
			printf_at(7,0,"Interrupt counters\r\n");
			for (i=Interrupt::tyFirst; i < Interrupt::tyLast; i++) {
				printf("%04x", (int)Interrupt::aN[i]);
				printf(" ");
				}
			break;

        case 'b':
			printf_at(7,0,"Base registers\r\n");
		    Print_Dump((unsigned long*)GT_CPU_CONFIG_REG, 64);
            break;

		case 'd':
			printf_at(7,0,"I2O registers\r\n");
		   	Print_Dump((unsigned long*)(GT_I2O_BASE), 32);
		   	break;
		   	
		case 'D':
			printf_at(7,0,"DMA registers\r\n");
		   	Print_Dump((unsigned long*)(GT_DMA_CH0_COUNT_REG), 32);
		   	break;
		   
		case 'p':
			printf_at(7,0,"PCI registers\r\n");
		   	Print_Dump((unsigned long*)(GT_PCI_COMMAND_REG), 64);
		   	break;

		case 'u':
			printf_at(7,0,"PCI internal registers\r\n");
			for (i=0x80000000; i < 0x80000120; i+= 4) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				*(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(i);
				printf("%08x", BYTE_SWAP32(*(unsigned long*)(GT_PCI_CONFIG_DATA_REG)));
			}
			break;

		// FCP specific command extensions
        case 't':
        // spare
			break;

        case 'y':
        // spare
			break;

#if !defined(NIC_TEST_BUILD)
		case 'h':
			Ext_Read_Test(Drive);		// drive global

			break;

		case 'j':
			Ext_Sts_Read_Test(TestVdn);		// Virtual Device global

			break;

        case 's':
			Ext_Scan_For_Drives(NULL);
			break;
#endif

		// Loop Stuff
		case 'w':
			// scan all loops
			Ext_Scan_Loops(0);

			break;
			
		case 'l':
			// reset loop
			Ext_LIP(0);		// Loop Instance 0

			break;
			
		case 'L':
			Ext_LIP(1);		// Loop Instance 1

			break;
			
		case 'v':
			// dump VP Database (2200 only)
			Ext_Get_VP_DB(0);		// Loop Instance 0

			break;
			
		case 'V':
			Ext_Get_VP_DB(1);		// Loop Instance 1

			break;
			
		case 'o':
			// Print the AL_PA Map
			Ext_Get_AL_PA_Map(0);

			break;
			
		case 'O':
			// Print the AL_PA Map
			Ext_Get_AL_PA_Map(1);

			break;
		
		// end of loop stuff
		
		case 'r':
			printf_at(7,0,"ISP 1 PCI registers\r\n");
			for (i=0x80004800; i < 0x80004840; i+= 4) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				*(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(i);
				printf("%08x", *(unsigned long*)(GT_PCI_CONFIG_DATA_REG));
			}
			break;

		case 'S':
			printf_at(7,0,"ISP 2 PCI registers\r\n");
			for (i=0x80003000; i < 0x80003040; i+= 4) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				*(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(i);
				printf("%08x", *(unsigned long*)(GT_PCI_CONFIG_DATA_REG));
			}
			break;

		case 'z':
			printf_at(7,0,"ISP 1 registers\r\n");
			for (i=0; i < 0x100; i+= 2) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				printf("%04x", 
					BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address | 0xA0000000) + i))));
			}
		   	break;

		case 'Z':
			printf_at(7,0,"ISP 2 registers\r\n");
			for (i=0; i < 0x100; i+= 2) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				printf("%04x",
					BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address2 | 0xA0000000) + i))));
			}
		   	break;

		case 'f':
			U16 *ispcs = (U16 *)((ISP_memory_base_address | K1BASE)+PCI_CS);
			U16 *isphccr = (U16 *)((ISP_memory_base_address | K1BASE)+PCI_HCCR);
			// Clear RISC interrupt (FCP_Mailbox_Wait_Ready_Intr) if needed
			
			*isphccr=BYTE_SWAP16((UNSIGNED)HCTLPAUSERISC); // Pause RISC
			printf_at(7,0,"ISP 1 RISC/FB/FPM registers\r\n");
			for (j=0; j<0x40; j+=0x10) {
			
				RegHeader(j); // Header for each register
				
				*ispcs=BYTE_SWAP16(j); // Set ISP Control/Status register
				for (i=0x80; i<0x100; i+=2) {
					if (!(i % 0x10)) {
						printf("\n\r");
						printf("%04x",i&0xff);
					}
					printf(" ");
					printf("%04x", BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address | K1BASE)+i))));
				}
			printf("\r\n");
			}
			*isphccr=BYTE_SWAP16((UNSIGNED)HCTLRLSRISC); // Unpause RISC
			break;
		case 'F':
			U16 *ispcs2=(U16*)((ISP_memory_base_address2 | K1BASE)+PCI_CS);
			U16 *isphccr2=(U16*)((ISP_memory_base_address2|K1BASE)+PCI_HCCR);
			
			*isphccr2=BYTE_SWAP16((UNSIGNED)HCTLPAUSERISC); // Pause RISC
			printf_at(7,0,"ISP 2 RISC/FB/FPM registers\r\n");
			for (j=0;j<0x40;j+=0x10) {

				RegHeader(j); // Header for each register
				
				*ispcs2=BYTE_SWAP16(j); // ISP Control/Status register
				for (i=0x80;i<0x100; i+=2) {
					if (!(i % 0x10)) {
						printf("\n\r");
						printf("%04x",i&0xff);
				    }
				    printf(" ");
				    printf("%04x",BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address2 | K1BASE)+i))));
				}
				printf("\r\n");
			}
			*isphccr2=BYTE_SWAP16((UNSIGNED)HCTLRLSRISC); // Unpause RISC
			break;

        case 'x':
        	// x - cause address exception
        	// and return to boot code			
            unsigned long *g = ( unsigned long * ) 0x1;
            *g = 0xFFFF0000;
            break;

        case 'q':
        	// scan all PCI slots for devices
        	// print only the devices found
        	printf("PCI Devices:\n\r");
        	for (i = 0; i < 31; i++) {
        		U32		reg;
        		
        		reg =  GetPciReg(0, i, 0, 0);
        		if (reg == 0xffffffff)
        			continue;
        		printf("S:%02d = %08x\n\r", i, reg);
        	}
			break;

		case 'W':
		{
			printf("Write verify test\r\n");
			
			Write_Verify_Test();
		}
			break;

		case 'R':
		{
			printf("Read verify test\r\n");
			
			Read_Verify_Test();
		}
			break;

        default:
        	printf("%c", key);
            break;

         }  /* switch (key = Get_Char()) */

		NU_Sleep(cMsDelay);
		cMsDelay=5;
		}  /* while(1) */
	}


void DrawScreen() {
	printf("%s", clrscn);
    printf_at(1, 10, "***************************************************************");
    printf_at(2, 10, "***  ConvergeNet Technologies - Odyssey 2000 Demonstration  ***");
	printf_at(3, 10, "*** I - init  s - scan for disks  0-9 Trace (+/-) X - exit  ***");
	printf_at(4, 10, "***************************************************************\r\n");
}

void RegHeader(U16 modselect) { // Module Select Bits - ISP C/S bits 5-4
	switch (modselect) {
		case 0x00:
			printf("\r\n     RISC Processor Registers");
			break;
		case 0x10:
			printf("\r\n     FB Registers");
			break;
		case 0x20:
			printf("\r\n     FPM Registers Page 1");
			break;
		case 0x30:
			printf("\r\n     FPM Registers Page 2");
			break;
	}
}
void
printf_at (int row, int col, char *string)
{
    printf("\033[%d;%dH%s", row, col, string);

}

void
Print_Dump(unsigned long *p, int cb)
{
   int i;
   int j;
   for (i=0; i < cb; i += 4)
   {
      printf("%04x", i*4 + (((int)p) & 0xFFFF));
      printf(": ");
      for (j=0; j < 4; j++)
      {
        char fNMI=(((int)p & 0xFFFF) == 0xC00 && (i+j==12 || i+j == 61));
        unsigned long l=(fNMI? 0 :p[i+j]);
        if (fNMI)
	        printf("XXXXXXXX");
	    else
			printf("%08x", GTREG(l));
      	putchar(' ');
      }
      printf("\r\n");
   }
}

/*************************************************************************/
// 	Write_Verify_Test
// Send number of blocks to device specified by TestVdn.
/*************************************************************************/
void Write_Verify_Test()
{
	// Ask user for logical block address.
	U32 logical_block_address = Get_Hex("\nEnter logical block address ");
	
	// Ask user for number of blocks.
	U32 block_count = Get_Hex("\nEnter block count ");
	
	Ext_Sts_Write_Verify_Test(TestVdn, logical_block_address, block_count);
}

/*************************************************************************/
// 	Read_Verify_Test
// Read number of blocks from device specified by TestVdn.
/*************************************************************************/
void Read_Verify_Test()
{
	// Ask user for logical block address.
	U32 logical_block_address = Get_Hex("\nEnter logical block address ");
	
	// Ask user for number of blocks.
	U32 block_count = Get_Hex("\nEnter block count ");
	
	Ext_Sts_Read_Verify_Test(TestVdn, logical_block_address, block_count);
}

/*************************************************************************/
// 	Get_Hex
// Print message to prompt user, then Get hex number from keyboard.
/*************************************************************************/
UI64 Get_Hex(char *p_message)
{
	U32 invalid_digit = 1;
	UI64 number = 0;
	char next_char = 0;
	UI64 next_value;
	while(invalid_digit)
	{
		Tracef(p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = getchar();
			switch (next_char)
			{
				case '\n':
					return number;
				case '\r':
					return number;
				case '0': next_value = 0; break;
				case '1': next_value = 1; break;
				case '2': next_value = 2; break;
				case '3': next_value = 3; break;
				case '4': next_value = 4; break;
				case '5': next_value = 5; break;
				case '6': next_value = 6; break;
				case '7': next_value = 7; break;
				case '8': next_value = 8; break;
				case '9': next_value = 9; break;
				case 'a': case 'A': next_value = 10; break;
				case 'b': case 'B': next_value = 11; break;
				case 'c': case 'C': next_value = 12; break;
				case 'd': case 'D': next_value = 13; break;
				case 'e': case 'E': next_value = 14; break;
				case 'f': case 'F': next_value = 15; break;
				default:
					invalid_digit = 1;
					Tracef("\nInvalid digit");
					break;
			}
			number = number * 16 + next_value;
		}
	}
	
	return number;
}

#endif



//====================================================================================
//
//	Start of the Console / Test DDM
//	This DDM becomes a system entry to start it early.  We can do test messages
//	and external entry points here now.  To start with, we will use this DDM to
//	send messages and receive replies.
//
//====================================================================================

#include "DdmConsoleTest.h"

// BuildSys Linkage
#include "BuildSys.h"

CLASSNAME(DdmConsoleTest, SINGLE);

// Table method context
typedef struct _CT_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
} CT_TBL_CONTEXT, *PCT_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/
DdmConsoleTest	*pDdmConsoleTest = NULL;

/*************************************************************************/
// DdmConsoleTest -- Constructor
/*************************************************************************/
DdmConsoleTest::DdmConsoleTest(DID did) : Ddm(did) {

	TRACEF(TRACE_L8, ("EXEC  DdmConsoleTest::DdmConsoleTest\n"));
	
	pDdmConsoleTest = this;

}

/*************************************************************************/
// Initialize -- Process Initialize
/*************************************************************************/

STATUS DdmConsoleTest::Initialize(Message *pArgMsg) {

	TRACEF(TRACE_L8, ("EXEC  DdmConsoleTest::Initialize;\n"));
	
	Reply(pArgMsg);	// Complete Initialize
	
	return OK;
}

/*************************************************************************/
// Enable -- Process Enable
/*************************************************************************/

STATUS DdmConsoleTest::Enable(Message *pArgMsg) {

	TRACEF(TRACE_L8, ("EXEC  DdmConsoleTest::Enable;\n"));
	
	Reply(pArgMsg);
	
	return OK;
}

/*************************************************************************/
// DoWork -- Process messages and replies
/*************************************************************************/
STATUS DdmConsoleTest::DoWork(Message *pMsg) {

	TRACE_ENTRY(DdmConsoleTest::DoWork);

	STATUS status=Ddm::DoWork(pMsg);
	
	TRACE_ENTRY(DdmConsoleTest::DoWork Ddm::DoWork);
	
	if (status != OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION)
		return status;

	// This ISM handles Replies to messages sent from the monitor Handler
	if (pMsg->IsReply()) {
	
		// don't do anything with replies, no context, no action
		//break;
		
#if 0
		switch (pMsg->reqCode) {
		
		// Only one reply is valid
		case SCSI_SCB_EXEC:
			{

					
				default:
					// error - invalid action
					break;
				}
			}
			break;
						
		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}
#endif
	}
	
	// New service message has been received
	else switch(pMsg->reqCode) {


		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
	}

	// Return success, we have already delivered the message.
	return OS_DETAIL_STATUS_SUCCESS;
	
}	// DoWork

//==========================================================================
//
//	End of console DDM
//
//==========================================================================


//==========================================================================
//
//	console DDM external entry points
//	These routines will send a message to a DDM, the reply will be
//	returned to the DoWork() method above and discarded
//
//==========================================================================
#include "DM_Messages.h"
#include "SystemVdn.h"

void Ext_Read_Test(U32 drive_no)
{
	DmReadTest *pMsg = new DmReadTest;
	pMsg->payload.drive = drive_no;
	pMsg->payload.type = DM_TYP_DRIVE;
	pMsg->payload.vdnSTS = 0;
	
	pDdmConsoleTest->Send((VDN)vdnDM, pMsg);	
}


void Ext_Sts_Read_Test(U32 vdn)
{
	DmReadTest *pMsg = new DmReadTest;
	pMsg->payload.drive = Drive;
	pMsg->payload.type = DM_TYP_STS;
	pMsg->payload.vdnSTS = vdn;
	
	pDdmConsoleTest->Send((VDN)vdnDM, pMsg);	
}

		
void Ext_Scan_For_Drives(Message *pMsgn)
{
	Message *pMsg = new Message(DM_SCAN);
	pDdmConsoleTest->Send((VDN)vdnDM, pMsg);	
}

void Ext_Sts_Write_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count)
{
	DmWriteVerifyTest *pMsg = new DmWriteVerifyTest;
	pMsg->payload.drive = Drive;
	pMsg->payload.type = 0;
	pMsg->payload.vdnSTS = vdn;
	pMsg->payload.block = logical_block_address;
	pMsg->payload.count = block_count;
	
	pDdmConsoleTest->Send((VDN)vdnDM, pMsg);	
}


void Ext_Sts_Read_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count)
{
	DmReadVerifyTest *pMsg = new DmReadVerifyTest;
	pMsg->payload.drive = Drive;
	pMsg->payload.type = 0;
	pMsg->payload.vdnSTS = vdn;
	pMsg->payload.block = logical_block_address;
	pMsg->payload.count = block_count;
	
	pDdmConsoleTest->Send((VDN)vdnDM, pMsg);	
}


//==========================================================================
//
//	End of console DDM external entry points
//
//==========================================================================

