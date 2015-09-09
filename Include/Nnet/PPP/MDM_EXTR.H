/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1998      Accelerated Technology, Inc.           */
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
/*      mdm_extr.h                                        2.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MODEM                                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes and structure definitions */
/*      used by the modem module of PPP  and which are also accessible   */
/*      to other moodules.                                               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME                    DATE             REMARKS                     */
/*                                                                       */
/*  G. Johnson              01/09/97        Created initial version.     */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus      */
/*                                          NET 4.0. Creating verion 2.0 */
/*                                                                       */
/*************************************************************************/

/* Check to see if the file has been included already.  */

#ifndef MDM_EXTR_H
#define MDM_EXTR_H

#ifdef __cplusplus
extern "C" {
#endif

STATUS  MDM_Init(VOID);
VOID    MDM_Receive(VOID);
STATUS  MDM_Data_Ready(VOID);
STATUS  MDM_Control_String(CHAR *string);
CHAR   *MDM_Get_Modem_String(CHAR *response, CHAR *dial_string);
STATUS  MDM_Get_Char(CHAR *);
VOID    MDM_Delay(UNSIGNED milliseconds);
STATUS  MDM_Dial(CHAR *number);
VOID    MDM_Hangup(VOID);
VOID    MDM_Purge_Input_Buffer(VOID);
STATUS  MDM_Change_Communication_Mode(INT mode);
VOID    MDM_Wait_For_Client (VOID);

#ifdef __cplusplus
}
#endif

#endif
