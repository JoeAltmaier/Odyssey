/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1993 - 1996 by Accelerated Technology, Inc.           */
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
/*      netevent                                              4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will define the events used for the NCSA Telnet.  */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Craig L. Meredith, Accelerated Technology Inc.                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  global compenent data stuctures defined in this file                    */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*      No functions defined in this file                                   */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*      No other file dependencies                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*      Craig L. Meredith       04/10/93        Initial version.            */
/*      Craig L. Meredith       08/17/93        Added header, Neil's mods.  */
/*                                                                          */
/****************************************************************************/

#ifndef NETEVENT_H
#define NETEVENT_H

/* Events processed by the Events Dispatcher. */
#define CONOPEN             1   /* connection has opened, CONCLASS */
#define CONCLOSE            2   /* the other side has closed its side of the connection */
#define CONFAIL             3   /* connection open attempt has failed */
#define CONRX               4   /*  just received a buffer.  */
#define CONNULL             5   /*      Just a null event...     */
                                /*      Used to wake up the dispatcher from a
                                 *      indefinite sleep should a timer queue
                                 *      entry be posted...could be used for other
                                 *      purposes also.
                                 */

#define UDPDATA             6   /* UDP data has arrived on listening port, USERCLASS */
#define TCPRETRANS          7   /* TCP segment retransmission event */
#define WINPROBE            8   /* Window Probe event. */
#define TCPACK              9   /* TCP ACK transmission event */
#define CONTX               10  /* buffer needs to be sent. */
#define SELECT              11  /* TCP Select timeout event. */
#define ARPRESOLVE          12  /* ARP event. */
#define RARP_REQUEST        13  /* A RARP request event. */

/* PPP Events */
#define LCP_RESEND          14  /* Resend the last sent LCP packet, PPP_CLASS */
#define LCP_SEND_CONFIG     15  /* Send a LCP config req packet, PPP_CLASS */
#define HANGUP_MDM          16  /* Hangup the mdm up, PPP_CLASS */
#define LCP_ECHO_REQ        17  /* Send an echo request packet, PPP_CLASS */
#define NCP_RESEND          18  /* Resend the last send NCP packet, PPP_CLASS */
#define NCP_SEND_CONFIG     19  /* Send a NCP config req packet, PPP_CLASS */
#define LCP_CLOSE_LINK      20  /* Close the link and all sockets, PPP_CLASS */
#define PAP_SEND_AUTH       21  /* Send a PAP authentication packet, PPP_CLASS */
#define CHAP_RESEND         22  /* Resend the last CHAP response, PPP_CLASS */
#define CHAP_CHALL          23  /* Send a challenge to the host, PPP_CLASS */

#define EV_IGMP_REPORT      24  /* Send an IGMP Group Report. */
#define EV_IP_REASSEMBLY    25  /* Timeout the reassembly of an IP datagram. */

#define NU_CLEAR       0
#define NU_SET         1

#endif  /* NETEVENT_H */
