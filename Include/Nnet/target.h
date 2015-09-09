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
/*  target                                                     1.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This file will hold all of those defines and setups used by the         */
/*  TCP/IP code which are processor dependent.                              */
/*  This version is for the borlandc.                                       */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*  Glen Johnson                                                            */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*  None.                                                                   */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*  None.                                                                   */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*  None.                                                                   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME           DATE        REMARKS                                      */
/*                                                                          */
/*  Glen Johnson   6/19/95     Inital version.  This file was created by    */
/*                             combinig PCDEFS.H and COMPILER.H.  Neither   */
/*                             of these files are still used.               */
/*  Sudhir Kasargod 5/7/99     Removed #include of stdio.h, string.h and    */
/*                             stdlib.h                                     */
/*  Sudhir Kasargod 5/7/99     Changed the typedef for int32 and uint32
/*                             stdlib.h                                     */
/*                                                                          */
/****************************************************************************/

#ifndef TARGET_H
#define TARGET_H

#if 0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

/* This is not target specific, but it is useful to have it here. */
typedef struct _RTAB_Route         RTAB_ROUTE;
typedef struct SCK_IP_ADDR_STRUCT  SCK_IP_ADDR;
typedef struct _DV_DEVICE_ENTRY    DV_DEVICE_ENTRY;
typedef struct _DEV_DEVICE         DEV_DEVICE;
typedef struct _DV_REQ             DV_REQ;
typedef struct _IP_MULTI           IP_MULTI;
typedef struct _IP_MULTI_OPTIONS   IP_MULTI_OPTIONS;
typedef struct sock_struct         SOCKET;
typedef struct SCK_SOCKADDR_IP_STRUCT SCK_SOCKADDR_IP;

/*
*  Setup the current limits and size type defs for the defined processor.
*  This will define constants for the sizes of integral types, with
*  minimums and maximum values allowed for the define.
*  Note: mins and maxs will be different for different processors.
*                               Size (Bytes)    Alignment (Bytes)
*/
/* size casting */
typedef char int8;              /*  1                   1   */
typedef unsigned char uint8;    /*  1                   1   */
typedef short int int16;        /*  2                   2   */
typedef unsigned short uint16;  /*  2                   2   */
typedef int int32;         /*  4                   4   */
typedef unsigned int uint32;   /*  4                   4   */

/* generic names for size types */
typedef char schar;             /*  1                   1   */
typedef unsigned char uchar;    /*  1                   1   */
typedef short int sshort;       /*  2                   2   */
typedef unsigned short ushort;  /*  2                   2   */
typedef unsigned int uint;      /*  2                   2   */
typedef int sint;               /*  2                   2   */
typedef long int slint;         /*  4                   4   */
typedef long int slong;         /*  4                   4   */
typedef unsigned long ulint;    /*  4                   4   */
typedef unsigned long ulong;    /*  4                   4   */

/* max/mins */
#define USHORT_MAX   65535  /* The maximum value of a uint16 variable. */

/* These macroes are used for the versions of Nucleus NET that run in real-mode
   on the PC.  For all architectures other than rel-mode PC these should be
   defined to be nothing.
*/
#undef   FAR
#undef   HUGE
#define  HUGE
#define  FAR

/* The following definitions specify which routines have been implemented
 * in assembly language.  A C implementation of each can be found in the
 * file NEW_IP.C.  An assembly implementation of tcpcheck may especially
 * increase performance.  An one indicates an assembly implementation exists.
 */
#define IPCHECK_ASM     0
#define TCPCHECK_ASM    0
#define LONGSWAP_ASM    0
#define INTSWAP_ASM     0
#define COMPAREN_ASM    0

/* The PACKET definition controls how packets are sent.  If PACKET is defined
 * then each packet is transmited as soon as it is ready.  If PACKET is
 * undefined the packets are placed into a transmit queue when they are
 * ready. */
/*
   Commented by Sudhir Kasargod
#define PACKET
*/

/* The PRINT_ERROR_MSG define controls whether a error message is printed to
 * the console when NU_Tcp_Log_Error is called.  The error is logged
 * regardless of PRINT_ERROR_MSG define.  However, an error message will only
 * be printed if PRINT_ERROR_MSG is defined.   */
#undef PRINT_ERROR_MSG

/* The SWAPPING macro controls whether byte swapping is performed on packets.
   SWAPPING should be defined for platforms that store integers in Little
   Endian format, and undefined for platforms that store integers in Big
   Endian format. */
#undef SWAPPING

/* This is the number of Nucleus PLUS timer ticks that occur per second. */
#define TICKS_PER_SECOND        108

/*
*  how often to poke a TCP connection to keep it alive and make
*  sure other side hasn't crashed. (poke) in 1/18ths sec
*  And, timeout interval. Only used in polled mode.
*/
#define POKEINTERVAL  (180 * TICKS_PER_SECOND)
#define MAXRTO        (8 * TICKS_PER_SECOND)  /* Maximum retransmit timeout.*/
#define MINRTO        (3 * TICKS_PER_SECOND)  /* Min. retransmit timeout. */
#define ARPTO         (1 * TICKS_PER_SECOND)  /* ARP timeout. */
#define CACHETO       (400 * TICKS_PER_SECOND)  /* ARP cache timeout. */
#define WAITTIME      (2 * TICKS_PER_SECOND)  /* Length of time to wait before
                                               reusing a port. */

#define ARP_CACHE_LENGTH   10                    /* Size of the ARP cache. */
#define CREDIT             4096

#define NPORTS          50              /* Maximum number of TCP ports. */
#define NUPORTS         30              /* Maximum number of UDP ports.  */
#define NSOCKETS        80              /* Total number of socket descriptors.
                                           This should  be NPORTS + NUPORTS */

#define MAXBUFFERS      200             /* The max number of incoming packets
                                         * that can be held in the queue. */

#define MAX_RETRANSMITS 5               /* The max number of times to
                                         * retransmit a packet. */
#define WINDOWSIZE      16000           /* Size of buffers for TCP in/out */
#define UMAXDGRAMS      5               /* Maximum UDP data grams that
                                           can be buffered for a single port */

/* This is the local host's name.  It can be a maximum of 32 charaters long. */
#define HOSTNAME   "odyssey"

/* Nucleus NET relies on a couple of tasks to perform its duties. The priority
 * of each is defiend below. */
#define EV_PRIORITY   3   /* The Events Dispather priority. */
#define TM_PRIORITY   3   /* The Timer Task priority. */

/* SWSOVERIDE is the amount of time to wait before overriding the Nagle
   algorithm.  The Nagle algorithm is aimed at preventing the transmission of
   lots of tiny packets.  However, we only want to delay a packet for a short
   period of time.  RFC 1122 recommends a delay of 0.1 to 1.0 seconds.  We
   default to a delay of a 1/4 second. */
#define SWSOVERRIDE       (TICKS_PER_SECOND >> 2)  /* Delay of a 1/4 second */

/*  PROBETIMEOUT  is the delay before a window probe is sent.  */
#define PROBETIMEOUT      (TICKS_PER_SECOND << 1)  /*  Delay of 2 seconds. */

/* CFG_NETMASK is the mask used by the protocol stack to decide if a node is
 * on the local network.  If a value of 0 is used, then the protocol stack
 * chooses a mask based on the local host's IP address.  If any other value is
 * used, that value will become the network mask.  Allowing the protocol stack
 * to choose the net work mask is recommended.
 */
#define CFG_NETMASK  0x0L

/* This macro controls whether the code to forward IP packets will be included
   in the library.
*/
#define INCLUDE_IP_FORWARDING       1

/* By default RARP is not included in the Nucleus NET build.  To include RARP
   change the 0 to a 1. See the Nucleus NET reference manual for more
   information on RARP.
*/
#define RARP_INCLUDED       0

/* By default DNS is included in the Nucleus NET build.  To exclude it change
   the 1 below to a 0.  See the Nucleus NET reference Manual for more
   information on DNS.
*/
#define DNS_INCLUDED        1

/* By default DHCP client is included in the Nucleus NET build.  To exclude
   it change the 1 below to a 0. DHCP validate callback and vendor options
   callback can be enabled in a like manner. See the Nucleus NET reference
   Manual for   more information on DHCP.
*/
#define DHCP_INCLUDED                   0
#define DHCP_VALIDATE_CALLBACK		0
#define DHCP_VENDOR_OPTS_CALLBACK	0

/* By default the DHCP client does not wait for the final ACK packet sent
   from the DHCP server.  To wait for the ACK packet change the 1 below to
   a 0.  See the Nucleus NET reference manual for more information on DHCP.
*/
#define DHCP_ACK_NOWAIT                 1

/* By default IP reassembly is included in the Nucleus NET library. Change this
   definition to a 0 to exclude IP reassembly. */
#define INCLUDE_IP_REASSEMBLY           1

/* By default IP fragmentation is included in the Nucleus NET library. Change
   this definition to a 0 to exclude IP fragmentation. */
#define INCLUDE_IP_FRAGMENT             1

/* By default IP Multicasting is included in the Nucleus NET library. Change
   this definition to a 0 to exclude IP Multicasting. */
#define INCLUDE_IP_MULTICASTING         1

/* This macro controls the inclusion of RIP2 in the Nucleus NET library.  By
   default RIP2 is excluded.  Change the definition to a 1 if RIP2 is desired.
   Note that RIP2 is a seperate product and this macro only turns on the
   creation of the RIP2 task. */
#define INCLUDE_RIP2                    0


#define SNMP_INCLUDED       1

#if (SNMP_INCLUDED==0)
#define SNMP_sysDescr(string)
#define SNMP_sysObjectID(string)
#define SNMP_sysUpTime(value)
#define SNMP_sysUpTime_Inc
#define SNMP_sysContact(string)
#define SNMP_sysName(string)
#define SNMP_sysLocation(string)
#define SNMP_sysServices(value)
#define SNMP_atTableUpdate(command, index, phys_addr, net_addr)
#define SNMP_ipInReceives_Inc
#define SNMP_ipInHdrErrors_Inc
#define SNMP_ipInAddrErrors_Inc
#define SNMP_ipForwDatagrams_Inc
#define SNMP_ipInUnknownProtos_Inc
#define SNMP_ipInDiscards_Inc
#define SNMP_ipInDelivers_Inc
#define SNMP_ipOutRequests_Inc
#define SNMP_ipOutDiscards_Inc
#define SNMP_ipOutNoRoutes_Inc
#define SNMP_ipReasmTimeout(value)
#define SNMP_ipReasmReqds_Inc
#define SNMP_ipReasmOKs_Inc
#define SNMP_ipReasmFails_Inc
#define SNMP_ipFragOKs_Inc
#define SNMP_ipFragFails_Inc
#define SNMP_ipFragCreates_Inc
#define SNMP_ipAdEntUpdate(command, index, ip_addr, mask, bcast, reasm_size)
#define SNMP_ipRouteDest(index, dest)
#define SNMP_ipRouteIfIndex(index, value)
#define SNMP_ipRouteMetric1(index, metric)
#define SNMP_ipRouteMetric2(index, metric)
#define SNMP_ipRouteMetric3(index, metric)
#define SNMP_ipRouteMetric4(index, metric)
#define SNMP_ipRouteMetric5(index, metric)
#define SNMP_ipRouteNextHop(index, next)
#define SNMP_ipRouteType(index, type)
#define SNMP_ipRouteProto(index, proto)
#define SNMP_ipRouteAge(index, age)
#define SNMP_ipRouteMask(index, mask)
#define SNMP_ipRouteInfo(index, info)
#define SNMP_ipRoutingDiscards_Inc
#define SNMP_ipNetToMediaTableUpdate(command, index, phys_addr, net_addr, type)
#define SNMP_icmpInMsgs_Inc
#define SNMP_icmpInErrors_Inc
#define SNMP_icmpInDestUnreachs_Inc
#define SNMP_icmpInTimeExcds_Inc
#define SNMP_icmpInParmProbs_Inc
#define SNMP_icmpInSrcQuenchs_Inc
#define SNMP_icmpInRedirects_Inc
#define SNMP_icmpInEchos_Inc
#define SNMP_icmpInEchoReps_Inc
#define SNMP_icmpInTimeStamps_Inc
#define SNMP_icmpInTimeStampReps_Inc
#define SNMP_icmpInAddrMasks_Inc
#define SNMP_icmpInAddrMaskReps_Inc
#define SNMP_icmpOutMsgs_Inc
#define SNMP_icmpOutErrors_Inc
#define SNMP_icmpOutDestUnreachs_Inc
#define SNMP_icmpOutTimeExcds_Inc
#define SNMP_icmpOutParmProbs_Inc
#define SNMP_icmpOutSrcQuenchs_Inc
#define SNMP_icmpOutRedirects_Inc
#define SNMP_icmpOutEchos_Inc
#define SNMP_icmpOutEchoReps_Inc
#define SNMP_icmpOutTimestamps_Inc
#define SNMP_icmpOutTimestampReps_Inc
#define SNMP_icmpOutAddrMasks_Inc
#define SNMP_icmpOutAddrMaskReps_Inc
#define SNMP_tcpRtoAlgorithm(value)
#define SNMP_tcpRtoMin(value)
#define SNMP_tcpRtoMax(value)
#define SNMP_tcpMaxCon(value)
#define SNMP_tcpActiveOpens_Inc
#define SNMP_tcpPassiveOpens_Inc
#define SNMP_tcpAttemptFails_Inc
#define SNMP_tcpEstabResets_Inc
#define SNMP_tcpInSegs_Inc
#define SNMP_tcpOutSegs_Inc
#define SNMP_tcpRetransSegs_Inc
#define SNMP_tcpInErrs_Inc
#define SNMP_tcpOutRsts_Inc
#define SNMP_udpInDatagrams_Inc
#define SNMP_udpNoPorts_Inc
#define SNMP_udpInErrors_Inc
#define SNMP_udpoutDatagrams_Inc
#define SNMP_ifNumber(value)
#define SNMP_ifDescr(index, string)
#define SNMP_ifType(index, value)
#define SNMP_ifMtu(index, value)
#define SNMP_ifSpeed(index, value)
#define SNMP_ifPhysAddress(index, addr)
#define SNMP_ifAdminStatus(index, status)
#define SNMP_ifOperStatus(index, status)
#define SNMP_ifLastChange(index, time)
#define SNMP_ifInOctets(index, value)
#define SNMP_ifInUcastPkts_Inc(index)
#define SNMP_ifInNUcastPkts_Inc(index)
#define SNMP_ifInDiscards_Inc(index)
#define SNMP_ifInErrors_Inc(index)
#define SNMP_ifInUnknownProtos_Inc(index)
#define SNMP_ifOutOctets(index, value)
#define SNMP_ifOutUcastPkts_Inc(index)
#define SNMP_ifOutNUcastPkts_Inc(index)
#define SNMP_ifOutDiscards_Inc(index)
#define SNMP_ifOutErrors_Inc(index)
#define SNMP_ifOutQLen_Inc(index)
#define SNMP_ifSpecific(index, string)
#endif /* SNMP_INCLUDED */

#endif  /* TARGET_H */
