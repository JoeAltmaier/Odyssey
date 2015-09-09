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
 | FILE NAME   : 1213at.c  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 AT Group
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
#include "1213at.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;
u16 Get1213AtTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

u16 
atIndex( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,3,1,1,1,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213AtTab(obj, idlen, sublen, (u32 *)new, getflag)) 
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
atPhysAddress( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,3,1,1,2,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213AtTab(obj, idlen, sublen, (u32 *)new, getflag)) 
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
atNetAddress( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,3,1,1,3,0,0,0,0,0,0};
u32 getflag;
u16 sublen = 5;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213AtTab(obj, idlen, sublen, (u32 *)new, getflag)) 
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
