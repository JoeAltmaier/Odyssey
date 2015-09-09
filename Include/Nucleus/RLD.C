/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1996 Accelerated Technology, Inc.           */
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
/*      rld.c                                   PLUS/IDT5000/GNU 1.0     */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      RL - Release Information                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains information about this release of Nucleus     */
/*      PLUS.                                                            */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      RLD_Release_String                  Release information string   */
/*      RLD_Special_String                  Special Nucleus PLUS string  */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           System definitions           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      S. Nguyen       06-30-1998      Version 1.0 for IDT5000          */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "nucleus.h"                 /* System definitions        */


/* RLD_Release_String contains a string describing this release of Nucleus
   PLUS.  */
   
const CHAR  RLD_Release_String[] = 
  "Copyright (c) 1993-1998 ATI - Nucleus PLUS - Version 1.0.G1.3";


/* RLD_Special_String contains information about the origins of the Nucleus
   PLUS system.  */
   
const CHAR  RLD_Special_String[] =
  "G,M,D,GB,GL,AG,KL,CR,HR,NH,DL,BH,LP,AP,HA,ME,KC,KH,GF,RG,HS,DS,KY,BC,LC,TD";

