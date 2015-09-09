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
 | FILE NAME   : mac.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : MAC layer support functions
 | AUTHOR      : Robert Winter
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "ifudp.h"
#include "eth.h"
#include "xcfig.h"


#define MAC_TIME x_timeusec()

mac_prot_t        *macProtList=0;
mac_type_t        *macTypeList=0;
mac_coll_t        *macCollList=0;
mac_perf_t        macPerf;

//extern void SetMAC(i32 unit, mac_iface_t *mac);
extern xsnmp_cfig_t	xsnmp_cfg;
extern mac_iface_t *GetMAC( i32 unit );
extern void SetMAC( i32 unit, mac_iface_t *mac );

extern mac_iface_t	*gmac[];

void
AddMac( i8 *name, u32 type, u16 mtu, ul32 speed, u8 *addr, u8 *addrbroad,
		u16 addrsize, u8 *host, u16 hostaddrsize  )
{
mac_iface_t		*mac, *imac;
mac_ethstats_t	*eth;
i32 			i = 2;

    imac = xsnmp_cfg.mac_iface;
	if( !imac ) {
		imac = (mac_iface_t *)x_malloc(sizeof(mac_iface_t));
		if( imac ) {
			xsnmp_cfg.mac_iface = imac;
			x_memset(imac, 0, sizeof(mac_iface_t));
			eth = (mac_ethstats_t *)x_malloc(sizeof(mac_ethstats_t));
			if( eth ) {
				x_memset(eth, 0, sizeof(mac_ethstats_t));
				x_bcopy( (i8 *)name, (i8 *)&imac->descr[0], 48 );
				imac->type 			= (u16)type;
				imac->index 		= 1;
				imac->mtu 			= mtu;
				imac->speed			= speed;
				imac->statusAdmin	= TRUE;
				imac->statusOper	= TRUE;
                imac->arp           = MAC_ARP_NONE;
				x_memcpy( &imac->addr[0], addr, addrsize );
				x_memcpy( &imac->addrBroadcast[0], addrbroad, addrsize );
				imac->addrLength	= addrsize;
				x_memcpy( &imac->addrHost[0], host, hostaddrsize );
				imac->addrHostLen	= hostaddrsize;
				imac->frameId		= 0;
				imac->eth      		= eth;
				imac->next			= 0;
                SetMAC( 0, imac );
			} else {
				x_dbg("XSNMP, AddMac: no memory-1\n", TRUE);
			}
		} else {
			x_dbg("XSNMP, AddMac: no memory-2\n", TRUE);
		}
	} else {
    	mac = (mac_iface_t *)x_malloc( sizeof(mac_iface_t) );
    	if( mac ) {
			x_memset(mac, 0, sizeof(mac_iface_t));
			eth = (mac_ethstats_t *)x_malloc(sizeof(mac_ethstats_t));
			if( eth ) {
				x_memset(eth, 0, sizeof(mac_ethstats_t));
        		while( imac->next ) {
            		imac = imac->next;
            		i++;
        		}
				imac->next = mac;
				x_bcopy( (i8 *)name, (i8 *)&mac->descr[0], 48 );
				mac->type 			= (u16)type;
				mac->index 			= i;
				mac->mtu 			= mtu;
				mac->speed			= speed;
				mac->statusAdmin	= TRUE;
				mac->statusOper		= TRUE;
                mac->arp            = MAC_ARP_NONE;
				x_memcpy( &mac->addr[0], addr, addrsize );
				x_memcpy( &mac->addrBroadcast[0], addrbroad, addrsize );
				mac->addrLength		= addrsize;
				x_memcpy( &mac->addrHost[0], host, hostaddrsize );
				mac->addrHostLen	= hostaddrsize;
				mac->frameId		= 0;
				mac->eth     		= eth;
				mac->next			= 0;
                SetMAC( i-1, mac );
			} else {
        		x_dbg("XSNMP, AddMac: no memory-3\n", TRUE);
			}
     	} else {
        	x_dbg("XSNMP, AddMac: no memory\n", TRUE);
		}
    }
}

i32
GetIfaceIndex( i8 *name )
{
mac_iface_t *iface;

    for( iface=xsnmp_cfg.mac_iface; iface!=0; iface=iface->next ) {
        if( !(x_bcmp((i8 *)&iface->descr[0], (i8 *)name, x_strlen( name ))) ) {
            return(iface->index);
        }
    }
    return(0);
}


bool
MacInit( void )
{
static bool  init = FALSE;

    if (!init) {
        macPerf.on=FALSE;
        macPerf.pkts=0UL;
        macPerf.octets=0UL;
        macPerf.timeTotal=0UL;
        macPerf.timeMin=0xffffffffUL;
        macPerf.timeMax=0UL;
        init = TRUE;
    }
    return init;
}


bool
MacIfaceRegister( mac_iface_t *iface )
{
    iface->next = xsnmp_cfg.mac_iface;
    xsnmp_cfg.mac_iface = iface;

    return TRUE;
}


bool
MacIfaceRemove( mac_iface_t *iface )
{
mac_iface_t **p;

    p=&xsnmp_cfg.mac_iface;
    while (*p!=0)
    {
        if (*p==iface)
            *p=(*p)->next;
        else
            p=&(*p)->next;
    }
    return TRUE;
}

bool
MacTypeRegister( mac_type_t *type )
{
    type->next = macTypeList;
    macTypeList = type;

    return TRUE;
}

bool
MacTypeRemove( mac_type_t *type )
{
mac_type_t **p;

    p=&macTypeList;
    while (*p!=0) {
        if (*p==type)
            *p=(*p)->next;
        else
            p=&(*p)->next;
    }
    return TRUE;
}

bool
MacCollRegister( mac_coll_t *coll )
{
    coll->next = macCollList;
    macCollList = coll;

    return TRUE;
}

bool
MacCollRemove( mac_coll_t *coll )
{
mac_coll_t **p;

    p=&macCollList;
    while (*p!=0)
    {
        if (*p==coll)
            *p=(*p)->next;
        else
            p=&(*p)->next;
    }
    return TRUE;
}

bool
MacProtRegister( mac_prot_t *prot )
{
    prot->next = macProtList;
    macProtList = prot;

    return TRUE;

}

bool
MacProtRemove( mac_prot_t *prot )
{
mac_prot_t **p;

    p=&macProtList;
    while (*p!=0)
    {
        if (*p==prot)
            *p=(*p)->next;
        else
            p=&(*p)->next;
    }
    return TRUE;
}


bool
MacUpdate( u32 iIndex, i8 *name, u32 operStatus  )
{
    mac_iface_t     *mac;

    mac = GetMAC( iIndex );
    x_bcopy( (i8 *)name, (i8 *)&mac->descr[0], 48 );
    if ( operStatus == 1 )
        mac->statusOper = TRUE;
    else
        mac->statusOper = FALSE;

    return TRUE;
}

u32
Compare_Mac_OperStatus(u32 iIndex, u32 operStatus) {
    mac_iface_t     *mac;

    mac = GetMAC( iIndex );
    if ( (operStatus == 1) && (mac->statusOper) )
        return 1;
    if ( (operStatus == 2) && (!mac->statusOper) )
        return 1;
    return 0;
}

bool
MacRcve( mac_iface_t *iface, link_t *link, mac_info_t *info )
{
mac_coll_t    *coll;
static 		  prot_pkt_t  MacRcvepkt;

    if (macCollList != 0) {
        if (ProtFrame(&MacRcvepkt, link->buffer+link->offset,
            link->length, info->length, info->time,
			info->status, iface->type, iface->index, iface->frameId)) {
            iface->frameId++;
            if (iface->frameId > 0x7FFFFFFFL)
                iface->frameId = 0;
            for (coll=macCollList; coll!=0; coll=coll->next) {
				if( coll->ifindex == iface->index ) {
                	coll->Rcve(coll, &MacRcvepkt);
				}
			}
            ProtFree(&MacRcvepkt);
        }
    }

    return TRUE;
}


u16
MacIfaceCount( void )
{
mac_iface_t *p;
u16 count = 0;

    for (p=xsnmp_cfg.mac_iface; p!=0; p=p->next) {
        count++;
	}
    return count;
}


bool
MacIfaceCheck( void )
{
mac_iface_t *p;
u16 mini = 16, maxi = 0, count = 0;

    for (p=xsnmp_cfg.mac_iface; p!=0; p=p->next) {
        if (p->index < mini)
            mini = p->index;
        if (p->index > maxi)
            maxi = p->index;
        count++;
    }

    if (xsnmp_cfg.mac_iface != NULL && (mini != 1 || count != (maxi-mini+1)))
        return FALSE;

    return TRUE;
}


mac_iface_t *
MacIfaceFind( u8 *descr )
{
mac_iface_t *p;

    for (p=xsnmp_cfg.mac_iface; p!=0; p=p->next) {
        if (x_strcmp((i8 *)p->descr, (i8 *)descr)==0)
            return p;
    }
    return 0;
}

mac_iface_t *
MacIfaceGet( u16 mindex )
{
mac_iface_t *p;

    for (p=xsnmp_cfg.mac_iface; p!=0; p=p->next) {

           if (p->index==mindex) {
            return p;
		}
    }
    return 0;
}


bool
MacPerfStatistics( mac_perf_t *perf )
{
    x_memcpy(perf, &macPerf, sizeof(mac_perf_t));
    return TRUE;
}


bool
MacPerfSwitch( bool on )
{
    macPerf.on = on;
    macPerf.pkts=0UL;
    macPerf.octets=0UL;
    macPerf.timeTotal=0UL;
    macPerf.timeMin=0xffffffffUL;
    macPerf.timeMax=0UL;
    return TRUE;
}


bool
MacUndersize( u16 type, u16 len )
{
	switch (type) {
	case MAC_TYPE_ETHERNET_CSMACD:
	case MAC_TYPE_88023_CSMACD:
		if (len >= 60)
			return FALSE;
		break;
	}

	return TRUE;
}


bool
MacOversize( u16 type, u16 len )
{
	switch (type) {
	case MAC_TYPE_ETHERNET_CSMACD:
	case MAC_TYPE_88023_CSMACD:
		if (len <= 1514)
			return FALSE;
		break;
	}
	return TRUE;
}


void
EthRcve( u8 *frame, i32 length, mac_iface_t *mac )
{
static			link_t			EthRcvelink;
static			mac_info_t		EthRcveinfo;
u8				*p;

	p = (u8 *)frame;
    EthRcveinfo.copied      = length;
    EthRcveinfo.length      = length;
    EthRcveinfo.time        = 0;
    EthRcveinfo.promiscuous = ((p[0] & (u8)0x01)!=(u8)0x01 && x_memcmp(p, mac->addr, 6)!=0);
	EthRcveinfo.status      = 0;

    if (LinkAlloc(&EthRcvelink, p, (u16)length, (u16)length, 0, 0)==0)
        return;

    MacRcve(mac, &EthRcvelink, &EthRcveinfo);
}
