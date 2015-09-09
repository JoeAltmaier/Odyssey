/* TestTable.cpp -- System Test Tables
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
**/

// Revision History:
// 12/09/99 Ryan Braun: Created from FailTable.cpp
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "TestTable.h"

TestEntry * TestTable::pFirstEntry = NULL;
TestEntry * TestTable::pLastEntry = NULL;
U32 TestTable::nEntry = 0;

// .Add -- Add Test to System Test Table ---------------------------------TestTable-
//
BOOL TestTable::Add(char *pszName,char *pszArgs,DID did,TySlot slot)
{
	TestEntry *pEntry;;

	pEntry = new TestEntry(pszName,pszArgs,did,slot);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find Test in system table by name ------------------------------TestTable-
//
// Scan Test tables, return pointer to Test entry or NULL if not found.
//
TestEntry * TestTable::Find(char *pszName)
{
	for (TestEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (strcmp(pszName, pEntry->pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Dump -- Dump data in table ------------------------------------TestTable-
//
void TestTable::Dump() {

	Tracef("Test Table:\n");
	Tracef("     TestName TestArgs DID Slot\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (TestEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("     \"%s\"  \"%s\"  %d  %d\n",pEntry->pszName,pEntry->pszRules,pEntry->did,pEntry->slot);
	}
	Tracef("\n");
}

TestEntry * TestTable::GetFirst() {

	return pFirstEntry;

}
