#define _DEBUG
#include "Oos.h"
#include "Galileo.h"
#include "Ddm.h"
#include "Interrupt.h"
#include "DdmManager.h"
#include "Transport.h"
#include "MipsIo.h"
//#include "HeapNoFrag.h"
#include "TestDdm.h"
#include "Odyssey_Trace.h"
//#include "CallbackTimer.h"

#include "stdio.h"

extern "C" {
void    exit(int status);
void	DrawScreen();
void    task_0(UNSIGNED argc, VOID *argv);
}

void	PrintAt (int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);
void DebugOutput();

   /* useful ascii escape sequences */
   char *clreol = "\033[K";                /* clear to the end of the line */
   char *clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
   char *bspace = "\033[D \033[D";         /* backspace */

char CallbackProc(void *id);

char CallbackProc(void *id) {
	printf((char*)id);
	return TRUE;
	}

void   task_0(UNSIGNED , VOID *) {
	unsigned  key = 0;
	unsigned  i;
	unsigned  cMsDelay=1;

	TraceLevel=TRACE_ALL_LVL;
	
	DrawScreen();
	printf("This test loops, queuing messages to A then B then C.\n");
	printf("To do it with a different thread blocked on each queue, type 'i'.\n");
	printf("To do it with only one thread (no suspend/resumes at all), type 'j'.\n");

//DebugOutput();
	DdmManager::fQueue=true;
	HeapNoFrag::fDebug=false;
	
	/* Task0 forever loop */
	while(1) {

		if (true) {
			cMsDelay=1;

            switch (key = SccGetChar()) {
            case ' ':  /* SPACEBAR - redraw the screen */
				DrawScreen();
                break;

			case '+':
				HeapNoFrag::fDebug=!HeapNoFrag::fDebug;
				printf("Heap debug "); 
				printf(HeapNoFrag::fDebug? "on\n" :"off\n");
				break;

            case 's':
            case 'S':
				TestDdm::fStress = !TestDdm::fStress;
				if (TestDdm::fStress) 
					printf("Stress test on\n");
				else
					printf("Stress test off\n");
				break;

			case 'c':
			case 'C':
				printf("nQueueFault=%d\n", Transport::nQueueFault);
				printf("Dma::cTransfer=%d\n", Dma::cTransfer);
				break;
				
			case 'm':
			case 'M':
				// Message tests
				Message *pMsg=new MessagePrivate(0x77);
				printf("pMsg=%lx\n", pMsg);
				struct {
					int i;
					int j;
					char aCh[10];
					} payload={0x1234, 0x5678, "HELLO"};
				pMsg->AddPayload(&payload, sizeof(payload));
				printf("After AddPayload:\n");
			   	Print_Dump((unsigned long*)pMsg, 32);
				
				char aData[]="This is the data.";
				char aReply[128];
				pMsg->AddSgl(1, &aData, sizeof(aData), SGL_SEND);
				printf("After AddSgl(1):\n");
			   	Print_Dump((unsigned long*)pMsg, 32);

				pMsg->AddSgl(0, &aReply, sizeof(aReply), SGL_REPLY);
				printf("After AddSgl(0):\n");
			   	Print_Dump((unsigned long*)pMsg, 32);
			   	break;
				

			case 'j':
			case 'J':
				DdmManager::fQueue=false;
				
            case 'i':
            case 'I':
				Oos::Initialize();
            	break;


			case 'r':
			case 'R':
				printf("Read from a3000040:%lx\n", (*(long*)0xa3000040));
				break;
				
			case 'w':
			case 'W':
				printf("Write to a3000040\n");
				*(long*)0xa3000040=0;
				break;
				
            case 'n':
            case 'N':
				PrintAt(7,0,"Interrupt counters\n");
				for (i=Interrupt::tyFirst; i < Interrupt::tyLast; i++) {
					printf("%x ", (int)Interrupt::aN[i]);
					}
				break;
				
            case 'b':
            case 'B':
				PrintAt(7,0,"Base registers\n");
			    Print_Dump((unsigned long*)GT_CPU_CONFIG_REG, 64);
                break;

			case '2':
				PrintAt(7,0,"I2O registers\n");
			   	Print_Dump((unsigned long*)(GT_I2O_BASE), 32);
			   	break;
			   	
			case 'd':
			case 'D':
				PrintAt(7,0,"DMA registers\n");
			   	Print_Dump((unsigned long*)(GT_DMA_CH0_COUNT_REG), 32);
			   	break;
			   
			case 'p':
			case 'P':
				PrintAt(7,0,"PCI registers\n");
			   	Print_Dump((unsigned long*)(GT_PCI_COMMAND_REG), 64);
			   	break;
			   
			case 'u':
			case 'U':
				PrintAt(7,0,"PCI internal registers\n");
				for (i=0x80000000; i < 0x80000120; i+= 4) {
			    	if (i % 0x10 == 0) {
					    printf("\n%2x", (i & 0xFF));
					   	}
				*(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = GTREG(i);
				printf(" %lx", GTREG(*(unsigned long*)(GT_PCI_CONFIG_DATA_REG)));
				}

				break;

			case 't':
			case 'T':
				printf("task_0 creating timer callback, one second interval.\n");
				Callback *pCbk=new CallbackPProc(NULL, &CallbackProc, "Expire.\n");
				new CallbackTimer(pCbk, 1000000, 1000000);
				break;

            case '!':   /* ! - cause address exception and return to boot code */
            case 'x':   /* ! - cause address exception and return to boot code */
            case 'X':   /* ! - cause address exception and return to boot code */
				printf(clrscn);
				printf("Exit with exception\n\n");
                unsigned long *d = ( unsigned long * ) 0x1;
                *d = 0xFFFF0000;
                break;

            case 0x08:  /* BACKSPACE */
            case 0x09:  /* TAB */
            case 0x0D:  /* ENTER */
            case 0x1B:  /* ESC */
            	printf(" \007");
                break;

            default:
            	printf("%c", key);
                break;

	            }  /* switch (key = Get_Char()) */

    		}  /* if (Char_Present()) */
    		
		NU_Sleep(cMsDelay);
		cMsDelay=1;
		}  /* while(1) */
	}


void DrawScreen() {
	printf(clrscn);
    printf( "          ***************************************************************\n");
    printf( "          ***  ConvergeNet Technologies - Odyssey 2000 Demonstration  ***\n");
//	printf("          B - base registers  2 - I2O registers  D - DMA registers  P - PCI registers\n");
//	printf("          U - PCI internal  N - interrupt ctrs  X - exit\n");
//	printf("          *** I - init  + - toggle heap debug  V - verbose  X - exit  ***\n");
	printf("          ***************************************************************\n");

	printf("To exit, type X.\n");
	}

void
PrintAt (int row, int col, char *string)
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
   	  printf("%x: ", (i*4 + (((int)p) & 0xFFFF)));
      for (j=0; j < 4; j++)
      {
        char fNMI=(((int)p & 0xFFFF) == 0xC00 && (i+j==12 || i+j == 61));
        unsigned long l=(fNMI? 0 :p[i+j]);
        if (fNMI)
	        printf("XXXXXXXX ");
	    else
			printf("%lx ", GTREG(l));
      }
      printf("\n");
   }
}

void DebugOutput() {
char buf[512];
			int i=99;
			sprintf(buf, "%i ITERATION \n", i);
			SccPutStr(buf);
}
