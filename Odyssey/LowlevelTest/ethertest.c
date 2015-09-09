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
#define	PLUS	1

#include  "nucleus.h"

#include "externs.h"
#include "protocol.h"
#include "tcp_errs.h"
#include "data.h"
#include "dec21143.h"
#include "net.h"
#include "dev.h"

#include  "types.h"
#include  "tty.h"
#include  "sysflash.h"
#include  "hw.h"
#include  "pcimap.h"
#include  "pcidev.h"
#include  "gt.h"
#include  "pcialloc.h"
#include  "system.h"
#include  "que.h"


void		ether_send();
/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;
NU_TASK         Test_Task3;


//#define	CMB_HACK
/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void 	Cleanup_System();
void 	Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);
void    test_3(UNSIGNED argc, VOID *argv);


int	send_flag = 0;
int	print_flag = 0;
int	print_data_flag = 0;
int	tx_count[2] = { 0, 0};
int rx_count[2] = { 0, 0};
unsigned char	multi_add_byte = 1;
unsigned char	multi_del_byte = 1;
uint8	maddr[6] = { 0x01, 0x01, 0x5e, 0x02, 0x03, 0x00 };

extern	U32 r7k_get_ic();
extern	void r7k_set_ic(U32 val);
extern	U32 r7k_get_18();
extern	U32 r7k_get_19();

STATUS Init_Boot_Hbc();

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
                        first_available_memory, 2000000, 56, NU_FIFO);
#endif
                        
    /* Create IOPMaster_Task */
    NU_Allocate_Memory(&System_Memory, &pointer, 10000, NU_NO_SUSPEND);
    NU_Create_Task(&IOPMaster_Task, "IOPMaster", IOPMaster, 0, NU_NULL, 
					pointer, 10000, 1, 20, NU_PREEMPT, NU_START);

}
void 		(*o_lih)(int);
void		m_lih(int);


void m_lih(int v)
{
	printf("Unhandled Interrupt %d\n\r", v);
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
	/* Initialize LISRs for Unhandled Interrupts */
#ifdef CMB_HACK
	Init_Boot_Hbc();
#else
	if ((status = Init_Hardware()) != NU_SUCCESS)
			printf("Error in init %d\r\n", status);
//	init_slave_bridges();
#endif
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

	if ( NU_Create_Event_Group(&Buffers_Available, "BUFAVA") != NU_SUCCESS) {
		printf("Error Creating the Event Group\n\r");
		return;
	}

    /* Create Test_Task 0 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task0, "Test0", test_0, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
    /* Create Test_Task 1 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task1, "Test1", test_1, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);

    /* Create Test_Task 2 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task2, "Test2", test_2, 0, NU_NULL, 
					Stack_Pointer, 10000, 3, 20, NU_PREEMPT, NU_START);
    /* Create Test_Task 3 */
    NU_Allocate_Memory(Memory_Pool, &Stack_Pointer, 50000, NU_NO_SUSPEND);
    NU_Create_Task(&Test_Task3, "Test3", test_3, 0, NU_NULL, 
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
	printf("\033[02;15HConvergeNet Technologies - Ethernet Demonstration"); 
	printf("\033[03;07H*********************************************************************");
	printf("\033[05;18Hp) Probe the Devices on the Bus 0");
#ifdef	INCLUDE_ODYSSEY
	printf("\033[06;18Hl) Probe the Devices on the Local Bus");
#endif
	printf("\033[07;18Hi) Initialize the Ethernet");
	printf("\033[08;18Ht) Start/Stop Transmit Packets");

	printf("\033[10;18He) Error Counters");
	printf("\033[11;18Hc) Toggle print_flag");
	printf("\033[12;18Hd) Toggle print__data_flag");
	printf("\033[13;18Hr) Clear the counts");
	printf("\033[14;18H+) Add a Multicast Address");
	printf("\033[15;18H-) Delete a Multicast Address");
	printf("\n\r\n\r");
}



/* Ethernet Related Stuffs */

extern	NU_EVENT_GROUP    Buffers_Available;




DEV_DEVICE	dev_devices[2];
char	dev_name[] = "de0";
char	dev1_name[] = "de1";

DV_DEVICE_ENTRY	*device = (DV_DEVICE_ENTRY *)NU_NULL;
extern	void DEC21143_Printcounters (DV_DEVICE_ENTRY *device);
DV_REQ *d_req; 	/* I dont use this */

void
init_devptr()
{
	dev_devices[0].dv_name = dev_name;
	dev_devices[0].dv_hw.ether.dv_irq = 0;
	dev_devices[0].dv_init = DEC21143_Init;

	dev_devices[1].dv_name = dev1_name;
	dev_devices[1].dv_hw.ether.dv_irq = 0;
	dev_devices[1].dv_init = DEC21143_Init;
		
}
extern	INT TCD_Interrupt_Level;
extern	INT TCD_R7KInterrupt_Level;
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
	
	print_help();
	for(;;){
		if ( ttyA_poll() == 0) {
			ch = ttyA_in();
			switch(ch){
			case 'i':
			case 'I':
				/* Initialize Memory */
				MEM_Init();
				/* Initialize the dev structure */
				init_devptr();
				/* Initialize the device */
				DEV_Init_Devices(dev_devices, 2);
				break;
			case '0':
				device = DEV_Get_Dev_By_Name("de0");
				break;
			case '1':
				device = DEV_Get_Dev_By_Name("de1");
				break;
			case 'p':
			case 'P':
				printf("Devices Found on the PCI Bus 0\n\r");
				probe_pci_devs(0);
				break;
#ifdef	INCLUDE_ODYSSEY
			case 'l':
			case 'L':
				printf("Devices Found on the Local Bus \n\r");
				probe_pci_devs(PCI_E_BUSNUM);
				break;
			case 'x':
			case 'X':
				if ( device != NU_NULL) {
					DEC21143_XDATA          *dec_data;
					DEC21143_DESCRIPTOR		*desc;
					dec_data = (DEC21143_XDATA *) device->user_defined_1;
					
					desc = dec_data->DEC21143_First_TX_Descriptor;
					printf("\n\r");
					while(desc) {
						desc = (DEC21143_DESCRIPTOR *)((U32)(desc) | 0xA0000000);
						printf("Control	:%08X,   ", desc->control_count);
						printf("Status	:%08X,   ", desc->status);
						printf("Buffer	:%08X\n\r", desc->buffer);
					
						desc = desc->next_descriptor;
					}
				} else {
					printf("Device not Initialized\n\r");
				}
				printf("TCD_Interrupt_Level	:%08X\n\r",TCD_Interrupt_Level);
				printf("TCD_R7KInterrupt_Level	:%08X\n\r",TCD_R7KInterrupt_Level);
				printf("IC Register		%x\n\r", r7k_get_ic());
				printf("18 Register		%x\n\r", r7k_get_18());
				printf("19 Register		%x\n\r", r7k_get_19());
				break;
#endif
			case 'T':
			case 't':
				if ( device != NU_NULL) {
					send_flag ^= 1;
				} else {
					printf("Device not Initialized\n\r");
				}
				break;
			case 'e':
			case 'E':
				if ( device != NU_NULL) {
					DEC21143_Printcounters (device);
				} else {
					printf("Device not Initialized\n\r");
				}
				break;
			case 'c':
			case 'C':
				print_flag ^= 1;
				break;
			case 'd':
			case 'D':
				print_data_flag ^= 1;
				break;
			case 'r':
			case 'R':
				tx_count[0] = 0;
				tx_count[1] = 0;
				rx_count[0] = 0;
				rx_count[1] = 0;
				break;
			case '+':
				if ( device != NU_NULL) {
					maddr[5] = multi_add_byte;
					Ether_Add_Multi(device, maddr);
					multi_add_byte++;
					DEC21143_Ioctl(device, DEV_ADDMULTI, d_req);
				} else {
					printf("Device not Initialized\n\r");
				}
				break;
			case '-':
				if ( device != NU_NULL) {
					maddr[5] = multi_del_byte;
					if (Ether_Del_Multi(device, maddr) == NU_SUCCESS)
						multi_del_byte++;
					DEC21143_Ioctl(device, DEV_DELMULTI, d_req);
				} else {
					printf("Device not Initialized\n\r");
				}
				break;
			case 'y':
				{	
					int *vd = (int *)(0xA0000001);
					*vd = 1;
					break;
				}
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
    STATUS    		status;
	UNSIGNED  		bufs_ava;
	DV_DEVICE_ENTRY	*device;
	U8				*ptr;
	NET_BUFFER		*buf_ptr, *tptr;
	int				total, i;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		NU_Retrieve_Events(&Buffers_Available, 2, NU_OR_CONSUME, &bufs_ava,
							NU_SUSPEND);
		if ( print_flag)
			printf("\n\rRecived a Packet\n\r");
		DISABLE_INT;
		while(MEM_Buffer_List.head) {
			device = MEM_Buffer_List.head->mem_buf_device;
			buf_ptr = MEM_Buffer_List.head;
			tptr = buf_ptr;
			total = 0;
			while ( tptr ) {
				ptr = (U8 *)(tptr->data_ptr);
				total += tptr->data_len;
				if(print_data_flag) {
					printf("\n\r%s :", device->dev_net_if_name);
					for(i=0; i < tptr->data_len; i++)
						printf("%02X ", ptr[i]);
				}
					
				tptr = tptr->next_buffer;
			}
			if (print_flag) {
				printf("\n\r%s : ", device->dev_net_if_name);
				printf("Len %d\n\r", total);
			}
			rx_count[device->dev_index]++;
			

			/* Deallocate the space taken up by this useless packet. */
			/* Drop the packet by placing it back on the buffer_freelist. */
			MEM_Buffer_Chain_Free (&MEM_Buffer_List, &MEM_Buffer_Freelist);
		}
		ENABLE_INT;
	}
	
}



void   
test_2(UNSIGNED argc, VOID *argv)
{
    STATUS    		status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		if( send_flag )
			ether_send();
	}
	
}

void   
test_3(UNSIGNED argc, VOID *argv)
{
    STATUS    		status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
	while(1) {
		NU_Sleep(100);
		if ( device != NU_NULL) 
			printf("\r de0: Tx count %d	Rx count %d,	de1: Tx count %d	Rx count %d", tx_count[0], rx_count[0], tx_count[1], rx_count[1]);
	}
	
}

void
ether_send()
{
	NET_BUFFER	*buf_ptr, *tptr;
	int			nbytes;
	
	
	nbytes = 1500;
	

	if (!(device-> dev_flags & DV_UP)) {
		if ( print_flag)
			printf("Device is down\n\r");
		return;
	}

	DISABLE_INT;
	buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, nbytes);
	if ( buf_ptr == NU_NULL ){
		ENABLE_INT;
		if ( print_flag)
			printf("Error Allocating the buffer\n\r");
		return;
	}
	ENABLE_INT;
	
	/* Set the deallocation list pointer. */
	buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

	buf_ptr->mem_total_data_len = nbytes;
	buf_ptr->data_ptr = buf_ptr->mem_parent_packet;
	if ( nbytes > NET_PARENT_BUFFER_SIZE )
		buf_ptr->data_len = NET_PARENT_BUFFER_SIZE;
	else
		buf_ptr->data_len = nbytes;
	nbytes -= buf_ptr->data_len;
	tptr = buf_ptr->next_buffer;
	while ( nbytes ) {
		if ( nbytes > NET_MAX_BUFFER_SIZE )
			tptr->data_len = NET_MAX_BUFFER_SIZE;
		else
			tptr->data_len = nbytes;
		
		nbytes -= tptr->data_len;
		tptr->data_ptr = tptr->mem_parent_packet;
		tptr = tptr->next_buffer;
	}
	

	
	buf_ptr->data_ptr[0] = 0xFF;
	buf_ptr->data_ptr[1] = 0xFF;
	buf_ptr->data_ptr[2] = 0xFF;
	buf_ptr->data_ptr[3] = 0xFF;
	buf_ptr->data_ptr[4] = 0xFF;
	buf_ptr->data_ptr[5] = 0xFF;

	buf_ptr->data_ptr[6] = device->dev_mac_addr[0];
	buf_ptr->data_ptr[7] = device->dev_mac_addr[1];
	buf_ptr->data_ptr[8] = device->dev_mac_addr[2];
	buf_ptr->data_ptr[9] = device->dev_mac_addr[3];
	buf_ptr->data_ptr[10] = device->dev_mac_addr[4];
	buf_ptr->data_ptr[11] = device->dev_mac_addr[5];
		
	/* Place the buffer on the device's transmit queue so that this
       buffer will be freed. */
	DISABLE_INT;
    MEM_Buffer_Enqueue(&device->dev_transq, buf_ptr);
	while (DEC21143_TX_Packet(device, buf_ptr) != NU_SUCCESS) {
		ENABLE_INT;
		if ( print_flag)
			printf("Send Failed\n\r");
		DISABLE_INT;
	}
	ENABLE_INT;
	tx_count[device->dev_index]++;
	printf("\r.");
	if ( print_flag)
		printf("Send Successful\n\r");
}


STATUS Ether_Add_Multi(DV_DEVICE_ENTRY *dev, uint8 *multi_addr)
{
    INT         irq_level;
    NET_MULTI   *em;
	int			i;

    irq_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* Verify that the ethernet multicast address is valid. */
    if (((multi_addr[0] & 0xFF) != 1) || ((multi_addr[2] & 0xFF) != 0x5e))
    {
        NU_Control_Interrupts(irq_level);
		printf("Invalid Address\n\r");
        return (-1);
    }

    /* Has this address already been added to the list. */
    for ( em = dev->dev_ethermulti;
          em != NU_NULL && (bcmp(em->nm_addr, multi_addr, 6) != 0);
          em = em->nm_next) ;

    if(em != NU_NULL)
    {
        /* Found a match. Increment the reference count. */
        em->nm_refcount++;
        NU_Control_Interrupts(irq_level);
		printf("Entry Exists\n\r");
        return (NU_SUCCESS);
    }

    /* This is a new address. Allocate some memory for it. */
    if (NU_Allocate_Memory(&System_Memory, (VOID **)&em,
                         sizeof (*em), (UNSIGNED)NU_NO_SUSPEND) != NU_SUCCESS)
    {
		printf("Mem Alloc Error\n\r");
        NU_Control_Interrupts(irq_level);
        return(NU_MEM_ALLOC);
    }

    /* Initialize the new entry. */
    bcopy(multi_addr, em->nm_addr, 6);
    em->nm_device = dev;
    em->nm_refcount = 1;

    /* Link it into the list. */
    em->nm_next = dev->dev_ethermulti;
    dev->dev_ethermulti = em;

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(irq_level);

	printf("\n\rEntry Added----");
	for(i=0; i < 6; i++)
		printf("%02X:", multi_addr[i]);
	printf("\n\r");
    return(NU_SUCCESS);

} /* NET_Add_Multi */

STATUS Ether_Del_Multi(DV_DEVICE_ENTRY *dev, uint8 *multi_addr)
{
    INT         irq_level;
    NET_MULTI   *em;
    NET_MULTI   **ptr;
	int			i;

    irq_level = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);


    /* Verify that the ethernet multicast address is valid. */
    if (((multi_addr[0] & 0xFF) != 1) || ((multi_addr[2] & 0xFF) != 0x5e))
    {
        NU_Control_Interrupts(irq_level);
		printf("Invalid Address\n\r");
        return (-1);
    }

    /* Find this address in the list. */
    for ( em = dev->dev_ethermulti;
          em != NU_NULL && (bcmp(em->nm_addr, multi_addr, 6) != 0);
          em = em->nm_next) ;

    if(em == NU_NULL)
    {
        /* Found a match. Increment the reference count. */
        NU_Control_Interrupts(irq_level);
		printf("Entry Not Found\n\r");
        return (NU_INVAL);
    }

    /* If this is not the last refernce then return after decremanting the 
       reference count. */
    if (--em->nm_refcount != 0)
    {
        NU_Control_Interrupts(irq_level);
		printf("Entry found ...But Ref not 0\n\r");
        return(NU_SUCCESS);
    }

    /* If we made this far then there are no more references to this entry.
       So unlink and deallocte it. */
    for ( ptr = &em->nm_device->dev_ethermulti;
          *ptr != em;
          ptr = &(*ptr)->nm_next)
        continue;

    *ptr = (*ptr)->nm_next;

    /*  Restore the previous interrupt lockout level.  */
    NU_Control_Interrupts(irq_level);

    /* Deallocate the structure. */
    NU_Deallocate_Memory(em);

	printf("\n\rEntry Deleted----");
	for(i=0; i < 6; i++)
		printf("%02X:", multi_addr[i]);
	printf("\n\r");

    return(NU_SUCCESS);

} /* NET_Add_Multi */

STATUS 
NET_Add_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req)
{
	return(NU_RESET);		
}
STATUS 
NET_Del_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req)
{
	return(NU_RESET);		
}

#ifdef  INCLUDE_ODYSSEY
STATUS
Init_Boot_Hbc()
{
	STATUS	rc = OK;
	U16	int_mask = 0;
	
#ifdef	DEBUG
	printf("Initializing the Hardware\n\r");
#endif
	mips_init_cache();
	/* Initalize the TLB */
	int_mask = NU_Control_Interrupts(0x0000);
	tlb_init();
	NU_Control_Interrupts(int_mask);

	/* Hack to Overcome The CMB Protocol
	 * Since we dont have one yet
	 */
	pciconf_writew(0x80000000, PCI_CONF_COMM,
		PCI_COMM_IO | PCI_COMM_MEM | PCI_COMM_MASTER | PCI_COMM_B2B);

	/* Turn on the Quick-Switches */
	*((U8 *)(0xBC0F8000)) = 0;
	
	/* Probe Galileo for PCI_0 */
	if (pciconf_readl(0x80000000, PCI_CONF_VENID) != GAL120_ID ) {
					/* Could not find Galileo PCI_0 */
		return(GALILEO_0_ACCESS_ERROR);
	}
#ifdef	DEBUG
	printf("Galileo PCI_0 .........OK\n\r");
#endif

	/* Initialize the Galileo */
	init_galileo();

	/* Intialize the On-board Bridges */
	rc = init_onboard_bridges();
	if(rc != OK)
		return(rc);
		
	/* create the Memory Map Data Structure */
	create_memmap();
	
	/* Initialize the Galiloe BARS */
	init_galileo_bars();
	
	/*Initialize the Local PCI Bus Devices*/
	init_local_devs();
		
	return(OK);
}

STATUS
init_onboard_bridges()
{
	U32		pci_id;
	U32		mbase, mlmt;
	
	printf("\n\r");
	/* Initialize the Bridge E */
	printf("Initializing Bridge E ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_E_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(64);
	mlmt  = mbase - 1 + M(64);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_E_BUSNUM, PCI_E_SUBNUM, mbase, mlmt); 
	printf("OK\n\r");

	/* Initialize the Bridge F */
	printf("Initializing Bridge F ....");
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_F_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(1024);
	bridge_init(pci_id, PCI_G_BUSNUM, PCI_F_BUSNUM, PCI_F_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge H */
	printf("Initializing Bridge H ....");
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_H_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(512);
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_H_BUSNUM, PCI_H_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge C */
	printf("Initializing Bridge C ....");
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_C_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(768);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_C_BUSNUM, PCI_C_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge D */
	printf("Initializing Bridge D ....");
	pci_id = get_pci_id(PCI_F_BUSNUM, BRIDGE_21154_ID, 2);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_D_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(1024);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_F_BUSNUM, PCI_D_BUSNUM, PCI_D_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge A */
	printf("Initializing Bridge A ....");
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 0);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_A_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(256);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_A_BUSNUM, PCI_A_SUBNUM, mbase, mlmt);
	printf("OK\n\r");

	/* Initialize the Bridge B */
	printf("Initializing Bridge B ....");
	pci_id = get_pci_id(PCI_H_BUSNUM, BRIDGE_21154_ID, 1);
	if ( !pci_id) {
		printf("Fail\n\r");
		return(BRIDGE_B_ACCESS_ERROR);
	}
	mbase = PCI_WINDOW_START+M(512);
	mlmt  = mbase - 1 + M(256);
	bridge_init(pci_id, PCI_H_BUSNUM, PCI_B_BUSNUM, PCI_B_SUBNUM, mbase, mlmt);
	printf("OK\n\r");
	
	/* Make Bridge E non-pre-fetchable */
	pci_id = get_pci_id(PCI_G_BUSNUM, BRIDGE_21154_ID, 0);
	
	/* Read the Bridge E Mem abse and Mem Limit */
	mbase = pciconf_readw(pci_id, PCI_BCONF_PMEMBASE) & 0xFFF0;
	mlmt  = pciconf_readw(pci_id, PCI_BCONF_PMEMLIMIT) | 0xF;
	
	/* Now Disable the Pref Memory */
	pciconf_writew(pci_id, PCI_BCONF_PMEMBASE, (0xFFFFFFFF >> PCI_BMEM_SHIFT));
	pciconf_writew(pci_id, PCI_BCONF_PMEMLIMIT, (0 >> PCI_BMEM_SHIFT));
	
	/* Now Enable the Non-Pref Memory Window */
	pciconf_writew(pci_id, PCI_BCONF_MEMBASE, mbase );
	pciconf_writew(pci_id, PCI_BCONF_MEMLIMIT, mlmt );
	
	/* Configure the local bridge for not support fast back-to-back
	 * on the secondary Bus */
	pciconf_writew(pci_id, PCI_BCONF_CNTL, 0);
		
	return(OK);
}
#endif
