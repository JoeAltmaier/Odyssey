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
/*    PW_LIST.C                                              2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the data structure that holds a list of allowed    */
/*    users and their passwords.                                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  struct pw_list _passwordlist                                            */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  pap_defs.h                                                              */
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

#include "pap_defs.h"

/* This is the password list. When authenticating a user it is this list
   that will be used to compare the ID and PW. The length of the ID and PW
   pair is define in PPP_OPTS.H. The only restriction placed on this list is
   that the last entry MUST end with {0, 0}.
*/

struct pw_list _passwordlist [] =
{
    {"ppp", "ppp"},
    {"PPP", "PPP"},
    {0, 0}
};
