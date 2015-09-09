/*************************************************************************/
// File: HscsiTest.c
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
#define HSCSI_DEBUG 
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_APP_INIT
#include "Odyssey_Trace.h"

#include "HscsiError.h"
#include "HscsiTrace.h"
#include "HscsiEvent.h"
#include "Pci.h"			// BYTE_SWAP() macros
#include "HscsiProto.h"
#include "Odyssey.h"		// System-wide parameters

#include "Hscsi.h"
#include "HscsiDebug.h"
#include "HscsiData.h"
#include "HscsiIsr.h"

#include "Os.h"
#include "Ddm.h"
#include "Interrupt.h"

#include "pcidev.h"
// #include "pcimap.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
int		Initialize_PCI();

void	HSCSI_Display_Pci(void);

void	DrawScreen();
void	RegHeader(U16 i);
void    printf_at(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
extern	"C" int kbhit();
extern	"C" void ttyA_out(U32 data);

extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);
STATUS	HSCSI_Execute_RISC_Firmware(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Load_RISC_RAM(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Mailbox_Test(PHSCSI_INSTANCE_DATA Id);
STATUS	HSCSI_Verify_RISC_Checksum(PHSCSI_INSTANCE_DATA Id);
}

/*************************************************************************/
// Globals
/*************************************************************************/
int					PCI_error_code;
UNSIGNED			ISP_memory_base_address;
UNSIGNED			ISP_memory_base_address2;
UNSIGNED			ISP_memory_base_address3;

HSCSI_INSTANCE_DATA	H_Instance_Data;

#define		FCP_TARGET_MEMORY_SIZE		(1048576*2)		// TODO get the real thing
#define		FCP_INITIATOR_MEMORY_SIZE	1048576

#define		ISP2100_1_ADDRESS			0x10000000	// Base address for board #1
#define		ISP2100_2_ADDRESS			0x10002000	// Base address for board #2
#define		ISP1040_1_ADDRESS			0x10004000	// Base address for HSCSI

#define		MEMORY_SIZE					(1048576*7)	// 7Mb Just for test
#define		OOS_MEMORY_SIZE				(1048576*2)	// HeapNoFrag size for test

void		task_0(void);

// useful ascii escape sequences
char		*clreol = "\033[K";                /* clear to the end of the line */
char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
char		*bspace = "\033[D \033[D";         /* backspace */

#define		GALILEO_PCI_SLOT			0		// we are always 0
#define		GALILEO_1_PCI_SLOT			7		// the *other* guys can be 7 or 8
// #define		GALILEO_2_PCI_SLOT			8
#define		ISP1040_1_SLOT				8
#define		ISP2100_1_SLOT				9
#define		ISP2100_2_SLOT				6

unsigned	TestVdn = 8;

// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
unsigned	index[15] =	// pick the array indexes that will be used
			{
				TRACE_FCP,					// 0
				TRACE_DAISY_MONITOR,		// 1 HDMHSCSI
				TRACE_HSCSI,				// 2
				TRACE_DRIVE_MONITOR,		// 3 SCSIMONITOR
				TRACE_APP_INIT,				// 4
				TRACE_OOS,					// 5
				TRACE_ECHO_SCSI,			// 6
				TRACE_DDM,					// 7
				TRACE_DDM_MGR,				// 8
				TRACE_MESSENGER1,			// 9
				TRACE_TRANSPORT,			// 10 !
				TRACE_PTS,					// 11 @
				TRACE_MESSENGER,			// 12 #
				0,
				0
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


// Setup QL1040 board (Slot 8)
	data=GetPciReg(0, ISP1040_1_SLOT, 0, 0);
	if (data != 0x10201077)									/* 10201077 = QLogic ISP1xxx */
	{
		printf("\n\r QL1040 board not detected.");
		return 3;
	}
	SetPciReg(0, ISP1040_1_SLOT, 0, 4, 0x00000015); // Enable Bus Master/Slave
	SetPciReg(0, ISP1040_1_SLOT, 0, 0x10, ISP1040_1_ADDRESS);
	ISP_memory_base_address3 = GetPciReg(0, ISP1040_1_SLOT, 0, 0x10);
	ISP_memory_base_address3 &= 0xfffffff0;
	
   return 0;

} // Initialize_PCI


/*************************************************************************/
// HSCSI_Display_Pci
// Display some special registers
/*************************************************************************/
void   HSCSI_Display_Pci(void)
{
	//SetPciReg(0, 0, 1, 0x114, 0x01000000);
	
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rBAR 0 = ", GetPciReg(0, 0, 0, 0x10));
	HSCSI_PRINT_HEX(TRACE_L8, " SWAP BAR 0 = ", GetPciReg(0, 0, 1, 0x110));
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rBAR 1 = ", GetPciReg(0, 0, 0, 0x14));
	HSCSI_PRINT_HEX(TRACE_L8, " SWAP BAR 1 = ", GetPciReg(0, 0, 1, 0x114));
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rBAR 2 = ", GetPciReg(0, 0, 0, 0x18));
	HSCSI_PRINT_HEX(TRACE_L8, " SWAP BAR 2 = ", GetPciReg(0, 0, 1, 0x118));
	HSCSI_PRINT_HEX(TRACE_L8, "\n\rPCI_0 Enables = ", BYTE_SWAP32(*((unsigned long *)(0xB4000C3c))));
	
}

/*************************************************************************/
// StartTask
// Come here when Nucleus has finished initializing.  Startup the Target
// and the Initiator Tasks
/*************************************************************************/
void   StartTask(UNSIGNED argc, VOID *argv)
{
	HSCSI_INSTANCE_DATA	*Id;
	
	PCI_error_code = Initialize_PCI();
	
	//dma_base_addr = (void *)memmaps.pciSlave;
	//dma_base_addr = (void *)0;
	
#if defined(HSCSI_DEBUG) && defined(_DEBUG)

	// All levels set to 0 by default
	TraceLevel[TRACE_APP_INIT] = TRACE_ALL_LVL;		// set trace levels for debug
	TraceLevel[TRACE_DDM_MGR] = TRACE_OFF_LVL;
	TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;	
	TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;	
	TraceLevel[TRACE_BSA] = TRACE_L2;	
	TraceLevel[TRACE_FCP] = TRACE_L2;	
	TraceLevel[TRACE_FCP_TARGET] = TRACE_L2;	
	TraceLevel[TRACE_FCP_INITIATOR] = TRACE_L2;	
	TraceLevel[TRACE_HSCSI] = TRACE_L2;	
	TraceLevel[TRACE_HEAP1] = TRACE_L1;	
	TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;	
	
	HSCSI_PRINT_STRING(TRACE_ENTRY_LVL, "StartTask entered\n\r");
	HSCSI_PRINT_NUMBER(TRACE_L5, "PCI_error_code = ", PCI_error_code);
	HSCSI_PRINT_HEX(TRACE_L5, "\n\rISP_memory_base_address = ", ISP_memory_base_address);
	HSCSI_PRINT_HEX(TRACE_L5, "\n\rISP_memory_base_address3 = ", ISP_memory_base_address3);

	// DEBUG - Lets look at some of the registers that are preset by RESET
	HSCSI_Display_Pci();
#endif

	// setup for Eval Board addresses
	// poke the actual values into the RAC/NIC config stuctures

	extern HSCSI_CONFIG HdmHscsiParms;
	
	HdmHscsiParms.base_ISP_address = ISP_memory_base_address3;
	HdmHscsiParms.interrupt = 0; // ISP_INTERRUPT_VECTOR_NUMBER;
	HdmHscsiParms.config_instance = INITIATOR_INSTANCE;
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
	unsigned  i,s;
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

#if defined(HSCSI_DEBUG) && defined(_DEBUG)
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
			for (idx = 0; idx < 15; idx++)
			{
				TraceLevel[index[idx]] = TRACE_ALL_LVL;
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
			}
			break;

		case 'm':
			// Set the Global TraceLevel for all 15 to min
			for (idx = 0; idx < 15; idx++)
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
			
		case 'U':
			printf_at(7,0,"ISP1040/PCI internal registers\r\n");
			for (i=0x80004000; i < 0x80004100; i+= 4) { // slot 6 = 80003000
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				*(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(i);
//				printf("%08x", BYTE_SWAP32(*(unsigned long*)(GT_PCI_CONFIG_DATA_REG)));
				printf("%04x%04x", (*(unsigned long*)(GT_PCI_CONFIG_DATA_REG)&0xffff),
					(*(unsigned long*)(GT_PCI_CONFIG_DATA_REG)>>16));
			}
			break;


		// FCP specific command extensions
        case 't':
        	extern void S_Test_Unit_Ready(U32 drive_no);
        	S_Test_Unit_Ready(0);
			break;

        case 'y':
        // spare
			break;

#if !defined(NIC_TEST_BUILD)
		case 'h':
			void S_Read_Test(U32 drive_no);
			
			S_Read_Test(0);		// drive 0

			break;

		case 'j':
			void S_Sts_Read_Test(U32 drive_no);
			
			S_Sts_Read_Test(TestVdn);		// Virtual Device 

			break;

        case 's':
        {
        	// defined in the DriveMonitor
			extern void S_Scan_For_Drives(Message *pMsg);
		
			S_Scan_For_Drives(NULL);
			s=1; // keep track of start/stop
		}
			break;
		
		case 'S':
			s=s^0x0001;
			extern void S_Stop_Drive(U32 drive_no, unsigned s);
			S_Stop_Drive(0,s);
			break;
#endif

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

		case 'R':
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
		
		case 'H':
			printf_at(7,0,"ISP 3 registers\r\n");
			for (i=0;i<0x100;i+=2) {
				if (i%0x10==0) {
					printf("\n\r");
					printf("%04x",i&0xff);
				}
				printf(" ");
				printf("%04x",BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address3 | 0xa0000000)+i))));
			}
			break;

		case 'f':
			U16 *ispcs = (U16 *)((ISP_memory_base_address3 | K1BASE)+HSCSI_CONFIG_1);
			U16 *isphccr = (U16 *)((ISP_memory_base_address3 | K1BASE)+HSCSI_HCCR);
			// Clear RISC interrupt (FCP_Mailbox_Wait_Ready_Intr) if needed
			
			*isphccr=BYTE_SWAP16((UNSIGNED)HCCRPAUSERISC); // Pause RISC
			printf_at(7,0,"HSCSI PBIU/RISC registers\r\n");
			j = BYTE_SWAP16(*ispcs)&0x00f7;
										
			*ispcs=BYTE_SWAP16(j); // Set ISP Control/Status register
			for (i=0x00; i<0x100; i+=2) {
				if (!(i % 0x10)) {
					printf("\n\r");
					printf("%04x",i&0xff);
				}
				printf(" ");
				printf("%04x", BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address3 | K1BASE)+i))));
			}
			printf("\r\n");
			*isphccr=BYTE_SWAP16((UNSIGNED)HCCRRLSRISC); // Unpause RISC
			break;
		case 'F':
			U16 *ispcs2=(U16*)((ISP_memory_base_address3 | K1BASE)+HSCSI_CONFIG_1);
			U16 *isphccr2=(U16*)((ISP_memory_base_address3|K1BASE)+HSCSI_HCCR);
			U16 *sxp=(U16*)((ISP_memory_base_address3|K1BASE)+0xa4);
			
			*isphccr2=BYTE_SWAP16((UNSIGNED)HCCRPAUSERISC); // Pause RISC
			printf_at(7,0,"HSCSI PBIU/SXP registers\r\n");
			j = BYTE_SWAP16(*ispcs2)|0x0008; // SXP select
				
			*ispcs2=BYTE_SWAP16(j); // ISP Config 1 register
//			*sxp = BYTE_SWAP16(0x0c00); // SXP override
			for (i=0x00;i<0x100; i+=2) {
				if (!(i % 0x10)) {
					printf("\n\r");
					printf("%04x",i&0xff);
			    }
			    printf(" ");
			    printf("%04x",BYTE_SWAP16(*((U16*)((UNSIGNED)(ISP_memory_base_address3 | K1BASE)+i))));
			}
			printf("\r\n");
			
			j = j&0x00f7; // SXP select bit off
			*ispcs2=BYTE_SWAP16(j);
			*isphccr2=BYTE_SWAP16((UNSIGNED)HCCRRLSRISC); // Unpause RISC
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
