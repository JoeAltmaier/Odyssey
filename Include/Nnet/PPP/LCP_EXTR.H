/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1998 by Accelerated Technology, Inc.             */
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
/*      LCP_EXTR.H                                            2.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      LCP                                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes                           */
/*      used by LCP and also accessible to other modules.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Uriah T. Pollock                                                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*       NAME                 DATE            REMARKS                    */
/*                                                                       */
/*  Uriah T. Pollock        08/18/97      Created initial version 1.0    */
/*  Uriah T. Pollock        11/18/97      Updated PPP to version 1.1     */
/*  Uriah T. Pollock        05/06/98      Integrated PPP with Nucleus    */
/*                                          NET 4.0. Creating verion 2.0 */
/*                                                                       */
/*************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

VOID   LCP_Open_State (NET_BUFFER *buf_ptr);
VOID   LCP_Closing_State (NET_BUFFER *buf_ptr);
VOID   LCP_Stopping_State (NET_BUFFER *buf_ptr);
VOID   PPP_LCP_Interpret (NET_BUFFER *buf_ptr);
VOID   LCP_Ack_Sent_State (NET_BUFFER *buf_ptr);
VOID   LCP_Ack_Rcvd_State (NET_BUFFER *buf_ptr);
VOID   LCP_Req_Sent_State (NET_BUFFER *buf_ptr);
VOID   LCP_Send_Config_Req(void);
VOID   LCP_Timer_Expire(UNSIGNED);
VOID   LCP_Send_Echo_Reply (NET_BUFFER *buf_ptr);
VOID   LCP_Send_Code_Reject (NET_BUFFER *, uint16);
VOID   LCP_Send_Terminate_Req(void);
VOID   LCP_Send_Terminate_Ack(NET_BUFFER *buf_ptr, uint16);
VOID   LCP_Send_Echo_Req (DV_DEVICE_ENTRY *dev_ptr);
VOID   LCP_Echo_Expire (UNSIGNED unused);
VOID   LCP_Send_Protocol_Reject (NET_BUFFER *);
STATUS LCP_Code_Reject_Check (NET_BUFFER *buf_ptr);
STATUS LCP_Configure_Nak_Check (NET_BUFFER *buf_ptr);
VOID   LCP_Configure_Reject_Check (NET_BUFFER *buf_ptr);
STATUS LCP_Configure_Req_Check (NET_BUFFER *buf_ptr);
uint32 LCP_Random_Number32(void);
uint8  LCP_Random_Number(void);

#ifdef __cplusplus
}
#endif
