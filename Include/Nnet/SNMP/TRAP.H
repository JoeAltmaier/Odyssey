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
#ifndef _TRAP_H_
#define _TRAP_H_

extern i8 *TrapSpecific[];

enum TrapSpecificCodes {
	TRAP_SPECIFIC_NONE = 0,
	TRAP_SPECIFIC_RISINGALARM,
	TRAP_SPECIFIC_FALLINGALARM,
	TRAP_SPECIFIC_PACKETMATCH,
	TRAP_SPECIFIC_REGISTER,
	TRAP_SPECIFIC_NEUROTHRESHOLD
};

#endif
