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
/*      tftp.h                                          TFTP  4.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TFTP -  Trivial File Transfer Protocol                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of for TFTP client        */
/*      services accessible from user applications.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-27-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/


#define NU_Get  TFTPC_Get
#define NU_Put  TFTPC_Put

int32  TFTPC_Get(char *, int32, char *, char *);
int32  TFTPC_Put(char *, int32, char *, char *);
