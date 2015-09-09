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
/*  PPP_EXTR.H                                            2.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*  PPP                                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*  This file contains function prototypes                               */
/* used by PPP and also accessible to other modules.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*  Uriah T. Pollock                                                     */
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
#ifndef PPP_EXTR_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#define PPP_EXTR_H

VOID   PPP_HISR_Entry (VOID);
uint16 PPP_Compute_TX_FCS (uint16, NET_BUFFER *);
uint16 PPP_Compute_RX_FCS (uint16, PPP_TEMP_BUFFER *);
STATUS PPP_Lower_Layer_Up (uint8 *);
VOID   PPP_Set_Login (char [], char[]);
VOID   PPP_Kill_All_Open_Sockets (DV_DEVICE_ENTRY *dev_ptr);
STATUS PPP_Safe_To_Hangup (DV_DEVICE_ENTRY *dev_ptr);
STATUS PPP_Still_Connected (VOID);
uint32 PPP_Two_To_Power (uint8 exponent);
STATUS PPP_Hangup(uint8);
STATUS PPP_TX_Packet (DV_DEVICE_ENTRY *, NET_BUFFER *);
STATUS PPP_RX_Packet (DV_DEVICE_ENTRY *);
STATUS PPP_Input (VOID);
STATUS PPP_Output (NET_BUFFER *, DV_DEVICE_ENTRY *,
                SCK_SOCKADDR_IP *, RTAB_ROUTE *);
extern STATUS PPP_Initialize (DV_DEVICE_ENTRY *dev_ptr);
STATUS PPP_Wait_For_Client (uint8 *server_ip_address);
STATUS PPP_Dial (CHAR *number);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
