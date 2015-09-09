/*************************************************************************/
// File: Nac.cpp
// 
// Description:
// This file is the startup code for the NAC Image
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
//	$Log: /Gemini/Odyssey/Nac/Nac.cpp $
// 
// 17    2/03/00 7:16p Jlane
// Add utility routine to check if we're in a RAC slot (24/28).
// 
// 16    2/02/00 5:50p Jlane
// Remove code to switch FCPort 2 to backplane for internal drives
// 
// 15    1/27/00 4:51p Jaltmaier
// Profiling.
// 
// 14    1/09/00 4:51p Mpanas
// Convert to using CtTtyUtils
// remove globals defined elsewhere
// 
// 13    1/07/00 4:30p Cwohlforth
// Changed menu title if SYSTEM_EXERCISER is defined in the prefix file.
// Change per Bob Weast.
// 
// 12    12/22/99 5:43p Jlane
// #define StartOS again to start up by default and turn default trace
// back down to preferred levels.
// 
// 11    12/21/99 1:48p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 10    11/06/99 5:53p Vnguyen
// Add flag to check if NAC is already initialized.  Do not allow init a
// second time as that will hang Gemini.
// 
// 9     10/14/99 3:33p Jlane
// #define START_OS so no 'i' is needed to start OS.
// 
// 7     9/16/99 11:12p Jlane
// Turn on TRACE_MESSAGE
// 
// 6     9/14/99 1:36p Agusev
// MGP - change trace levels for better debug
// 
// 5     9/01/99 7:04p Jlane
// Don't include pcimap.h it gives mmap redeclared errors.  Also don't
// call init hardware anymore CHAOS does it.
// 
// 4     8/10/99 8:48p Mpanas
// _DEBUG cleanup
// 
// 3     7/28/99 6:19p Mpanas
// Changes for E2/E1 in the same file
// - must compile one or the other
// 
// 2     7/23/99 1:38p Mpanas
// remove bootblockp since it is defined
// in Drivers
// 
// 1     7/21/99 10:12p Mpanas
// First pass at a NAC Image
// 
//
// 04/21/99 Michael G. Panas: Create file
/*************************************************************************/


/*************************************************************************/

extern "C" {
#include "CtTtyUtils.h"
}

#include "stdio.h"
#include "OsTypes.h"

// Debugging is turned on for now
#ifdef _DEBUG
#define FCP_DEBUG 
#endif
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_APP_INIT
#include "Odyssey_Trace.h"

#include "FcpError.h"
#include "FcpTrace.h"
#include "FcpEvent.h"
#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "FcpProto.h"
#include "Odyssey.h"		// Systemwide parameters
#include "RqOsStatus.h"
#include "DdmManager.h"

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

#include "SystemVdn.h"

// Turn this on for 3 ports, off for 2 ports
#define	NAC_E2

#define	KEYS			// turn on the key decoder
#define	START_OS		// start CHAOS before entring the key decoder

#define	NUMBER_OF_INSTANCES		3		// number of QLogic chips supported here

/*************************************************************************/
// Forward References
/*************************************************************************/
void	DrawScreen();
extern	"C" int ttyA_in();
extern	"C" int ttyA_poll();
extern  "C" void galileo_regdump();
extern	U32 IsNacE1();

extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);

void	Ext_Scan_Loops(Message *pMsg);
void	Ext_LIP(U32 loop);
void	Ext_Get_VP_DB(U32 loop);
void	Ext_Get_AL_PA_Map(U32 loop);
}

void Read_Verify_Test();
void Write_Verify_Test();
void extended_keys();
void set_RAC_mode();
void set_NIC_mode();

// external entry methods
void Ext_Read_Test(U32 drive_no);
void Ext_Sts_Read_Test(U32 vdn);
void Ext_Scan_For_Drives(VDN vdn);
void Ext_Sts_Write_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);
void Ext_Sts_Read_Verify_Test(U32 vdn, U32 logical_block_address,
		U32 block_count);

/*************************************************************************/
// Globals
/*************************************************************************/
UNSIGNED			ISP_memory_base_address[NUMBER_OF_INSTANCES];
UNSIGNED			ISP_Interrupt[NUMBER_OF_INSTANCES];
UNSIGNED			ISP_Type[NUMBER_OF_INSTANCES] = 
						{ 0x00002200,
						  0x00002200,
						  0x00002200 };

INSTANCE_DATA		Instance_Data[NUMBER_OF_INSTANCES];	// Instance data

void		task_0(void);

char		fVerbose = false;
char		fOSInitializeCalled = false;

unsigned	TestVdn = 8;
unsigned	Drive = 0;

// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
unsigned	index[15] =	// pick the array indexes that will be used
			{
				TRACE_FCP,					// 0
				TRACE_FCP_TARGET,			// 1
				TRACE_FCP_INITIATOR,		// 2
				TRACE_DRIVE_MONITOR,		// 3
				TRACE_RAM_DISK,				// 4
				TRACE_SCSI_TARGET_SERVER,	// 5
				TRACE_BSA,					// 6
				TRACE_DDM,					// 7
				TRACE_DDM_MGR,				// 8
				TRACE_MESSENGER1,			// 9
				TRACE_TRANSPORT,			// 10 !
			//	TRACE_PTS,					// 11 @
				TRACE_VIRTUAL_MGR,	//11
				TRACE_MESSENGER,			// 12 #
				TRACE_FCP_LOOP,				// 13 $
				TRACE_MESSAGE				// 14 %
			};

// NAC hardware values
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
	TraceLevel[TRACE_APP_INIT] = TRACE_L2;		// set trace levels for debug
	TraceLevel[TRACE_DDM_MGR] = TRACE_OFF_LVL;
	TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;	
	TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;	
	TraceLevel[TRACE_BSA] = TRACE_L2;	
	TraceLevel[TRACE_FCP] = TRACE_L2;	
	TraceLevel[TRACE_FCP_TARGET] = TRACE_L2;	
	TraceLevel[TRACE_FCP_INITIATOR] = TRACE_L2;	
	TraceLevel[TRACE_HEAP1] = TRACE_OFF_LVL;	
	TraceLevel[TRACE_FCP_LOOP] = TRACE_L2;	
	TraceLevel[TRACE_VIRTUAL_MGR] = TRACE_OFF_LVL;	
	//TraceLevel[TRACE_MESSAGE] = TRACE_L4;	
	//TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;	
	//TraceLevel[TRACE_TRANSPORT] = TRACE_ALL_LVL;	
	
	
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
#endif

	// clear the instance data
	memset((void *)&Instance_Data[0], 0, sizeof(INSTANCE_DATA) * NUMBER_OF_INSTANCES);
	
	// setup for NAC Board addresses
	// poke the actual values into the instance stuctures
	Instance_Data[0].ISP_Regs = (void *)ISP_memory_base_address[0];
	Instance_Data[0].FCP_interrupt = ISP_Interrupt[0];
	Instance_Data[0].ISP_Type = ISP_Type[0];

#if 1		// not working on E0 boards
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

	#if false
	// Check which slot we are in, 24 and 28 are DDH slots, all others
	// are external.  Turn on the correct Fibre port for chip 2
	if ((bootblock.b_slot == IOP_RAC0) || (bootblock.b_slot == IOP_RAC1))
	{
		// use internal DDHs
		set_RAC_mode();
		printf("\n\rFC Port 2 Configured as Internal (DDH)\n\r");
	}
	else
	#endif
	{
		set_NIC_mode();
		printf("\n\rFC Port 2 Configured as External\n\r");
	}
	
#ifdef KEYS
#ifdef START_OS
	#if false
	printf("\r\nCHAOS coming soon to a Gemini near you...\r\n");
	printf("\r\nPress any key to avert CHAOS....\r\n");
	unsigned time = 2 * 1000;
	int start = true;
	while (time > 0)
	{
		if ( ttyA_poll() != 0)
		{
			int ch = ttyA_in();
			if (ch == ' ')
			{
				start = false;
				break;
			}
		}
		NU_Sleep(20);
		time -= 20;
	}
	if (start)
	{
		// start the OS before key decoder
		printf("\r\nCHAOS coming now to a Gemini near you...\r\n");
		fOSInitializeCalled = true;
		Oos::Initialize();
	}
	else
		printf("\r\nCHAOS narrowly averted...\r\n");
	#else
	// start the OS before key decoder
	fOSInitializeCalled = true;
	Oos::Initialize();
	#endif
#endif

	// loop forever
	task_0();
#else
	// start the OS
	fOSInitializeCalled = true;
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

            case ' ':  /* SPACEBAR - redraw the screen */
				DrawScreen();
                break;

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

			// Oos specific command extensions
            case 'i':
            	if (fOSInitializeCalled)
					printf("\nError: Os::Initialize() has already been called.\n");
				else
				{
		           	TRACE_HEX(TRACE_L2, "\n\rOos:Initialize called ", (U32)Os::Initialize);
					fOSInitializeCalled = true;
					Os::Initialize();
				}
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
				Ext_Read_Test(Drive);			// drive global
	
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
	
            case 'x':
            	{
            	Critical 	section;
            	for (;;)
            	;
				}
                break;

            case 'q':
            	//isp_dumpreg(0x10000000);
            	//print_devs();
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
        		printf("%c", key);
                break;

	            }  /* switch (key = getchar()) */

		NU_Sleep(cMsDelay);
		cMsDelay=25;
		}  /* while(1) */
	}


void DrawScreen() {
	printf("%s", clrscn);
    printf_at(1, 10, "***************************************************************");
#ifndef SYSTEM_EXERCISER
    printf_at(2, 10, "***    ConvergeNet Technologies - Odyssey 2000 NAC Image    ***");
#else
    printf_at(2, 10, "***    ConvergeNet Technologies - Odyssey 2000 NAC SysEx    ***");
#endif
	printf_at(3, 10, "***       i - init   0-9 Trace (+/-)  X - exit              ***");
	printf_at(4, 10, "*** e-n - NIC  e-r - RAC               Slot = x             ***");
	printf_at(4, 55, " ");
	printf("%d", bootblock.b_slot);
	printf_at(5, 10, "***************************************************************\r\n");
	
}


//**************************************************************************
// Handle extended keys
//
//	r - configure FC hub on port 2 to RAC mode (internal DDHs)
//	n - configure FC hub on port 2 to NIC mode (external FC)
//	l - enable loopback mode
//	d - disable loopback mode
//**************************************************************************
void extended_keys()
{
	int ch;
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'r':
				printf("Configured as RAC\n\r");
				set_RAC_mode();
				break;
			case 'n':
				printf("Configured as NIC\n\r");
				set_NIC_mode();
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
			case 'v':	// dump routing info:
				DdmManager::DumpMaps();
				break;
			case 'w':	// Dump IOP states
				RqOsStatusDumpIst::Invoke();
				break;
			case 'x':	// Dump local dids
				RqOsStatusDumpDidActivity::Invoke();
				break;
			case 'y':	// Dump all dids
				RqOsStatusDumpAllDidActivity::Invoke();
				break;
			case 'z':	// Dump Virtual Device Table
				RqOsStatusDumpVdt::Invoke();
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

#endif	// KEYS

//==========================================================================
//
//	Start of NAC hardware utility routines
//
//==========================================================================

/*************************************************************************/
// Check_DDH_Slot
// Check to see if we are in slot 24 or 28 (DDH slots), return 1 if we are.
/*************************************************************************/
U32 Check_DDH_Slot(void)
{
	if (fOSInitializeCalled)
		if ((Address::iSlotMe == IOP_RAC0) || (Address::iSlotMe == IOP_RAC1))
			return(1);

	// not a DDH slot
	return(0);

} // Check_DDH_Slot


// change port 2 to RAC mode
// Talk to DDH (internal FC)
void set_RAC_mode()
{
	is_nic = 0;
	if ( is_loopback )
		*((U8 *)L3_CONF_CS) = 5;
	else
		*((U8 *)L3_CONF_CS) = CONF_RAC;
}

// change port 2 to NIC mode
// Talk to extenal FC
void set_NIC_mode()
{
	is_nic = 1;
	if ( is_loopback )
		*((U8 *)L3_CONF_CS) = 4;
	else
		*((U8 *)L3_CONF_CS) = CONF_NIC;
}


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

