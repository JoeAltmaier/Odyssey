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
//	$Log: /Gemini/Odyssey/Hbc Ethernet/HbcMenu.cpp $
// 
// 3     10/20/99 5:42p Iowa
// Added DdmProfile
// 
// 1     8/24/99 6:33p Jaltmaier
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

#include "stdio.h"
#include "OsTypes.h"

// Debugging is turned on for now
#define _DEBUG
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_APP_INIT
#include "Odyssey_Trace.h"

#include "Pci.h"			// need the BYTE_SWAP() macros for everybody
#include "Odyssey.h"		// Systemwide parameters

#include "HbcMenu.h"
#include "Pci.h"

#include "Os.h"
#include "Ddm.h"
#include "Interrupt.h"

#include "TestDdm.h"

//#include "pcidev.h"

#define	KEYS			// turn on the key decoder
//#define	START_OS		// start CHAOS before entring the key decoder


/*************************************************************************/
// External References
/*************************************************************************/
extern "C" STATUS init_hbc();				// defined in Drivers/inithd.c.  Need a header.
extern "C" STATUS init_iop(U32 slotnum);	// defined in Drivers/inithd.c.  Need a header.


/*************************************************************************/
// Forward References
/*************************************************************************/
void	DrawScreen();
void    printf_at(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
extern	"C" int ttyA_in();
UI64 Get_Hex(char *p_message);


extern "C" {
void   	StartTask(UNSIGNED argc, VOID *argv);
}


/*************************************************************************/
// Globals
/*************************************************************************/

void		task_0(void);

// useful ascii escape sequences
char		*clreol = "\033[K";                /* clear to the end of the line */
char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
char		*bspace = "\033[D \033[D";         /* backspace */

char		fVerbose = false;
char		fOSInitializeCalled = false;

#if defined(_DEBUG)
// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
#define		NUM_AUTO_TRACES 9			// MUST be # of entries in following array.
unsigned	index[NUM_AUTO_TRACES] =	// pick the arrray indexes that will be used
			{	TRACE_APP_INIT,
				TRACE_MESSENGER1,
				TRACE_OOS,
				TRACE_DDM_MGR,
				TRACE_DDM,
				TRACE_TRANSPORT,
				TRACE_VIRTUAL_MGR,
				TRACE_PTSPROXY,
				TRACE_MESSAGE
			};
#endif


/*************************************************************************/
// StartTask
// Come here when Nucleus has finished initializing.  Startup the Target
// and/or the Initiator Tasks
/*************************************************************************/
void   StartTask(UNSIGNED argc, VOID *argv)
{
#pragma unused(argc)
#pragma unused(argv)
	
//	x = Init_Hardware();
	
#if defined(_DEBUG)
	// All levels set to 0 by default
//	TraceLevel[TRACE_APP_INIT] = TRACE_ALL_LVL;		// set trace levels for debug
//	TraceLevel[TRACE_DDM_MGR] = TRACE_L7;
//	TraceLevel[TRACE_TRANSPORT] = TRACE_ALL_LVL;
//	TraceLevel[TRACE_MESSAGE] = TRACE_L4;
	//TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;		//TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;	
	//TraceLevel[TRACE_BSA] = TRACE_L2;	
	//TraceLevel[TRACE_FCP] = TRACE_L2;	
	//TraceLevel[TRACE_FCP_TARGET] = TRACE_L2;	
	//TraceLevel[TRACE_FCP_INITIATOR] = TRACE_L2;	
	//TraceLevel[TRACE_HEAP1] = TRACE_L1;	
	//TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;	
#endif


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
            case '!':   /* ! - cause address exception and return to boot code */
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
				printf("\n\rTraceLevel[%d] = %d",
							index[idx], TraceLevel[index[idx]]);
				break;

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
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					TraceLevel[index[idx]] = TRACE_ALL_LVL;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
				
			case 'm':
				// Set the Global TraceLevel for all 10 to min
				for (idx = 0; idx < NUM_AUTO_TRACES; idx++)
				{
					TraceLevel[index[idx]] = TRACE_L2;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
#endif

			case 'r':
			case 'R':
				printf("Sgl test started\n");
				TestDdm::StartSgl();
				break;

            case 's':
            case 'S':
				printf("Stress test started\n");
				TestDdm::StartStress();
				break;

			case 'z':
				printf("Profile ON\n");
				TestDdm::StartProfile();
				break;

			case 'x':
				printf("Profile OFF\n");
				TestDdm::StopProfile();
				break;

			case 'c':
				printf("Profile CLEAR\n");
				TestDdm::ClearProfile();
				break;

			// Oos specific command extensions
			case 'h':
				// Initialize the PCI bridges.  IOPs need this to talk to us.
				printf("\n");
				printf("13 12 11 10       19 1A 1B 18\n");
				printf("            00 01 \n");
				printf("17 16 15 14       1D 1E 1F 1C\n");
				I64 slotnum = Get_Hex("\nEnter slot# to Init: ");
				init_iop((U32)slotnum);
				printf("\n");
				break;

            case 'i':
            	if (fOSInitializeCalled)
					printf("\nError: Os::Initialize() has already been called.");
				else
				{
				    printf("\nCalling OS:Initialize()... ");
					Os::Initialize();
					printf("\n... OS:Initialize() complete.");
				}
            	break;
            	
            case 'j':
            	printf("\n");
				printf("13 12 11 10       19 1A 1B 18\n");
				printf("            00 01 \n");
				printf("17 16 15 14       1D 1E 1F 1C\n");
				printf("\n");

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
    printf_at(2, 10, "***  ConvergeNet Technologies - Odyssey 2000 Demonstration  ***");
    printf_at(3, 10, "***                        HBC Image                        ***");
	printf_at(4, 10, "*** i = Init  s - stress test  r - sgl test                 ***");
	printf_at(6, 10, "***************************************************************\r\n");
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

#endif	// KEYS


