/*************************************************************************/
/*                                                                       */
/*       Copyright (c) 1993-1996 Accelerated Technology, Inc.            */
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
/*      ps_nuc.c                                       WEBSERV 1.0       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file holds the Nucleus Web Server tasks.  It handles task   */
/*      initialization.  The tasks initialized handle binding a socket,  */
/*      listening to a socket, accepting a connection, and processing a  */
/*      connection to handle http connections with a web browser. Also   */
/*      contains support routines for the web serv product.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*                                                                       */
/*      main_entry_task_ptr         Pointer to TCP client task           */
/*                                  control block and initialize         */
/*                                  the network and the webserver.       */
/*      StrEcho                     Pointer to string echo task          */
/*                                  control block.                       */
/*      socketQueue                 Pointer to socket queue              */
/*                                  control block.                       */
/*                                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PRINTF                      Prints strings to the console if     */
/*                                  supported.                           */
/*      make_worker_process         Creates the pico_worker task.        */
/*      pico_main                   Initializes Nucleus Web Server,      */
/*                                  creates Web server task, opens       */
/*                                  socket connection, Binds socket      */
/*                                  connection, Listens to the socket    */
/*                                  connection, and Accepts sockets      */
/*                                  indefinitely and transmits the       */
/*                                  sockets through a queue to the server*/
/*                                  task.                                */
/*      ps_malloc                   Allocates memory with a call to      */
/*                                  NU_Allocate Memory.                  */
/*      pico_worker                 Receives a connection and processes  */
/*                                  the HTTP request.                    */
/*      Webserv_init                Initialize WebServ.                  */
/*      os_stat_file                OS-dependent check for external      */
/*                                  storage.                             */
/*      os_send_file                Write the os file to socket.         */
/*      os_read_file                Copy the OS file to an array.        */
/*      ps_mfree                    Deallocates memory with a call       */
/*                                  to NU_Deallocate_Memory.             */
/*      ps_net_write                Writes data out to the Network.      */
/*      ps_net_flush                Flushes the output buffer.           */
/*      ps_net_read                 Read data from network once a        */
/*                                  connection has been made.            */
/*      ps_gettime                  Get time and convert it to seconds.  */
/*      os_write_fs                 Write to the os dependent mass       */
/*                                  storage medium.                      */
/*      os_file_name                Places the filename into correct     */
/*                                  format.                              */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     ps_pico.h                           Defines and Data Structures   */
/*                                         related to Nucleus Webserv.   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      Don Sharer       06/26/98           Modified WebServer to handle */
/*                                          multiple Interfaces.         */
/*                                                                       */
/*                                                                       */
/*************************************************************************/




#ifdef _MNT
#include <windows.h> 
#include <WINBASE.H>
#include "hardware.h"
#endif

#include "ps_pico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef MEMORY_DEBUG
#include "memdebug.h"
#endif

#include "nucleus.h"
#include "externs.h"
#include "socketd.h"                                /* socket interface structures */
#include "tcpdefs.h"
#include "data.h"
#include "falfl.h"

#define	NUC_FS_LEN		100
#define NUC_FBUF_SZ		512
#ifdef _MNT

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_                                /* _WINSOCKAPI_ must be defined for _MNT */
#endif                   
			
#endif

/*
 *
 *      IMPORTANT SYSTEM PARAMETERS 
 */

#define QSIZE 128
#define STACK_SIZE 12000
#define NUM_WORKS  5        /*  Inidicates the number of */
						    /*  worker tasks started.    */ 


void   pico_main(UNSIGNED  argc, VOID  *argv);
void   pico_worker(UNSIGNED argc, VOID  *argv);
void   ps_net_flush(Request * req);
void   make_worker_process(int index);



Ps_server master_server;                            /* declare space for the master server structure */

#define Exit(X)  NU_Terminate_Task(NU_Current_Task_Pointer());

/* Define Application data structures.  */

extern NU_MEMORY_POOL   System_Memory;
NU_TASK                 PicoServer;
NU_QUEUE                socketQueuea;
NU_TASK                 servers[NUM_WORKS];


unsigned long Total_Bytes_Received;
unsigned long Total_Bytes_Sent;
unsigned long Total_Packets_Received;
int created=0;
int suspended=0;    
int terminated=0;
int graphics_val= 0;

int count=0;

extern void pico_init(void);
VOID Webserv_init(VOID);

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      Webserv_init                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that allows easy interface to hook in the Nucleus       */
/*      Webserv product. This function initializes the web server tasks  */
/*      and the queue.                                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      main_entry_task             Initializes network.                 */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory          Allocate memory from memory pool     */
/*      Exit                        Exit on error.                       */
/*      NU_Create_Task              Create an application task.          */
/*      NU_Create_Queue             Create a message Queue               */
/*      printf                      Prints a string to the screen if     */
/*                                  applicable.                          */        
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

VOID Webserv_init()
{
    VOID           *pointer;
    STATUS         status;

    /* Create the Application tasks. */
    
    status = NU_Allocate_Memory(&System_Memory, &pointer, STACK_SIZE, 
				NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {

#ifdef DEBUG
	printf ("Can not create memory for pico_main.\n");
#endif
	Exit(-1);
    }

	/* this creates the main nucleus server process */ 
    /*  Create Nucleus Web Server Task  */
    status = NU_Create_Task(&PicoServer, "NUWebSrv",pico_main, 0, 
			    NU_NULL, pointer, STACK_SIZE, 3, 1000,
			    NU_PREEMPT, NU_START);

    if (status != NU_SUCCESS)
    {
#ifdef DEBUG
	printf ("Cannot create pico_main\n");
#endif
	Exit (2);
    }
    
    status = NU_Allocate_Memory(&System_Memory, &pointer, 
			QSIZE*sizeof(UNSIGNED), NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
    {
#ifdef DEBUG
	printf ("Can not create memory for SocQueue.\n");
#endif 
	Exit (-1);
    }

    status = NU_Create_Queue(&socketQueuea, "SocQa", pointer, 
			QSIZE, NU_FIXED_SIZE, 1, NU_FIFO);

    if (status != NU_SUCCESS)
    {
#ifdef DEBUG
	printf ("Can not create Socket Queue.\n");
#endif
	Exit (-1);
    }


}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      pico_main                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The main pico server function.  It calls make_worker process to  */
/*      create the recieve task based on the number of workers defined.  */
/*      It uses a queue to transmit the the connected socket once the    */
/*      it has been accepted.                                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      pico_init                       Initializes Nucleus Web Server.  */
/*      make_worker_process             Creates the pico_worker task.    */
/*      memcpy                          Copies one location in memory    */
/*                                      to another with a specified      */
/*                                      length.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void pico_main(UNSIGNED  argc, VOID * argv)
{
    int32               socketd, newsock;           /* the socket descriptor */
    struct addr_struct  servaddr;                   /* holds the server addre struct*/
    struct addr_struct  client_addr;
    STATUS              status=0;
    int16               i;

    Ps_server           *this_server;
   
    /*  Remove warnings for unused parameters.  */
    status = (STATUS) argc + (STATUS) argv + status;

#ifndef FS_IN_MEMORY
/*  Register Task as File User */
#ifdef NUCLEUS_FILE_INCLUDED
    /*  Each task must register task as a file user */
    if (!NU_Become_File_User())
         return;

    /*  Prepare the disk for use by this task.  */
    if(!NU_Open_Disk(__RAM))
        return;

    /*  Set the default drive   */
    if(!pc_set_default_drive(__RAM))
       return;

#endif
#endif
    /* Intialize global counters.  */
    Total_Bytes_Received = 0;
    Total_Packets_Received = 0;
    this_server = &master_server;

    /*  Print banner.  */
#ifdef  DEBUG
    printf("Nucleus Web Server\nHTTP 1.0 Server Version 4.00\n\n");
#endif /* Debug */
	
	pico_init();



	/* Create the worker process
	 * The worker process is an endless loop
	 * that handels HTTP (WWW server) requests.
	 * 
	 * The requests are taken off the QUEUE of
	 * socket descripters
	 */
     for (i= 0 ;i< NUM_WORKS; i++)
     {
	 make_worker_process(i);
     }




	/* this process hangs in an endless loop
	 * doing socket accept's. Each accept yields
	 * a socket descripter which is put on the Queue
	 */

    /* open a connection via the socket interface */
    if ((socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0)) >=0 )
    {

	/* fill in a structure with the server address */
	servaddr.family    = NU_FAMILY_IP;
	servaddr.port      = this_server->port;
    /*  Set the ip address to IP addr ANY  */
    memset(servaddr.id.is_ip_addrs, 0, 4);


	/* make an NU_Bind() call to bind the server's address */
	if ((NU_Bind((int16)socketd, &servaddr, 0))>=0)
	{
	    /* be ready to accept connection requests */
	    status = NU_Listen((int16)socketd, 10);

	    if (status == NU_SUCCESS)
	    {

		for (;;)
		{
		    /* block in NU_Accept until a client connects */

		    newsock = NU_Accept((int16)socketd, &client_addr, 0);
		    if (newsock >= 0)
		    {
#ifdef DEBUG
					    printf("accept\n");
#endif
			/* process the new connection */

			status = NU_Send_To_Queue(&socketQueuea,
					     (UNSIGNED *)&newsock, 1, NU_SUSPEND);
						    

		    }                               /* end successful NU_Accept */
		}                                   /* end for loop */


	    }                                       /* end successful NU_Listen */
	}                                           /* end successful NU_Bind */

    }                                               /* end successful NU_Socket */

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      make_worker_process                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates the pico_worker task.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      pico_main                   Initializes Nucleus Web Server,      */
/*                                  creates Web server task, opens       */
/*                                  socket connection, Binds socket      */
/*                                  connection, Listens to the socket    */
/*                                  connection, and Accepts sockets      */
/*                                  indefinitely and transmits the       */
/*                                  sockets through a queue to the server*/
/*                                  task.                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory          Allocate memory from memory pool     */
/*      Exit                        Exit on error.                       */
/*      NU_Create_Task              Create an application task.          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      index                       Number of server entry.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void make_worker_process(int index)
{
    VOID        *pointer;
	STATUS  status;


	status = NU_Allocate_Memory(&System_Memory, &pointer,STACK_SIZE, 
							     NU_NO_SUSPEND);

    if (status != NU_SUCCESS) 
    {
#ifdef DEBUG
	printf ("no mem for server req\n");
#endif
	Exit (-1);
	}

    /*    Allows for multiple server entries.  The value should be modified if 
	  more than one server is required(inside of pico_main).       */
	status = NU_Create_Task(&servers[index], "server", pico_worker, 
				    0, NU_NULL, pointer,STACK_SIZE, 3, 1000, 
			    NU_PREEMPT, NU_START);

	if (status != NU_SUCCESS) 
    {
#ifdef  DEBUG
	printf ("Cannot create server_request = %d\n",status);
#endif
	Exit (2);
    }

}



int SC=0;
int SE =0;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      pico_worker                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The function is executed and is suspended on a Queue waiting for */
/*      a connection to occur with the server.  The queue transmits the  */
/*      socket descriptor.  Then the function services the request on    */
/*      the socket descriptor.  It then closes the connection when       */
/*      processing is complete.  Finally, it is suspended again waiting  */
/*      for another socket descriptor to be sent through the queue.      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ps_malloc                       Allocates memory with a call to  */
/*                                      NU_Allocate Memory.              */
/*      NU_Receive_From_Queue           Retrieves a message from the     */
/*                                      specified queue.                 */
/*      NU_Recv                         Receive data across a network    */
/*                                      during  a connection-oriented    */
/*                                      transfer.                        */
/*      NU_Sleep                        Sleeps for specified number of   */
/*                                      timer ticks.                     */
/*      printf                          Prints a string to the screen if */
/*                                      applicable.                      */
/*      in_string                       Compares string to actual buffer.*/                                      
/*      NU_GET_Peer_Name                Gets Clients IP address.         */       
/*      parse_http_request              Parses HTTP request and calls    */
/*                                      necessary routines required for  */
/*                                      processing the parsed request.   */
/*      ps_net_flush                    Flushes the output buffer.       */
/*      ps_mfree                        Deallocates memory with a call   */                                 
/*                                      to NU_Deallocate_Memory.         */
/*      Exit                            Exit on error.                   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void pico_worker(UNSIGNED argc, VOID *argv)
{
    int16                   bytes_recieved;
    int16                   connection_bytes;
    int32                   sockfd;
    STATUS                  status=0;
    uint32                  total_pkts = 0;
    Request                 Req;
    FD_SET                  readfs, writefs, exceptfs;
    struct sockaddr_struct  client_addr;
    int16                   i;
    short                   size;
    char *                  line;
    char *                  out_head;
    extern Token            * free_tokens;
    struct  req_data        *       reqdat;
    UNSIGNED                actSize;
    int16                   index=0;
    int16                   connected=0;

    /*  Remove warnings for unused parameters.  */
    status = (STATUS) argc + (STATUS) argv + status;

    reqdat = (struct req_data *) ps_malloc(sizeof( struct req_data));
    line = &reqdat->lbuf[0];
    free_tokens = (Token *) &reqdat->heap;
    out_head = &reqdat->out_head[0];

/*  Register Task as File User */
#ifndef FS_IN_MEMORY
#ifdef NUCLEUS_FILE_INCLUDED
    /*  Each task must register task as a file user */
    if (!NU_Become_File_User())
         return;

    /*  Prepare the disk for use by this task.  */
    if(!NU_Open_Disk(__RAM))
        return;

    /*  Set the default drive   */
    if(!pc_set_default_drive(__RAM))
       return;

#endif
#endif

    while ( 1 )                                     /* endless loop  processing HTTP requests for all of time */
    {   


		status = NU_Receive_From_Queue(&socketQueuea, (UNSIGNED *)
						   &sockfd, 1, &actSize, NU_SUSPEND);


		if (status != NU_SUCCESS)
		{
#ifdef DEBUG
		    printf ("no mesg from socket Q");
#endif
            continue;

		}

	
		if (sockfd <= 0)
			continue;
        connected = 1;
		connection_bytes=0;
        index= 0;
        while(portlist[index] !=NU_NULL)
        {
            if (sockfd == portlist[index]->p_socketd)
            {
                break;
            }
            index++;
       }
      /*  Get the IP address for this connection */
       for(i=0; i<IPADDR; i++)
          master_server.ip[i] = portlist[index]->tcp_laddr[i];
                

	   while(connected)
       {    
	        /*  Use NU_Select to try timeout of socket  */
	        NU_FD_Init(&readfs);
	        NU_FD_Set((int16)sockfd, &readfs);
	        if((status = NU_Select(NSOCKETS, &readfs, &writefs, &exceptfs, HTTP_TIMEOUT)) == NU_SUCCESS)
	        {
		   
		        i= NU_Recv((int16)sockfd, line + connection_bytes, RECEIVE_SIZE, NU_NO_SUSPEND);                      
              
		        bytes_recieved = i;        
		        Total_Bytes_Received += bytes_recieved;                       
		        if (bytes_recieved == NU_NOT_CONNECTED)
                {
                        connected=0;
                        break;
                }
		        else if (bytes_recieved < 0) 
		        {
#ifdef DEBUG
			        printf("\n NU_Recv error\n");
#endif
                    connected=0;
			        break;
		        }
	
		        connection_bytes += bytes_recieved;
		        total_pkts++;
		        line[connection_bytes]='\0';
			
		        if( in_string("\r\n\r\n",line) == FAILURE ) 
                         continue;
		
		        size= sizeof (struct sockaddr_struct);
		        /*Get the clients IP address */


		        if(( i=  NU_Get_Peer_Name((int16)sockfd, &client_addr,&size) ) != NU_SUCCESS)
		        {
#ifdef DEBUG
				    printf("BAD Peer NAME\n");
#else
			        ;
#endif /* Debug */

		        }
		        else 
		        {
			        for( i=0; i<IPADDR; i++)
				        Req.ip[i]= *(((char * )&client_addr)+i);
		        }

		        /* Setup state for server */
		        Req.pg_args = (Token * ) 0;
		        Req.server= &master_server;             /* per server info */
		        Req.sd = (int16)sockfd;                            /* socket descripter */
		        out_head[0]=0;                              
		        Req.response = out_head;                /* storage/output header */
		        Req.obufcnt=0;
		        Req.rdata = reqdat;                     /* per request allocated data */
		        Req.nbytes = connection_bytes;

		        /* call the server for every HTTP request */

		        parse_http_request(&Req, line);             
		    
		        ps_net_flush(&Req);                     /* flush the output buffer */
                break;

		    }   /*  If Statement  */                       
            else
            {
                /*  Select Failed Exit out of While loop and wait for the next data socket */
                connected=0;
            }
       }    /*  while(connected) */
	   SC++;
	   if ((NU_Close_Socket((int16)sockfd)) != NU_SUCCESS)
       {
#ifdef DEBUG
			printf("\nError from NU_Close.");
#endif
	    	SE++;
			SC--;
		}   
	
#ifdef DEBUG
		printf("REQUEST DONE\n");
#endif
	   
    }    /* (END while ( 1 ) loop */

}



/***********************************************************************************/
/*                                                      OS DEPENDENT ROUTINES 
 ************************************************************************************/

/* THESE ROUTINES 
 * SUPPORT FILE I/
 * WHEN THERE IS MASS
 * STORAGE HARDWARE 
 */

/* fill in the stat structure
 * in the request structure  
 */

 
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      os_stat_file                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Checks if the file to be saved is on external storage and fills  */
/*      out the request stat structure on file size.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*     Don Sharer Accelerated Technology Inc.                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      md_file_stat                Checks to see if the file is         */
/*                                  in memory or os-dependent file       */
/*                                  system.                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      os_file_name                Puts the filename in the correct     */
/*                                  format.                              */
/*      FAL_findfirst               Returns statistics about a file of a */
/*                                  particular sequence.                 */
/*      FAL_findclose               Frees all internal data structures   */
/*                                  used with the FAL_findfirst command. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Return SUCCESS if Successful.                                    */
/*      Returns Failure if unsuccessful.                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int os_stat_file(Request * req)
{
#ifndef FS_IN_MEMORY
    FAL_DIR  statobj;
    char buf[NUC_FS_LEN];
    int attrib=0;

    os_file_name(req,buf);

    if (FAL_findfirst((uint8 *)buf,&statobj,attrib)== RESULT)
    {
        req->stat->size=statobj.fsize;
        req->stat->flags= FOUND;
        FAL_findclose(0,&statobj);
        return(SUCCESS);
    }
    else
        return(FAILURE);
#else
    req=req;
    return(FAILURE);
#endif
	
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      os_file_name                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Places the file name in the correct format so it can be read, or */
/*      written with an external file system.                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*     Don Sharer Accelerated Technology Inc.                            */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      md_file_stat                Checks to see if the file is         */
/*                                  in memory or os-dependent file       */
/*                                  system.                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      strcpy                      Function to copy one string to       */
/*                                  another.                             */
/*      strcat                      Function to contanate a string to    */
/*                                  another.                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*      buf                         Pointer to the filename that will    */
/*                                  contain the proper command.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  os_file_name(Request * req, char * buf )
{
	char * n;

	strcpy(buf,OS_FS_PREFIX);		/* base of server root */
	n = req->fname;

	while (*n == '/')
		n++;
	
	strcat(buf,n);	
	
#ifdef DEBUG
	printf("NAME = %s\n",buf);
#endif

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      os_send_file                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to write the os file to socket.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*     Don Sharer Accelerated Technology Inc.                            */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      serve_file                  This function sends the file to the  */
/*                                  client over the network.             */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      os_file_name                Puts the filename in the correct     */
/*                                  format.                              */
/*      FAL_fopen                   Opens the filename to be written.    */
/*      FAL_fread                   Reads bytes from the file.           */
/*      ps_net_write                Writes bytes out to the network.     */
/*      FAL_fclose                  Closes the file descriptor.          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Return SUCCESS if Successful.                                    */
/*      Returns Failure if unsuccessful.                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int os_send_file(Request * req)
{
#ifndef FS_IN_MEMORY
    char buf[NUC_FS_LEN];
    char fbuf[NUC_FBUF_SZ];
    FAL_FILE fd=FILE_CHECK;
    int j,mode;


    os_file_name(req,buf);

#ifdef NUCLEUS_FILE_INCLUDED
    mode = PS_IREAD;
#endif
    if( (fd = FAL_fopen((uint8 *)buf, PO_RDONLY,(UCOUNT)mode,(FAL_FILE)fd)) != (FAL_FILE )FILE_CHECK)
    {

        while( (j=FAL_fread( (uint8 *)fbuf,1,NUC_FBUF_SZ,fd)) > 0)
                ps_net_write(req,fbuf,j);
    

    }
    else
    {
#ifdef DEBUG
        printf("can't open %s \n",buf);
#endif
     return FAILURE;
    }    

    FAL_fclose(fd,0);
	return SUCCESS;
#else
    req=req;
    return FAILURE;
#endif

}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      os_read_file                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Copy the os file to an array.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*     Don Sharer Accelerated Technology Inc.                            */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      process_ssi                 Process a Get Operation with the .SSI*/
/*                                  extension.                           */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      os_file_name                Puts the filename in the correct     */
/*                                  format.                              */
/*      FAL_fopen                   Opens the filename to be written.    */
/*      FAL_fread                   Reads bytes from the file.           */
/*      FAL_fclose                  Closes the file descriptor.          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*      buffer                      Pointer to the buffer that bytes read*/
/*                                  from the file is to be placed.       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Return SUCCESS if Successful.                                    */
/*      Returns Failure if unsuccessful.                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int os_read_file(Request * req, char * buffer )
{
        /*  Remove Warnings */
#ifndef FS_IN_MEMORY        
    char buf[NUC_FS_LEN];
    FAL_FILE fd=FILE_CHECK;
    int j,mode=0;

    os_file_name(req,buf);

#ifdef NUCLEUS_FILE_INCLUDED
    mode = PS_IREAD;
#endif
    if( (fd = FAL_fopen((uint8 *)buf, PO_RDONLY,(UCOUNT)mode,(FAL_FILE)fd)) != (FAL_FILE )FILE_CHECK)
    {

        j=FAL_fread( (uint8 *)buffer,1,req->stat->size,fd);
        
        if (j != (int)req->stat->size)
        {
            FAL_fclose(fd,0);
            return(FAILURE);
        }
        else
        {
            FAL_fclose(fd,0);
            return(SUCCESS);
        }

    }
    else
    {
#ifdef DEBUG
        printf("can't open %s \n",buf);
#endif
     return FAILURE;
    }    
#else
    req=req;
    buffer= buffer;
    return FAILURE;
#endif
    

}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      os_write_fs                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that used for hook to implement support for a hard disk */
/*      or other(non memory-base FS) mass storage.                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     Don Sharer Accelerated Technology Inc.                            */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      save_os_fs                  Saves a file in an os supported file */
/*                                  system.                              */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      printf                      Prints strings to the console if     */
/*                                  supported.                           */
/*      os_file_name                Puts the filename in the correct     */
/*                                  format.                              */
/*      FAL_fopen                   Opens the filename to be written.    */
/*      FAL_fwrite                  Writes bytes to a designated file.   */
/*      FAL_fclose                  Closes the file descriptor.          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                         Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*      fname                       File name to be written under.       */
/*      filemem                     The pointer to the file in memory.   */
/*      length                      The length of the file.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Return SUCCESS if Successful.                                    */
/*      Returns Failure if unsuccessful.                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int os_write_fs(Request * req, char * fname, char * filemem, int length)
{
#ifndef FS_IN_MEMORY 
    FAL_FILE    fd=FILE_CHECK;
    int         mode=0;


#ifdef NUCLEUS_FILE_INCLUDED
       mode = PS_IWRITE;
#endif
     
    if( (fd = FAL_fopen((uint8 *)fname, PO_WRONLY|PO_CREAT,(UCOUNT)mode,(FAL_FILE)fd)) != (FAL_FILE )FILE_CHECK)
    {

        if( FAL_fwrite((uint8 *)filemem, 1, length, fd) != (uint32)length )
		{
             FAL_fclose(fd,0);
             fd = (FAL_FILE)FILE_CHECK;
			return FAILURE;			
		}

        FAL_fclose(fd,0);
        return(SUCCESS);
    }    
    else
    {
#ifdef DEBUG
        printf("can't create %s \n",fname);
#endif
     return FAILURE;
    }    
#else
    req=req;
    fname=fname;
    filemem=filemem;
    length=length;
    return FAILURE;
#endif
}


/* PS_API machdep */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_malloc                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to Allocate memory based on a finite size.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      memory_dump                 Plugin used to dump a specified      */
/*                                  memory address and length to an      */
/*                                  HTML page.                           */
/*      save_mem_fs                 Saves a file that has been uploaded  */
/*                                  into the in memory file system.      */
/*      save_os_fs                  Saves a file in an os supported file */
/*                                  system.                              */
/*      file_upload                 Plugin that uploads a file into the  */
/*                                  in-memory or os supported file       */
/*                                  system.                              */
/*      process_ssi                 Process a Get Operation with the .SSI*/
/*                                  extension.                           */
/*      pico_worker                 Receives a connection and processes  */
/*                                  the HTTP request.                    */ 
/*      desinit                     Initializes the DES encryption\      */
/*                                  decryption.                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory          Allocate memory from memory pool     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      size                        Size of memory to be allocated.      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Returns failure if it could not allocate the memory.             */
/*      If it is succesful it returns back the pointer to the allocated  */
/*      memory.                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  * ps_malloc(unsigned int size)
{
	int status;
	void * ppage;

	status = NU_Allocate_Memory(&System_Memory, &ppage, size, NU_NO_SUSPEND);
    if (status != NU_SUCCESS) 
    {

	return( (char*) FAILURE);
    }

	else    return(ppage);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_mfree                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function used to deallocate memory.                              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      memory_dump                 Plugin used to dump a specified      */
/*                                  memory address and length to an      */
/*                                  HTML page.                           */
/*      save_mem_fs                 Saves a file that has been uploaded  */
/*                                  into the in memory file system.      */
/*      save_os_fs                  Saves a file in an os supported file */
/*                                  system.                              */
/*      process_ssi                 Process a Get Operation with the .SSI*/
/*                                  extension.                           */
/*      pico_worker                 Receives a connection and processes  */
/*                                  the HTTP request.                    */ 
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Deallocate_Memory        Deallocates memory from the memory   */
/*                                  pool.                                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      addr                        The pointer to where the memory      */
/*                                  should be deallocated.               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void ps_mfree(void* addr)
{

	NU_Deallocate_Memory((VOID **)addr);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_net_write                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to output data to the network.                          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_sendurl                      Validates User ID and key and    */
/*                                      calls appropriate URI.           */
/*      send_auth_salt                  Sends random number to applet.   */
/*      buf_putc                        Puts a character in a buffer.    */
/*      list_directory                  Plugin to list all files in the  */
/*                                      in memory or os dependent file   */
/*                                      system.                          */
/*      process_GET                     Processes an HTTP response by    */
/*                                      using the GET method.            */
/*      process_POST                    Processes Post form commands.    */
/*      process_ssi                     Process a Get Operation with the */
/*                                      .SSI extension.                  */
/*      send_client_html_msg            Sends HTML error message to      */
/*                                      client.                          */
/*      ps_proto_status                 Sets up the HTTP response.       */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*      ip_addr                         Plugin to transmit IP address.   */
/*      server_stat                     Plugin to transmit server stats. */
/*      task_stat                       Plugin to give task statistics.  */
/*      ran_num                         Plugin to give random number.    */
/*      memory_dump                     Plugin used to dump a specified  */
/*                                      memory address and length to an  */
/*                                      HTML page.                       */
/*      arg                             Plugin to process an incoming    */
/*                                      argument.                        */
/*      form_echo                       Plugin to process a form         */
/*                                      operation and transmits the      */
/*                                      name and value back.             */
/*      process_answer                  Plugin to process a POST form and*/
/*                                      retruns a response based on the  */
/*                                      input.                           */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send                         Transmits data across a network. */
/*      memcpy                          Copies one location in memory    */
/*                                      to another with a specified      */
/*                                      length.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*       req                            Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to the HTTP request.             */
/*       buf                            The buffer that holds the        */
/*                                      information to be written out on */
/*                                      the network.                     */
/*       sz                             The size of the buffer that is to*/
/*                                      be written out.                  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*       None.                                                           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void ps_net_write(Request * req, char * buf, int sz)
{
	

	Total_Bytes_Sent+=sz;

	while( (sz + req->obufcnt) > OBUFSZ-1 ) 
    {
		if( req->obufcnt ) 
        {
            memcpy(&req->rdata->obuf[req->obufcnt], buf,
				   (OBUFSZ - req->obufcnt));
			NU_Send((short)req->sd,req->rdata->obuf,OBUFSZ,0);
			sz -= (OBUFSZ - req->obufcnt);
			buf += (OBUFSZ - req->obufcnt);
			req->obufcnt=0;
		}
        else
        {
            NU_Send((short)req->sd,buf,(uint16)sz,(uint16)0);
			sz =0;
		}
	}


	if( sz ) 
    {
		memcpy(&req->rdata->obuf[req->obufcnt],buf,sz);
		req->obufcnt += sz;
	}

	return;
}




/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_net_flush                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that handles buffered output flush.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      pico_worker                 Receives a connection and processes  */
/*                                  the HTTP request.                    */ 
/*      ps_proto_finish             Completes HTTP prototyping.          */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send                     Transmit data across a network       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*       req                        Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void ps_net_flush(Request * req)
{
    if( req->obufcnt) 
    {
		NU_Send((short)req->sd,req->rdata->obuf,req->obufcnt,0);
		req->obufcnt=0;
	}
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_net_read                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to read data after a connection has been made from the  */
/*      network.                                                         */                                                                     
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      file_upload                 Plugin that uploads a file into the  */
/*                                  in-memory or os supported file       */
/*                                  system.                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Recv                     Receive data across a network during */
/*                                  a connection-oriented transfer.      */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*       req                        Pointer to Request structure that    */
/*                                  holds all information pertaining to  */
/*                                  the HTTP request.                    */
/*       buf                        The buffer that holds the information*/
/*                                  to be written out on the network.    */
/*       sz                         The size of the buffer that is to be */
/*                                  written out.                         */
/*       timeout                    A finite amount of time that the     */
/*                                  request should timeout during a      */
/*                                  NU_recv call.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      bytes_received              The total bytes received from an     */
/*                                  NU_recv call.                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int ps_net_read(Request * req, char * buf, int sz, int time_out)
{
	int bytes_recieved;
        /*  Remove Warning */
        time_out = time_out;
       /* turn on the "block during a read" flag */
       NU_Fcntl((short)req->sd, NU_SETFLAG, NU_BLOCK);
        
        bytes_recieved = NU_Recv((short)req->sd, buf, (uint16)sz, 0);
       NU_Fcntl((short)req->sd, NU_SETFLAG, NU_FALSE);
	return(bytes_recieved);
}

#define CLOCK_HZ 30


/* used for authentiaction timeout  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_gettime                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to get the time and converts it to seconds.             */          
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      check_timeout                       Checks the timeout.          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TMT_Retrieve_Clock                  Retrieve system clock        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      t                                   Time in seconds              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int ps_gettime(void)
{
	unsigned int t;

	t = TMT_Retrieve_Clock();
	t = t/ CLOCK_HZ;                                    /* convert to seconds */
	return((int)t);
}







