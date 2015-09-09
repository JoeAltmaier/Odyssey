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
 | FILE NAME   :                                        
 | VERSION     :      
 | COMPONENT   :
 | DESCRIPTION :                                                           
 | AUTHOR      :                                                            
 *************************************************************************/
#ifndef PROT_H
#define PROT_H

bool ProtXsnmpPrint (prot_obj_t *Obj, u8 **StrPtr);
extern i8 *protocolString;
extern i8 *protXsnmpString[];

static prot_ptr_t ProtPtr[] = {
    {"UNKNOWN"},
    {"XSNMP", NULL, NULL, ProtXsnmpPrint, protXsnmpString, 6}
};

#endif
