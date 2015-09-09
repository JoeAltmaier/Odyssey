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
 | FILE NAME   : ring.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Ring buffer support                                       
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
#include "ring.h"

#ifdef XMIB_RMON1
extern rip_t *find_rip( u16 elem );
#endif

Ring_t *
RingAlloc( l32 size, l32 elems, void *ptr )
{
Ring_t *ring;

	if(elems < 0 || elems > size)
		return FALSE;                               
        
	if((ring = (Ring_t *)x_malloc(sizeof(Ring_t))) == NULL)
		return NULL;
		
	ring->Size = size;
	ring->Start = 0;
	ring->Stop = 0;
	
	ring->PosCount = 0;
	ring->PosMax = elems;
	
	if ((ring->Buffer = (u8 *)x_malloc(size)) == NULL) {
		RingFree(ring);
		return NULL;
	}
		
	return ring;
}


bool 
RingSetMaxElems( Ring_t *ring, l32 elems )
{
	if(ring == NULL || elems < ring->PosCount || elems > ring->Size)
		return FALSE;

	ring->PosMax = elems;
	
	return TRUE;
}


l32 
RingGetMaxElems( Ring_t *ring )
{						
	if(ring == NULL)
		return 0;
	return ring->PosMax;
}


l32 
RingGetElems( Ring_t *ring )
{
	if(ring == NULL)
		return 0;
	return ring->PosCount;
}


void 
RingFree( Ring_t *ring )
{
	if(ring == NULL)
		return;
	x_free(ring->Buffer);
	x_free(ring);
}


bool 
RingPutMem( Ring_t *ring, u8 *mem, l32 len, i32 unit )
{
	if(ring == NULL || ring->PosCount >= ring->PosMax) {
		return FALSE;
	} else {
		ring->PosCount++;
	}

	return TRUE;
}


bool 
RingGetMem( Ring_t *ring, u8 *mem, l32 *len )
{
	return TRUE;
}


#ifdef XMIB_RMON1
bool 
RingPeekMem( Ring_t *ring, l32 elem, l32 offset, u8 **p, u8 *mem, 
			l32 *len, i32 flag, i32 unit)
{
l32 n;
rip_t	*rip;

#ifdef XMIB_MIB2
	if(ring == NULL || elem >= ring->PosCount )
		return FALSE;

	if(rip = (rip_t *)find_rip((u16)elem)) { 
		if(rip->rip_used )
			n = rip->rip_size;
	} else {
		x_dbg("XRMON1: RingPeekMem; bad rip\n", TRUE);
		return(0);
	}

    if(mem != NULL && len != NULL && *len > 0 )
        n = *len;

	if( mem != NULL ) {
		if( flag ) {						/* if flag=0, get non-buffer info */
			x_memcpy( mem, rip, sizeof(rip_t) );
			*p = (u8 *)rip;
		} else {
			x_memcpy( mem, (u8 *)((u32)(rip->rip_pktp)+offset), n );
			*p = rip->rip_pktp;
		}
	}

	if( len != NULL )
		*len = n;
#endif
        
	return TRUE;
}
#endif /* XMIB_RMON1 */


l32 
RingAvailMem( Ring_t *ring )
{
	if(ring == NULL)
		return 0;
	return 1;  
}
