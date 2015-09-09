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
 | FILE NAME   : hash.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Hashing functions                                         
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
#include "hash.h"

#define DEFAULT_HTSIZE  53

static 	HashKey_t DefaultHashFunc(u8 * key, u32 n, u32 htsize);
static 	HashTable_t *NewHashTable(u32 size, HashFunction_t hf);
static 	void DelHashTable(HashTable_t * htp);
static 	HashKey_t GetHashKey(HashTable_t * ht, u8 * key, u32 n);
static 	BucketEntry_t *NewBucketEntry(u8 * key, u32 n);
static 	BucketEntry_t *ReturnBucketEntry(HashTable_t * ht, HashKey_t hk, u8 * key, u32 n);
static 	BucketEntry_t *GetBucketEntry(HashTable_t * ht, HashKey_t hk, u8 * key, u32 n);
static 	void DelBucketEntry(HashTable_t * ht, HashKey_t hk, BucketEntry_t * be);
static 	void DelBucketEntries(HashTable_t * ht, HashKey_t hk);
static 	void CleanBucketEntries(HashTable_t * ht, HashKey_t hk, HashCleanFunction_t f);
static 	void *GetEntryData(BucketEntry_t * be);
static 	void *SetEntryData(BucketEntry_t * be, void * data);
static 	u32 KeyEntryCmp(BucketEntry_t * be, u8 * key, u32 n);
static 	u32 KeyCmp(u8 * key1, u32 n1, u8 * key2, u32 n2);
static 	HashTable_t *curr_ht;

HashTable_t 	*NewHash(u32 size, HashFunction_t hf);
void 			DelHash(HashTable_t * ht);
void 			CleanHash(HashTable_t * ht, HashCleanFunction_t f);
void 			*HashAdd(HashTable_t * ht, u8 * key, u32 n, void * data);
void 			*HashRemove(HashTable_t * ht, u8 * key, u32 n);
void 			*HashSearch(HashTable_t * ht, u8 * key, u32 n);
u32 			HashSize(HashTable_t * ht);

HashKey_t 
DefaultHashFunc( u8 * key, u32 n, u32 htsize )
{
HashKey_t hk = 0;
u16 *keys;

	if (key == NULL || n == 0) return((HashKey_t)(-1));

	keys = (u16 *)key;
    if ( n > 6 )
        hk = keys[0] + keys[1] + keys[2] + keys[3] + keys[4] + keys[5];
    else
        hk = keys[0] + keys[1] + keys[2];

	return hk % htsize;
}

HashTable_t *
NewHash( u32 size, HashFunction_t hf )
{
	return NewHashTable(size, hf);
}

void 
DelHash( HashTable_t * ht )
{
	DelHashTable(ht);
}

void *
HashAdd( HashTable_t * ht, u8 * key, u32 n, void * data )
{
HashKey_t hk;
BucketEntry_t *be;

	curr_ht = ht;

	hk = GetHashKey(ht, key, n);

	if ((be = ReturnBucketEntry(ht, hk, key, n)) == NULL)
		return NULL;

	if (GetEntryData(be) != NULL)
		return NULL;

	SetEntryData(be, data);

	return data;
}

void *
HashRemove( HashTable_t * ht, u8 * key, u32 n )
{
void *data;
HashKey_t hk;
BucketEntry_t *be;

	hk = GetHashKey(ht, key, n);

	if ((be = GetBucketEntry(ht, hk, key, n)) == NULL)
		return NULL;

	data = GetEntryData(be);

	DelBucketEntry(ht, hk, be);

	return data;
}

void *
HashSearch( HashTable_t * ht, u8 * key, u32 n )
{
HashKey_t hk;
BucketEntry_t *be;

	hk = GetHashKey(ht, key, n);

	if ((be = GetBucketEntry(ht, hk, key, n)) == NULL)
		return NULL;

	return GetEntryData(be);
}

u32 
HashSize( HashTable_t * ht )
{
	return ht->Total;
}

void 
CleanHash( HashTable_t * htp, HashCleanFunction_t f )
{
HashKey_t hk;

	if (htp == NULL)
		return;

	for (hk = 0; hk < htp->Size && htp->Occupied > 0; hk++)
		CleanBucketEntries(htp, hk, f);

	return;
}

static HashTable_t *
NewHashTable( u32 size, HashFunction_t hf )
{
HashTable_t *htp;

	if (size > MAX_HASH_SIZE)
		return NULL;

	if ((htp = (HashTable_t *)x_malloc(sizeof(HashTable_t))) == NULL)
		return NULL;

	if (size == 0)
		htp->Size = size = DEFAULT_HTSIZE;
	else
		htp->Size = size;

	if (hf == NULL) {
		htp->HashFunc = hf = DefaultHashFunc;
	} else {
		htp->HashFunc = hf;
	}

	htp->Occupied = 0;
	htp->Total = 0;

	htp->Table = NULL;

	if ((htp->Table = (BucketEntry_t **)x_malloc(htp->Size * sizeof(BucketEntry_t *))) == NULL) {
		x_free(htp);
		return NULL;
	}

	x_memset(htp->Table, 0, htp->Size * sizeof(BucketEntry_t *));

	return htp;
}

static void 
DelHashTable( HashTable_t * htp )
{
HashKey_t hk;

	if (htp == NULL)
		return;

	for (hk = 0; hk < htp->Size && htp->Occupied > 0; hk++)
		DelBucketEntries(htp, hk);

	htp->Size = 0;
	htp->Occupied = 0;
	htp->HashFunc = NULL;

	x_free(htp->Table);
	htp->Table = NULL;

	x_free(htp);

	return;
}

static BucketEntry_t *
NewBucketEntry( u8 * key, u32 n )
{
BucketEntry_t *bep;

	if ((bep = (BucketEntry_t *)x_malloc(sizeof(BucketEntry_t))) == NULL)
		return NULL;

	bep->Key = key;
	bep->KeySize = n;
	bep->Contents = NULL;
	bep->Prev = NULL;
	bep->Next = NULL;

	return bep;
}

static HashKey_t 
GetHashKey( HashTable_t * ht, u8 * key, u32 n )
{
	if (ht == NULL)
		return (HashKey_t) - 1;
	return ht->HashFunc(key, n, ht->Size);
}

static BucketEntry_t *
ReturnBucketEntry( HashTable_t * ht, HashKey_t hk, u8 * key, u32 n )
{
BucketEntry_t *be;

	if (ht == NULL)
		return NULL;

	if (ht->Table[hk] == NULL)
	{
		if ((be = NewBucketEntry(key, n)) == NULL)
			return NULL;
		be->Prev = NULL;
		be->Next = NULL;
		ht->Table[hk] = be;
		ht->Occupied++;
		ht->Total++;

		return be;
	}

	for (be = ht->Table[hk]; be != NULL; be = be->Next)
		if (!KeyEntryCmp(be, key, n))
			break;

	if (be != NULL)
		return be;

	if ((be = NewBucketEntry(key, n)) == NULL)
		return NULL;
	be->Prev = NULL;
	be->Next = ht->Table[hk];
	ht->Table[hk]->Prev = be;
	ht->Table[hk] = be;
	ht->Total++;

	return be;
}

static BucketEntry_t *
GetBucketEntry( HashTable_t * ht, HashKey_t hk, u8 * key, u32 n )
{
BucketEntry_t *be;

	if (ht == NULL)
		return NULL;

	for (be = ht->Table[hk]; be != NULL; be = be->Next)
		if (!KeyEntryCmp(be, key, n))
			break;

	if (be != NULL)
		return be;

	return NULL;
}

static void 
DelBucketEntry( HashTable_t * ht, HashKey_t hk, BucketEntry_t * be )
{
	if (be->Prev != NULL)
		be->Prev->Next = be->Next;
	else
		ht->Table[hk] = be->Next;
	if (be->Next != NULL)
		be->Next->Prev = be->Prev;

	x_free(be);

	ht->Total--;
	if (ht->Table[hk] == NULL)
		ht->Occupied--;

	return;
}

static void 
DelBucketEntries( HashTable_t * ht, HashKey_t hk )
{
BucketEntry_t *be1, *be2;

	if (ht == NULL)
		return;

	for (be2 = ht->Table[hk]; be2 != NULL; be2 = be1)
	{
		be1 = be2->Next;
		x_free(be2);
	}
}

static void 
CleanBucketEntries( HashTable_t * ht, HashKey_t hk, HashCleanFunction_t f )
{
BucketEntry_t *be;

	if (ht == NULL)
		return;

	for (be = ht->Table[hk]; be != NULL; be = be->Next)
	{
		f(ht, GetEntryData(be));
	}
}

static void *
GetEntryData( BucketEntry_t * be )
{
	if (be == NULL)
		return NULL;
	return be->Contents;
}

static void *
SetEntryData( BucketEntry_t * be, void * data )
{
	if (be == NULL)
		return NULL;
	return be->Contents = data;
}

static u32 
KeyEntryCmp( BucketEntry_t * be, u8 * key, u32 n )
{
	if (be == NULL)
		return 1;
	return KeyCmp(be->Key, be->KeySize, key, n);
}

static u32 
KeyCmp( u8 * key1, u32 n1, u8 * key2, u32 n2 )
{
	u32 i;

	i = 0;
	while (i < n1 && i < n2 && key1[i] == key2[i])
		i++;

	if (i == n1 || i == n2)
		if (n1 == n2)
			return 0;
		else
			return (n1 < n2) ? -1 : 1;
	else
		return (key1[i] < key2[i]) ? -1 : 1;
}

