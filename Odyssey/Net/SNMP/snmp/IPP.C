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
 | FILE NAME   : ipp.c                                  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : IP protocol specific functions                            
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


#define IP_VERSION 		4
#define SWAP(a) ((u16)((((ul32)(((u16)a))) << 16 | ((ul32)(((u16)a)))) >> 8))

typedef struct hdr_s hdr_t;

struct hdr_s {
	u8		ver_ihl;
	u8		tos;
	u16		length;
	u16		id;
	u16		flags_offset;
	u8		ttl;
	u8		prot;
	u16		check;
	ul32	src;
	ul32	dst;
};


static ul32 SumLink(link_t *link, u16 length);
static ul32 SumBuffer(u16 *buffer, u16 count);

u16 
IpH2NWord( u16 w )
{
    return (u16)x_htons(w);
}


u16 
IpN2HWord( u16 w )
{
    return (u16)x_ntohs(w);
}

ul32 
IpH2NDWord( ul32 d )
{
    return (ul32)x_htonl(d);
}

ul32 
IpN2HDWord( ul32 d )
{
    return (ul32)x_ntohl(d);
}


link_t *
IpHdrDecode( link_t *link, ip_hdr_t *ipHdr )
{
hdr_t   *h;
u16    n;

    h = (hdr_t *)LinkPop(&link, sizeof(hdr_t));
    if (h==0)
        return 0;

    ipHdr->ver      = (u8)(h->ver_ihl >> 4);
    ipHdr->ihl      = (h->ver_ihl & 0x0f) * 4;
    ipHdr->tos      = h->tos;
    ipHdr->length   = IpN2HWord(h->length);
    ipHdr->id       = IpN2HWord(h->id);
    ipHdr->flags    = (u8)(IpN2HWord(h->flags_offset) >> 13);
    ipHdr->offset   = IpN2HWord(h->flags_offset) & 0x1fff;
    ipHdr->ttl      = h->ttl;
    ipHdr->prot     = h->prot;
    ipHdr->check    = IpN2HWord(h->check);
    ipHdr->src      = IpN2HDWord(h->src);
    ipHdr->dst      = IpN2HDWord(h->dst);
    ipHdr->iol      = ipHdr->ihl - sizeof(hdr_t);
    
    
    LinkPush(&link, sizeof(hdr_t));

    if (IpHdrCheck(link, ipHdr->ihl) != 0)
        return 0;

    LinkPop(&link, sizeof(hdr_t));
    
    if (ipHdr->iol != 0)
    {
        ipHdr->options = (u8 *)LinkPop(&link, ipHdr->iol);
        if (ipHdr->options==0)
            return 0;
    }

    if (ipHdr->ver != IP_VERSION)
        return 0;

    n = ipHdr->length - ipHdr->ihl;
    if (link->length > n)
        link->length = n;

    return link;
}



link_t *
IpHdrEncode( link_t *link, ip_hdr_t *ipHdr )
{
hdr_t   *h;
u8    	*p;

    
    ipHdr->ver      = IP_VERSION;
    ipHdr->ihl      = sizeof(hdr_t) + ipHdr->iol;
    ipHdr->length   = ipHdr->ihl + LinkLength(link);
    ipHdr->check    = 0;

    if (ipHdr->iol != 0)
    {
        p = (u8 *)LinkPush(&link, ipHdr->iol);
        if (p==0)
            return 0;
        x_memcpy(p, ipHdr->options, ipHdr->iol);
    }

    h = (hdr_t *)LinkPush(&link, 20);
    if (h==0)
        return 0;

    h->ver_ihl      = (u8)((ipHdr->ver << 4) | (ipHdr->ihl/4 & 0x0f));
    h->tos          = ipHdr->tos;
    h->length       = IpH2NWord(ipHdr->length);
    h->id           = IpH2NWord(ipHdr->id);
    h->flags_offset = IpH2NWord((u16)((ipHdr->flags << 13) | (ipHdr->offset & 0x1fff))); 
    h->ttl          = ipHdr->ttl; 
    h->prot         = ipHdr->prot; 
    h->check        = IpH2NWord(ipHdr->check);
    h->src          = IpH2NDWord(ipHdr->src); 
    h->dst          = IpH2NDWord(ipHdr->dst);

    h->check        = IpHdrCheck(link, ipHdr->ihl);

    return link;
}


u16 
IpHdrCheck( link_t *link, u16 length )
{
ul32  sum = 0L;

    sum = SumLink(link, length);
    sum = (sum >> 16) + (sum & 0xffffL);
    sum = (sum >> 16) + (sum & 0xffffL);
    
    return (u16)~sum;
}


static ul32 
SumLink( link_t *link, u16 length )
{
ul32 sum = 0L;
bool odd = FALSE;
u16  n;
u8   *p;
    
    while (link != 0 && length > 0)
    {
        n = link->length;
        if (n > length)
        {
            n = length;
            length = 0;
        }
        else
        {
            length -= n;
        }

        if (n > 0)
        {
            p = link->buffer + link->offset;
            if (odd)
            {
                sum += (u16)*p << 8;
                p += 1;
                n -= 1;
                odd = FALSE;
            } 
            sum += SumBuffer((u16 *)p, (u16)(n >> 1));
            if (n & 1)
            {
                sum += (u16)*(p + n - 1);
                odd = TRUE;
            }
        }
        link = link->next;
    }
    return sum;
}


static ul32 
SumBuffer(u16 *buffer, u16 count)
{
ul32 sum = 0L;

    while (count-- > 0)
        sum += *buffer++;
    return sum;
}
