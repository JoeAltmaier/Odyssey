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
 | FILE NAME   : 1213snmp.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213snmp.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_SNMP_H_
#define _1213_SNMP_H_

u16 snmpInPkts					(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutPkts					(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInBadVersions			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInBadCommunityNames		(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInBadCommunityUses		(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInASNParseErrs			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInTooBigs				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInNoSuchNames			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInBadValues				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInReadOnlys				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInGenErrs				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInTotalReqVars			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInTotalSetVars			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInGetRequests			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInGetNexts				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInSetRequests			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInGetResponses			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpInTraps					(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutTooBigs				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutNoSuchNames			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutBadValues			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutGenErrs				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutGetRequests			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutGetNexts				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutSetRequests			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutGetResponses			(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpOutTraps				(snmp_object_t *obj, u16 idlen, void *param);
u16 snmpEnableAuthenTraps		(snmp_object_t *obj, u16 idlen, void *param);

#endif
