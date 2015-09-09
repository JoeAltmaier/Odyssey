/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1998by Accelerated Technology, Inc.                  */
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
/*    CHP_EXTR.H                                             2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the function prototypes to support the file CHAP.C */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  none                                                                    */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  none                                                                    */
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

STATUS CHAP_Check_Response (NET_BUFFER *);
VOID   CHAP_MD5_Encrypt (unsigned char *string, unsigned char *digest);
VOID   CHAP_Respond_To_Challenge (NET_BUFFER *);
VOID   PPP_CHAP_Interpret (NET_BUFFER *);
VOID   CHAP_Timer_Expire (UNSIGNED unused);
VOID   CHAP_Send_Success (NET_BUFFER *);
VOID   CHAP_Send_Failure (NET_BUFFER *);
VOID   CHAP_Send_Challenge (VOID);

#ifdef __cplusplus
}
#endif
