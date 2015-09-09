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
 | FILE NAME   : 1213egp.h  
 | VERSION     : 1.1     
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213egp.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_EGP_H_
#define _1213_EGP_H_

u16 egpInMsgs      				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpInErrors  				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpOutMsgs  				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpOutErrors 				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighState 				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighAddr   				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighAs       			(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighInMsgs				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighInErrs				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighOutMsgs				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighOutErrs				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighInErrMsgs			(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighOutErrMsgs			(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighStateUps			(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighStateDowns 			(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighIntervalHello		(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighIntervalPoll		(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighMode  				(snmp_object_t *obj, u16 idlen, void *param);
u16 egpNeighEventTrigger		(snmp_object_t *obj, u16 idlen, void *param);
u16 egpAs                     	(snmp_object_t *obj, u16 idlen, void *param);

#endif
