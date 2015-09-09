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
 | FILE NAME   : math.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Miscellaneous math functions                              
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"
#include "math.h"


u16 
WordMin( u16 a, u16 b )
{
    if (a < b) 	return a;
    else 		return b;
}


i32 
IntMin( i32 a, i32 b )
{
    if (a < b) 	return a;
    else 		return b;
}


l32 
LongMin( l32 a, l32 b )
{
    if (a < b) 	return a;
    else 		return b;
}


l32 
LongAbs( l32 a )
{
    if (a < 0) 	return -a;
    else 		return a;
}
