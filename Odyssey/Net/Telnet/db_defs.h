/*************************************************************************/
/*                                                                       */
/*            Copyright (c) 1998 Accelerated Technology, Inc.            */
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
/*      db_defs.c                                       DBUG+  1.1       */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DEBUG       Nucleus PLUS Debugger                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*    This file contains Nucleus debugger constants.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      David L. Lamie, Accelerated Technology, Inc.                     */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      D. Lamie        07-15-1993      Created initial version 1.0      */
/*      D. Lamie        08-22-1993      Created version 1.0a             */
/*      D. Lamie        09-15-1994      Created version 1.1              */
/*                                                                       */
/*      R. Pfaff        09-16-1994      Verified version 1.1             */
/*                                                                       */
/*************************************************************************/

#define COL            80                   /* Maximum column size       */
#define ROW            24                   /* Maximum rows on screen    */
#define BS             0x08                 /* Back space value in HEX   */
#define CR             0x0D                 /* Return value in HEX       */
#define NUL            0
#define MAX_NAME       8
#define MAX_ITEMS      20

#define DUPLEX         1                    /* FULL DUPLEX = 1;
                                               HALF DUPLEX = 0           */
#define DB_TRUE        1                    /* Constant for true         */
#define DB_FALSE       0                    /* Constant for false        */
#define DB_SUCCESS     1                    /* Successful completion     */
#define DB_ERROR       0                    /* Error in attempt          */

#define DB_ITEM_SIZE   10                   /* Maximum words per item 
                                               when making NU_Send_Item
                                               or NU_Retrieve_Item calls */

#define DB_BUFFER_SIZE 2000                 /* Size of structure buffer
                                               used in DBC.  Note that 
                                               size must be greater 
                                               maximum Nucleus struct and
                                               greater than any queue 
                                               item size.  Size is in 
                                               bytes.  */
#define TASK                       1
#define MAILBOX                    2
#define QUEUE                      3
#define PIPE                       4
#define SEMAPHORE                  5
#define EVENT                      6
#define SIGNAL                     7
#define TIMER                      8
#define PARTITION                  9
#define MEMORY                     10

#define TS                         0         /* Task Status              */
#define MS                         1         /* Mailbox Status           */
#define QS                         2         /* Queue Status             */
#define PS                         3         /* Partition Status         */
#define SS                         4         /* Semaphore Status         */
#define ES                         5         /* Event Status             */
#define SI                         6         /* Signal Status            */
#define TI                         7         /* Timer Status             */
#define PM                         8         /* Partition Memory         */
#define DM                         9         /* Dynamic Memory Status    */
#define M                          10        /* Display Memory (long)    */
#define SM                         11        /* Set Memory (long)        */
#define HELP                       12        /* Help request             */
#define NU_RESUME_TASK             13        /* Nucleus Service Call     */
#define NU_SUSPEND_TASK            14        /* Nucleus Service Call     */
#define NU_TERMINATE_TASK          15        /* Nucleus Service Call     */
#define NU_RESET_TASK              16        /* Nucleus Service Call     */
#define NU_CHANGE_PRIORITY         17        /* Nucleus Service Call     */
#define NU_BROADCAST_TO_MAILBOX    18        /* Nucleus Service Call     */
#define NU_RECEIVE_FROM_MAILBOX    19        /* Nucleus Service Call     */
#define NU_RESET_MAILBOX           20        /* Nucleus Service Call     */
#define NU_SEND_TO_MAILBOX         21        /* Nucleus Service Call     */
#define NU_BROADCAST_TO_QUEUE      22        /* Nucleus Service Call     */
#define NU_RECEIVE_FROM_QUEUE      23        /* Nucleus Service Call     */
#define NU_RESET_QUEUE             24        /* Nucleus Service Call     */
#define NU_SEND_TO_FRONT_OF_QUEUE  25        /* Nucleus Service Call     */
#define NU_SEND_TO_QUEUE           26        /* Nucleus Service Call     */
#define NU_BROADCAST_TO_PIPE       27        /* Nucleus Service Call     */
#define NU_RECEIVE_FROM_PIPE       28        /* Nucleus Service Call     */
#define NU_RESET_PIPE              29        /* Nucleus Service Call     */
#define NU_SEND_TO_FRONT_OF_PIPE   30        /* Nucleus Service Call     */
#define NU_SEND_TO_PIPE            31        /* Nucleus Service Call     */
#define NU_OBTAIN_SEMAPHORE        32        /* Nucleus Service Call     */
#define NU_RELEASE_SEMAPHORE       33        /* Nucleus Service Call     */
#define NU_RESET_SEMAPHORE         34        /* Nucleus Service Call     */
#define NU_RETRIEVE_EVENTS         35        /* Nucleus Service Call     */
#define NU_SET_EVENTS              36        /* Nucleus Service Call     */
#define NU_SEND_SIGNALS            37        /* Nucleus Service Call     */
#define NU_CONTROL_TIMER           38        /* Nucleus Service Call     */
#define NU_RESET_TIMER             39        /* Nucleus Service Call     */
#define NU_RETRIEVE_CLOCK          40        /* Nucleus Service Call     */
#define NU_SET_CLOCK               41        /* Nucleus Service Call     */
#define NU_ALLOCATE_PARTITION      42        /* Nucleus Service Call     */
#define NU_DEALLOCATE_PARTITION    43        /* Nucleus Service Call     */
#define NU_ALLOCATE_MEMORY         44        /* Nucleus Service Call     */
#define NU_DEALLOCATE_MEMORY       45        /* Nucleus Service Call     */


