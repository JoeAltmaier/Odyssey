#define _TRACEF
#define DECETHER

#include "ansi/stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Odyssey_trace.h"
#include "Trace_Index.h"
#include "target.h"
#include "externs.h"
#include "nucleus.h"
#include "socketd.h"    /* socket interface structures */
#include "time.h"
#include "snmp_g.h"
#include "snmp.h"
#include "mib.h"
#include "xtern.h"
#include "dec21143.h"

#include "Os.h"
#include "Odyssey_Trace.h"

extern  long    TraceLevel[];

extern  void    init_galileo();

/* Define Application data structures.  */
NU_MEMORY_POOL    System_Memory;
NU_TASK           dec21143_task_ptr;
NU_TASK           init_xsnmp_task_ptr;

/* Define prototypes for function references. */
void              dec21143_task(UNSIGNED argc, VOID *argv);
void              init_xsnmp_task(UNSIGNED argc, VOID *argv);

/* Defines associated with SNMP. */
extern  "C" void  xsnmp_task(void);
extern  "C" void  xsnmp_init2(void);
#define SNMP_STACK 64000

void    DEMO_Exit(int n);
#define SNMP_Exit DEMO_Exit

int               error = 0;

void StartTask(VOID* pParam)
{
    //TraceLevel[TRACE_DDM_MGR] = 8;
    //TraceLevel[TRACE_DDM] = 8;
    //Oos::DumpTables("BuildSys Tables");

    Oos::Initialize();
}

void    Application_Initialize(void *first_available_memory)
{

    VOID           *pointer;
    STATUS         status;

    Tracef("\n\r");
    Tracef("Application_Initialize\n");

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM",
                        first_available_memory, 800000, 50, NU_FIFO);
    if (status != NU_SUCCESS)
    {
        Tracef("NU_Create_Memory_Pool error\n");
        error++;
        SNMP_Exit (-1);
    }

    /* Allocate stack space for DEC21143 task in the system.  */
    status = NU_Allocate_Memory(&System_Memory, &pointer, 5000, NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit (-2);
    }

    /* Create dec21143_task.  */
    status = NU_Create_Task(&dec21143_task_ptr, "DECSERV", dec21143_task,
                        0, NU_NULL, pointer, 5000, 3, 0, NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit(-3);
    }

    /* Allocate stack space for DEC21143 task in the system.  */
    status = NU_Allocate_Memory (&System_Memory, &pointer, SNMP_STACK, NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit(-4);
    }

    /* Create SNMP task */
    status = NU_Create_Task (&init_xsnmp_task_ptr, "SNMP", init_xsnmp_task, 0,
                    NU_NULL, pointer, SNMP_STACK, 3, 0, NU_PREEMPT, NU_NO_START);

    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit(-5);
    }

    Tracef("dec21143_task - calling StartTask\n");
    StartTask(NULL);

    //while(1) {
        //NU_Sleep(5000);
    //}

    //Tracef("SNMP_Test exiting\n");
    //return;
}   /* end Application_Initialize */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      dec21143_task                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a task that will accept connections from   */
/*      clients.  The socket descriptor for the connection is then placed*/
/*      on the queue.                                                    */
/*                                                                       */
/*************************************************************************/
void dec21143_task(UNSIGNED argc, VOID *argv)
{
    int16               socketd, newsock;       /* the socket descriptor */
    struct addr_struct  servaddr;               /* holds the server address structure */
    unsigned int        i;
    STATUS              status;
    struct addr_struct  client_addr;
    uint32              ipaddr;
    SNMP_System_Group   *sys_group;
    uint8               ip_addr[] = {10,30,1,41};
    uint8               subnet[]  = {255,255,255,0};
    NU_DEVICE           devices[1];

/* Initialize the Hardware */
   init_galileo();

    if (NU_Init_Net())  /* call network initialization */
    {
        Tracef("dec21143_task - Error at call to NU_Initialize() from TCP_server_task\n\r");
        DEMO_Exit(0);
    }
    Tracef("dec21143_task - Network Initialized..\n");

    status = NU_Allocate_Memory( &System_Memory, (VOID **)&sys_group,
                                 sizeof(*sys_group),
                                 NU_NO_SUSPEND);
    if(status != NU_SUCCESS)
    {
        Tracef("dec21143_task - NU_Allocate_Memory failed, status = %d\n", status);
        error++;
        NU_Suspend_Task(NU_Current_Task_Pointer());
        SNMP_Exit(0);
    }

    SNMP_Initialize();
    Tracef("dec21143_task - SNMP initialized\n");

    SNMP_System_Group_Initialize(sys_group);
    Tracef("dec21143_task - SNMP System Group initialized\n");

    NU_Deallocate_Memory(sys_group);
    Tracef("dec21143_task - NU_Deallocate_Memory called\n");

    /* set up the ethernet */
    devices[0].dv_name = "DEC21143_0";
    devices[0].dv_hw.ether.dv_irq = 0;             /* The DEC21143 does */
    devices[0].dv_hw.ether.dv_io_addr = 0x0L;      /* not use these     */
    devices[0].dv_hw.ether.dv_shared_addr = 0;     /* fields.           */

    devices[0].dv_init = DEC21143_Init;
    devices[0].dv_flags = 0;
    memcpy (devices[0].dv_ip_addr, ip_addr, 4);
    memcpy (devices[0].dv_subnet_mask, subnet, 4);
    devices[0].dv_use_rip2 = 0;
    devices[0].dv_ifmetric = 0;
    devices[0].dv_recvmode = 0;
    devices[0].dv_sendmode = 0;

    status = NU_Init_Devices(devices, 1);
    if (status == NU_SUCCESS)
    {
      Tracef("dec21143_task - Ethernet device up\n");
    }
    else
    {
      Tracef("dec21143_task - NU_Init_Devices fail! (%d)\n", status);
    }

    NU_Resume_Task(&init_xsnmp_task_ptr);

    NU_Suspend_Task(NU_Current_Task_Pointer());

} /* dec21143_task */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      init_xsnmp_task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a task that will wait for data             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
void init_xsnmp_task(UNSIGNED argc, VOID *argv)
{
    STATUS          status;

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    xsnmp_init2();

    xsnmp_task();
}  /* init_xsnmp_task */


void DEMO_Exit(int n)
{
    NU_Sleep(10);
    exit(n);
}
