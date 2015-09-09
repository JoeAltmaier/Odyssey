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
/*    PPP_OPTS.H                                             2.0            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains all configuration options used by PPP. These can   */
/*    be modified in order to fit the specific needs of the applicaiton.    */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
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
#ifndef PPP_OPTS_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define PPP_OPTS_H

/* This defines the total length that a password and id can be */
#define PPP_MAX_ID_LENGTH               31
#define PPP_MAX_PW_LENGTH               31

/* These defines are for monitoring the modem link.

   MAX_ECHO
   is the number of times we will transmit an echo request without
   receiving a reply. After this value is reached the link will
   be considered to be closed.

   ECHO_VALUE
   is the time to wait between echo requests. This value is in clock
   ticks and is specific to the target hardware. A value that is too
   small will just conjest the network and slow the PPP link down. It also
   may not give enough time for the reply to come back, in which case the
   link will be considered to be closed even when it is not.
*/
#define LCP_MAX_ECHO                    2
#define LCP_ECHO_VALUE                  (TICKS_PER_SECOND * 60) /* 60 secs */

/* LCP default configuration values. */

/* This defines which ASCII characters, 0 through 32, will be affected
   by transparency. On some links there may be hardware devices that add
   characters or effect the control characters. This will ensure that the
   control characters are transparent to these devices and that if a device
   adds any characters they will be dropped. Each bit represents its
   coorosponding ASCII characters. By default all characters 0 - 32 are
   made transparent.
*/

/* This define is the default for the foreign end of the link. */
#define LCP_FOREIGN_DEFAULT_ACCM        0xffffffff

/* This define is for the local end of the link. */
#define LCP_LOCAL_DEFAULT_ACCM          0xffffffff

/* Will the above ACCM option be used?                 */
#define LCP_USE_ACCM                    NU_TRUE

/* Will the max receive unit option be used? This options should
   only be used if the MRU is different from the default size of
   1500 bytes. */
#define LCP_USE_MRU                     NU_FALSE

/* Will the magic number configuration option be used? */
#define LCP_DEFAULT_MAGIC_NUMBER        NU_TRUE

/* Will protocol field compression be used? */
#define LCP_DEFAULT_PROTOCOL_COMPRESS   NU_TRUE

/* Will address and control field compression be used? */
#define LCP_DEFAULT_ADDRESS_COMPRESS    NU_TRUE

/* This will be used when in SERVER mode. It will be the first
   authentication protocol that is tried with the CLIENT.

   0xc223 is for CHAP with MD5 encryption
   0xc023 is for PAP
*/
#define LCP_DEFAULT_AUTH_PROTOCOL       0xc023

/* How many rings before the phone is answered. To change this simply
   change the 2 to the number of rings that is required.
*/
#define     MDM_ACCEPT_CALL             "ATS0=2^M"  /* Answer on two rings */

/* The following defines are the number of times PPP will retransmit
   the packet.

   LCP_MAX_CONFIGURE    is for configure request packets, LCP and NCP.
   LCP_MAX_TERMINATE    is for terminate request packets.
   LCP_MAX_AUTHENTICATE is for authentication packets, PAP and CHAP.
*/
#define LCP_MAX_CONFIGURE               10
#define LCP_MAX_TERMINATE               3
#define LCP_MAX_AUTHENTICATE            3

/*  This timeout value is the time PPP will wait before retransmiting
    the above mentioned packet types. This value is in clock ticks
    an is specific to the target hardware.
*/
#define LCP_TIMEOUT_VALUE               (TICKS_PER_SECOND * 3) /* 3 secs */

/* This defines the length of the ring buffer used by PPP to hold
   packets for transfer between the LISR and the HISR. This should not
   be made to small or else packets may become overwritten within the buffer.
*/
#define PPP_MAX_HOLDING_PACKETS         10

/* These defines are used for debugging.

   DEBUG_PRINT     will turn on the printing of information about
                   link negotiation.

   _PRINT          will direct where to print the above information.

   DEBUG_PKT_TRACE will capture all packets sent and received. They
                   are stored in an array defined in PPP.C

   PKT_TRACE_SIZE  defines the size of the packets trace buffers. Note
                   that these are circular buffers.
*/




#define DEBUG_PRINT
#define _PRINT Tracef
/*
#define DEBUG_PKT_TRACE
#define PKT_TRACE_SIZE  10000

*/

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif
