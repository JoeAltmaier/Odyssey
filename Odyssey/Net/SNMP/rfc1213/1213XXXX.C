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
 | FILE NAME   : 1213xxxx.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support functions for RFC 1213 Groups
 | AUTHOR      : Robert Winter
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "xsnmp.h"
#include "mac.h"
#include "xcfig.h"
#include "mib.h"
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "load.h"
#include "1213xxxx.h"


#ifdef X_PSOS
#include "lan_mib.h"
extern mib_stat	MG_stat;
#endif

static u8 bcaststr[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/*
 *  Do packet type processing for MIB2 and RMON1
 */
u32
xsnmp_pkttype( u32 unit, u8 *pkt )
{
	if( ((u8 *)pkt)[0] & 0x01 ) {
		if( ((u8 *)pkt)[0] == 0xFF ) {
			if( x_bcmp((c_void *)pkt, (c_void *)bcaststr, 6 ) == 0 )  {
				return BCAST;
			}
		} else {
			return MCAST;
		}
	} else {
		return UNICAST;
	}
	return UNICAST;
}

/*
 * Storage for RFC 1213 MIB variables; if local, done here
 */
#ifdef INTERNAL_RFC1213_STORAGE
rfc1213_vars_t			rfc1213_vars;
#else
extern rfc1213_vars_t          rfc1213_vars;
#endif

static i32				udplistencnt = 0;
	   udp_socks_t		udpsocks[MAX_1213_UDPLISTEN];

extern mac_iface_t		*gmac[];
extern i32				xsnmp_initialized;

void xset_1213outQLen			( u32 unit, u32 value );
void xset_1213outErrors			( u32 unit, u32 value );
void xset_1213outDiscards		( u32 unit, u32 value );
void xset_1213outNUcastPkts		( u32 unit, u32 value );
void xset_1213outUcastPkts		( u32 unit, u32 value );
void xset_1213outOctets			( u32 unit, u32 value );
void xset_1213inUcastPkts		( u32 unit, u32 value );
void xset_1213inNUcastPkts		( u32 unit, u32 value );
void xset_1213inUnknownProtos	( u32 unit, u32 value );
void xset_1213inErrors			( u32 unit, u32 value );
void xset_1213inDiscards		( u32 unit, u32 value );
void xset_1213inOctets			( u32 unit, u32 value );

void
SetUDPListener( ul32 addr, u16 port )
{
static u8 Listenerbuf[16];
union {
	ul32 lh;
	struct {
		u8 b0;
		u8 b1;
		u8 b2;
		u8 b3;
	} b;
} h;

	if( udplistencnt > MAX_1213_UDPLISTEN ) return;

	x_bzero( (i8 *)Listenerbuf, 16 );
	h.lh = addr;
#ifdef XLIB_XSNMP
	plist[0].intval = h.b.b0;
	plist[1].intval = h.b.b1;
	plist[2].intval = h.b.b2;
	plist[3].intval = h.b.b3;
	x_sprintf( (i8 *)Listenerbuf, "%03d.%03d.%03d.%03d");
#else
	sprintf( (i8 *)Listenerbuf, "%03d.%03d.%03d.%03d",  h.b.b0, h.b.b1, h.b.b2, h.b.b3 );
#endif
	x_bcopy( (i8 *)Listenerbuf, (i8 *)udpsocks[udplistencnt].address, 16 );
	udpsocks[udplistencnt].port = port;
	udplistencnt++;
}

udp_socks_t *
GetUDPListener( i32 ndex )
{
i32 idx;

	idx = ndex-1;
	if( idx < 0 ) return 0;

	if( udpsocks[idx].port ) {
		return( &udpsocks[idx] );
	}
	return(0);
}

/*---------------  The Glue functions for RFC1213 ----------------------*
 *
 * These functions are very interface and stack-specific and details must
 * be supplied by the user. They are to be called either from the
 * driver supported the requested interface or the protocol stack.
 * See the "typical" driver in the "xport" directory for details on
 * implementation.
 *
 *----------------------------------------------------------------------*/
void
xset_1213outQLen( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outQLen = value;
                }
			}
		}
	}
}

void
xset_1213outErrors( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outErrors = value;
                }
			}
		}
	}
}

void
xset_1213outDiscards( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outDiscards = value;
                }
			}
		}
	}
}

void
xset_1213outNUcastPkts( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outNUcastPkts = value;
                }
			}
		}
	}
}

void
xset_1213outUcastPkts( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outUcastPkts = value;
                }
			}
		}
	}
}

void
xset_1213outOctets( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->outOctets = value;
                }
			}
		}
	}
}

void
xset_1213inUcastPkts( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->inUcastPkts = value;
                }
			}
		}
	}
}

void
xset_1213inNUcastPkts( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->inNUcastPkts = value;
                }
			}
		}
	}
}

void
xset_1213inUnknownProtos( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->inUnknownProtos = value;
                }
			}
		}
	}
}

void
xset_1213inErrors( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->inErrors = value;
                }
			}
		}
	}
}

void
xset_1213inDiscards( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
    				eth->inDiscards = value;
                }
			}
		}
	}
}

void
xset_1213inOctets( u32 unit, u32 value )
{
mac_iface_t 	*mac;
mac_ethstats_t	*eth;

	if( xsnmp_initialized ) {
		if( (unit >=0) && (unit < MAX_PORTS) ) {
            if (gmac[unit] != NULL) {
                mac = gmac[unit];
                if (mac->eth != NULL) {
                    eth = mac->eth;
                    eth->inOctets = value;
                }
            }
		}
	}
}


/*---------------  The MIB2 Table support functions --------------------*
 *
 * These functions are specific to MIB2 and the tables it supports.  MIB2
 * has some fairly sophisticated indexing schemes.  These routines are
 * used by the MIB2 tables.
 *
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 * IP Address Table
 *----------------------------------------------------------------------*/
u16
Get1213IpAddrTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
ipaddrtab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len, foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_ip.ipAddrTab;
	while(next) {
		last = next;
		newname[idlen]   = next->ipAdEntAddr[0];
		newname[idlen+1] = next->ipAdEntAddr[1];
		newname[idlen+2] = next->ipAdEntAddr[2];
		newname[idlen+3] = next->ipAdEntAddr[3];
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			}
		}
		next = next->next;
	}

	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (len * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipAdEntAddr[0], MAX_1213_NETADDRSIZE);
				break;
			case 2:
				obj->Syntax.LngInt = next->ipAdEntIfIndex;
				break;
			case 3:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipAdEntNetMask[0], MAX_1213_NETADDRSIZE);
				break;
			case 4:
				obj->Syntax.LngInt = next->ipAdEntBcastAddr;
				break;
			case 5:
				obj->Syntax.LngInt = next->ipAdEntReasmMaxSize;
				break;
		}
	} else {
		return(0);
	}
	return(1);
}

i32
Add1213IpAddrTab(ipaddrtab_t *entry)
{
ipaddrtab_t *start = rfc1213_vars.rfc1213_ip.ipAddrTab;
ipaddrtab_t *new, *last, *next;
u32 entered;
i32 result = 0;

	entered = 0;
	if(rfc1213_vars.rfc1213_ip.ipAddrTab == 0) {
		start = (ipaddrtab_t *)x_malloc(sizeof(ipaddrtab_t));
		x_memset(start,0,sizeof(ipaddrtab_t));
		rfc1213_vars.rfc1213_ip.ipAddrTab = start;
		x_memset((u8 *)start, 0, sizeof(ipaddrtab_t));
		x_memcpy(start->ipAdEntAddr, entry->ipAdEntAddr, MAX_1213_NETADDRSIZE);
		start->ipAdEntIfIndex = entry->ipAdEntIfIndex;
		x_memcpy(start->ipAdEntNetMask, entry->ipAdEntNetMask, MAX_1213_NETADDRSIZE);
		start->ipAdEntBcastAddr = entry->ipAdEntBcastAddr;
		start->ipAdEntReasmMaxSize = entry->ipAdEntReasmMaxSize;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (ipaddrtab_t *)x_malloc(sizeof(ipaddrtab_t));
	x_memset(new, 0, sizeof(ipaddrtab_t));
	next = rfc1213_vars.rfc1213_ip.ipAddrTab;
	while(next) {
		last = next;
		result = x_memcmp(entry->ipAdEntAddr,next->ipAdEntAddr,MAX_1213_NETADDRSIZE);
		if(!result) {
			x_free((void *)new);
			return 0;
		}
		if(result < 0) {
			x_memcpy(new->ipAdEntAddr, entry->ipAdEntAddr, MAX_1213_NETADDRSIZE);
			new->ipAdEntIfIndex = entry->ipAdEntIfIndex;
			x_memcpy(new->ipAdEntNetMask, entry->ipAdEntNetMask, MAX_1213_NETADDRSIZE);
			new->ipAdEntBcastAddr = entry->ipAdEntBcastAddr;
			new->ipAdEntReasmMaxSize = entry->ipAdEntReasmMaxSize;
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_ip.ipAddrTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		x_memcpy(new->ipAdEntAddr, entry->ipAdEntAddr, MAX_1213_NETADDRSIZE);
		new->ipAdEntIfIndex = entry->ipAdEntIfIndex;
		x_memcpy(new->ipAdEntNetMask, entry->ipAdEntNetMask, MAX_1213_NETADDRSIZE);
		new->ipAdEntBcastAddr = entry->ipAdEntBcastAddr;
		new->ipAdEntReasmMaxSize = entry->ipAdEntReasmMaxSize;
		last->next = new;
		new->last = last;
		new->next = 0;
	}
	return(entered);
}

void
Del1213IpAddrTab(ipaddrtab_t *entry)
{
ipaddrtab_t *last, *next, *start;
i32 result = 0;

	start = next = rfc1213_vars.rfc1213_ip.ipAddrTab;
	while(next) {
		result = x_memcmp(entry->ipAdEntAddr,next->ipAdEntAddr,MAX_1213_NETADDRSIZE);
		if(!result) {
			/* single node to be deleted */
			if( (next->last == 0) && (next->next == 0)) {
				x_free((void *)next);
				rfc1213_vars.rfc1213_ip.ipAddrTab = 0;
				break;
			}
			/* beginning or first node to be deleted */
			else if(!next->last) {
				last = next;
				next = next->next;
				rfc1213_vars.rfc1213_ip.ipAddrTab = next;
				next->last = last->next = 0;
				x_free((void *)last);
				break;
			}
			/* last or end node to be deleted */
			else if(!next->next) {
				next->last = last->next = 0;
				rfc1213_vars.rfc1213_ip.ipAddrTab = start;
				x_free((void *)next);
				break;
			}
			/* node in between to be deleted */
			else {
				last->next = next->next;
				last->next->last = next->last;
				rfc1213_vars.rfc1213_ip.ipAddrTab = start;
				x_free((void *)next);
				break;
			}
		}
		last = next;
		next = next->next;
	}
}

u32
Size1213IpAddrTab()
{
ipaddrtab_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_ip.ipAddrTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213IpAddrTab()
{
ipaddrtab_t *start = rfc1213_vars.rfc1213_ip.ipAddrTab;

	while(start) {
		kprintf("Dump - ipAdEntAddr = %x.%x.%x.%x\n",
			start->ipAdEntAddr[0],start->ipAdEntAddr[1],
			start->ipAdEntAddr[2],start->ipAdEntAddr[3]);
		kprintf("Dump - ipAdEntIfIndex = %x\n",start->ipAdEntIfIndex);
		kprintf("Dump - ipAdEntNetMask = %x.%x.%x.%x\n",
			start->ipAdEntNetMask[0],start->ipAdEntNetMask[1],
			start->ipAdEntNetMask[2],start->ipAdEntNetMask[3]);
		kprintf("Dump - ipAdEntBcastAddr = %x\n",start->ipAdEntBcastAddr);
		kprintf("Dump - ipAdEntReasmMaxSize = %x\n",start->ipAdEntReasmMaxSize);
		start = start->next;
	}
}
#endif

/*----------------------------------------------------------------------*
 * IP Route Table
 *----------------------------------------------------------------------*/
u16
Get1213IpRouteTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
iproutetab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len, foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_ip.ipRouteTab;
	while(next) {
		last = next;
		newname[idlen]   = next->ipRouteDest[0];
		newname[idlen+1] = next->ipRouteDest[1];
		newname[idlen+2] = next->ipRouteDest[2];
		newname[idlen+3] = next->ipRouteDest[3];
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			}
		}
		next = next->next;
	}

	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (obj->IdLen * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipRouteDest[0], MAX_1213_NETADDRSIZE);
				break;
			case 2:
				obj->Syntax.LngInt = next->ipRouteIfIndex;
				break;
			case 3:
				obj->Syntax.LngInt = next->ipRouteMetric1;
				break;
			case 4:
				obj->Syntax.LngInt = next->ipRouteMetric2;
				break;
			case 5:
				obj->Syntax.LngInt = next->ipRouteMetric3;
				break;
			case 6:
				obj->Syntax.LngInt = next->ipRouteMetric4;
				break;
			case 7:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipRouteNextHop[0], MAX_1213_NETADDRSIZE);
				break;
			case 8:
				obj->Syntax.LngInt = next->ipRouteType;
				break;
			case 9:
				obj->Syntax.LngInt = next->ipRouteProto;
				break;
			case 10:
				obj->Syntax.LngInt = next->ipRouteAge;
				break;
			case 11:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipRouteMask[0], MAX_1213_NETADDRSIZE);
				break;
			case 12:
				obj->Syntax.LngInt = next->ipRouteMetric5;
				break;
			case 13:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipRouteInfo[0], MAX_1213_NETADDRSIZE);
				break;
		}
	} else {
		return(0);
	}
	return(1);
}

i32
Add1213IpRouteTab(iproutetab_t *entry)
{
iproutetab_t *start = rfc1213_vars.rfc1213_ip.ipRouteTab;
iproutetab_t *new, *last, *next;
u32 entered;
i32 result = 0;;

	entered = 0;
	if(rfc1213_vars.rfc1213_ip.ipRouteTab == 0) {
		start = (iproutetab_t *)x_malloc(sizeof(iproutetab_t));
		x_memset(start,0,sizeof(iproutetab_t));
		rfc1213_vars.rfc1213_ip.ipRouteTab = start;
		x_memset((u8 *)start, 0, sizeof(iproutetab_t));
		x_memcpy(start->ipRouteDest, entry->ipRouteDest, MAX_1213_NETADDRSIZE);
		start->ipRouteIfIndex = entry->ipRouteIfIndex;
		start->ipRouteMetric1 = entry->ipRouteMetric1;
		start->ipRouteMetric2 = entry->ipRouteMetric2;
		start->ipRouteMetric3 = entry->ipRouteMetric3;
		start->ipRouteMetric4 = entry->ipRouteMetric4;
		x_memcpy(start->ipRouteNextHop, entry->ipRouteNextHop, MAX_1213_NETADDRSIZE);
		start->ipRouteType = entry->ipRouteType;
		start->ipRouteProto = entry->ipRouteProto;
		start->ipRouteAge = entry->ipRouteAge;
		x_memcpy(start->ipRouteMask, entry->ipRouteMask, MAX_1213_NETADDRSIZE);
		start->ipRouteMetric5 = entry->ipRouteMetric5;
		x_memcpy(start->ipRouteInfo, entry->ipRouteInfo, MAX_1213_NETADDRSIZE);
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (iproutetab_t *)x_malloc(sizeof(iproutetab_t));
	x_memset(new,0,sizeof(iproutetab_t));
	next = rfc1213_vars.rfc1213_ip.ipRouteTab;
	while(next) {
		last = next;
		result = x_memcmp(entry->ipRouteDest,next->ipRouteDest,MAX_1213_NETADDRSIZE);
		if(!result) {
			x_free((void *)new);
			return 0;
		}
		if(result < 0) {
			x_memcpy(new->ipRouteDest, entry->ipRouteDest, MAX_1213_NETADDRSIZE);
			new->ipRouteIfIndex = entry->ipRouteIfIndex;
			new->ipRouteMetric1 = entry->ipRouteMetric1;
			new->ipRouteMetric2 = entry->ipRouteMetric2;
			new->ipRouteMetric3 = entry->ipRouteMetric3;
			new->ipRouteMetric4 = entry->ipRouteMetric4;
			x_memcpy(new->ipRouteNextHop, entry->ipRouteNextHop, MAX_1213_NETADDRSIZE);
			new->ipRouteType = entry->ipRouteType;
			new->ipRouteProto = entry->ipRouteProto;
			new->ipRouteAge = entry->ipRouteAge;
			x_memcpy(new->ipRouteMask, entry->ipRouteMask, MAX_1213_NETADDRSIZE);
			new->ipRouteMetric5 = entry->ipRouteMetric5;
			x_memcpy(new->ipRouteInfo, entry->ipRouteInfo, MAX_1213_NETADDRSIZE);
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_ip.ipRouteTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		x_memcpy(new->ipRouteDest, entry->ipRouteDest, MAX_1213_NETADDRSIZE);
		new->ipRouteIfIndex = entry->ipRouteIfIndex;
		new->ipRouteMetric1 = entry->ipRouteMetric1;
		new->ipRouteMetric2 = entry->ipRouteMetric2;
		new->ipRouteMetric3 = entry->ipRouteMetric3;
		new->ipRouteMetric4 = entry->ipRouteMetric4;
		x_memcpy(new->ipRouteNextHop, entry->ipRouteNextHop, MAX_1213_NETADDRSIZE);
		new->ipRouteType = entry->ipRouteType;
		new->ipRouteProto = entry->ipRouteProto;
		new->ipRouteAge = entry->ipRouteAge;
		x_memcpy(new->ipRouteMask, entry->ipRouteMask, MAX_1213_NETADDRSIZE);
		new->ipRouteMetric5 = entry->ipRouteMetric5;
		x_memcpy(new->ipRouteInfo, entry->ipRouteInfo, MAX_1213_NETADDRSIZE);
		last->next = new;
		new->last = last;
		new->next = 0;
	}
	return(entered);
}

void
Del1213IpRouteTab(iproutetab_t *entry)
{
iproutetab_t *last, *next, *start;
i32 result = 0;

	start = next = rfc1213_vars.rfc1213_ip.ipRouteTab;
	while(next) {
		result = x_memcmp(entry->ipRouteDest,next->ipRouteDest,MAX_1213_NETADDRSIZE);
		if(!result) {
			/* single node to be deleted */
			if( (next->last == 0) && (next->next == 0)) {
				x_free((void *)next);
				rfc1213_vars.rfc1213_ip.ipRouteTab = 0;
				break;
			}
			/* beginning or first node to be deleted */
			else if(!next->last) {
				last = next;
				next = next->next;
				rfc1213_vars.rfc1213_ip.ipRouteTab = next;
				next->last = last->next = 0;
				x_free((void *)last);
				break;
			}
			/* last or end node to be deleted */
			else if(!next->next) {
				next->last = last->next = 0;
				rfc1213_vars.rfc1213_ip.ipRouteTab = start;
				x_free((void *)next);
				break;
			}
			/* node in between to be deleted */
			else {
				last->next = next->next;
				last->next->last = next->last;
				rfc1213_vars.rfc1213_ip.ipRouteTab = start;
				x_free((void *)next);
				break;
			}
		}
		last = next;
		next = next->next;
	}
}

u32
Size1213IpRouteTab()
{
iproutetab_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_ip.ipRouteTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213IpRouteTab()
{
iproutetab_t *start = rfc1213_vars.rfc1213_ip.ipRouteTab;

	while(start) {
		kprintf("Dump - ipRouteDest = %x.%x.%x.%x\n",
			start->ipRouteDest[0],start->ipRouteDest[1],
			start->ipRouteDest[2],start->ipRouteDest[3]);
		kprintf("Dump - ipRouteIfIndex = %x\n",start->ipRouteIfIndex);
		kprintf("Dump - ipRouteMetric1 = %x\n",start->ipRouteMetric1);
		kprintf("Dump - ipRouteMetric2 = %x\n",start->ipRouteMetric2);
		kprintf("Dump - ipRouteMetric3 = %x\n",start->ipRouteMetric3);
		kprintf("Dump - ipRouteMetric4 = %x\n",start->ipRouteMetric4);
		kprintf("Dump - ipRouteNextHop = %x.%x.%x.%x\n",
			start->ipRouteNextHop[0],start->ipRouteNextHop[1],
			start->ipRouteNextHop[2],start->ipRouteNextHop[3]);
		kprintf("Dump - ipRouteType = %x\n",start->ipRouteType);
		kprintf("Dump - ipRouteProto = %x\n",start->ipRouteProto);
		kprintf("Dump - ipRouteAge = %x\n",start->ipRouteAge);
		kprintf("Dump - ipRouteMask = %x.%x.%x.%x\n",
			start->ipRouteMask[0],start->ipRouteMask[1],
			start->ipRouteMask[2],start->ipRouteMask[3]);
		kprintf("Dump - ipRouteMetric5 = %x\n",start->ipRouteMetric5);
		kprintf("Dump - ipRouteInfo = %x.%x.%x.%x\n",
			start->ipRouteInfo[0],start->ipRouteInfo[1],
			start->ipRouteInfo[2],start->ipRouteInfo[3]);
		start = start->next;
	}
}
#endif

/*----------------------------------------------------------------------*
 * IP Net2Media Table
 *----------------------------------------------------------------------*/ 
u16
Get1213IpNet2MediaTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
ipnet2media_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len;
u16 foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;
	while(next) {
		last = next;
		newname[idlen]   = next->ipNetToMediaIfIndex;
		newname[idlen+1] = next->ipNetToMediaNetAddress[0];
		newname[idlen+2] = next->ipNetToMediaNetAddress[1];
		newname[idlen+3] = next->ipNetToMediaNetAddress[2];
		newname[idlen+4] = next->ipNetToMediaNetAddress[3];
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			} 
		}
		next = next->next;
	}
	
	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (obj->IdLen * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->Syntax.LngInt = next->ipNetToMediaIfIndex;
				break;
			case 2:
				obj->SyntaxLen = MAX_1213_PADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipNetToMediaPhysAddress[0], MAX_1213_PADDRSIZE);
				break;
			case 3:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->ipNetToMediaNetAddress[0], MAX_1213_NETADDRSIZE);
				break;
			case 4:
				obj->Syntax.LngInt = next->ipNetToMediaType;
				break;
		}
	} else {
		return(0);
	}
	return(foundit);
}

i32
Add1213IpNet2MediaTab(ipnet2media_t *entry)
{
ipnet2media_t *start = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;
ipnet2media_t *new, *last, *next;
u32 entered;
i32 result1=0,result2=0;

	entered = 0;
	if(rfc1213_vars.rfc1213_ip.ipNet2MediaTab == 0) {
		start = (ipnet2media_t *)x_malloc(sizeof(ipnet2media_t));
		x_memset(start,0,sizeof(ipnet2media_t));
		rfc1213_vars.rfc1213_ip.ipNet2MediaTab = start;
		x_memset((u8 *)start, 0, sizeof(ipnet2media_t));
		start->ipNetToMediaIfIndex = entry->ipNetToMediaIfIndex;
		x_memcpy(start->ipNetToMediaPhysAddress, entry->ipNetToMediaPhysAddress, MAX_1213_PADDRSIZE);
		x_memcpy(start->ipNetToMediaNetAddress, entry->ipNetToMediaNetAddress, MAX_1213_NETADDRSIZE);
		start->ipNetToMediaType = entry->ipNetToMediaType;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (ipnet2media_t *)x_malloc(sizeof(ipnet2media_t));
	x_memset(new,0,sizeof(ipnet2media_t));
	next = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;
	while(next) {
		last = next;
		if(entry->ipNetToMediaIfIndex < next->ipNetToMediaIfIndex)
			result1 = -1;
		else
			if(entry->ipNetToMediaIfIndex == next->ipNetToMediaIfIndex)
				result1 = 0;
		
		result2 = x_memcmp(entry->ipNetToMediaNetAddress,next->ipNetToMediaNetAddress,MAX_1213_NETADDRSIZE);
		if(!result1 && !result2) {
			x_free((void *)new);
			return 0; 
		}
		if( (result1 < 0) && (result2 < 0) ) {
			new->ipNetToMediaIfIndex = entry->ipNetToMediaIfIndex;
			x_memcpy(new->ipNetToMediaPhysAddress, entry->ipNetToMediaPhysAddress, MAX_1213_PADDRSIZE);
			x_memcpy(new->ipNetToMediaNetAddress, entry->ipNetToMediaNetAddress, MAX_1213_NETADDRSIZE);
			new->ipNetToMediaType = entry->ipNetToMediaType;
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_ip.ipNet2MediaTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		new->ipNetToMediaIfIndex = entry->ipNetToMediaIfIndex;
		x_memcpy(new->ipNetToMediaPhysAddress, entry->ipNetToMediaPhysAddress, MAX_1213_PADDRSIZE);
		x_memcpy(new->ipNetToMediaNetAddress, entry->ipNetToMediaNetAddress, MAX_1213_NETADDRSIZE);
		new->ipNetToMediaType = entry->ipNetToMediaType;
		last->next = new;
		new->last = last;
		new->next = 0;	
	}
	return(entered);
}

void
Del1213IpNet2MediaTab(ipnet2media_t *entry)
{
ipnet2media_t *last, *next;
i32 result1=0, result2=0;

	next = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;
	while(next) {
		last = next;
		if(entry->ipNetToMediaIfIndex == next->ipNetToMediaIfIndex)
			result1 = 0;
		result2 = x_memcmp(entry->ipNetToMediaNetAddress,next->ipNetToMediaNetAddress,MAX_1213_NETADDRSIZE);
		if(!result1 && !result2) {
			if(!next->last) {
				rfc1213_vars.rfc1213_ip.ipNet2MediaTab = next->next;
				next->last = 0;
				break;
			} else {
				last->next = next->next;
				next->last = last;
				break;
			}
		} 
		next = next->next;
	}
}

u32
Size1213IpNet2MediaTab()
{
ipnet2media_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213IpNet2MediaTab()
{
ipnet2media_t *start = rfc1213_vars.rfc1213_ip.ipNet2MediaTab;

	while(start) {
		kprintf("Dump - ipNetToMediaIfIndex = %x\n",start->ipNetToMediaIfIndex);
		kprintf("Dump - ipNetToMediaPhysAddress = %x.%x.%x.%x.%x.%x\n",
			start->ipNetToMediaPhysAddress[0],start->ipNetToMediaPhysAddress[1],
			start->ipNetToMediaPhysAddress[2],start->ipNetToMediaPhysAddress[3],
			start->ipNetToMediaPhysAddress[4],start->ipNetToMediaPhysAddress[5]);
		kprintf("Dump - ipNetToMediaNetAddress = %x.%x.%x.%x\n",
			start->ipNetToMediaNetAddress[0],start->ipNetToMediaNetAddress[1],
			start->ipNetToMediaNetAddress[2],start->ipNetToMediaNetAddress[3]);
		kprintf("Dump - ipNetToMediaType = %x\n",start->ipNetToMediaType);
		start = start->next;
	}
}
#endif

/*----------------------------------------------------------------------*
 * UDP Listener Table
 *----------------------------------------------------------------------*/ 
u16
Get1213UdpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
udplisttab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len;
u16 foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_udp.udpListTab;
	while(next) {
		last = next;
		newname[idlen]     = next->udpLocalAddress[0];
		newname[idlen+1]   = next->udpLocalAddress[1];
		newname[idlen+2]   = next->udpLocalAddress[2];
		newname[idlen+3]   = next->udpLocalAddress[3];
		newname[idlen+4]   = next->udpLocalPort;
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			} 
		}
		next = next->next;
	}
	
	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (obj->IdLen * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->udpLocalAddress[0], MAX_1213_NETADDRSIZE);
				break;
			case 2:
				obj->Syntax.LngInt = next->udpLocalPort;
				break;
		}
	} else {
		return(0);
	}
	return(foundit);
}

i32
Add1213UdpListTab(udplisttab_t *entry)
{
udplisttab_t *start = rfc1213_vars.rfc1213_udp.udpListTab;
udplisttab_t *new, *last, *next;
u32 entered;
i32 result1=0, result2=0;

	entered = 0;
	if(rfc1213_vars.rfc1213_udp.udpListTab == 0) {
		start = (udplisttab_t *)x_malloc(sizeof(udplisttab_t));
		x_memset(start,0,sizeof(udplisttab_t));
		rfc1213_vars.rfc1213_udp.udpListTab = start;
		x_memset((u8 *)start, 0, sizeof(udplisttab_t));
		x_memcpy(start->udpLocalAddress, entry->udpLocalAddress, MAX_1213_NETADDRSIZE);
		start->udpLocalPort = entry->udpLocalPort;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (udplisttab_t *)x_malloc(sizeof(udplisttab_t));
	x_memset(new,0,sizeof(udplisttab_t));
	next = rfc1213_vars.rfc1213_udp.udpListTab;
	while(next) {
		last = next;
		result1 = x_memcmp(entry->udpLocalAddress,next->udpLocalAddress,MAX_1213_NETADDRSIZE);
		if(entry->udpLocalPort < next->udpLocalPort)
			result2 = -1;
		else
			if(entry->udpLocalPort == next->udpLocalPort)
				result2 = 0;
		if(!result1 && !result2) {
			x_free((void *)new);
			return 0; 
		}
		if((result1 < 0) && (result2 < 0)) {
			x_memcpy(new->udpLocalAddress, entry->udpLocalAddress, MAX_1213_NETADDRSIZE);
			new->udpLocalPort = entry->udpLocalPort;
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_udp.udpListTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		x_memcpy(new->udpLocalAddress, entry->udpLocalAddress, MAX_1213_NETADDRSIZE);
		new->udpLocalPort = entry->udpLocalPort;
		last->next = new;
		new->last = last;
		new->next = 0;	
	}
	return(entered);
}

void
Del1213UdpListTab(udplisttab_t *entry)
{
udplisttab_t *last, *next;
i32 result1=0, result2=0;

	next = rfc1213_vars.rfc1213_udp.udpListTab;
	while(next) {
		last = next;
		result1 = x_memcmp(entry->udpLocalAddress,next->udpLocalAddress,MAX_1213_NETADDRSIZE);
		if(entry->udpLocalPort == next->udpLocalPort)
			result2 = 0;
		if(!result1 && !result2) {
			if(!next->last) {
				rfc1213_vars.rfc1213_udp.udpListTab = next->next;
				next->last = 0;
				break;
			} else {
				last->next = next->next;
				next->last = last;
				break;
			}
		} 
		next = next->next;
	}
}

u32
Size1213UdpListTab()
{
udplisttab_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_udp.udpListTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213UdpListTab()
{
udplisttab_t *start = rfc1213_vars.rfc1213_udp.udpListTab;

	while(start) {
		kprintf("Dump - udpLocalAddress = %x.%x.%x.%x\n",
			start->udpLocalAddress[0],start->udpLocalAddress[1],
			start->udpLocalAddress[2],start->udpLocalAddress[3]);
		kprintf("Dump - udpLocalPort = %x\n",start->udpLocalPort);
		start = start->next;
	}

}
#endif

/*----------------------------------------------------------------------*
 * TCP Connection Table
 *----------------------------------------------------------------------*/ 
u16
Get1213TcpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
tcpcontab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len;
u16 foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_tcp.tcpConnTab;
	while(next) {
		last = next;
		newname[idlen]       = next->tcpConnLocalAddress[0];
		newname[idlen+1]     = next->tcpConnLocalAddress[1];
		newname[idlen+2]     = next->tcpConnLocalAddress[2];
		newname[idlen+3]     = next->tcpConnLocalAddress[3];
		newname[idlen+4]     = next->tcpConnLocalPort;
		newname[idlen+5]     = next->tcpConnRemAddress[0];
		newname[idlen+6]     = next->tcpConnRemAddress[1];
		newname[idlen+7]     = next->tcpConnRemAddress[2];
		newname[idlen+8]     = next->tcpConnRemAddress[3];
		newname[idlen+9]     = next->tcpConnRemPort;
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			} 
		}
		next = next->next;
	}
	
	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (obj->IdLen * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->Syntax.LngInt = next->tcpConnState;
				break;
			case 2:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->tcpConnLocalAddress[0], MAX_1213_NETADDRSIZE);
				break;
			case 3:
				obj->Syntax.LngInt = next->tcpConnLocalPort;
				break;
			case 4:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->tcpConnRemAddress[0], MAX_1213_NETADDRSIZE);
				break;
			case 5:
				obj->Syntax.LngInt = next->tcpConnRemPort;
				break;
		}
	} else {
		return(0);
	}
	return(foundit);
}

i32
Add1213TcpTab(tcpcontab_t *entry)
{
tcpcontab_t *start = rfc1213_vars.rfc1213_tcp.tcpConnTab;
tcpcontab_t *new, *last, *next;
u32 entered;
i32 result1=0, result2=0, result3=0, result4=0;

	entered = 0;
	if(rfc1213_vars.rfc1213_tcp.tcpConnTab == 0) {
		start = (tcpcontab_t *)x_malloc(sizeof(tcpcontab_t));
		x_memset(start,0,sizeof(tcpcontab_t));
		rfc1213_vars.rfc1213_tcp.tcpConnTab = start;		
        x_memset((u8 *)start, 0, sizeof(tcpcontab_t));
        
        rfc1213_vars.rfc1213_tcp.tcpConnTab->tcpConnState = entry->tcpConnState;

		x_memcpy(start->tcpConnLocalAddress, entry->tcpConnLocalAddress, MAX_1213_NETADDRSIZE);
		start->tcpConnLocalPort = entry->tcpConnLocalPort;
		x_memcpy(start->tcpConnRemAddress, entry->tcpConnRemAddress, MAX_1213_NETADDRSIZE);
		start->tcpConnRemPort = entry->tcpConnRemPort;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (tcpcontab_t *)x_malloc(sizeof(tcpcontab_t));
	x_memset(new,0,sizeof(tcpcontab_t));
	next = rfc1213_vars.rfc1213_tcp.tcpConnTab;
	while(next) {
		last = next;
		result1 = x_memcmp(entry->tcpConnLocalAddress,next->tcpConnLocalAddress,MAX_1213_NETADDRSIZE);
		if(entry->tcpConnLocalPort < next->tcpConnLocalPort)
			result2 = -1;
		else
			if(entry->tcpConnLocalPort == next->tcpConnLocalPort)
				result2 = 0;
		result3 = x_memcmp(entry->tcpConnRemAddress,next->tcpConnRemAddress,MAX_1213_NETADDRSIZE);
		if(entry->tcpConnRemPort < next->tcpConnRemPort)
			result4 = -1;
		else
			if(entry->tcpConnRemPort == next->tcpConnRemPort)
				result4 = 0;
		if(!result1 && !result2 && !result3 && !result4) {
			x_free((void *)new);
			return 0; 
		}
		if((result1 < 0) && (result2 < 0) && (result3 < 0) && (result4 < 0) ) {
			x_memcpy(new->tcpConnLocalAddress, entry->tcpConnLocalAddress, MAX_1213_NETADDRSIZE);
			new->tcpConnLocalPort = entry->tcpConnLocalPort;
			x_memcpy(new->tcpConnRemAddress, entry->tcpConnRemAddress, MAX_1213_NETADDRSIZE);
			new->tcpConnRemPort = entry->tcpConnRemPort;
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_tcp.tcpConnTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		x_memcpy(new->tcpConnLocalAddress, entry->tcpConnLocalAddress, MAX_1213_NETADDRSIZE);
		new->tcpConnLocalPort = entry->tcpConnLocalPort;
		x_memcpy(new->tcpConnRemAddress, entry->tcpConnRemAddress, MAX_1213_NETADDRSIZE);
		new->tcpConnRemPort = entry->tcpConnRemPort;
		last->next = new;
		new->last = last;
		new->next = 0;	
	}
	return(entered);
}

void
Del1213TcpTab(tcpcontab_t *entry)
{
tcpcontab_t *last, *next;
i32 result1=0, result2=0, result3=0, result4=0;

	next = rfc1213_vars.rfc1213_tcp.tcpConnTab;
	while(next) {
		last = next;
		result1 = x_memcmp(entry->tcpConnLocalAddress,next->tcpConnLocalAddress,MAX_1213_NETADDRSIZE);
		if(entry->tcpConnLocalPort == next->tcpConnLocalPort)
			result2 = 0;
		result3 = x_memcmp(entry->tcpConnRemAddress,next->tcpConnRemAddress,MAX_1213_NETADDRSIZE);
		if(entry->tcpConnRemPort == next->tcpConnRemPort)
			result4 = 0;
		if(!result1 && !result2 && !result3 && !result4) {
			if(!next->last) {
				rfc1213_vars.rfc1213_tcp.tcpConnTab = next->next;
				next->last = 0;
				break;
			} else {
				last->next = next->next;
				next->last = last;
				break;
			}
		} 
		next = next->next;
	}
}

u32
Size1213TcpTab()
{
tcpcontab_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_tcp.tcpConnTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213TcpTab()
{
tcpcontab_t *start = rfc1213_vars.rfc1213_tcp.tcpConnTab;

	while(start) {
		kprintf("Dump - tcpConnLocalAddress = %x.%x.%x.%x\n",
			start->tcpConnLocalAddress[0],start->tcpConnLocalAddress[1],
			start->tcpConnLocalAddress[2],start->tcpConnLocalAddress[3]);
		kprintf("Dump - tcpConnLocalPort = %x\n",start->tcpConnLocalPort);
		kprintf("Dump - tcpConnRemAddress = %x.%x.%x.%x\n",
			start->tcpConnRemAddress[0],start->tcpConnRemAddress[1],
			start->tcpConnRemAddress[2],start->tcpConnRemAddress[3]);
		kprintf("Dump - tcpConnRemPort = %x\n",start->tcpConnRemPort);
		start = start->next;
	}

}
#endif

/*----------------------------------------------------------------------*
 * AT Table
 *----------------------------------------------------------------------*/ 
u16
Get1213AtTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
rfc1213_at_t *nxt, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len, foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	nxt = rfc1213_vars.rfc1213_at;
	while(nxt) {
		last = nxt;
		newname[idlen]       = nxt->atIfIndex;
		newname[idlen+1]     = nxt->atNetAddress[0];
		newname[idlen+2]     = nxt->atNetAddress[1];
		newname[idlen+3]     = nxt->atNetAddress[2];
		newname[idlen+4]     = nxt->atNetAddress[3];
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			} 
		}
		nxt = nxt->next;
	}

	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, ((obj->IdLen + 1) * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->Syntax.LngInt = nxt->atIfIndex;
				break;
			case 2:
				obj->SyntaxLen = MAX_1213_PADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &nxt->atPhysAddress[0], MAX_1213_PADDRSIZE);
				break;
			case 3:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &nxt->atNetAddress[0], MAX_1213_NETADDRSIZE);
				break;
		}
	} else {
		return(0);
	}
	return(1);
}

i32
Add1213AtTab(rfc1213_at_t *entry)
{
rfc1213_at_t *start = rfc1213_vars.rfc1213_at;
rfc1213_at_t *new, *last, *next;
u32 entered;
i32 result1=0, result2=0;

	entered = 0;
	if(rfc1213_vars.rfc1213_at == 0) {
		start = (rfc1213_at_t *)x_malloc(sizeof(rfc1213_at_t));
		x_memset(start,0,sizeof(rfc1213_at_t));
		rfc1213_vars.rfc1213_at = start;
		start->atIfIndex = entry->atIfIndex;
		x_memcpy(start->atPhysAddress, entry->atPhysAddress, MAX_1213_PADDRSIZE);
		x_memcpy(start->atNetAddress, entry->atNetAddress, MAX_1213_NETADDRSIZE);
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (rfc1213_at_t *)x_malloc(sizeof(rfc1213_at_t));
	x_memset(new,0,sizeof(rfc1213_at_t));
	next = rfc1213_vars.rfc1213_at;
	while(next) {
		last = next;
		if(entry->atIfIndex < next->atIfIndex)
			result1 = -1;
		else
			if(entry->atIfIndex == next->atIfIndex)
				result1 = 0;
		result2 = x_memcmp(entry->atNetAddress,next->atNetAddress,MAX_1213_NETADDRSIZE);
		if(!result1 && !result2) {
			x_free((void *)new);
			return 0; 
		}
		if((result1 < 0) && (result2 < 0) ) {
			new->atIfIndex = entry->atIfIndex;
			x_memcpy(new->atPhysAddress, entry->atPhysAddress, MAX_1213_PADDRSIZE);
			x_memcpy(new->atNetAddress, entry->atNetAddress, MAX_1213_NETADDRSIZE);
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_at = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		new->atIfIndex = entry->atIfIndex;
		x_memcpy(new->atPhysAddress, entry->atPhysAddress, MAX_1213_PADDRSIZE);
		x_memcpy(new->atNetAddress, entry->atNetAddress, MAX_1213_NETADDRSIZE);
		last->next = new;
		new->last = last;
		new->next = 0;	
		entered++;
	}
	return(entered);
}

void
Del1213AtTab(rfc1213_at_t *entry)
{
rfc1213_at_t *last, *next;
i32 result1=0, result2=0;

	next = rfc1213_vars.rfc1213_at;
	while(next) {
		last = next;
		if(entry->atIfIndex == next->atIfIndex)
			result1 = 0;
		result2 = x_memcmp(entry->atNetAddress,next->atNetAddress,MAX_1213_NETADDRSIZE);
		if(!result1 && !result2) {
			if(!next->last) {
				rfc1213_vars.rfc1213_at = next->next;
				next->last = 0;
				break;
			} else {
				last->next = next->next;
				next->last = last;
				break;
			}
		} 
		next = next->next;
	}
}

u32
Size1213AtTab()
{
rfc1213_at_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_at;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213AtTab()
{
rfc1213_at_t *start = rfc1213_vars.rfc1213_at;

	while(start) {
		kprintf("Dump - atIfIndex = %x\n",start->atIfIndex);
		kprintf("Dump - atPhysAddress = %x.%x.%x.%x.%x.%x\n",
			start->atPhysAddress[0],start->atPhysAddress[1],
			start->atPhysAddress[2],start->atPhysAddress[3],
			start->atPhysAddress[4],start->atPhysAddress[5]);
		kprintf("Dump - atNetAddress = %x.%x.%x.%x\n",
			start->atNetAddress[0],start->atNetAddress[1],
			start->atNetAddress[2],start->atNetAddress[3]);
		start = start->next;
	}

}
#endif

/*----------------------------------------------------------------------*
 * EGP Table
 *----------------------------------------------------------------------*/ 
u16
Get1213EgpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
egpneightab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len;
u16 foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1213_vars.rfc1213_egp.egpNeighTab;
	while(next) {
		last = next;
		newname[idlen]       = next->egpNeighAddr[0];
		newname[idlen+1]     = next->egpNeighAddr[1];
		newname[idlen+2]     = next->egpNeighAddr[2];
		newname[idlen+3]     = next->egpNeighAddr[3];
		if (obj->IdLen < len && !getflag) {
			foundit++;
			break;
		}
		result = x_memcmp(obj->Id, newname, (len * sizeof(l32)));
		if( getflag ) {
			if(!result) {
				foundit++;
				break;
			}
		} else {
			if(result < 0) {
				foundit++;
				break;
			} 
		}
		next = next->next;
	}
	
	if(foundit) {
		obj->IdLen = idlen+sublen;
		x_memcpy(obj->Id, newname, (obj->IdLen * sizeof(l32)));
		switch(obj->Id[idlen-1]) {
			default:
			case 1:
				obj->Syntax.LngInt = next->egpNeighState;
				break;
			case 2:
				obj->SyntaxLen = MAX_1213_NETADDRSIZE;
				x_memcpy(obj->Syntax.BufChr, &next->egpNeighAddr[0], MAX_1213_NETADDRSIZE);
				break;
			case 3:
				obj->Syntax.LngInt = next->egpNeighAs;
				break;
			case 4:
				obj->Syntax.LngInt = next->egpNeighInMsgs;
				break;
			case 5:
				obj->Syntax.LngInt = next->egpNeighInErrs;
				break;
			case 6:
				obj->Syntax.LngInt = next->egpNeighOutMsgs;
				break;
			case 7:
				obj->Syntax.LngInt = next->egpNeighOutErrs;
				break;
			case 8:
				obj->Syntax.LngInt = next->egpNeighInErrMsgs;
				break;
			case 9:
				obj->Syntax.LngInt = next->egpNeighOutErrMsgs;
				break;
			case 10:
				obj->Syntax.LngInt = next->egpNeighStateUps;
				break;
			case 11:
				obj->Syntax.LngInt = next->egpNeighStateDowns;
				break;
			case 12:
				obj->Syntax.LngInt = next->egpNeighIntervalHello;
				break;
			case 13:
				obj->Syntax.LngInt = next->egpNeighIntervalPoll;
				break;
			case 14:
				obj->Syntax.LngInt = next->egpNeighMode;
				break;
			case 15:
				obj->Syntax.LngInt = next->egpNeighEventTrigger;
				break;
		}
	} else {
		return(0);
	}
	return(foundit);
}

i32
Add1213EgpTab(egpneightab_t *entry)
{
egpneightab_t *start = rfc1213_vars.rfc1213_egp.egpNeighTab;
egpneightab_t *new, *last, *next;
u32 entered;
i32 result = 0;

	entered = 0;
	if(rfc1213_vars.rfc1213_egp.egpNeighTab == 0) {
		start = (egpneightab_t *)x_malloc(sizeof(egpneightab_t));
		x_memset(start,0,sizeof(egpneightab_t));
		rfc1213_vars.rfc1213_egp.egpNeighTab = start;
		x_memset((u8 *)start, 0, sizeof(egpneightab_t));
		start->egpNeighState = entry->egpNeighState;
		x_memcpy(start->egpNeighAddr, entry->egpNeighAddr, MAX_1213_NETADDRSIZE);
		start->egpNeighAs = entry->egpNeighAs;
		start->egpNeighInMsgs = entry->egpNeighInMsgs;
		start->egpNeighInErrs = entry->egpNeighInErrs;
		start->egpNeighOutMsgs = entry->egpNeighOutMsgs;
		start->egpNeighOutErrs = entry->egpNeighOutErrs;
		start->egpNeighInErrMsgs = entry->egpNeighInErrMsgs;
		start->egpNeighOutErrMsgs = entry->egpNeighOutErrMsgs;
		start->egpNeighStateUps = entry->egpNeighStateUps;
		start->egpNeighStateDowns = entry->egpNeighStateDowns;
		start->egpNeighIntervalHello = entry->egpNeighIntervalHello;
		start->egpNeighIntervalPoll = entry->egpNeighIntervalPoll;
		start->egpNeighMode = entry->egpNeighMode;
		start->egpNeighEventTrigger = entry->egpNeighEventTrigger;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (egpneightab_t *)x_malloc(sizeof(egpneightab_t));
	x_memset(new,0,sizeof(egpneightab_t));
	next = rfc1213_vars.rfc1213_egp.egpNeighTab;
	while(next) {
		last = next;
		result = x_memcmp(entry->egpNeighAddr,next->egpNeighAddr,MAX_1213_NETADDRSIZE);
		if(!result) {
			x_free((void *)new);
			return 0; 
		}
		if(result < 0) {
			new->egpNeighState = entry->egpNeighState;
			x_memcpy(new->egpNeighAddr, entry->egpNeighAddr, MAX_1213_NETADDRSIZE);
			new->egpNeighAs = entry->egpNeighAs;
			new->egpNeighInMsgs = entry->egpNeighInMsgs;
			new->egpNeighInErrs = entry->egpNeighInErrs;
			new->egpNeighOutMsgs = entry->egpNeighOutMsgs;
			new->egpNeighOutErrs = entry->egpNeighOutErrs;
			new->egpNeighInErrMsgs = entry->egpNeighInErrMsgs;
			new->egpNeighOutErrMsgs = entry->egpNeighOutErrMsgs;
			new->egpNeighStateUps = entry->egpNeighStateUps;
			new->egpNeighStateDowns = entry->egpNeighStateDowns;
			new->egpNeighIntervalHello = entry->egpNeighIntervalHello;
			new->egpNeighIntervalPoll = entry->egpNeighIntervalPoll;
			new->egpNeighMode = entry->egpNeighMode;
			new->egpNeighEventTrigger = entry->egpNeighEventTrigger;
			new->next = next;
			if(!next->last) {
				new->last = 0;
				rfc1213_vars.rfc1213_egp.egpNeighTab = new;
			} else {
				new->last = next->last;
				new->last->next = new;
			}
			next->last = new;
			entered++;
			break;
		}
		next = next->next;
	}
	if(!entered) {
		new->egpNeighState = entry->egpNeighState;
		x_memcpy(new->egpNeighAddr, entry->egpNeighAddr, MAX_1213_NETADDRSIZE);
		new->egpNeighAs = entry->egpNeighAs;
		new->egpNeighInMsgs = entry->egpNeighInMsgs;
		new->egpNeighInErrs = entry->egpNeighInErrs;
		new->egpNeighOutMsgs = entry->egpNeighOutMsgs;
		new->egpNeighOutErrs = entry->egpNeighOutErrs;
		new->egpNeighInErrMsgs = entry->egpNeighInErrMsgs;
		new->egpNeighOutErrMsgs = entry->egpNeighOutErrMsgs;
		new->egpNeighStateUps = entry->egpNeighStateUps;
		new->egpNeighStateDowns = entry->egpNeighStateDowns;
		new->egpNeighIntervalHello = entry->egpNeighIntervalHello;
		new->egpNeighIntervalPoll = entry->egpNeighIntervalPoll;
		new->egpNeighMode = entry->egpNeighMode;
		new->egpNeighEventTrigger = entry->egpNeighEventTrigger;
		last->next = new;
		new->last = last;
		new->next = 0;	
	}
	return(entered);
}

void
Del1213EgpTab(egpneightab_t *entry)
{
egpneightab_t *last, *next;
i32 result=0;

	next = rfc1213_vars.rfc1213_egp.egpNeighTab;
	while(next) {
		last = next;
		result = x_memcmp(entry->egpNeighAddr,next->egpNeighAddr,MAX_1213_NETADDRSIZE);
		if(!result) {
			if(!next->last) {
				rfc1213_vars.rfc1213_egp.egpNeighTab = next->next;
				next->last = 0;
				break;
			} else {
				last->next = next->next;
				next->last = last;
				break;
			}
		} 
		next = next->next;
	}
}

u32
Size1213EgpTab()
{
egpneightab_t *next;
u32 counter = 0;;

	next = rfc1213_vars.rfc1213_egp.egpNeighTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1213EgpTab()
{
egpneightab_t *start = rfc1213_vars.rfc1213_egp.egpNeighTab;

	while(start) {
		kprintf("Dump - egpNeighState = %x\n",start->egpNeighState);
		kprintf("Dump - egpNeighAddr = %x.%x.%x.%x\n",
			start->egpNeighAddr[0],start->egpNeighAddr[1],
			start->egpNeighAddr[2],start->egpNeighAddr[3]);
		kprintf("Dump - egpNeighAs = %x\n",start->egpNeighAs);
		kprintf("Dump - egpNeighInMsgs = %x\n",start->egpNeighInMsgs);
		kprintf("Dump - egpNeighInErrs = %x\n",start->egpNeighInErrs);
		kprintf("Dump - egpNeighOutMsgs = %x\n",start->egpNeighOutMsgs);
		kprintf("Dump - egpNeighOutErrs = %x\n",start->egpNeighOutErrs);
		kprintf("Dump - egpNeighInErrMsgs = %x\n",start->egpNeighInErrMsgs);
		kprintf("Dump - egpNeighOutErrMsgs = %x\n",start->egpNeighOutErrMsgs);
		kprintf("Dump - egpNeighStateUps = %x\n",start->egpNeighStateUps);
		kprintf("Dump - egpNeighStateDowns = %x\n",start->egpNeighStateDowns);
		kprintf("Dump - egpNeighIntervalHello = %x\n",start->egpNeighIntervalHello);
		kprintf("Dump - egpNeighIntervalPoll = %x\n",start->egpNeighIntervalPoll);
		kprintf("Dump - egpNeighMode = %x\n",start->egpNeighMode);
		kprintf("Dump - egpNeighEventTrigger = %x\n",start->egpNeighEventTrigger);
		start = start->next;
	}
}
#endif
