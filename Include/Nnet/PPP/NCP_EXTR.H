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
/*      NCP_EXTR.H                                            2.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      NCP                                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes                           */
/*      used by NCP and also accessible to other modules.                */
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

STATUS NCP_Configure_Nak_Check (NET_BUFFER *);
STATUS NCP_IP_Code_Reject_Check (NET_BUFFER *);
STATUS NCP_Configure_Req_Check (NET_BUFFER *);
STATUS NCP_Set_Client_IP_Address (uint8 *ip_address);
VOID   NCP_IP_Stopping_State (NET_BUFFER *);
VOID   NCP_Starting_State (NET_BUFFER *);
VOID   NCP_IP_Ack_Sent_State (NET_BUFFER *);
VOID   NCP_IP_Ack_Rcvd_State (NET_BUFFER *);
VOID   NCP_IP_Req_Sent_State (NET_BUFFER *);
VOID   NCP_IP_Opened_State (NET_BUFFER *);
VOID   NCP_IP_Send_Config_Req (uchar ip[]);
VOID   PPP_NCP_IP_Interpret (NET_BUFFER *);
VOID   NCP_Timer_Expire(UNSIGNED unused);
STATUS NCP_Change_IP_Mode(int8 new_mode);

#ifdef __cplusplus
}
#endif
