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
/*    PAP_DEFS.H                                                 2.0        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Contains the constant and structure definitions for PAP.C             */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah Pollock                                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*      pw_list                                                             */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      ppp_opts.h                                                          */
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

#ifndef PAP_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define PAP_DEFS_H

#include "ppp_opts.h"

/* this is the structure that will hold the password list */
struct pw_list
{
    char id[PPP_MAX_ID_LENGTH];
    char pw[PPP_MAX_PW_LENGTH];
};

#define PAP_AUTHENTICATE_REQUEST    1
#define PAP_AUTHENTICATE_ACK        2
#define PAP_AUTHENTICATE_NAK        3

#define PAP_ID_LENGTH_OFFSET        4
#define PAP_ID_OFFSET               5

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
