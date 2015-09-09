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
/*      mbd.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      mailbox management component.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      MBD_Created_Mailboxes_List          Pointer to the linked-list   */
/*                                            of created mailboxes       */
/*      MBD_Total_Mailboxes                 Total number of created      */
/*                                            mailboxes                  */
/*      MBD_List_Protect                    Mailbox list protection      */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      mb_defs.h                           Mailbox Management constants */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Modified file header, resulting  */
/*                                        in version 1.1                 */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "mb_defs.h"                 /* Mailbox constants         */


/* MBD_Created_Mailboxes_List is the head pointer of the linked list of 
   created mailboxes.  If the list is NU_NULL, there are no mailboxes
   created.  */
   
CS_NODE        *MBD_Created_Mailboxes_List;


/* MBD_Total_Mailboxes contains the number of currently created 
   mailboxes.  */

UNSIGNED        MBD_Total_Mailboxes;


/* MBD_List_Protect is a list protection structure used to block any other
   thread from access to the created mailbox list.  */
   
TC_PROTECT      MBD_List_Protect;

