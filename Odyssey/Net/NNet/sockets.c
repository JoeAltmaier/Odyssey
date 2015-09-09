/****************************************************************************/
/*                                                                          */
/*       CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.            */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  sockets.c                                                 4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file defines our interface to the functions using the Berkely        */
/*  socket standard TCP/IP interface.                                         */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*  Craig L. Meredith, Accelerated Technology Inc.                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*  global compenent data stuctures defined in this file                      */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*     NU_Abort                                                               */
/*     NU_Accept                                                              */
/*     NU_Bind                                                                */
/*     NU_Close_Socket                                                        */
/*     NU_Connect                                                             */
/*     NU_Fcntl                                                               */
/*     NU_Get_Host_by_Addr                                                    */
/*     NU_Get_Host_by_Name                                                    */
/*     NU_Get_Peer_Name                                                       */
/*     NU_Get_UDP_Pnum                                                        */
/*     NU_GetPnum                                                             */
/*     NU_Getsockopt                                                          */
/*     NU_Init_Net                                                            */
/*     NU_Ioctl                                                               */
/*     NU_Is_Connected                                                        */
/*     NU_Listen                                                              */
/*     NU_Push                                                                */
/*     NU_Recv                                                                */
/*     NU_Recv_From                                                           */
/*     NU_SearchTaskList                                                      */
/*     NU_Send                                                                */
/*     NU_Send_To                                                             */
/*     NU_Setsockopt                                                          */
/*     NU_Socket                                                              */
/*     NU_Socket_Connected                                                    */
/*     NU_TaskTable_Entry_Delete                                              */
/*     NU_TaskTableAdd                                                        */
/*     NU_UpdateTaskTable                                                     */
/*     NU_Add_Route                                                           */
/*     SCK_Create_Socket                                                      */
/*     SCK_Suspend_Task                                                       */
/*     timer_task                                                             */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*  No other file dependencies                                                */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME               DATE        REMARKS                                  */
/*                                                                            */
/*  Barbara G. Harwell   10/06/92   Initial version.                          */
/*  Craig L. Meredith    02/15/94   Clean up and changed code for             */
/*                                  the interrupt version.                    */
/*  Glen Johnson         04/30/96   Made some changes based on recommedations */
/*                                  of K. Goto of Hitachi.                    */
/*  Maiqi Qianon         02/26/97   Added new function NU_Abort               */
/*                                                                            */
/******************************************************************************/
#ifdef PLUS
  #include "nucleus.h"
#else /* PLUS */
  #include "nu_defs.h"
  #include "nu_extr.h"
#endif /* !PLUS */
#include "target.h"
#include "ip.h"
#include "externs.h"
#include "data.h"
#include "sockext.h"
#include "netevent.h"
#include "socketd.h"
#include "protocol.h"
#include "dns.h"

#if SNMP_INCLUDED
#include "snmp_g.h"
#endif

/* the following includes are for Nucleus references */
#include "tcpdefs.h"
#include "tcp_errs.h"
#include "netevent.h"

#ifdef PLUS
  /* Global Declarations */
  NU_QUEUE          eQueue;
  NU_SEMAPHORE      TCP_Resource;
  NU_TASK           NU_EventsDispatcher_ptr;
  NU_TASK           timer_task_ptr;
#if (INCLUDE_RIP2)
  NU_TASK           rip2_task_ptr;
#endif
  NU_EVENT_GROUP    Buffers_Available;

  VOID    NU_EventsDispatcher(UNSIGNED argc, VOID *argv);
  VOID    timer_task (UNSIGNED argc, VOID *argv);
  VOID    rip2_task (UNSIGNED argc, VOID *argv);
#endif

/*  The following value is used to remove warnings from the compiler
    for unused parameters.  */
uint32 unused_parameter;

/*  next_socket_no is used to record the last position searched in the
    socketlist.  The next time the socket list is serached an unused socket
    should be found immediately.  The alternative is to begin searchin from the
    start of the list everytime.  Chances are a lot of used sockets would be
    found before finding an unused one.
*/
sint next_socket_no;


/******************************************************************************/
/*                                                                            */
/*  FUNCTION                                "NU_Init_Net"                     */
/*                                                                            */
/*                                                                            */
/*  DESCRIPTION                                                               */
/*                                                                            */
/*      This function initializes the network for Nucleus NET TCP/IP          */
/*      operations.  It should only be called once.                           */
/*                                                                            */
/*  AUTHOR                                                                    */
/*                                                                            */
/*      Neil Henderson              Accelerated Technology, Inc.              */
/*                                                                            */
/*  CALLED FROM                                                               */
/*                                                                            */
/*      User Applications                                                     */
/*                                                                            */
/*  ROUTINES CALLED                                                           */
/*                                                                            */
/*      NU_Allocate_Memory                                                    */
/*      NU_Create_Event_Group                                                 */
/*      netinit                                                               */
/*      NU_Create_Queue                                                       */
/*      NU_Create_Semaphore                                                   */
/*      NU_Create_Task                                                        */
/*      UTL_Zero                                                              */
/*                                                                            */
/*  INPUTS                                                                    */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/*  OUTPUTS                                                                   */
/*                                                                            */
/*      status                      status of the initialization              */
/*                                                                            */
/*         NAME            DATE       REMARKS                                 */
/*                                                                            */
/*     G. Johnson        05/31/96     Moved creation of NET tasks to after    */
/*                                    creation of resources (semaphores etc.) */
/*                                                                            */
/******************************************************************************/
STATUS NU_Init_Net(VOID)
{
    sint     status;
#ifdef PLUS
    VOID     *pointer;
#endif

    /* Initialize the socket_list pointer-arry. MQ  12/13/96. */
    UTL_Zero((CHAR *)socket_list, sizeof(socket_list));

    /* Initialize next_socket_no */
    next_socket_no = 0;

#ifdef PLUS
    /* Create Event queue.  */
    status = NU_Allocate_Memory(&System_Memory, &pointer,
                            (UNSIGNED)(2400*sizeof(UNSIGNED)),
                            (UNSIGNED)NU_NO_SUSPEND);

    if (status != NU_SUCCESS)
        return(NU_MEM_ALLOC);

    pointer = normalize_ptr(pointer);

    status = NU_Create_Queue(&eQueue, "EvtQueue", pointer, (UNSIGNED)800,
                            NU_FIXED_SIZE, (UNSIGNED)3, NU_FIFO);
    if (status != NU_SUCCESS)
        return(NU_INVAL);

    /* Create synchronization semaphore.  */
    status = NU_Create_Semaphore(&TCP_Resource, "TCP", (UNSIGNED)1, NU_FIFO);
    if (status != NU_SUCCESS)
        return(NU_INVAL);

    status = NU_Create_Event_Group(&Buffers_Available, "BUFAVA");
    if (status != NU_SUCCESS)
        return(NU_INVAL);

    /* Create Event Queue Dispatcher task.  */
    status = NU_Allocate_Memory(&System_Memory, &pointer, (UNSIGNED)5000,
                                (UNSIGNED)NU_NO_SUSPEND);
    if (status != NU_SUCCESS)
        return(NU_MEM_ALLOC);

    pointer = normalize_ptr(pointer);

    status = NU_Create_Task(&NU_EventsDispatcher_ptr, "EvntDisp",
                        NU_EventsDispatcher, (UNSIGNED)0, NU_NULL, pointer,
                        (UNSIGNED)5000, 3, (UNSIGNED)0, NU_PREEMPT, NU_START);
    if (status != NU_SUCCESS)
        return(NU_INVAL);


    /* Create timer task */
    status = NU_Allocate_Memory(&System_Memory, &pointer, (UNSIGNED)5000,
                               (UNSIGNED)NU_NO_SUSPEND);
   if (status != NU_SUCCESS)
        return(NU_MEM_ALLOC);

    pointer = normalize_ptr(pointer);

   status = NU_Create_Task(&timer_task_ptr, "TIMER", timer_task, (UNSIGNED)0,
                            NU_NULL, pointer, (UNSIGNED)5000, 3, (UNSIGNED)0,
                            NU_PREEMPT, NU_START);
   if (status != NU_SUCCESS)
        return(NU_INVAL);

#if (INCLUDE_RIP2)
    /* Create RIP2 task */
    status = NU_Allocate_Memory(&System_Memory, &pointer, (UNSIGNED)3000,
                               (UNSIGNED)NU_NO_SUSPEND);
   if (status != NU_SUCCESS)
        return(NU_MEM_ALLOC);

    pointer = normalize_ptr(pointer);

   status = NU_Create_Task(&rip2_task_ptr, "RIP2TASK", rip2_task, (UNSIGNED)0,
                            NU_NULL, pointer, (UNSIGNED)3000, 3, (UNSIGNED)0,
                            NU_PREEMPT, NU_NO_START);
   if (status != NU_SUCCESS)
        return(NU_INVAL);

#endif  /* INCLUDE_RIP2 */


#endif /* PLUS */

    status = netinit();

    return(status);
} /* NU_Init_Net */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Socket                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for establishing a new socket          */
/*   descriptor and defining the type of communication protocol to       */
/*   be established.  Must be called by both clien and server whether    */
/*   connection-oriented or connectionless transfer is established.      */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      SCK_Create_Socket                                                */
/*                                                                       */
/*************************************************************************/
STATUS NU_Socket (int16 family, int16 type, int16 protocol)
{
    sint return_status;   /* initialized to error status */
   
	/* possible protocols based on [FAMILY, TYPE] */
    INT NU_proto_list[][5] =
	{
        {NU_PROTO_INVALID,  NU_PROTO_INVALID,  NU_PROTO_INVALID,
             NU_PROTO_INVALID,  NU_PROTO_INVALID},
        {NU_PROTO_INVALID,  NU_PROTO_INVALID,  NU_PROTO_INVALID,
             NU_PROTO_INVALID,  NU_PROTO_INVALID},
        {NU_PROTO_TCP,      NU_PROTO_UDP,      NU_PROTO_ICMP,
             NU_PROTO_INVALID,  NU_PROTO_INVALID}
    };

    /*  Clean up warnings.  This parameter is required for socket compatibility
        but we are currently not making any use of it.  */
    return_status = (sint)protocol;

    /* Make sure that family and type are in range. */
    if (family < 0 || family > 2 || type < 0 || type > 4)
        return (NU_INVALID_PROTOCOL);

    /* verify that we support the programmer's choice */
	if(!NU_proto_list[family][type])
		return(NU_INVALID_PROTOCOL);

    /*  Don't let any other users in until we are done.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

	/* Create a new socket. */
	return_status = SCK_Create_Socket(NU_proto_list[family][type]);

#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif

    /* return a socket_list index or an error status to caller */
    return (return_status);

}  /*  NU_Socket  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SCK_Create_Socket                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a new socket.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_Socket							Create a socket for an       */
/*                                            application.               */
/*      netlisten                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      NU_Tcp_Log_Error                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      INT                                 Protocol type (TCP or UDP).  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      A socket descriptor is returned on success.  NU_NO_SOCKET_SPACE  */
/*      (-31) is returned on failure.                                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*    Glen Johnson      11-23-1997      Created intial version. This was */
/*                                      part of NU_Socket.               */
/*                                                                       */
/*************************************************************************/
INT SCK_Create_Socket(INT protocol)
{
    struct sock_struct *sockptr;            /* pointer to current socket */
#ifdef PLUS
    STATUS status;
#else
    sint status;                        /* status of memory allocation */
#endif
    INT return_status = NU_NO_SOCKET_SPACE;   /* initialized to error status */
    sint counter;                              /* to traverse the socket list */
	
    /* search the socket list to be sure there is room for another connection */
    for (counter = 0; counter < NSOCKETS; counter++)
	{
        if (socket_list[next_socket_no] == NU_NULL)
        {
            /* allocate a socket structure */
#ifdef PLUS
            status = NU_Allocate_Memory(&System_Memory, (VOID **) &sockptr,
                            (UNSIGNED)sizeof(struct sock_struct),
                            (UNSIGNED)NU_NO_SUSPEND);
#else
            status = NU_Alloc_Memory (sizeof(struct sock_struct),
                                      (unsigned int **)&sockptr,
                                      NU_NO_TIMEOUT);
#endif
            if (status == NU_SUCCESS)
            {
                sockptr = (struct sock_struct *)normalize_ptr(sockptr);
                socket_list[next_socket_no] = sockptr;

                /* fill only the protocol portion of the socket structure */
                sockptr->protocol = protocol;

                /* initialize the port and ip portions of the socket */
                sockptr->local_addr.ip_num.is_ip_addrs[0] = NULL_IP;
                sockptr->local_addr.ip_num.is_ip_addrs[1] = NULL_IP;
                sockptr->local_addr.ip_num.is_ip_addrs[2] = NULL_IP;
                sockptr->local_addr.ip_num.is_ip_addrs[3] = NULL_IP;
                sockptr->local_addr.port_num = NU_IGNORE_VALUE;
                sockptr->foreign_addr.ip_num.is_ip_addrs[0] = NULL_IP;
                sockptr->foreign_addr.ip_num.is_ip_addrs[1] = NULL_IP;
                sockptr->foreign_addr.ip_num.is_ip_addrs[2] = NULL_IP;
                sockptr->foreign_addr.ip_num.is_ip_addrs[3] = NULL_IP;
                sockptr->foreign_addr.port_num = NU_IGNORE_VALUE;

				sockptr->s_recvlist.head = sockptr->s_recvlist.tail = NU_NULL;
				sockptr->s_recvbytes = sockptr->s_recvpackets = 0;
				sockptr->s_TXTask = sockptr->s_RXTask = NU_NULL;
				
				/* Set the intial state to connecting. */
				sockptr->s_state = SS_ISCONNECTING;
								
		        /* Initialize the socket options.  The abiltiy to transmit
                   broadcast messages is enabled by default. */
                sockptr->s_options = SO_BROADCAST;

                /* Clear the flags field. */
                sockptr->s_flags = 0;
                
                /* Initially there is no multicast option data. */
                sockptr->s_moptions = NU_NULL;
    
                /* return the socket_list index */
                return_status = next_socket_no;

                next_socket_no++;
                if (next_socket_no >= NSOCKETS)
                  next_socket_no = 0;

            }  /* end status == NU_SUCCESS */
            else
            {
                NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_RECOVERABLE,
                                    __FILE__, __LINE__);

                /*  Tell the user there is no memory.  */
                return_status = NU_NO_SOCK_MEMORY;
            }  /* end status != NU_SUCCESS */

            /* discontinue after we find an empty space in the socket list */
            break;

        }  /* end socket_list slot is null */

        next_socket_no++;
        if (next_socket_no >= NSOCKETS)
          next_socket_no = 0;
    }

	return (return_status);

} /* SCK_Create_Socket */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Bind                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for assigning a local address          */
/*   to a socket.                                                        */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*                                                                       */
/*************************************************************************/
STATUS NU_Bind(int16 socketd, struct addr_struct *myaddr, int16 addrlen)
{

    /*  Clean up warnings.  This parameter is used for socket compatibility
        but we are currently not making any use of it.  */
    unused_parameter = addrlen;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS))
        return(NU_INVALID_SOCKET);

    /*  Don't let any other users in until we are done.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
      
    /*  Fill the local portion of the socket descriptor */
    memcpy (&(socket_list[socketd]->local_addr.ip_num), &(myaddr->id),
            IP_ADDR_LEN);
    socket_list[socketd]->local_addr.port_num  = (int16)myaddr->port;
      
    /*  Allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /*  Return the updated socket descriptor to the caller */
    return(socketd);

}  /*  end of NU_Bind  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Listen                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for indicating that the server is      */
/*   willing to accept connection requests from clients.                 */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                                               */
/*      NU_Fcntl                                                         */
/*      NU_Task_Table_Add                                                */
/*      NU_TaskTableAdd                                                  */
/*      NU_Tcp_Log_Error                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     B. Harwell        10/21/92             Created Initial version.   */
/*     G. Johnson        10/26/95    Combined 3 calls to allocate memory */
/*                                   into 1 for efficiency.              */
/*                                                                       */
/*************************************************************************/
STATUS NU_Listen(int16 socketd, uint16 backlog)
{
    int16         return_status = NU_SUCCESS; /* initialize to SUCCESS */
#ifdef PLUS
    STATUS status;
    NU_TASK *task_id;
#else
    int16 status;                      /* status of memory allocation */
    int16 task_id;
#endif
    char        *return_ptr;                /* pointer to memory block */
    struct TASK_TABLE_STRUCT
                *Task_Entry;                /* structure of connections for
                                               this server/task_id */
    uint16       counter;                   /* used for initialization of
                                               Task_Entry */
      
    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /*  Don't let any other users in until we are done.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
      
    /* Allocate memory required for a Task_Entry structure, connection status
       array, and portlist entry array all at once.  This is done for efficiency
       (1 service call as opposed to 3).  Also, each of the 3 indepedent
       allocations was less than the default min allocation size (50 bytes) of
       PLUS, so memory was being wasted.
    */
#ifdef PLUS
    status = NU_Allocate_Memory(&System_Memory, (VOID **) &return_ptr,
                                (UNSIGNED)(sizeof(struct TASK_TABLE_STRUCT) +
                                (backlog * sizeof(unsigned short)) +
                                (backlog * sizeof(int16))),
                                (UNSIGNED)NU_NO_SUSPEND);
#else
    status = NU_Alloc_Memory(sizeof(struct TASK_TABLE_STRUCT) +
                            (backlog * sizeof(unsigned short)) +
                            (backlog * sizeof(int16)),
                            (unsigned int **)&return_ptr,
                            NU_NO_TIMEOUT);
#endif
      
    /* verify a successful allocation */
    if (status == NU_SUCCESS)
    {
        return_ptr = (char *)normalize_ptr(return_ptr);

        /* Break up the block of memory allocated into three seperate chunks. */
        Task_Entry = (struct TASK_TABLE_STRUCT *)return_ptr;

        Task_Entry->stat_entry = (short *)(return_ptr +
                                           sizeof(struct TASK_TABLE_STRUCT));

        Task_Entry->port_entry = (int16 *)(return_ptr +
                                           sizeof(struct TASK_TABLE_STRUCT) +
                                          (backlog * sizeof(unsigned short)));

        /* retrieve the local port number from the socket descriptor
           for the task table */
        Task_Entry->local_port_num =
                   (uint16)socket_list[socketd]->local_addr.port_num;

        /* record the socket number in the socketd field of Task_Entry */
        Task_Entry->socketd = socketd;

        /* retrieve the current task id for the task table */
#ifdef PLUS
        task_id = NU_Current_Task_Pointer();
        /* verify that a task is running */
        if (task_id != NU_NULL)
#else
        task_id = NU_Current_Task_ID();
        /* verify that a task is running */
        if (task_id != NU_SYSTEM)
#endif

            Task_Entry->Task_ID = task_id;
        else
            /* return the error to the caller */
            return_status = NU_NOT_A_TASK;

        /* initialize the port_entries and stat_entries */
        for (counter = 0; counter < backlog; counter++)
        {
            Task_Entry->port_entry[counter] = NU_IGNORE_VALUE;
            Task_Entry->stat_entry[counter] = NU_IGNORE_VALUE;
        }

        /* initialize the current connection pointer to the first
           space in the table */
        Task_Entry->current_idx = 0;

        /* store the number of backlog queues possible */
        Task_Entry->total_entries = backlog;

        /* Initialize the accept flag. */
        Task_Entry->acceptFlag = 0;

        /* initialize the next pointer to NU_NULL */
        Task_Entry->next = NU_NULL;

        /* call a routine to attach this structure to the
           linked list of structures */
        NU_TaskTableAdd(Task_Entry);

        /* Mark this socket as a listener. */
        socket_list[socketd]->s_flags |= SF_LISTENER;

    }
    else
    {
        NU_Tcp_Log_Error(TCP_SESS_MEM, TCP_RECOVERABLE,
                         __FILE__, __LINE__);

        /* Tell them no more memory. */
        return_status = NU_NO_SOCK_MEMORY;
    }
      
    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /* Turn on blocking for this socket so that blocking is the default for the
       NU_Accept call */
    NU_Fcntl(socketd, NU_SETFLAG, NU_BLOCK);

    /* return to the caller */
    return(return_status);

} /*  end of NU_Listen */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Accept                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for establishing a new socket          */
/*   descriptor containing info on both the server and a client          */
/*   with whom a connection has been successfully established.           */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      NU_GetPnum                                                       */
/*      NU_SearchTaskList                                                */
/*      NU_UpdateTaskTable                                               */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Allocate_Memory                                               */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/*   History:                                                            */
/*       created - 10/22/92, bgh                                         */
/*                                                                       */
/*************************************************************************/
STATUS NU_Accept(int16 socketd, struct addr_struct *peer, int16 *addrlen)
{
    uint16      server_port_num;
#ifdef PLUS
    NU_TASK     *Task_ID;        /* current task pointer */
#else
    sint         Task_ID;        /* current task id */
#endif
    struct port *pprt, *prt;     /* port pointer */
    struct TASK_TABLE_STRUCT   *Task_Entry = NU_NULL;  /* structure of connections
                                                          for this server/task_id */
    int16         pnum;           /* portlist index */
    int16         new_sock;       /* index of new socket in socket_list */
    int16         return_status = NU_INVALID_SOCKET;

    /* Handle compiler warnings. */
    unused_parameter = (uint32)addrlen;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL) ||
        !(socket_list[socketd]->s_flags & SF_LISTENER))
    {
        return(return_status);
    }

    /* init the new_sock value for the possible return if not set */
    new_sock = NU_NO_SOCKET_SPACE;

    /*  Prior to the NU_Accept() service call, an application must call
     *  NU_Listen().  NU_Listen() sets up a table to accept connection
     *  requests.  This table must be ready before NU_Accept() begins
     *  to wait for connection attempts. */

    /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    /* get the current task id for the next 2 service calls */
#ifdef PLUS
    Task_ID = NU_Current_Task_Pointer();
#else
    Task_ID = NU_Current_Task_ID();
#endif

    /* retrieve the local port number from the socket descriptor
       for comparison to the task table */
    server_port_num = (uint16)socket_list[socketd]->local_addr.port_num;

    /*  Check for an established connection.  If there isn't one, then
        suspend the calling task until the connection is made.  */
    do
    {

        /* search the task table for this port number/task id */
        Task_Entry = NU_SearchTaskList(Task_Entry, Task_ID,
                                       server_port_num, SEST, -1);

        /* continue only if a match was found in the task table */
        if (Task_Entry != NU_NULL)
        {
            /* get a pointer into the port list table at the oldest
               table entry */
            pprt =
                portlist[Task_Entry->port_entry[Task_Entry->current_idx]];

			/* Grab the index of the socket that was created when the 
			   connection was establisehd. */
			new_sock = pprt->p_socketd;

            /*  Make sure we got one.  */
            if (new_sock >= 0)
            {

               /* copy known information from the original socket
                  descriptor */
			   memcpy ( &socket_list[new_sock]->local_addr, 
						&socket_list[socketd]->local_addr, 
						sizeof(struct sockaddr_struct) );

			   memcpy ( &socket_list[new_sock]->foreign_addr, 
						&socket_list[socketd]->foreign_addr, 
						sizeof(struct sockaddr_struct) );

               socket_list[new_sock]->protocol = socket_list[socketd]->protocol;

               /* fill the client portion of the new socket descriptor */
               socket_list[new_sock]->foreign_addr.port_num =
                                   (int16)intswap (pprt->tcpout.dest);

               memcpy((uchar *)
                   &(socket_list[new_sock]->foreign_addr.ip_num),
                     pprt->tcp_faddr, IP_ADDR_LEN);

               /* store the new socket descriptor for return */
               return_status = new_sock;

               peer->family = NU_FAMILY_IP;
               peer->port = (uint16)socket_list[new_sock]->foreign_addr.port_num;
               memcpy (&peer->id, pprt->tcp_faddr, IP_ADDR_LEN);

               /* call a routine to delete this entry from the task
                  table and increment the current task entry pointer */
               NU_UpdateTaskTable(Task_Entry);

               /* Associate the new socket with this port. */
			   pprt->p_socketd = new_sock;

               /* Get the port number so that we can clear the transmit
                  Task ID */
               if ((pnum = NU_GetPnum(socket_list[new_sock])) >= 0)
               {
                   /* get the pointer */
                   prt = portlist[pnum];

                   /* Clear out the task entry information. */
                   prt->task_entry = NU_NULL;

                   /* clear the portlist task id */
#ifdef PLUS
                   socket_list[new_sock]->s_TXTask = NU_NULL; 
#else
				   socket_list[new_sock]->s_TXTask = -1
#endif
                }
                else {

                    return_status = NU_NO_PORT_NUMBER;
		}
            }
            else
            {
                /* return the error status returned from the socket call */
                return_status = new_sock;
            }
        }

        /* There are no connections to accept so check to see if the application
           wants to block pending a connection. */
        else if( !(socket_list[socketd]->s_flags & SF_BLOCK) )
        {
            /* Blocking is not desired so indicate that no connection was made. */
            return_status = NU_NO_TASK_MATCH;
            break;
        }
        else /* no match in task table */
        {
            /* start at the head of the list */
            Task_Entry = Task_Head;

            /* verify that the list is not empty */
            while (Task_Entry != NU_NULL)
            {
                /* discontinue searching if a match is found */
                if ((Task_Entry->Task_ID == Task_ID) &&
                  (Task_Entry->local_port_num == server_port_num))
                {
                    break;
                }
                /* continue checking the next structure */
                Task_Entry = Task_Entry->next;
            }

            /* Indicate that this task is accepting connections. */
            Task_Entry->acceptFlag = 1;

            /*  Let others in while we are waiting for the connection.  */
            SCK_Suspend_Task(Task_ID);

            /* Clear the accept flag. */
            Task_Entry->acceptFlag = 0;

            Task_Entry = NU_NULL;
        }

    }while (Task_Entry == NU_NULL);   /*  while waiting for a task entry. */

    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif

    /* return the new socket descriptor to the server */
    return(return_status);

} /* end of NU_Accept  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Send                                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for transmitting data across a         */
/*   network during a connection-oriented transfer.                      */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      netwrite                                                         */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/*   History:                                                            */
/*       created - 10/8/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
STATUS NU_Send(int16 socketd, CHAR *buff, uint16 nbytes, int16 flags)
{
    NET_BUFFER_SUSPENSION_ELEMENT   *waiting_for_buffer;
    struct sock_struct		*sockptr;
    uint16 bytes_to_go;                      /* number of bytes yet to transmit */
    int16 count;                             /* number of bytes written */
    int16 curr_count;                        /* number of bytes written this call */
    int16 return_status = NU_NO_PORT_NUMBER; /* initialized to error status */
    int16   status = 0;

    /*  Clean up warnings.  This parameter is used for socket compatibility
        but we are currently not making any use of it.  */
    unused_parameter = flags;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
      
    /* Get a pointer to the socket */
    sockptr = socket_list[socketd];

    /* Check for a NULL socket ptr and a connected socket. */
    if ( sockptr && (sockptr->s_state & SS_ISCONNECTED) )
    {
        /* initialize the byte counters */
        count = 0;
        bytes_to_go = nbytes;
      
        while ((bytes_to_go > 0))
        {
            /* call tcp/ip library netwrite routine */
            curr_count = netwrite( sockptr, (uint8 *)(buff + count), bytes_to_go, 
                                   &status);

            /* If 0 was returned, then for some reason we failed to send the
               data.  Check to see if we failed because we have already
               filled up the window of the remote host.  If so just
               continue.  We will suspend below, and be awakened when the
               remote host acks some of the data we have already sent.  At
               that point some more data can be transmitted.  If we failed
               for any other reason check to see if some data has already
               been sent.  If so return the number of bytes that have
               already been sent.  If no data has been sent (i.e., count ==
               0) then set count equal to the error code so the application
               layer will see the error.
            */
            if (curr_count == 0)
            {
                if ((status != NU_WINDOW_FULL) && (status != NU_NO_BUFFERS))
                {
                    if (count == 0)
                        count = curr_count;
                    break;
                }
            }

            /* update the bytes counter */
            bytes_to_go -= curr_count;

            /* update the total bytes sent */
            count += curr_count;

            /* If the status is NO BUFFERS then we will suspend until
               more buffers become available. */
            if (status == NU_NO_BUFFERS)
            {
                /* Allocate memory for a suspension list element. */
                status = NU_Allocate_Memory (&System_Memory,
                                        (VOID **)&waiting_for_buffer,
                                        sizeof (NET_BUFFER_SUSPENSION_ELEMENT),
                                        NU_NO_SUSPEND);

                /* Make sure we got the memory. */
                if (status == NU_SUCCESS)
                {
                    /* Add this task to the element. */
                    waiting_for_buffer->waiting_task = NU_Current_Task_Pointer();

                    /* Put this element on the buffer suspension list */
                    dll_enqueue (&MEM_Buffer_Suspension_List, waiting_for_buffer);

                    /* Suspend this task. */
                    SCK_Suspend_Task (waiting_for_buffer->waiting_task);

                    /* Give the memory back. */
                    NU_Deallocate_Memory (waiting_for_buffer);

                }
                else
                    NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_RECOVERABLE, 
                                        __FILE__, __LINE__);
            }
            
            /* If the number of bytes to go indicates more to be sent
               then suspend until there are buffers available. */
            else if (bytes_to_go > 0)
            {

                /* Increment the number of tasks that are waiting for
                    buffers before they can send. */
                tasks_waiting_to_send++;
#ifdef PLUS
                sockptr->s_TXTask = NU_Current_Task_Pointer();
                SCK_Suspend_Task(sockptr->s_TXTask);
                sockptr->s_TXTask = NU_NULL;
#else /* RTX */
                sockptr->s_TXTask = NU_Current_Task_ID();
                SCK_Suspend_Task(sockptr->s_TXTask);
                sockptr->s_TXTask = -1;
#endif /* PLUS */

            } /* still bytes to be sent */

        } /* while still bytes to transmit. */

        /* verify success of transfer, if a code of less than
         * zero is received that means that the connection
         * is broken, the return_status is set up to
         * indicate that. */
        if ((status == NU_SUCCESS) && (count >= 0))
            /* return number of bytes transferred */
            return_status = count;
        else
            return_status = status;
    }
    /* Either the socket pointer was NULL or the connection has closed. */
    else
        return_status = NU_NOT_CONNECTED;
           
    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /* return to caller */
    return(return_status);

}  /*  end of NU_Send  */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Send_To                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for transmitting data across           */
/*   a network during a connectionless transfer.                         */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      comparen                                                         */
/*      intswap                                                          */
/*      makeuport                                                        */
/*      UDP_Send                                                         */
/*      NU_Get_UDP_Pnum                                                  */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*                                                                       */
/*   History:                                                            */
/*       created - 10/8/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
STATUS NU_Send_To(int16 socketd, char *buff, uint16 nbytes, int16 flags,
     struct addr_struct *to, int16 addrlen)
{
    int16 count;                              /* number of bytes written */
    struct uport       *uprt;
    struct sock_struct *sockptr;            /* pointer to current socket */
    int16 port_num;                          /* local machine's port number */
    int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */
    uint16 dest_port;

    /*  Clean up warnings.  This parameter is used for socket compatibility
     *  but we are currently not making any use of it.  */
    unused_parameter = flags;
    unused_parameter = addrlen;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
                (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /* added 11/3/92 - reserve the resource */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    /* Pick up a pointer to the socket list entry.  */
    sockptr = socket_list[socketd];

    /* get the server's port number */
    dest_port = to->port;

    /* verify that a port number exists */
    if (dest_port)
    {
        /*  Make sure that he gave us the IP number. */
        if (comparen (to->id.is_ip_addrs, (VOID *)IP_Null, 4))
        {
            /* allow others to use the TCP resource */
#ifdef PLUS
            NU_Release_Semaphore(&TCP_Resource);
#else
            NU_Release_Resource(TCP_Resource);
#endif
      
            return(NU_INVALID_ADDRESS);
        }

        /* Pick up the foreign IP address and port number. */
        sockptr->foreign_addr.port_num = (int16)dest_port;
        memcpy(&(sockptr->foreign_addr.ip_num), &(to->id), IP_ADDR_LEN);

        /*  Make a port for this send if one does not already exist. */
        if (sockptr->local_addr.port_num > 0)
        {

            /*  Check if there is already a port structure set up
             *  for this UDP communication.  */
            port_num = NU_Get_UDP_Pnum(sockptr);
        }
        else
            port_num = NU_IGNORE_VALUE;

        /*  If we did not find a port for this entry then we need to create
            one. */
        if (port_num == NU_IGNORE_VALUE)
        {
            port_num = makeuport(sockptr->local_addr.port_num, socketd);

            if (port_num < 0)
            {
                /* allow others to use the TCP resource */
#ifdef PLUS
                NU_Release_Semaphore(&TCP_Resource);
#else
                NU_Release_Resource(TCP_Resource);
#endif
      
                return(NU_NO_PORT_NUMBER);
            }
        }

        /*  Fill in some of the fields that were not set up in makeuport. */
        uprt = uportlist[port_num];

        /*  If makeuport created a port number for me then put it in
         *  the socket.  */
        if (sockptr->local_addr.port_num <= 0)
           sockptr->local_addr.port_num = (int16)intswap(uprt->up_lport);

        /*  Get his IP number.  */
        memcpy (uprt->up_faddr, to->id.is_ip_addrs, IP_ADDR_LEN);

        /*  Get his port number. */
        uprt->up_fport = intswap(dest_port);

        /*  Get my port number. */
        uprt->up_lport = intswap((uint16)sockptr->local_addr.port_num);

        /*  Send the data. */
        count = UDP_Send(uprt, (unsigned char *) buff, nbytes,
                         sockptr->s_options);

        /*  Let them know if it worked OK. */
        if (count < 0)
            /* return an error status */
            return_status = NU_NO_DATA_TRANSFER;
        else
            /* return number of bytes transferred */
            return_status = count;

    }  /*  He specified a destination port. */
           
    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /* return to caller */
    return(return_status);

}  /*  end of NU_Send_To  */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_Recv                                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This fuction will handle receiving data across a network during a       */
/*  connection oriented transfer.                                           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  Barb G. Harwell, Accelerated Technology Inc.                            */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      User Applications                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    SCK_Suspend_Task                                                      */
/*    NU_Obtain_Semaphore                                                   */
/*    NU_Release_Semaphore                                                  */
/*    NU_Current_Task_Pointer                                               */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*  No inputs to the function                                               */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*  No outputs from this function                                           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                   DATE        REMARKS                              */
/*                                                                          */
/*  Barb G. Harwell      10/08/92      Initial version.                     */
/*  Craig L. Meredith    02/15/94      Clean up, added headers, interrupt   */
/*                                     version.                             */
/*                                                                          */
/****************************************************************************/
STATUS NU_Recv (int16 socketd, char *buff, uint16 nbytes, int16 flags)
{
   int16 count;                              /* number of bytes written */
   int16 port_num;                           /* local machine's port number */
   int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */
   struct sock_struct *sockptr;            /* pointer to current socket */
      


   /*  Clean up warnings.  This parameter is used for socket compatibility
       but we are currently not making any use of it.  */
   unused_parameter = flags;

   /*  Validate the socket number.  */
   if ((socketd < 0) || (socketd >= NSOCKETS))
   {
       return (NU_INVALID_SOCKET);
   }

   if( (socket_list[socketd] == NU_NULL) )
   {
       return (NU_NOT_CONNECTED);
   }

   /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
   NU_Obtain_Semaphore (&TCP_Resource, NU_SUSPEND);
#else  /* PLUS */
   NU_Request_Resource (TCP_Resource, NU_WAIT_FOREVER);
#endif /* !PLUS */
      
   /*  Pick up a pointer to the socket list. */
   sockptr = socket_list[socketd];

#ifdef PLUS
   sockptr->s_RXTask = NU_Current_Task_Pointer();
   /* verify that a task is running */
   if (sockptr->s_RXTask != NU_NULL)
#else  /* PLUS */
   sockptr->s_RXTask = NU_Current_Task_ID();
   /* verify that a task is running */
   if (sockptr->s_RXTask != NU_SYSTEM)
#endif /* !PLUS */
   {
       /* retrieve the local port number from the socket descriptor */
       port_num = sockptr->local_addr.port_num;
      
       /* verify that a port number exists */
       if (port_num)
       {
           /* 
            * Check the socket's block flag to see if the caller wants to
            * wait for data or not, and if so, there is no data in the
            * input buffer and the connection is still alive.
            */
           if ( (sockptr->s_flags & SF_BLOCK) && (sockptr->s_recvbytes == 0)
                && (sockptr->s_state & SS_ISCONNECTED) )
           {
               /*  Give up the resource until we can run again.  */
#ifdef PLUS
               SCK_Suspend_Task (NU_Current_Task_Pointer ());
#else  /* PLUS */
               SCK_Suspend_Task (NU_Current_Task_ID());
#endif /* !PLUS */
           }

           /* call tcp/ip library netread routine */
           count = netread (sockptr, buff, nbytes);

           /* verify success of transfer */
           if (count < 0)
           {
               /* return an error status */
               return_status = NU_NOT_CONNECTED;                   
           }
           else
           {
               /* return number of bytes transferred */
               return_status = count;
           }
       }
           
       /* clear the portlist task id */
#ifdef PLUS
       sockptr->s_RXTask = NU_NULL;
#else  /* PLUS */
       sockptr->s_RXTask = -1;
#endif /* !PLUS */

   }  /*  System is running, not a task  */
   else
   {
       /* return the error to the caller */
       return_status = NU_NOT_A_TASK;
   }
           
   /* allow others to use the TCP resource */
#ifdef PLUS
   NU_Release_Semaphore (&TCP_Resource);
#else  /* PLUS */
   NU_Release_Resource (TCP_Resource);
#endif /* !PLUS */
      
   /* return to caller */
   return (return_status);
} /* NU_Recv */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Recv_From                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for receiving data across a network    */
/*   during a connectionless transfer.                                   */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      makeuport                                                        */
/*      UDP_Read                                                         */
/*      NU_Get_UDP_Port                                                  */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/*   History:                                                            */
/*    Barbara G. Harwell   10/08/92      Initial version.                */
/*                                                                       */
/*************************************************************************/
STATUS NU_Recv_From(int16 socketd, char *buff, int16 nbytes, int16 flags,
 struct addr_struct *from, int16 *addrlen)
{
    struct uport       *uprt;
    int16 port_num;                           /* local machine's port number */
    int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */
    sshort my_port;
    struct sock_struct *sockptr;            /* pointer to current socket */

    /*  Clean up warnings.  This parameter is used for socket compatibility
        but we are currently not making any use of it.  */
    unused_parameter = flags;
    unused_parameter = nbytes;
    unused_parameter = (int16)*addrlen;

    *addrlen = sizeof(struct addr_struct);

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    /*  Pick up a pointer to the socket entry. */
    sockptr = socket_list[socketd];

    /* get my port number */
    my_port = sockptr->local_addr.port_num;

    /* verify that a port number exists */
    if (my_port)
    {

        /*  Check if there is already a port structure set up
            for this UDP communication.  */
        port_num = NU_Get_UDP_Pnum(sockptr);

        if (port_num != NU_IGNORE_VALUE)
            /*  If there is a port number make sure that this is the one. */
            if (sockptr->local_addr.port_num != my_port)
                port_num = NU_IGNORE_VALUE;

        /*  If we did not find a port for this entry then we need to create
            one. */
        if (port_num == NU_IGNORE_VALUE)
        {
            port_num = makeuport(sockptr->local_addr.port_num, socketd);

            if (port_num < 0)
                return(NU_NO_PORT_NUMBER);
        }

        /*  Get a pointer to the UDP port. */
        uprt = uportlist[port_num];

        if(!uprt->in_dgrams)
        {
            /* initialize user task number */
#ifdef PLUS
            uprt->RXTask = NU_Current_Task_Pointer();
#else
            uprt->RXTask = NU_Current_Task_ID();
#endif

            /* Initialize the port number that we are expecting to receive. */
            uprt->up_lport = intswap((uint16)sockptr->local_addr.port_num);

#ifdef PLUS
            SCK_Suspend_Task (NU_Current_Task_Pointer ());

            /* initialize user task number */
            uprt->RXTask = NU_NULL;
#else  /* PLUS */
            SCK_Suspend_Task (NU_Current_Task_ID());

            /* initialize user task number */
            uprt->RXTask = -1;
#endif /* !PLUS */
        }

        /*  Pick up the data. */
        return_status = UDP_Read(uprt, buff, from, sockptr);

    }  /*  He specified a destination port. */
           
    /* allow others to use the TCP resource */
#ifdef PLUS
NU_Release_Semaphore(&TCP_Resource);
#else
NU_Release_Resource(TCP_Resource);
#endif
      
    /* return to caller */
    return(return_status);
} /* NU_Recv_From */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Close_Socket                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is responsible for breaking the socket connection     */
/*   in the socket_list at the index specified by socketd.               */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Deallocate_Memory                                             */
/*      netclose                                                         */
/*      NU_Get_UDP_Port                                                  */
/*      NU_GetPnum                                                       */
/*      NU_TaskTable_Entry_Delete                                        */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/*   History:                                                            */
/*       created - 10/8/92, bgh                                          */
/*       added NU_TaskTable_Entry_Delete, 02/29/96                       */
/*                                                                       */
/*************************************************************************/
STATUS NU_Close_Socket(int16 socketd)
{
    struct sock_struct		*sockptr;
    int16 port_num;                           /* local machine's port number */
    int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */
    uint *tempPtr;                            /* temp pointer to the given socket */
    int16 close_status = -1;                  /* status returned from netclose */
      
    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /* assume return successfully to caller */
    return_status = NU_SUCCESS;

    /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    sockptr = socket_list[socketd]; 
      
    /*  Do different processing based on the protocol type.  */
    if (sockptr->protocol == NU_PROTO_TCP)
    {

        if ((port_num = NU_GetPnum(sockptr)) >= 0)
        {
            while (close_status < 0)
            {
                /* Mark the socket state as disconnecting. */
                SCK_DISCONNECTING(socketd);

                /* Close this connection. */
                close_status = netclose(port_num, sockptr);

                /*  If netclose did not return a successful value then we need
                    to wait until the connection is closed.  */
                if (close_status < 0)
                {

#ifdef PLUS
                    sockptr->s_RXTask = NU_Current_Task_Pointer();
#else  /* PLUS */
                    sockptr->s_RXTask = NU_Current_Task_ID();
#endif /* !PLUS */
                    SCK_Suspend_Task (sockptr->s_RXTask);
                }
            }
        }
        else
            /*  The port structure has already been deallocated by the stack. */
            return_status = NU_SUCCESS;

    }
    else
    {

        port_num = NU_Get_UDP_Pnum(sockptr);

        if (port_num >= 0)
        {

#if SNMP_INCLUDED
            /* Update the UDP Listen Table. */
            SNMP_udpListenTableUpdate(SNMP_DELETE, sockptr->local_addr.ip_num.is_ip_addrs,
                                      sockptr->local_addr.port_num);
#endif

#ifdef PLUS
            /*  Clear this port list entry.  */
            NU_Deallocate_Memory((uint *) uportlist[port_num]);
#else
            /*  Clear this port list entry.  */
            NU_Dealloc_Memory((unsigned int *) uportlist[port_num]);
#endif

            /*  Indicate that this port is no longer used. */
            uportlist[port_num] = NU_NULL;
        }
        else
            /*  The port structure has already been deallocated by the stack. */
            return_status = NU_SUCCESS;
    }

    /* delete its task table entry */
    NU_TaskTable_Entry_Delete(socketd);

    /* create a temporary pointer to the current socket descriptor */
    tempPtr = (uint *)socket_list[socketd];
          
    /* clear this socket pointer for future use */
    socket_list[socketd] = NU_NULL;
          
    /* release the memory used by this socket */
#ifdef PLUS
    /*  Clear this port list entry.  */
    NU_Deallocate_Memory((uint *) tempPtr);
#else
    /*  Clear this port list entry.  */
    NU_Dealloc_Memory((unsigned int *)tempPtr);
#endif
          
    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /* return to caller */
    return(return_status);
} /* NU_Close_Socket */

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*      NU_UpdateTaskTable                                                  */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function is responsible for clearing a connection from             */
/*  the task table structure.                                               */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      NU_Accept                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                   DATE        REMARKS                            */
/*                                                                          */
/*    Barbara G. Harwell   10/22/92   Initial version.                      */
/*    Glen Johnson         12/11/95   Made changes based on recommendations */
/*                                    from Kazuomi Tanaka of Hitachi.  The  */
/*                                    changes keep a connection from being  */
/*                                    accepted before a connection is       */
/*                                    completely made.                      */
/*                                                                          */
/****************************************************************************/
VOID NU_UpdateTaskTable(struct TASK_TABLE_STRUCT *Task_Entry)
{
      /* clear the portlist number entries at the current index */
      Task_Entry->port_entry[Task_Entry->current_idx] = NU_IGNORE_VALUE;
      Task_Entry->stat_entry[Task_Entry->current_idx] = NU_IGNORE_VALUE;
} /* NU_UpdateTaskTable */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_TaskTableAdd                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for appending a task table structure    */
/*  to the end of a linked list of task table structures.                */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      NU_Listen                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/*   History:                                                            */
/*       created: 10/23/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
VOID NU_TaskTableAdd(struct TASK_TABLE_STRUCT *Task_Entry)
{
      /* see if the list is empty */
      if (Task_Tail == NU_NULL)
      {
          /* this is the only structure in the list */
          Task_Head = Task_Entry;
          Task_Tail = Task_Entry;
      }
      else
      {
          /* append this structure to the end of the list */
          Task_Tail->next = Task_Entry;
          
          /* move the tail pointer to the end of the list */
          Task_Tail = Task_Entry;
      }
} /* NU_TaskTableAdd */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_TaskTable_Entry_Delete                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for deleting a task table entry from    */
/*  the task table.                                                      */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      NU_Close_Socket                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Deallocate_Memory                                             */
/*                                                                       */
/*   History:                                                            */
/*       created: 02/29/96, MQ                                           */
/*       modified: 03/08/96, MQ                                          */
/*                                                                       */
/*************************************************************************/
STATUS NU_TaskTable_Entry_Delete(int16 socketd)
{
	struct TASK_TABLE_STRUCT *list, *entry;

    if (Task_Head == NU_NULL)
		return(0);

	/* pick up the task entry according to its port number, delete it */
    /* Check the first entry, if there is a match this is the easy case. */
    if (Task_Head->socketd==socketd)
	{
        /* Remove the entry. */
        entry = Task_Head;
		Task_Head = Task_Head->next;
        if (Task_Head == NU_NULL)
            Task_Tail = NU_NULL;
	}
	else
	{
        /* Search for the correct entry. */
        list = Task_Head;
		while (1)
		{
            if (list->next == NU_NULL)
				return(0);
			if (list->next->socketd==socketd)
				break;
			else
				list = list->next;
		}

        /* If we made it to here then the correct entry was found, else we would
           have returned above.  Point to the entry to be removed. */
        entry = list->next;

        /* Cut the entry out of the list. */
        list->next = list->next->next;

        /* If the last entry was removed, then update the tail pointer. */
        if (entry==Task_Tail)
			Task_Tail = list;
	}

	NU_Deallocate_Memory((uint *) entry);

    return(1);
} /* NU_TaskTable_Entry_Delete */

/****************************************************************************/
/*                                                                          */
/* FUNCTION                                                                 */
/*                                                                          */
/*      NU_SearchTaskList                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function is responsible for searching the task list table          */
/*  structures until an entry with a match for the Task ID and server       */
/*  port number specified by the caller is found.                           */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      NU_Accept                                                           */
/*      tcpdo                                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME                   DATE        REMARKS                            */
/*                                                                          */
/*    Barbara G. Harwell   10/23/92   Initial version.                      */
/*    Glen Johnson         12/11/95   Made changes based on recommendations */
/*                                    from Kazuomi Tanaka of Hitachi.  The  */
/*                                    changes keep a connection from being  */
/*                                    accepted before a connection is       */
/*                                    completely made.                      */
/*                                                                          */
/****************************************************************************/
struct TASK_TABLE_STRUCT * NU_SearchTaskList(struct TASK_TABLE_STRUCT 
  *Task_Entry,
#ifdef PLUS
  NU_TASK *Task_ID,
#else
  sint Task_ID,
#endif
  uint16 server_port_num, int16 state, int16 pnum)
{
    uint16 index;
    int16  match = NU_FALSE;

    /* start at the head of the list */
    Task_Entry = Task_Head;

    /* verify that the list is not empty */
    while (Task_Entry != NU_NULL)
    {
        /* discontinue searching if a match is found */
        if ((Task_Entry->Task_ID == Task_ID) &&
           (Task_Entry->local_port_num == server_port_num))
        {
            for(index = 0; index < Task_Entry->total_entries; index++)
            {
                /* verify that there is a portlist number entered.  If a state
                   of zero is passed in we don't care about the state.  So look
                   for a match on the port_entry alone.
                */
                if((Task_Entry->port_entry[index] >= 0) &&
                   ((pnum == -1) || (Task_Entry->port_entry[index] == pnum)) &&
                   ((state == -1) || (Task_Entry->stat_entry[index] == state)))
                {
                    Task_Entry->current_idx = index;
                    match = NU_TRUE;
                    break;
                } /* if */
            } /* for */
        }/* if */

        if(match == NU_TRUE)
            break;

        /* continue checking the next entry in the list */
        Task_Entry = Task_Entry->next;
    }/* while */

    /* return pointer to caller */
    return(Task_Entry);
}   /* end NU_SearchTaskList */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_GetPnum                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for returning a portlist entry number   */
/*  to a caller based on a match with the port information sent in the   */
/*  socket descriptor.                                                   */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      NU_Accept                                                        */
/*      NU_Send                                                          */
/*      NU_Recv                                                          */
/*      NU_Close_Socket                                                  */
/*      NU_Push                                                          */
/*      NU_Is_Connected                                                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*  Barbara G. Harwell   10/29/92   Initial version.                     */
/*  G. Johnson           10-12-95   Modified so hosts with different     */
/*                                  IP's but same port #'s could be      */
/*                                  distinguished.                       */
/*                                                                       */
/*************************************************************************/
int16 NU_GetPnum(struct sock_struct *socket_desc)
{
      int16 pnum = NU_IGNORE_VALUE;    /* portlist entry number initialized */
      int16 index;
      struct port *prt;
      
      for (index=0; index<NPORTS; index++)
      {
          prt = portlist[index];
          if ((prt != NU_NULL) &&
            (prt->in.port  == (uint16)socket_desc->local_addr.port_num) &&
            (prt->out.port == (uint16)socket_desc->foreign_addr.port_num) &&
            comparen(prt->tcp_faddr,
                     socket_desc->foreign_addr.ip_num.is_ip_addrs, IP_ADDR_LEN))
          {
              pnum=index;
              break;
          } /* end if */
      } /* end for */
      
      /* return a portlist entry number to the caller */
      return(pnum);
} /* NU_GetPnum */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Get_UDP_Pnum                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function is responsible for returning a portlist entry number   */
/*  to a caller based on a match with the port information sent in the   */
/*  socket descriptor.                                                   */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      NU_Abort                                                         */
/*      NU_Close_Socket                                                  */
/*      NU_Recv_From                                                     */
/*      NU_Send_To                                                       */
/*      SEL_Check_Recv                                                   */
/*      SEL_Setup_Recv_Ports                                             */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*                                                                       */
/*   History:                                                            */
/*       created: 10/29/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
int16 NU_Get_UDP_Pnum(struct sock_struct *socket_desc)
{
      int16 pnum = NU_IGNORE_VALUE;    /* portlist entry number initialized */
      int16 index;
      struct uport *uprt;
      
      for (index=0; index<NUPORTS; index++)
      {
          uprt = uportlist[index];
          if ( uprt != NU_NULL &&
              (uprt->up_lport == intswap((uint16)socket_desc->local_addr.port_num)))
          {
              pnum=index;
              break;
          } /* end if */
      } /* end for */
      
      /* return a portlist entry number to the caller */
      return(pnum);
} /* NU_Get_UDP_Pnum */


/********************************************************************
*   FUNCTION
*
*       NU_Fcntl
*
*   DESCRIPTION
*
*       This function is responsible for enabling/disabling the "block"
*       flag for the specified socket descriptor.
*
*   AUTHOR
*
*       Neil Henderson
*
*  CALLED FROM
*
*      User Applications
*
*   CALLS
*
*   INPUTS
*
*       int16  -- socketd index sinto the socket_list array.
*       int16  -- command NU_SETFLAG for setting flags
*       int16  -- arguement used to say if blocking is on or off.
*
*   OUTPUTS
*
*
*   HISTORY:
*
*       BGH     11/06/92        created
*
*********************************************************************/

STATUS NU_Fcntl (int16 socketd, int16 command, int16 arguement)
{
    int16 return_status = NU_NO_ACTION;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) || (!socket_list[socketd]))
        return(NU_INVALID_SOCKET);

    /* check for the command flag is ok */
    if (command != NU_SETFLAG)
    {
        return (NU_NO_ACTION);
    }

    if (arguement == NU_BLOCK)
    {
        socket_list[socketd]->s_flags |= SF_BLOCK;
        return_status = NU_SUCCESS;
    }
    else if (arguement == NU_FALSE)
    {
        socket_list[socketd]->s_flags &= ~SF_BLOCK;
        return_status = NU_SUCCESS;
    }

    /* return to caller */
    return (return_status);
} /* NU_fcntl */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Ioctl                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Performs special functions on a interface or other object.       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Applications                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DEV_Get_Dev_By_Name                                              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      <Inputs>                            <Description>                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
STATUS NU_Ioctl(INT optname, SCK_IOCTL_OPTION *option, INT optlen)
{
    DV_DEVICE_ENTRY     *dev;
    STATUS              status;

    switch (optname)
    {
    
    case IOCTL_GETIFADDR :
        
        if (optlen < sizeof(SCK_IOCTL_OPTION))
        {
            status = NU_INVAL;
            break;
        }

        if ((dev = DEV_Get_Dev_By_Name((CHAR *)option->s_optval)) == NU_NULL)
        {
            status = NU_INVALID_PARM;
            break;
        }


        memcpy (option->s_ret.s_ipaddr, dev->dev_addr.dev_ip_addr, IP_ADDR_LEN);
        status = NU_SUCCESS;
        break;

    default :
        status = NU_INVALID_OPTION;
    
    }

    return status;

} /* NU_Ioctl */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Connect                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   This function is called by a client wishing to establish a          */
/*   connection with a server.                                           */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      intswap                                                          */
/*      netxopen                                                         */
/*      SCK_Suspend_Task                                                 */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Release_Semaphore                                             */
/*      NU_Current_Task_Pointer                                          */
/*                                                                       */
/*   History:                                                            */
/*       created: 10/6/92, bgh                                           */
/*                                                                       */
/*************************************************************************/
STATUS NU_Connect (int16 socketd, struct addr_struct *servaddr, int16 addrlen)
{
    struct sock_struct		*sockptr;
    struct port *pprt;
    uint16 dest_port;          /* destination (server's) port number */
    sshort port_num;           /* client port number returned from netxopen */
    int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */

    /*  Clean up warnings.  This parameter is used for socket compatibility
        but we are currently not making any use of it.  */
    unused_parameter = (int16) addrlen;

    if (servaddr == NU_NULL)
        return (NU_INVALID_PARM);

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /* get the server's port number */
    dest_port = servaddr->port;

    /* verify that a port number exists */
    if (dest_port)
    {
        /* added 11/3/92 - reserve the resource */
#ifdef PLUS
        NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
        NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
		sockptr = socket_list[socketd];

        /* Attempt to establish the connection. */
        if ((port_num = netxopen((uint8 *)&servaddr->id,
                                 (int16)dest_port, socketd)) >= 0)
        {
            /* store the current task in the portlist  - we need to be able
              to restart this task after the NU_Stop - 11/4/92, bgh */
            pprt = portlist[port_num];

#ifdef VIRTUAL_NET_INCLUDED
            /* It is possible when using the Virtual Network environement for 
               the connection to be established by the time this point is 
               reached.  This check guarantees that the task will not be 
               suspended indefinitely when using VNET. 
            */
            if (pprt->state != SEST)
#endif
            {
#ifdef PLUS
            sockptr->s_TXTask = NU_Current_Task_Pointer();
            SCK_Suspend_Task(sockptr->s_TXTask);
            sockptr->s_TXTask = NU_NULL;
#else
            sockptr->s_TXTask = NU_Current_Task_ID();
            SCK_Suspend_Task(sockptr->s_TXTask);
            sockptr->s_TXTask = -1;
#endif
            }
            /*  Check to see if the connection was OK.  */
            /* NOTE: When the migration of data from the port structure to the 
                     socket structre is completed, I think the s_state field 
                     should be checked instead of the port state. */
            if (pprt->state == SEST)
            {
                /* store the foreign (server's) port # and ip # in the socket
                   descriptor */
                sockptr->foreign_addr.port_num = (int16)servaddr->port;
                memcpy ( &(sockptr->foreign_addr.ip_num), &(servaddr->id), 
                         IP_ADDR_LEN);

                /* store the local (client's) port # and ip # in the socket
                    descriptor */
                sockptr->local_addr.port_num =
                    (int16)intswap(pprt->tcpout.source);
                memcpy ( &(sockptr->local_addr.ip_num), pprt->tcp_laddr, 
                         IP_ADDR_LEN);

                /* return the socket descriptor to the caller */
                return_status = socketd;

            } /* connection established OK. */
            else
                return_status = NU_NOT_CONNECTED;

        } /* end connection established */
        else
			return_status = port_num;

        /* allow others to use the TCP resource */
#ifdef PLUS
        NU_Release_Semaphore(&TCP_Resource);
#else
        NU_Release_Resource(TCP_Resource);
#endif
    }  /* end if dest_port was not NU_NULL */

    /* return to caller */
    return(return_status);
}  /* NU_Connect */


/******************************************************************************/
/*                                                                            */
/* FUNCTION                                                                   */
/*                                                                            */
/*      NU_Setsockopt                                                         */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This function is responsible for setting socket options.  Modeled         */
/*  after the BSD version of this service, the initial implementation         */
/*  only allows broadcasting to be enabled/disabled.  Other options will      */
/*  be added in the future as needed.  The ability to broadcast is            */
/*  enabled by default.                                                       */
/*                                                                            */
/*  CALLED FROM                                                               */
/*                                                                            */
/*      Application programs                                                  */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*                                                                            */
/* NAME            DATE                    REMARKS                            */
/*                                                                            */
/* G. Johnson    04-11-1996      Created Initial version.                     */
/*                                                                            */
/******************************************************************************/
STATUS NU_Setsockopt(int16 socketd, INT level, INT optname, void *optval,
                    INT optlen)
{
    STATUS      status;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS))
        return (NU_INVALID_SOCKET);

    if( (socket_list[socketd] == NU_NULL) )
        return (NU_NOT_CONNECTED);

    /* Is the specified level supported. */
    switch (level)
    {
    
    case SOL_SOCKET:

        switch(optname)
        {

         case SO_BROADCAST:
            /* Was a value specified and an appropriate size specified. */
            if (optval == NU_NULL || optlen  < sizeof(int16))
                return (NU_INVAL);

            /* The flag is set when a nonzero value is used and cleared when
               a 0 is passed in. */
            if (*(int16 *)optval)
                socket_list[socketd]->s_options |= optname;
            else
                socket_list[socketd]->s_options &= ~optname;

            status = NU_SUCCESS;
            break;

        default:
            status = NU_INVALID_OPTION;
            break;
        }
        break;

    case IPPROTO_IP :

        status = IP_Set_Opt (socketd, optname, optval, optlen);
        break;

    default :
        
        status = NU_INVALID_LEVEL;
        break;
    }

    return (status);

} /* end NU_Setsockopt */


/******************************************************************************/
/*                                                                            */
/* FUNCTION                                                                   */
/*                                                                            */
/*      NU_Getsockopt                                                         */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This function is responsible for returning the current setting of the     */
/*  various socket flags.  Modeled after the BSD version of this service, the */
/*  initial implementation only returns the status of the broadcasting option.*/
/*  Other options will be added in the future as needed.                      */
/*                                                                            */
/*  CALLED FROM                                                               */
/*                                                                            */
/*      Application programs                                                  */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*                                                                            */
/* NAME            DATE                    REMARKS                            */
/*                                                                            */
/* G. Johnson    04-11-1996      Created Initial version.                     */
/*                                                                            */
/******************************************************************************/
STATUS NU_Getsockopt(int16 socketd, INT level, INT optname, void *optval,
                    INT *optlen)
{
    STATUS       status;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS))
        return (NU_INVALID_SOCKET);

    if( (socket_list[socketd] == NU_NULL) )
        return (NU_NOT_CONNECTED);

    /* Was a value specified and an appropriate size specified. */
    if (optval == NU_NULL)
        return (NU_INVAL);

    switch (level)
    {
    case SOL_SOCKET : /* Socket Level */

        switch(optname)
        {

        case SO_BROADCAST:
        
            /* If the flag is already set then the & will result in a
               non-zero value, else zero will result. */
            *(int16 *)optval = socket_list[socketd]->s_options & optname;
            *optlen = sizeof(socket_list[socketd]->s_options);

            status = NU_SUCCESS;
            break;

        default:
            return (NU_INVALID_OPTION);
        }

        break;

    case IPPROTO_IP : /* IP Level. */

        status = IP_Get_Opt (socketd, optname, optval, optlen);
        break;

    default :
        status = NU_INVALID_LEVEL;
        break;
    }

    return (status);

} /* end NU_Setsockopt */


/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   NU_Get_Host_By_Name                                                      */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    Given a host name this function searches for the host in the "hosts     */
/*    file".  A struture allocated by the calling application is filled in    */
/*    with the host information.                                              */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Barbara Harwell,   Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    Applications                                                            */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    DNS_Find_Host_By_Name     Look up a host.                               */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    name                      The name of the host to look for.             */
/*    host_entry                A pointer to a structure that will be filled  */
/*                                in with the host information.               */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_INVALID_PARM           Indicates that one of the parameters was a    */
/*                                NULL pointer.                               */
/*    NU_NOT_A_HOST             The host was not found.                       */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Barbara Harwell   11/12/92      Created Initial version.                */
/*    Glen Johnson      06/16/97      Update to use for NET 3.1 to use DNS.   */
/*                                                                            */
/******************************************************************************/
STATUS NU_Get_Host_By_Name(CHAR *name, NU_HOSTENT *host_entry)
{
    DNS_HOST        *hst;
      
    /* Check for a NULL pointer. */
    if ((!name) || (!host_entry))
        return NU_INVALID_PARM;

    if ((hst = DNS_Find_Host_By_Name(name)) != NU_NULL)
    {
        /* The correct entry was found. Fill in host_entry. */
        host_entry->h_addr = hst->dns_ipaddr;
        host_entry->h_name  = hst->dns_name;
        host_entry->h_alias = (char **)NU_NULL;  /* unused */
        host_entry->h_addrtype = NU_FAMILY_IP;
        host_entry->h_length = IP_ADDR_LEN;                /* unused */

        return NU_SUCCESS;
    }
    else
        return (NU_NOT_A_HOST);

} /* NU_Get_Host_By_Name */

/******************************************************************************/
/* FUNCTION                                                                   */
/*                                                                            */
/*   NU_Get_Host_By_Addr                                                      */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*    Given a host IP Addresse this function searches for the host in the     */
/*    "hosts file".  A struture allocated by the calling application is       */
/*    filled in with the host information.                                    */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*    Glen Johnson,      Accelerated Technology Inc.                          */
/*                                                                            */
/* CALLED BY                                                                  */
/*                                                                            */
/*    Applications                                                            */
/*                                                                            */
/* CALLS                                                                      */
/*                                                                            */
/*    DNS_Find_Host_By_Addr     Look up a host.                               */
/*                                                                            */
/* INPUTS                                                                     */
/*                                                                            */
/*    addr                      Pointer to the IP address of the host to find.*/
/*    len                       The length of the address.  Always 4.         */
/*    type                      The type of address.  Always NU_FAMILY_IP.    */
/*    host_entry                A pointer to a structure that will be filled  */
/*                                in with the host information.               */
/*                                                                            */
/* OUTPUTS                                                                    */
/*                                                                            */
/*    NU_SUCCESS                Indicates successful operation.               */
/*    NU_INVALID_PARM           Indicates that one of the parameters was a    */
/*                                NULL pointer.                               */
/*    NU_NOT_A_HOST             The host was not found.                       */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME                DATE        REMARKS                                 */
/*                                                                            */
/*    Glen Johnson      06/16/97      Created Initial version for NET 3.1     */
/*                                                                            */
/******************************************************************************/
STATUS NU_Get_Host_By_Addr(CHAR *addr, INT len, INT type, NU_HOSTENT *host_entry)
{
    DNS_HOST        *hst;

    /* Verify the paramaters are correct. */
    if ((!addr) || (!host_entry) || (len != 4) || (type != NU_FAMILY_IP))
        return NU_INVALID_PARM;

    if ((hst = DNS_Find_Host_By_Addr(addr)) != NU_NULL)
    {
        /* The correct entry was found. Fill in host_entry. */
        host_entry->h_name  = hst->dns_name;
        host_entry->h_alias = (char **)NU_NULL;  /* unused */
        host_entry->h_addrtype = NU_FAMILY_IP;
        host_entry->h_length = IP_ADDR_LEN;                /* unused */
        host_entry->h_addr = (char *)hst->dns_ipaddr;

        return NU_SUCCESS;
    }
    else
        return (NU_NOT_A_HOST);

} /* NU_Get_Host_By_Addr */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Get_Peer_Name                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function returns the client endposint for the specified socket. */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      User Applications                                                */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Socket_Connected                                              */
/*                                                                       */
/*   History:                                                            */
/*       created: 11/16/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
STATUS NU_Get_Peer_Name(int16 socketd, struct sockaddr_struct *peer, 
                        int16 *addr_length)
{
      int16 return_status = NU_SUCCESS;           /* initialized to success */
      
      if ( *addr_length < sizeof (struct sockaddr_struct) )
          return (NU_INVALID_PARM);

      /* check validity of socket descriptor */
      if (socketd < 0 || socketd > NSOCKETS)
           /* return an error status */
           return_status = NU_BAD_SOCKETD;
      else
      {
          /* verify that the socket has a connection to a client */
          if ( (socket_list[socketd]->foreign_addr.port_num == NU_IGNORE_VALUE) 
               || (!NU_Socket_Connected(socketd)))
               /* return error status to caller */
               return_status = NU_NOT_CONNECTED;
          else
          {
              /* store the client endposint for return to the caller */
              memcpy (&peer->ip_num, &socket_list[socketd]->foreign_addr, 4);
              
              /* store the length of the endpoint structure for return to the 
                caller */
              *addr_length = sizeof(struct sockaddr_struct);
          }
      }
      
      /* return to caller */
      return(return_status);
}  /* NU_Get_Peer_Name */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Socket_Connected                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function returns NU_TRUE if the specified socket has a foreign  */
/*  connection and NU_FALSE if it does not.                              */
/*                                                                       */
/*  CALLED FROM                                                          */
/*                                                                       */
/*      NU_Get_Peer_Name                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*   History:                                                            */
/*       created: 11/16/92, bgh                                          */
/*                                                                       */
/*************************************************************************/
STATUS NU_Socket_Connected(int16 socketd)
{
    if ((socketd < 0) || (socketd >= NSOCKETS))
        return (NU_INVALID_SOCKET);

    if (socket_list[socketd])
        return (socket_list[socketd]->s_state & SS_ISCONNECTED);
    else
        return (NU_NOT_CONNECTED);
      
} /* NU_Socket_Connected */

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "NU_Push"                   */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This functions sets the push bit for a given output port        */
/*      and as a side affect, returns whether there are any bytes       */
/*      left to be sent.                                                */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      Neil Henderson          Accelerated Technology, Inc.            */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      various                                                         */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      NU_GetPnum                                                      */
/*      netpush                                                         */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      socketd                     The socket descriptor for the       */
/*                                  port to be pushed.                  */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      bytes_remaining             Number of bytes returned by netpush */
/*                                                                      */
/************************************************************************/
STATUS NU_Push(int16 socketd)
{

    int16         pnum;
    PORT          *p;
    STATUS        ret_status = NU_SUCCESS;


#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif

    /*  Convert the input socket descriptor to a portlist number. */
    pnum = NU_GetPnum(socket_list[socketd]);

    if((pnum != NU_IGNORE_VALUE) && (NU_NULL != (p=portlist[pnum])))
        p->out.push = 1;
    else
        ret_status = -2;

#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif

    return(ret_status);
}  /* NU_Push */


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "NU_Is_Connected"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function determines if a connection has been established   */
/*      for the input socket descriptor.                                */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      Neil Henderson          Accelerated Technology, Inc.            */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      various                                                         */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      NU_GetPnum                                                      */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      socketd                     The socket descriptor for the       */
/*                                  port to be checked for connection   */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      connected                   NU_TRUE = connected, NU_FALSE = not */
/*                                                                      */
/************************************************************************/
STATUS NU_Is_Connected(int16 socketd)
{

    int16        pnum;
    int16        connected = NU_FALSE;
    struct port *prt;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS))
        return (NU_INVALID_SOCKET);

    if( (socket_list[socketd] == NU_NULL) )
        return (NU_FALSE);

    /*  Convert the input socket descriptor to a portlist number. */
    if ( (pnum = NU_GetPnum(socket_list[socketd])) < 0)
        return (NU_FALSE);

    /*  Get the address to the port.  */
    if ( (prt = portlist[pnum]) == NU_NULL)
        return NU_FALSE;

    /*  Check and see if this port is connected. */
    if (prt->state == SEST)
        connected = NU_TRUE;

    return(connected);
}  /* NU_Is_Connected */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SCK_Suspend_Task                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This function suspends the calling task.  First it releases the      */
/*  TCP Resource.  Upon resumption it grabs the TCP Resource.            */
/*                                                                       */
/* CALLED BY                                                             */
/*      NU_Accept                                                        */
/*      NU_Recv                                                          */
/*      NU_Recv_From                                                     */
/*      NU_Close_Socket                                                  */
/*      NU_Connect                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*      NU_Release_Semaphore                                             */
/*      NU_Suspend_Task                                                  */
/*      NU_Obtain_Semaphore                                              */
/*      NU_Change_Preemption                                             */
/*                                                                       */
/* INPUTS                                                                */
/*      Task_ID         Task to suspend.                                 */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     G. Johnson        05/15/95             Created Initial version.   */
/*                                                                       */
/*************************************************************************/
#ifdef PLUS
VOID SCK_Suspend_Task(NU_TASK *Task_ID)
#else
VOID SCK_Suspend_Task(sint Task_ID)
#endif
{

#ifdef PLUS
    NU_Change_Preemption(NU_NO_PREEMPT);
    NU_Release_Semaphore(&TCP_Resource);
    NU_Suspend_Task(Task_ID);
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
    NU_Change_Preemption(NU_PREEMPT);
#else
    NU_Disable_Preemption();
    NU_Release_Resource(TCP_Resource);
    NU_Stop(Task_ID);
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
    NU_Enable_Preemption();
#endif

} /* SCK_Suspend_Task */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      rip2_task                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function starts up the RIP2 task.                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
#if (INCLUDE_RIP2)
VOID rip2_task (UNSIGNED argc, VOID *argv)
{
   STATUS status;   /* return status for resource availability */

   /* Get rid of compilation warnings. */
   status =  (STATUS) argc + (STATUS) argv;

   NU_Rip2(NU_NULL);			/* no RIP2 authinacation */
}  /* rip2_task */
#endif /* INCLUDE_RIP2 */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      timer_task                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function causes netsleep() to be called every               */
/*      8 time ticks; netsleep in turn calls NET_Demux() to retrieve     */
/*      packets from the network.                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Neil Henderson, Accelerated Technology, Inc.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Release_Semaphore      Releases an instance of the specified */
/*                                 semaphore.                            */
/*      NU_Obtain_Semaphore       Obtain an instance of the specified   */
/*                                 semaphore.                            */
/*      netsleep                   Sleep while demuxing packets          */
/*      NU_Sleep                   Suspend the calling task for a        */
/*                                 number of timer ticks.                */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      N. Henderson    06-28-1994      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
#ifdef PLUS
VOID timer_task (UNSIGNED argc, VOID *argv)
#else
VOID timer_task (VOID)
#endif
{
   STATUS status;   /* return status for resource availability */
#ifdef INTERRUPT
#ifdef PLUS
   UNSIGNED  bufs_ava;
#else
   unsigned int bufs_ava;
#endif
#endif

#ifdef PLUS
   /* Get rid of compilation warnings. */
   status =  (STATUS) argc + (STATUS) argv;
#endif

   while(1)
   {
#ifdef PLUS
       status = NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
       status = NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
       /* verify that resource was available */
       if (status == NU_SUCCESS)
       {
#if (defined INTERRUPT) && !(defined PACKET_DRV)
           /* process all of the packets which came in */
           NET_Demux();
#else /* POLLED */
           /* Process a single packet at a time */
           netsleep (0);
#endif /* POLLED */

#ifdef PLUS
           NU_Release_Semaphore(&TCP_Resource);
#else
           NU_Release_Resource(TCP_Resource);
#endif
       }

#ifdef PLUS
#if (defined INTERRUPT) && !(defined PACKET_DRV)
        NU_Retrieve_Events(&Buffers_Available, 2, NU_OR_CONSUME, &bufs_ava,
                                           NU_SUSPEND);
#else
        NU_Sleep(1);
#endif
#else
#if (defined INTERRUPT) && !(defined PACKET_DRV)
        NU_Wait_For_Events(BUFFERS_AVAILABLE, NU_EVENT_OR_CONSUME, 2, &bufs_ava,
                           NU_WAIT_FOREVER);
#else
        NU_Sleep(1);
#endif
#endif

   }   /* end while for task */
}  /* end timer_task */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Abort                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function aborts a TCP or UDP connection.  In the case of TCP*/
/*      a RESET is sent to the remote host.  All resources are freed up. */
/*                                                                       */
/* CALLED BY                                                             */
/*      applications                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*      NU_Release_Semaphore                                             */
/*      NU_Obtain_Semaphore                                              */
/*      NU_GetPnum                                                       */
/*      tcp_sendack                                                      */
/*      NU_Resume_Task                                                   */
/*      TCP_Cleanup                                                      */
/*      NU_Get_UDP_Pnum                                                  */
/*      NU_Deallocate_Memory                                             */
/*      NU_TaskTable_Entry_Delete                                        */
/*                                                                       */
/* INPUTS                                                                */
/*      socketd         Socket number of the socket to abort.            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*     Maiqi Qianon      02/26/97     Created initial version.           */
/*                                                                       */
/*************************************************************************/
STATUS NU_Abort(int16 socketd)
{
    int16 port_num;                           /* local machine's port number */
    int16 return_status = NU_NO_PORT_NUMBER;  /* initialized to error status */
    uint *tempPtr;                            /* temp pointer to the given socket */
    struct port *prt;

    /*  Validate the socket number.  */
    if ((socketd < 0) || (socketd >= NSOCKETS) ||
        (socket_list[socketd] == NU_NULL))
        return(NU_INVALID_SOCKET);

    /* assume return successfully to caller */
    return_status = NU_SUCCESS;

    /*  Don't let anyone else in until we are through.  */
#ifdef PLUS
    NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else
    NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif
      
    /*  Do different processing based on the protocol type.  */
    if (socket_list[socketd]->protocol == NU_PROTO_TCP)
    {

        if ((port_num = NU_GetPnum(socket_list[socketd])) >= 0)
        {

            prt = portlist[port_num];

            /* Send a RESET to the other guy. */
            prt->tcpout.flags = TRESET;
            tcp_sendack(prt);

            /* The connection is  closed.  Cleanup. */
            TCP_Cleanup(prt);

            prt->state = SCLOSED;

#if SNMP_INCLUDED
            SNMP_tcpConnTableUpdate(SNMP_DELETE, SEST, prt->tcp_laddr,
                                  prt->in.port, prt->tcp_faddr,
                                  prt->out.port);
#endif

        }
    }
    else
    {

        port_num = NU_Get_UDP_Pnum(socket_list[socketd]);

        if (port_num >= 0)
        {

#if SNMP_INCLUDED
            /* Update the UDP Listen Table. */
            SNMP_udpListenTableUpdate(SNMP_DELETE, socket_list[socketd]->local_addr.ip_num.is_ip_addrs,
                                      socket_list[socketd]->local_addr.port_num);
#endif

#ifdef PLUS
            /*  Clear this port list entry.  */
            NU_Deallocate_Memory((uint *) uportlist[port_num]);
#else
            /*  Clear this port list entry.  */
            NU_Dealloc_Memory((unsigned int *) uportlist[port_num]);
#endif

            /*  Indicate that this port is no longer used. */
            uportlist[port_num] = NU_NULL;
        }
        else
            /*  There is no port number for this guy, bad problem.  */
            return_status = NU_NO_PORT_NUMBER;

    }

    /* delete its task table entry */
    NU_TaskTable_Entry_Delete(socketd);

    /* create a temporary pointer to the current socket descriptor */
    tempPtr = (uint *)socket_list[socketd];
          
    /* clear this socket pointer for future use */
    socket_list[socketd] = NU_NULL;
          
    /* release the memory used by this socket */
#ifdef PLUS
    /*  Clear this port list entry.  */
    NU_Deallocate_Memory((uint *) tempPtr);
#else
    /*  Clear this port list entry.  */
    NU_Dealloc_Memory((unsigned int *)tempPtr);
#endif
          
    /* allow others to use the TCP resource */
#ifdef PLUS
    NU_Release_Semaphore(&TCP_Resource);
#else
    NU_Release_Resource(TCP_Resource);
#endif
      
    /* return to caller */
    return(return_status);
} /* NU_Abort */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      NU_Add_Route                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Add a route to the routing table.                                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Applications                                                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IP_Find_Route                                                    */
/*      RTAB_Add_Route                                                   */
/*      RTAB_Free                                                        */
/*      RTAB_Set_Default_Route                                           */
/*      UTL_Zero                                                         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ip_dest                 The destination IP address for the route */
/*                                to be added.  There are three possible */
/*                                cases.                                 */
/*                                1) A host IP address - 192.200.100.13  */
/*                                2) A network address - 192.200.100.0   */
/*                                3) A NULL address for adding a default */
/*                                gateway.                               */
/*      mask                    A mask to be used with this route.       */
/*      gw_addr                 The IP address of the gateway or next hop*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      <Outputs>                           <Description>                */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
STATUS NU_Add_Route(uint8 *ip_dest, uint8* mask, uint8 *gw_addr)
{
    DV_DEVICE_ENTRY     *dev;
    RTAB_ROUTE          route;
    SCK_SOCKADDR_IP     *dest;
    uint16              flags = 0;


    /* Are the parameters valid? */
    if ((!ip_dest) || (!mask) || (!gw_addr))
        return NU_INVALID_PARM;

    UTL_Zero(&route, sizeof (route));

    /* Point to the destination. */
    dest = (SCK_SOCKADDR_IP *) &route.rt_ip_dest;
    dest->sck_family = SK_FAM_IP;
    dest->sck_len = sizeof (*dest);
    dest->sck_addr = *(uint32 *)gw_addr;
    
    /* Find a route to the gateway/next hop. In order for this to be a valid 
       route the gateway should be directly connected and thus we should be 
       able to find a route to it. */
    IP_Find_Route(&route);

    /* If a route to the gw_addr can not be found then the user is attempting to 
       add an invalid route. */
    if (route.rt_route == NU_NULL)
    {
        return NU_INVALID_PARM;
    }

    /* Free the route that was just discovered since we were really only 
       interested in the device that is associated with the route. */
    RTAB_Free(route.rt_route);

    dev = route.rt_route->rt_device;

    /* Add the new route. If the destination IP address for the route is 0, then
       a default route is being added. */
    if (*(uint32 *)ip_dest == 0)
        RTAB_Set_Default_Route(dev, *(uint32 *)gw_addr, (uint16)(RT_UP | RT_GATEWAY));
    else
    {
        flags = (RT_UP | RT_GATEWAY);

        if (*(uint32 *)mask == 0xFFFFFFFF)
            flags |= RT_HOST;

        RTAB_Add_Route( dev, *(uint32 *)ip_dest, *(uint32 *)mask, 
                        *(uint32 *)gw_addr, flags);
    }

    return (NU_SUCCESS);

} /* NU_Add_Route */
