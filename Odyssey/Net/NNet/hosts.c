/****************************************************************************/
/*                                                                          */
/*    CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.               */
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
/*  Hosts                                                      4.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This file contains the hostTable structure.                         */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Accelerated Technology Inc.                                         */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*                                                                          */
/****************************************************************************/


#include "protocol.h"
#include "socketd.h"
#include "ip.h"
    

struct host hostTable[] =
{
    {"GLEN",       198, 100, 100, 10},
    {"allynon",    206, 202, 34, 80},
    {"DAVID",      198, 100, 100, 4},
    {"LAB",        198, 100, 100, 3},
    {"MODEL",      198, 100, 100, 1},
    {"NULL",       0, 0, 0, 0}

};

