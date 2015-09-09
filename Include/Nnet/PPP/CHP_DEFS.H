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
/*    CHP_DEFS.H                                             2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the constant definitions to support the challenge  */
/*    handshake authentication protocol, CHAP.C                             */
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

#define CHAP_CHALLENGE  1
#define CHAP_RESPONSE   2
#define CHAP_SUCCESS    3
#define CHAP_FAILURE    4

#define CHAP_VALUE_LENGTH_OFFSET    4
#define CHAP_VALUE_OFFSET           5
#define CHAP_MD5_VALUE_SIZE         16
#define CHAP_CHALLENGE_VALUE_SIZE   4

#ifdef __cplusplus
}
#endif
