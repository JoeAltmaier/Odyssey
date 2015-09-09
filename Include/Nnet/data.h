/*************************************************************************/
/*                                                                       */
/*   Copyright (c) 1993 - 1996 by Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/*
*
* Portions of this program were written by:       */
/***************************************************************************
*                                                                          *
*                                                                          *
*      NCSA Telnet                                                         *
*      by Tim Krauskopf, VT100 by Gaige Paulsen, Tek by Aaron Contorer     *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
****************************************************************************
*/
/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   DATA.H                                             NET 4.0               */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   External definitions of global variables used by Nucleus NET.            */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*      NAME            DATE                    REMARKS                       */
/*                                                                            */
/******************************************************************************/

#ifndef DATA_H
#define DATA_H

#include "target.h"

/*  The following externs are defined in file TCPVARS.C.  */

extern int16 tasks_waiting_to_send;

extern struct tqhdr tcptimer_freelist;
extern struct tqhdr tcp_timerlist;

extern struct port *portlist [NPORTS];     /* allocate like iobuffers in UNIX */
extern struct uport *uportlist [NUPORTS];  /* allocate like iobuffers in UNIX */
extern struct host hostTable[];

#endif  /* DATA_H */
