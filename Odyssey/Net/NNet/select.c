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
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  select.c                                                  3.2           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This file contains those functions associated with the NU_Select        */
/*  service.                                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qina,           Accelerated Technology Inc.                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*     NU_Select                                                            */
/*     NU_FD_Check                                                          */
/*     NU_FD_Set                                                            */
/*     NU_FD_Init                                                           */
/*     NU_FD_Reset                                                          */
/*     SEL_Check_Recv                                                       */
/*     SEL_Setup_Recv_Ports                                                 */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME              DATE        REMARKS                                 */
/*                                                                          */
/*    MQ Qina         10/06/95      Initial version.                        */
/*    Glen Johnson    07/11/97      Added the ability to select on Accept.  */
/*                                                                          */
/****************************************************************************/
#ifdef PLUS
  #include "nucleus.h"
#else /* PLUS */
  #include "nu_defs.h"
  #include "nu_extr.h"
#endif /* !PLUS */
#include "target.h"
#include "protocol.h"
#include "tcpdefs.h"
#include "socketd.h"
#include "sockext.h"
#include "netevent.h"
#include "externs.h"
#include "data.h"

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_Select                                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This fuction allows an application to check for data on multiple        */
/*  sockets.  Alternatively multiple server sockets (those that are         */
/*  listening for connections) can be checked for established conections.   */
/*  The calling application can choose to return immediatley, suspend, or   */
/*  specify a timeout.                                                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian, Accelerated Technology Inc.                                    */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      DNS_Query                                                           */
/*      NU_Rip2                                                             */
/*      Applications                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      SEL_Check_Recv                                                      */
/*      SEL_Setup_Recv_Ports                                                */
/*      NU_Obtain_Semaphore                                                 */
/*      NU_Release_Semaphore                                                */
/*      UTL_Timerset                                                        */
/*      UTL_Timerunset                                                      */
/*      NU_Current_Task_Pointer                                             */
/*      NU_Suspend_Task                                                     */
/*      NU_Change_Preemption                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*      max_sockets                 Maximum socket to check.                */
/*      readfs                      A bit field indicating which sockets    */
/*                                    to check for data.                    */
/*      writefs                     Not currently used.                     */
/*      exceptfs                    A bit field indicating which sockets    */
/*                                    to check for conections.              */
/*      timeout                     Indicates the timeout desired.  Either  */
/*                                    NU_SUSPEND, NU_NO_SUSPEND, or a       */
/*                                    timeout value.                        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      NU_SUCCESS                  Indicates successfull completion        */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                   DATE        REMARKS                              */
/*                                                                          */
/*  MQ Qina              10/12/94      Initial version.                     */
/*  MQ Qina              11/07/94      Last modification date               */
/*  Glen Johnson         11/16/95      Modified so that a UDP socket can be */
/*                                     selected on.  Modularized the code.  */
/*                                                                          */
/****************************************************************************/
#ifdef PLUS
STATUS NU_Select(INT max_sockets,
            FD_SET *readfs, FD_SET *writefs, FD_SET *exceptfs, UNSIGNED timeout)
#else
STATUS NU_Select(INT max_sockets,
            FD_SET *readfs, FD_SET *writefs, FD_SET *exceptfs, sint timeout)
#endif
{
    int16 read_flag = 0;
    int16 i, j=0;
    STATUS return_status = NU_NO_DATA;  /* initialized to error status */
    NU_TASK     *Task_ID;

    /*  Clean up warnings.  This parameter is used for socket compatibility
       but we are currently not making any use of it.  */
    Task_ID = (NU_TASK *)writefs;
    Task_ID = (NU_TASK *)exceptfs;

    if((max_sockets == 0) || (max_sockets > NSOCKETS))
        return(NU_NO_SOCKETS);


    if ( readfs == NU_NULL )
         return(NU_NO_SOCKETS);


    /* Make sure that at least one bit is set. */
    for(i=0; i<FD_ELEMENTS; i++)
    {

        if ((uint32)readfs && ((uint32)readfs->words[i] != 0))
        {
            read_flag = 1;
        }
    }

    /* Was at least one bit found to be set in the read group. */
    if ( !read_flag )        
        return(NU_NO_SOCKETS);


    while (1)
	{
        /*  Don't let anyone else in until we are through.  */
#ifdef PLUS                                                                     /* necessary ? */
		NU_Obtain_Semaphore(&TCP_Resource, NU_SUSPEND);
#else  /* PLUS */
		NU_Request_Resource(TCP_Resource, NU_WAIT_FOREVER);
#endif /* !PLUS */

        /* Check to see if any of the sockets are data ready.  If so readfs will
           be returned with only the bits associated with the ready sockets set.
           Else readfs will be returned with no bits changed.
        */
        return_status = SEL_Check_Recv(max_sockets, readfs);
        
        /* If at least one socket was data ready then break out of this while
           loop and return.  Break also if suspension was not desired, or if j =
           1.  Note that j is only set to one if a timeout is desired but not
           unconditional suspension.  If unconditional suspension was specified
           then we will only return when a data ready socket is found, even if
           this task is some how inadvertantly woken up.
         */
#ifdef PLUS
        if ( (return_status == NU_SUCCESS) || (timeout == NU_NO_SUSPEND) || j)
#else
        if ( (return_status == NU_SUCCESS) || (timeout == NU_NO_TIMEOUT) || j)
#endif
			break;

        /* Go ahead and retrieve the current task pointer once.  It is used in
           several places below. */
        Task_ID = NU_Current_Task_Pointer();

        return_status = -1;

        if(read_flag)
#ifdef PLUS
            return_status = SEL_Setup_Recv_Ports(max_sockets, readfs, Task_ID);
             
#else
            return_status = SEL_Setup_Recv_Ports(max_sockets, readfs,
                                               NU_Current_Task_ID());
#endif
        /* If we did not successfully setup a recv port then get out.
           There is no point in continuing. */
        if ( return_status != NU_SUCCESS )
            break;

#ifdef PLUS
        if (timeout != NU_SUSPEND)
#else
        if (timeout != NU_WAIT_FOREVER)
#endif
        {
            /* Set up the timer to wake us up if the event never occurs. */
            UTL_Timerset(SELECT, (UNSIGNED)Task_ID, timeout, 0);            
            j = 1;
        }

        /* Let others in while we are waiting for the connection. */
#ifdef PLUS
        NU_Change_Preemption(NU_NO_PREEMPT);
        NU_Release_Semaphore(&TCP_Resource);

        /* suspend the current task until data ready */        
        NU_Suspend_Task(Task_ID);
        NU_Change_Preemption(NU_PREEMPT);
#else
        NU_Release_Resource(TCP_Resource);

        /* suspend the current task until data ready */
        NU_Stop(NU_Current_Task_ID());
#endif

#ifdef PLUS
        if (timeout != NU_SUSPEND)
#else
        if (timeout != NU_WAIT_FOREVER)
#endif
        {
            /* At this point there is no way to tell if we were resumed because
               of a timeout or because the event we are waitting on ocurred.  In
               the former case the event will already be cleared.  Try to clear
               it here anyway. */
            
            UTL_Timerunset(SELECT, (UNSIGNED)Task_ID, (int32)1);
        }

        /* We need to clean these RXTask's after waking up */
        if (read_flag)
            SEL_Setup_Recv_Ports(max_sockets, readfs, NU_NULL);

	} /* while(1) */

	/*  Let others in while we are waiting for the connection.  */
#ifdef PLUS
	NU_Release_Semaphore(&TCP_Resource);
#else
	NU_Release_Resource(TCP_Resource);
#endif


    return return_status;

} /*  end of NU_Select  */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_FD_Check                                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This fuction will check to see if a particular bit has been set in a    */
/*  bit map.                                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian, Accelerated Technology Inc.                                    */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      User Applications                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  No functions called by this function                                    */
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
/*  MQ Qina              10/12/94      Initial version.                     */
/*  MQ Qina              11/07/94      Last modification date               */
/*                                                                          */
/****************************************************************************/

INT NU_FD_Check(INT socket, FD_SET *fd)
{
        if (fd->words[socket/FD_BITS] & ((uint32)NU_TRUE<<((uint32)socket%(uint32)FD_BITS)))
		return(NU_TRUE);
	else
		return(NU_FALSE);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_FD_Set                                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sets a bit in a bit map.                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian, Accelerated Technology Inc.                                    */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      User Applications                                                   */
/*			str_echo                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  No functions called by this function                                    */
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
/*  MQ Qina              10/12/94      Initial version.                     */
/*  MQ Qina              11/07/94      Last modification date               */
/*                                                                          */
/****************************************************************************/
VOID NU_FD_Set(int16 socket, FD_SET *fd)
{

        fd->words[socket/FD_BITS] |= (uint32)NU_TRUE<<((uint32)socket%(uint32)FD_BITS);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_FD_Init                                                              */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Sets all bits in a bit map to 0.                                        */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian, Accelerated Technology Inc.                                    */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      User Applications                                                   */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  No functions called by this function                                    */
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
/*  MQ Qina              10/12/94      Initial version.                     */
/*  MQ Qina              11/07/94      Last modification date               */
/*                                                                          */
/****************************************************************************/
VOID NU_FD_Init(FD_SET *fd)
{
    INT i;

	for (i=0; i<FD_ELEMENTS; fd->words[i++]=0L);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  NU_FD_Reset                                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Resets a bit in a bit map.                                              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  MQ Qian, Accelerated Technology Inc.                                    */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      User Applications                                                   */
/*			str_echo                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*  No functions called by this function                                    */
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
/*  MQ Qina              10/12/94      Initial version.                     */
/*  MQ Qina              11/07/94      Last modification date               */
/*                                                                          */
/****************************************************************************/
VOID NU_FD_Reset(INT socket, FD_SET *fd)
{
	fd->words[socket/FD_BITS] &= ~(NU_TRUE<<(socket%FD_BITS));
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  SEL_Check_Recv                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  Checks the sockets specified in a bitmap to see if any are data ready.  */
/*  If any are found to be data ready the bitmap is modified so that only   */
/*  the bits for the data ready sockets are set.  Else the bitmap is        */
/*  untouched.                                                              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  Glen Johnson, Accelerated Technology Inc.                               */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      NU_Select                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_FD_Init                                                          */
/*      NU_FD_Check                                                         */
/*      NU_FD_Reset                                                         */
/*      NU_GetPnum                                                          */
/*      NU_Get_UDP_Pnum                                                     */
/*      makeuport                                                           */
/*      NU_SearchTaskList                                                   */
/*      NU_Current_Task_Pointer                                             */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*      max_sockets          The max number of sockets that can be open.    */
/*      readfs               Bitmap of sockets to check for data.           */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      NU_SUCCES if at least one socket is data ready, NU_NO_DATA otherwise*/
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                   DATE        REMARKS                              */
/*                                                                          */
/*  Glen Johnson         11/16/95      Initial version.                     */
/*                                                                          */
/****************************************************************************/
STATUS SEL_Check_Recv(INT max_sockets, FD_SET *readfs)
{
    INT    i;                   /* Loop counter */
    STATUS return_status = NU_NO_DATA;  /* Assume failure, if a data ready
                                           socket is found status will be updated
                                           to success. */
    FD_SET  tmpfs;
    struct sock_struct *sockptr; /* pointer to current socket */
    int16 pnum;                  /* local machine's port number */
    struct uport   *uprt;
#ifdef PLUS
    NU_TASK     *Task_ID;        /* current task pointer */
#else
    sint         Task_ID;        /* current task id */
#endif
    uint16       server_port;
    struct TASK_TABLE_STRUCT   *Task_Entry = NU_NULL;


    /* Preserve the current state of readfs.  It may be needed below. */
    NU_FD_Init(&tmpfs);
    memcpy(&tmpfs, readfs, sizeof(FD_SET));

    for (i = 0; i < max_sockets; i++)
    {
        /* Is the bit for socket i set. */
        if (NU_FD_Check(i, readfs)==NU_NULL)
            continue;

        /*  Pick up a pointer to the socket list. */
        if ((sockptr = socket_list[i]) == NU_NULL)
            continue;

        if (sockptr->protocol == NU_PROTO_TCP)
        {
            /* Check to see if this is a server socket or communication
               socket. */
            if ( !(sockptr->s_flags & SF_LISTENER))
            {
                /* Check to see if there is received data. At this point it does 
                   not matter if the connection has been closed. If there is 
                   received data then it should be passed to the application 
                   layer. 
				*/
				if (sockptr->s_recvbytes == 0)
                {
                    NU_FD_Reset(i, readfs);
                    continue;
                }

                /* verify there are data in and no other task using this socket
                   and make sure no other process accessing it now ! */
#ifdef PLUS                                                            
                if (sockptr->s_RXTask == NU_NULL && (sockptr->s_recvbytes >0))
#else  /* PLUS */
                if (sockptr->s_RXTask == NU_SYSTEM || (sockptr->s_recvbytes >0))
#endif /* !PLUS */
                    return_status = NU_SUCCESS;
                else
                    NU_FD_Reset(i, readfs);
            }
            else
            {
                /* This is a server socket, acepting connections. */
                /* get the current task id for the next 2 service calls */
#ifdef PLUS
                Task_ID = NU_Current_Task_Pointer();
#else
                Task_ID = NU_Current_Task_ID();
#endif

                /* retrieve the local port number from the socket descriptor
                   for comparison to the task table */
                server_port = sockptr->local_addr.port_num;

                /* search the task table for this port number/task id */
                if ( NU_SearchTaskList(Task_Entry, Task_ID, server_port,
                                        SEST, -1) != NU_NULL)
                    return_status = NU_SUCCESS;
                else
                    NU_FD_Reset(i, readfs);
            }

        } /* end this is a TCP socket. */
        else if (sockptr->protocol == NU_PROTO_UDP)
        {
            if(!sockptr->local_addr.port_num)
            {
                NU_FD_Reset(i, readfs);
                continue;
            }

            if((pnum = NU_Get_UDP_Pnum(sockptr)) == NU_IGNORE_VALUE)
            {
                if ((pnum = makeuport(sockptr->local_addr.port_num, i)) < 0)
                {
                    NU_FD_Reset(i, readfs);
                    continue;
                }
            }

            uprt = uportlist[pnum];

            if( (uprt->RXTask == NU_NULL) && (uprt->in_dgrams))
                return_status = NU_SUCCESS;
            else
                NU_FD_Reset(i, readfs);

        }  /* end this is a UDP socket */
    } /* for (i=0; i<max_sockets; i++) */

    if(return_status != NU_SUCCESS)
        memcpy(readfs, &tmpfs, sizeof(FD_SET));

    return(return_status);

} /* end SEL_Check_Recv */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  SEL_Setup_Recv_Ports                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function marks the specified ports so that if data arrives on any  */
/*  of them the current task will be resumed.                               */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  Glen Johnson, Accelerated Technology Inc.                               */
/*                                                                          */
/*  CALLED FROM                                                             */
/*                                                                          */
/*      NU_Select                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*      NU_FD_Check                                                         */
/*      NU_Get_UDP_Pnum                                                     */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*      Returns the socket index of the last socket setup.                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME                   DATE        REMARKS                              */
/*                                                                          */
/*  Glen Johnson         11/16/95      Initial version.                     */
/*                                                                          */
/****************************************************************************/
#ifdef PLUS
STATUS SEL_Setup_Recv_Ports(INT max_sockets, FD_SET *readfs, NU_TASK *Task_ID)
#else
INT SEL_Setup_Recv_Ports(INT max_sockets, FD_SET *readfs, int16 Task_ID)
#endif
{
    INT     i;
    int16   pnum;                  /* local machine's port number */
    struct sock_struct *sockptr; /* pointer to current socket */
    struct uport   *uprt;
    STATUS          return_status = -1;
    INT                         server_port;
    struct TASK_TABLE_STRUCT    *Task_Entry = NU_NULL;

    /* we need to assign the current thread to each (!!!) port
        we are checking, then NU_EventsDispatcher() could wake up
        this task when any of these ports gets data */
    for (i = 0; i < max_sockets; i++)
    {
        if (NU_FD_Check(i, readfs)==NU_NULL)
            continue;

        if ((sockptr = socket_list[i]) == NU_NULL)
            continue;

        if (sockptr->protocol == NU_PROTO_TCP)
        {
            /* Check to see if this is a server socket or communication
               socket. */
            if ( !(sockptr->s_flags & SF_LISTENER) )
            {
                /* Check to see if data can still be received on the port. */
                if ( sockptr->s_state == SS_ISCONNECTED )
                {
                    sockptr->s_RXTask = Task_ID;
                    return_status = NU_SUCCESS;
                }
            }
            else
            {
                /* retrieve the local port number from the socket descriptor
                   for comparison to the task table */
                server_port = sockptr->local_addr.port_num;

                /* start at the head of the list */
                Task_Entry = Task_Head;

                /* verify that the list is not empty */
                while (Task_Entry != NU_NULL)
                {
                    /* discontinue searching if a match is found */
                    if ((Task_Entry->Task_ID == Task_ID) &&
                      (Task_Entry->local_port_num == server_port))
                    {
                        break;
                    }

                    /* continue checking the next structure */
                    Task_Entry = Task_Entry->next;
                }

                /* If an entry was found, set it up. */
                if (Task_Entry)
                {
                    /* Should the accept flag be set or cleared. */
                    if (Task_ID)
                        Task_Entry->acceptFlag = 1;
                    else
                        Task_Entry->acceptFlag = 0;

                    /* Setup the Task ID field so that this task can be resumed. */
                    Task_Entry->Task_ID = Task_ID;

                    return_status = NU_SUCCESS;
                }
            }
        }
        else if(sockptr->protocol == NU_PROTO_UDP)
        {
            if ((pnum = NU_Get_UDP_Pnum(sockptr)) == NU_IGNORE_VALUE)
                continue;

            if ((uprt = uportlist[pnum]) == NU_NULL)
                continue;

            uprt->RXTask = Task_ID;
            return_status = NU_SUCCESS;
        }
    } /* end for i to max_sockets */

    return (return_status);
} /* SEL_Setup_Recv_Ports */
