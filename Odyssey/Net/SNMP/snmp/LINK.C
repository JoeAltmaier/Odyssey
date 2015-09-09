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
 | FILE NAME   : link.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports the LINK notion                                  
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


u16 
LinkAvailSpace( link_t *link )
{
    return link->offset;
}

u16 
LinkAvailData( link_t *link )
{
    return link->length;
}

u16 
LinkLength( link_t *link )
{
u16        n=0;
link_t     *p;

    for (p=link; p!=0; p=p->next)
        n += p->length;

    return n;
}

u16 
LinkSize( link_t *link )
{
u16        n=0;
link_t     *p;

    for (p=link; p!=0; p=p->next)
        n += p->size;

    return n;
}


u8 *
LinkPush( link_t **link, u16 size )
{
link_t   *p;


    p = *link;
    if (p==0 || p->offset < size) {
        p = (link_t *)x_malloc(sizeof(link_t));
        if (p == 0) {
            return 0;
		}
		if(!size)
			x_dbg("XSNMP, LinkPush: zero size\n", TRUE);
        p->buffer = (u8 *)x_malloc(size);
        if (p->buffer == 0) {
            return 0;
		}
        p->size   = size;
        p->length = size;
        p->offset = 0;
        p->flags  = LINK_FLAG_BUFFER | LINK_FLAG_LINK;
        p->next   = *link;
        *link    = p;
    } else {
        p->length += size;
        p->offset -= size;
    }

    return (p->buffer + p->offset);
}


u8 *
LinkPop( link_t **link, u16 size )
{
link_t   *p;
    
    p = *link;
    if (p==0) {
        return 0;
	}
    
    if (p->length < size) {
        return 0;
    } else {
        p->length -= size;
        p->offset += size;
    }
    return (p->buffer + p->offset - size);
}


link_t *
LinkAlloc( link_t *link, u8 *buffer, u16 size, u16 length, u16 offset, link_t *next )
{
    if (link == 0) {
        link = (link_t *)x_malloc(sizeof(link_t));
        if (link == 0) {
            return 0;
		}
        link->flags = LINK_FLAG_LINK;
    }
    if (buffer == 0) {
		if(!size)
			x_dbg("XSNMP, LinkAlloc: size == 0\n", TRUE);
        buffer = (u8 *)x_malloc(size);
        if (buffer == 0) {
            return 0;
		}
        link->flags |= LINK_FLAG_BUFFER;
    }
    
    link->buffer   = buffer;
    link->size     = size;
    link->length   = length;
    link->offset   = offset;
    link->next     = next;
    
    return link;
}

void 
LinkFree( link_t *link )
{
   if (link->flags & LINK_FLAG_BUFFER) {
       x_free(link->buffer);
   }
   if (link->flags & LINK_FLAG_LINK) {
       x_free(link);
   }
}

bool 
LinkCopy( link_t *link, u8 *buffer, u16 size )
{
    if (LinkLength(link) > size) {
        return FALSE;
	}
    
    while (link!=0) {
        x_memcpy(buffer, link->buffer + link->offset, link->length);
        buffer += link->length;
        link = link->next;
    }
    return TRUE;
}


bool 
LinkSplit( link_t *head, link_t *tail, u16 length )
{
link_t *p;

    if (head==0 || tail==0) {
        return FALSE;
	}
    
    p = head;
    while (p != 0 && length > p->length) {
        length -= p->length;
        p       = p->next;
    }

    if (p==0) {
        tail->buffer = 0;
        tail->size   = 0;
        tail->offset = 0;
        tail->length = 0;
        return FALSE;
    } else {
        tail->next   = p->next;
        tail->buffer = p->buffer;
        tail->size   = p->size;
        tail->offset = p->offset + length;
        tail->length = p->length - length;
        head->length = length;
        return TRUE;
    }
}

