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
/*      sockdefs                                              4.0           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*      This include file will define socket type error return codes, socket*/
/*      options, and socket protocol types.                                 */
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
/*      Scott Murrill           03/18/97        Added NU_NONE macro as a    */
/*                                              catch-all for unused        */
/*                                              parameters (SPR 275)        */
/*                                                                          */
/****************************************************************************/

#ifndef SOCKDEFS_H
#define SOCKDEFS_H

/* A generic catch-all for unused parameters. */
#define NU_NONE         0

/* Address family equates */
#define SK_FAM_UNSPEC   0               /* unspecified */
#define SK_FAM_LOCAL    1
#define SK_FAM_UNIX     SK_FAM_LOCAL
#define SK_FAM_IP       2               /* Internet:  UDP, TCP, etc. */
#define SK_FAM_ROUTE    17              /* Internal routing protocol */
#define SK_FAM_LINK     18              /* Link layer interface.     */

/* These equates are for backwards compatability */
#define NU_FAMILY_UNIX SK_FAM_UNIX        /* Unix */
#define NU_FAMILY_IP   SK_FAM_IP          /* Internet       - valid entry */

/* TYPE equates */
#define NU_TYPE_STREAM    0     /* stream Socket             - valid entry */
#define NU_TYPE_DGRAM     1     /* datagram Socket           - valid entry */
#define NU_TYPE_RAW       2     /* raw Socket                - valid entry */
#define NU_TYPE_SEQPACKET 3     /* sequenced packet Socket */
#define NU_TYPE_RDM       4     /* reliably delivered msg Socket */

/* PROTOCOL equates */
#define NU_PROTO_INVALID  0
#define NU_PROTO_TCP      1
#define NU_PROTO_UDP      2
#define NU_PROTO_ICMP     3

/***************************  SOCKET OPTIONS  *****************************/
/* SOCKET OPTION control flags */
#define NU_SETFLAG        1
#define NU_BLOCK          1

/* PROTOCOL LEVELS */
#define NU_SOCKET_LEVEL   0


/* Get SOCKET OPTION errors */
#define NU_BAD_SOCKETD    0
#define NU_BAD_LEVEL      1
#define NU_BAD_OPTION     2
   
/* Levels used in the call to NU_Setsockopt */
#define SOL_SOCKET          1
#define IPPROTO_IP          2
#define IPPROTO_TCP         3

/*
 * Options for use with [gs]etsockopt at the socket level.
 * First word of comment is data type; bool is stored in int.
 */
#define SO_BROADCAST        1  /* permission to transmit broadcast messages? */

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define	IP_OPTIONS          1    /* buf/ip_opts; set/get IP options */
#define	IP_HDRINCL          2    /* int; header is included with data */
#define	IP_TOS              3    /* int; IP type of service and preced. */
#define	IP_TTL              4    /* int; IP time to live */
#define	IP_RECVOPTS         5    /* bool; receive all IP opts w/dgram */
#define	IP_RECVRETOPTS      6    /* bool; receive IP opts for response */
#define	IP_RECVDSTADDR      7    /* bool; receive IP dst addr w/dgram */
#define	IP_RETOPTS          8    /* ip_opts; set/get IP options */
#define	IP_MULTICAST_IF     9    /* u_char; set/get IP multicast i/f  */
#define	IP_MULTICAST_TTL    10   /* u_char; set/get IP multicast ttl */
#define	IP_MULTICAST_LOOP   11   /* u_char; set/get IP multicast loopback */
#define	IP_ADD_MEMBERSHIP   12   /* ip_mreq; add an IP group membership */
#define	IP_DROP_MEMBERSHIP  13   /* ip_mreq; drop an IP group membership */

/*******************  SOCKET ERROR CODES  ****************************/

#define NU_ARP_FAILED           -26     /*  ARP failed to resolve addr. */
#define NU_INVALID_PROTOCOL     -27     /*  Invalid network protocol */
#define NU_NO_DATA_TRANSFER     -28     /*  Data was not written/read
                                            during send/receive function */
#define NU_NO_PORT_NUMBER       -29     /*  No local port number was stored
                                            in the socket descriptor */
#define NU_NO_TASK_MATCH        -30     /*  No task/port number combination
                                            existed in the task table */
#define NU_NO_SOCKET_SPACE      -31     /*  The socket structure list was full
                                            when a new socket descriptor was
                                            requested */
#define NU_NO_ACTION            -32     /*  No action was processed by
                                            the function */
#define NU_NOT_CONNECTED        -33     /*  A connection has been closed
                                            by the network.  */
#define NU_INVALID_SOCKET       -34     /*  The socket ID passed in was
                                            not in a valid range.  */
#define NU_NO_SOCK_MEMORY       -35     /*  Memory allocation failed for
                                            internal sockets structure.  */
#define NU_NOT_A_TASK           -36     /*  Attempt was made to make a
                                            sockets call from an interrupt
                                            without doing context save.  */
#define NU_INVALID_ADDRESS      -37     /*  An incomplete address was sent */
#define NU_NO_HOST_NAME         -38     /*  No host name specified in a  */
#define NU_RARP_INIT_FAILED     -39     /*  During initialization RARP failed. */
#define NU_BOOTP_INIT_FAILED    -40     /*  During initialization BOOTP failed. */
#define NU_INVALID_PORT         -41     /*  The port number passed in was
                                            not in a valid range. */
#define NU_NO_BUFFERS           -42     /*  There were no buffers to place */
                                        /*  the outgoing packet in. */
#define NU_NOT_ESTAB            -43     /*  A connection is open but not in
                                            an established state. */
#define NU_INVALID_BUF_PTR      -44     /*  The buffer pointer is invalid */
#define NU_WINDOW_FULL          -45     /*  The foreign host's in window is
                                            full. */
#define NU_NO_SOCKETS           -46     /*  No sockets were specified. */
#define NU_NO_DATA              -47     /*  None of the specified sockets were
                                            data ready.  NU_Select. */



/* The following errors are reported by the NU_Setsockopt and NU_Getsockopt
   service calls. */
#define NU_INVALID_LEVEL        -48     /*  The specified level is invalid. */
#define NU_INVALID_OPTION       -49     /*  The specified option is invalid. */
#define NU_INVAL                -50     /*  General purpose error condition. */
#define NU_ACCESS               -51     /*  The attempted operation is not   */
                                        /*  allowed on the  socket           */
#define NU_ADDRINUSE            -52

#define NU_HOST_UNREACHABLE     -53     /*  Host unreachable */
#define NU_MSGSIZE              -54     /*  Packet is to large for interface. */
#define NU_NOBUFS               -55     /*  Could not allocate a memory buffer. */
#define NU_UNRESOLVED_ADDR      -56     /*  The MAC address was not resolved.*/
#define NU_CLOSING              -57     /*  The other side in a TCP connection*/
                                        /*  has sent a FIN */
#define NU_MEM_ALLOC            -58     /* Failed to allocate memory. */
#define NU_RESET                -59   

/* These error codes are returned by DNS. */
#define NU_INVALID_LABEL        -60     /* Indicates a domain name with an
                                           invalid label.                   */
#define NU_FAILED_QUERY         -61     /* No response received for a DNS
                                           Query. */
#define NU_DNS_ERROR            -62     /* A general DNS error status. */
#define NU_NOT_A_HOST           -63     /* The host name was not found. */
#define NU_INVALID_PARM         -64     /*  A parameter has an invalid value. */

#define NU_NO_IP                -65     /*  An IP address was not specified for
                                            the device. */
#define NU_DHCP_INIT_FAILED     -66     /*  During initialization BOOTP failed. */
#endif  /* SOCKDEFS_H */
