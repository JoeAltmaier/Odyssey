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
/*      dmd.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Dynamic Memory Management component.                             */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      DMD_Created_Pools                   Pointer to the linked-list   */
/*                                            of created dynamic pools   */
/*      DMD_Total_Pools                     Total number of created      */
/*                                            dynamic pools              */
/*      DMD_List_Protect                    Dynamic pool list protect    */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      dm_defs.h                           Dynamic memory constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Modified copyright, resulting in */
/*                                        version 1.1                    */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "dm_defs.h"                 /* Dynamic memory constants  */


/* DMD_Created_Pools_List is the head pointer of the linked list of 
   created dynamic memory pools.  If the list is NU_NULL, there are no 
   dynamic memory pools created.  */
   
CS_NODE        *DMD_Created_Pools_List;


/* DMD_Total_Pools contains the number of currently created 
   dynamic memory pools.  */

UNSIGNED        DMD_Total_Pools;


/* DMD_List_Protect is a list protection structure used to block any other
   thread from access to the created dynamic memory pool list.  */
   
TC_PROTECT      DMD_List_Protect;

