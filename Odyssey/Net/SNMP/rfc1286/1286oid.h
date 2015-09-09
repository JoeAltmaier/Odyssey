/************************************************************************* 
 |                                                                         
 | FILE NAME   : 1286xxxx.h
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : OID file for RFC 1286
 | AUTHOR      : Ryan Braun
 | DATE	       : 8/17/99
 |
 *************************************************************************/
 
#ifndef _1286_OID_H_
#define _1286_OID_H_

/*------------------ dot1dBase Group ---------------------------------------*/
    { {1,3,6,1,2,1,17,1,1}, 9,  BaseBridgeAddress, SNMP_OCTETSTR, MIB_READ},
    { {1,3,6,1,2,1,17,1,2}, 9,  BaseNumPorts, SNMP_INTEGER, MIB_READ},    
    { {1,3,6,1,2,1,17,1,3}, 9,  BaseType, SNMP_INTEGER, MIB_READ},
    { {1,3,6,1,2,1,17,1,4,1,1}, 11, BasePort, SNMP_INTEGER, MIB_READ},
    { {1,3,6,1,2,1,17,1,4,1,2}, 11, BasePortIfIndex, SNMP_INTEGER, MIB_READ},

#endif
