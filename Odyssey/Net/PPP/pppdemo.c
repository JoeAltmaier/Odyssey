/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: erc.c
//
// Description:
// This file contains the Application_Initialize() routine, which is the
// main application level module.  The demo version starts several tasks
// to demonstrate the capabilities of various synchronization methods.
//
//
// Update Log:
// 10/06/98 Jeff Nespor: Modifications previously made to R4640 source.
//                       - Added serial i/o and the demo screen, providing
//                         a convenient place to hang code which can be
//                         executed at a particular keystroke.
//                       - Removed original serial i/o and LED display code.
// 11/03/98 Jeff Nespor: Removed a couple of unused variables from tasks.
/*************************************************************************/

#define PLUS

/* Include necessary Nucleus PLUS files.  */

#include  "externs.h"
#include  "socketd.h"
#include  "tcpdefs.h"
#include  "nucleus.h"
#include  "tc_defs.h"
#include  "ppp.h"
#include  "string.h"

	extern unsigned long gSize_available_memory = 0x400000;
	extern unsigned long gSize_small_heap = 0x200000;


/* This is the IP address that will be assigned to a client during PPP
   negotiaton. */
uint8             client_ip_address[] =  {172, 16, 1, 2};

/* This is the IP address the server will use. */
uint8             server_ip_address[] =  {172, 16, 1, 1};

/* Define Application data structures.  */

NU_MEMORY_POOL    System_Memory;
NU_TASK           tcp_server_task_ptr;
NU_TASK           StrEcho0;
NU_TASK           StrEcho1;
NU_QUEUE          socketQueue;

/* Define prototypes for function references.  */

VOID              tcp_server_task(UNSIGNED argc, VOID *argv);
VOID              str_echo(UNSIGNED argc, VOID *argv);
CHAR              *DEMO_Get_Modem_String(CHAR *response);
VOID              Error_Loop (INT error_num);

#define ECHO_LENGTH 1500
char              line[ECHO_LENGTH + 1];

/* Define Application data structures.  */
NU_MEMORY_POOL  System_Memory;
NU_TASK         Task_0;
NU_TASK         Task_1;
NU_QUEUE          socketQueue;

/* Allocate global counters. */
UNSIGNED  Net_Status;

/* Define prototypes for function references.  */
VOID    task_0(UNSIGNED argc, VOID *argv);
VOID	str_echo(UNSIGNED argc, VOID *argv);

/* Network global structures */
NU_DEVICE	devices[1];

/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
void    Application_Initialize(void *first_available_memory)
{
    VOID    *pointer;
    STATUS         status;

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
                        first_available_memory, 400000, 56, NU_FIFO);
    
    /* Create task 0.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_0, "TASK 0", task_0, 0, NU_NULL, pointer,
                   2000, 1, 20, NU_PREEMPT, NU_START); 

    /* Create strech task  */
    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    NU_Create_Task(&Task_1, "strecho", str_echo, 0, NU_NULL,
                pointer, 2000, 3, 0, NU_PREEMPT, NU_START);

    /* Create Socket queue.  */
    NU_Allocate_Memory(&System_Memory, &pointer, 10*sizeof(UNSIGNED),
                                                        NU_NO_SUSPEND);
    status = NU_Create_Queue(&socketQueue, "SocQueue", pointer, 10, NU_FIXED_SIZE,
                             1, NU_FIFO);


}

void  task_0(UNSIGNED argc, VOID *argv)
{
	STATUS	status, sstatus;
	uchar	subnet[] = {255,255,255,255};
    char	null_ip[] = {0, 0, 0, 0};   /* Not used by PPP */
	int		socketd, newsock;
//	struct	addr_struct	*servaddr;
	struct	addr_struct	client_addr;	
	VOID	*pointer;
	uchar	ipaddr[] = {0, 0, 0, 0};
	char      string[20];
    char                mstring[80];
    struct addr_struct  servaddr;               /* holds the server address structure */
    unsigned int i;

	NU_IOCTL_OPTION     ppp_ioctl;
	
    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;
    
    Net_Status = 0;
       
    status = NU_Init_Net();

    if (status == NU_SUCCESS) {

      
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
   	   Net_Status = 1;    
   	   
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
//    sprintf (mstring, "CONNECT 57600\n\r");
    NU_Modem_Control_String("CONNECT 57600\n\r");

    /* Change to SERVER mode */
    NCP_Change_IP_Mode (SERVER);

    /* Set the IP address to assign to the client */
    NU_Set_PPP_Client_IP_Address (client_ip_address);

    /* Switch to PPP mode. */
    NU_Change_Communication_Mode(MDM_NETWORK_COMMUNICATION);

    /* Start the PPP negotiation. This call is only used for the null modem
       demos. It is normally handled by NU_Wait_For_PPP_Client. */

//    NU_Set_PPP_Login ("rbraun", "xxx");
    
    status = PPP_Lower_Layer_Up(server_ip_address);


    if (status == NU_SUCCESS)
    {
    
            /* Get the subnet mask for this type of address. */
        IP_Get_Net_Mask ((CHAR *)server_ip_address, (CHAR *)subnet);

        /* Set our new address */
        DEV_Attach_IP_To_Device ("PPP_Link", server_ip_address, subnet);

        /* open a connection via the socket interface */
        if ((socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0))>=0)
        {


            /* Fill in a structure with the server address. */
            servaddr.family    = NU_FAMILY_IP;
            servaddr.port      = 7;
            *((uint32 *)servaddr.id.is_ip_addrs) = IP_ADDR_ANY;

            /* make an NU_Bind() call to bind the server's address */
            if ((NU_Bind(socketd, &servaddr, 0))>=0)
            {
              /* be ready to accept connection requests */
              status = NU_Listen(socketd, 10);

              if (status == NU_SUCCESS)
              {
                  /* Listen for up to 2 connections. */
                  for (i=0; i < 1; i++)
                  {
                      /* Accept a connection.  This service blocks by default until
                         a client has connected.
                      */
                      newsock = NU_Accept(socketd, &client_addr, 0);

                      /* If a client has connected then send the socket descriptor
                         to one of the echo tasks. */
                      if (newsock >= 0)
                      {

                          /* process the new connection */
                          status = NU_Send_To_Queue(&socketQueue,
                                                (UNSIGNED *)&newsock,
                                                1, NU_SUSPEND);
                          NU_Sleep(2);

                      } /* end successful NU_Accept */
                  }  /* end for loop */

              } /* end successful NU_Listen */
            } /* end successful NU_Bind */



        }
    }
/*

   	   if (status == NU_SUCCESS)
   	     Net_Status = 123;
   	   
   	}
    else
       Net_Status = 2;
             
    }
    
    if (status == NU_MEM_ALLOC)
      Net_Status = 666;
    
    if (status == NU_INVAL)
      Net_Status = 666;
    
    if (Net_Status == 0)
      Net_Status = 666;
      */

//	Convert_Number(string,Net_Status);
//  Print_String(string);
}
}

}


VOID    str_echo(UNSIGNED argc, VOID *argv)
{
    int       connected = 1;
    int       bytes_recv, bytes_sent;
    int       sockfd;
    STATUS    status;
    UNSIGNED  actSize;

    /*  Remove compilation warnings for unused parameters.  */
    status = (STATUS) argc + (STATUS) argv;

    status = NU_Receive_From_Queue(&socketQueue, (UNSIGNED *) &sockfd, 1,
                                      &actSize, NU_SUSPEND);

    /* Turn on the "block during a read" flag.  NU_Receive is non-blocking by
       default.  This will cause the receive call to block until data is
       received.
    */
    NU_Fcntl(sockfd, NU_SETFLAG, NU_BLOCK);

    while(connected)
    {

         /* Receive some data. */
         bytes_recv = NU_Recv(sockfd, line, ECHO_LENGTH, 0);

         /* Check to see if the client closed his end of the connection.  If so
            then we need to close this end as well. */
         if (bytes_recv < 0)
         {
             connected = 0;
             continue;
         }
         else
             /* Echo the data back to the client. */
             bytes_sent = NU_Send(sockfd, line, bytes_recv, 0);
    }

    /* close the connection */
    NU_Close_Socket(sockfd);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION                                                                   */
/*                                                                            */
/*      DEMO_Get_Modem_String                                                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*     This function is used to receive "modem" commands from Windows 95.     */
/*                                                                            */
/******************************************************************************/
CHAR *DEMO_Get_Modem_String(CHAR *response)
{
    CHAR            c;
    CHAR            *write_ptr;

    write_ptr = response;
    *write_ptr = NU_NULL;

    while (1)
    {
		/* get a character from the port if one's there */
        if (NU_Terminal_Data_Ready())
        {
            NU_Get_Terminal_Char(&c);

            switch (c)
            {
                case 0xD:                /* CR - return the result string */
                    if (*response)
                        return response;
					continue;
				default:
                    if (c != 10)
                    {      /* add char to end of string */
                        *write_ptr++ = (char)c;
                        *write_ptr = NU_NULL;
						/* ignore RINGING and the dial string */
					}
			}
		}
        else
            NU_Sleep(5);

	}

} /* DEMO_Get_Modem_String */

