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
 | FILE NAME   : 1213ip.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213ip.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_IP_H_
#define _1213_IP_H_

u16 ipForwarding				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipDefaultTTL				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInReceives				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInHdrErrors				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInAddrErrors				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipForwDatagrams				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInUnknownProtos			(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInDiscards				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipInDelivers				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipOutRequests				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipOutDiscards				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipOutNoRoutes				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipReasmTimeout				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipReasmReqds				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipReasmOKs					(snmp_object_t *obj, u16 idlen, void *param);
u16 ipReasmFails				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipFragOKs   				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipFragFails 				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipFragCreates				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipAdEntAddr  				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipAdEntIfIndex				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipAdEntNetMask				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipAdEntBcastAddr			(snmp_object_t *obj, u16 idlen, void *param);
u16 ipAdEntReasmMaxSize			(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteDest  				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteIfIndex				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMetric1				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMetric2				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMetric3				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMetric4				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteNextHop				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteType  				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteProto  				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteAge    				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMask   				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteMetric5				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRouteInfo   				(snmp_object_t *obj, u16 idlen, void *param);
u16 ipNetToMediaIfIndex			(snmp_object_t *obj, u16 idlen, void *param);
u16 ipNetToMediaPhysAddress		(snmp_object_t *obj, u16 idlen, void *param);
u16 ipNetToMediaNetAddress		(snmp_object_t *obj, u16 idlen, void *param);
u16 ipNetToMediaType			(snmp_object_t *obj, u16 idlen, void *param);
u16 ipRoutingDiscards			(snmp_object_t *obj, u16 idlen, void *param);

#endif
