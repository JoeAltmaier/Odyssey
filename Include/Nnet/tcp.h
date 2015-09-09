/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
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
/*      TCP.H                                             4.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TCP - Transmission Control Protocol                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Holds the defines for the TCP protocol.                          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Accelerated Technology Inc.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*  global compenent data stuctures defined in this file                 */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      No functions defined in this file                                */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      No other file dependencies                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME                            DATE            REMARKS          */
/*                                                                       */
/*************************************************************************/
#ifndef TCP_H
#define TCP_H

#include "protocol.h"

#define DATA           1
#define ACK            2


/* This is the number of ticks to delay sending an ACK.  A value of
   approximately a 1/4 is recommended. */
#define ACKTIMEOUT     (TICKS_PER_SECOND >> 2)

/*****************************************************************************/
/* IN_CLASSA,  IN_CLASSB, IN_CLASSC                                          */
/*                                                                           */
/* These macros are used to determine the class of an IP address.  Note that */
/* they will only work if i is unsigned.  The class of the address is        */
/* determined by the first two bits of the IP address.                       */
/*                                                                           */
/*****************************************************************************/
#define IN_CLASSC(i)   ((i >> 30) == 3)
#define IN_CLASSB(i)   ((i >> 30) == 2)
#define IN_CLASSA(i)   ((i >> 30) <= 1)


#endif /* TCP.H */

