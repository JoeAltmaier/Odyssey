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
// This file contains the core routines for the Error management component.
//
//
// Update Log:
// 10/06/98 Jeff Nespor: Modifications previously made to R4640 source.
//                       - Added prototype for serial output routine.
//                       - Added serial output to ERC_System_Error().
/*************************************************************************/


/*************************************************************************/
// The following is the original Accelerated Technology file prologue.
/*************************************************************************/

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
/*      erc.c                                           PLUS 1.3         */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ER - Error Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Error management    */
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
/*      ERC_System_Error                    System error function        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_defs.h                           Thread control definitions   */
/*      er_extr.h                           Error handling functions     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Modified copyright notice,       */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE

#ifdef          NU_ERROR_STRING
#include        <stdio.h>                   /* Standard I/O functions    */
#endif
#include        "tc_defs.h"                 /* Thread control constants  */
#include        "er_extr.h"                 /* Error handling functions  */


/* Define external inner-component global data references.  */

extern  INT     ERD_Error_Code;

#ifdef          NU_ERROR_STRING
extern  CHAR    ERD_Error_String[];
#endif

/* Define direct access to a thread component variable.  */

extern  VOID   *TCD_Current_Thread;

/* Prototype for serial output routine.  */

void    Print_String(char *string);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_System_Error                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processes system errors detected by various        */
/*      system components.  Typically an error of this type is           */
/*      considered fatal.                                                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Various Components                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      error_code                          Code of detected system error*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*                                                                       */
/*************************************************************************/
VOID  ERC_System_Error(INT error_code)
{
#ifdef          NU_ERROR_STRING
INT     i;
CHAR   *pointer;
CHAR    name[NU_MAX_NAME+1];
#endif

    Print_String("\n\rERC_System_Error");

    /* First place the error code into the global variable.  */
    ERD_Error_Code =  error_code;
    
#ifdef          NU_ERROR_STRING
    /* Build string that corresponds to the error.  */
    switch(error_code)
    {

    case        NU_ERROR_CREATING_TIMER_HISR:
    
        /* Build string that indicates an error occurred creating the timer
           HISR.  */
        sprintf(ERD_Error_String,"%s\n", "Error Creating Timer HISR");
        break;

    case        NU_ERROR_CREATING_TIMER_TASK:
    
        /* Build string that indicates an error occurred creating the timer
           Task.  */
        sprintf(ERD_Error_String,"%s\n", "Error Creating Timer Task");
        break;

    case        NU_STACK_OVERFLOW:
    
        Print_String("\n\rStackOverflow");
    
        /* Build string that indicates a stack overflow occurred.  */
        name[NU_MAX_NAME] =  (CHAR) 0;
        pointer =  (((TC_TCB *) TCD_Current_Thread) -> tc_name);
        for (i = 0; i < NU_MAX_NAME; i++)
            name[i] =  *pointer++;
        sprintf(ERD_Error_String,"%s %s\n", "Stack Overflow in task/HISR: ",
                                                                        name);
        break;

    
    case        NU_UNHANDLED_INTERRUPT:    

        /* Build string that indicates an error occurred because of an 
           unhandled interrupt.  */
        sprintf(ERD_Error_String,"%s\n", "Unhandled interrupt error");
        break;
    }
#endif

    /* This function cannot return, since the error is fatal.  */
    while(1)
    {
    }
}

