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
#ifndef _HASH_H
#define _HASH_H

#define MAX_HASH_SIZE           16384
#define MAX_BUCKET_SIZE         4096

typedef u32 HashKey_t;
typedef HashKey_t(*HashFunction_t) (u8 *, u32, u32);

typedef struct BucketEntry_s {
	u8						*Key;
	u32						KeySize;
	void					*Contents;
	struct BucketEntry_s	*Prev;
	struct BucketEntry_s	*Next;
} BucketEntry_t;

typedef struct HashTable_s {
	u32						Size;
	u32						Occupied;
	u32						Total;
	HashFunction_t			HashFunc;
	BucketEntry_t			**Table;
} HashTable_t;

typedef void (*HashCleanFunction_t) (HashTable_t *ht, void *);

#endif
