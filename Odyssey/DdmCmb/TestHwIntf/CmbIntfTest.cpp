/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CmbIntfTest.cpp
// 
// Description:
//    This file contains a Nucleus-based test program for exercising
//    the low-level CMB / MIPS interface code.
//
//    Note that the interface code is contained in file ..\CmbIsr.cpp,
//    which is the same file used in the actual CMB interface DDM.
//    Thus any fixes made using this test program will be immediately
//    reflected in the CMB DDM.
// 
// $Log: /Gemini/Odyssey/DdmCmb/TestHwIntf/CmbIntfTest.cpp $
// 
// 1     7/20/99 5:37p Ewedel
// [At Taylor] Initial revision.
// 
/*************************************************************************/


/* Include necessary Nucleus PLUS files.  */

#include  "nucleus.h"
#include  "tc_defs.h"
#include  "cmbhwintfmsgs.h"	
#include  "ansi/stdio.h"

extern	void CMB_Initialize(void) ;
extern	void	BuildCMBreceived(char *) ;
extern	STATUS	CMB_SendCommand (unsigned char *) ;

// CMB stuff
	CmbPacket		CmdPacket;
	
extern "C" {
	char	CmdString[80] ;
	unsigned char	unsolicited = 0 ;
extern	unsigned char	CMB_My_ID ;
NU_TASK         Task_0;
NU_QUEUE        Queue_1;
NU_SEMAPHORE    Semaphore_0;
NU_EVENT_GROUP  Event_Group_0;
NU_MEMORY_POOL  System_Memory;


/* Allocate global counters. */
UNSIGNED  Task_Time;
unsigned  long	received_size ;
unsigned  long	*pReceived_size = &received_size ;
unsigned  		Receive_Message ;

/* Define prototypes for function references.  */
void    probe_pci(void);  /*JSNX*/
void    CmbDemoTask(UNSIGNED argc, VOID *argv);
void    task_1(UNSIGNED argc, VOID *argv);
void    task_2(UNSIGNED argc, VOID *argv);
unsigned char	Get_MIPS_ID (void) ;
unsigned char	GetResponseToMasterCommand(void) ; 
unsigned char	GetMasterCommand(void) ; 
unsigned char	GetLocalCommand(void) ; 


};  /* end of extern "C" */


//  here are a couple of symbols which our CHAOS-tailored MSL needs

//  eval board has 8MB, but we're only supposed to report the amount
//  which our load image is not using.. hmm!
//  [This tiny number should be fine on real Odyssey hardware, which
//   is where we run.]
extern "C" U32  gSize_available_memory = 0x480000;

//  size of small "non-fragmenting" heap (twice this amount is used
//  by the small heap when in debug mode)
extern "C" U32  gSize_small_heap = 0x100000;


/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */
   
//void    Application_Initialize(void *first_available_memory)
extern "C"
void  StartTask(UNSIGNED , VOID *)
{
    VOID    *pointer;

    printf("Here we go...\n");

    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
//    NU_Create_Memory_Pool(&System_Memory, "SYSMEM", 
//                        first_available_memory, 40000, 56, NU_FIFO);
                        
    /* Create each task in the system.  */
    
/* initialize MIPS ISR interface with CMA */
	CMB_Initialize() ;
    /* Create task 0.  */
//    NU_Allocate_Memory(&System_Memory, &pointer, 2000, NU_NO_SUSPEND);
    pointer = new char [2000];
    NU_Create_Task(&Task_0, "CMB Demo Task", CmbDemoTask, 0, NU_NULL, pointer,
                   2000, 1, 20, NU_PREEMPT, NU_START);


    /* Create communication queue.  */
//    NU_Allocate_Memory(&System_Memory, &pointer, 100*sizeof(UNSIGNED), 
//                       NU_NO_SUSPEND);
    pointer = new char [100];
    NU_Create_Queue(&Queue_1, "QUEUE 0", pointer, 100, NU_FIXED_SIZE, 
                    1, NU_FIFO);

    /* Create synchronization semaphore.  */
    NU_Create_Semaphore(&Semaphore_0, "SEM 0", 1, NU_FIFO);
    
    /* Create event flag group.  */
    NU_Create_Event_Group(&Event_Group_0, "EVGROUP0");

    #ifdef USE_SERIAL
    Serial_Init (1, 19200, 0, 0, 0);
    #endif
}


/*
01    *********************************************************************
02            ConvergeNet Technologies - Nucleus PLUS Demonstration
03    *********************************************************************
04   
05               Task0 <EVENT>     Events Set:        NNNNNNNNNN
24
*/


/* Define a task that sleeps for a period of time and then sets 
   an event flag that wakes up task 5.  */  
#define GOOD_STATUS	0

void   CmbDemoTask(UNSIGNED argc, VOID *argv)
{
    STATUS    status;
    char      string[20];
	unsigned  char	command, destination ;
    unsigned  more=1;
    unsigned  key = 0;
    unsigned  screen_refresh = 1;
    int       a = 12; /*JSNX*/
    int       b = 15; /*JSNX*/
    int       p =  0; /*JSNX*/

    /* useful ascii escape sequences */
    char *clreol = "\033[K";                /* clear to the end of the line */
    char *clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
    char *bspace = "\033[D \033[D";         /* backspace */
    char *cpecho = "\033[12;64H";           /* cursor position for keystroke echo */

    /* Access argc and argv just to avoid compilation warnings.  */
    status =  (STATUS) argc + (STATUS) argv;

    /* Set the clock to 0.  This clock ticks every 10 system timer ticks. */
    Task_Time =  0;
    /* Task0 forever loop */
    while(more)
    {

	    if (screen_refresh)
        {
            printf("\n");
            printf("\n*********************************************************************");
            printf("\n     ConvergeNet Technologies - Nucleus PLUS CMB simulation");
            printf("\n*********************************************************************");
		}
	    printf("\n\r Enter 'M' for MIPS command or 'C' for CMA command or 'X' to exit \n\r");

        switch (key = getchar())
		{
			case 'M':
			case 'm':
			/* get MIPS command to send to CMA */
					if (Get_MIPS_ID()) 
					{ /* Slave IOP commands available */
						destination = 0x7D ;
    					printf("\n\r Destination ID set to 0x7D \n\r");
					}
					else
					{ /* master commands available */
						while (1)
						{
							printf("Is the command local? (y/n)\n\r") ;
							key = getchar() ;
							if (key == 'y' || key == 'Y')
							{ /* set destination ID to 0x7D */
								destination = 0x7D ;
		         				printf("\n\r Destination ID set to 0x7D \n\r");
								break ;
							}
							else if (key == 'n' || key == 'N') 
							{ /* set our ID to 0x10 */
								destination = 0x10 ;
 		        				printf("\n\r Destination ID set to 10H \n\r");
								break ;
							}
						}
					}
					CmdPacket.Hdr.bDestAddr = destination ;
				    CmdPacket.Hdr.bSrcAddr = CMB_My_ID ;
				    CmdPacket.Hdr.bStatus = 0x89 ;
					if (destination == 0x7D)
					{
						command = GetLocalCommand() ; /* get local command & set in CMB packet */
					}
					else
					{
						command = GetMasterCommand() ;
					}
					if (command)
					{ /* send command packet & wait for response */
						status = CMB_SendCommand((unsigned char *)&CmdPacket) ;
						if (status != GOOD_STATUS)
						{// command failed
	   		        		printf("\n\r Failed to send command \n\r");
						}
						else
						{
	   		        		printf("\n\r Sending/sent command \n\r");
							printf("\n\r Waiting for CMA to receive/respond \n\r");
//	wait for CMA to send response - MIPS reception will trigger Q1 message
         					status =  NU_Receive_From_Queue(&Queue_1, &Receive_Message, 1, &received_size, NU_SUSPEND);
					        if (status == NU_SUCCESS)
							{
								printf("\n\r CMA response received by MIPS\n\r");
								BuildCMBreceived(&CmdString[0]) ;
								printf(&CmdString[0]) ;
								printf("\n\r Hit any key to continue \n\r") ;
						        key = getchar() ;
							}
						}

					}
                    break;
			case 'C':
			case 'c':
					printf("\n\r Must be simulation mode to use this command \n\r") ;
                    break;
			case 'X':
			case 'x':
			/* exit program */
					more = 0 ;
                    break;
		}
    }  /* while(1) */
}


/* Define the queue receiving task for CMA ISR simulation.
   this task suspends waiting for a message (i.e. simulated interrupt)
*/

unsigned char	GetLocalCommand(void) 
{ /* get local command & set in CMB packet */
    unsigned  i=1 ;
    unsigned  key = 0 ;
    unsigned  char	command, length ;
    unsigned  char	datax ;
	while(i)
	{	i = 0 ;
        printf("\n\r MIPS commands available to IOP \n\r");
        printf("I = Get Board Info \n\r");
        printf("P = Get Boot Params \n\r");
        printf("R = Enable SPI Reset \n\r");
        printf("S = Set MIPS state \n\r");
        printf("T = Get local board temperature \n\r") ;
   		printf("U = Enable Unsolicited\n\r");
   		printf("Enter Q to leave local command loop\n\r");
 		key = getchar() ;
 		switch(key)
 		{
 			case 'I':
 			case 'i':
 					command = 0x08 ; // get board info
 					length = 0 ;
 					break ;
 			case 'P':
 			case 'p':
 					command = 0x0A ; // get boot params
 					length = 0 ;
 					break ;
 			case 'R':
 			case 'r':
 					command = 0x16 ; // enable SPI reset
  					length = 0 ;
  					break ;
			case 'S':
  			case 's':
           			printf("Always send state 9 for now \n\r");
  					command = 0x0B ; // set MIPS state
  					length = 1 ;
  					datax = 9 ; 
  					break ;
  			case 'T':
			case 't':
					command = 0x20 ; // Read parameter
  					length = 2 ;
					CmdPacket.abTail[0] = 3 ; // Temperature
					CmdPacket.abTail[1] = 0 ; // CRC for data
  					break ;
			case 'U':
  			case 'u':
  					command = 0x0C ; // enable unsolicited
  					length = 0 ;
  					break ;

  			case 'Q':
  			case 'q':
  					command = 0 ; // escape loop command
  					break ;

  			default:
  					i = 1 ; //force entry from acceptable set
  					break ;
		}
	}
	CmdPacket.Hdr.bCommand = command ;
	CmdPacket.Hdr.cbData = length ;
	CmdPacket.Hdr.bHeaderCRC = 0 ; // not used
	if (length == 1)
	{
		CmdPacket.abTail[0] = datax ; // only 1 data byte for command
		CmdPacket.abTail[1] = 0 ; // CRC for data
	}
	return (command) ;
}

unsigned char	GetMasterCommand(void) 
{ /* get Master command to send to IOP */
    unsigned  i=1 ;
    unsigned  key = 0 ;
    unsigned  char	command, length ;
	while(i)
	{	i = 0 ;
        printf("\n\r Master MIPS commands available to send to IOPs \n\r");
        printf("D = PCI Disable \n\r");
        printf("E = PCI Enable \n\r");
        printf("F = Turn power off \n\r");
        printf("I = NMI IOP \n\r");
        printf("N = Turn power on \n\r");
        printf("P = Poll for status \n\r");
        printf("R = Reset IOP \n\r");
        printf("T = Get local board temperature \n\r") ;
   		printf("Enter Q to leave local command loop\n\r");
 		key = getchar() ;
 		switch(key)
 		{
 			case 'D':
 			case 'd':
 					command = 0x13 ; // disable PCI
 					length = 0 ;
 					break ;
 			case 'E':
 			case 'e':
 					command = 0x12 ; // enable PCI
 					length = 0 ;
 					break ;
 			case 'F':
 			case 'f':
 					command = 0x11 ; // turn off power to MIPS
 					length = 0 ;
 					break ;
 			case 'I':
 			case 'i':
 					command = 0x15 ; // NMI command
 					length = 0 ;
 					break ;
 			case 'N':
 			case 'n':
 					command = 0x10 ; // turn on MIPS
 					length = 0 ;
 					break ;
 			case 'P':
 			case 'p':
 					command = 0x03 ; // get status
 					length = 0 ;
 					break ;
 			case 'R':
 			case 'r':
 					command = 0x14 ; // reset MIPS
  					length = 0 ;
  					break ;
			case 'T':
			case 't':
					command = 0x20 ; // Read parameter
  					length = 2 ;
					CmdPacket.abTail[0] = 3 ; // Temperature
					CmdPacket.abTail[1] = 0 ; // CRC for data
  					break ;
  			case 'Q':
  			case 'q':
  					command = 0 ; // escape loop command
  					break ;

  			default:
  					i = 1 ; //force entry from acceptable set
  					break ;
		}
	}
	CmdPacket.Hdr.bCommand = command ;	// store command in packet
	CmdPacket.Hdr.cbData = length ;
	CmdPacket.Hdr.bHeaderCRC = 0 ; // not used
	return (command) ;
}

unsigned char	GetResponseToMasterCommand(void) 
{ /* get Master command to send to IOP */
    unsigned  i=1 ;
    unsigned  key = 0 ;
    unsigned  char	command, length ;
    unsigned  char	status, reason ;

	while(i)
	{	i = 0 ;
        printf("\n\r Master MIPS responses available to be received \n\r");
        printf("A = Acknowledge \n\r");
        printf("I = Invalid command \n\r");
        printf("P = Invalid parameters \n\r");
        printf("T = Timeout \n\r");
   		printf("U = Unsolicited command preemption\n\r");
   		printf("Enter Q to leave response loop\n\r");
 		key = getchar() ;
		command = 0x03 ; // assume response selected - all responses are to status command from HBC master MIPS 
 		switch(key)
 		{
 			case 'A':
 			case 'a':
 					status = 0x49 ; // ACK & state running image
 					length = 0 ;
 					break ;
 			case 'I':
 			case 'i':
 					status = 0x09 ; // NACK & state running image
					reason = 1 ; // Invalid command
 					length = 1 ;
 					break ;
 			case 'P':
 			case 'p':
 					status = 0x09 ; // NACK & state running image
					reason = 2 ; // Invalid params sent with command
 					length = 1 ;
 					break ;
 			case 'T':
 			case 't':
 					status = 0x09 ; // NACK & state running image
					reason = 5 ; // Response preempted ??
 					length = 1 ;
 					break ;
			case 'U':
  			case 'u':
 					status = 0x09 ; // NACK & state running image
					reason = 3 ; // Response preempted ??
 					length = 1 ;
  					break ;
  			case 'Q':
  			case 'q':
  					command = 0 ; // escape loop command
  					break ;

  			default:
  					i = 1 ; //force entry from acceptable set
  					break ;
		}
	}
	CmdPacket.Hdr.bCommand = command ; // response is always to status command from MIPS
	CmdPacket.Hdr.cbData = length ;
	CmdPacket.Hdr.bHeaderCRC = 0 ; // not used
	if (length)
	{
		CmdPacket.abTail[0] = reason ;  // only 1 data byte for NAK
		CmdPacket.abTail[1] = 0 ; // CRC for data
	}
	return (command) ;
}

unsigned  char Get_MIPS_ID (void) 
{	
	unsigned  char	key ;
	while (1)
	{
		printf("Are we the master HBC? (y/n)\n\r") ;
		key = getchar() ;
		if (key == 'y' || key == 'Y')
		{ /* set our ID to 0 */
			CMB_My_ID = 0 ;
	   		printf("\n\r MIPS ID set to 0 \n\r");
			break ;
		}
		else if (key == 'n' || key == 'N') 
		{ /* set our ID to 0x10 */
			CMB_My_ID = 0x10 ;
        	printf("\n\r MIPS ID set to 10H \n\r");
			break ;
		}
	}
	return (CMB_My_ID) ;
}
