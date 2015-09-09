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
 | FILE NAME   : 1213ip.c  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 IP Group
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
#include "1213ip.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;		

static bool ipMib( snmp_object_t * obj, u16 idlen );
u16 Get1213IpAddrTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);
u16 Get1213IpRouteTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);
u16 Get1213IpNet2MediaTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

bool 
ipMib( snmp_object_t * obj, u16 idlen )
{
	if(obj->Request != SNMP_PDU_NEXT) {
		if(obj->IdLen != (u32) (idlen + 1))   
			return FALSE;
		return TRUE;
	}
	if(obj->IdLen < idlen || obj->IdLen > (u32) (idlen + 1))	
		return FALSE;                         
	return TRUE;
}


u16 
ipForwarding( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	if(obj->Request == SNMP_PDU_SET) {
		if((obj->Syntax.LngInt == 1) || (obj->Syntax.LngInt == 2)) {
			rfc1213_vars.rfc1213_ip.ipForwarding = obj->Syntax.LngInt;
		} else {
			return SNMP_GENERROR;
		}
	} else {
		obj->Syntax.LngInt = rfc1213_vars.rfc1213_ip.ipForwarding;
	}
	return SNMP_NOERROR;
}

u16 
ipDefaultTTL( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	if(obj->Request == SNMP_PDU_SET) {
		rfc1213_vars.rfc1213_ip.ipDefaultTTL = obj->Syntax.LngInt;
	} else {
		obj->Syntax.LngInt = rfc1213_vars.rfc1213_ip.ipDefaultTTL;
	}
	return SNMP_NOERROR;
}


u16 
ipInReceives( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInReceives;
	return SNMP_NOERROR;
}

u16 
ipInHdrErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInHdrErrors;
	return SNMP_NOERROR;
}


u16 
ipInAddrErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInAddrErrors;
	return SNMP_NOERROR;
}

u16 
ipForwDatagrams( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipForwDatagrams;
	return SNMP_NOERROR;
}


u16 
ipInUnknownProtos( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInUnknownProtos;
	return SNMP_NOERROR;
}

u16 
ipInDiscards( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInDiscards;
	return SNMP_NOERROR;
}

u16 
ipInDelivers( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipInDelivers;
	return SNMP_NOERROR;
}


u16 
ipOutRequests( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipOutRequests;
	return SNMP_NOERROR;
}

u16 
ipOutDiscards( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipOutDiscards;
	return SNMP_NOERROR;
}

u16 
ipOutNoRoutes( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipOutNoRoutes;
	return SNMP_NOERROR;
}


u16 
ipReasmTimeout( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngInt = rfc1213_vars.rfc1213_ip.ipReasmTimeout;
	return SNMP_NOERROR;
}

u16 
ipReasmReqds( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipReasmReqds;
	return SNMP_NOERROR;
}


u16 
ipReasmOKs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipReasmOKs;
	return SNMP_NOERROR;
}


u16 
ipReasmFails( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipReasmFails;
	return SNMP_NOERROR;
}


u16 
ipFragOKs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipFragOKs;
	return SNMP_NOERROR;
}

u16 
ipFragFails( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipFragFails;
	return SNMP_NOERROR;
}

u16 
ipFragCreates( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipFragCreates;
	return SNMP_NOERROR;
}

u16 
ipRoutingDiscards( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_ip.ipRoutingDiscards;
	return SNMP_NOERROR;
}


u16
ipAdEntAddr( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,20,1,1,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpAddrTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipAdEntIfIndex( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,20,1,2,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpAddrTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipAdEntNetMask( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,20,1,3,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpAddrTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipAdEntBcastAddr( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,20,1,4,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpAddrTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipAdEntReasmMaxSize( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,20,1,5,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpAddrTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteDest( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,1,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteIfIndex( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,2,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMetric1( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,3,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMetric2( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,4,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMetric3( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,5,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMetric4( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,6,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteNextHop( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,7,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteType( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,8,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteProto( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,9,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteAge( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,10,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMask( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,11,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteMetric5( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,12,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipRouteInfo( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,21,1,13,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpRouteTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipNetToMediaIfIndex( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,22,1,1,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpNet2MediaTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipNetToMediaPhysAddress( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,22,1,2,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpNet2MediaTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipNetToMediaNetAddress( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,22,1,3,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpNet2MediaTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ipNetToMediaType( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,4,22,1,4,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213IpNet2MediaTab(obj, idlen, sublen, (u32 *)new, getflag))
			return SNMP_NOSUCHNAME;
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		return SNMP_READONLY;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

