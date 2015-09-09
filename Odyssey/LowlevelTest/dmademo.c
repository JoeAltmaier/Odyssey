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
#include  "galileo.h"
#include  "pcimap.h"
#include  "pcidev.h"
#include  "gt.h"
#include  "pcialloc.h"
#include  "que.h"
#include  "common.h"
#include  "ioptypes.h"
#include  "bootblock.h"

extern  bootblock_t bootblock;
extern void *GetSystemPool(int size);
extern void *GetPciPool(int size);

#define	IS_HBC0	((bootblock.b_type == IOPTY_HBC) && (bootblock.b_slot== 0))?1:0
#define	IS_HBC1	((bootblock.b_type == IOPTY_HBC) && (bootblock.b_slot== 1))?1:0


int	nohide = 0;

typedef	struct	_btest {
	int	 slot_no;
	int  op_type;
	int	 active;
	char key_in;
} btest_t;
btest_t		board_list[MAX_PCI_SLOTS];
int			board_list_count = 0;

/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory_G;
NU_MEMORY_POOL  Local_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;

/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void 	print_bootblock();

STATUS init_buf(int slot_no);
void create_board_list(int type);


#define	DMABUF_ALIGN_SIZE	512

/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
#ifdef	INCLUDE_ODYSSEY
void    Application_Initialize(void *first_available_memory)
#else
void    DmaTest_Initialize(void *first_available_memory)
#endif
{
    VOID    *pointer;

#ifdef	INCLUDE_ODYSSEY
    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    NU_Create_Memory_Pool(&System_Memory_G, "SYSMEM", 
                        first_available_memory, M(16), 56, NU_FIFO);
    NU_Create_Memory_Pool(&Local_Memory, "LOCAL", 
                        (void *)(0xA0000000 + M(64)), M(16), 56, NU_FIFO);
                        

    /* Create IOPMaster_Task */
    NU_Allocate_Memory(&System_Memory_G, &pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&IOPMaster_Task, "IOPMaster", IOPMaster, 0, NU_NULL, 
					pointer, 10000, 1, 20, NU_PREEMPT, NU_START);
#else

    /* System Meomry is is in the PCI Space */
    
    pointer = GetPciPool(M(16));
    if ( pointer == 0 ) {
    	printf("Error getting System Memory\n\r");
    	return;
    }
    NU_Create_Memory_Pool(&System_Memory_G, "SYSMEM", 
    					pointer, M(16), 56, NU_FIFO);
    					
    /* Local bufs are in the System Space */
    pointer = GetSystemPool(M(16));
    if ( pointer == 0 ) {
    	printf("Error getting Pci Memory\n\r");
    	return;
    }
    NU_Create_Memory_Pool(&Local_Memory, "LOCAL", 
                        pointer, M(16), 56, NU_FIFO);
	IOPMaster(0, pointer);
#endif

}



void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int		  ch = 0;
	int		  i;
	unsigned long pci_id, devven_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	printf("\n\r");
#ifdef	INCLUDE_ODYSSEY
	printf("Executing the System Code....\n\r");

	/* Initilaize the Hardware */
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
#endif

	/* Probe the Devices, on HBC0 only */
	if (IS_HBC0) {
		/* Init the bridges again */
		InitBridgeFTree();
		for(i=2; i < MAX_SLOTS; i++)
			if ( memmaps.aIdPci[i]) {
				devven_id = pciconf_readl(memmaps.aIdPci[i], 0);
				if ( devven_id == BRIDGE_21154_ID)
					memmaps.aBoardType[i] = BOARD_NAC;
				if ( devven_id == GAL120_ID)
					memmaps.aBoardType[i] = BOARD_SSD;
			}
		/* Update the Bootblock */
		memcpy(&(bootblock.b_memmap), &memmaps, sizeof(memmaps));
	}

	/* Init the Galileo for DMA */
	gt_init();
	gt_initdma(0);
	gt_initdma(1);
	
	/* Initialize the Buffers */
	init_buf(bootblock.b_slot);
	

	/* Spawn the rest of the tasks and initialize */
	Init_System(&System_Memory_G);
	
#ifdef	INCLUDE_ODYSSEY
	/* Lower the Priority of this task */
	NU_Change_Priority( (NU_TASK *)&IOPMaster_Task, 250 );
	
	/* Now this task will be active with lowest priority and can be used to
	 * any cleanup required
	 */
	for(;;){
		NU_Sleep(500);
		Cleanup_System();
	}
#endif
}


void
Init_System( NU_MEMORY_POOL  *Memory_Pool)
{
	VOID *Stack_Pointer;
	STATUS sts;

#ifdef	INCLUDE_ODYSSEY
    /* Create Test_Task0 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task1 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 4, 20, NU_PREEMPT, NU_START);
#else
    /* Create Test_Task1 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 21, 20, NU_PREEMPT, NU_START);
#endif
}

void
Cleanup_System()
{
}

extern int	rlen;
#ifdef	INCLUDE_ODYSSEY
char *clrscn = "\033[H\033[2J\033[0m";
#else
extern	char *clrscn;
#endif

void
print_menu()
{
	printf("\033[07;18Hc) Start DMA Write Test");
	printf("\033[08;18Hf) Start DMA Read  Test");
	printf("\033[09;18Hg) Stop all Tests");
	printf("\033[10;18Hk) Stats On/Off");
	if ( nohide ) {
		printf("\033[11;18Hg) Print Our Galileo Configuration Space");
		printf("\033[12;18Hp) Probe the Devices on the Bus 0");
		printf("\033[13;18Hb) Print BootBlock");
		printf("\033[14;18Hi) Init I2O Buffers");
	}
	printf("\n\r\n\r");
		
}
void
print_help()
{
	printf("%s", clrscn);
	printf("\033[01;07H*********************************************************************");
	printf("\033[02;15HConvergeNet Technologies - %s Demonstration, Slot %s", 
					bname[bootblock.b_type], slotname[bootblock.b_slot]);
	printf("\033[03;07H*********************************************************************");
	print_menu();
}
#ifndef	INCLUDE_ODYSSEY
void
DmaMenu(int ch)
{
	int i;
#else
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
	int	ch;
	int	i=0, j, k;
	U32	padr;
	U32 *ptr;
	int bus_num = 0;
	U16 val;
	U32 pci_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
#endif
			switch(ch){
			case 'b':
			case 'B':
				print_bootblock();
				break;
			case 'i':
			case 'I':
				InitI2oBuffers(&System_Memory_G);
				break;
			case 'j':
			case 'J':
				break;
			case 'G':
				printf("Our Galileo Configuration Space:\n\r");
				pciconf_print(0x80000000);
				break;
			case 'p':
			case 'P':
				printf("Devices Found on the PCI Bus 0\n\r");
				probe_pci_devs(0);
				break;
			case 'k':
				display_flag ^= 1;
				if ( display_flag)
					print_result();
				else
#ifdef	INCLUDE_ODYSSEY
					print_help();
#else
					DrawScreen();
#endif
				break;
			case 'H':
			case 'h':
				nohide ^= 1;
				if ( display_flag)
					print_result();
				else
					print_help();
				break;
			case 'c':
				stopped = 0;
				create_board_list(TYPE_WRITE);
				for(i=0; i < board_list_count; i++) {
					if ( board_list[i].op_type == TYPE_WRITE) {
						board_list[i].active = 0;
						printf("Started DMA Write from slot %s\n\r", 
							slotname[(board_list[i].slot_no)]);
						i2o_send_msg(board_list[i].slot_no, TYPE_WRITE, 
						sizeof(buf_t), get_pci_buf(board_list[i].slot_no));
					}
				}
				break;
			case 'f':
				stopped = 0;
				create_board_list(TYPE_READ);
				for(i=0; i < board_list_count; i++) {
					if ( board_list[i].op_type == TYPE_READ) {
						board_list[i].active = 0;
						printf("Started DMA Read from slot %s\n\r", 
							slotname[(board_list[i].slot_no)]);
						i2o_send_msg(board_list[i].slot_no, TYPE_READ, 
						sizeof(buf_t), get_pci_buf(board_list[i].slot_no));
					}
				}
				break;
			case 'g':
				if ( !stopped ) 
					printf("Stopping the Tests\n\r");
				stopped = 1;
				for(i=0; i < board_list_count; i++) {
					if ( board_list[i].op_type ) {
						board_list[i].active = 0;
						i2o_send_msg(board_list[i].slot_no, TYPE_STOP, 
						sizeof(buf_t), get_pci_buf(board_list[i].slot_no));
					}
				}
				break;
			case ' ':
				if ( display_flag)
					print_result();
				else
#ifdef	INCLUDE_ODYSSEY
					print_help();
#else
					DrawScreen();
#endif
				break;
			default:
				printf("%c", ch);
				break;
			}
#ifdef	INCLUDE_ODYSSEY
		}
		
		NU_Sleep(10);
	}
#endif
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
	for(i=0; i < MAX_SLOTS; i++) {
		if ( bootblock.b_memmap.aIdPci[i]) {
			lbufs[i] = dmabuf;
			dmabuf++;
		} else {
			lbufs[i] = 0;
		}
	}
	/* Allocate PCI Buffers */
	rc = NU_Allocate_Memory(&System_Memory_G, (VOID **)&pcibuf,
		((sizeof(buf_t) * MAX_PCI_SLOTS) + DMABUF_ALIGN_SIZE), NU_NO_SUSPEND);
	
	if ( rc != NU_SUCCESS) {
		printf("Error in Allocating Pci Bufs\n\r");
		return(rc);
	}
	
	/* Align the Address */
	pcibuf = (buf_t *)(ALIGN((U32)pcibuf, DMABUF_ALIGN_SIZE));

	for(i=0; i < MAX_SLOTS; i++) {
		if ( bootblock.b_memmap.aIdPci[i]) {
			pbufs[i] = pcibuf;
			pcibuf++;
		} else {
			pbufs[i] = 0;
		}
	}
	
	mem_write((U32)(lbufs[0]), 0, 0);
	mem_write((U32)(pbufs[0]), 0, 0);
	lbufs[0]->flag = slot_no;
	pbufs[0]->flag = slot_no;
	/* Initialize the Buffers */
	for(i=1; i < MAX_SLOTS; i++) {
		if ( lbufs[i] ) {
			DMASTARTED(0);
			gt_dmachain(0, VTOP((U32)(lbufs[0])), VTOP((U32)(lbufs[i])), 
						sizeof(buf_t), 0, 0);
			while(!(DMADONE(0))) ;
			lbufs[i]->flag = slot_no;
		}
		if ( pbufs[i] ) {
			DMASTARTED(0);
			gt_dmachain(0, VTOP((U32)(pbufs[0])), VTOP((U32)(pbufs[i])), 
						sizeof(buf_t), 0, 0);
			while(!(DMADONE(0))) ;
			pbufs[i]->flag = slot_no;
		}
	}
	printf("Done\n\r");
	return(OK);
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
	char	ch;
	int i, j;

	for(i=0; i < board_list_count; i++) 
		board_list[i].op_type = 0;
	board_list_count = 0;
	for(i=0; i < MAX_SLOTS; i++) {
		if ( bootblock.b_memmap.aBoardType[i]) { 
			if ( i == bootblock.b_slot )
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
				slotname[(board_list[i].slot_no)]);

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

void
print_bootblock()
{
	int i;
	printf("Slot        %d\n\r", bootblock.b_slot);
	printf("Board type  %d\n\r", bootblock.b_type);
	printf("MBoard type %d\n\r", bootblock.b_memmap.aBoardType[bootblock.b_slot]);
	printf("MPCI id     %08X\n\r", bootblock.b_memmap.aIdPci[bootblock.b_slot]);
	printf("MPCI Mem    %08X\n\r", bootblock.b_memmap.aPaPci[bootblock.b_slot]);
	printf("MCabinet    %d\n\r", bootblock.b_memmap.iCabinet);
	printf("MSlot       %d\n\r", bootblock.b_memmap.iSlot);
	printf("MPCI Mem    %08X\n\r", bootblock.b_memmap.pciSlave);
	printf("MPCI Slave  %08X\n\r", bootblock.b_memmap.paSlave);
	printf("MPCI Size   %08X\n\r", bootblock.b_memmap.cbSlave);
	
	printf("Info of the other boards:\n\r");
	for(i=0; i < MAX_SLOTS; i++) {
		printf("Board Type %d, PCI Id %08X, PCI Mem %08X\n\r", 
				bootblock.b_memmap.aBoardType[i], 
				bootblock.b_memmap.aIdPci[i], 
				bootblock.b_memmap.aPaPci[i]);
	}
	return;
}

extern	void        (*old_lisr_gt)(int);
extern	void        gt_lih(int );
void
DmaRegisterLISR()
{
	NU_Register_LISR(0, gt_lih, &old_lisr_gt);
}
