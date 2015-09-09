/* ClassTable.h -- System Class Tables (Private)
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *     11/14/98 Tom Nelson: Changed to dynamic tables
 *
**/

#ifndef __ClassTable_h
#define __ClassTable_h


#include <String.h>
#include "OsTypes.h"
#include "WaitQueue_T.h"

typedef WaitQueue_T<class DidLnk> DidQueue;
class DdmTask;

class ClassEntry {
public:
	char *pszName;
	U32  cbStack;
	U32   sQueue;
	CtorFunc ctor;
	U32 flags;
	
	DidQueue *pQueue;
	DdmTask  *pTask;
	U32 nDidInstance;
	U32 nDdmInstance;
	ClassEntry *pNextEntry;

	ClassEntry(char *_pszName,CtorFunc _ctor,U32 _flags,U32 _cbStack,U32 _sQueue) 	{ 
		pQueue = NULL;
		pTask = NULL;
		
		pszName = _pszName;
		ctor = _ctor;
		cbStack = _cbStack;
		sQueue = _sQueue;
		flags = _flags;
		
		pNextEntry = NULL;
		nDidInstance = nDdmInstance = 0;
	}
};

	
class ClassTable {
public:
	static ClassEntry *pFirstEntry;
	static ClassEntry *pLastEntry;
	static U32 nEntry;
	
public:
	static BOOL Add(char *pszName,U32 cbStack,U32 sQueue,CtorFunc ctor,ClassFlags flags);
	static BOOL Add(CtorFunc ctor,U16 reqCode,BOOL fVirtual);
	static ClassEntry * Find(CtorFunc pCtor);
	static ClassEntry * Find(char *pszName);
	static ClassEntry * GetFirst() 	{ return pFirstEntry; }
	static U32 GetCount()			{ return nEntry;	  }

	static void Dump(void);
};

#endif	// __ClassTable_H
