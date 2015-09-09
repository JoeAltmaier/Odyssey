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
 | FILE NAME   : 1213icmp.c  
 | VERSION     : 1.1      
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 ICMP Group
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
#include "1213icmp.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;		

u16 
icmpInMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInMsgs;
	return SNMP_NOERROR;
}

u16 
icmpInErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInErrors;
	return SNMP_NOERROR;
}

u16 
icmpInDestUnreachs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInDestUnreachs;
	return SNMP_NOERROR;
}

u16 
icmpInTimeExcds( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInTimeExcds;
	return SNMP_NOERROR;
}

u16 
icmpInParmProbs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInParmProbs;
	return SNMP_NOERROR;
}

u16 
icmpInSrcQuenchs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInSrcQuenchs;
	return SNMP_NOERROR;
}

u16 
icmpInRedirects( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInRedirects;
	return SNMP_NOERROR;
}

u16 
icmpInEchos( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInEchos;
	return SNMP_NOERROR;
}

u16 
icmpInEchoReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInEchoReps;
	return SNMP_NOERROR;
}

u16 
icmpInTimestamps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInTimestamps;
	return SNMP_NOERROR;
}

u16 
icmpInTimestampReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInTimestampReps;
	return SNMP_NOERROR;
}

u16 
icmpInAddrMasks( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInAddrMasks;
	return SNMP_NOERROR;
}

u16 
icmpInAddrMaskReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpInAddrMaskReps;
	return SNMP_NOERROR;
}

u16
icmpOutMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
    if(MibSimple(obj, idlen) == FALSE)
        return SNMP_NOSUCHNAME;
    obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutMsgs;
    return SNMP_NOERROR;
}

u16 
icmpOutErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutErrors;
	return SNMP_NOERROR;
}

u16 
icmpOutDestUnreachs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutDestUnreachs;
	return SNMP_NOERROR;
}

u16 
icmpOutTimeExcds( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutTimeExcds;
	return SNMP_NOERROR;
}

u16 
icmpOutParmProbs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutParmProbs;
	return SNMP_NOERROR;
}

u16 
icmpOutSrcQuenchs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutSrcQuenchs;
	return SNMP_NOERROR;
}

u16 
icmpOutRedirects( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutRedirects;
	return SNMP_NOERROR;
}

u16 
icmpOutEchos( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutEchos;
	return SNMP_NOERROR;
}

u16 
icmpOutEchoReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutEchoReps;
	return SNMP_NOERROR;
}

u16 
icmpOutTimestamps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutTimestamps;
	return SNMP_NOERROR;
}

u16 
icmpOutTimestampReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutTimestampReps;
	return SNMP_NOERROR;
}

u16 
icmpOutAddrMasks( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutAddrMasks;
	return SNMP_NOERROR;
}

u16 
icmpOutAddrMaskReps( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_icmp.icmpOutAddrMaskReps;
	return SNMP_NOERROR;
}
