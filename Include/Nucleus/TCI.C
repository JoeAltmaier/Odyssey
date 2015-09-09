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
/*      tci.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for this           */
/*      component.                                                       */
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
/*      TCI_Initialize                      Thread Control Initialization*/
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        08-09-1993      Corrected problem initializing   */
/*                                        the LISR association table,    */
/*                                        resulting in version 1.0a      */
/*      D. Lamie        08-09-1993      Verified version 1.0a            */
/*      W. Lamie        03-01-1994      Changed to initialize the system */
/*                                        protection rather than the     */
/*                                        schedule protection, resulting */
/*                                        in version 1.1                 */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*      Sudhir Kasargod 06/14/99        Added Init for the Variable      */
/*                                      TCD_R7KInterrupt_Level           */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_defs.h"                 /* Common Service constants  */
#include        "tc_defs.h"                 /* Thread Control constants  */

/* Define external inner-component global data references.  */

extern CS_NODE         *TCD_Created_Tasks_List;
extern UNSIGNED         TCD_Total_Tasks;
extern TC_TCB          *TCD_Priority_List[TC_PRIORITIES];
extern UNSIGNED         TCD_Priority_Groups;
extern DATA_ELEMENT     TCD_Sub_Priority_Groups[TC_MAX_GROUPS];
extern INT              TCD_Highest_Priority;
extern TC_TCB          *TCD_Execute_Task;
extern VOID            *TCD_Current_Thread;
extern VOID            *TCD_Protect_Thread;
extern UNSIGNED_CHAR    TCD_Registered_LISRs[NU_MAX_VECTORS];
extern VOID           (*TCD_LISR_Pointers[NU_MAX_LISRS])(INT vector);
extern INT              TCD_Interrupt_Count;
extern INT              TCD_Stack_Switched;
extern TC_PROTECT       TCD_List_Protect;
extern TC_PROTECT       TCD_System_Protect;
extern TC_PROTECT       TCD_LISR_Protect;
extern CS_NODE         *TCD_Created_HISRs_List;
extern UNSIGNED         TCD_Total_HISRs;
extern TC_HCB          *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES];
extern TC_HCB          *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES];
extern TC_HCB          *TCD_Execute_HISR;
extern TC_PROTECT       TCD_HISR_Protect;
extern INT              TCD_Interrupt_Level;
extern INT              TCD_R7KInterrupt_Level;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the TC component.  The system is initialized as     */
/*      idle.                                                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INC_Initialize                      System initialization        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      TCD_Created_Tasks_List              List of created tasks        */
/*      TCD_Total_Tasks                     Number of created tasks      */
/*      TCD_Priority_List                   Ready task array             */
/*      TCD_Priority_Groups                 Ready priority group bit map */
/*      TCD_Sub_Priority_Groups             Sub-priority groups bit map  */
/*      TCD_Highest_Priority                Highest priority task        */
/*      TCD_Execute_Task                    Top priority task to execute */
/*      TCD_Created_HISRs_List              List of created HISRs        */
/*      TCD_Total_HISRs                     Number of created HISRs      */
/*      TCD_Active_HISR_Heads               Active HISR list head ptrs   */
/*      TCD_Active_HISR_Tails               Active HISR list tail ptrs   */
/*      TCD_Execute_HISR                    Top priority HISR to execute */
/*      TCD_Current_Thread                  Current thread pointer       */
/*      TCD_Registered_LISRs                Registered LISRs list        */
/*      TCD_LISR_Pointers                   Pointers to active LISRs     */
/*      TCD_Interrupt_Count                 Interrupt in progress counter*/
/*      TCD_Stack_Switched                  Interrupt stack switched flag*/
/*      TCD_List_Protect                    Protection of task list      */
/*      TCD_Schedule_Protect                Protection of scheduling     */
/*                                            data structures.           */
/*      TCD_HISR_Protect                    Protection of created HISR   */
/*                                            list                       */
/*      TCD_Interrupt_Level                 Enable interrupt level       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        08-09-1993      Corrected problem initializing   */
/*                                        the LISR association table,    */
/*                                        resulting in version 1.0a      */
/*      D. Lamie        08-09-1993      Verified version 1.0a            */
/*      W. Lamie        03-01-1994      Changed to initialize the system */
/*                                        protection rather than the     */
/*                                        schedule protection, resulting */
/*                                        in version 1.1                 */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/
VOID  TCI_Initialize(VOID)
{

int             i;                          /* Working index variable    */


    /* Initialize the created task list to NU_NULL.  */
    TCD_Created_Tasks_List =  NU_NULL;
    
    /* Initialize the total number of created tasks to 0.  */
    TCD_Total_Tasks =  0;
    
    /* Initialize the array of ready task lists.  */
    for (i = 0; i < TC_PRIORITIES; i++)
        TCD_Priority_List[i] =  0;
        
    /* Initialize the bit maps that represent that at least one task is
       ready at the same priority level.  */
    TCD_Priority_Groups =  0;
    
    for (i = 0; i < TC_MAX_GROUPS; i++)
        TCD_Sub_Priority_Groups[i] =  0;
        
    /* Initialize the highest priority flag.  */
    TCD_Highest_Priority =  TC_PRIORITIES;
    
    /* Initialize pointers to the task to execute, HISR to execute, 
       and current thread of execution.  */
    TCD_Execute_Task =          NU_NULL;
    TCD_Execute_HISR =          NU_NULL;
    TCD_Current_Thread =        NU_NULL;

    /* Initialize the created HISRs list to NU_NULL.  */
    TCD_Created_HISRs_List =  NU_NULL;
    
    /* Initialize the total number of created HISRs to 0.  */
    TCD_Total_HISRs =  0;
    
    /* Initialize the array of ready HISR list pointers.  */
    for (i = 0; i < TC_HISR_PRIORITIES; i++)
    {
        TCD_Active_HISR_Heads[i] =  NU_NULL;
        TCD_Active_HISR_Tails[i] =  NU_NULL;
    }

    /* Initialize the LISR interrupt control data structures.  */
    for (i = 0; i < NU_MAX_VECTORS; i++)
        TCD_Registered_LISRs[i] =  NU_FALSE;
        
    for (i = 0; i < NU_MAX_LISRS; i++)
        TCD_LISR_Pointers[i] =  NU_NULL;

    /* Initialize the interrupt processing variables.  */
    TCD_Interrupt_Count =  0;
    TCD_Stack_Switched =   0;
    
    /* Initialize the task control protection structures.  */
    TCD_List_Protect.tc_tcb_pointer =           NU_NULL;
    TCD_System_Protect.tc_tcb_pointer =         NU_NULL;
    TCD_LISR_Protect.tc_tcb_pointer =           NU_NULL;
    TCD_HISR_Protect.tc_tcb_pointer =           NU_NULL;

    /* Initialize the interrupt level to enable all interrupts.  */
	set_interrupt_level();
}

