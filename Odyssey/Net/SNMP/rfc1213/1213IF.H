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
 | FILE NAME   : 1213if.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213if.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_IF_H_
#define _1213_IF_H_

u16 ifNumber           (snmp_object_t *obj, u16 idlen, void *param);
u16 ifIndex            (snmp_object_t *obj, u16 idlen, void *param);
u16 ifDescr            (snmp_object_t *obj, u16 idlen, void *param);
u16 ifType             (snmp_object_t *obj, u16 idlen, void *param);
u16 ifMtu              (snmp_object_t *obj, u16 idlen, void *param);
u16 ifSpeed            (snmp_object_t *obj, u16 idlen, void *param);
u16 ifPhysAddress      (snmp_object_t *obj, u16 idlen, void *param);
u16 ifAdminStatus      (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOperStatus       (snmp_object_t *obj, u16 idlen, void *param);
u16 ifLastChange       (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInOctets         (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInUcastPkts      (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInNUcastPkts     (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInDiscards       (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInErrors         (snmp_object_t *obj, u16 idlen, void *param);
u16 ifInUnknownProtos  (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutOctets        (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutUcastPkts     (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutNUcastPkts    (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutDiscards      (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutErrors        (snmp_object_t *obj, u16 idlen, void *param);
u16 ifOutQLen          (snmp_object_t *obj, u16 idlen, void *param);
u16 ifSpecific         (snmp_object_t *obj, u16 idlen, void *param);

#endif
