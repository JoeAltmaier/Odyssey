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
 | FILE NAME   : udp.c                                  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : UDP protocol support functions                            
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
#include "xcfig.h"
#include "timer.h"
#include "xarp.h"
#include "ipp.h"
#include "ifudp.h"
#include "udp.h"
#include "1213udp.h"


typedef struct hdr_s        hdr_t;
typedef struct hdr_pseudo_s hdr_pseudo_t;


struct hdr_pseudo_s {
    ul32   src;
    ul32   dst;
    u8    zero;
    u8    prot;
    u16    length;
};

struct hdr_s {
    u16    src;
    u16    dst;           
    u16    length;
    u16    check;
};

static bool Addr2HostPort( i8 *addr, ul32 *host, u16 *port );

udp_descr_t *udpDescrList = 0;
udp_stat_t  udpStat;
u16      	udpPort = 1024;

bool 
UdpInit( void )
{
	NwInit();
    return TRUE;
}

udp_stat_t *
UdpStatistics( void )
{
    return &udpStat;
}

bool 
UdpSend( link_t *link, udp_hdr_t *udpHdr, ip_hdr_t *ipHdr, udp_descr_t *descr )
{
bool     success = FALSE;
u8       *frame;
static i8       UdpSendaddr[32];
i32      length;
    
    length = LinkLength(link);
    frame = (u8 *)x_malloc(length);  
    if(frame!=0) {
        if(LinkCopy(link, frame, (u16)length)) {
#ifdef XSTK_NUCNET
#ifdef XLIB_XSNMP
			plist[0].charptr = (i8 *)Inet_NtoA(ipHdr->dst);
			plist[1].intval  = udpHdr->dst;
            x_sprintf(UdpSendaddr,"udp:%s:%d"); 
#else
            sprintf(UdpSendaddr,"udp:%s:%d",Inet_NtoA(ipHdr->dst),udpHdr->dst); 
#endif
#else
#ifdef XLIB_XSNMP
			plist[0].charptr = (i8 *)Inet_NtoA(x_htonl(ipHdr->dst));
			plist[1].intval  = udpHdr->dst;
            x_sprintf(UdpSendaddr,"udp:%s:%d"); 
#else
            sprintf(UdpSendaddr,"udp:%s:%d",Inet_NtoA(x_htonl(ipHdr->dst)),udpHdr->dst); 
#endif
#endif
        
            if(NwDgSendTo(UdpSendaddr, frame, length, udpHdr->src)) {
                success = TRUE;
			} else {
			}
        }
       x_free(frame);
    }
    
    if(success) {
        udpStat.outDatagrams++;
	} else {
        udpStat.outErrors++;
	}
       
    return success;         
}

    
u16 
UdpAnyPort( void )
{
    return 0;
}

void 
UdpRcve( i8 *addr, u8 *frame, i32 length, void *parm )
{
static			udp_hdr_t		UdpRcveudpHdr; 
static			ip_hdr_t		UdpRcveipHdr;
link_t			*link;   
udp_descr_t		*descr = (udp_descr_t *) parm;
bool     		success = FALSE;
           
    if(Addr2HostPort(addr, &UdpRcveipHdr.src, &UdpRcveudpHdr.src))  {
        UdpRcveipHdr.dst=descr->locAddr;
        UdpRcveudpHdr.dst=descr->locPort;     
    
        link=LinkAlloc(0, frame, (u16)length, (u16)length, 0, 0); 
        if(link!=0) {
            if(descr->Rcve(descr, link, &UdpRcveudpHdr, &UdpRcveipHdr)) { /* RcveUdp in agent.c */
                success = TRUE;
            }
            LinkFree(link);
        }
    }
    
    if(success)
        udpStat.inDatagrams++;
    else
        udpStat.inErrors++;
}


static bool 
Addr2HostPort( i8 *addr, ul32 *host, u16 *port )
{
i8	*p,*q; 
static i8	HostPortstring[32]; 
    
    p=addr;
    
    q=HostPortstring;
    while(*p!=':' && *p!=0)
        *q++ = *p++; /*tolower(*p++); */
    *q++=0; 
    
    if(x_strcmp(HostPortstring,"udp")!=0) {
        return FALSE;
	}
    
    if(*p++!=':')  {
        return FALSE;
	}
    
    q=HostPortstring;
    while(*p!=':' && *p!=0)
        *q++=*p++;
    *q++=0; 
        
    *host=x_ntohl(x_inet_addr(HostPortstring));
    
    if(*p++!=':')  {
        return FALSE;
	}
    
    q=HostPortstring;
    while(*p!=':' && *p!=0)
        *q++=*p++;
    *q++=0;  
    
    *port=x_atoi(HostPortstring);
    
    return TRUE;
}

