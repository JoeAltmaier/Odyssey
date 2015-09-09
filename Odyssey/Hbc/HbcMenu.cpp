/*************************************************************************/
// File: HbcMenu.cpp
// 
// Description:
// This file is the startup menu code for the HBC Image
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
//	$Log: /Gemini/Odyssey/Hbc/HbcMenu.cpp $
// 
// 32    2/11/00 1:52p Rbraun
// Added comment on debug IP address entry
// 
// 31    2/08/00 7:21p Jlane
// Set TRACE_HEAP1 to TRACE_OFF_LVL so the debugging heap won't run so
// slow as to cause the system to hang.
// 
// 30    1/26/00 2:52p Joehler
// Fixed typo.
// 
// 29    1/07/00 4:05p Cwohlforth
// Change menu title when SYSTEM_EXERCISER is defined, this for
// diagnostics per Bob Weast.
// 
// 28    12/21/99 1:54p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 27    12/09/99 3:59p Joehler
// Moved output routines to CtTtyUtils.[cpp/h] in Util library
// 
// 26    11/21/99 8:11p Jlane
// Bump Boot Trace level.
// 
// 25    11/15/99 2:28p Jlane
// Add new TRACE levels to default set.
// 
// 24    11/11/99 2:57p Bbutler
// Added flag to enable/disable call to FF_Create on first HBC use.
// 
// 23    11/08/99 9:51a Jlane
// Update slot map display.
// 
// 22    11/03/99 3:33p Dpatel
// added trace for RMSTR
// 
// 21    10/30/99 4:12p Sgavarre
// Menu fixups.
// 
// 20    10/28/99 7:20p Sgavarre
// Many changes for persistance support.
// 
// 19    10/04/99 1:55p Cwohlforth
// This is Andrey's temporary get IP address from HBC menu change.  This
// sets the Gemini IP address to what the user entered through the HBC
// menu.
// 
// 19    9/30/99 10:44a Agusev
// Added code to enter the IP (option 'q')
// Also, put back the requirement to hit 'i' before the OOS starts.
// 
// 17    9/07/99 9:05p Agusev
// Turn on SSAPI manager trace by default and add same to set of trace
// levels managed by menu.  Last but not least add 'o' menu command to
// turn auto set off.
// 
// 16    9/01/99 8:05p Iowa
// 
// 14    8/25/99 7:06p Jlane
// Start the OS without waiting for a keytstroke.
// 
// 13    8/20/99 10:17a Jlane
// Turn off message trace. by default.
// 
// 12    8/15/99 12:08p Jlane
// Added a flag to prevent calling OS:Initialize twice.
// 
// 11    8/11/99 1:19p Jlane
// Cleanup compiler warnings.
// 
// 10    8/09/99 6:47p Jlane
// Turned on TRACE_MESSAGE by default.
// 
// 9     7/23/99 3:01p Jlane
// Changed keystroke 'h' to prompt for a slot number and then call
// init_iop for that slot.  ALso added keystroke 'j' to display slot
// numbers.
// 
// 8     7/10/99 2:39p Jlane
// Added VirtualProxy and PTSProxy to defaulkt trace enum.
// 
// 7     7/09/99 6:11p Jlane
// Remove FCP leftovers.
// 
// 6     7/08/99 4:50p Jlane
// Add Transport and DDM entries in TRACE array.
// 
// 5     7/07/99 2:38p Jlane
// Make # of entries in TRACE array dynamic.
// 
// 4     5/13/99 11:36a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 3     4/26/99 4:00p Jlane
// Spruced up the menu to tell it from RAC/NIC.
// 
// 2     4/26/99 2:02p Jlane
// Added 'h' command as an Init_Hbc call to initialize the bridges.  This
// allows IOPs to restart arbitrarily with minimal human intervention if
// not automatically under programmatic control.
// 
// 1     4/21/99 4:09p Jlane
// Added HBCmenu.
// 
/*************************************************************************/


/*************************************************************************/

extern "C" {
#include "CtTtyUtils.h"
}
#include "stdio.h"

#include "OsTypes.h"

// Debugging is turned on for now
#define _DEBUG
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_APP_INIT
#include "Odyssey_Trace.h"

#include "Pci.h"
#include "Odyssey.h"		// Systemwide parameters
#include "RqOsStatus.h"
#include "HbcMenu.h"

#include "Os.h"
#include "Ddm.h"
#include "Interrupt.h"
#include "DdmPts.h"
#include "DdmManager.h"

#include "pcidev.h"
//#include "pcimap.h"

#define	KEYS			// turn on the key decoder
//#define	START_OS		// start CHAOS before entring the key decoder


/*************************************************************************/
// External References
/*************************************************************************/
extern "C" STATUS init_hbc();				// defined in Drivers/inithd.c.  Need a header.
extern "C" STATUS init_iop(U32 slotnum);	// defined in Drivers/inithd.c.  Need a header.
extern unsigned char box_IP_address[4];

#include "Watch.h"	// Usage ex.: Watch(WATCH1, (void *)&data, sizeof(data), WATCHSTORE);

/*************************************************************************/
// Forward References
/*************************************************************************/
void	DrawScreen();
void EnterIPAddr();


extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);
}


/*************************************************************************/
// Globals
/*************************************************************************/

void		task_0(void);


char		fVerbose = false;
char		fOSInitializeCalled = false;
char		fbox_IP_address_set = false;

#if defined(_DEBUG)
// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
#define		NUM_AUTO_TRACES 20			// MUST be # of entries in following array.
unsigned	index[NUM_AUTO_TRACES] =	// pick the arrray indexes that will be used
			{
				TRACE_ODY_DRIVERS,		// 0
				TRACE_HEAP1,			// 1
				TRACE_APP_INIT,			// 2
				TRACE_OOS,				// 3
				TRACE_DIDMAN,			// 4
				TRACE_DDM_MGR,			// 5
				TRACE_DDM,				// 6
				TRACE_MESSAGE,			// 7
				TRACE_TRANSPORT,		// 8
				TRACE_MESSENGER,		// 9
				TRACE_VIRTUAL_MGR,		// 10 !
				TRACE_VIRTUAL_MSTR,		// 11 @
				TRACE_PTS,				// 12 #
				TRACE_BOOT,				// 13 $
				TRACE_FAILOVER,			// 14 %
				TRACE_RMSTR,			// 15 ^
				TRACE_SSAPI_MANAGERS,	// 16 &
				TRACE_PARTITIONMGR,		// 17 *
				TRACE_CHAOSFILE,		// 18 (
				TRACE_SSD,				// 19 )
			};
#endif

#include "DdmPartitionMgr.h"
/*
struct PmConfig
{
	U32 cPages;
	U16 bytesPerPage;
	FF_CONFIG *flashConfig;
	CM_CONFIG *cacheConfig;
	U32 reformat; // temp for testing
};
*/
//FF_CONFIG flashConfig = { FF_CONFIG_VERSION, sizeof(FF_CONFIG), NULL, 0, 1, 1, 0, 1, 1, 25, 1, 0, 0, 0, 0, 0, 0 };
//CM_CONFIG cacheConfig = { CM_CONFIG_VERSION, sizeof(CM_CONFIG), 1024, 256, NULL, NULL, 0, 64, 60, 95, 0, 0};
//PmConfig bddPartitionMgr={2048, 2048, &flashConfig, &cacheConfig, true};
extern PmConfig bddPartitionMgr;	// If reformat is true DdmPartitionMgr will reformat the flash.
extern BOOL fLoadFromFlash;	// Defined in DdmPTS.cpp.  If TRUE PTS is loaded from flash.
extern U32 cbrgDataBlks;	// Defined in DdmPTS.cpp.


/*************************************************************************/
// StartTask
// Come here when Nucleus has finished initializing.  Startup the Target
// and/or the Initiator Tasks
/*************************************************************************/
void   StartTask(UNSIGNED argc, VOID *argv)
{
#pragma unused(argc)
#pragma unused(argv)
//	STATUS			x;
	
//	x = Init_Hardware();
	
#if defined(_DEBUG)
	// All levels set to default
	TraceLevel[TRACE_ODY_DRIVERS] = TRACE_OFF_LVL;	// 0
	TraceLevel[TRACE_HEAP1] = TRACE_OFF_LVL;		// 1
	TraceLevel[TRACE_APP_INIT] = TRACE_OFF_LVL;		// 2
	TraceLevel[TRACE_OOS] = TRACE_L3;				// 3
	TraceLevel[TRACE_DIDMAN] = TRACE_OFF_LVL;		// 4
	TraceLevel[TRACE_DDM_MGR] = TRACE_L3;		// 5
	TraceLevel[TRACE_DDM] = TRACE_OFF_LVL;			// 6
	TraceLevel[TRACE_MESSAGE] = TRACE_OFF_LVL;		// 7
	TraceLevel[TRACE_TRANSPORT] = TRACE_OFF_LVL;	// 8
	TraceLevel[TRACE_MESSENGER] = TRACE_OFF_LVL;	// 9
	TraceLevel[TRACE_VIRTUAL_MGR] = TRACE_OFF_LVL;	// 10 !
	TraceLevel[TRACE_VIRTUAL_MSTR] = TRACE_OFF_LVL;	// 11 @
	TraceLevel[TRACE_PTS] = TRACE_OFF_LVL;			// 12 #
	TraceLevel[TRACE_BOOT] = TRACE_ALL_LVL;			// 13 $
	TraceLevel[TRACE_FAILOVER] = TRACE_OFF_LVL;		// 14 %
	TraceLevel[TRACE_RMSTR] = TRACE_OFF_LVL;		// 15 ^
	TraceLevel[TRACE_SSAPI_MANAGERS] = TRACE_L1;	// 16 &
	TraceLevel[TRACE_PARTITIONMGR] = TRACE_L3;		// 17 *
	TraceLevel[TRACE_CHAOSFILE] = TRACE_L3;			// 18 (
	TraceLevel[TRACE_SSD] = TRACE_L3;				// 19 )
#endif

//Watch(WATCH1, (void *)&bootblock, 0x0800, WATCHSTORE);
//Watch(WATCH2, (void *)0xA01B7000, 0x0800, WATCHEXECUTE);

#ifdef KEYS
#ifdef START_OS
	// start the OS before key decoder
	Os::Initialize();
	fOSInitializeCalled = true;
#endif

	// loop forever
	task_0();
#else
	// start the OS
	Os::Initialize();
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

#if defined(_DEBUG)
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
				// Set the Global TraceLevel for all 20 to max
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					TraceLevel[index[idx]] = TRACE_ALL_LVL;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
				
			case 'o':
				// Set the Global TraceLevel for all 20 to off
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					TraceLevel[index[idx]] = TRACE_OFF_LVL;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
				
			case 'm':
				// increase the TraceLevel for our defined trace indices
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					if (TraceLevel[index[idx]] > TRACE_OFF_LVL)
						TraceLevel[index[idx]]--;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
			case 'M':
				// decrease the TraceLevel for our defined trace indices
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					if (TraceLevel[index[idx]] < TRACE_ALL_LVL)
					TraceLevel[index[idx]]++;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
#endif

			// Oos specific command extensions
			case 'h':
				// Initialize the PCI bridges.  IOPs need this to talk to us.
				// The slot map.
				printf("\n In Hex:\n");
				printf("13 12 11 10       19 1A 1B 18\n");
				printf("            00 01 \n");
				printf("17 16 15 14       1D 1E 1F 1C\n");
				printf("\n Decimal:\n");
				printf("19 18 17 16       25 26 27 24\n");
				printf("            00 01 \n");
				printf("23 22 21 20       29 30 31 28\n");
				printf("\n English:\n");
				printf("B4 B3 B2 B1       A1 A2 A3 A4\n");
				printf("            00 01 \n");
				printf("D4 D3 D2 D1       C1 C2 C3 C4\n");
				UI64 slotnum = Get_Hex("\nEnter slot# to Init: ");
				init_iop((U32)slotnum);
				printf("\n");
				break;
				
			case 'q':
				EnterIPAddr();				
				break;

			case 'f':
				if (!fOSInitializeCalled)
					printf("\nError: Can't Flush!  Os::Initialize() has not yet been called.");
				else
				if (!DdmPTS::pDdmPTS)
					printf("\nError: Can't Flush!  DdmPTS::pDdmPTS has not been initialized.");
				else
				{
					printf("\nCalling FlushToFlash()... ");
					STATUS status = DdmPTS::pDdmPTS->FlushToFlash();
					printf("\nFlushToFlash() done.  Status = %x.", status);
				}			
			   	break;

			case '~':  // Allow creation of the FSS 
				if (fOSInitializeCalled)
					printf("\nError: Can't set create boot option once Os::Initialize() has been called.");
				else
				{
					bddPartitionMgr.allowCreate = !bddPartitionMgr.allowCreate;
					if (bddPartitionMgr.allowCreate)
						printf("The HBC Partition Manager WILL now call FF_Create if necessary on new HBCs\n");
					else
						printf("The HBC Partition Manager WILL NOT call FF_Create on new HBCs\n");
				}			
				break;
				
			case 'I':
				if (fOSInitializeCalled)
					printf("\nError: Can't set reformat boot option once Os::Initialize() has been called.");
				else
				{
					bddPartitionMgr.reformat = !bddPartitionMgr.reformat;
					fLoadFromFlash = !bddPartitionMgr.reformat;
				}			
				
            case 'i':
            	if (fOSInitializeCalled)
					printf("\nError: Os::Initialize() has already been called.\n");
				else
				{
	            	// Make sure ip address has been set.
					if (!fbox_IP_address_set)
					{
						Tracef("\nA debug IP Address has not been set...\n");
						Tracef("Entering an IP address now will place it into PTS.\n");						
						Tracef("To boot using the IP currently in PTS, press [Enter] four\n");												
						Tracef("times to bypass this debug IP address.\n");																		
						EnterIPAddr();
					}		
					printf("\nReformat boot option set to %d.\n", bddPartitionMgr.reformat);
					printf("\nPTS fLoadFromFlash set to %d.\n", fLoadFromFlash);
				    printf("\nCalling OS:Initialize()...\n");
					fOSInitializeCalled = true;
					Os::Initialize();
					printf("\n... OS:Initialize() complete.\n");
				}
            	break;
            	
            case 'j':
				// The slot map.
				printf("\n In Hex:\n");
				printf("13 12 11 10       19 1A 1B 18\n");
				printf("            00 01 \n");
				printf("17 16 15 14       1D 1E 1F 1C\n");
				printf("\n Decimal:\n");
				printf("19 18 17 16       25 26 27 24\n");
				printf("            00 01 \n");
				printf("23 22 21 20       29 30 31 28\n");
				printf("\n English:\n");
				printf("B4 B3 B2 B1       A1 A2 A3 A4\n");
				printf("            00 01 \n");
				printf("D4 D3 D2 D1       C1 C2 C3 C4\n");
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
        		printf("%c", key);
                break;

	            }  /* switch (key = getchar()) */

		NU_Sleep(cMsDelay);
		cMsDelay=25;
		}  /* while(1) */
	}


void DrawScreen() {
	printf("%s", clrscn);
	U32 L = 1;
    printf_at(L++, 10, "*****************************************************************");
#ifndef SYSTEM_EXERCISER
    printf_at(L++, 10, "*** Gemini Storage Domain Manager Debug Console (HBC) v1.0    ***");
#else
    printf_at(L++, 10, "*** Gemini Storage Domain Manager SysEx Console (HBC) v1.0    ***");
#endif
    printf_at(L++, 10, "***                        Menu Keys                          ***");
	printf_at(L++, 10, "*** I: Format Flash (erasing all configuration data) & Boot.  ***");
	printf_at(L++, 10, "*** i: Boot the Gemini using existing configuration data.     ***");
	printf_at(L++, 10, "*** j: Display a slot number map.                             ***");
	#if false
	printf_at(L++, 10, "*** n: Display Interrupt counters.                            ***");
	printf_at(L++, 10, "*** b: Display Galileo Base Registers.                        ***");
	printf_at(L++, 10, "*** D: Display Galileo DMA Registers.                         ***");
	printf_at(L++, 10, "*** p: Display Galileo PCI Registers.                         ***");
	printf_at(L++, 10, "*** u: Galileo PCI internal registers.                        ***");
	#endif
	#ifdef _DEBUG
	printf_at(L++, 10, "*** 1-9,!-): Set output for any predefined TRACE_INDEX types  ***");
	printf_at(L++, 10, "*** a: Set TRACE_LVL_ALL for 20 predefined TRACE_INDEX types  ***");
	printf_at(L++, 10, "*** m: Decrease TRACE_LVL for 20 predefined TRACE_INDEX types ***");
	printf_at(L++, 10, "*** M: Increase TRACE_LVL for 20 predefined TRACE_INDEX types ***");
	printf_at(L++, 10, "*** o: Set TRACE_LVL_OFF for 20 predefined TRACE_INDEX types  ***");
	#endif
	printf_at(L++, 10, "*** h: call  init_iop for any particular IOP slot number.     ***");
	printf_at(L++, 10, "*** H: call  init_Hardware to reinitialize all PCI bridges.   ***");
	printf_at(L++, 10, "*** X: exit                                                   ***");
	printf_at(L++, 10, "*****************************************************************\r\n");
}

void EnterIPAddr()
{
fbox_IP_address_set = true;
// set the IP of the box
box_IP_address[0] = (unsigned char)Get_U32("\nEnter IP, 1st byte :");
box_IP_address[1] = (unsigned char)Get_U32("\nEnter IP, 2nd byte :");
box_IP_address[2] = (unsigned char)Get_U32("\nEnter IP, 3rd byte :");
box_IP_address[3] = (unsigned char)Get_U32("\nEnter IP, 4th byte :");
printf("\nThe IP will be set to %d.%d.%d.%d", box_IP_address[0], box_IP_address[1], box_IP_address[2], box_IP_address[3] );
}


#endif	// KEYS


