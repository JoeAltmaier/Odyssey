/* FailTable.cpp -- System Failure Tables
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
// 08/25/99 Joe Altmaier: Created from DeviceTable.cpp
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <String.h>
#include "FailTable.h"

FailEntry * FailTable::pFirstEntry = NULL;
FailEntry * FailTable::pLastEntry = NULL;
U32 FailTable::nEntry = 0;

// .Add -- Add Fail to System Fail Table ---------------------------------FailTable-
//
BOOL FailTable::Add(char *pszName,InitFunc function)
{
	FailEntry *pEntry;;

	pEntry = new FailEntry(pszName,function);
		
	if (pFirstEntry == NULL)
		pFirstEntry = pLastEntry = pEntry;
	else {
		pLastEntry->pNextEntry = pEntry;
		pLastEntry = pEntry;
	}			
	nEntry++;
			
	return TRUE;
}

// .Find -- Find Fail in system table by name ------------------------------FailTable-
//
// Scan Fail tables, return pointer to Fail entry or NULL if not found.
//
FailEntry * FailTable::Find(char *pszName)
{
	for (FailEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry)
		if (strcmp(pszName, pEntry->pszName) == 0)
			return pEntry;
				
	return NULL;
}

// .Start -- Invoke all Fail cleanup functions -----------------------------FailTable-
//
void FailTable::Fail()
{		
	for (FailEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry = pEntry->pNextEntry) {
//Tracef("start:\"%s\"\n",pEntries->aEntry[iEntry].pszName);		
			(*(pEntry->function))();
	}
}

// .Dump -- Dump data in persistant table ------------------------------------FailTable-
//
void FailTable::Dump() {

	Tracef("Fail Table:\n");
	Tracef("     Function  FailName\n");
	if (pFirstEntry == NULL)
		Tracef("  <empty>\n");

	for (FailEntry *pEntry = pFirstEntry; pEntry != NULL; pEntry=pEntry->pNextEntry) {
		Tracef("    %08x  \"%s\"\n",pEntry->function,pEntry->pszName);
	}
	Tracef("\n");
}
