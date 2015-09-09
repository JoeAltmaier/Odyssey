/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
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
/*      smf.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Semaphore  */
/*      Management component.                                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      SMF_Established_Semaphores          Number of created semaphores */
/*      SMF_Semaphore_Pointers              Build semaphore pointer list */
/*      SMF_Semaphore_Information           Retrieve semaphore info      */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      sm_extr.h                           Semaphore functions          */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1994      Initial version of semaphore     */
/*                                        fact service file, version 1.1 */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       11-18-1996      Protected Informational service  */
/*                                      from NULL Control Block pointers */
/*                                      creating 1.2a. (SPR220)          */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "sm_extr.h"                 /* Semaphore functions       */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE         *SMD_Created_Semaphores_List;
extern UNSIGNED         SMD_Total_Semaphores;
extern TC_PROTECT       SMD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMF_Established_Semaphores                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      semaphores.  Semaphores previously deleted are no longer         */
/*      considered established.                                          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      SMD_Total_Semaphores                Number of established        */
/*                                            semaphores                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Modified function interface,     */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  SMF_Established_Semaphores(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established semaphores.  */
    return(SMD_Total_Semaphores);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMF_Semaphore_Pointers                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of semaphore pointers, starting at   */
/*      the specified location.  The number of semaphore pointers        */
/*      placed in the list is equivalent to the total number of          */
/*      semaphores or the maximum number of pointers specified in the    */
/*      call.                                                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pointers                            Number of semaphores placed  */
/*                                            in the list                */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        08-09-1993      Corrected pointer retrieval      */
/*                                       loop, resulting in version 1.0a */
/*      D. Lamie        08-09-1993      Verified version 1.0a            */
/*      W. Lamie        03-01-1994      Modified function interface,     */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  SMF_Semaphore_Pointers(NU_SEMAPHORE **pointer_list,
                                                UNSIGNED maximum_pointers)
{
CS_NODE         *node_ptr;                  /* Pointer to each SMB       */
UNSIGNED         pointers;                  /* Number of pointers in list*/


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;
    
    /* Protect against access to the list of created semaphores.  */
    TCT_Protect(&SMD_List_Protect);

    /* Loop until all semaphore pointers are in the list or until the maximum 
       list size is reached.  */
    node_ptr =  SMD_Created_Semaphores_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {
    
        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_SEMAPHORE *) node_ptr;
        
        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;
        
        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == SMD_Created_Semaphores_List)
        
            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    } 
                
    /* Release protection against access to the list of created semaphores. */
    TCT_Unprotect();

    /* Return the number of pointers in the list.  */
    return(pointers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMF_Semaphore_Information                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified semaphore. */
/*      However, if the supplied semaphore pointer is invalid, the       */
/*      function simply returns an error status.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect semaphore            */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Pointer to the semaphore     */
/*      name                                Destination for the name     */
/*      current_count                       Destination for semaphore    */
/*                                            instance count             */
/*      suspend_type                        Destination for the type of  */
/*                                            suspension                 */
/*      tasks_waiting                       Destination for the tasks    */
/*                                            waiting count              */
/*      first_task                          Destination for the pointer  */
/*                                            to the first task waiting  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid semaphore pointer */
/*                                            is supplied                */
/*      NU_INVALID_SEMAPHORE                If semaphore pointer invalid */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Modified function interface,     */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M. Trippi       11-18-1996      Corrected SPR220.                */
/*                                                                       */
/*************************************************************************/
STATUS  SMF_Semaphore_Information(NU_SEMAPHORE *semaphore_ptr, CHAR *name, 
                  UNSIGNED *current_count, OPTION *suspend_type, 
                  UNSIGNED *tasks_waiting, NU_TASK **first_task)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this semaphore id is valid.  */
    if ((semaphore != NU_NULL) && (semaphore -> sm_id == SM_SEMAPHORE_ID))
    {
    
        /* Setup protection of the semaphore.  */
        TCT_System_Protect();

        /* The semaphore pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;
        
        /* Copy the semaphore's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  semaphore -> sm_name[i];

        /* Determine the suspension type.  */
        if (semaphore -> sm_fifo_suspend)
            *suspend_type =          NU_FIFO;
        else
            *suspend_type =          NU_PRIORITY;
            
        /* Return the current semaphore available instance count.  */
        *current_count =  semaphore -> sm_semaphore_count;
        
        /* Retrieve the number of tasks waiting and the pointer to the 
           first task waiting.  */
        *tasks_waiting =  semaphore -> sm_tasks_waiting;
        if (semaphore -> sm_suspension_list)
    
            /* There is a task waiting.  */
            *first_task = (NU_TASK *) 
                (semaphore -> sm_suspension_list) -> sm_suspended_task;
        else
        
            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;
    
        /* Release protection of the semaphore.  */
        TCT_Unprotect();
    }
    else
    
        /* Indicate that the semaphore pointer is invalid.   */
        completion =  NU_INVALID_SEMAPHORE;

    /* Return the appropriate completion status.  */
    return(completion);
}

