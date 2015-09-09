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
 | FILE NAME   : 1213sys.h  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1213sys.c
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#ifndef _1213_SYS_H_
#define _1213_SYS_H_

bool SysInit (void);
ul32 SysTime (void);
 
u16 sysDescr				(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysObjectID				(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysUpTime				(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysContact				(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysName					(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysLocation				(snmp_object_t *Obj, u16 IdLen, void *param);
u16 sysServices				(snmp_object_t *Obj, u16 IdLen, void *param);

#endif
