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
/*      Holds the defines related to the TCP protocol.                   */
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

              /* the definitions for Nucleus - TCP/IP program */

#ifndef TCPDEFS_H
#define TCPDEFS_H

#ifdef PLUS
  #include "nucleus.h"
#else
  #include "nu_extr.h"
  #include "nu_defs.h"
#endif


/************ RESOURCES ************/
#ifdef PLUS
extern NU_SEMAPHORE             TCP_Resource;
extern NU_PARTITION_POOL        Mace_Pool;
#else
#define TCP_Resource            0
#endif



/************* EVENTS **************/
#ifdef PLUS
extern NU_QUEUE        eQueue;
extern NU_MEMORY_POOL  System_Memory;
extern NU_EVENT_GROUP  Buffers_Available;
#else
#define eQueue        0         /* event queue is at index 0 */
#define NU_EventsDispatcherID 0 /* Events Dispatcher task is at index 0. */
#define BUFFERS_AVAILABLE   0   /* Event group used by a driver to communicate
                                    with the protocol stack */
#endif

#endif /* TCPDEFS.H */
