#define _DEBUG
#include "Os.h"
#include "Galileo.h"
#include "Ddm.h"
#include "Interrupt.h"
#include "DdmManager.h"
#include "Transport.h"
#include "TestDdm.h"
#include "OsHeap.h"
#include "pcimap.h"

#include "stdio.h"

extern "C" {
void    exit(int status);
void	DrawScreen();
void    StartTask(UNSIGNED argc, VOID *argv);
char _etext[];
void	bzero(void*, U32);
void	bcopy(void*, void*, U32);
}

void	PrintAt (int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);

   /* useful ascii escape sequences */
   char *clreol = "\033[K";                /* clear to the end of the line */
   char *clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
   char *bspace = "\033[D \033[D";         /* backspace */

/*
char CallbackProc(void *id);

char CallbackProc(void *id) {
	printf((char*)id);
	return TRUE;
	}
*/

	U32 gTimeStart;

	U32 gTimeSendRemote;
	U32 gTimeDmaTransfer;
	U32 gTimeDmaGo;
	U32 gTimeDmaInt;
	U32 gTimeSendFixup;
	U32 gTimePost;
	U32 gTimeInt;
	U32 gTimeDone;
	U32 gTimeSetup;
	U32 gTimeAlloc;
	U32 gTimeAddress;
	U32 gTimeGlobal;

void Fault(int i, int cb);
void Fault(int i, int cb) { printf("bzero or bcopy failed! i=%d cb=%d\n", i, cb); }

void   StartTask(UNSIGNED , VOID *) {
	unsigned  key = 0;
	unsigned  i;
	unsigned  cMsDelay=1;

#ifdef _DEBUG
	TraceLevel[TRACE_OOS]=0;
	TraceLevel[TRACE_DDM_MGR]=0;
	TraceLevel[TRACE_DDM]=0;
	TraceLevel[TRACE_HEAP]=0;
	TraceLevel[TRACE_VIRTUAL_MGR]=0;
	TraceLevel[TRACE_TRANSPORT]=0;
#endif

	DrawScreen();
	struct CabSlot {U32 iCabinet; TySlot iSlot;} *pCabSlot = (CabSlot*)Os::GetBootData("DdmManager");

	printf("Slot %s initializing...", St_Slot(pCabSlot->iSlot));
	Os::Initialize();
	printf("done\n");

	/* Task0 forever loop */
	while(1) {

		if (true) {
			cMsDelay=1;

            switch (key = getchar()) {
            case ' ':  /* SPACEBAR - redraw the screen */
				DrawScreen();
                break;

			case '!':
				printf("Heap test\n");
				char *pChar=new (tSMALL) char[128];
				pChar[128]='\000';
				pChar[-1]='\000';
				pChar=new (tSMALL) char[128];
				break;

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

			case 'c':
			case 'C':
				printf("cTransfer=%d\n", cTransfer);
				printf("SendRemote=%d DmaTransfer=%d DmaGo=%d DmaInt=%d SendFixup=%d Post=%d Int=%d Done=%d\n",
				gTimeSendRemote, gTimeDmaTransfer, gTimeDmaGo, gTimeDmaInt, gTimeSendFixup, gTimePost, gTimeInt, gTimeDone);
				break;
				
            case 'i':
            case 'I':
				Oos::Initialize();
            	break;

            case 'n':
            case 'N':
				PrintAt(7,0,"Interrupt counters\n");
				for (i=Interrupt::tyFirst; i < Interrupt::tyLast; i++) {
					printf("%x ", (int)Interrupt::aN[i]);
					}
				break;

			case 'v':
				// Memory test
				printf("total memory %08lx free memeory %08lx\n", OsHeap::heapSmall.cbAllocTotal, OsHeap::heapSmall.CbFree());
				for (int cb=0; cb <= 0x10000; cb++) {
					void *p=new char[cb];
					if (p == NULL)
						printf("Failed to allocate %d bytes\n", cb);
					else
						delete p;
					}
				printf("total memory %08lx free memeory %08lx\n", OsHeap::heapSmall.cbAllocTotal, OsHeap::heapSmall.CbFree());
				break;
				
			unsigned char aData[256];
			unsigned char aDest[256];
			case 'q':
				printf("bzero test\n");
				for (int i=0; i < 16; i++) {
					printf("offset %d\n", i);
					for (int cb=0; cb < 128; cb++) {
						for (int ib=0; ib < sizeof(aData); ib++) aData[ib]=0xFF;
						bzero(&aData[i+16], cb);
						// Test
						for (int ib=0; ib < i+16; ib++)
							if (aData[ib] != (unsigned char)0xFF)
								Fault(i, cb);
						for (int ib=i+16; ib < i+16+cb; ib++)
							if (aData[ib] != 0)
								Fault(i, cb);
						for (int ib=i+16+cb; ib < sizeof(aData); ib++)
							if (aData[ib] != (unsigned char)0xFF)
								Fault(i, cb);
						}
					}
				printf("bzero test done\n");


				printf("bcopy test\n");
				for (int i1=0; i1 < 16; i1++) {
				for (int i2=0; i2 < 16; i2++) {
					printf("offset %d - %d\n", i1, i2);
					for (int cb=0; cb < 128; cb++) {
						for (int ib=0; ib < sizeof(aData); ib++) { aData[ib]=ib; aDest[ib]=0xFF; }
if (i1 == 5 && i2 == 1 && cb == 12)
printf("12!");
						bcopy(&aData[i1+16], &aDest[i2+16], cb);
						// Test
						for (int ib=0; ib < i2+16; ib++)
							if (aDest[ib] != (unsigned char)0xFF)
								Fault(i2, cb);
						int iTest=i1+16;
						for (int ib=i2+16; ib < i2+16+cb; ib++)
							if (aDest[ib] != iTest++)
								Fault(i2, cb);
						for (int ib=i2+16+cb; ib < sizeof(aDest); ib++)
							if (aDest[ib] != (unsigned char)0xFF)
								Fault(i2, cb);
						}
					}
					}
				printf("bcopy test done\n");
				break;
				
            case 'a':
            case 'A':
				PrintAt(7,0,"All GT64120 registers\n");
				printf("Base registers\n");
			    Print_Dump((unsigned long*)GT_CPU_CONFIG_REG, 72);
				printf("SDRAM registers\n");
			   	Print_Dump((unsigned long*)(GT_DRAM_RAS0_LO_REG), 32);
				printf("DMA registers\n");
			   	Print_Dump((unsigned long*)(GT_DMA_CH0_COUNT_REG), 32);
				printf("PCI registers\n");
			   	Print_Dump((unsigned long*)(GT_PCI_COMMAND_REG), 64);
				printf("I2O registers\n");
			   	Print_Dump((unsigned long*)(GT_I2O_BASE), 32);
                break;

            case 'b':
            case 'B':
				PrintAt(7,0,"Base registers\n");
			    Print_Dump((unsigned long*)GT_CPU_CONFIG_REG, 64);
                break;

			case '0':
				PrintAt(7,0,"I2O registers\n");
			   	Print_Dump((unsigned long*)(GT_I2O_BASE), 32);
			   	break;
			   	
			case '2':
				PrintAt(7,0,"I2O registers, HBC0\n");
			   	Print_Dump((unsigned long*)0xa2000000, 32);
			   	break;
			   	
			case '3':
				PrintAt(7,0,"I2O registers, HBC1\n");
			   	Print_Dump((unsigned long*)0xa3000000, 32);
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
			{
				char *p=(char*)0xa331c000;
				int cb=8192;
				int iStart=-1;
				printf("PCI test to %08lx[%d]\n", p, cb);
				char aCh[8192];
				for (int i=0; i < sizeof(aCh); i++)
					aCh[i]=i;

				printf("byte move:");
				for (int i=0; i < sizeof(aCh); i++)
					p[i]=aCh[i];
				for (int i=0; i < sizeof(aCh); i++)
					if (p[i] != aCh[i]) {
						if (iStart == -1)
							iStart=i;
						}
					else if (iStart != -1) {
						printf("bad data from %08lx to %08lx\n", p+iStart, p+i);
						iStart=-1;
						}

				printf("...done\nshort move:");
				for (int i=0; i < sizeof(aCh)/sizeof(short); i++)
					((short*)p)[i]=((short*)&aCh[0])[i];
				for (int i=0; i < sizeof(aCh)/sizeof(short); i++)
					if (((short*)p)[i] != ((short*)&aCh[0])[i]) {
						if (iStart == -1)
							iStart=i;
						}
					else if (iStart != -1) {
						printf("bad data from %08lx to %08lx\n", p+iStart*sizeof(short), p+i*sizeof(short));
						iStart=-1;
						}

				printf("...done\nlong move:");
				for (int i=0; i < sizeof(aCh)/sizeof(long); i++)
					((long*)p)[i]=((long*)&aCh[0])[i];
				for (int i=0; i < sizeof(aCh)/sizeof(long); i++)
					if (((long*)p)[i] != ((long*)&aCh[0])[i]) {
						if (iStart == -1)
							iStart=i;
						}
					else if (iStart != -1) {
						printf("bad data from %08lx to %08lx\n", p+iStart*sizeof(long), p+i*sizeof(long));
						iStart=-1;
						}
				printf("...done\nI64 move:");

				printf("I64 move:");
for (int loop=0; loop < 1024; loop++) {
				for (int i=0; i < sizeof(aCh)/sizeof(I64); i++)
					((I64*)p)[i]=((I64*)&aCh[0])[i];
				for (int i=0; i < sizeof(aCh)/sizeof(I64); i++)
					if (((I64*)p)[i] != ((I64*)&aCh[0])[i]) {
						if (iStart == -1)
							iStart=i;
						}
					else if (iStart != -1) {
						printf("bad data from %08lx to %08lx\n", p+iStart*sizeof(I64), p+i*sizeof(I64));
						iStart=-1;
						}
}
				printf("...done\n");
				}
				break;

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
//            	printf("%c", key);
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
	printf("          B - base registers  012 - I2O registers  D - DMA registers  P - PCI registers\n");
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
   	  printf("%4x: ", (i*4 + (((int)p) & 0xFFFF)));
      for (j=0; j < 4; j++)
      {
        char fNMI=(((int)p & 0xFFFF) == 0xC00 && (i+j==12 || i+j == 13 || i+j == 61 || i+j == 63));
        unsigned long l=(fNMI? 0 :p[i+j]);
        if (fNMI)
	        printf("XXXXXXXX ");
	    else
			printf("%8lx ", GTREG(l));
      }
      printf("\n");
   }
}
