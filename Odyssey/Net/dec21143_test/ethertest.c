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
#ifndef PLUS
#define PLUS    1
#endif

//#define INCLUDE_ODYSSEY

extern unsigned long gSize_available_memory = 0x400000;
extern unsigned long gSize_small_heap = 0x200000;

#include  "nucleus.h"
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

#include "externs.h"
#include "protocol.h"
#include "tcp_errs.h"
#include "data.h"
#include "dec21143.h"
#include "net.h"
#include "dev.h"

#include "odyssey_trace.h"

#define printf Tracef

void            ether_send();
/* Define Application data structures.  */
NU_TASK         IOPMaster_Task;
NU_MEMORY_POOL  System_Memory;


NU_TASK         Test_Task0;
NU_TASK         Test_Task1;
NU_TASK         Test_Task2;
NU_TASK         Test_Task3;


/* Define prototypes for function references.  */
void    IOPMaster(UNSIGNED argc, VOID *argv);

void    Cleanup_System();
void    Init_System( NU_MEMORY_POOL  *Memory_Pool);
void    test_0(UNSIGNED argc, VOID *argv);
void    test_1(UNSIGNED argc, VOID *argv);
void    test_2(UNSIGNED argc, VOID *argv);
void    test_3(UNSIGNED argc, VOID *argv);
STATUS	Ether_Del_Multi(DV_DEVICE_ENTRY *dev, uint8 *multi_addr);
STATUS Ether_Add_Multi(DV_DEVICE_ENTRY *dev, uint8 *multi_addr);

int     send_flag = 0;
int     print_flag = 0;
int     print_data_flag = 0;
int     tx_count = 0;
int rx_count = 0;
unsigned char   multi_add_byte = 1;
unsigned char   multi_del_byte = 1;
uint8   maddr[6] = { 0x01, 0x01, 0x5e, 0x02, 0x03, 0x00 };

extern STATUS bcopy(void *, void *, U32);
extern STATUS bcmp(void *, void *, U32);

/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
void    Application_Initialize(void *first_available_memory)
{
    VOID    *pointer;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
#ifdef  INCLUDE_ODYSSEY
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
void            (*o_lih)(int);
void            m_lih(int);


void m_lih(int v)
{
        printf("Unhandled Interrupt %d\n\r", v);
}

void   IOPMaster(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
        int               ch = 0;
        int               i;
        unsigned long pci_id;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
        
        printf("\n\r");
        printf("Executing the System Code....\n\r");
        /* Initilaize the Hardware */
#ifdef  INCLUDE_ODYSSEY
        /* Initialize LISRs for Unhandled Interrupts */
//        for(i=0; i < 8; i++) {
//                NU_Register_LISR(i, m_lih, &o_lih);
//        }
        if ((status = Init_Hardware()) != NU_SUCCESS)
                        printf("Error in init %d\r\n", status);
        NU_Control_Interrupts(0xC400);
#else
        init_galileo();
//        NU_Control_Interrupts(0x400);
#endif  INCLUDE_ODYSSEY

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

extern int      rlen;
char *clrscn = "\033[H\033[2J\033[0m";
void
print_help()
{
        printf("%s", clrscn);
        printf("\033[01;07H*********************************************************************");
        printf("\033[02;15HConvergeNet Technologies - Ethernet Demonstration"); 
        printf("\033[03;07H*********************************************************************");
        printf("\033[05;18Hp) Probe the Devices on the Bus 0");
#ifdef  INCLUDE_ODYSSEY
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

extern  NU_EVENT_GROUP    Buffers_Available;




DEV_DEVICE      device_st;
DEV_DEVICE      *device = &device_st;
char    dev_name[] = "de0";
DV_DEVICE_ENTRY *dev_ptr = (DV_DEVICE_ENTRY *)NU_NULL;
extern  void phy_make_intr(DV_DEVICE_ENTRY *device);
extern  void DEC21143_Printcounters (DV_DEVICE_ENTRY *device);
DV_REQ *d_req;  /* I dont use this */

void
init_devptr()
{
        device->dv_name = dev_name;
#ifdef  INCLUDE_ODYSSEY
        device->dv_hw.ether.dv_irq = 6;
#else
        device->dv_hw.ether.dv_irq = 0;
#endif
        device->dv_init = DEC21143_Init;
                
}
void   
test_0(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
        int     ch;
        int     i=0, j, k;
        U32     padr;
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
                                DEV_Init_Devices(device, 1);
                                dev_ptr = DEV_Get_Dev_By_Name("de0");
                                break;
                        case 'p':
                        case 'P':
                                printf("Devices Found on the PCI Bus 0\n\r");
                                probe_pci_devs(0);
                                break;
#ifdef  INCLUDE_ODYSSEY
                        case 'l':
                        case 'L':
                                printf("Devices Found on the Local Bus \n\r");
                                probe_pci_devs(PCI_E_BUSNUM);
                                break;
                        case 'x':
                        case 'X':
                                if ( dev_ptr != NU_NULL) {
                                        DEC21143_XDATA          *dec_data;
                                        DEC21143_DESCRIPTOR             *desc;
                                        dec_data = (DEC21143_XDATA *) dev_ptr->user_defined_1;
                                        
                                        desc = dec_data->DEC21143_First_TX_Descriptor;
                                        printf("\n\r");
                                        while(desc) {
                                                desc = (DEC21143_DESCRIPTOR *)((U32)(desc) | 0xA0000000);
                                                printf("Control :%08X,   ", desc->control_count);
                                                printf("Status  :%08X,   ", desc->status);
                                                printf("Buffer  :%08X\n\r", desc->buffer);
                                        
                                                desc = desc->next_descriptor;
                                        }
                                } else {
                                        printf("Device not Initialized\n\r");
                                }
                                break;
#endif
                        case 'T':
                        case 't':
                                if ( dev_ptr != NU_NULL) {
                                        send_flag ^= 1;
                                } else {
                                        printf("Device not Initialized\n\r");
                                }
                                break;
                        case 'e':
                        case 'E':
                                if ( dev_ptr != NU_NULL) {
                                        DEC21143_Printcounters (dev_ptr);
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
                                tx_count = 0;
                                rx_count = 0;
                                break;
                        case '+':
                                if ( dev_ptr != NU_NULL) {
                                        maddr[5] = multi_add_byte;
                                        Ether_Add_Multi(dev_ptr, maddr);
                                        multi_add_byte++;
                                        DEC21143_Ioctl(dev_ptr, DEV_ADDMULTI, d_req);
                                } else {
                                        printf("Device not Initialized\n\r");
                                }
                                break;
                        case '-':
                                if ( dev_ptr != NU_NULL) {
                                        maddr[5] = multi_del_byte;
                                        if (Ether_Del_Multi(dev_ptr, maddr) == NU_SUCCESS)
                                                multi_del_byte++;
                                        DEC21143_Ioctl(dev_ptr, DEV_DELMULTI, d_req);
                                } else {
                                        printf("Device not Initialized\n\r");
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
    STATUS              status;
        UNSIGNED                bufs_ava;
        DV_DEVICE_ENTRY *device;
        U8                              *ptr;
        NET_BUFFER              *buf_ptr, *tptr;
        int                             total, i;
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
                                        for(i=0; i < tptr->data_len; i++)
                                                printf("%02X ", ptr[i]);
                                }
                                        
                                tptr = tptr->next_buffer;
                        }
                        if (print_flag)
                                printf("\n\rLen %d\n\r", total);
                        rx_count++;
                        

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
    STATUS              status;
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
    STATUS              status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
        while(1) {
                NU_Sleep(100);
                if ( dev_ptr != NU_NULL) 
                        printf("\r Tx count %d          Rx count %d", tx_count, rx_count);
        }
        
}

void
ether_send()
{
        NET_BUFFER      *buf_ptr, *tptr;
        int                     nbytes;
        
        
        nbytes = 1500;
        

        if (!(dev_ptr-> dev_flags & DV_UP)) {
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

        buf_ptr->data_ptr[6] = 0x00;
        buf_ptr->data_ptr[7] = 0x08;
        buf_ptr->data_ptr[8] = 0x22;
        buf_ptr->data_ptr[9] = 0x2B;
        buf_ptr->data_ptr[10] = 0xDF;
        buf_ptr->data_ptr[11] = 0x79;
                
        /* Place the buffer on the device's transmit queue so that this
       buffer will be freed. */
        DISABLE_INT;
    MEM_Buffer_Enqueue(&dev_ptr->dev_transq, buf_ptr);
        while (DEC21143_TX_Packet(dev_ptr, buf_ptr) != NU_SUCCESS) {
                ENABLE_INT;
                if ( print_flag)
                        printf("Send Failed\n\r");
                DISABLE_INT;
        }
        ENABLE_INT;
        tx_count++;
        printf("\r.");
        if ( print_flag)
                printf("Send Successful\n\r");
}

void NU_Tcp_Log_Error (uint16 err_num, uint8 stat, int8 *file, uint16 line)
{
                /*
        printf("Err %d, Stat %d, File %s, Line %d\n", err_num, stat, file, line);
        */
}


STATUS Ether_Add_Multi(DV_DEVICE_ENTRY *dev, uint8 *multi_addr)
{
    INT         irq_level;
    NET_MULTI   *em;
        int                     i;

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
          em = em->nm_next);

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
        int                     i;

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
          em = em->nm_next);

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