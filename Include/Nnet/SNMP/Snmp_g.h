
#ifndef SNMP_H
#define SNMP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

/* These definitions keep some of XACT's header files from being included.  We
   have some duplicate defintions.
*/
#define  _IN_H_
#define _NETDB_H_

#include "xport.h"
#include "xtypes.h"
#include "..\rfc1213\1213xxxx.h"
#ifdef XMIB_RMON1
#include "..\rfc1757\1757xxxx.h"
#endif
#include "target.h"

extern rfc1213_vars_t  rfc1213_vars;

void xset_1213outQLen                   ( u32 unit, u32 value );
void xset_1213outErrors                 ( u32 unit, u32 value );
void xset_1213outDiscards               ( u32 unit, u32 value );
void xset_1213outNUcastPkts             ( u32 unit, u32 value );
void xset_1213outUcastPkts              ( u32 unit, u32 value );
void xset_1213outOctets                 ( u32 unit, u32 value );
void xset_1213inUcastPkts               ( u32 unit, u32 value );
void xset_1213inNUcastPkts              ( u32 unit, u32 value );
void xset_1213inUnknownProtos   ( u32 unit, u32 value );
void xset_1213inErrors                  ( u32 unit, u32 value );
void xset_1213inDiscards                ( u32 unit, u32 value );
void xset_1213inOctets                  ( u32 unit, u32 value );

void xrmon1_pktcontent( u32 unit, u32 size, u8 *pkt );
u32 xrmon1_pktsize( u32 unit, u32 size );
void xset_1757Octets                            ( u32 unit, u32 value );
void xset_1757Pkts                                      ( u32 unit, u32 value );
void xset_1757BroadcastPkts                     ( u32 unit, u32 value );
void xset_1757MulticastPkts                     ( u32 unit, u32 value );
void xset_1757CRCAlignErrors            ( u32 unit, u32 value );
void xset_1757UndersizePkts                     ( u32 unit, u32 value );
void xset_1757OversizePkts                      ( u32 unit, u32 value );
void xset_1757Fragments                         ( u32 unit, u32 value );
void xset_1757Jabbers                           ( u32 unit, u32 value );
void xset_1757Collisions                        ( u32 unit, u32 value );
void xset_1757Pkts64Octets                      ( u32 unit, u32 value );
void xset_1757Pkts65to127Octets         ( u32 unit, u32 value );
void xset_1757Pkts128to255Octets        ( u32 unit, u32 value );
void xset_1757Pkts256to511Octets        ( u32 unit, u32 value );
void xset_1757Pkts512to1023Octets       ( u32 unit, u32 value );
void xset_1757Pkts1024to1518Octets      ( u32 unit, u32 value );
/* SNMP_Driver_Stats keeps track of all data for an interface.  There should be
   one of these per interface.
*/

typedef struct SNMP_DRIVER_STATS {
    UNSIGNED     index;
	/* rfc1213 */
    UNSIGNED     out_octets;
    UNSIGNED     out_mcasts;
    UNSIGNED     out_ucasts;
    UNSIGNED     out_discards;
    UNSIGNED     out_errors;
    UNSIGNED     out_qlen;
    UNSIGNED     out_colls;

	/* rfc1213 */
    UNSIGNED     in_octets;
    UNSIGNED     in_mcasts;
    UNSIGNED     in_ucasts;
    UNSIGNED     in_discards;
    UNSIGNED     in_errors;
    UNSIGNED     in_noproto;

	/* rfc1757 */
#ifdef XMIB_RMON1
    UNSIGNED     octets;
    UNSIGNED     pkts;
    UNSIGNED     bcast;
    UNSIGNED     mcast;
    UNSIGNED     crcalign;
    UNSIGNED     runt;
    UNSIGNED     giant;
    UNSIGNED     frags;
    UNSIGNED     jabber;
    UNSIGNED     colls;
    UNSIGNED     pkts64;
    UNSIGNED     pkts65to127;
    UNSIGNED     pkts128to255;
    UNSIGNED     pkts256to511;
    UNSIGNED     pkts512to1023;
    UNSIGNED     pkts1024to1518;
#endif
} SNMP_DRIVER_STATS;

extern SNMP_DRIVER_STATS   SNMP_Drv_Stats[MAX_1213_IF];

/* Function Prototypes. */
STATUS SNMP_Initialize(VOID);
STATUS SNMP_System_Group_Initialize(rfc1213_sys_t *sys_group);
STATUS SNMP_atTableUpdate(INT command, INT index, uint8 *phys_addr,
			    uint8 *net_addr);
STATUS SNMP_ipAdEntUpdate(INT command, INT index, uint8 *ip_addr, uint8 *mask,
			    INT bcast, INT reasm_size);
STATUS SNMP_ipRouteTableUpdate(INT command, INT index, uint8 *dest,
			UNSIGNED metric1, UNSIGNED metric2, UNSIGNED metric3,
			UNSIGNED metric4, UNSIGNED metric5, uint8 *next_hop,
			UNSIGNED type, UNSIGNED proto, UNSIGNED age,
			uint8 *mask, CHAR *info);
STATUS SNMP_ipNetToMediaTableUpdate(INT command, UNSIGNED index,
				    uint8 *phys_addr, uint8 *net_addr,
				    UNSIGNED type);
STATUS SNMP_udpListenTableUpdate(INT command, uint8 *addr, UNSIGNED port);
STATUS  SNMP_tcpConnTableUpdate(INT command, UNSIGNED state, uint8 *local_addr,
				UNSIGNED local_port, uint8 *rem_addr,
				UNSIGNED rem_port);

#define SNMP_System_Group   rfc1213_sys_t

/* Define service completion status constants.  */

#define SNMP_SUCCESS                NU_SUCCESS
#define SNMP_INVALID_POINTER        NU_INVALID_POINTER
#define SNMP_INVALID_PARAMETER      -100

/* Command definitions. */
#define     SNMP_ADD            1
#define     SNMP_DELETE         2

/* Definitions related to the IP group of MIB2. */
#define RFC1213_IP_FORWARD          1
#define RFC1213_IP_NO_FORWARD       2

/* Definitions for the interface group of MIB2. */
#define RFC1213_IF_UP               1
#define RFC1213_IF_DOWN             2
#define RFC1213_IF_TESTING          3

/* These macros are for updating the System Group variables. */
#define SNMP_sysDescr(string)       \
    strcpy((char *)rfc1213_vars.rfc1213_sys.sysDescr, (char *)string)

#define SNMP_sysObjectID(string)    \
    strcpy((char *)rfc1213_vars.rfc1213_sys.sysObjectID, (char *)string)

#define SNMP_sysUpTime(value)       \
    rfc1213_vars.rfc1213_sys.sysUpTime = value

#define SNMP_sysUpTime_Inc    \
    rfc1213_vars.rfc1213_sys.sysUpTime++

#define SNMP_sysContact(string)     \
    strcpy((char *)rfc1213_vars.rfc1213_sys.sysContact, (char *)string)

#define SNMP_sysName(string)        \
    strcpy((char *)rfc1213_vars.rfc1213_sys.sysName, (char *)string)

#define SNMP_sysLocation(string)    \
    strcpy((char *)rfc1213_vars.rfc1213_sys.sysLocation, (char *)string)

#define SNMP_sysServices(value)     \
    rfc1213_vars.rfc1213_sys.sysServices = value


/* These macros are for updating the IP Group. */
#define SNMP_ipForwarding(value)     \
    rfc1213_vars.rfc1213_ip.ipForwarding = value

#define SNMP_ipDefaultTTL(value)     \
    rfc1213_vars.rfc1213_ip.ipDefaultTTL = value

#define SNMP_ipInReceives_Inc     \
    rfc1213_vars.rfc1213_ip.ipInReceives++

#define SNMP_ipInHdrErrors_Inc    \
    rfc1213_vars.rfc1213_ip.ipInHdrErrors++

#define SNMP_ipInAddrErrors_Inc   \
    rfc1213_vars.rfc1213_ip.ipInAddrErrors++

#define SNMP_ipForwDatagrams_Inc  \
    rfc1213_vars.rfc1213_ip.ipForwDatagrams++

#define SNMP_ipInUnknownProtos_Inc  \
    rfc1213_vars.rfc1213_ip.ipInUnknownProtos++

#define SNMP_ipInDiscards_Inc  \
    rfc1213_vars.rfc1213_ip.ipInDiscards++

#define SNMP_ipInDelivers_Inc  \
    rfc1213_vars.rfc1213_ip.ipInDelivers++

#define SNMP_ipOutRequests_Inc  \
    rfc1213_vars.rfc1213_ip.ipOutRequests++

#define SNMP_ipOutDiscards_Inc  \
    rfc1213_vars.rfc1213_ip.ipOutDiscards++

#define SNMP_ipOutNoRoutes_Inc  \
    rfc1213_vars.rfc1213_ip.ipOutNoRoutes++

#define SNMP_ipReasmTimeout(value)  \
    rfc1213_vars.rfc1213_ip.ipReasmTimeout = value

#define SNMP_ipReasmReqds_Inc  \
    rfc1213_vars.rfc1213_ip.ipReasmReqds++

#define SNMP_ipReasmOKs_Inc  \
    rfc1213_vars.rfc1213_ip.ipReasmOKs++

#define SNMP_ipReasmFails_Inc  \
    rfc1213_vars.rfc1213_ip.ipReasmFails++

#define SNMP_ipFragOKs_Inc  \
    rfc1213_vars.rfc1213_ip.ipFragOKs++

#define SNMP_ipFragFails_Inc  \
    rfc1213_vars.rfc1213_ip.ipFragFails++

#define SNMP_ipFragCreates_Inc  \
    rfc1213_vars.rfc1213_ip.ipFragCreates++

#define SNMP_ipRoutingDiscards_Inc \
	rfc1213_vars.rfc1213_ip.ipRoutingDiscards++



/* These macros are for updating the ICMP Group. */
#define SNMP_icmpInMsgs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInMsgs++

#define SNMP_icmpInErrors_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInErrors++

#define SNMP_icmpInDestUnreachs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInDestUnreachs++

#define SNMP_icmpInTimeExcds_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInTimeExcds++

#define SNMP_icmpInParmProbs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInParmProbs++

#define SNMP_icmpInSrcQuenchs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInSrcQuenchs++

#define SNMP_icmpInRedirects_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInRedirects++

#define SNMP_icmpInEchos_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInEchos++

#define SNMP_icmpInEchoReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInEchoReps++

#define SNMP_icmpInTimeStamps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInTimestamps++

#define SNMP_icmpInTimeStampReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInTimestampReps++

#define SNMP_icmpInAddrMasks_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInAddrMasks++

#define SNMP_icmpInAddrMaskReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpInAddrMaskReps++

#define SNMP_icmpOutMsgs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutMsgs++

#define SNMP_icmpOutErrors_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutErrors++

#define SNMP_icmpOutDestUnreachs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutDestUnreachs++

#define SNMP_icmpOutTimeExcds_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutTimeExcds++

#define SNMP_icmpOutParmProbs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutParmProbs++

#define SNMP_icmpOutSrcQuenchs_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutSrcQuenchs++

#define SNMP_icmpOutRedirects_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutRedirects++

#define SNMP_icmpOutEchos_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutEchos++

#define SNMP_icmpOutEchoReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutEchoReps++

#define SNMP_icmpOutTimestamps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutTimestamps++

#define SNMP_icmpOutTimestampReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutTimestampReps++

#define SNMP_icmpOutAddrMasks_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutAddrMasks++

#define SNMP_icmpOutAddrMaskReps_Inc   \
	rfc1213_vars.rfc1213_icmp.icmpOutAddrMaskReps++



/* These macros are for updating the TCP Group. */
#define SNMP_tcpRtoAlgorithm(value) \
	rfc1213_vars.rfc1213_tcp.tcpRtoAlgorithm = value

/* value is the min RTO in ticks. */
#define SNMP_tcpRtoMin(value) \
	rfc1213_vars.rfc1213_tcp.tcpRtoMin = ((value * 1000) / TICKS_PER_SECOND)

#define SNMP_tcpRtoMax(value) \
	rfc1213_vars.rfc1213_tcp.tcpRtoMax = ((value * 1000) / TICKS_PER_SECOND)

#define SNMP_tcpMaxCon(value)   \
	rfc1213_vars.rfc1213_tcp.tcpMaxConn = value

#define SNMP_tcpActiveOpens_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpActiveOpens++

#define SNMP_tcpPassiveOpens_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpPassiveOpens++

#define SNMP_tcpAttemptFails_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpAttemptFails++

#define SNMP_tcpEstabResets_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpEstabResets++

#define SNMP_tcpInSegs_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpInSegs++

#define SNMP_tcpOutSegs_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpOutSegs++

#define SNMP_tcpRetransSegs_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpRetransSegs++

#define SNMP_tcpInErrs_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpInErrs++

#define SNMP_tcpOutRsts_Inc   \
	rfc1213_vars.rfc1213_tcp.tcpOutRsts++



/* These macros are for updating the UDP Group. */

#define SNMP_udpInDatagrams_Inc   \
	rfc1213_vars.rfc1213_udp.udpInDatagrams++

#define SNMP_udpNoPorts_Inc   \
	rfc1213_vars.rfc1213_udp.udpNoPorts++

#define SNMP_udpInErrors_Inc   \
	rfc1213_vars.rfc1213_udp.udpInErrors++

#define SNMP_udpoutDatagrams_Inc   \
	rfc1213_vars.rfc1213_udp.udpOutDatagrams++



/* These macros are for updating the Interface Group. */

#define SNMP_ifNumber(value)    \
	rfc1213_vars.rfc1213_ifNumber = value

#define SNMP_ifDescr(index, string)    \
	strcpy(rfc1213_vars.rfc1213_if[index].ifDescr, string)

#define SNMP_ifType(index, value)  \
	rfc1213_vars.rfc1213_if[index].ifType = value

#define SNMP_ifMtu(index, value)  \
	rfc1213_vars.rfc1213_if[index].ifMtu = value

#define SNMP_ifSpeed(index, value)  \
	rfc1213_vars.rfc1213_if[index].ifSpeed = value

#define SNMP_ifPhysAddress(index, addr)  \
	memcpy(rfc1213_vars.rfc1213_if[index].ifPhysAddress, addr, \
	       MAX_1213_PADDRSIZE)

#define SNMP_ifAdminStatus(index, status) \
	rfc1213_vars.rfc1213_if[index].ifAdminStatus = status

#define SNMP_ifOperStatus(index, status) \
	rfc1213_vars.rfc1213_if[index].ifOperStatus = status

#define SNMP_ifLastChange(index, time) \
	rfc1213_vars.rfc1213_if[index].ifLastChange = time

#define SNMP_ifInOctets(index, value) \
	xset_1213inOctets(index, (SNMP_Drv_Stats[index].in_octets += value))

#define SNMP_ifInUcastPkts_Inc(index) \
	xset_1213inUcastPkts(index, ++SNMP_Drv_Stats[index].in_ucasts)

#define SNMP_ifInNUcastPkts_Inc(index) \
	xset_1213inNUcastPkts(index, ++SNMP_Drv_Stats[index].in_mcasts)

#define SNMP_ifInDiscards_Inc(index) \
	xset_1213inDiscards(index, ++SNMP_Drv_Stats[index].in_discards)

#define SNMP_ifInErrors_Inc(index) \
	xset_1213inErrors(index, ++SNMP_Drv_Stats[index].in_errors)

#define SNMP_ifInUnknownProtos_Inc(index) \
	xset_1213inUnknownProtos(index, ++SNMP_Drv_Stats[index].in_noproto)

#define SNMP_ifOutOctets(index, value) \
	xset_1213outOctets(index, (SNMP_Drv_Stats[index].out_octets += value))

#define SNMP_ifOutUcastPkts_Inc(index) \
	xset_1213outUcastPkts(index, ++SNMP_Drv_Stats[index].out_ucasts)

#define SNMP_ifOutNUcastPkts_Inc(index) \
	xset_1213outNUcastPkts(index, ++SNMP_Drv_Stats[index].out_mcasts)

#define SNMP_ifOutDiscards_Inc(index) \
	xset_1213outDiscards(index, ++SNMP_Drv_Stats[index].out_discards)

#define SNMP_ifOutErrors_Inc(index) \
	xset_1213outErrors(index, ++SNMP_Drv_Stats[index].out_errors)

#define SNMP_ifOutQLen_Inc(index) \
	xset_1213outQLen(index, ++SNMP_Drv_Stats[index].out_qlen)

#define SNMP_ifOutQLen_Dec(index) \
	xset_1213outQLen(index, --SNMP_Drv_Stats[index].out_qlen)

#define SNMP_ifSpecific(index, string) \
	strcpy(rfc1213_vars.rfc1213_if[index].ifSpecific, string)

#ifdef XMIB_RMON1
#define RMON_Octets(index, value) \
	xset_1757Octets(index, (SNMP_Drv_Stats[index].octets += value))

#define RMON_Pkts_Inc(index) \
	xset_1757Pkts(index, ++SNMP_Drv_Stats[index].pkts)

#define RMON_BroadcastPkts_Inc(index) \
	xset_1757BroadcastPkts(index, ++SNMP_Drv_Stats[index].bcast)

#define RMON_MulticastPkts_Inc(index) \
	xset_1757MulticastPkts(index, ++SNMP_Drv_Stats[index].mcast)

#define RMON_CRCAlignErrors_Inc(index) \
	xset_1757CRCAlignErrors(index, ++SNMP_Drv_Stats[index].crcalign)

#define RMON_UndersizePkts_Inc(index) \
	xset_1757UndersizePkts(index, ++SNMP_Drv_Stats[index].runt)

#define RMON_OversizePkts_Inc(index) \
	xset_1757OversizePkts(index, ++SNMP_Drv_Stats[index].giant)

#define RMON_Fragments_Inc(index) \
	xset_1757Fragments(index, ++SNMP_Drv_Stats[index].frags)

#define RMON_Jabbers_Inc(index) \
	xset_1757Jabbers(index, ++SNMP_Drv_Stats[index].jabber)

#define RMON_Collisions_Inc(index) \
	xset_1757Collisions(index, ++SNMP_Drv_Stats[index].colls)

#define RMON_Pkts64Octets_Inc(index) \
	xset_1757Pkts64Octets(index, ++SNMP_Drv_Stats[index].pkts64)

#define RMON_Pkts65to127Octets_Inc(index) \
	xset_1757Pkts65to127Octets(index, ++SNMP_Drv_Stats[index].pkts65to127)

#define RMON_Pkts128to255Octets_Inc(index) \
	xset_1757Pkts128to255Octets(index, ++SNMP_Drv_Stats[index].pkts128to255)

#define RMON_Pkts256to511Octets_Inc(index) \
	xset_1757Pkts256to511Octets(index, ++SNMP_Drv_Stats[index].pkts256to511)

#define RMON_Pkts512to1023Octets_Inc(index) \
	xset_1757Pkts512to1023Octets(index, ++SNMP_Drv_Stats[index].pkts512to1023)

#define RMON_Pkts1024to1518Octets_Inc(index) \
	xset_1757Pkts1024to1518Octets(index, ++SNMP_Drv_Stats[index].pkts1024to1518)

#endif /* XMIB_RMON1 */

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* SNMP_H */
