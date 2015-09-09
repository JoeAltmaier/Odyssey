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
#include  "common.h"

#define	PCI_G_NUM	0
#define	PCI_E_NUM	23
#define	PCI_F_NUM	1
#define	PCI_H_NUM	2
#define	PCI_A_NUM	3
#define	PCI_B_NUM	8
#define	PCI_C_NUM	13
#define	PCI_D_NUM	18

#define	PCI_A_SUB	(PCI_B_NUM - 1)
#define	PCI_B_SUB	(PCI_C_NUM - 1)
#define	PCI_C_SUB	(PCI_D_NUM - 1)
#define	PCI_D_SUB	(PCI_E_NUM - 1)
#define	PCI_E_SUB	(PCI_E_NUM )
#define	PCI_F_SUB	(PCI_E_NUM - 1)
#define	PCI_H_SUB	(PCI_C_NUM - 1)

U32	a_id, b_id, c_id, d_id, e_id, f_id, h_id;

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

/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;
NU_MEMORY_POOL  Local_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;

extern U32 board_type;
/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);

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
                        first_available_memory, M(32), 56, NU_FIFO);
    NU_Create_Memory_Pool(&Local_Memory, "LOCAL", 
                        (void *)(0xA0000000 + M(64)), M(32), 56, NU_FIFO);
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
#if 0
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
	gt_init();
	gt_initdma(0);
	gt_initdma(1);
	board_type = memmaps.aBoardType[getSlotNumber()];
	init_buf(get_slotno());
#else
	init_galileo();
	/* enable the Switches */
	*((U8 *)(0xBC0F8000)) = 0;

	a_id = b_id = c_id = d_id = e_id = f_id = h_id = 0;
	/* Init the E bridge */
	printf("Initializing the Bridge E....");
	pci_id = get_pci_id(0, BRIDGE_21154_ID, 0);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_G_NUM, PCI_E_NUM, PCI_E_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		e_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the F bridge */
	printf("Initializing the Bridge F....");
	pci_id = get_pci_id(0, BRIDGE_21154_ID, 1);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_G_NUM, PCI_F_NUM, PCI_F_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		f_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the H bridge */
	printf("Initializing the Bridge H....");
	pci_id = get_pci_id(PCI_F_NUM, BRIDGE_21154_ID, 0);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_F_NUM, PCI_H_NUM, PCI_H_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		h_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the C bridge */
	printf("Initializing the Bridge C....");
	pci_id = get_pci_id(PCI_F_NUM, BRIDGE_21154_ID, 1);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_F_NUM, PCI_C_NUM, PCI_C_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		c_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the D bridge */
	printf("Initializing the Bridge D....");
	pci_id = get_pci_id(PCI_F_NUM, BRIDGE_21154_ID, 2);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_F_NUM, PCI_D_NUM, PCI_D_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		d_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the A bridge */
	printf("Initializing the Bridge A....");
	pci_id = get_pci_id(PCI_H_NUM, BRIDGE_21154_ID, 0);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_H_NUM, PCI_A_NUM, PCI_A_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		a_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
	/* Init the B bridge */
	printf("Initializing the Bridge B....");
	pci_id = get_pci_id(PCI_H_NUM, BRIDGE_21154_ID, 1);
	if ( pci_id ) {
		bridge_init(pci_id, PCI_H_NUM, PCI_B_NUM, PCI_B_SUB, 0xFFFFFFFF, 0);
		printf("OK\n\r");
		b_id = pci_id;
	} else {
		printf("Fail\n\r");
		goto done;
	}
#endif
	
#else
	init_galileo();
#endif	INCLUDE_ODYSSEY


	
done:

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
    /* Create IOPMaster_Task */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);
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
	printf("\033[02;15HConvergeNet Technologies - E2 Demonstration");
	printf("\033[03;07H*********************************************************************");
	printf("\033[05;18Hi) Print Our Galileo Configuration Space");
	printf("\033[06;18Hg) Probe the Devices on the Bus G");
	printf("\033[07;18He) Probe the Devices on the Bus E");
	printf("\033[08;18Hf) Probe the Devices on the Bus F");
	printf("\033[09;18Hh) Probe the Devices on the Bus H");
	printf("\033[10;18Hw) Probe Test forever");
	printf("\033[11;18Ha) Probe the Devices on the Bus A");
	printf("\033[12;18Hb) Probe the Devices on the Bus B");
	printf("\033[13;18Hc) Probe the Devices on the Bus C");
	printf("\033[14;18Hd) Probe the Devices on the Bus D");
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
	
	configured = 1;
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'i':
			case 'I':
				printf("Galileo PCI Configuration Registers\n\r");
				pciconf_print(0x80000000);
				break;
			case 'g':
			case 'G':
				printf("Devices Found on the PCI Bus G\n\r");
				probe_pci_devs(0);
				break;
			case 'e':
			case 'E':
				printf("Devices Found on the PCI Bus E\n\r");
				probe_pci_devs(PCI_E_NUM);
				break;
			case 'f':
			case 'F':
				printf("Devices Found on the PCI Bus F\n\r");
				probe_pci_devs(PCI_F_NUM);
				break;
			case 'h':
			case 'H':
				printf("Devices Found on the PCI Bus H\n\r");
				probe_pci_devs(PCI_H_NUM);
				break;
			case 'a':
			case 'A':
				printf("Devices Found on the PCI Bus A\n\r");
				probe_pci_devs(PCI_A_NUM);
				break;
			case 'b':
			case 'B':
				printf("Devices Found on the PCI Bus B\n\r");
				probe_pci_devs(PCI_B_NUM);
				break;
			case 'c':
			case 'C':
				printf("Devices Found on the PCI Bus C\n\r");
				probe_pci_devs(PCI_C_NUM);
				break;
			case 'd':
			case 'D':
				printf("Devices Found on the PCI Bus D\n\r");
				probe_pci_devs(PCI_D_NUM);
				break;
			case 'w':
			case 'W':
				while (1) { 
					if ( ttyA_poll() == 0) {
						ch = ttyA_in();
						break;
					}
					printf("Devices Found on the PCI Bus G\n\r");
					probe_pci_devs(0);
					printf("Devices Found on the PCI Bus E\n\r");
					probe_pci_devs(PCI_E_NUM);
					printf("Devices Found on the PCI Bus F\n\r");
					probe_pci_devs(PCI_F_NUM);
					printf("Devices Found on the PCI Bus H\n\r");
					probe_pci_devs(PCI_H_NUM);
				}
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

void   
test_1(UNSIGNED argc, VOID *argv)
{
    STATUS    status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	process();
}



STATUS
init_buf(int slot_no)
{
	STATUS rc;
	buf_t	*dmabuf, *pcibuf;
	int		i;
	U32		pci_start_phys;
	
	
	if ( lbufs[0] )
		return(NU_SUCCESS);

	printf("Initalizing Buffers...");
	/* Allocate the Local Buffers */
	rc = NU_Allocate_Memory(&Local_Memory, (VOID **)&dmabuf,
		((sizeof(buf_t) * MAX_PCI_SLOTS) + DMABUF_ALIGN_SIZE), NU_NO_SUSPEND);
	
	if ( rc != NU_SUCCESS) {
		printf("Error in Allocating Local Bufs\n\r");
		return(rc);
	}
	
	/* Align the Address */
	dmabuf = (buf_t *)(ALIGN((U32)dmabuf, DMABUF_ALIGN_SIZE));
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		lbufs[i] = dmabuf;
		dmabuf++;
	}
	/* Allocate PCI Buffers */
	rc = NU_Allocate_Memory(&System_Memory, (VOID **)&pcibuf,
		((sizeof(buf_t) * MAX_PCI_SLOTS) + DMABUF_ALIGN_SIZE), NU_NO_SUSPEND);
	
	if ( rc != NU_SUCCESS) {
		printf("Error in Allocating Pci Bufs\n\r");
		return(rc);
	}
	
	/* Align the Address */
	pcibuf = (buf_t *)(ALIGN((U32)pcibuf, DMABUF_ALIGN_SIZE));

	for(i=0; i < MAX_PCI_SLOTS; i++) {
		pbufs[i] = pcibuf;
		pcibuf++;
	}
	
	mem_write((U32)(lbufs[0]), 0, 0);
	mem_write((U32)(pbufs[0]), 0, 0);
	/* Initialize the Buffers */
	for(i=1; i < MAX_PCI_SLOTS; i++) {
		DMASTARTED(0);
		gt_dmachain(0, VTOP((U32)(lbufs[0])), VTOP((U32)(lbufs[i])), 
						sizeof(buf_t), 0, 0);
		while(!(DMADONE(0))) ;
		DMASTARTED(0);
		gt_dmachain(0, VTOP((U32)(pbufs[0])), VTOP((U32)(pbufs[i])), 
						sizeof(buf_t), 0, 0);
		while(!(DMADONE(0))) ;
		lbufs[i]->flag = slot_no;
		pbufs[i]->flag = slot_no;
	}
	printf("Done\n\r");
	return(NU_SUCCESS);
}

U32
get_local_buf(int slot_no)
{
	U32	p;
	
	p = (U32)(lbufs[slot_no]);
	return(VTOP(p));
}

U32
get_pci_buf(int slot_no)
{
	U32	p;
	
	p = (U32)(pbufs[slot_no]);
	p = VTOP(p);
	p = p - memmaps.aPa[0];
	return(memmaps.pciSlave + p);
}



void 
create_board_list(int type)
{
	pcislot_t   *slotp;
	char	ch;
	int i, j;

	for(i=0; i < board_list_count; i++) 
		board_list[i].op_type = 0;
	board_list_count = 0;
	for(i=0; i < MAX_PCI_SLOTS; i++) {
		slotp = &pcislot[i];
		if ( slotp->pci_id ) {
			if ( i == get_slotno())
				continue;
			board_list[board_list_count].slot_no = i;
			if ( board_list_count > 8)
				board_list[board_list_count].key_in = 'A' +(board_list_count-9);
			else
				board_list[board_list_count].key_in = '1' + board_list_count;
			board_list_count++;
		}
	}
	
	j =1;
	for(i=0; i < board_list_count; i++) {
		printf("%c - %s", board_list[i].key_in,
				pcislot[board_list[i].slot_no].slotname);
		if ( (j % 4) == 0)
			printf("\n\r");
		else
			printf(",	");	
		j++;
	}
	printf("\n\r");
	while(1) {
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			if ( (ch == '\r') || (ch == '\n')) {
				printf("\n\r");
				return;
			}
			printf("%c", ch);
			for(i = 0; i < board_list_count; i++) {
				if ( board_list[i].key_in == ch)
					board_list[i].op_type = type;
			}
		}
		NU_Sleep(10);
	}
}

void
update_active(int slot_no)
{
	int i;
	for(i = 0; i < board_list_count; i++) {
		if ( board_list[i].slot_no == slot_no)
			board_list[i].active++;
	}
}

