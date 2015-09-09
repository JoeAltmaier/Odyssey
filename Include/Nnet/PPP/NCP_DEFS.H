/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1998 by Accelerated Technology, Inc.                  */
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
/*    NCP_DEFS.H                                              2.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains constant definitions and structure definitions     */
/* to support the file ncp.c                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0       */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1        */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus       */
/*                                          NET 4.0. Creating verion 2.0    */
/*                                                                          */
/****************************************************************************/
#ifndef NCP_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define NCP_DEFS_H

#define NCP_IP_COMPRESSION      2
#define NCP_IP_ADDRESS          3
#define IP_ADDRESS_LENGTH       4
#define NCP_IP_ADDRESS_LENGTH   6

#define CLIENT                  5
#define SERVER                  10

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
