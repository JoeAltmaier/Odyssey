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
 | FILE NAME   : 1213tcp.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213tcp.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_TCP_H_
#define _1213_TCP_H_

u16 tcpRtoAlgorithm				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpRtoMin    				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpRtoMax   				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpMaxConn  				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpActiveOpens				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpPassiveOpens				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpAttemptFails  			(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpEstabResets				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpCurrEstab				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpInSegs     				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpOutSegs   				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpRetransSegs 				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpConnState  				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpConnLocalAddress			(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpConnLocalPort			(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpConnRemAddress			(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpConnRemPort				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpInErrs    				(snmp_object_t *obj, u16 idlen, void *param);
u16 tcpOutRsts    				(snmp_object_t *obj, u16 idlen, void *param);

#endif
