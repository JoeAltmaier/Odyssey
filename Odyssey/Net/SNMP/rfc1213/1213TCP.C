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
 | FILE NAME   : 1213tcp.c  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 TCP Group
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
#include "1213tcp.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;		

static bool tcpMib( snmp_object_t * obj, u16 idlen );
u16 Get1213TcpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

bool 
tcpMib( snmp_object_t * obj, u16 idlen )
{
	if(obj->Request != SNMP_PDU_NEXT) {
		if(obj->IdLen != (u32)(idlen + 1))
			return FALSE;
		return TRUE;
	}
	if(obj->IdLen < idlen || obj->IdLen > (u32)(idlen + 1))	/* bad index */
		return FALSE;
	return TRUE;
}

u16 
tcpRtoAlgorithm( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngInt = rfc1213_vars.rfc1213_tcp.tcpRtoAlgorithm;
	return SNMP_NOERROR;
}

u16 
tcpRtoMin( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngInt = rfc1213_vars.rfc1213_tcp.tcpRtoMin;
	return SNMP_NOERROR;
}

u16 
tcpRtoMax( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngInt = rfc1213_vars.rfc1213_tcp.tcpRtoMax;
	return SNMP_NOERROR;
}

u16 
tcpMaxConn( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngInt = rfc1213_vars.rfc1213_tcp.tcpMaxConn;
	return SNMP_NOERROR;
}

u16 
tcpActiveOpens( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpActiveOpens;
	return SNMP_NOERROR;
}

u16 
tcpPassiveOpens( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpPassiveOpens;
	return SNMP_NOERROR;
}

u16 
tcpAttemptFails( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpAttemptFails;
	return SNMP_NOERROR;
}

u16 
tcpEstabResets( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpEstabResets;
	return SNMP_NOERROR;
}

u16 
tcpCurrEstab( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpCurrEstab;
	return SNMP_NOERROR;
}

u16 
tcpInSegs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpInSegs;
	return SNMP_NOERROR;
}

u16 
tcpOutSegs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpOutSegs;
	return SNMP_NOERROR;
}

u16 
tcpRetransSegs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpRetransSegs;
	return SNMP_NOERROR;
}

u16 
tcpInErrs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpInErrs;
	return SNMP_NOERROR;
}

u16 
tcpOutRsts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_tcp.tcpOutRsts;
	return SNMP_NOERROR;
}

u16
tcpConnState( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,6,13,1,1,0,0,0,0,0,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 10;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213TcpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
tcpConnLocalAddress( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,6,13,1,2,0,0,0,0,0,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 10;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213TcpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
tcpConnLocalPort( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,6,13,1,3,0,0,0,0,0,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 10;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213TcpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
tcpConnRemAddress( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,6,13,1,4,0,0,0,0,0,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 10;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213TcpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
tcpConnRemPort( snmp_object_t *obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,6,13,1,5,0,0,0,0,0,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 10;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213TcpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
