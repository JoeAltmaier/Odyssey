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
/*  wpw_list                                                   1.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This file contains the  Nucleus WebServ user_id password            */
/*      structure for HTTP 1.0 Basic Authentication.                        */
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


#include "nucleus.h"
#include "target.h"
#include "protocol.h"
#include "ip.h"
#include "b_auth.h"
    
/*  This table is in the format of {User_ID, Password, Writes} */
struct wpwlist webpwTable[] =
{
    {"john", "doe","ALL"},
    {"fred", "fish","ALL"},
    {"willy","wildcat@1","ALL"},
    {0,0,0}
};

