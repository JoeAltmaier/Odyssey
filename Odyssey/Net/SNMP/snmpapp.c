/*************************************************************************/
/*                                                                       */
/*      Copyright (c) 1998 Accelerated Technology, Inc.                  */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*     SNMPAPP.C                                          1.1            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Example of SNMP of Nucleus NET4.0                                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*  Patti Hill                                                           */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*    NAME            DATE           COMMENTS                            */
/*                                                                       */
/*    S. Nguyen       06/01/98       Modified for IDT4640 using SONIC    */
/*                                   driver.                             */
/*    S. Nguyen       08/26/98       Re-organized directory structure    */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
/* Include necessary Nucleus PLUS files.  */

/*
*   Includes
*/

//#ifndef _WINSOCKAPI_
//#define _WINSOCKAPI_
//#endif

#define DECETHER

#include "ansi/stdio.h"
#include "stdlib.h"
#include "string.h"
#include "odyssey_trace.h"

#include "target.h"
#include "externs.h"
#include "nucleus.h"
#include "socketd.h"    /* socket interface structures */
#include "time.h"
#include "snmp_g.h"
#include "snmp.h"
#include "mib.h"
#include "xtern.h"

extern  void    init_galileo();
extern  STATUS  Init_Hardware();

/* SNMP Externs */
extern  void    AddHost(i32 idx, ul32 hst);
extern  i32     GetCommIndex( i8 * );
extern  u32     get_hostid(void);
extern  void    get_macaddr(i32 port, u8 *m);
extern  void    get_macbroad(i32 port, u8 *m);
extern  void    AddMac(i8 *name, u32 type, u16 mtu, ul32 speed, u8 *addr, u8 *addrbroad, u16 addrsize, u8 *host, u16 hostaddrsize);
extern  void    SendENTTrap(u16 gen, u16 spec, snmp_object_t *list, u16 listLen);
extern  bool    MacUpdate( u32 iIndex, i8 *name, u32 operStatus  );

#ifdef DECETHER
#include "dec21143.h"
#endif

extern u32 NUM_ACTIVE_PORTS;

extern unsigned long gSize_available_memory = 0x800000;
extern unsigned long gSize_small_heap = 0x200000;


/* Define Application data structures.  */
NU_MEMORY_POOL    System_Memory;
NU_TASK           tcp_server_task_ptr;

NU_TASK           StrEcho;
NU_TASK           init_xsnmp_task_ptr;
NU_QUEUE          socketQueue;

/* Define prototypes for function references. */
void              tcp_server_task(UNSIGNED argc, VOID *argv);
void              str_echo(UNSIGNED argc, VOID *argv);

void              DEMO_Exit(int n);

#define ECHO_LENGTH 1500
char              line[ECHO_LENGTH + 1];


#define SNMP_Exit DEMO_Exit

/* screen output handle for apl */
#define TESTTIME  300

/* Defines associated with SNMP Demo. */
void               init_xsnmp_task(UNSIGNED argc, VOID *argv);
extern  void       xsnmp_task(void);
extern  void       xsnmp_init2(void);
#define SNMP_STACK 64000
#define init_mib2_objectid "1.3.6.1.4.1.2993.1.1.1.1"
void    x_bcopy( i8 *s1, i8 *s2, u32 len );
#define MAX_SYS_STRINGS 48
#define SNMP_TRAP_GENERAL_TYPE_ENTERPRISE 6
#define SNMP_TRAP_SPECIFIC_TYPE_WARNING 1000
#define SNMP_TRAP_SPECIFIC_TYPE_MINOR 1001
#define SNMP_TRAP_SPECIFIC_TYPE_CRITICAL 1002
#define SNMP_TRAP_SPECIFIC_TYPE_LINKDOWN 1003
#define SNMP_TRAP_SPECIFIC_TYPE_LINKUP 1004
#define SNMP_TRAP_SPECIFIC_TYPE_COLDSTART 1005

int               error = 0;

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      Application_Initialize                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Define the Application_Initialize routine that determines the    */
/*      initial Nucleus PLUS application environment.                    */
/*                                                                       */
/*************************************************************************/
void    Application_Initialize(void *first_available_memory)
{

    VOID           *pointer;
    STATUS         status;

    VOID (*oldlisr)(INT);

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */

    status = NU_Create_Memory_Pool(&System_Memory, "SYSMEM",
                        first_available_memory, 800000, 50, NU_FIFO);

    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit (-1);
    }

    /* Allocate stack space for and create each task in the system.  */

    /* Create tcp_server_task.  */
    status = NU_Allocate_Memory(&System_Memory, &pointer, 5000, NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit (-1);
    }

    status = NU_Create_Task(&tcp_server_task_ptr, "TCPSERV", tcp_server_task,
                        0, NU_NULL, pointer, 5000, 3, 0, NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit (2);
    }

    /* Create SNMP task  */
    status = NU_Allocate_Memory (&System_Memory, &pointer, SNMP_STACK,
                                NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit(1);
    }

    status = NU_Create_Task (&init_xsnmp_task_ptr, "SNMP", init_xsnmp_task, 0,
                    NU_NULL, pointer, SNMP_STACK, 3, 0, NU_PREEMPT, NU_NO_START);

    if (status != NU_SUCCESS)
    {
        error++;
        SNMP_Exit(1);
    }

}   /* end Application_Initialize */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCP_server_task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a task that will accept connections from   */
/*      clients.  The socket descriptor for the connection is then placed*/
/*      on the queue.  The StrEcho task will then remove the Socket      */
/*      Descriptor from the queue.                                       */
/*                                                                       */
/*************************************************************************/
void tcp_server_task(UNSIGNED argc, VOID *argv)
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

    Tracef("\n\r");

/* Initialize the Hardware */
#ifdef  INCLUDE_ODYSSEY
        /* Initialize LISRs for Unhandled Interrupts */
//        for(i=0; i < 8; i++) {
//                NU_Register_LISR(i, m_lih, &o_lih);
//        }
        if ((status = Init_Hardware()) != NU_SUCCESS)
                        printf("Error in init %d\r\n", status);
//        NU_Control_Interrupts(0xC400);
        NU_Control_Interrupts(0x1C400);
#else
        init_galileo();
#endif  INCLUDE_ODYSSEY



    if (NU_Init_Net())  /* call network initialization */
    {
        Tracef("snmpapp - Error at call to NU_Initialize() from TCP_server_task\n\r");
        DEMO_Exit(0);
    }
    Tracef("snmpapp - Network Initialized..\n");

    status = NU_Allocate_Memory( &System_Memory, (VOID **)&sys_group,
                                 sizeof(*sys_group),
                                 NU_NO_SUSPEND);
    if(status != NU_SUCCESS)
    {
        Tracef("snmpapp - NU_Allocate_Memory failed, status = %d\n", status);
        error++;
        NU_Suspend_Task(NU_Current_Task_Pointer());
        SNMP_Exit(0);
    }

    Tracef("snmpapp - Calling SNMP_Initialize\n");
    SNMP_Initialize();
    Tracef("snmpapp - Calling SNMP_System_Group_Initialize\n");
    SNMP_System_Group_Initialize(sys_group);
    Tracef("snmpapp - Calling NU_Deallocate_Memory\n");
    NU_Deallocate_Memory(sys_group);

#ifdef PPP
        /* PPP Initialization */
        memcpy (devices[0].dv_ip_addr, null_ip, 4);         /* Not use by PPP. */
        memcpy (devices[0].dv_subnet_mask, subnet, 4); /* Not use by PPP. */
        devices[0].dv_name = "PPP_Link";
        devices[0].dv_init = PPP_Initialize;
        devices[0].dv_flags = (DV_POINTTOPOINT | DV_NOARP);

        devices[0].dv_hw.uart.com_port         = COM1;
        devices[0].dv_hw.uart.baud_rate        = 57600;
        devices[0].dv_hw.uart.parity           = PARITY_NONE;
        devices[0].dv_hw.uart.stop_bits        = STOP_BITS_1;
        devices[0].dv_hw.uart.data_bits        = DATA_BITS_8;


        if(NU_Init_Devices(devices, 1) == NU_SUCCESS)
        {
           NU_Change_Communication_Mode(MDM_TERMINAL_COMMUNICATION);

            /* This while loop simply receives strings from the serial port and echos
               back OK in response.  This code is used to trick Windows into thinking
               that it is communicating with modem, when in fact it is connected via
               null modem to an embedded device.
            */
            while(1)
            {
                /* Receive a MODEM command. */
                DEMO_Get_Modem_String(mstring);

                /* Respond with OK. */
                NU_Modem_Control_String("OK\n\r");

                /* If the command received was a command to dial then Windows 95 now
                   thinks that it has a modem connection to a remote HOST.  Get out of
                   this loop because data exchanged beyond this point will be in the
                   form of IP packets. */
                if (strncmp(mstring, "ATDT", 4) == 0)
                    break;
            }

            /* Act like a modem. */
            NU_Modem_Control_String("CONNECT 57600\n\r");

            /* Change to SERVER mode */
            NCP_Change_IP_Mode (SERVER);

            /* Set the IP address to assign to the client */
            NU_Set_PPP_Client_IP_Address (client_ip_address);

            /* Switch to PPP mode. */
            NU_Change_Communication_Mode(MDM_NETWORK_COMMUNICATION);

            /* Start the PPP negotiation. This call is only used for the null modem
               demos. It is normally handled by NU_Wait_For_PPP_Client. */

            status = PPP_Lower_Layer_Up(server_ip_address);


            if (status == NU_SUCCESS)
            {

                /* Get the subnet mask for this type of address. */
                IP_Get_Net_Mask ((CHAR *)server_ip_address, (CHAR *)subnet);

                /* Set our new address */
                DEV_Attach_IP_To_Device ("PPP_Link", server_ip_address, subnet);
            }

        }
#endif
#ifdef DECETHER

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
      Tracef("snmpapp - Ethernet device up\n");
    }
    else
    {
      Tracef("snmpapp - NU_Init_Devices fail! (%d)\n", status);
    }

//    NU_Add_Route(netwrk, subnet, router);

#endif

    NU_Resume_Task(&init_xsnmp_task_ptr);

    NU_Suspend_Task(NU_Current_Task_Pointer());

} /* tcp_server_task */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      str_echo                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     Read a stream socket one line at a time                           */
/*     and write each line back to the sender.                           */
/*     Return when the connection is terminated.                         */
/*                                                                       */
/*************************************************************************/
char where_we_are;
void str_echo(UNSIGNED argc, VOID *argv)
{
    int       exit_err = 100;
    int16     bytes_recv, bytes_sent;
    int16     sockfd;
    STATUS    status;
    UNSIGNED  actSize;

    /*  Remove compilation warnings for unused parameters.  */
    status = (STATUS) argc + (STATUS) argv;

    status = NU_Receive_From_Queue(&socketQueue, (UNSIGNED *) &sockfd, 1,
                                    &actSize, NU_SUSPEND);
    if (status != NU_SUCCESS)
    {
        Tracef("snmpapp - Unable to receive a message from socket Queue\n\r");
        SNMP_Exit (-1);
    }

    /* turn on the "block during a read" flag */
    NU_Fcntl(sockfd, NU_SETFLAG, NU_BLOCK);

    while (1)
    {
        where_we_are = 0;
        bytes_recv = NU_Recv(sockfd, line, ECHO_LENGTH, 0);

        where_we_are = 10;
        if (bytes_recv == NU_NOT_CONNECTED)
        {
            exit_err = where_we_are;
            break;
        }
        else if (bytes_recv < 0)
            Tracef("\nsnmpapp - str_echo: NU_Recv error\n\r");
        else
        {
            line[bytes_recv] = 0x0;
            Tracef("snmpapp - Received from the client: %s\n\r", line);
        }

        where_we_are = 15;
        bytes_sent = NU_Send(sockfd, line, bytes_recv, 0);
        where_we_are = 20;
        if(bytes_sent != bytes_recv)
        {
            exit_err = where_we_are;
            Tracef("\nsnmpapp - str_echo: NU_Send error\n\r");
        }

        where_we_are = 25;
        NU_Sleep(2);
    }

    /* turn off the "block during a read" flag -
             other reads may not want to block  */
    NU_Fcntl(sockfd, NU_SETFLAG, NU_FALSE);

        where_we_are = 30;
    /* close the connection */
    if ((NU_Close_Socket(sockfd)) != NU_SUCCESS)
    {
        Tracef("\nsnmpapp - Error from NU_Close_Socket.\n\r");
        exit_err = where_we_are;
    }

    /*  Indicate that all went well. */
    SNMP_Exit(exit_err);

    NU_Delete_Task(NU_Current_Task_Pointer());
} /* str_echo */



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
    u32             host;
    snmp_object_t   *myTrap;
    snmp_syntax_t   *mySyntax;
    char            *sLocation;
    static u8       buf[49];
    static u8       mac[6];
    static u8       broad[6];


    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    x_bcopy((i8 *)init_mib2_objectid, (i8 *)(rfc1213_vars.rfc1213_sys.sysObjectID), MAX_SYS_STRINGS );
    xsnmp_init2();

    /* Change mib2 system fields */
    SNMP_sysUpTime(0);
    SNMP_sysDescr("Version 0.36 on Sept 23, 1999");
    SNMP_sysContact("www.convergenet.com");
    SNMP_sysLocation("Nashua, NH");
    SNMP_sysName("Gemini Eval System");
    //SNMP_sysObjectID("1.3.6.1.4.1.2993.1.1.1.1.4");
    SNMP_sysServices(72);                               //IP host + Applications (8 + 64)
    SNMP_ipForwarding(2);                               //not-forwarding

    /* Add management stations */
    AddHost( GetCommIndex( "public" ), 0x0A1E011CUL );  /* Add 10.30.1.28 (Loon) */
    //AddHost( GetCommIndex( "public" ), 0x0A1E0116UL );  /* Add 10.30.1.22 (Moose) */
    //AddHost( GetCommIndex( "public" ), 0x0A1E0127UL );  /* Add 10.30.1.39 (Khan) */

   /*host = get_hostid();
    get_macaddr(0, mac);
    get_macbroad(0, broad);
    sprintf((i8 *) buf, "HBC_in_Slot_0" );
    AddMac( (i8 *)buf, 6L, 1500L, 10000000L, mac, broad, 6L, (u8 *)&host, 4L);
    sprintf((i8 *) buf, "HBC_in_Slot_1" );
    AddMac( (i8 *)buf, 6L, 1500L, 10000000L, mac, broad, 6L, (u8 *)&host, 4L);
    sprintf((i8 *) buf, "NAC_in_Slot_a1" );
    AddMac( (i8 *)buf, 0L, 0L, 0L, mac, broad, 6L, (u8 *)&host, 4L);
    sprintf((i8 *) buf, "NAC_in_Slot_a2" );
    AddMac( (i8 *)buf, 0L, 0L, 0L, mac, broad, 6L, (u8 *)&host, 4L);
    sprintf((i8 *) buf, "SSD_in_Slot_a3" );
    AddMac( (i8 *)buf, 0L, 0L, 0L, mac, broad, 6L, (u8 *)&host, 4L);
    sprintf((i8 *) buf, "SSD_in_Slot_a4" );
    AddMac( (i8 *)buf, 0L, 0L, 0L, mac, broad, 6L, (u8 *)&host, 4L);*/

    MacUpdate( 0, "HBC_in_Slot_0", 1);
    NUM_ACTIVE_PORTS++;
    MacUpdate( 1, "HBC_in_Slot_1", 1);
    NUM_ACTIVE_PORTS++;
    MacUpdate( 2, "SSD_in_Slot_a1", 1);
    NUM_ACTIVE_PORTS++;
    MacUpdate( 3, "SSD_in_Slot_a2", 2);
    NUM_ACTIVE_PORTS++;
    MacUpdate( 4, "NAC_in_Slot_a3", 1);
    NUM_ACTIVE_PORTS++;
    MacUpdate( 5, "NAC_in_Slot_a4", 1);
    NUM_ACTIVE_PORTS++;

    /*myTrap = (snmp_object_t *)x_malloc(2 * sizeof(snmp_object_t));
    myTrap->Request = SNMP_PDU_GET;
    //myTrap->Id = {1, 3, 6, 1, 4, 1, 2993, 3, 1, 1, 2};
    myTrap->Id[0] = 1;
    myTrap->Id[1] = 3;
    myTrap->Id[2] = 6;
    myTrap->Id[3] = 1;
    myTrap->Id[4] = 4;
    myTrap->Id[5] = 1;
    myTrap->Id[6] = 2993;
    myTrap->Id[7] = 3;
    myTrap->Id[8] = 1;
    myTrap->Id[9] = 1;
    myTrap->Id[10] = 2;
    myTrap->IdLen = 11;
    myTrap->Type = SNMP_OCTETSTR;
    sLocation = "SSD_in_Slot_a4";
    sprintf((i8 *) myTrap->Syntax.BufChr, sLocation );
    myTrap->SyntaxLen = strlen(sLocation);

    myTrap++;
    myTrap->Request = SNMP_PDU_GET;
    myTrap->Id[0] = 1;
    myTrap->Id[1] = 3;
    myTrap->Id[2] = 6;
    myTrap->Id[3] = 1;
    myTrap->Id[4] = 4;
    myTrap->Id[5] = 1;
    myTrap->Id[6] = 2993;
    myTrap->Id[7] = 3;
    myTrap->Id[8] = 1;
    myTrap->Id[9] = 1;
    myTrap->Id[10] = 3;
    myTrap->IdLen = 11;
    myTrap->Type = SNMP_OCTETSTR;
    sLocation = "Exceeded temperature threshhold";
    sprintf((i8 *) myTrap->Syntax.BufChr, sLocation );
    myTrap->SyntaxLen = strlen(sLocation);
    myTrap--;

    SendENTTrap(SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, SNMP_TRAP_SPECIFIC_TYPE_CRITICAL, myTrap, 2);*/

    SendENTTrap(SNMP_TRAP_GENERAL_TYPE_ENTERPRISE, SNMP_TRAP_SPECIFIC_TYPE_COLDSTART, 0, 0);

     xsnmp_task();

}  /* init_xsnmp_task */


void DEMO_Exit(int n)
{
    NU_Sleep(36);
    exit(n);
}



