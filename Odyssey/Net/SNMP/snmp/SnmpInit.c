#include "OsTypes.h"
#include "target.h"
#include "externs.h"
#include "nucleus.h"
#include "snmp_g.h"
#include "snmp.h"

NU_TASK           init_xsnmp_task_ptr;
extern NU_MEMORY_POOL    System_Memory;
void              init_xsnmp_task(UNSIGNED argc, VOID *argv);

/* Defines associated with SNMP. */
extern  void  xsnmp_task(void);
extern  void  xsnmp_init2(void);

#define SNMP_STACK 96000
#define init_mib2_objectid "1.3.6.1.4.1.2993.1.1.1.1"
#define MAX_SYS_STRINGS 48

SNMP_System_Group   *sys_group;
VOID   *pointer;

STATUS NU_Init_Snmp(void *first_available_memory)
{
    STATUS status;
    VOID   *pointer;

    printf("NU_Init_Snmp entered\n");
    //printf("SNMP first_available_memory = %8x\n", first_available_memory);
    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM", first_available_memory, 200000, 56, NU_FIFO);
    if (status != NU_SUCCESS)
    {
        printf("NU_Init_Snmp - NU_Create_Memory_Pool failed\n");
        printf("status = %x\n", status);
        return(-1);
    }

    status = NU_Allocate_Memory( &System_Memory, (VOID **)&sys_group,
                                 sizeof(*sys_group), NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
    {
        printf("NU_Init_Snmp - NU_Allocate_Memory of sys_group failed\n");
        printf("status = %x\n", status);
        return(-2);
    }
    printf("Calling SNMP_System_Group_Initialize\n");
    SNMP_System_Group_Initialize(sys_group);

    /* Allocate stack space for SNMP task in the system.  */
    status = NU_Allocate_Memory (&System_Memory, &pointer, (UNSIGNED)SNMP_STACK, (UNSIGNED)NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {
        printf("NU_Init_Snmp - NU_Allocate_Memory of SNMP_STACK failed\n");
        printf("status = %x\n", status);
        return(-3);
    }

    /* Create SNMP task */
    printf("Starting SNMP task\n");
    status = NU_Create_Task (&init_xsnmp_task_ptr, "SNMP", init_xsnmp_task, (UNSIGNED)0,
                    NU_NULL, pointer, (UNSIGNED)SNMP_STACK, (UNSIGNED)3, (UNSIGNED)0, NU_PREEMPT, NU_START);

    if (status != NU_SUCCESS)
    {
        printf("NU_Init_Snmp - NU_Create_Task failed\n");
        return(-4);
    }

    return 0;
}   /* end NU_Init_Snmp */


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
    STATUS status;
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    printf("NU_Init_Snmp - init_xsnmp_task entered\n");
    memcpy((void *)(rfc1213_vars.rfc1213_sys.sysObjectID), (const void *)init_mib2_objectid, MAX_SYS_STRINGS);

    printf("NU_Init_Snmp - xsnmp_init2\n");
    xsnmp_init2();

    /* Change mib2 system fields */
    SNMP_sysUpTime(0);
    //SNMP_sysDescr("Build 1.5 on Sept 23, 1999");        // jbytest
    //SNMP_sysContact("www.convergenet.com");             // jbytest
    //SNMP_sysLocation("Nashua, NH");                     // jbytest
    //SNMP_sysName("Gemini System");                      // jbytest
    SNMP_sysServices(72);                               //IP host + Applications (8 + 64)  jbytest
    SNMP_ipForwarding(2);                               //not-forwarding  jbytest
    AddHost( GetCommIndex( "public" ), 0x0A1E011CUL );  /* jbytest - add 10.30.1.28 (Loon) */
    //AddHost( GetCommIndex( "public" ), 0x0A1E0116UL );  /* jbytest - add 10.30.1.22 (Moose) */
    printf("NU_Init_Snmp - xsnmp_task\n");
    xsnmp_task();
    printf("NU_Init_Snmp - init_xsnmp_task complete\n");
}  /* init_xsnmp_task */

