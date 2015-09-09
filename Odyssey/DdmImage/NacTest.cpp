/*************************************************************************/
// File: NacTest.c
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
// 11/24/98 Michael G. Panas: convert to RAC Test Module
// 01/27/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
// 02/12/99 Michael G. Panas: convert to new Oos model
// 07/06/99 Michael G. Panas: create from RacTest.cpp
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
#include "pcimap.h"

#include "SystemVdn.h"

/*************************************************************************/
// Forward References
/*************************************************************************/
#ifndef	PCITEST
void	DrawScreen();
#endif
void    printf_at(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
extern	"C" int ttyA_in();
extern	"C" int ttyA_poll();
extern  "C" void galileo_regdump();
extern  "C" void skip_hbc_init();
extern  "C" void init_hardware_hack();
extern  "C" U32 get_boardtype();
extern  "C" STATUS init_nic();
extern	U32 IsNacE1();

extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);

void	Ext_Scan_Loops(Message *pMsg);
void	Ext_LIP(U32 loop);
void	Ext_Get_VP_DB(U32 loop);
void	Ext_Get_AL_PA_Map(U32 loop);
}
#ifdef	PCITEST
extern "C" {
void	gt_init();
void	gt_initdma(int );
void	DmaTest_Initialize(void *);
void	DmaMenu(int );
void	DmaRegisterLISR();
void	print_menu();
void	DrawScreen();
}
#endif

void Read_Verify_Test();
void Write_Verify_Test();
U32 Get_Hex_Number();
UI64 Get_Hex(char *p_message);
void extended_keys();

// external entry methods
void Ext_Read_Test(U32 drive_no);
void Ext_Sts_Read_Test(U32 vdn);
void Ext_Scan_For_Drives(VDN vdn);
void Ext_Sts_Write_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);
void Ext_Sts_Read_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);

#define	NUMBER_OF_INSTANCES		3

/*************************************************************************/
// Globals
/*************************************************************************/
void				*dma_base_addr;

UNSIGNED			ISP_memory_base_address[NUMBER_OF_INSTANCES];
UNSIGNED			ISP_Interrupt[NUMBER_OF_INSTANCES];
UNSIGNED			ISP_Type[NUMBER_OF_INSTANCES] = 
						{ 0x00002200,
						  0x00002200,
						  0x00002200 };

INSTANCE_DATA		Instance_Data[NUMBER_OF_INSTANCES];		// Instance data


void		task_0(void);

// useful ascii escape sequences
char		*clreol = "\033[K";                /* clear to the end of the line */
char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
char		*bspace = "\033[D \033[D";         /* backspace */

char		fVerbose = false;

unsigned	TestVdn = 8;
unsigned	Drive = 0;
unsigned	Test_Reg = 6;

// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
unsigned	index[15] =	// pick the array indexes that will be used
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
				0							// 14 %
			};

#define	MY_SLOT_NO	0x0


int	is_nic	= 1;
int	is_loopback	= 0;
#define	L3_CONF_CS		0xBC048000
#define	CONF_NIC		6
#define	CONF_RAC		3

/*************************************************************************/
// StartTask
// Come here when Nucleus has finished initializing.  Startup the Target
// and/or the Initiator Tasks
/*************************************************************************/
void   StartTask(UNSIGNED argc, VOID *argv)
{
	INSTANCE_DATA	*Id;
	int				mask;
	STATUS			x;
	int				skip_hbc_init_flag = 0;
#ifdef	PCITEST
	U32				first_available_memory;
	int				i;
#endif
	
	x = Init_Hardware();
#ifdef	PCITEST
	first_available_memory = 0xA0000000 + M(32);
	DmaTest_Initialize((void *)first_available_memory);
#endif
	memmaps.iSlot = MY_SLOT_NO;
	is_loopback = 0;
	// Check which slot we are in, 24 and 28 are DDH slots, all others
	// are external.  Turn on the correct Fibre port for chip 2
	if ((bootblock.b_slot == IOP_RAC0) || (bootblock.b_slot == IOP_RAC1)) {
		// use internal DDHs
		is_nic = 0;
		*((U8 *)L3_CONF_CS) = CONF_RAC;		
	} else {
		is_nic = 1;
		*((U8 *)L3_CONF_CS) = CONF_NIC;
	}
	
	dma_base_addr = (void *)memmaps.pciSlave;
	
	// get base addresses
	ISP_memory_base_address[0] = get_isp_base(0);
	ISP_Interrupt[0] = get_isp_int(0);
#if 1
	if (IsNacE1())
	{
		ISP_memory_base_address[1] = get_isp_base(1);
		ISP_Interrupt[1] = get_isp_int(1);
	}
#endif
	ISP_memory_base_address[2] = get_isp_base(2);
	ISP_Interrupt[2] = get_isp_int(2);
		
#if defined(FCP_DEBUG) && defined(_DEBUG)

	// All levels set to 0 by default
	TraceLevel[TRACE_APP_INIT] = TRACE_ALL_LVL;		// set trace levels for debug
	TraceLevel[TRACE_DDM_MGR] = TRACE_OFF_LVL;
	TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;	
	TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;	
	TraceLevel[TRACE_BSA] = TRACE_L2;	
	TraceLevel[TRACE_FCP] = TRACE_ALL_LVL;	
	TraceLevel[TRACE_FCP_TARGET] = TRACE_L2;	
	TraceLevel[TRACE_FCP_INITIATOR] = TRACE_ALL_LVL;	
	
	
	FCP_PRINT_STRING(TRACE_ENTRY_LVL, "StartTask entered\n\r");
	TRACEF(TRACE_L5, ("\n\rISP0: base=%X, Int=%X, Type=%X", 
						ISP_memory_base_address[0], ISP_Interrupt[0], ISP_Type[0]));
	if (IsNacE1())
	{
		TRACEF(TRACE_L5, ("\n\rISP0: base=%X, Int=%X, Type=%X", 
						ISP_memory_base_address[1], ISP_Interrupt[1], ISP_Type[1]));
	}
	TRACEF(TRACE_L5, ("\n\rISP0: base=%X, Int=%X, Type=%X", 
						ISP_memory_base_address[2], ISP_Interrupt[2], ISP_Type[2]));
	FCP_PRINT_HEX(TRACE_L5, "\n\dma_base_addr = ", dma_base_addr);
#endif

	// clear the instance data
	memset((void *)&Instance_Data[0], 0, sizeof(INSTANCE_DATA)*NUMBER_OF_INSTANCES);
	
	// setup for NAC Board addresses
	// poke the actual values into the instance stuctures
	Instance_Data[0].ISP_Regs = (void *)ISP_memory_base_address[0];
	Instance_Data[0].FCP_interrupt = ISP_Interrupt[0];
	Instance_Data[0].ISP_Type = ISP_Type[0];

#if 1		// not working
	if (IsNacE1())
	{
		Instance_Data[1].ISP_Regs = (void *)ISP_memory_base_address[1];
		Instance_Data[1].FCP_interrupt = ISP_Interrupt[1];
		Instance_Data[1].ISP_Type = ISP_Type[1];
	}
#endif

	Instance_Data[2].ISP_Regs = (void *)ISP_memory_base_address[2];
	Instance_Data[2].FCP_interrupt = ISP_Interrupt[2];
	Instance_Data[2].ISP_Type = ISP_Type[2];

#ifdef KEYS
#ifdef START_OS
	// start the OS before key decoder
	Oos::Initialize();
#endif
#ifdef	PCITEST
	for (i = 0; i < 15; i++)
	{
		TraceLevel[index[i]] = TRACE_L2;
		printf("\n\rTraceLevel[%d] = %d", index[i], TraceLevel[index[i]]);
	}
	// start the OS
	Oos::Initialize();
	gt_init();
	DmaRegisterLISR();
	gt_initdma(0);
	gt_initdma(1);
	NU_Sleep(10);
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
	unsigned  cMsDelay=1;
	
	TRACE_ENTRY(task_0);
	
	// start on a fresh line
	printf("\n\r");
	
	DrawScreen();
	
	/* Task0 forever loop */
	while(1) {

		cMsDelay=1;
		
        switch (key = ttyA_in()) {
        
        case -1:
        	break;			// no char available
        
        // The first set of commands are general purpose commands
        // for all projects
        case 'X':   /* X - cause address exception and return to boot code */
			printf("Exit with exception\r\n\r\n");
            unsigned long *d = ( unsigned long * ) 0x1;
            *d = 0xFFFF0000;
            break;

#ifndef	PCITEST
        case ' ':  /* SPACEBAR - redraw the screen */
			DrawScreen();
            break;
#endif

        case 0x08:  /* BACKSPACE */
        case 0x09:  /* TAB */
        case 0x1B:  /* ESC */
    		printf(" /007");
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
			// Set the Global TraceLevel for all 10 to max
			for (idx = 0; idx < 15; idx++)
			{
				TraceLevel[index[idx]] = TRACE_ALL_LVL;
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
			}
			break;
			
		case 'm':
			// Set the Global TraceLevel for all 10 to min
			for (idx = 0; idx < 15; idx++)
			{
				TraceLevel[index[idx]] = TRACE_L2;
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
			}
			break;
#endif

        case 'i':
#ifndef	PCITEST
        	TRACE_HEX(TRACE_L2, "\n\rOos:Initialize called ", (U32)Os::Initialize);
			Os::Initialize();
#endif
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
        	printf("Board Status Reg Contents : %08X\n\r", 
					*((unsigned long *)0xBC0F0000));
			break;

        case 'y':
        	printf("Galileo Registers\n");
        	galileo_regdump();
        	
			break;

		case 'h':
			Ext_Read_Test(Drive);		// drive global

			break;

		case 'j':
			Ext_Sts_Read_Test(TestVdn);		// Virtual Device global

			break;

        case 's':
			Ext_Scan_For_Drives(vdnDM);
	
			break;

        case 'J':
			Ext_Scan_For_Drives(vdnDM1);
	
			break;

        case 'S':
			Ext_Scan_For_Drives(vdnDM2);
	
			break;

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
			if (IsNacE1())
			{
				Ext_LIP(1);		// Loop Instance 1
			}

			break;
			
		case 'V':
			Ext_LIP(2);		// Loop Instance 2

			break;
			
		case 'v':
			// dump VP Database (2200 only)
			Ext_Get_VP_DB(0);		// Loop Instance 0
			if (IsNacE1())
			{
				Ext_Get_VP_DB(1);		// Loop Instance 1
			}
			Ext_Get_VP_DB(2);		// Loop Instance 2

			break;
			
		case 'o':
			// Print the AL_PA Map for all
			Ext_Get_AL_PA_Map(0);
			if (IsNacE1())
			{
				Ext_Get_AL_PA_Map(1);
			}
			Ext_Get_AL_PA_Map(2);

			break;
			
		// end of loop stuff
		
		case 'r':
		{
			U32 id;
			S32 reg;
			
			printf_at(7,0,"ISP 0 PCI registers\r\n");
			id = PADDR(0, 2, 0);

			for (reg = 0; reg < 0x40; reg += 4) {
		    	if (reg % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", reg & 0xFF);
				   	}
				printf(" ");
				printf("%08x", pciconf_readl(id, reg));
			}
		}
			break;

		case 'T':
		{
			U32 id;
			S32 reg;
			
			printf_at(7,0,"ISP 2 PCI registers\r\n");
			id = PADDR(0, 4, 0);

			for (reg = 0; reg < 0x40; reg += 4) {
		    	if (reg % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", reg & 0xFF);
				   	}
				printf(" ");
				printf("%08x", pciconf_readl(id, reg));
			}
		}
			break;

		case 'z':
			printf_at(7,0,"ISP 0 registers\r\n");
			for (i=0; i < 0x100; i+= 2) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				printf("%04x", 
					BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[0]|0xA0000000) + i))));
			}
		   	break;

		case 'Y':
			printf_at(7,0,"ISP 1 registers\r\n");
			for (i=0; i < 0x100; i+= 2) {
		    	if (i % 0x10 == 0) {
				    printf("\n\r");
				   	printf("%04x", i & 0xFF);
				   	}
				printf(" ");
				printf("%04x", 
					BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[1]|0xA0000000) + i))));
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
					BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[2]|0xA0000000) + i))));
			}
		   	break;

		case 'A':
		{
			U16		reg;
			
			i = Test_Reg;
			printf("ISP 0 register Loop, addr=%X\r\n", 
							((ISP_memory_base_address[0]|0xA0000000) + i));
			
			while(1)
			{
				if (ttyA_poll() == 0)
					break;
				printf(" ");
				printf("%04x", 
					BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[0]|0xA0000000) + i))));
			}
		}
		   	break;

		case 'B':
		{
			U16		reg;
			
			i = Test_Reg;
			printf("ISP 2 register Loop, addr=%X\r\n", 
							((ISP_memory_base_address[2]|0xA0000000) + i));
			
			while(1)
			{
				if (ttyA_poll() == 0)
					break;
				printf(" ");
				printf("%04x", 
					BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[2]|0xA0000000) + i))));
			}
		}
		   	break;

		case 'C':
		{
			U16		reg;
			
			i = Test_Reg;
			printf("ISP 0 register Loop, addr=%X\r\n", 
							((ISP_memory_base_address[0]|0xA0000000) + i));
			
			while(1)
			{
				if (ttyA_poll() == 0)
					break;
				reg = BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[0]|0xA0000000) + i)));
			}
		}
		   	break;

		case 'E':
		{
			U16		reg;
			
			i = Test_Reg;
			printf("ISP 2 register Loop, addr=%X\r\n", 
							((ISP_memory_base_address[2]|0xA0000000) + i));
			
			while(1)
			{
				if (ttyA_poll() == 0)
					break;
				reg = BYTE_SWAP16(*((U16*)(
					(UNSIGNED)(ISP_memory_base_address[2]|0xA0000000) + i)));
			}
		}
		   	break;

        case 'x':
        	// x - clear ISP Interrupts then cause address exception
        	// and return to boot code
            unsigned long *g = ( unsigned long * ) 0x1;
            *g = 0xFFFF0000;
            break;

        case 'q':
        	//isp_dumpreg(0x10000000);
        	//print_devs();
        	{
        		U32 pci_id;
        		printf("Galileo PCI Registers\n");
        		pciconf_print(0x80000000);
        		pci_id = PADDR(0, 2, 0);
        		printf("ISP PCI Registers\n");
        		pciconf_print(pci_id);
        		
        	}
        	break;

         case 'Q':
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
		case 'e':
			extended_keys();
			break;

           default:
#ifdef	PCITEST
				DmaMenu(key);
#else
        		printf("%c", key);
#endif
                break;

	            }  /* switch (key = getchar()) */

		NU_Sleep(cMsDelay);
		cMsDelay=25;
		}  /* while(1) */
	}


void DrawScreen() {
	printf("%s", clrscn);
    printf_at(1, 10, "***************************************************************");
    printf_at(2, 10, "***     ConvergeNet Technologies - Odyssey 2000 NAC Demo    ***");
	printf_at(3, 10, "*** I - init  s - scan for disks 0-9 Trace (+/-)  X - exit  ***");
	printf_at(4, 10, "*** e-n - NIC,  e-r - RAC, e-l - Lback, e-d - Disable Lback                                    ***");
	printf_at(5, 10, "***************************************************************\r\n");
#ifdef	PCITEST
	print_menu();
#endif
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
	U32 block_count = Get_Hex("\nEnter block count");
	
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
	U32 block_count = Get_Hex("\nEnter block count");
	
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

		
void Ext_Scan_For_Drives(VDN vdn)
{
	Message *pMsg = new Message(DM_SCAN);
	pDdmConsoleTest->Send((VDN)vdn, pMsg);	
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


void extended_keys()
{
	int ch;
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'r':
				printf("Configured as RAC\n\r");
				is_nic = 0;
				if ( is_loopback )
					*((U8 *)L3_CONF_CS) = 5;
				else
					*((U8 *)L3_CONF_CS) = CONF_RAC;
				break;
			case 'n':
				printf("Configured as NIC\n\r");
				is_nic = 1;
				if ( is_loopback )
					*((U8 *)L3_CONF_CS) = 4;
				else
					*((U8 *)L3_CONF_CS) = CONF_NIC;
				break;
			case 'l':
				printf("Configured for Loopback\n\r");
				if ( is_nic )
					*((U8 *)L3_CONF_CS) = 4;
				else
					*((U8 *)L3_CONF_CS) = 5;
				is_loopback = 1;
				*((U8 *)0xBC058000) = 7;
				break;
			case 'd':
				printf("Disable Loopback\n\r");
				if ( is_nic )
					*((U8 *)L3_CONF_CS) = CONF_NIC;
				else
					*((U8 *)L3_CONF_CS) = CONF_RAC;
				is_loopback = 0;
				*((U8 *)0xBC058000) = 0;
				break;
			case ' ':
				break;
			default:
				printf("%c", ch);
				break;
			}
			return;
		}
		NU_Sleep(10);
	}
}
//==========================================================================
//
//	End of console DDM external entry points
//
//==========================================================================



