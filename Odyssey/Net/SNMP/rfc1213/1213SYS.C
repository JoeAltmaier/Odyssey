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
 | FILE NAME   : 1213sys.c  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 System Group
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
#include "1213sys.h"
#include "1213xxxx.h"

extern 	xsnmp_cfig_t	xsnmp_cfg;
extern 	rfc1213_vars_t	rfc1213_vars;



bool 
Mib2Init( void )
{
i8  *t;
static i8	ObjectID[SNMP_SIZE_BUFCHR];

#ifdef INTERNAL_RFC1213_STORAGE
	x_bzero( (i8 *)&(rfc1213_vars), sizeof(rfc1213_vars_t) );
#endif

	rfc1213_vars.rfc1213_sys.sysUpTime					= x_timeticks(); 
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysDescr),    		&xsnmp_cfg.sys_description[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysContact),     	&xsnmp_cfg.sys_contact[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysName),        	&xsnmp_cfg.sys_name[0]);
	x_strcpy((i8 *)(rfc1213_vars.rfc1213_sys.sysLocation),    	&xsnmp_cfg.sys_location[0]);
	rfc1213_vars.rfc1213_sys.sysServices 				= xsnmp_cfg.sys_services;
	x_strcpy(ObjectID, (c_i8 *)&xsnmp_cfg.sys_objectid[0]);

	rfc1213_vars.rfc1213_sys.sysObjectIDLen     	= 0;
	if((t = (i8 *)x_strtok(ObjectID, ".")) != NULL) {
		rfc1213_vars.rfc1213_sys.sysObjectID[rfc1213_vars.rfc1213_sys.sysObjectIDLen++] = x_atol(t);
		while((t = (i8 *)x_strtok(NULL, ".")) != NULL) {
			rfc1213_vars.rfc1213_sys.sysObjectID[rfc1213_vars.rfc1213_sys.sysObjectIDLen++] = x_atol(t);
		}
	}
	return TRUE;
}

u16 
sysDescr( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	x_strcpy((i8 *)Obj->Syntax.BufChr, (c_i8 *)&(rfc1213_vars.rfc1213_sys.sysDescr[0]));
	Obj->SyntaxLen = x_strlen((c_i8 *)Obj->Syntax.BufChr);
	return SNMP_NOERROR;
}

u16 
sysObjectID( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	x_memcpy(Obj->Syntax.BufInt, rfc1213_vars.rfc1213_sys.sysObjectID, 
			rfc1213_vars.rfc1213_sys.sysObjectIDLen * sizeof(l32));
	Obj->SyntaxLen = rfc1213_vars.rfc1213_sys.sysObjectIDLen;
	return SNMP_NOERROR;
}

u16 
sysUpTime( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
#ifdef XOS_NUC
	Obj->Syntax.LngUns = SysTime();
#else
	Obj->Syntax.LngUns = rfc1213_vars.rfc1213_sys.sysUpTime;
#endif
	return SNMP_NOERROR;
}

u16 
sysContact( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	if (Obj->Request == SNMP_PDU_SET) {
		x_memcpy(rfc1213_vars.rfc1213_sys.sysContact, Obj->Syntax.BufChr, Obj->SyntaxLen);
		rfc1213_vars.rfc1213_sys.sysContact[Obj->SyntaxLen] = '\0';
	} else {
		x_strcpy((i8 *)Obj->Syntax.BufChr, (i8 *)rfc1213_vars.rfc1213_sys.sysContact);
		Obj->SyntaxLen = x_strlen((c_i8 *)Obj->Syntax.BufChr);
	}
	return SNMP_NOERROR;
}

u16 
sysName( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	if (Obj->Request == SNMP_PDU_SET) {
		x_memcpy(rfc1213_vars.rfc1213_sys.sysName, Obj->Syntax.BufChr, Obj->SyntaxLen);
		rfc1213_vars.rfc1213_sys.sysName[Obj->SyntaxLen] = '\0';
	} else {
		x_strcpy((i8 *)Obj->Syntax.BufChr, (i8 *)rfc1213_vars.rfc1213_sys.sysName);
		Obj->SyntaxLen = x_strlen((c_i8 *)Obj->Syntax.BufChr);
	}
	return SNMP_NOERROR;
}

u16 
sysLocation( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	if(Obj->Request == SNMP_PDU_SET) {
		x_memcpy(rfc1213_vars.rfc1213_sys.sysLocation, Obj->Syntax.BufChr, Obj->SyntaxLen);
		rfc1213_vars.rfc1213_sys.sysLocation[Obj->SyntaxLen] = '\0';
	} else {
		x_strcpy((i8 *)Obj->Syntax.BufChr, (i8 *)rfc1213_vars.rfc1213_sys.sysLocation);
		Obj->SyntaxLen = x_strlen((c_i8 *)Obj->Syntax.BufChr);
	}
	return SNMP_NOERROR;
}

u16 
sysServices( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	Obj->Syntax.LngInt = rfc1213_vars.rfc1213_sys.sysServices;
	return SNMP_NOERROR;
}
