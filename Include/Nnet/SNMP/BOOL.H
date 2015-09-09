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
#ifndef _BOOLSET_H
#define _BOOLSET_H

typedef ul32 BooleanSet_t;

#define BooleanSetAllFalse(b)		(b = (BooleanSet_t) 0UL)
#define BooleanSetAllTrue(b)		(b = (BooleanSet_t)0xFFFFFFFFUL)
#define BooleanSetFalse(b,n)		(b &= ~(2 << (n)))
#define BooleanSetTrue(b,n)			(b |= (2 << (n)))
#define BooleanCheckAllFalse(b)		(b == (BooleanSet_t) 0UL)
#define BooleanCheckAllTrue(b)		(b == (BooleanSet_t)0xFFFFFFFFUL)
#define BooleanCheck(b,n)			(b & (2 << (n)))

#endif

