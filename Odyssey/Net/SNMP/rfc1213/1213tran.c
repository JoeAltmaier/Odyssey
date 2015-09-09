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
 | FILE NAME   : 1213tran.c  
 | VERSION     : 1.0  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Supports RFC 1213 Transmission Group
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
#include "1213tran.h"
#include "1213xxxx.h"

extern 	rfc1213_vars_t	rfc1213_vars;


u16 
transmission( snmp_object_t * Obj, u16 IdLen, void * param )
{
	if(MibSimple(Obj, IdLen) == FALSE)
		return SNMP_NOSUCHNAME;
	x_memcpy(Obj->Syntax.BufInt, rfc1213_vars.rfc1213_trans.transNumber,
			rfc1213_vars.rfc1213_trans.transLen * sizeof(l32));
	Obj->SyntaxLen = rfc1213_vars.rfc1213_trans.transLen;
	return SNMP_NOERROR;
}
