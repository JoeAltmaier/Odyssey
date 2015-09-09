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
 | FILE NAME   : 1213udp.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213udp.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_UDP_H_
#define _1213_UDP_H_

u16 udpInDatagrams				(snmp_object_t *obj, u16 idlen, void *param);
u16 udpNoPorts					(snmp_object_t *obj, u16 idlen, void *param);
u16 udpInErrors					(snmp_object_t *obj, u16 idlen, void *param);
u16 udpOutDatagrams				(snmp_object_t *obj, u16 idlen, void *param);
u16 udpLocalAddress				(snmp_object_t *obj, u16 idlen, void *param);
u16 udpLocalPort				(snmp_object_t *obj, u16 idlen, void *param);

#endif
