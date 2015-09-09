/************************************************************************* 
 |                                                                         
 | FILE NAME   : 1286base.h
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Support file for 1286base.c
 | AUTHOR      : Ryan Braun
 | DATE	       : 8/17/99
 |
 *************************************************************************/
 
#ifndef _1286_BASE_H_
#define _1286_BASE_H_

u16 BaseBridgeAddress( snmp_object_t * obj, u16 idlen, void * param );
u16 BaseNumPorts( snmp_object_t * obj, u16 idlen, void * param );
u16 BaseType( snmp_object_t * obj, u16 idlen, void * param );

#endif