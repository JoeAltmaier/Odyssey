/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993 - 1998 by Accelerated Technology, Inc.           */
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
/*      sockext                                               4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will handle all the external references for the   */
/*      socket structures.                                                  */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Craig L. Meredith, Accelerated Technology Inc.                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  global compenent data stuctures defined in this file                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      No functions defined in this file                                   */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      No other file dependencies                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Craig L. Meredith       04/10/93        Initial version.            */
/*      Craig L. Meredith       08/17/93        Added header, Neil's mods.  */
/*                                                                          */
/****************************************************************************/

#ifndef SOCKEXT_H
#define SOCKEXT_H

#include "sockdefs.h"    /* socket definitions */

/* array allocated for holding the pointers to the current sockets */
extern struct sock_struct *socket_list[NSOCKETS];

/* task table structure - created during an NU_Listen call to
   store status on x number of connections for a single port number
   from a single task id */

/* pointer to the first task structure */
extern struct TASK_TABLE_STRUCT *Task_Head;
/* pointer to the last task structure */
extern struct TASK_TABLE_STRUCT *Task_Tail;
#endif	/* SOCKEXT_H */
