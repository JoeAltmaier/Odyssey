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
/*    PAP_EXTR.H                                                 2.0        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Contains the function prototypes for PAP.C                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
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
#ifdef __cplusplus
extern "C" {
#endif

void   PAP_Send_Authentication (void);
void   PPP_PAP_Interpret (NET_BUFFER *);
STATUS PAP_Check_Login (char id[PPP_MAX_ID_LENGTH], char pw[PPP_MAX_PW_LENGTH]);
void   PAP_Send_Authentication_Ack_Nak(NET_BUFFER *, int pkt_type, int req_id);
void   PAP_Timer_Expire (UNSIGNED);

#ifdef __cplusplus
}
#endif
