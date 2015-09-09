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
 | FILE NAME   : 1213if.c
 | VERSION     : 1.1
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 IF Group
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
#include "1213if.h"

static bool IfMib( snmp_object_t * obj, u16 idlen );
static bool IfMibNext( snmp_object_t * obj, u16 idlen );

extern u32  get_numports(void);

bool
IfMib( snmp_object_t * obj, u16 idlen )
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
IfMibNext( snmp_object_t * obj, u16 idlen )
{
	if(obj->IdLen == idlen) {
		obj->Id[idlen] = 1;
		obj->IdLen = idlen + 1;
		return TRUE;
    } else if(get_numports() > obj->Id[idlen]) {
		obj->Id[idlen]++;
		obj->IdLen = idlen + 1;
		return TRUE;
	}

	return FALSE;
}

u16
ifNumber( snmp_object_t * obj, u16 idlen, void * param )
{
	if(MibSimple(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;
    obj->Syntax.LngInt = get_numports();
	return SNMP_NOERROR;
}

u16
ifIndex( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL) {
			return SNMP_NOSUCHNAME;
		}
		obj->Syntax.LngInt = obj->Id[idlen];
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
ifDescr( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
#ifdef XLIB_XSNMP
		plist[0].charptr = iface->descr;
		x_sprintf((i8 *)obj->Syntax.BufChr, "%s");
#else
		sprintf((i8 *)obj->Syntax.BufChr, "%s", iface->descr);
#endif
		obj->SyntaxLen = x_strlen((c_i8 *)obj->Syntax.BufChr);
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
ifType( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngInt = iface->type;
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
ifMtu( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngInt = iface->mtu;
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
ifSpeed( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->speed;
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
ifPhysAddress( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE) {
			return SNMP_NOSUCHNAME;
		}
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL) {
			return SNMP_NOSUCHNAME;
		}
		x_memcpy(obj->Syntax.BufChr, iface->addr, obj->SyntaxLen = iface->addrLength);
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
ifAdminStatus( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngInt = (iface->statusAdmin == TRUE ? 1 : 2);
		return SNMP_NOERROR;
	case SNMP_PDU_SET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		if(obj->Syntax.LngInt != 1 && obj->Syntax.LngInt != 2)
			return SNMP_BADVALUE;
		iface->statusAdmin = (obj->Syntax.LngInt == 1 ? TRUE : FALSE);
		return SNMP_NOERROR;
	case SNMP_PDU_COMMIT:
		return SNMP_NOERROR;
	case SNMP_PDU_UNDO:
		return SNMP_GENERROR;
	}
	return SNMP_GENERROR;
}

u16
ifOperStatus( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngInt = (iface->statusOper == TRUE ? 1 : 2);
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
ifLastChange( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = 0;
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
ifInOctets(snmp_object_t * obj, u16 idlen, void * param)
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inOctets;
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
ifInUcastPkts( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inUcastPkts;
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
ifInNUcastPkts( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inNUcastPkts;
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
ifInDiscards( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inDiscards;
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
ifInErrors( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inErrors;
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
ifInUnknownProtos( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->inUnknownProtos;
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
ifOutOctets( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outOctets;
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
ifOutUcastPkts( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outUcastPkts;
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
ifOutNUcastPkts( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outNUcastPkts;
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
ifOutDiscards( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outDiscards;
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
ifOutErrors( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outErrors;
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
ifOutQLen( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t *iface;

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch(obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		obj->Syntax.LngUns = iface->eth->outQLen;
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
ifSpecific( snmp_object_t * obj, u16 idlen, void * param )
{
mac_iface_t	*iface;
static l32 		dot3[] = {1, 3, 6, 1, 2, 1, 10, 7};
static l32 		oidnull[] = {0, 0};

	if(IfMib(obj, idlen) == FALSE)
		return SNMP_NOSUCHNAME;

	switch (obj->Request) {
	case SNMP_PDU_NEXT:
		if(IfMibNext(obj, idlen) == FALSE)
			return SNMP_NOSUCHNAME;
	case SNMP_PDU_GET:
		if((iface = MacIfaceGet((u16)obj->Id[idlen])) == NULL)
			return SNMP_NOSUCHNAME;
		switch (iface->type) {
		case MAC_TYPE_ETHERNET_CSMACD:
		case MAC_TYPE_88023_CSMACD:
			x_memcpy(obj->Syntax.BufInt, dot3, sizeof(dot3));
			obj->SyntaxLen = sizeof(dot3) / sizeof(dot3[0]);
			break;
		default:
			x_memcpy(obj->Syntax.BufInt, oidnull, sizeof(oidnull));
			obj->SyntaxLen = sizeof(oidnull) / sizeof(oidnull[0]);
			break;
		}
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
