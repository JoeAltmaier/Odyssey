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
#ifndef _RING_H
#define _RING_H

typedef struct RingPos_s {
	l32			Start;
	l32			Len;
} RingPos_t;

typedef struct Ring_s {
	l32			Size;
	l32 		Start;
	l32 		Stop;
	RingPos_t 	*PosList;
	l32 		PosCount;
	l32 		PosMax;
	u8 			*Buffer;
} Ring_t;

Ring_t 	*RingAlloc(l32 size, l32 elems, void *ptr);
void 	RingFree(Ring_t *ring);
bool 	RingSetMaxElems(Ring_t *ring, l32 elems);
l32 	RingGetMaxElems(Ring_t *ring);
l32 	RingGetElems(Ring_t *ring);
bool 	RingPutMem(Ring_t *ring, u8 *mem, l32 len, i32 unit);
bool 	RingGetMem(Ring_t *ring, u8 *mem, l32 *len);
bool 	RingPeekMem(Ring_t *ring, l32 elem, l32 offset, u8 **p, u8 *mem, l32 *len, i32 flag, i32 unit);
l32 	RingAvailMem(Ring_t *ring);

#endif
