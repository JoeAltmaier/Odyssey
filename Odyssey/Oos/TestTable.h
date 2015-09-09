/* TestTable.h -- System Test Tables (Private to Chaos.lib)
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
 *     12/09/99 Ryan Braun: Created from FailTable.h
 *
**/

#ifndef __TestTable_h
#define __TestTable_h


#include <String.h>
#include "Os.h"

class TestEntry {
public:
	char *pszName;
	char *pszRules;	
	DID did;
	TySlot slot;
	
	TestEntry *pNextEntry;
	
	TestEntry(char *_pszName,char *_pszRules,DID _did,TySlot _slot) {
//		pszName = _pszName;
//		pszRules = _pszRules;		
		pszName = new char[strlen(_pszName) + 1];
		pszRules = new char[strlen(_pszRules) + 1];		
		memcpy(pszName, _pszName, strlen(_pszName) + 1);
		memcpy(pszRules, _pszRules, strlen(_pszRules) + 1);
		did = _did; 
		slot = _slot;
		pNextEntry = NULL;
	}
};

class TestTable {
public:
	static TestEntry *pFirstEntry;
	static TestEntry *pLastEntry;
	static U32 nEntry;

public:
	static BOOL Add(char *pszName,char *pszRules,DID _did,TySlot _slot);
	static TestEntry * Find(char *pszName);

	static void Dump(void);
	
	static TestEntry * GetFirst();
};

#endif	// __TestTable_H
