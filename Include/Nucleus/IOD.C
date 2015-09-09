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
/*      iod.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Input/Output Driver Management component.                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      IOD_Created_Drivers_List            Pointer to the linked-list   */
/*                                            of created I/O drivers     */
/*      IOD_Total_Drivers                   Total number of created      */
/*                                            I/O drivers                */
/*      IOD_List_Protect                    I/O driver list protection   */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      io_defs.h                           I/O Driver Management consts */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Updated copyright notice,        */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "io_defs.h"                 /* I/O Driver constants      */
#include        "tc_defs.h"                 /* Thread control constants  */


/* IOD_Created_Drivers_List is the head pointer of the linked list of 
   created I/O drivers.  If the list is NU_NULL, there are no I/O drivers
   created.  */
   
CS_NODE        *IOD_Created_Drivers_List;


/* IOD_Total_Drivers contains the number of currently created 
   I/O drivers.  */

UNSIGNED        IOD_Total_Drivers;


/* IOD_List_Protect is a list protection structure used to block any other
   thread from access to the created drivers list.  */
   
TC_PROTECT      IOD_List_Protect;

