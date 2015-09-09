/****************************************************************************/
/*                                                                          */
/*       CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.            */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in             */
/* the subject matter of this material.  All manufacturing, reproduction,   */
/* use, and sales rights pertaining to this subject matter are governed     */
/* by the license agreement.  The recipient of this software inplicitly     */
/* accepts the terms of the license.                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* FILE NAME                                        VERSION                 */
/*                                                                          */
/*      TCPVARS.C                                      4.0                  */
/*                                                                          */
/* COMPONENT                                                                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/* This file will hold all the global variables used int the current TCP/IP */
/* source call for interfacing with the packets and data.                   */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/* Craig L. Meredith                                                        */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/* None.                                                                    */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/* None.                                                                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*     NAME             DATE        REMARKS                                 */
/*  Craig L. Meredith  02/03/94    Inital version.                          */
/*                                                                          */
/****************************************************************************/

#ifdef PLUS
#include "nucleus.h"
#else   /* !PLUS */
#include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
#include "nu_extr.h"
#endif  /* !PLUS */
#include "sockdefs.h"     /* socket definitions */
#include "target.h"
#include "protocol.h"
#include "ip.h"

/****************************************************************************/
/*                                                                          */
/*                               sint                                       */
/*                                                                          */
/*      Various global data variables used for maintaining TCP/IP stack     */
/*      information.                                                        */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/* A global counter of the number of tasks waiting for buffers to be freed
 * so that data can be sent via TCP.  Initialized in tcpinit. */
int16 tasks_waiting_to_send;

sint SQwait = 0;
sint OKpackets = 0;

/****************************************************************************/
/*                                                                          */
/*                          MISCELLANEOUS                                   */
/*                                                                          */
/*      Various global data variables used for maintaining TCP/IP stack     */
/*      information.                                                        */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/*
 * Define the timerlist control pointers.  These are globally used to
 * maintain the system timer list that is used for retry timeouts and
 * the like.
 */
struct tqhdr tcp_timerlist, tcptimer_freelist;

/*
 * Define the portlist table.  This is a critical structure in Nucleus NET
 * as it maintains information about all open connections.
 */
struct port *portlist[NPORTS];        /* allocate like iobuffers in UNIX */

/*
 * Define the UDP portlist table.  This is a critical structure in Nucleus NET
 * as it maintains information about all open UDP ports.
 */
struct uport *uportlist[NUPORTS];      /* allocate like iobuffers in UNIX */

/* 
 * The following structure is used to define the socket list for Nucleus NET.
 * Sockets are requried for all TCP connections.
 */
struct sock_struct *socket_list[NSOCKETS] = {NU_NULL};

/*
 * Task list entries are maintained on NU_Listen calls to ensure that
 * the proper number of attempted connections can be queued up while
 * one is being processed.
 */
struct TASK_TABLE_STRUCT *Task_Head;  /* pointer to the first task structure */
struct TASK_TABLE_STRUCT *Task_Tail;  /* pointer to the last task structure */
