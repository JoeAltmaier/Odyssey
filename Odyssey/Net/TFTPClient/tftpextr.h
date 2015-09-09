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
/*      tftpextr.h                                      TFTP  4.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TFTP -  Trivial File Transfer Protocol                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all TFTP functions.    */
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
/*      tftpdefs.h                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-27-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/

STATUS TFTPC_Read_Request(char *, char *, TFTP_CB *);
STATUS TFTPC_Write_Request(char *, char *, TFTP_CB *);
int16  TFTPC_Recv(TFTP_CB *);
STATUS TFTPC_Process_Data(TFTP_CB *, int16);
STATUS TFTPC_Ack(TFTP_CB *);
STATUS TFTPC_Send_Data(TFTP_CB *);
STATUS TFTPC_Process_Ack(TFTP_CB *);
STATUS TFTPC_Retransmit(TFTP_CB *, int16);
STATUS TFTPC_Error(TFTP_CB *, int16, char *);


