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
/*    PPP.H                                                  2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file includes all header files that are needed for PPP. This     */
/*    file should be used by the application. Some PPP modules also use     */
/*    this file.                                                            */
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
#ifndef PPP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define PPP_H

//#define uint8 U8
//#define uint16 U16
//#define uint32 U32
//#define uchar unsigned char

#include "ppp_opts.h"
#include "ppp_defs.h"
#include "ppp_extr.h"
#include "lcp_defs.h"
#include "lcp_extr.h"
#include "urt_defs.h"
#include "urt_extr.h"
#include "mdm_defs.h"
#include "mdm_extr.h"
#include "ncp_defs.h"
#include "ncp_extr.h"
#include "chp_defs.h"
#include "chp_extr.h"
#include "pap_extr.h"
#include "pap_defs.h"
#include "String.h"
#include "Odyssey_Trace.h"

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
