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
 | FILE NAME   : xsnmp.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Task entry to XSNMPv1
 | AUTHOR      : Robert Winter
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "bool.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "ifudp.h"
#include "udp.h"
#include "mib.h"
#include "ring.h"
#include "hash.h"

#include "1213at.h"
#include "1213egp.h"
#include "1213icmp.h"
#include "1213if.h"
#include "1213ip.h"
#include "1213snmp.h"
#include "1213sys.h"
#include "1213tcp.h"
#include "1213udp.h"
#include "1213xxxx.h"

#ifdef XMIB_RMON1
#include "1757stat.h"
#include "1757log.h"
#include "1757evnt.h"
#include "1757alrm.h"
#include "1757chan.h"
#include "1757filt.h"
#include "1757cap.h"
#include "1757host.h"
#include "1757matx.h"
#include "1757topn.h"
#include "1757hist.h"
#include "1757xxxx.h"
#endif

void UdpRcve( i8 *addr, u8 *frame, i32 length, void *parm );

#ifdef X_XACT
#include "1213_xact.h"
#endif

/*
 * The MIB (s)
 */
mib_element_t xsnmpObj[] = {
#include "1213oid.h"  		/* RFC 1213, MIB-II 							*/
#ifdef XMIB_RMON1
#include "1757oid.h"		/* RFC 1757, RMON version 1                     */
#endif
#ifdef X_XACT
#include "xact_oid.h" 		/* Xact Sample Enterprise Configuration MIB 	*/
#endif
};

#ifdef XMIB_RMON1
extern bool Rmon1Init(void);
#endif

extern rfc1213_vars_t	rfc1213_vars;

i8  								xsnmp_buf[XSNMP_BUFSIZE];
i32									xsnmp_fromlen, n;
i32									xsnmp_initialized = 0;
xsnmp_cfig_t						xsnmp_cfg;
struct	sockaddr_in 				xsnmp_ctl_addr = { 4, AF_INET };
struct	sockaddr_in 				xsnmp_from;
i32           						xsnmp_ctl;
extern i32 nwUdpSocket;

mac_iface_t							*gmac[MAX_PORTS];

void 								xsnmp_update(void);

extern udp_descr_t					agentUdp;
extern i8 							fw_rev_level[];
extern bool 						sn_mac_stats(mac_iface_t *f, mac_stat_t *s);
extern void							get_system_contact(u8 *buf);
extern void							get_system_description(u8 *buf);
extern void							get_system_location(u8 *buf);
extern void							get_system_name(u8 *buf);
extern void							get_system_objectid(u8 *buf);
extern i32  						get_system_services(void);
extern u32  						get_authentrap(void);
extern u32  						get_coldstarttrap(void);
extern u32  						get_snmpport(void);

#ifdef XMIB_RMON1
extern u32  						get_maxbuff(void);
extern u32  						get_maxHTS(void);
extern u32  						get_maxMTS(void);
extern u32  						get_maxbuckets(void);
extern u32  						get_maxlogs(void);
#endif

//extern u32                          get_numports(void);
extern void							get_macaddr(i32 port, u8 *m);
extern void							get_macbroad(i32 port, u8 *m);
extern u32 							get_hostid(void);
extern void							AddMac(i8 *name, u32 type, u16 mtu, ul32 speed, u8 *addr, u8 *addrbroad, u16 addrsize, u8 *host, u16 hostaddrsize);
extern void							nc_init(void);
extern void							cc_init(void);
extern void							sc_init(void);
extern void							hc_init(void);
extern bool							Mib2Init(void);
extern bool							BaseMibInit(void);
extern void							SendENTTrap(u16 gen, u16 spec, snmp_object_t *list, u16 listLen);
extern void							SetUDPListener(ul32 addr, u16 port);
extern i8							*Sock2Addr(struct sockaddr_in *sa);
extern void							UupRcve(nw_dg_t *dg, i8 *addr, u8 *frame, i32 length, void *parm);

/*
 * XSNMP init
 */
void
xsnmp_init(void)
{
static u8 	buf[MAX_SYS_STRINGS+ 1];
static u8 	mac[MAC_ETHER_ADDR_LEN +1];
static u8 	broad[MAC_ETHER_ADDR_LEN +1];
u32	host;
i32	i,numports;

	x_bzero( (i8 *)buf, MAX_SYS_STRINGS+1 );
	get_system_contact( buf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_contact[0]), (i8 *)buf, MAX_SYS_STRINGS+1 );
	x_bzero( (i8 *)buf, MAX_SYS_STRINGS+1 );
	get_system_description( buf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_description[0]), (i8 *)buf, MAX_SYS_STRINGS+1 );
	x_bzero( (i8 *)buf, MAX_SYS_STRINGS + 1 );
	get_system_location( buf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_location[0]), (i8 *)buf, MAX_SYS_STRINGS+1 );
	x_bzero( (i8 *)buf, MAX_SYS_STRINGS + 1 );
	get_system_name( buf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_name[0]), (i8 *)buf, MAX_SYS_STRINGS+1 );
	x_bzero( (i8 *)buf, MAX_SYS_STRINGS + 1 );
	get_system_objectid( buf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_objectid[0]), (i8 *)buf, MAX_SYS_STRINGS+1 );

	xsnmp_cfg.sys_services 					= get_system_services();
	xsnmp_cfg.authentrap_enable				= get_authentrap();
	xsnmp_cfg.coldtrap_enable				= get_coldstarttrap();
	xsnmp_cfg.local_port					= get_snmpport();

#ifdef XMIB_RMON1
	xsnmp_cfg.cbuff_size					= get_maxbuff();
	xsnmp_cfg.host_maxnrhosts				= get_maxHTS();
	xsnmp_cfg.matrix_maxnrsrcdsts			= get_maxMTS();
	xsnmp_cfg.hist_maxnrbuckets				= get_maxbuckets();
	xsnmp_cfg.event_maxnrlogs				= get_maxlogs();
	xsnmp_cfg.disc_maxnrnodes				= 10000L;
	xsnmp_cfg.disc_nodetimeout				= 10000L;
#endif

//#ifndef XSTK_NUCNET
    /*
	 * seed the mac_comm entries in xcfig
 	 * Communities must be added before hosts in those communities
	 *
     */
	xsnmp_cfg.mac_iface = 0;

    /*numports = get_numports();
	for( i=0; i<numports; i++ ) {
		get_macaddr( i, mac );
		get_macbroad( i, broad );
		host = get_hostid();
#ifdef XLIB_XSNMP
		plist[0].intval = i+1;
		x_sprintf((i8 *) buf, "XSNMPv1_if%d\n");
#else
		sprintf((i8 *) buf, "XSNMPv1_if%d\n",i+1 );
#endif
        AddMac( (i8 *)buf, MAC_TYPE_ETHERNET_CSMACD, 1500L, 10000000L, mac, broad, 6L, (u8 *)&host, 4L);
    }*/

    for( i=0; i < MAX_PORTS; i++ ) {
		get_macaddr( i, mac );
		get_macbroad( i, broad );
		host = get_hostid();
#ifdef XLIB_XSNMP
		plist[0].intval = i+1;
		x_sprintf((i8 *) buf, "XSNMPv1_if%d\n");
#else
		sprintf((i8 *) buf, "XSNMPv1_if%d\n",i+1 );
#endif
        AddMac( (i8 *)buf, MAC_TYPE_OTHER, 0L, 0L, 0L, 0L, 6L, (u8 *)&host, 4L);
    }
//#endif /* XSTK_NUC */

}

/*
 * XSNMP Update
 */
void
xsnmp_update(void)
{
static u8 Updatebuf[MAX_SYS_STRINGS+ 1];

	x_bzero( (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	get_system_contact( Updatebuf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_contact[0]), (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	x_bzero( (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	get_system_description( Updatebuf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_description[0]), (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	x_bzero( (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	get_system_location( Updatebuf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_location[0]), (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	x_bzero( (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	get_system_name( Updatebuf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_name[0]), (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	x_bzero( (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );
	get_system_objectid( Updatebuf );
	x_memcpy( (i8 *)(&xsnmp_cfg.sys_objectid[0]), (i8 *)Updatebuf, MAX_SYS_STRINGS + 1 );

	xsnmp_cfg.sys_services 					= get_system_services();

#ifdef XMIB_RMON1
	xsnmp_cfg.cbuff_size					= get_maxbuff();
	xsnmp_cfg.host_maxnrhosts				= get_maxHTS();
	xsnmp_cfg.matrix_maxnrsrcdsts			= get_maxMTS();
	xsnmp_cfg.hist_maxnrbuckets				= get_maxbuckets();
	xsnmp_cfg.event_maxnrlogs				= get_maxlogs();
	xsnmp_cfg.disc_maxnrnodes				= 10000;
	xsnmp_cfg.disc_nodetimeout				= 10000;
#endif

	xsnmp_cfg.authentrap_enable				= get_authentrap();
	xsnmp_cfg.coldtrap_enable				= get_coldstarttrap();
	xsnmp_cfg.local_port					= get_snmpport();
}

/*============================================================================
 * XSNMP initialization function
 *
 */
void
xsnmp_init2(void)
{

 	if( !x_meminit(100 *1024) )	x_dbg("XSNMP, Xmeminit failed\n", TRUE);
 	if( !x_timerinit() )	x_dbg("XSNMP, Xmeminit failed\n", TRUE);

 	nc_init();
  	cc_init();
 	hc_init();
 	sc_init();

 	xsnmp_init();
 	xsnmp_update();

	if( !NwInit() ) 			x_dbg("XSNMP, NwInit failed\n", TRUE);
    if( !MacInit() ) 			x_dbg("XSNMP, MacInit failed\n", TRUE);
    if( !AgentInit(xsnmp_cfg.local_port) )
								x_dbg("XSNMP, AgentInit failed\n", TRUE);
    if( !MibInit(xsnmpObj, sizeof(xsnmpObj)/sizeof(xsnmpObj[0])) )
        						x_dbg("XSNMP, MibInit failed\n", TRUE);
    if( !UdpInit() ) 			x_dbg("XSNMP, UdpInit failed\n", TRUE);
    if( !Mib2Init() ) 			x_dbg("XSNMP, Mib2Init failed\n", TRUE);

#ifdef XMIB_RMON1
	if( !Rmon1Init() )			x_dbg("XSNMP, Rmon1Init failed\n", TRUE);
#endif

	if( xsnmp_cfg.coldtrap_enable == TRUE )
		SendENTTrap( SNMP_TRAP_COLDSTART, 0, 0, 0);

#ifdef XSNMPV1_SHOW_BANNER
    x_msg("XSNMPv1(tm), Copyright (c) Xact Inc., 1997.\n\n");
#endif

	xsnmp_initialized = TRUE;

}

/*============================================================================
 * XSNMP main function
 *
 */
void
xsnmp_task(void)
{
i8 *addr;

	x_bzero( (i8 *)&xsnmp_ctl_addr, sizeof(xsnmp_ctl_addr) );
	xsnmp_ctl_addr.sin_family 		= AF_INET;
	xsnmp_ctl_addr.sin_len    		= MAX_1213_NETADDRSIZE;
	xsnmp_ctl_addr.sin_addr.s_addr	= INADDR_ANY;
	xsnmp_ctl_addr.sin_port			= x_htons( XSNMP_SNMP_PORT );

	xsnmp_ctl = x_socket( AF_INET, SOCK_DGRAM, 0 );
	if( xsnmp_ctl < 0 )
		goto stopit;

	if( x_bind((i32)xsnmp_ctl, (void *)&xsnmp_ctl_addr, sizeof(struct sockaddr_in)) < 0 )
		goto stopit;

	SetUDPListener( 0, XSNMP_SNMP_PORT );

	while( 1 ) {
		xsnmp_fromlen = sizeof( xsnmp_from );
		n = x_recvfrom( (i32)xsnmp_ctl, xsnmp_buf, sizeof(xsnmp_buf), 0,
			(void *)&xsnmp_from, &xsnmp_fromlen );
		if( n < 0 ) {
            //x_dbg("XSNMP, x_recvfrom error\n", TRUE);
			continue;
			//break;
		} else {
            //x_dbg("XSNMP, x_recvfrom OK\n", TRUE);
			addr = (i8 *)Sock2Addr( &xsnmp_from );
			UdpRcve( (i8 *)addr, (u8 *)xsnmp_buf, n, (void *)&agentUdp );
		}
	}

stopit:
    return;
}

