/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDdm.cpp
// 
// Description:
// This file implements the Device Dependent Module for the
// Solid State Drive.
// 
// Update Log
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/
#define _ODYSSEY			// target odyssey (not eval)

#define _DEBUG
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_SSD1
#include "Odyssey_Trace.h"

#include "SsdDDM.h"
#include "BuildSys.h"
#include "ErrorLog.h"
#include "PciDev.h"
#include "RequestCodes.h"
#include "LED.h"

// The SSD is a SINGLE DDM -- only one instance of a SSD per IOP.
CLASSNAME(SSD_Ddm, SINGLE);	// Class Link Name used by Buildsys.cpp

/*************************************************************************/
// KEYS turns on the keyboard decoder for debugging.
/*************************************************************************/
#define	KEYS			
#ifdef KEYS
#include "stdio.h"
#include "Galileo.h"
#include "pci.h"
#define	START_OS		// start CHAOS before entring the key decoder
void		task_0(void);
#endif // KEYS

char		fOSInitializeCalled = false;

/*************************************************************************/
// Break here for debugging.  Come here when ESC key is hit.
// Active when KEYS is defined.
/*************************************************************************/
void		Break(void);
void		Break()
{
	// Set break here
}

/*************************************************************************/
// SSD_Ddm static variables
/*************************************************************************/
SSD_Device	SSD_Ddm::m_device;

// Temporary pointer to object so that keyboard Close command can work.
SSD_Ddm *p_SSD_ddm;

#if defined(_DEBUG)
// Initialize the TraceLevel[] array remotely
//extern		long TraceLevel[200] = {0};
unsigned	idx = 0;
unsigned	index[10] =	// pick the array indexes that will be used
			{	TRACE_APP_INIT,
				TRACE_SSD1,
				TRACE_SSD,
				TRACE_TRANSPORT,
				TRACE_DRIVE_MONITOR,
				TRACE_SCSI_TARGET_SERVER,
				TRACE_BSA,
				TRACE_DDM,
				TRACE_DDM_MGR,
//				TRACE_PTS,
				TRACE_MESSENGER1
			};
#endif

/*************************************************************************/
// SSD_Ddm constructor.
// Get our table data from the persistent data store.
/*************************************************************************/
SSD_Ddm::SSD_Ddm(DID did): Ddm(did)
{
	// Tell the base class Ddm where my config area is.
	SetConfigAddress(&m_config, sizeof(m_config));

	MyVdn = GetVdn();
	MyDid = GetDid();

	// TEMPORARY save pointer to object for debugging.
	p_SSD_ddm = this;

} // SSD_Ddm
	
/*************************************************************************/
// SSD_Ddm Ctor -- Create an instance of this DDM and return a pointer to it.
/*************************************************************************/
Ddm * SSD_Ddm::Ctor(DID did)
{
	return new SSD_Ddm(did);
} // SSD_Ddm

/*************************************************************************/
// SSD_Ddm::Initialize -- gets called only once.
/*************************************************************************/
STATUS SSD_Ddm::Initialize(Message *p_message)	// virtual
{
	// Initialize callback context to be used for quiesce.
	m_request_context.Initialize();

	// Tell Chaos what messages to send us.
	DispatchRequest(BSA_BLOCK_READ, (RequestCallback) &Process_BSA_Request);
	DispatchRequest(BSA_BLOCK_WRITE, (RequestCallback) &Process_BSA_Request);
	DispatchRequest(BSA_DEVICE_RESET, (RequestCallback) &Process_BSA_Request);
	DispatchRequest(BSA_STATUS_CHECK, (RequestCallback) &Process_BSA_Request);
	DispatchRequest(BSA_POWER_MANAGEMENT, (RequestCallback) &Process_BSA_Request);
	DispatchRequest(BSA_MEDIA_FORMAT, (RequestCallback) &Process_Format_Request);

	DispatchRequest(PHS_RESET_STATUS, (RequestCallback) &Process_PHS_Request);
	DispatchRequest(PHS_RETURN_STATUS, (RequestCallback) &Process_PHS_Request);
	DispatchRequest(PHS_RESET_PERFORMANCE, (RequestCallback) &Process_PHS_Request);
	DispatchRequest(PHS_RETURN_PERFORMANCE, (RequestCallback) &Process_PHS_Request);
	DispatchRequest(PHS_RETURN_RESET_PERFORMANCE, (RequestCallback) &Process_PHS_Request);

	// Initialize the PHS members
	memset(&m_Status, 0, sizeof(SSD_STATUS));
	memset(&m_Performance, 0, sizeof(SSD_PERFORMANCE));

	// Get our table data from the Persistent Data Store
    Ssd_Table_Initialize(p_message);

	return OS_DETAIL_STATUS_SUCCESS;

} // SSD_Ddm::Initialize

/*************************************************************************/
// StartTask.
/*************************************************************************/
extern "C" void  StartTask(UNSIGNED argc, VOID *argv);
void  StartTask(UNSIGNED , VOID *)
{
//	Status status = Init_Hardware(); - not needed, MSL_Initialize does this earlier.

#if 1
	// All levels set to 0 by default
	TraceLevel[TRACE_SSD1] = TRACE_L3;
	TraceLevel[TRACE_SSD] = TRACE_L3;	
	TraceLevel[TRACE_APP_INIT] = TRACE_ALL_LVL;		// set trace levels for debug
	TraceLevel[TRACE_DDM_MGR] = TRACE_OFF_LVL;
	TraceLevel[TRACE_DRIVE_MONITOR] = TRACE_L2;
	TraceLevel[TRACE_SCSI_TARGET_SERVER] = TRACE_L2;
	TraceLevel[TRACE_BSA] = TRACE_L2;
	//TraceLevel[TRACE_MESSENGER1] = TRACE_L2;
	//TraceLevel[TRACE_DDM_MGR] = TRACE_L2;
	//TraceLevel[TRACE_DDM] = TRACE_L2;
	TraceLevel[TRACE_HEAP1] = TRACE_L1;
	//TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;
#endif

	Os::DumpTables("In StartTask...");

#ifdef KEYS
#ifdef START_OS
	// start the OS before key decoder
	Oos::Initialize();
	fOSInitializeCalled = true;
	printf("\n... OS:Initialize() complete.\n");
#endif // START_OS

	// loop forever
	task_0();
#else
	// start the OS
	Oos::Initialize();
#endif

} // StartTask


#ifdef KEYS
#define EOL "\n"

/*************************************************************************/
// Forward References
/*************************************************************************/
void	DrawScreen();
void    printf_at(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
extern	"C" int ttyA_in();
extern	"C" int ttyA_poll();
extern  "C" void galileo_regdump();

/*************************************************************************/
// task_0
// Dummy task to allow keyboard input while system is running
/*************************************************************************/
void   task_0(void) 
{
	unsigned  key = 0;
	unsigned  i;
	unsigned  cMsDelay=1;
	char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */

	TRACE_ENTRY(task_0);

	// start on a fresh line
	printf("\n\r");

	DrawScreen();

	/* Task0 forever loop */
	while(1) 
	{
			cMsDelay=1;

            switch (key = ttyA_in()) 
            {

            case -1:
            	break;			// no char available

            // The first set of commands are general purpose commands
            // for all projects
            case 'X':   /* X - cause address exception and return to boot code */
				printf(EOL, "Exit with exception");
                unsigned long *d = ( unsigned long * ) 0x1;
                *d = 0xFFFF0000;
                break;

            case 'x':
            	// x - clear ISP Interrupts then cause address exception
            	// and return to boot code
                unsigned long *g = ( unsigned long * ) 0x1;
                *g = 0xFFFF0000;
                break;

            case ' ':  /* SPACEBAR - redraw the screen */
				DrawScreen();
                break;

            case 0x1B:  /* ESC */
        		printf(EOL, "Break");
        		Break();
                break;

            case 0x0D:  /* ENTER */
        	case 0x0A:  /*  or the real ENTER */
				printf("\n\r");
                break;

#if 0
			case '0':
				CPU_LED_1_GREEN;
				break;
			case '1':
				CPU_LED_1_RED;
				break;
			case '!':
				CPU_LED_1_OFF;
				break;
			case '2':
				CPU_LED_1_YELLOW;
				break;
			case '3':
				CPU_LED_2_GREEN;
				break;
			case '4':
				CPU_LED_2_RED;
				break;
			case '@':
				CPU_LED_2_OFF;
				break;
			case '5':
				CPU_LED_2_YELLOW;
				break;
			case '6':
				CPU_LED_3_GREEN;
				break;
			case '7':
				CPU_LED_3_RED;
				break;
			case '8':
				CPU_LED_3_YELLOW;
				break;
			case '#':
				CPU_LED_3_OFF;
				break;
#endif
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
				for (idx = 0; idx < 10; idx++)
				{
					TraceLevel[index[idx]] = TRACE_ALL_LVL;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;

			case 'm':
				// Set the Global TraceLevel for all 10 to min
				for (idx = 0; idx < 10; idx++)
				{
					TraceLevel[index[idx]] = TRACE_L2;
					printf("\n\rTraceLevel[%d] = %d",
								index[idx], TraceLevel[index[idx]]);
				}
				break;
#endif

            case 'i':
            case 'I':
				if (fOSInitializeCalled)
					printf("\nError: Can't set reformat boot option once Os::Initialize() has been called.");
				else
				{
	            	TRACE_HEX(TRACE_L2, "\n\rOs:Initialize called ", (U32)Os::Initialize);
					TRACE_STRING(TRACE_L2, "\n\r");
					fOSInitializeCalled = true;
					Os::Initialize();
					printf("\n... OS:Initialize() complete.\n");
				}
            	break;

            case 'b':
            case 'B':
				printf(EOL,"Base registers");
			    Print_Dump((unsigned long*)GT_CPU_CONFIG_REG, 64);
                break;

			case 'd':
				printf(EOL,"I2O registers");
			   	Print_Dump((unsigned long*)(GT_I2O_BASE), 32);
			   	break;

			case 'D':
				printf(EOL,"DMA registers");
			   	Print_Dump((unsigned long*)(GT_DMA_CH0_COUNT_REG), 32);
			   	break;

            case 'e':
            case 'E':
				if (!fOSInitializeCalled)
				{
					fOSInitializeCalled = true;
					Os::Initialize();
					printf("\n... OS:Initialize() complete.\n");
				}
				if (p_SSD_ddm)
				{
					printf(EOL,"Enable flash file system");
					p_SSD_ddm->Send(p_SSD_ddm->GetDid(), new Message(REQ_OS_DDM_ENABLE));
				}
				else
					printf(EOL,"p_SSD_ddm is NULL");
                break;

			case 'p':
			case 'P':
				printf(EOL,"PCI registers");
			   	Print_Dump((unsigned long*)(GT_PCI_COMMAND_REG), 64);
			   	break;

            case 'q':
            case 'Q':
				printf(EOL,"Quiesce flash file system");
				p_SSD_ddm->Send(p_SSD_ddm->GetDid(), new Message(REQ_OS_DDM_QUIESCE));
                break;

			case 'u':
				printf(EOL,"PCI internal registers");
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

			case 'r':
			{
				U32 id;
				S32 reg;

				printf(EOL,"ISP 1 PCI registers");
				id = get_isp_pci_id();
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

            case 'g':
        	// dump the Galileo PCI Registers
        	{
        		U32 pci_id;
        		printf("Galileo PCI Registers\n");
        		pciconf_print(0x80000000);
        		pci_id = get_pci_id(0, ISP_ID, 0);
        		printf("ISP PCI Registers\n");
        		pciconf_print(pci_id);
        	}
        	break;

            case 'f':
            case 'F':
				printf(EOL,"Display flash statistics");
				FF_Display_Stats_And_Events(p_SSD_ddm->Flash_Handle());
                break;

            case 'c':
            case 'C':
				printf(EOL,"Display Cache statistics");
				FF_Display_Cache_Stats_And_Events(p_SSD_ddm->Flash_Handle());
                break;

            default:
        		printf("%c", key);
                break;
        }  /* switch (key = getchar()) */

		NU_Sleep(cMsDelay);
		cMsDelay=25;
		}  /* while(1) */
	}


void
printf_at (int row, int col, char *string)
{
    printf("\033[%d;%dH%s", row, col, string);
}

void DrawScreen() 
{
	char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */

	printf("%s", clrscn);
    printf_at(1, 10, "***************************************************************");
#ifndef SYSTEM_EXERCISER
    printf_at(2, 10, "***    ConvergeNet Technologies - Odyssey 2000 SSD Image    ***");
#else
    printf_at(2, 10, "***    ConvergeNet Technologies - Odyssey 2000 SSD SysEx    ***");
#endif
	printf_at(3, 10, "***                     Slot = x                            ***");
	printf_at(3, 41, " ");
	printf("%d", bootblock.b_slot);
    printf_at(4, 10, "***************************************************************");
	printf(EOL "E - Enable (after quiesce)");
	printf(EOL "Q - Quiesce");
	printf(EOL "I - init");
	printf(EOL "X - Exit with exception");
	printf(EOL "F - Flash Statistics ");
	printf(EOL "C - Cache Statistics ");
	printf(EOL "ESC - break");
	printf(EOL "SPACEBAR - redraw the screen");

} // DrawScreen

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
      printf("");
   }

} // Print_Dump


#endif // KEYS