/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: init.c
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
#include  "pcimap.h"
#include  "pcidev.h"
#include  "gt.h"
#include  "pcialloc.h"
#include  "que.h"


extern	unsigned long long *hbc_ctl;

#define	READ_SUPPORT	1

extern	int	not_an_hbc;
#define		DEBUG	1
#define		BANK2	1

typedef	struct	_btest {
	int	 slot_no;
	int  op_type;
	int	 active;
	char key_in;
} btest_t;
char	*verify[2] = { "Off", "On" };
btest_t		board_list[MAX_PCI_SLOTS];
int			prev_val[MAX_PCI_SLOTS];
int			board_list_count = 0;
void 		(*o_lih)(int);
void		m_lih(int);
/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;


extern U32 board_type;
int	tty_inited = 0;
/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);



STATUS init_buf(int slot_no);
void create_board_list(int type);



#define	DMABUF_ALIGN_SIZE	512
#define	INC		1
#define	DEC		2

/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
void    Application_Initialize(void *first_available_memory)
{
    VOID    *pointer;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
#ifdef	INCLUDE_ODYSSEY
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 100000000, 56, NU_FIFO);
#else
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 200000, 56, NU_FIFO);
#endif
                        
    /* Create IOPMaster_Task */
    NU_Allocate_Memory(&System_Memory, &pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&IOPMaster_Task, "IOPMaster", IOPMaster, 0, NU_NULL, 
					pointer, 10000, 1, 20, NU_PREEMPT, NU_START);

}



void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int		  ch = 0;
	int		  i;
	unsigned long pci_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	printf("\n\r");
	printf("Executing the System Code....\n\r");
	/* Initilaize the Hardware */
#ifdef	INCLUDE_ODYSSEY
	/*
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
	*/
#else
	init_galileo();
#endif	INCLUDE_ODYSSEY

	/* Spawn the rest of the tasks and initialize */
	Init_System(&System_Memory);
	
	/* Lower the Priority of this task */
	NU_Change_Priority( (NU_TASK *)&IOPMaster_Task, 250 );
	
	/* Now this task will be active with lowest priority and can be used to
	 * any cleanup required
	 */
	for(;;){
		NU_Sleep(500);
		Cleanup_System();
		/* rlw: can we implement erc91 functionality here? */
	}
}


void
Init_System( NU_MEMORY_POOL  *Memory_Pool)
{
	VOID *Stack_Pointer;
	STATUS sts;

    /* Create IOPMaster_Task */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
}

void
Cleanup_System()
{
}

extern int	rlen;
char *clrscn = "\033[H\033[2J\033[0m";
void
print_help()
{
	printf("%s", clrscn);
	printf("\033[01;07H*********************************************************************");
	printf("\033[02;15HConvergeNet Technologies - %s Demonstration, Slot %s", 
					bname[board_type], pcislot[get_slotno()].slotname);
	printf("\033[03;07H*********************************************************************");
	printf("\033[05;18Hi) Enable all Interrupts");
	printf("\033[06;18Hp) Probe the Devices on the Bus 0");
	printf("\n\r\n\r");
}
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	int	i=0, j, k;
	U32	padr;
	U32 *ptr;
	int bus_num = 0;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	NU_Register_LISR(3, m_lih, &o_lih);
	
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'i':
			case 'I':
				break;
			case 'p':
			case 'P':
				printf("Devices Found on the PCI Bus 0\n\r");
				probe_pci_devs(0);
				break;
			case ' ':
				print_help();
				break;
			default:
				printf("%c", ch);
				break;
			}
		}
		
		NU_Sleep(10);
	}
}


void m_lih(int v)
{
	int i = v;
	ttyA_out('1');
}
