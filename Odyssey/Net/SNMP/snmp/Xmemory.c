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
 | FILE NAME   : memory.c                              
 | VERSION     : 1.1 
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Memory size conversion functions                                                          
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

#ifdef __cplusplus
extern "C" {
#endif

l32  mem2long(u8 *mem) { return *(l32 *)mem; } 

void long2mem(u8 *mem, l32 val) { *(l32 *)mem = val; }

ul32  mem2ulong(u8 *mem) { return *(u32 *)mem; }
void  ulong2mem(u8 *mem, u32 val) { *(u32 *)mem = val; }

u16   mem2word(u8 *mem) { return *(u16 *)mem; }
void  word2mem(u8 *mem, u16 val) { *(u16 *)mem = val; }

ul32  mem2dword(u8 *mem) { return *(ul32 *)mem; }
void  dword2mem(u8 *mem, ul32 val) { *(ul32 *)mem = val; }

ul32  mem2lword(u8 *mem) { return *(ul32 *)mem; }
void  lword2mem(u8 *mem, ul32 val) { *(ul32 *)mem = val; }

i32   mem2int(u8 *mem) { return *(i32 *)mem; }
void  int2mem(u8 *mem, i32 val) { *(i32 *)mem = val; }

u32   mem2uint(u8 *mem) { return *(u32 *)mem; }
void  uint2mem(u8 *mem, u32 val) { *(u32 *)mem = val; }
#ifdef __cplusplus
}
#endif
