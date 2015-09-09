/************************************************************************* 
 |                                                                         
 | FILE NAME   : 1286xxxx.c
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support functions for RFC 1286 Groups
 | AUTHOR      : Ryan Braun
 | DATE	       : 8/17/99
 |
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
#include "1286xxxx.h"

#ifdef INTERNAL_RFC1286_STORAGE
rfc1286_vars_t		rfc1286_vars;
#else
extern rfc1286_vars_t		rfc1286_vars;
#endif

/*----------------------------------------------------------------------*
 * Base Port Table
 *----------------------------------------------------------------------*/ 
u16
Get1286BasePortTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag)
{
baseporttab_t *next, *last;
i32	newname[SNMP_SIZE_SMALLOBJECTID];
u32 len, foundit = 0;
i32 result = 0;

	len = idlen+sublen;
	x_memset(newname,0,(SNMP_SIZE_SMALLOBJECTID * sizeof(u32)));
	x_memcpy(newname,new,(len * sizeof(u32)));

	next = rfc1286_vars.rfc1286_Base.BasePortTab;
	while(next) {
		last = next;
		newname[idlen]   = next->BasePort;
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
Add1286BasePortTab(baseporttab_t *entry)
{
baseporttab_t *start = rfc1286_vars.rfc1286_Base.BasePortTab;
baseporttab_t *new, *last, *next;
u32 entered;
i32 result = 0;

	entered = 0;
	if(rfc1286_vars.rfc1286_Base.BasePortTab == 0) {
		start = (baseporttab_t *)x_malloc(sizeof(baseporttab_t));
		x_memset(start,0,sizeof(baseporttab_t));
		rfc1286_vars.rfc1286_Base.BasePortTab = start;
		x_memset((u8 *)start, 0, sizeof(baseporttab_t));
		x_memcpy(start->ipAdEntAddr, entry->ipAdEntAddr, MAX_1213_NETADDRSIZE);
		start->ipAdEntIfIndex = entry->ipAdEntIfIndex;
		x_memcpy(start->ipAdEntNetMask, entry->ipAdEntNetMask, MAX_1213_NETADDRSIZE);
		start->ipAdEntBcastAddr = entry->ipAdEntBcastAddr;
		start->ipAdEntReasmMaxSize = entry->ipAdEntReasmMaxSize;
		start->next = 0;
		start->last = 0;
		return(1);
	}
	new = (baseporttab_t *)x_malloc(sizeof(baseporttab_t));
	x_memset(new, 0, sizeof(baseporttab_t));
	next = rfc1286_vars.rfc1286_Base.BasePortTab;
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
				rfc1286_vars.rfc1286_Base.BasePortTab = new;
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
Del1286BasePortTab(baseporttab_t *entry)
{
baseporttab_t *last, *next, *start;
i32 result = 0;

	start = next = rfc1286_vars.rfc1286_Base.BasePortTab;
	while(next) {
		result = x_memcmp(entry->ipAdEntAddr,next->ipAdEntAddr,MAX_1213_NETADDRSIZE);
		if(!result) {
			/* single node to be deleted */
			if( (next->last == 0) && (next->next == 0)) {
				x_free((void *)next);
				rfc1286_vars.rfc1286_Base.BasePortTab = 0;
				break;
			} 
			/* beginning or first node to be deleted */
			else if(!next->last) {
				last = next;
				next = next->next;
				rfc1286_vars.rfc1286_Base.BasePortTab = next;
				next->last = last->next = 0;
				x_free((void *)last);
				break;
			}
			/* last or end node to be deleted */
			else if(!next->next) {
				next->last = last->next = 0;
				rfc1286_vars.rfc1286_Base.BasePortTab = start;
				x_free((void *)next);
				break;
			}
			/* node in between to be deleted */
			else {
				last->next = next->next;
				last->next->last = next->last;
				rfc1286_vars.rfc1286_Base.BasePortTab = start;
				x_free((void *)next);
				break;
			}
		} 
		last = next;
		next = next->next;
	}
}

u32
Size1286BasePortTab()
{
baseporttab_t *next;
u32 counter = 0;;

	next = rfc1286_vars.rfc1286_Base.BasePortTab;
	while(next) {
		counter++;
		next = next->next;
	}
	return(counter);
}

#if 0
void
Dump1286BasePortTab()
{
baseporttab_t *start = rfc1286_vars.rfc1286_Base.BasePortTab;

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
