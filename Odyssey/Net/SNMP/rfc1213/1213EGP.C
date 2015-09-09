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
 | FILE NAME   : 1213egp.c  
 | VERSION     : 1.1                        
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 EGP Group
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
#include "1213egp.h"
#include "1213xxxx.h"

extern rfc1213_vars_t	rfc1213_vars;		

static bool egpMib( snmp_object_t * obj, u16 idlen );
static bool egpMibNext( snmp_object_t * obj, u16 idlen, u32 range );
u16 Get1213EgpTab(snmp_object_t *obj, u16 idlen, u16 sublen, u32 new[], u32 getflag);

bool 
egpMib( snmp_object_t * obj, u16 idlen )
{
	if(obj->Request != SNMP_PDU_NEXT) {
		if(obj->IdLen != (u32) (idlen + 1))
			return FALSE;
		return TRUE;
	}
	if(obj->IdLen < idlen || obj->IdLen > (u32) (idlen + 1))	/* bad index */
		return FALSE;
	return TRUE;
}

bool 
egpMibNext( snmp_object_t * obj, u16 idlen, u32 range )
{
	if(obj->IdLen == idlen) {
		obj->Id[idlen] = 1;
		obj->IdLen = idlen + 1;
		return TRUE;
	} else if( range > obj->Id[idlen]) {
		obj->Id[idlen]++;
		obj->IdLen = idlen + 1;
		return TRUE;
	}

	return FALSE;
}

u16 
egpInMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_egp.egpInMsgs;             
	return SNMP_NOERROR;
}

u16 
egpInErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_egp.egpInErrors;             
	return SNMP_NOERROR;
}

u16 
egpOutMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_egp.egpOutMsgs;             
	return SNMP_NOERROR;
}


u16 
egpOutErrors( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_egp.egpOutErrors;             
	return SNMP_NOERROR;
}

u16 
egpNeighState( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,1,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighAddr( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,2,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighAs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,3,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighInMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,4,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighInErrs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,5,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighOutMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,6,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighOutErrs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,7,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighInErrMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,8,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighOutErrMsgs( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,9,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighStateUps( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,10,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighStateDowns( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,11,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighIntervalHello( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,12,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighIntervalPoll( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,13,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighMode( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,14,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpNeighEventTrigger( snmp_object_t * obj, u16 idlen, void * param )
{
i32 new[SNMP_SIZE_OBJECTID] = {1,3,6,1,2,1,8,5,1,15,0,0,0,0,0};
u32 getflag;
u16 sublen = 4;

	getflag = 0;
	switch (obj->Request) {
	case SNMP_PDU_GET:
		getflag++;
	case SNMP_PDU_NEXT:
		if(!Get1213EgpTab(obj, idlen, sublen, (u32 *)new, getflag))
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
egpAs( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
	obj->Syntax.LngUns = rfc1213_vars.rfc1213_egp.egpAs;             
	return SNMP_NOERROR;
}


