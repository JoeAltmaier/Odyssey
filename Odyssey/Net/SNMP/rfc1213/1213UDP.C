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
 | FILE NAME   : 1213udp.c  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 UDP Group
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
#include "1213udp.h"
#include "1213xxxx.h"

static bool udpMib(snmp_object_t * obj, u16 idlen);
u16 Get1213UdpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

extern rfc1213_vars_t	rfc1213_vars;


bool 
udpMib( snmp_object_t * obj, u16 idlen )
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
udpInDatagrams( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_udp.udpInDatagrams;
	return SNMP_NOERROR;
}

u16 
udpNoPorts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_udp.udpNoPorts;
	return SNMP_NOERROR;
}

u16 
udpInErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_udp.udpInErrors;
	return SNMP_NOERROR;
}

u16 
udpOutDatagrams( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	obj->Syntax.LngUns = rfc1213_vars.rfc1213_udp.udpOutDatagrams;
	return SNMP_NOERROR;
}

u16 
udpLocalAddress( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,7,5,1,1,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213UdpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
udpLocalPort( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,7,5,1,2,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213UdpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
