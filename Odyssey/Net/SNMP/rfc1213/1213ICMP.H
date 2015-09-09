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
 | FILE NAME   : 1213icmp.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213icmp.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_ICMP_H_
#define _1213_ICMP_H_

u16 icmpInMsgs					(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInErrors				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInDestUnreachs			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInTimeExcds				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInParmProbs				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInSrcQuenchs			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInRedirects				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInEchos					(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInEchoReps				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInTimestamps			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInTimestampReps			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInAddrMasks				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpInAddrMaskReps			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutMsgs                 (snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutErrors               (snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutDestUnreachs			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutTimeExcds			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutParmProbs			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutSrcQuenchs			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutRedirects			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutEchos				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutEchoReps				(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutTimestamps			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutTimestampReps		(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutAddrMasks			(snmp_object_t *obj, u16 idlen, void *param);
u16 icmpOutAddrMaskReps			(snmp_object_t *obj, u16 idlen, void *param);

#endif
