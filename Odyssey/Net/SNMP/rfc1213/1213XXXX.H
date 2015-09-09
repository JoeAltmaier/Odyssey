/*@*********************************************************************** 
 |                                                                         
 |             Copyright (c) 1995-1997 XACT Incporated                     
 |                                                                         
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject     
 | matter of this material.  All manufacturing, reproduction, use, and     
 | sales rights pertaining to this subject matter are governed by the      
 | license agreement.  The recipient of this software implicitly accepts   
 | the terms of the license.                                               
 |                                                                         
 |                                                                         
 | FILE NAME   : 1213xxxx.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for RFC 1213 Groups
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213XXXX_H_
#define _1213XXXX_H_
/*
 * Typedefs used by the RFC1213 structure.
 * Defined here for portability.  Use of local typedefs
 * should conform to the following definitions
 */
typedef unsigned long			x_u32;		/* unsigned 32 bit */
typedef unsigned char			x_u8;		/* unsigned  8 bit */
typedef signed char				x_i8;		/* signed    8 bit */
typedef long					x_l32;		/* signed   32 bit */
typedef unsigned short			x_u16;		/* unsigned 16 bit */
typedef signed long				x_i32;		/* signed   32 bit */

/*
 * Miscellaneous defines.  These defines should be migrated to
 * a "config structure" so that a particular environment can
 * be set for the stack/mib/snmp
 * For now, these defines are sufficient.
 */
#define MAX_1213_STRSIZE		256
#define MAX_1213_BUFINT			128
#define MAX_1213_OIDSIZE		32
#define MAX_1213_IF				MAX_PORTS 
#define MAX_1213_PADDRSIZE		6
#define MAX_1213_NETADDRSIZE	4
#define MAX_1213_IPS			4
#define MAX_1213_ROUTES			2
#define MAX_1213_TCPCONNS		2
#define MAX_1213_UDPLISTEN		2
#define MAX_1213_EGPNEIGH		2

/*
 * The system group
 */
typedef struct rfc1213_sys_s {
	x_i8					sysDescr[MAX_1213_STRSIZE];
	x_l32					sysObjectID[MAX_1213_BUFINT];     
	x_u16					sysObjectIDLen;
	x_u16					pad;
	x_l32					sysUpTime;
	x_i8					sysContact[MAX_1213_STRSIZE];
	x_i8					sysName[MAX_1213_STRSIZE];
	x_i8					sysLocation[MAX_1213_STRSIZE];
	x_i32					sysServices;
} rfc1213_sys_t;

/*
 * The Interface group
 */
typedef struct rfc1213_if_s {
	x_u8					ifDescr[MAX_1213_STRSIZE];
	x_u32					ifType;
	x_u32					ifMtu;
	x_u32					ifSpeed;
	x_u8					ifPhysAddress[MAX_1213_PADDRSIZE];
	x_u8					pad[2];
	x_u32					ifAdminStatus;
	x_u32					ifOperStatus;
	x_u32					ifLastChange;
	x_u32					ifInOctets;    
	x_u32					ifInUcastPkts;
	x_u32					ifInNUcastPkts;
	x_u32					ifInDiscards;
	x_u32					ifInErrors;
	x_u32					ifInUnknownProtos;
	x_u32					ifOutOctets;
	x_u32					ifOutUcastPkts;
	x_u32					ifOutNUcastPkts;
	x_u32					ifOutDiscards;
	x_u32					ifOutErrors;
	x_u32					ifOutQLen;
	x_u8					ifSpecific[MAX_1213_STRSIZE];
} rfc1213_if_t;

/*
 * The Address Translation group
 */
typedef struct rfc1213_at_s {
	x_u32					atIfIndex;
	x_u8					atPhysAddress[MAX_1213_PADDRSIZE];
	x_u8					pad[2];
	x_u8					atNetAddress[MAX_1213_NETADDRSIZE];
	struct rfc1213_at_s		*next;
	struct rfc1213_at_s		*last;
} rfc1213_at_t;

typedef struct ipaddrtab_s {
	x_u8					ipAdEntAddr[MAX_1213_NETADDRSIZE];
	x_u32					ipAdEntIfIndex;
	x_u8					ipAdEntNetMask[MAX_1213_NETADDRSIZE];
	x_u32					ipAdEntBcastAddr;
	x_u32					ipAdEntReasmMaxSize;
	struct ipaddrtab_s		*next;
	struct ipaddrtab_s		*last;
} ipaddrtab_t;

typedef struct iproutetab_s {
	x_u8					ipRouteDest[MAX_1213_NETADDRSIZE];
	x_u32					ipRouteIfIndex;
	x_u32					ipRouteMetric1;
	x_u32					ipRouteMetric2;
	x_u32					ipRouteMetric3;
	x_u32					ipRouteMetric4;
	x_u8					ipRouteNextHop[MAX_1213_NETADDRSIZE];
	x_u32					ipRouteType;
	x_u32					ipRouteProto;
	x_u32					ipRouteAge;
	x_u8					ipRouteMask[MAX_1213_NETADDRSIZE];
	x_u32					ipRouteMetric5;
	x_u8					ipRouteInfo[MAX_1213_STRSIZE];
	struct iproutetab_s		*next;
	struct iproutetab_s		*last;
} iproutetab_t;

typedef struct ipnet2media_s {
	x_u32					ipNetToMediaIfIndex;
	x_u8					ipNetToMediaPhysAddress[MAX_1213_PADDRSIZE];
	x_u8					pad[2];
	x_u8					ipNetToMediaNetAddress[MAX_1213_NETADDRSIZE];
	x_u32					ipNetToMediaType;
	struct ipnet2media_s	*next;
	struct ipnet2media_s	*last;
} ipnet2media_t;

/*
 * The IP Group
 */
typedef struct rfc1213_ip_s {
	x_u32					ipForwarding;
	x_u32					ipDefaultTTL;
	x_u32					ipInReceives;
	x_u32					ipInHdrErrors;
	x_u32					ipInAddrErrors;
	x_u32					ipForwDatagrams;
	x_u32					ipInUnknownProtos;
	x_u32					ipInDiscards;
	x_u32					ipInDelivers;
	x_u32					ipOutRequests;
	x_u32					ipOutDiscards;
	x_u32					ipOutNoRoutes;
	x_u32					ipReasmTimeout;
	x_u32					ipReasmReqds;
	x_u32					ipReasmOKs;
	x_u32					ipReasmFails;
	x_u32					ipFragOKs;
	x_u32					ipFragFails;
	x_u32					ipFragCreates;
	ipaddrtab_t				*ipAddrTab;    
	iproutetab_t			*ipRouteTab;
	ipnet2media_t			*ipNet2MediaTab;
	x_u32					ipRoutingDiscards;
} rfc1213_ip_t;

/*
 * The ICMP Group
 */
typedef struct rfc1213_icmp_s {
	x_u32					icmpInMsgs;
	x_u32					icmpInErrors;
	x_u32					icmpInDestUnreachs;
	x_u32					icmpInTimeExcds;
	x_u32					icmpInParmProbs;
	x_u32					icmpInSrcQuenchs;
	x_u32					icmpInRedirects;
	x_u32					icmpInEchos;
	x_u32					icmpInEchoReps;
	x_u32					icmpInTimestamps;
	x_u32					icmpInTimestampReps;
	x_u32					icmpInAddrMasks;
	x_u32					icmpInAddrMaskReps;
	x_u32					icmpOutMsgs;
	x_u32					icmpOutErrors;
	x_u32					icmpOutDestUnreachs;
	x_u32					icmpOutTimeExcds;
	x_u32					icmpOutParmProbs;
	x_u32					icmpOutSrcQuenchs;
	x_u32					icmpOutRedirects;
	x_u32					icmpOutEchos;
	x_u32					icmpOutEchoReps;
	x_u32					icmpOutTimestamps;
	x_u32					icmpOutTimestampReps;
	x_u32					icmpOutAddrMasks;
	x_u32					icmpOutAddrMaskReps;
} rfc1213_icmp_t;

typedef struct tcpcontab_s {
	x_u32					tcpConnState;
	x_u8					tcpConnLocalAddress[MAX_1213_NETADDRSIZE];
	x_u32					tcpConnLocalPort;
	x_u8					tcpConnRemAddress[MAX_1213_NETADDRSIZE];
	x_u32					tcpConnRemPort;
	struct tcpcontab_s		*next;
	struct tcpcontab_s		*last;
} tcpcontab_t;

/*
 * The TCP group
 */
typedef struct rfc1213_tcp_s {
	x_u32					tcpRtoAlgorithm;
	x_u32					tcpRtoMin;
	x_u32					tcpRtoMax;
	x_u32					tcpMaxConn;
	x_u32					tcpActiveOpens;
	x_u32					tcpPassiveOpens;
	x_u32					tcpAttemptFails;
	x_u32					tcpEstabResets;
	x_u32					tcpCurrEstab;
	x_u32					tcpInSegs;
	x_u32					tcpOutSegs;
	x_u32					tcpRetransSegs;
	tcpcontab_t				*tcpConnTab;    
	x_u32					tcpInErrs;
	x_u32					tcpOutRsts;
} rfc1213_tcp_t;

typedef struct udplisttab_s {
	x_u8					udpLocalAddress[MAX_1213_NETADDRSIZE];
	x_u32					udpLocalPort;
	struct udplisttab_s		*next;
	struct udplisttab_s		*last;
} udplisttab_t;

/*
 * The UDP group
 */
typedef struct rfc1213_udp_s {
	x_u32					udpInDatagrams;
	x_u32					udpNoPorts;
	x_u32					udpInErrors;
	x_u32					udpOutDatagrams;
	udplisttab_t			*udpListTab;
} rfc1213_udp_t;

typedef struct egpneightab_s {
	x_u32					egpNeighState;
	x_u8					egpNeighAddr[MAX_1213_NETADDRSIZE];
	x_u32					egpNeighAs;
	x_u32					egpNeighInMsgs;
	x_u32					egpNeighInErrs;
	x_u32					egpNeighOutMsgs;
	x_u32					egpNeighOutErrs;
	x_u32					egpNeighInErrMsgs;
	x_u32					egpNeighOutErrMsgs;
	x_u32					egpNeighStateUps;
	x_u32					egpNeighStateDowns;
	x_u32					egpNeighIntervalHello;
	x_u32					egpNeighIntervalPoll;
	x_u32					egpNeighMode;
	x_u32					egpNeighEventTrigger;
	struct egpneightab_s	*next;
	struct egpneightab_s	*last;
} egpneightab_t;

/*
 * The EGP Group
 * Only if you have Exterior Gateway Protocol
 */
typedef struct rfc1213_egp_s {
	x_u32					egpInMsgs;
	x_u32					egpInErrors;
	x_u32					egpOutMsgs;
	x_u32					egpOutErrors;
	egpneightab_t			*egpNeighTab;
	x_u32					egpAs;
} rfc1213_egp_t;

/*
 * The Transmission Group
 * This info is media specific
 */
typedef struct rfc1213_trans_s {
	x_l32					transNumber[MAX_1213_OIDSIZE];
	x_u16					transLen;
} rfc1213_trans_t;

/*
 * The SNMP Group
 * These stats are provided by the SNMP agent
 */
typedef struct rfc1213_snmp_s {
	x_u32					snmpInPkts;
	x_u32					snmpOutPkts;
	x_u32					snmpInBadVersions;
	x_u32					snmpInBadCommunityNames;
	x_u32					snmpInBadCommunityUses;
	x_u32					snmpInASNParseErrs;
	x_u32					snmpInTooBigs;
	x_u32					snmpInNoSuchNames;
	x_u32					snmpInBadValues;
	x_u32					snmpInReadOnlys;
	x_u32					snmpInGenErrs;
	x_u32					snmpInTotalReqVars;
	x_u32					snmpInTotalSetVars;
	x_u32					snmpInGetRequests;
	x_u32					snmpInGetNexts;
	x_u32					snmpInSetRequests;
	x_u32					snmpInGetResponses;
	x_u32					snmpInTraps;
	x_u32					snmpOutTooBigs;
	x_u32					snmpOutNoSuchNames;
	x_u32					snmpOutBadValues;
	x_u32					snmpOutGenErrs;
	x_u32					snmpOutGetRequests;
	x_u32					snmpOutGetNexts;
	x_u32					snmpOutSetRequests;
	x_u32					snmpOutGetResponses;
	x_u32					snmpOutTraps;
	x_u32					snmpEnableAuthenTraps;
} rfc1213_snmp_t;

/*
 * MIBII Group/Object Container
 */
typedef struct rfc1213_vars_s {
	rfc1213_sys_t			rfc1213_sys;
	x_u32					rfc1213_ifNumber; 
	rfc1213_if_t			rfc1213_if[MAX_1213_IF];
	rfc1213_at_t			*rfc1213_at;
	rfc1213_ip_t			rfc1213_ip;
	rfc1213_icmp_t			rfc1213_icmp;
	rfc1213_tcp_t			rfc1213_tcp;
	rfc1213_udp_t			rfc1213_udp;
	rfc1213_egp_t			rfc1213_egp;
	rfc1213_trans_t			rfc1213_trans;
	rfc1213_snmp_t			rfc1213_snmp;
} rfc1213_vars_t;


#endif
