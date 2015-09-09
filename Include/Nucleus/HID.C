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
/*      hid.c                                           PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      History Management component.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      HID_History_Enable                  History saving enabled flag  */
/*      HID_Write_Index                     Current write index into     */
/*                                            history table              */
/*      HID_Read_Index                      Current read index into      */
/*                                            history table              */
/*      HID_Entry_Count                     Number of entries in the     */
/*                                            table counter              */
/*      HID_History_Protect                 History protection           */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      hi_defs.h                           History Management constants */
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

#include        "hi_defs.h"                 /* History constants         */


/* HID_History_Enable is a flag that indicates whether or not history saving
   is enabled.  If this value is NU_FALSE, history saving is disabled.  
   Otherwise, history saving is enabled.  */
   
INT     HID_History_Enable;


/* HID_Write_Index is the index of the next available entry in the history 
   table.  */
   
INT     HID_Write_Index;


/* HID_Read_Index is the index of the oldest entry in the history table.  */

INT     HID_Read_Index;


/* HID_Entry_Count keeps track of the number of entries currently
   in the history table.  */
   
INT     HID_Entry_Count;


/* HID_History_Protect is a protection structure used to block any other
   thread from access to the history data structures.  */
   
TC_PROTECT      HID_History_Protect;

