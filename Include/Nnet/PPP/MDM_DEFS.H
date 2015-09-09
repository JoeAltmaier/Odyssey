/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1998      Accelerated Technology, Inc.           */
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
/*      mdm_defs.h                                        2.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MODEM                                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains constant definitions for the modem module     */
/*      of SLIP.                                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      MDM_BUFFER      The terminal mode receive buffer.                */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*  NAME                    DATE             REMARKS                     */
/*                                                                       */
/*  G. Johnson              01/09/97        Created initial version.     */
/*  Uriah T. Pollock        05/06/98        Integrated with Nucleus      */
/*                                          NET 4.0. Creating verion 2.0 */
/*                                                                       */
/*************************************************************************/

/* Check to see if the file has been included already.  */

#ifndef MDM_DEFS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define MDM_DEFS_H

typedef struct _MDM_BUFFER
{
    char    *mdm_head;
    char    *mdm_tail;
    char    *mdm_read;
    char    *mdm_write;
    INT     mdm_buffer_status;
} MDM_BUFFER;

/* The modem can be used in two different modes: a terminal mode and network
   communication mode.  The following definitions are used to control which mode
   is in use. */
#define MDM_NETWORK_COMMUNICATION      1
#define MDM_TERMINAL_COMMUNICATION     2
/* For use by console and other apps */
#define MDM_OTHER_COMMUNICATION		   3

/* This is the size of the receive buffer when in terminal mode. */
#define     MDM_RECV_BUFFER_SIZE        100

/* These are the possible values for the mdm_buffer_status. */
#define     MDM_BUFFER_EMPTY            1  /* No data in the buffer. */
#define     MDM_BUFFER_FULL             2  /* The buffer is full. */
#define     MDM_BUFFER_DATA             3  /* There is data in the buffer. */

#define     MDM_DIAL_PREFIX             "ATDT"
#define     MDM_HANGUP_STRING           "~~~+++~~~ATH0^M"
#define     MDM_IGNORE_CALLS            "ATS0=0^M"

/* How many rings before the phone is answered. To change this simply
   change the 2 to the number of rings that is required.
*/
#define     MDM_ACCEPT_CALL             "ATS0=2^M"  /* Answer on two rings */

#define     NU_NO_CONNECT               -101
#define     NU_NO_CARRIER               -50
#define     NU_BUSY                     -75
#define     NU_INVALID_MODE             -100

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
