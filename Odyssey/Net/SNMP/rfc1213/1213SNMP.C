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
 | FILE NAME   : 1213snmp.c  
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 SNMP Group
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
#include "1213snmp.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;
extern snmp_stat_t		SnmpStat;

u16 
snmpInPkts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInPkts = SnmpStat.InPkts;
	return SNMP_NOERROR;
}

u16 
snmpOutPkts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutPkts = SnmpStat.OutPkts;
	return SNMP_NOERROR;
}

u16 
snmpInBadVersions( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInBadVersions = SnmpStat.InBadVersions;
	return SNMP_NOERROR;
}

u16 
snmpInBadCommunityNames( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInBadCommunityNames =
				(AgentStatistics())->InBadCommunityNames;
	return SNMP_NOERROR;
}

u16 
snmpInBadCommunityUses( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInBadCommunityUses =
				(AgentStatistics())->InBadCommunityUses;
	return SNMP_NOERROR;
}

u16 
snmpInASNParseErrs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInASNParseErrs = SnmpStat.InASNParseErrs;
	return SNMP_NOERROR;
}

u16 
snmpInTooBigs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInTooBigs = SnmpStat.InTooBigs;
	return SNMP_NOERROR;
}

u16 
snmpInNoSuchNames( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInNoSuchNames = SnmpStat.InNoSuchNames;
	return SNMP_NOERROR;
}

u16 
snmpInBadValues( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInBadValues = SnmpStat.InBadValues;
	return SNMP_NOERROR;
}

u16 
snmpInReadOnlys( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInReadOnlys = SnmpStat.InReadOnlys;
	return SNMP_NOERROR;
}

u16 
snmpInGenErrs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInGenErrs = SnmpStat.InGenErrs;
	return SNMP_NOERROR;
}

u16 
snmpInTotalReqVars( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInTotalReqVars =
				(AgentStatistics())->InTotalReqVars;
	return SNMP_NOERROR;
}

u16 
snmpInTotalSetVars( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInTotalSetVars =
				(AgentStatistics())->InTotalSetVars;
	return SNMP_NOERROR;
}

u16 
snmpInGetRequests( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInGetRequests = SnmpStat.InGetRequests;
	return SNMP_NOERROR;
}

u16 
snmpInGetNexts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInGetNexts = SnmpStat.InGetNexts;
	return SNMP_NOERROR;
}

u16 
snmpInSetRequests( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInSetRequests = SnmpStat.InSetRequests;
	return SNMP_NOERROR;
}

u16 
snmpInGetResponses( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInGetResponses = SnmpStat.InGetResponses;
	return SNMP_NOERROR;
}

u16 
snmpInTraps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpInTraps = SnmpStat.InTraps;
	return SNMP_NOERROR;
}

u16 
snmpOutTooBigs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutTooBigs = SnmpStat.OutTooBigs;
	return SNMP_NOERROR;
}

u16 
snmpOutNoSuchNames( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutNoSuchNames = SnmpStat.OutNoSuchNames;
	return SNMP_NOERROR;
}

u16 
snmpOutBadValues( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutBadValues = SnmpStat.OutBadValues;
	return SNMP_NOERROR;
}

u16 
snmpOutGenErrs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutGenErrs = SnmpStat.OutGenErrs;
	return SNMP_NOERROR;
}

u16 
snmpOutGetRequests( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutGetRequests = SnmpStat.OutGetRequests;
	return SNMP_NOERROR;
}

u16 
snmpOutGetNexts( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutGetNexts = SnmpStat.OutGetNexts;
	return SNMP_NOERROR;
}

u16 
snmpOutSetRequests( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutSetRequests = SnmpStat.OutSetRequests;
	return SNMP_NOERROR;
}

u16 
snmpOutGetResponses( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutGetResponses = SnmpStat.OutGetResponses;
	return SNMP_NOERROR;
}

u16 
snmpOutTraps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = 
		rfc1213_vars.rfc1213_snmp.snmpOutTraps = SnmpStat.OutTraps;
	return SNMP_NOERROR;
}

u16 
snmpEnableAuthenTraps( snmp_object_t * obj, u16 idlen, void * param )
{
bool enabled;

	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch (obj->Request) {
	case SNMP_PDU_NEXT:
	case SNMP_PDU_GET:
		enabled = AgentGetAuthenTraps();
		obj->Syntax.LngUns = (enabled == TRUE ? 1 : 2);
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		if(obj->Syntax.LngUns != 1 && obj->Syntax.LngUns != 2)
			return SNMP_BADVALUE;
		AgentSetAuthenTraps(obj->Syntax.LngUns == 1 ? TRUE : FALSE);
		return SNMP_NOERROR;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}
